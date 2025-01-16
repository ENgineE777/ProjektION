#include "ChargeDrone.h"

#include "Common/Utils.h"
#include "World/WorldManager.h"
#include "Characters/Enemies/Soldier.h"

namespace Orin::Overkill
{
    ENTITYREG(SceneEntity, ChargeDrone, "Overkill/Characters", "ChargeDrone")

	META_DATA_DESC(ChargeDrone)
		BASE_SCENE_ENTITY_PROP(ChargeDrone)

        INT_PROP(ChargeDrone, killScore, 150, "ChargeDrone", "killScore", "Kill Score")
        MARK_DISABLED_FOR_INSTANCE()

        FLOAT_PROP(ChargeDrone, maxSpeed, 500.f, "Charge", "maxSpeed", "Max speed")
        MARK_DISABLED_FOR_INSTANCE()

		FLOAT_PROP(ChargeDrone, prepareAttackTime, 0.5f, "Charge", "prepareAttackTime", "Perare attack time")
        MARK_DISABLED_FOR_INSTANCE()

		FLOAT_PROP(ChargeDrone, maxAttackTime, 2.f, "Charge", "maxAttackTime", "Max attack time")
        MARK_DISABLED_FOR_INSTANCE()

		FLOAT_PROP(ChargeDrone, droneSphereRadius, 24.f, "Charge", "droneSphereRadius", "Drone radius")
        MARK_DISABLED_FOR_INSTANCE()

		VECTOR3_PROP(ChargeDrone, droneSpherePos, {}, "Charge", "droneSpherePos")
        MARK_DISABLED_FOR_INSTANCE()

        FLOAT_PROP(ChargeDrone, distanceToInstantKill, 18.f, "Drone", "distanceToInstantKill", "Distance to instant kill the player")
		MARK_DISABLED_FOR_INSTANCE()

        FLOAT_PROP(ChargeDrone, maxTimeToPlayerAppear, 1.f, "Sensor", "maxTimeToPlayerAppear", "Duration of wondering state")
        MARK_DISABLED_FOR_INSTANCE()

		ARRAY_PROP(ChargeDrone, sensors, Sensor, "Sensor", "sensors")
        MARK_DISABLED_FOR_INSTANCE()

        FLOAT_PROP(ChargeDrone, moveSpeed, 100.f, "Movement", "moveSpeed", "Move speed")
        MARK_DISABLED_FOR_INSTANCE()

        FLOAT_PROP(ChargeDrone, stopAtPathPointsTime, 0.f, "Movement", "stopAtPathPointsTime", "Stop and wait and path points (seconds)")
        MARK_DISABLED_FOR_INSTANCE()

        BOOL_PROP(ChargeDrone, shouldStopOnlyAtEndPoints, false, "Movement", "shouldStopOnlyAtEndPoints", "Stop movement in case the end point has been reached")
        MARK_DISABLED_FOR_INSTANCE()

        BOOL_PROP(ChargeDrone, loopPath, false, "Movement", "loopPath", "Move by path points in a loop")
        MARK_DISABLED_FOR_INSTANCE()

        FLOAT_PROP(ChargeDrone, rotationSpeed, 100.f, "Rotation", "rotationSpeed", "Rotation speed")
        MARK_DISABLED_FOR_INSTANCE()

        FLOAT_PROP(ChargeDrone, aimSpeed, 100.f, "Rotation", "aimSpeed", "Aim speed")
        MARK_DISABLED_FOR_INSTANCE()

        DRONE_ALL_OVERRIDES(ChargeDrone)
	META_DATA_DESC_END()

    void ChargeDrone::Init()
    {
        DroneV2::Init();
    }

    void ChargeDrone::Release()
    {
        DroneV2::Release();
    }

    void ChargeDrone::Play()
    {
        DroneV2::Play();

        shouldStopAttacking = false;
		attackTime = 0.f;
    }

    void ChargeDrone::OnRestart(const EventOnRestart &evt)
    {
        DroneV2::OnRestart(evt);

        shouldStopAttacking = false;
		attackTime = 0.f;
    }

    void ChargeDrone::OnPlayerVisiblityChanged(bool isVisible, int sensorId)
    {
        if (isVisible && state == State::Patrolling)
        {
            state = State::AimingToPlayer;
            Utils::PlaySoundEvent("event:/Drone/Drone trig", &transform.position);
        }
    }

    void ChargeDrone::OnPlayerAppeared()
    {
        state = State::AimingToPlayer;
        Utils::PlaySoundEvent("event:/Drone/Drone trig", &transform.position);
    }

    float ChargeDrone::GetPrepareAttackTime()
    {
        return prepareAttackTime * WorldManager::instance->GetDifficulty()->reactionMul;
    }

    float ChargeDrone::UpdateAgrometer(float dt)
    {
        if (state == State::PrepareAttack)
        {
            return Utils::Saturate(attackTime / GetPrepareAttackTime());
        }
        else if (state == State::Cooldown)
        {
            return 1.f - Utils::Saturate(attackTime / GetPrepareAttackTime());
        }
        else if (state == State::Attacking)
        {
            return 1.f;
        }
        
        return 0.f;
    }

    void ChargeDrone::Agro()
    {
        if (state == State::PrepareAttack || state == State::Attacking)
        {
            return;
        }

        state = State::PrepareAttack;
        attackTime = 0.f;
        moveDir = Utils::Angle::Degrees(transform.rotation.z).ToDirection();
        moveStart = transform.position;        
    }

    void ChargeDrone::DoUpdate(float dt)
    {
        const auto traceOffset = transform.GetGlobal().MulNormal(droneSpherePos);
        const auto curAngle = Utils::Angle::Degrees(transform.rotation.z);
        auto wishAngle      = curAngle;

        PathRes pathRes{{}, transform.position};

        UpdateSensors(dt);

        auto DoAimingToPlayer = [&](bool keepAiming) -> bool
        {
            if (keepAiming)
            {
                lastAngleToPlayer = WorldManager::instance->GetAngleToPlayer(transform.position);
            }
            wishAngle = Utils::Angle::Degrees(transform.rotation.z).MoveTo(lastAngleToPlayer, Utils::ToRadian(aimSpeed) * dt);
            return wishAngle.IsEqual(lastAngleToPlayer);
        };

        if (state == State::Patrolling)
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
                if (playerVisibleInSensorId == 0)
                {
                    state = State::PrepareAttack;
                    attackTime = 0.f;
                    moveDir    = Utils::Angle::Degrees(transform.rotation.z).ToDirection();
                    moveStart  = transform.position;                    
                }
                else
                {
                    state = State::Patrolling;
                }
            }
        }
        else if (state == State::Wondering)
        {
            state = State::AimingToPlayer;
            Utils::PlaySoundEvent("event:/Drone/Drone trig", &transform.position);
        }
        else if (state == State::PrepareAttack)
        {
			attackTime += dt;

			const float t = Utils::Saturate(attackTime / GetPrepareAttackTime());
			const Math::Vector3 moveEnd = moveStart - moveDir * TILE_HALF_SIZE;

			transform.position = Utils::Lerp(moveStart, moveEnd, Math::EaseOutQuart(t));
			controller->SetPosition(transform.position);

			if (auto castRes = Utils::SphereCastLine(moveStart + traceOffset, transform.position + traceOffset, droneSphereRadius, PhysGroup::WorldDoors))
			{
				const Math::Vector3 dir = Utils::Vector::Normalize(transform.position - moveStart);
				transform.position = moveStart + dir * castRes->hitLength + castRes->hitNormal * 2.f;
				controller->SetPosition(transform.position);

				state = State::Attacking;
                Utils::PlaySoundEvent("event:/Drone/Drone fly", &transform.position);
				attackTime = 0.f;
			}
            else
			if (t >= 1.f)
			{
				state = State::Attacking;                
                Utils::PlaySoundEvent("event:/Drone/Drone fly", &transform.position);
				attackTime = 0.f;
			}
        }
        else if (state == State::Attacking)
        {
			attackTime += dt;

			const float t = Utils::Saturate(attackTime / GetPrepareAttackTime());
			speed = Utils::Lerp(0.f, maxSpeed, Math::EaseOutQuart(t));

			float traceLen = speed * dt;

            eastl::vector<PhysScene::BodyUserData*> hitBodies;
            GetRoot()->GetPhysScene()->OverlapWithSphere(Sprite::ToUnits(transform.position), Sprite::ToUnits(distanceToInstantKill), PhysGroup::Enemy_, hitBodies);

            for (auto* body : hitBodies)
            {
                auto soldier = dynamic_cast<Soldier*>(body->object->GetParent());

                if (soldier)
                {
                    soldier->Hit(moveDir, 1);
                }
            }            

			if (auto castRes = Utils::SphereCast(transform.position, moveDir, traceLen, droneSphereRadius, PhysGroup::WorldDoors))
			{
                Utils::PlaySoundEvent("event:/Drone/Drone bump", &transform.position);

                const Math::Vector3 spherePos = transform.position + traceOffset + moveDir * castRes->hitLength + castRes->hitNormal * 2.f;
                transform.position = spherePos;
                controller->SetPosition(transform.position);
                moveDir = Utils::Vector::Reflect(moveDir, castRes->hitNormal);

                rotateToPlayer = true;
			}
			else
			{
				transform.position += speed * moveDir * dt;
				controller->SetPosition(transform.position);
			}
		
            Utils::Angle targetAngle = Utils::Angle::FromDirection(moveDir);

            if (rotateToPlayer)
            {
                targetAngle = WorldManager::instance->GetAngleToPlayer(transform.position);
            }

            const Utils::Angle curAngle    = Utils::Angle::Degrees(transform.rotation.z);
            transform.rotation = Utils::Vector::xyV(transform.rotation, curAngle.MoveTo(targetAngle, Utils::ToRadian(300.f) * dt).ToSignedDegrees());

			if (attackTime > maxAttackTime)
			{
				state = State::Cooldown;
                rotateToPlayer = false;
				attackTime = 0.f;
			}
        }
        else if (state == State::Cooldown)
        {
            attackTime += dt;

			const float t = Utils::Saturate(attackTime / GetPrepareAttackTime());
			speed = Utils::Lerp(maxSpeed, 0.f, Math::EaseOutQuart(t));

			bool shouldMove = true;		
            if (auto castRes = Utils::SphereCast(transform.position + traceOffset, moveDir, speed * dt, droneSphereRadius, PhysGroup::WorldDoors))
            {
                shouldMove = false;
                const Math::Vector3 spherePos = transform.position + moveDir * castRes->hitLength + castRes->hitNormal * 2.f;
                transform.position = spherePos;
                controller->SetPosition(transform.position);
                moveDir = Utils::Vector::Reflect(moveDir, castRes->hitNormal);
            }

			if (shouldMove)
			{
				transform.position += speed * moveDir * dt;
				controller->SetPosition(transform.position);
			}

			if (speed <= 0.f)
			{
				state = isPlayerVisible ? State::Wondering : State::Patrolling;
			}
        }

        if (state != State::PrepareAttack &&
            state != State::Attacking &&
            state != State::Cooldown)
        {
            ApplyTransform(pathRes.wishPos, wishAngle.ToSignedDegrees());
        }
    }
}