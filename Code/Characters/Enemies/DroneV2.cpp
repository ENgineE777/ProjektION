
#include "DroneV2.h"
#include "Root/Root.h"
#include "World/WorldManager.h"
#include "Common/Utils.h"
#include "Objects/Throwable/ThunderCharge.h"
#include "SceneEntities/2D/GenericMarker2D.h"

namespace Orin::Overkill
{
	META_DATA_DESC(DroneV2::Sensor)
		FLOAT_PROP(DroneV2::Sensor, coneAngle, 45.f, "Sensor", "coneAngle", "Cone Angle")
		FLOAT_PROP(DroneV2::Sensor, directionAngle, 0.f, "Sensor", "directionAngle", "Direction Angle")
		FLOAT_PROP(DroneV2::Sensor, distance, 256.f, "Sensor", "distance", "Distance")
		FLOAT_PROP(DroneV2::Sensor, alwaysVisibleDistance, 0.f, "Sensor", "alwaysVisibleDistance", "Always visible at distance")
	META_DATA_DESC_END()

	void DroneV2::OnRestart(const EventOnRestart &evt)
	{
		Enemy::OnRestart(evt);

		if (bodyElectro)
		{
			bodyElectro->anim.Reset();
		}

		state = State::Patrolling;

		totalTime = 0.f;
		flipped = initScale.x < 0.f;

		isStunned = false;
		stunTimer = -1.f;

		isPlayerVisible = false;

		selectNextPointTimer = -1.f;

		prevIndex = 0;
		nextIndex = 1;
		pathStep = 1;

		isAlarmStarted = false;

		if (bodySprite)
		{
			bodySprite->SetVisiblity(true);
		}

		ApplyTransform(initPos, initRotation.z);
	}

	void DroneV2::OnThunderHit(Math::Vector3 hitDir, ThunderCharge *source)
	{
		Utils::AddCameraShake(0.35f);

		isStunned = true;
		stunTimer = 5.f;
		stunSource = source;
	}

	void DroneV2::Init()
	{
		Enemy::Init();

		Tasks(false)->AddTask(0, this, (Object::Delegate)&DroneV2::EditorDraw);
		Tasks(true)->AddTask(10, this, (Object::Delegate)&DroneV2::Draw);
	}

	void DroneV2::Release()
	{
		Tasks(false)->DelTask(10, this);
		Tasks(false)->DelTask(0, this);

		Enemy::Release();
	}

	void DroneV2::Play()
	{
		Enemy::Play();

		moveSpeed = overrideMoveSpeed.enabled ? overrideMoveSpeed.value : moveSpeed;
		rotationSpeed = overrideRotationSpeed.enabled ? overrideRotationSpeed.value : rotationSpeed;
		aimSpeed = overrideAimSpeed.enabled ? overrideAimSpeed.value : aimSpeed;
		returnSpeed = overrideReturnSpeed.enabled ? overrideReturnSpeed.value : returnSpeed;
		maxTimeToPlayerAppear = overrideTimeToPlayerAppear.enabled ? overrideTimeToPlayerAppear.value : maxTimeToPlayerAppear;
		stopAtPathPointsTime = overrideStopAtPathPointsTime.enabled ? overrideStopAtPathPointsTime.value : stopAtPathPointsTime;
		shouldStopOnlyAtEndPoints = overrideShouldStopOnlyAtEndPoints.enabled ? overrideShouldStopOnlyAtEndPoints.value != 0.f : shouldStopOnlyAtEndPoints;

		flipped = initScale.x < 0.f;

		bodySprite = FindChild<SpriteEntity>("body");

		if (bodySprite)
		{
			bodyElectro = bodySprite->FindChild<AnimGraph2D>("electro");
		}

		phys = FindChild<PhysEntity2D>("phys");
		trigger = FindChild<PhysTriger2D>("trigger");
		sensorCenter = FindChild<Node2D>("sensorCenter");

		if (GenericMarker2D *path = FindChild<GenericMarker2D>())
		{
			if (path->instances.size() >= 2)
			{
				pathPoints.reserve(path->instances.size());
				for (auto &inst : path->instances)
				{
					pathPoints.push_back(inst.GetPosition());
				}

				transform.position = pathPoints[0];

				prevIndex = 0;
				nextIndex = 1;

				initPos      = transform.position;
				initRotation = transform.rotation;
			}
		}

		ApplyTransform(initPos, initRotation.z);
	}

	void DroneV2::EditorDraw(float dt)
	{
		if (GetScene()->IsPlaying() || !IsVisible())
		{
			return;
		}

		if (auto *path = FindChild<GenericMarker2D>())
		{
			if (!path->instances.empty())
			{
				path->instances[0].color = COLOR_GREEN_A(0.55f);
			}
		}

		sensorCenter = FindChild<Node2D>("sensorCenter");

		const float direction = Math::Sign(transform.scale.x);
		const auto parentAngle = Utils::Angle::Degrees(transform.rotation.z);
		const Math::Vector3 pos = Sprite::ToPixels((sensorCenter ? sensorCenter->GetTransform() : GetTransform()).GetGlobal().Pos());
		for (const auto &s : sensors)
		{
			const Utils::Angle angle = Utils::AdjustAngleByDirection(Utils::Angle::Degrees(s.directionAngle) + parentAngle, direction);
			const Utils::Angle cone = Utils::Angle::Degrees(s.coneAngle);
			Utils::DebugTrinagle(pos, angle - cone * 0.5f, angle + cone * 0.5f, s.distance, COLOR_YELLOW);
		}

		if (distanceToInstantKill > 0.f)
		{
			Utils::DebugSector(transform.position, {}, Utils::Angle::Degrees(180.f), COLOR_RED_A(0.35f), 10.f, distanceToInstantKill);
			Utils::DebugSector(transform.position, Utils::Angle::Degrees(180.f), {}, COLOR_RED_A(0.35f), 10.f, distanceToInstantKill);
		}
	}

	void DroneV2::ApplyTransform(Math::Vector3 wishPosition, float withAngle)
	{
		if (controller)
		{
			const float dt = GetRoot()->GetDeltaTime();
			controller->Move(Utils::Vector::xy(wishPosition - transform.position) * Utils::SafeInvert(dt), PhysGroup::WorldDoors);
		}
		else if (phys)
		{
			transform.position = wishPosition;
			// Will be applied on the next physics step!
			phys->body.body->SetTransform(transform.GetGlobal());
		}
		else if (trigger)
		{
			transform.position = wishPosition;
			trigger->body.body->SetTransform(transform.GetGlobal());
		}
		else
		{
			transform.position = wishPosition;
		}

		transform.rotation = Utils::Vector::xyV(transform.rotation, withAngle);
	}

	void DroneV2::ApplyTransform(Math::Matrix wishTm)
	{
		if (controller)
		{
			// TODO:
		}
		else if (phys)
		{
			transform.SetGlobal(wishTm);
			phys->body.body->SetTransform(wishTm);
		}
		else if (trigger)
		{
			transform.SetGlobal(wishTm);
			trigger->body.body->SetTransform(wishTm);
		}
		else
		{
			transform.SetGlobal(wishTm);
		}
	}

	void DroneV2::SelectNextPathPoint()
	{
		selectNextPointTimer = -1.f;

		if (loopPath)
		{
			prevIndex = nextIndex;
			nextIndex = (nextIndex + 1) % pathPoints.size();

			OnSelectNextPathPoint();
		}
		else
		{
			prevIndex = nextIndex;
			nextIndex = nextIndex + pathStep;
			if (nextIndex >= (int)pathPoints.size())
			{
				nextIndex = (int)pathPoints.size() - 2;
				pathStep  = -1;

				OnSwitchPathDirection();
			}
			else if (nextIndex < 0)
			{
				nextIndex = 1;
				pathStep  = 1;

				OnSwitchPathDirection();
			}

			OnSelectNextPathPoint();
		}
	}

	DroneV2::PathRes DroneV2::UpdatePath(float dt)
    {
        if (pathPoints.empty())
        {
            return {{}, transform.position};
        }

        if (selectNextPointTimer >= 0.f)
        {
            selectNextPointTimer -= dt;
            if (selectNextPointTimer < 0.f)
            {
                SelectNextPathPoint();
            }
        }

        const auto curPosition = transform.position;
        auto wishPosition = curPosition;

        const Math::Vector3 prevPoint = pathPoints[prevIndex];
        const Math::Vector3 nextPoint = pathPoints[nextIndex];
        const float distToNextPoint = (nextPoint - curPosition).Length();

        auto moveDir = (nextPoint - curPosition) * Utils::SafeInvert(distToNextPoint);

        const auto nextPosition = wishPosition + moveDir * moveSpeed * dt;

        const auto pathDir = Utils::Vector::Normalize(nextPoint - prevPoint);
        const float pathProj = (nextPosition - prevPoint).Dot(pathDir);
        const float pathT = pathProj / (nextPoint - prevPoint).Length();

        if (pathT < 1.f)
        {
            wishPosition = nextPosition;
        }
        else
        {
            wishPosition = nextPoint;
			moveDir = pathDir;

            if (selectNextPointTimer < 0.f)
            {
                float stopTime = stopAtPathPointsTime;
                if (shouldStopOnlyAtEndPoints)
                {
                    stopTime = (nextIndex == 0 || nextIndex == (int)pathPoints.size() - 1) ? stopAtPathPointsTime : 0.f;
                }

                selectNextPointTimer = stopTime;
            }
        }

        return {moveDir, wishPosition};
    }

	float DroneV2::UpdateAgrometer(float dt)
	{
		if (state == State::Wondering)
		{
			if (maxTimeToPlayerAppear > 0.01f)
			{
				return (1.0f - timeToPlayerAppear) / maxTimeToPlayerAppear;
			}
			else
			{
				return 1.0f;
			}
		}
		else if (state == State::Attacking)
		{
			return 1.0f;
		}

		return 0.f;
	}

	void DroneV2::UpdateSensors(float dt)
	{
		if (sensors.empty())
		{
			return;
		}

		const Math::Vector3 pos = Sprite::ToPixels((sensorCenter ? sensorCenter->GetTransform() : GetTransform()).GetGlobal().Pos());
		const float distaneToPlayer = WorldManager::instance->GetDistanceToPlayer(pos);

		const bool wasVisible = isPlayerVisible;

		isPlayerVisible = false;
		playerVisibleInSensorId = -1;

		const float direction = GetDirection() * Math::Sign(initScale.x);

		for (int i = 0, sz = (int)sensors.size(); i < sz; ++i)
		{
			const auto &s = sensors[i];

			const Utils::Angle angle = Utils::AdjustAngleByDirection(Utils::Angle::Degrees(s.directionAngle) + Utils::Angle::Degrees(transform.rotation.z), direction);
			if (distaneToPlayer <= s.distance - 24.f * TILE_SCALE)
			{
				isPlayerVisible |= WorldManager::instance->IsPlayerVisibleInSector(pos, s.distance, angle.ToSignedDegrees(), Utils::Angle::Degrees(s.coneAngle).ToSignedDegrees(), s.alwaysVisibleDistance);
				if (isPlayerVisible)
				{
					playerVisibleInSensorId = i;
					break;
				}
			}
		}

		if (isPlayerVisible != wasVisible)
		{
			OnPlayerVisiblityChanged(isPlayerVisible, playerVisibleInSensorId);
		}

		if (state == State::Wondering)
		{
			if (isPlayerVisible)
			{
				timeToPlayerAppear -= dt;

				if (timeToPlayerAppear < 0.0f)
				{
					state = State::Attacking;
					OnPlayerAppeared();
				}
			}
			else
			{
				timeToPlayerAppear += dt * 0.66f;

				if (timeToPlayerAppear > maxTimeToPlayerAppear)
				{
					state = State::Patrolling;
					OnPlayerDisappeared();
				}
			}
		}

		float agrometr = UpdateAgrometer(dt);

		for (const auto &s : sensors)
		{
			const Utils::Angle angle = Utils::AdjustAngleByDirection(Utils::Angle::Degrees(s.directionAngle) + Utils::Angle::Degrees(transform.rotation.z), direction);
			WorldManager::instance->AddViewCone(pos, angle, Utils::Angle::Degrees(s.coneAngle * 0.5f), s.distance, agrometr);
		}
	}
	
	void DroneV2::DoInstantKill()
	{
		WorldManager::instance->KillPlayer({}, DeathSource::Elecro);
		//PlaySound("SFX/electro_hit.wav");

		if (bodyElectro)
		{
			bodyElectro->anim.GotoNode("Anim", true);
		}
	}

	void DroneV2::UpdateInstantKill(float dt)
	{
		if (!WorldManager::instance || distanceToInstantKill < 0.f)
		{
			return;
		}

		if (!WorldManager::instance->IsPlayerKilled() &&
		     WorldManager::instance->GetDistanceToPlayer(transform.position) < distanceToInstantKill)
		{
			DoInstantKill();
		}
	}

	void DroneV2::StartAlarm()
	{
		if (!isAlarmStarted)
		{
			isAlarmStarted = true;
			WorldManager::instance->ChangeAlarmCount(true);
		}
	}

	void DroneV2::StopAlarm()
	{
		if (isAlarmStarted)
		{
			isAlarmStarted = false;
			WorldManager::instance->ChangeAlarmCount(false);
		}
	}

	void DroneV2::Kill(Math::Vector3 killDir, DeathSource deathSource)
	{
		Enemy::Kill(killDir, deathSource);

		if (bodySprite)
		{
			bodySprite->SetVisiblity(false);
		}
	}

	void DroneV2::Update(float dt)
	{
		if (!WorldManager::instance)
		{
			return;
		}

		if (WorldManager::instance->GetPlayer() && WorldManager::instance->GetFrontendState() != FrontendState::Gameplay)
		{
			return;
		}

		PreUpdate(dt);

		if (killed)
		{
			return;
		}

		totalTime += dt;
		if (totalTime <= 0.25f)
		{
			return;
		}

		if (stunTimer > 0.f)
		{
			stunTimer -= dt;

			if (stunTimer <= 0.f)
			{
				stunTimer = -1.f;
				isStunned = false;

				stunSource->OnThunderDisposed();
			}
		}

		if (!isStunned)
		{
			const State wasState = state;
			DoUpdate(dt);

			if (state != wasState)
			{
				if (wasState == State::PrepareToPatrolling || wasState == State::Patrolling)
				{
					StartAlarm();
				}
				else if (state == State::PrepareToPatrolling || state == State::Patrolling)
				{
					StopAlarm();
				}
			}
		}

		UpdateInstantKill(dt);

		UpdateAnimScale();
	}
}