#include "LaserDrone.h"

#include "Common/Utils.h"
#include "World/WorldManager.h"
#include "Effects/VFXEmitterInstantiable.h"
#include "Characters/Enemies/Enemy.h"

#include "EASTL/array.h"

namespace Orin::Overkill
{
    ENTITYREG(SceneEntity, LaserDrone, "Overkill/Characters", "LaserDrone")

    META_DATA_DESC(LaserDrone::LaserRay)
		FLOAT_PROP(LaserDrone::LaserRay, directionAngle, 0.f, "LaserRay", "directionAngle", "Direction Angle")
		FLOAT_PROP(LaserDrone::LaserRay, maxLength, -1.f, "LaserRay", "maxLength", "Max Length")
	META_DATA_DESC_END()

	META_DATA_DESC(LaserDrone)
		BASE_SCENE_ENTITY_PROP(LaserDrone)

        ARRAY_PROP(LaserDrone, laserRays, LaserRay, "Laser", "laserRays")
        MARK_DISABLED_FOR_INSTANCE()

		ASSET_TEXTURE_PROP(LaserDrone, laserBeam, "Laser", "laserBeam")
        MARK_DISABLED_FOR_INSTANCE()

        FLOAT_PROP(LaserDrone, distanceToInstantKill, 18.f, "Drone", "distanceToInstantKill", "Distance to instant kill the player")
		MARK_DISABLED_FOR_INSTANCE()

        FLOAT_PROP(LaserDrone, moveSpeed, 100.f, "Movement", "moveSpeed", "Move speed")
        MARK_DISABLED_FOR_INSTANCE()

        FLOAT_PROP(LaserDrone, stopAtPathPointsTime, 0.f, "Movement", "stopAtPathPointsTime", "Stop and wait and path points (seconds)")
        MARK_DISABLED_FOR_INSTANCE()

        BOOL_PROP(LaserDrone, shouldStopOnlyAtEndPoints, false, "Movement", "shouldStopOnlyAtEndPoints", "Stop movement in case the end point has been reached")
        MARK_DISABLED_FOR_INSTANCE()

        BOOL_PROP(LaserDrone, loopPath, false, "Movement", "loopPath", "Move by path points in a loop")
        MARK_DISABLED_FOR_INSTANCE()

        FLOAT_PROP(LaserDrone, rotationSpeed, 100.f, "Rotation", "rotationSpeed", "Rotation speed")
        MARK_DISABLED_FOR_INSTANCE()

		DRONE_MOVEMENT_OVERRIDES(LaserDrone)
		DRONE_ROTATION_OVERRIDES(LaserDrone)
	META_DATA_DESC_END()

    const int DotSize         = 65;
	const int DotHalfSize     = DotSize / 2;
	const float GlowIntencity = 0.4f;

    void LaserDrone::Init()
    {
        DroneV2::Init();
    }

    void LaserDrone::Release()
    {
        DroneV2::Release();

		for (auto& ray : laserRays)
		{
			RELEASE(ray.noiseSound);
		}
    }

	void LaserDrone::MakeActive(bool active)
	{
		for (auto& ray : laserRays)
		{
			ray.isActive = active;
			ray.hitPos = {};
		}

		if (trigger)
		{
			trigger->SetVisiblity(active);
		}
	}

	void LaserDrone::ActionOnTrigger()
	{
		active = false;

		MakeActive(active);
	}

    void LaserDrone::Play()
    {
        DroneV2::Play();

		laserCenter = FindChild<Node2D>("laserCenter");
		laserHit    = FindChild<VFXEmitterInstantiable>("laserHit");

		ClearLaserHit();		

		for (auto &ray : laserRays)
		{
			ray.noiseSound = GetRoot()->GetSounds()->CreateSoundEvent("event:/Laser");
		}

		if (trigger = FindChild<PhysTriger2D>())
		{
			trigger->SetPhysGroup(PhysGroup::LevelObjects);
			trigger->AddDelegate<eastl::function<void(int, SceneEntity*, int)>>("OnContactStart", this,
				[this](int index, SceneEntity* entity, int contactIndex)
				{
					if (Player* player = dynamic_cast<Player*>(entity->GetParent()))
					{
						ActionOnTrigger();
					}
				});
		}
    }

    void LaserDrone::OnRestart(const EventOnRestart &evt)
    {
        DroneV2::OnRestart(evt);

		bodySprite->SetVisiblity(true);
		
		active = activeOnReset;

		MakeActive(active);		
    }

	void LaserDrone::OnCheckpointReached(const EventOnCheckpointReached& evt)
	{
		DroneV2::OnCheckpointReached(evt);

		activeOnReset = active;
	}

	void LaserDrone::UpdateLasers(float dt)
	{
		for (int rayId = 0, sz = (int)laserRays.size(); rayId < sz; ++rayId)
		{
			auto &ray = laserRays[rayId];

			if (!ray.isActive)
			{
				if (laserHit)
				{
					laserHit->StopInstance(rayId);
				}
				if (ray.noiseSound)
				{
					ray.noiseSound->Stop();
				}
				continue;
			}

			const Utils::Angle angle      = Utils::Angle::Degrees(transform.rotation.z) + Utils::Angle::Degrees(ray.directionAngle);
			const Math::Vector3 traceDir  = angle.ToDirection();
			const Math::Vector3 traceFrom = Sprite::ToPixels(GetTransform().GetGlobal().Pos()) + traceDir * 8.0f;
			const float rayLength         = ray.maxLength > 0.f ? ray.maxLength : 1500.f;

			Math::Vector3 rayHitNormal = {0.f, 1.f, 0.f};

			ray.hitPos = traceFrom + rayLength * traceDir;
			if (auto castRes = Utils::SphereCast(traceFrom, traceDir, rayLength, 6.f, PhysGroup::WorldDoorsPlayer | PhysGroup::Enemy_))
			{
				ray.hitPos = castRes->hitPos + castRes->hitNormal * 6.f;
				rayHitNormal = castRes->hitNormal;

				if (auto castRes1 = Utils::RayCast(ray.hitPos, traceDir, TILE_SIZE, PhysGroup::WorldDoors))
				{
					ray.hitPos   = castRes1->hitPos + 1.f * castRes1->hitNormal;
					rayHitNormal = castRes1->hitNormal;

					if (laserHit)
					{
						laserHit->StartInstance(rayId);
					}
				}

				if (castRes->userdata)
				{
					if (dynamic_cast<Player*>(castRes->userdata->object->GetParent()))
					{
						WorldManager::instance->KillPlayer(traceDir, DeathSource::Laser);
					}
					else
					if (auto* enemy = dynamic_cast<Enemy*>(castRes->userdata->object->GetParent()))
					{
						enemy->Kill(traceDir, DeathSource::Laser);
					}
				}
			}

			if (laserHit)
			{
				Math::Matrix m;
				m.RotateZ(-Utils::Angle::FromDirection(rayHitNormal).ToRadian());
				m.Pos() = Sprite::ToUnits(ray.hitPos);

				laserHit->UpdateInstance(rayId, dt, m);
			}

			if (ray.noiseSound)
			{
				if (!ray.noiseSound->IsPlaying())
				{
					ray.noiseSound->Play();
				}
				ray.noiseSound->Set3DAttributes(ray.hitPos);
			}
		}
	}

    void LaserDrone::DoUpdate(float dt)
    {
		if (killed)
		{
			return;
		}

        const auto curAngle = Utils::Angle::Degrees(transform.rotation.z);
        auto wishAngle      = curAngle;

        const PathRes pathRes = UpdatePath(dt);
        if (Utils::IsNonZero(pathRes.moveDir))
        {
            const Utils::Angle targetAngle = Utils::Angle::FromDirection(pathRes.moveDir);
            wishAngle = curAngle.MoveTo(targetAngle, Utils::ToRadian(rotationSpeed) * dt);
        }
        else
        {
            wishAngle = curAngle.Add(rotationSpeed * dt, Utils::Angle::TDegrees{});
        }

        UpdateLasers(dt);

        ApplyTransform(pathRes.wishPos, wishAngle.ToSignedDegrees());
    }

    void LaserDrone::EditorDraw(float dt)
    {
        if (GetScene()->IsPlaying() || !IsVisible())
		{
			return;
		}

        DroneV2::EditorDraw(dt);

		laserCenter = FindChild<Node2D>("laserCenter");

        for (const auto &ray : laserRays)
		{
			const Utils::Angle angle = Utils::Angle::Degrees(transform.rotation.z) + Utils::Angle::Degrees(ray.directionAngle);
			const Math::Vector3 pos  = Sprite::ToPixels((laserCenter ? laserCenter->GetTransform() : GetTransform()).GetGlobal().Pos());

			if (ray.maxLength > 0.f)
			{
				Sprite::DebugLine(pos, pos + ray.maxLength * angle.ToDirection(), COLOR_RED_A(0.55f));
			}
			else
			{
				Utils::DebugArrow(pos, pos + 128.f * angle.ToDirection(), COLOR_YELLOW_A(0.55f));
			}
		}
    }

	void LaserDrone::ClearLaserHit()
	{
		if (laserHit)
		{
			laserHit->ClearInstances();

			Math::Matrix m = transform.GetGlobal();
			for (auto& ray : laserRays)
			{
				const int id = laserHit->AddInstance(m);
				laserHit->StopInstance(id);
			}
		}

	}

	void LaserDrone::OnThunderHit(Math::Vector3 hitDir, ThunderCharge* source)
	{
		DroneV2::OnThunderHit(hitDir, source);

		ClearLaserHit();
	}

	void LaserDrone::Kill(Math::Vector3 killDir, DeathSource deathSource)
	{
		Enemy::Kill(killDir, deathSource);
		bodySprite->SetVisiblity(false);

		ClearLaserHit();
	}

	void LaserDrone::Draw(float dt)
	{
		if (!GetScene()->IsPlaying() || !IsVisible() || isStunned || killed)
		{
			return;
		}

		Transform trans;
		trans.objectType = ObjectType::Object2D;

		const Math::Vector3 pos = Sprite::ToPixels((laserCenter ? laserCenter->GetTransform() : GetTransform()).GetGlobal().Pos());
		for (const auto &ray : laserRays)
		{
			if (!ray.isActive)
			{
				continue;
			}

			if (Utils::IsNonZero(ray.hitPos))
			{
				const float rayLength = (ray.hitPos - pos).Length();
				trans.offset   = Math::Vector2{0.f, 0.5f};
				trans.rotation = Math::Vector3{0.f, 0.f, Utils::Angle::FromDirection(Utils::Vector::Normalize(ray.hitPos - pos)).ToSignedDegrees()};				
				trans.size = laserBeam.GetSize();
				trans.scale = Math::Vector2{ rayLength / trans.size.x, 1.f };
				trans.position = pos;

				laserBeam.prg = Sprite::quadPrg;
				laserBeam.Draw(&trans, COLOR_WHITE, AnimGraph2D::pause ? 0.0f : dt);
			}
		}
	}
}