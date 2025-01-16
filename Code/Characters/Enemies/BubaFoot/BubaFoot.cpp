
#include "BubaFoot.h"
#include "BubaFootActivator.h"
#include "BubaFootShoot.h"
#include "Root/Root.h"
#include "World/WorldManager.h"
#include "World/BloodManager.h"
#include "Weapons/BubaFootMissle.h"
#include "Weapons/BubaFootFlamethrower.h"

namespace Orin::Overkill
{
	ENTITYREG(SceneEntity, BubaFoot, "Overkill/Characters", "BubaFoot")

	META_DATA_DESC(BubaFoot)
		BASE_SCENE_ENTITY_PROP(BubaFoot)

		SCENEOBJECT_PROP(BubaFoot, data, "Property", "data")

		FLOAT_PROP(BubaFoot, moveSpeed, 300.0f, "Property", "moveSpeed", "Move Speed")
		MARK_DISABLED_FOR_INSTANCE()

		FLOAT_PROP(BubaFoot, moveToZoneAccel, 1000.0f, "Property", "moveToZoneAccel", "Move to zone acceleration")
		MARK_DISABLED_FOR_INSTANCE()

		VECTOR2_PROP(BubaFoot, redZone, Math::Vector2(3.f * TILE_SIZE, 4.f * TILE_SIZE), "Property", "redZone")
		MARK_DISABLED_FOR_INSTANCE()

		VECTOR2_PROP(BubaFoot, greenZone, Math::Vector2(8.f * TILE_SIZE, 5.f * TILE_SIZE), "Property", "redZone")
		MARK_DISABLED_FOR_INSTANCE()

		BOOL_PROP(BubaFoot, debugDrawZones, false, "Property", "debugDrawZones", "Debug: DrawZones")
	META_DATA_DESC_END()

	void BubaFoot::Init()
	{
		Enemy::Init();

		Tasks(false)->AddTask(0, this, (Object::Delegate)&BubaFoot::EditorDraw);
	}

	void BubaFoot::Release()
	{
		Tasks(false)->DelTask(10, this);

		RELEASE(jetpackSound)

		Enemy::Release();
	}

	void BubaFoot::Play()
	{
		Enemy::Play();

		misslePrefab = GetRoot()->GetAssets()->GetAssetRef<AssetPrefabRef>(eastl::string("Prefabs/Characters/BubaFoot/Weapons/BubaFootMissle.prefab"));

		flamethrower = FindChild<BubaFootFlamethrower>("flamethrower");
		if (flamethrower)
		{
			flamethrower->Stop();
		}

		arm = FindChild<SpriteEntity>("arm");
		aimPoint = FindChild<Node2D>("aim");
		shootNode = FindChild<Node2D>("shoot");

		curMoveSpeed = moveSpeed;

		if (data)
		{
			path = data->FindChild<GenericMarker2D>();

			auto* activator = data->FindChild<BubaFootActivator>();

			if (activator)
			{
				activator->SetBubaFoot(this);
			}

			eastl::vector<BubaFootShoot*> shoots;
			data->FindChilds<BubaFootShoot>(shoots);

			for (auto* shoot : shoots)
			{
				shoot->SetBubaFoot(this);
			}
		}

		jetpackSound = GetRoot()->GetSounds()->CreateSoundEvent("event:/BobaFet/Jetpack");
	}

	void BubaFoot::SetNextPoint()
	{
		curPointSegment++;

		if (curPointSegment < path->instances.size())
		{
			auto dir = path->instances[curPointSegment].GetPosition() - transform.position;
			dirSegment = Math::Vector2(dir.x, dir.y);
			lenSegment = dirSegment.Normalize();
		}
		else
		{
			state = State::WaitingAttack;
			WorldManager::instance->ChangeAlarmCount(false);
		}
	}

	void BubaFoot::Activate()
	{
		if (state == State::Inactive)
		{
			state = State::Following;
			SetNextPoint();

			WorldManager::instance->ChangeAlarmCount(true);

			if (jetpackSound)
			{
				jetpackSound->Play();
			}
		}
	}

	void BubaFoot::LaunchMissle(Math::Vector3 dest)
	{
		if (!misslePrefab)
		{
			return;
		}

		const auto shootPos = Sprite::ToPixels(shootNode ? shootNode->GetTransform().GetGlobal().Pos() : transform.GetGlobal().Pos());
		const auto dir = Utils::Vector::Normalize(dest - shootPos);

		BubaFootMissle *missle = dynamic_cast<BubaFootMissle*>(misslePrefab->CreateInstance(GetScene()));
		missle->owner = this;
		missle->Play();
		missle->SetVisiblity(true);
		missle->targetPos = dest;
		missle->controller->SetPosition(shootPos);

		auto &tm = missle->GetTransform();

		tm.rotation = Utils::Vector::xyV(tm.rotation, rndGen.Range({-20.f, 20.f}) + ((dest.x - shootPos.x > 0.f) ? 0.f : 180.f));

		GetScene()->AddEntity(missle);

		Utils::PlaySoundEvent("event:/BobaFet/RocketLauncher", &transform.position);
	}

	void BubaFoot::Kill(Math::Vector3 killDir, DeathSource deathSource)
	{
		if (killed || state == State::Inactive)
		{
			return;
		}

		Enemy::Kill(killDir, deathSource);

		anim->anim.GotoNode("Killed", true);

		const float killAngle = Utils::Angle::FromDirection(killDir).ToDegrees();
		WorldManager::instance->GetBlood()->Spawn("Spray", nullptr, transform.position + Math::Vector3{0.f, 36.f, 0.f}, killAngle);
	}
	
	void BubaFoot::OnRestart(const EventOnRestart &evt)
	{
		Enemy::OnRestart(evt);
				
		state = State::Inactive;

		waveK = 0.0f;

		curPointSegment = -1;
		lenSegment = 0.0f;
		dirSegment = 0.0f;
		curMoveSpeed = moveSpeed;
		timeToStopAfterKill = 1.f;

		if (flamethrower)
		{
			flamethrower->ResetAndStop();
		}
	}

	void BubaFoot::EditorDraw(float dt)
	{
		if (GetScene()->IsPlaying() || !IsVisible())
		{
			return;
		}

		if (debugDrawZones)
		{
			Math::Matrix m;
			m.Pos() = transform.position;
			Sprite::DebugBox(m, 2.f * redZone, COLOR_RED);
			Sprite::DebugBox(m, 2.f * greenZone, COLOR_GREEN);
		}
	}
	
	void BubaFoot::Update(float dt)
	{
		if (!anim || !controller)
		{
			return;
		}

		if (killed)
		{
			return;
		}

		if (WorldManager::instance->GetPlayer() && WorldManager::instance->GetFrontendState() != FrontendState::Gameplay)
		{
			return;
		}

		waveK += dt * 4.0f;
		anim->GetTransform().position = Math::Vector3(0.0f, cosf(waveK) * 5.0f, 0.0f);

		Math::Vector2 moveDir = 0.0f;		

		int worldGroup = PhysGroup::WorldDoors;

		anim->SetVisiblity(state != State::Inactive);
		arm->SetVisiblity(state != State::Inactive);

		if (state != State::Inactive)
		{
			if (debugDrawZones)
			{
				Math::Matrix m;
				m.Pos() = transform.position;
				Sprite::DebugBox(m, 2.f * redZone, COLOR_RED);
				Sprite::DebugBox(m, 2.f * greenZone, COLOR_GREEN);
			}

			const float distToPlayerX = std::abs(WorldManager::instance->GetPlayerPos().x - transform.position.x);
			const float distToPlayerY = std::abs(WorldManager::instance->GetPlayerPos().y - transform.position.y);
			const float distToPlayer  = std::max(distToPlayerX, distToPlayerY);

			if (distToPlayerX >= greenZone.x || distToPlayerY >= greenZone.y)
			{
				worldGroup = PhysGroup::World;
				curMoveSpeed += moveToZoneAccel * dt;
			}
			else
			{
				curMoveSpeed = moveSpeed;
			}

			if (flamethrower && !doNotAttackPlayer)
			{
				bool keepShooting = true;
				if (WorldManager::instance->IsPlayerKilled())
				{
					timeToStopAfterKill -= dt;
					keepShooting = timeToStopAfterKill > 0.f;
				}

				if ((distToPlayerX <= redZone.x && distToPlayerY <= redZone.y) && keepShooting)
				{
					timeToNextFlamethrowerSFX -= dt;

					if (timeToNextFlamethrowerSFX < 0.0f)
					{
						timeToNextFlamethrowerSFX = 0.75f;
						Utils::PlaySoundEvent("event:/BobaFet/flamethrower", &transform.position);
					}

					flamethrower->Start();
					deathRayLength = Utils::Approach(deathRayLength, 2.f * redZone.Length(), dt, 0.5f);

					const auto dirToPlayer = WorldManager::instance->GetAngleToPlayer(transform.position).ToDirection();

					if (auto castRes = Utils::SphereCast(transform.position, dirToPlayer, deathRayLength, 4.f, PhysGroup::WorldDoorsPlayer))
					{
						if (castRes->userdata && dynamic_cast<Player*>(castRes->userdata->object->GetParent()) && !WorldManager::instance->IsPlayerKilled()
							&& !WorldManager::instance->HasPuppet())
						{
							WorldManager::instance->KillPlayer(dirToPlayer, DeathSource::Unknown);
						}
					}
				}
				else
				{
					flamethrower->Stop();
					deathRayLength = Utils::Approach(deathRayLength, 0.f, dt, 0.2f);
				}
			}

			if (jetpackSound)
			{
				jetpackSound->Set3DAttributes(transform.position);
			}
		}

		if (state == State::Inactive)
		{
			
		}
		else
		if (state == State::Following)
		{
			moveDir = dirSegment * curMoveSpeed;
			lenSegment -= curMoveSpeed * dt;

			if (arm)
			{
				auto pos = Sprite::ToPixels(aimPoint->GetTransform().GetGlobal().Pos());

				const Utils::Angle angleToPlayer = WorldManager::instance->GetAngleToPlayer(pos);

				auto& trans = arm->GetTransform();
				Math::Vector3 rotation = trans.rotation;

				float angle = Math::Clamp(angleToPlayer.ToSignedDegrees() + 27.0f, -60.0f, 60.0f);

				trans.rotation = Utils::Vector::xyV(rotation, angleToPlayer.ToSignedDegrees() + 27.0f);
			}
		}
		else	
		if (state == State::WaitingAttack)
		{
			
		}		

		UpdateAnimScale();		

		if (moveDir.Length() > 0.0f)
		{
			controller->Move(moveDir, 3);
		}

		if (state == State::Following && lenSegment < 0.0f)
		{
			SetNextPoint();
		}
	}
}