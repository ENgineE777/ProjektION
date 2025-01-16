#include "ShootingDrone.h"

#include "Common/Utils.h"
#include "World/WorldManager.h"

namespace Orin::Overkill
{
    ENTITYREG(SceneEntity, ShootingDrone, "Overkill/Characters", "ShootingDrone")

	META_DATA_DESC(ShootingDrone)
		BASE_SCENE_ENTITY_PROP(ShootingDrone)

        FLOAT_PROP(ShootingDrone, distanceToInstantKill, 18.f, "Drone", "distanceToInstantKill", "Distance to instant kill the player")
		MARK_DISABLED_FOR_INSTANCE()

        FLOAT_PROP(ShootingDrone, maxTimeToPlayerAppear, 1.f, "Sensor", "maxTimeToPlayerAppear", "Duration of wondering state")
        MARK_DISABLED_FOR_INSTANCE()

		ARRAY_PROP(ShootingDrone, sensors, Sensor, "Sensor", "sensors")
        MARK_DISABLED_FOR_INSTANCE()

        FLOAT_PROP(ShootingDrone, moveSpeed, 100.f, "Movement", "moveSpeed", "Move speed")
        MARK_DISABLED_FOR_INSTANCE()

        FLOAT_PROP(ShootingDrone, stopAtPathPointsTime, 0.f, "Movement", "stopAtPathPointsTime", "Stop and wait and path points (seconds)")
        MARK_DISABLED_FOR_INSTANCE()

        BOOL_PROP(ShootingDrone, shouldStopOnlyAtEndPoints, false, "Movement", "shouldStopOnlyAtEndPoints", "Stop movement in case the end point has been reached")
        MARK_DISABLED_FOR_INSTANCE()

        BOOL_PROP(ShootingDrone, loopPath, false, "Movement", "loopPath", "Move by path points in a loop")
        MARK_DISABLED_FOR_INSTANCE()

        FLOAT_PROP(ShootingDrone, rotationSpeed, 100.f, "Rotation", "rotationSpeed", "Rotation speed")
        MARK_DISABLED_FOR_INSTANCE()        

        FLOAT_PROP(ShootingDrone, aimSpeed, 100.f, "Rotation", "aimSpeed", "Aim speed")
        MARK_DISABLED_FOR_INSTANCE()

        FLOAT_PROP(ShootingDrone, returnSpeed, 200.f, "Rotation", "returnSpeed", "Aim return speed")
        MARK_DISABLED_FOR_INSTANCE()

        DRONE_ALL_OVERRIDES(ShootingDrone)
	META_DATA_DESC_END()

    void ShootingDrone::Init()
    {
        DroneV2::Init();
    }

    void ShootingDrone::Release()
    {
        DroneV2::Release();
    }

    void ShootingDrone::Play()
    {
        DroneV2::Play();

        shootFlash  = FindChild<PointLight2D>("flash");
		shootMuzzle = FindChild<SpriteEntity>("muzzle");

		if (shootFlash)
		{
			shootFlash->SetVisiblity(false);
		}

		if (shootMuzzle)
		{
			shootMuzzle->SetVisiblity(false);
		}
    }

    void ShootingDrone::OnRestart(const EventOnRestart &evt)
    {
        DroneV2::OnRestart(evt);
    }

    void ShootingDrone::UpdateShooting(float dt)
    {
        if (shootsBeforeStop == 0 && !isPlayerVisible)
        {
            timeToPlayerAppear = 0.0f;
            state = State::Wondering;
        }

        if (shootsBeforeStop > 0)
        {
            shootTime -= dt;

            if (shootTime < 0.01f)
            {
                if (!isPlayerVisible)
                {
                    shootsBeforeStop--;
                }
                else
                {
                    shootsBeforeStop = shootsMaxBeforeStop;
                }

                shootTime = shootTimeDelay;
                shootEffectTime = shootEffectMaxTime;

                const Math::Vector3 shootPos = transform.position;
                const Math::Vector3 shootDir = WorldManager::instance->CalcShootingDirToPlayer(shootPos, curShootCount, lastPlayerSnapshot);
                curShootCount++;

                WorldManager::instance->AddBulletProjectile(shootPos, shootDir * WorldManager::instance->maxBulletSpeed, 1500.f, false);

                //PlaySound("SFX/shoot.wav");

                if (WorldManager::instance->IsPlayerKilled() && --shootsToDeadPlayerLeft <= 0)
                {
                    state = State::Wondering;
                }
            }
        }

		const float isShootEffectVisible = shootEffectTime > 0.f;
		shootEffectTime -= dt;

		if (shootFlash)
		{
			shootFlash->SetVisiblity(isShootEffectVisible);
		}

		if (shootMuzzle)
		{
			shootMuzzle->SetVisiblity(isShootEffectVisible);
		}
    }

    void ShootingDrone::PreUpdate(float dt)
    {
        if (state != State::Attacking || isStunned)
		{
			if (shootFlash)
			{
				shootFlash->SetVisiblity(false);
			}

			if (shootMuzzle)
			{
				shootMuzzle->SetVisiblity(false);
			}
		}
    }

    void ShootingDrone::OnPlayerVisiblityChanged(bool isVisible, int sensorId)
    {
        if (isVisible && sensorId != 0 && (state == State::Patrolling || state == State::PrepareToPatrolling))
        {
            state = State::AimingToPlayer;
        }
        else if (isVisible && sensorId == 0 && state != State::Wondering)
        {
            state = State::Wondering;
            timeToPlayerAppear = maxTimeToPlayerAppear;
        }
    }

    void ShootingDrone::OnPlayerAppeared()
    {
        state = State::Attacking;
        shootTime = shootTimeDelay;
        shootsBeforeStop = shootsMaxBeforeStop;
        curShootCount = 0;
    }

    void ShootingDrone::OnPlayerDisappeared()
    {
        state = State::PrepareToPatrolling;
    }

    void ShootingDrone::DoUpdate(float dt)
    {
        const auto curAngle = Utils::Angle::Degrees(transform.rotation.z);
        auto wishAngle      = curAngle;

        PathRes pathRes{{}, transform.position};

        UpdateSensors(dt);

        auto DoAimingToPlayer = [&](bool keepAiming) -> bool
        {
            if (keepAiming)
            {
                lastAngleToPlayer = WorldManager::instance->GetAngleToPlayer(transform.position);
                lastPlayerSnapshot = WorldManager::instance->GetPlayerSnapshot();
            }
            wishAngle = Utils::Angle::Degrees(transform.rotation.z).MoveTo(lastAngleToPlayer, Utils::ToRadian(aimSpeed) * dt);
            return wishAngle.IsEqual(lastAngleToPlayer);
        };

        if (state == State::PrepareToPatrolling)
        {
            if (!pathPoints.empty())
            {
                state = State::Patrolling;
            }
            else
            {
                const Utils::Angle targetAngle = Utils::Angle::Degrees(initRotation.z);
                wishAngle = curAngle.MoveTo(targetAngle, Utils::ToRadian(returnSpeed) * dt);

                if (curAngle.IsEqual(targetAngle))
                {
                    state = State::Patrolling;
                }
            }
        }
        else if (state == State::Patrolling)
        {
            pathRes = UpdatePath(dt);

            if (Utils::IsNonZero(pathRes.moveDir))
            {
                const Utils::Angle targetAngle = Utils::Angle::FromDirection(pathRes.moveDir);
                wishAngle = curAngle.MoveTo(targetAngle, Utils::ToRadian(rotationSpeed) * dt);
            }
            else
            {
                wishAngle = curAngle.Add(rotationSpeed * dt, Utils::Angle::TDegrees{});
            }
        }
        else if (state == State::AimingToPlayer)
        {
            if (DoAimingToPlayer(true))
            {
                state = playerVisibleInSensorId == 0 ? State::Wondering : State::PrepareToPatrolling;
            }
        }
        else if (state == State::Wondering)
        {
            DoAimingToPlayer(isPlayerVisible);
        }
        else if (state == State::Attacking)
        {
            DoAimingToPlayer(true);
            UpdateShooting(dt);
        }

        ApplyTransform(pathRes.wishPos, wishAngle.ToSignedDegrees());
    }
}