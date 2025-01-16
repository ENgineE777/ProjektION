#include "SpiderDrone.h"

#include "Common/Utils.h"
#include "World/WorldManager.h"

namespace Orin::Overkill
{
    ENTITYREG(SceneEntity, SpiderDrone, "Overkill/Characters", "SpiderDrone")

    META_DATA_DESC(SpiderDrone)
        BASE_SCENE_ENTITY_PROP(SpiderDrone)

        ENUM_PROP(SpiderDrone, moveMode, MoveMode::Right, "Spider", "moveMode", "moveMode")
            ENUM_ELEM("Right", MoveMode::Right)
            ENUM_ELEM("Left", MoveMode::Left)
        ENUM_END

        FLOAT_PROP(SpiderDrone, attackSpeed, 500.f, "Spider", "attackSpeed", "Attack Speed")
        MARK_DISABLED_FOR_INSTANCE()

        FLOAT_PROP(SpiderDrone, maxAttackTime, 2.f, "Spider", "maxAttackTime", "Max attack time")
        MARK_DISABLED_FOR_INSTANCE()

        FLOAT_PROP(SpiderDrone, maxRadiusScaleTime, 0.15f, "Spider", "maxRadiusScaleTime", "Max radius scale time")
        MARK_DISABLED_FOR_INSTANCE()

        FLOAT_PROP(SpiderDrone, distanceToInstantKill, 18.f, "Drone", "distanceToInstantKill", "Distance to instant kill the player")
        MARK_DISABLED_FOR_INSTANCE()

        FLOAT_PROP(SpiderDrone, distanceToExplosionKill, 68.f, "Drone", "distanceToExplosionKill", "Distance to explosion kill the player")
        MARK_DISABLED_FOR_INSTANCE()

        FLOAT_PROP(SpiderDrone, maxTimeToPlayerAppear, 1.f, "Sensor", "maxTimeToPlayerAppear", "Duration of wondering state")
        MARK_DISABLED_FOR_INSTANCE()

        ARRAY_PROP(SpiderDrone, sensors, Sensor, "Sensor", "sensors")
        MARK_DISABLED_FOR_INSTANCE()

        FLOAT_PROP(SpiderDrone, moveSpeed, 100.f, "Movement", "moveSpeed", "Move speed")
        MARK_DISABLED_FOR_INSTANCE()

        FLOAT_PROP(SpiderDrone, stopAtPathPointsTime, 0.f, "Movement", "stopAtPathPointsTime", "Stop and wait and path points (seconds)")
        MARK_DISABLED_FOR_INSTANCE()

        BOOL_PROP(SpiderDrone, shouldStopOnlyAtEndPoints, false, "Movement", "shouldStopOnlyAtEndPoints", "Stop movement in case the end point has been reached")
        MARK_DISABLED_FOR_INSTANCE()

        BOOL_PROP(SpiderDrone, loopPath, false, "Movement", "loopPath", "Move by path points in a loop")
        MARK_DISABLED_FOR_INSTANCE()

        FLOAT_PROP(SpiderDrone, rotationSpeed, 100.f, "Rotation", "rotationSpeed", "Rotation speed")
        MARK_DISABLED_FOR_INSTANCE()

        FLOAT_PROP(SpiderDrone, aimSpeed, 100.f, "Rotation", "aimSpeed", "Aim speed")
        MARK_DISABLED_FOR_INSTANCE()

        DRONE_ALL_OVERRIDES(SpiderDrone)
    META_DATA_DESC_END()

    void SpiderDrone::Init()
    {
        DroneV2::Init();
    }

    void SpiderDrone::Release()
    {
        DroneV2::Release();
    }

    void SpiderDrone::Play()
    {
        DroneV2::Play();

        explosionRadius = FindChild<SpriteEntity>("radius");

        attackTime = 0.f;
        initialMoveSpeed = moveSpeed;
        initialStopAtPathPointsTime = stopAtPathPointsTime;

        auto moveDir = Math::Vector3{1.f, 0.f, 0.f} * (moveMode == MoveMode::Left ? -1.f : 1.f);
        moveDir = transform.GetGlobal().MulNormal(moveDir);

        flipped = Utils::IsFlipped(moveDir, transform.GetGlobal().Vy());

        lastPathRes = {{}, transform.position};
    }

    void SpiderDrone::OnRestart(const EventOnRestart &evt)
    {
        DroneV2::OnRestart(evt);

        attackTime = 0.f;
        moveSpeed = initialMoveSpeed;
        stopAtPathPointsTime = initialStopAtPathPointsTime;

        flipped = moveMode == MoveMode::Left;

        lastPathRes = {{}, transform.position};
        isDropAttack = false;

        if (explosionRadius)
		{
			explosionRadius->SetVisiblity(false);
		}
    }

    void SpiderDrone::OnPlayerVisiblityChanged(bool isVisible, int sensorId)
    {
        if (isVisible && state == State::Patrolling)
        {
            state = State::PrepareAttack;
            attackTime = maxAttackTime;
            if (explosionRadius)
            {
                explosionRadius->SetVisiblity(true);
            }
        }
    }

    void SpiderDrone::OnSwitchPathDirection()
    {
        flipped = !flipped;

        if (state == State::Attacking)
        {
            Explode(1.f);
        }
    }

    Utils::MaybeCastRes SpiderDrone::RayCastCorner(Math::Vector3 rayFrom, Math::Vector3 rayDir, float rayLength, Math::Vector3 cornerDir, uint32_t group)
    {
        const Math::Vector3 rayFrom1 = rayFrom + rayDir * rayLength;

        if (!Utils::RayHit(rayFrom, rayDir, rayLength, group) &&
            !Utils::RayHit(rayFrom1, cornerDir, TILE_HALF_SIZE * 1.25f, group))
        {
            const Math::Vector3 rayFrom2 = rayFrom1 + cornerDir * TILE_HALF_SIZE * 1.25f;
            return Utils::RayCast(rayFrom2, -rayDir, rayLength, group);
        }

        return eastl::nullopt;
    }

    Utils::MaybeCastRes SpiderDrone::QueryCorner(Math::Vector3 rayFrom, Math::Vector3 rayDir, float dist, Math::Vector3 surfaceNormal)
    {
        if (onCeiling)
        {
            return RayCastCorner(rayFrom, rayDir, dist, {0.f, 1.f, 0.f}, PhysGroup::WorldDoors);
        }
        else if (onWall)
        {
            auto rightCastRes = RayCastCorner(rayFrom, rayDir, dist, { 1.f, 0.f, 0.f}, PhysGroup::WorldDoors);
            auto leftCastRes  = RayCastCorner(rayFrom, rayDir, dist, {-1.f, 0.f, 0.f}, PhysGroup::WorldDoors);
            if (rightCastRes && leftCastRes)
            {
                if (CalcOuterDirAlongSurface(rayFrom, leftCastRes->hitPos, leftCastRes->hitNormal).Dot(surfaceNormal) < 0.f)
                {
                    return leftCastRes;
                }
                return rightCastRes;
            }
            else if (rightCastRes)
            {
                return rightCastRes;
            }
            else if (leftCastRes)
            {
                return leftCastRes;
            }
        }
        else if (onGround)
        {
            return RayCastCorner(rayFrom, rayDir, dist, {0.f, -1.f, 0.f}, PhysGroup::WorldDoors);
        }

        return eastl::nullopt;
    }

    Math::Vector3 SpiderDrone::CalcOuterDirAlongSurface(Math::Vector3 curPoint, Math::Vector3 hitPos, Math::Vector3 hitNormal)
    {
        const Math::Vector3 dir = curPoint - (hitPos + hitNormal * TILE_HALF_SIZE);
        return -Utils::Vector::Normalize(dir - dir.Dot(hitNormal) * hitNormal);
    };

    bool SpiderDrone::BuildAttackPath(float pathTime, float pathSpeed, eastl::vector<Math::Vector3> *resPath)
    {
        bool willPathKillThePlayer = false;

        Math::Vector3 curPoint = transform.position;
        Math::Vector3 curDir = lastPathRes.moveDir;
        Math::Vector3 lastPoint = curPoint;
        if (resPath)
        {
            resPath->clear();
            resPath->push_back(lastPoint);
        }

        Math::Vector3 surfaceNormal = Utils::Angle::Degrees(transform.rotation.z + 90.f).ToDirection();
        eastl::tie(onWall, onCeiling, onGround) = Utils::NormalToFlags(surfaceNormal);

        const float fixedDt = 0.01f;
        for (float dist = pathSpeed * pathTime; dist > 0.f;)
        {
            Math::Vector3 wishPoint = curPoint + curDir * pathSpeed * fixedDt;
            auto wishRes = Utils::RayCastLine(curPoint, wishPoint, PhysGroup::WorldDoors);
            if (wishRes && Utils::GetMovingSurfaceVel(wishRes))
            {
                wishRes = eastl::nullopt;
            }
            auto cornerRes = wishRes ? eastl::nullopt : QueryCorner(curPoint, curDir, 5.f, surfaceNormal);
            if (cornerRes && Utils::GetMovingSurfaceVel(cornerRes))
            {
                cornerRes = eastl::nullopt;
            }
            if (wishRes)
            {
                wishPoint = wishRes->hitPos + wishRes->hitNormal * TILE_HALF_SIZE;

                dist -= (wishPoint - lastPoint).Length();

                curPoint  = wishPoint;
                lastPoint = wishPoint;
                if (resPath)
                {
                    resPath->push_back(lastPoint);
                }

                const Math::Vector3 wishDir = Utils::Vector::Rotate90(wishRes->hitNormal);
                if (!Utils::RayHit(curPoint, wishDir, TILE_SIZE, PhysGroup::WorldDoors))
                {
                    curDir = wishDir;
                }
                else
                {
                    curDir = -wishDir;
                }

                surfaceNormal = wishRes->hitNormal;
                eastl::tie(onWall, onCeiling, onGround) = Utils::NormalToFlags(surfaceNormal);
            }
            else if (cornerRes)
            {
                if (resPath)
                {
                    resPath->push_back(wishPoint);
                }

                curDir = CalcOuterDirAlongSurface(curPoint, cornerRes->hitPos, cornerRes->hitNormal);
                wishPoint = cornerRes->hitPos + cornerRes->hitNormal * TILE_HALF_SIZE * 0.98f + curDir * controller->GetRadius();

                dist -= (wishPoint - lastPoint).Length();

                curPoint = wishPoint;
                lastPoint = wishPoint;
                if (resPath)
                {
                    resPath->push_back(lastPoint);
                }

                surfaceNormal = cornerRes->hitNormal;
                eastl::tie(onWall, onCeiling, onGround) = Utils::NormalToFlags(surfaceNormal);
            }
            else
            {
                dist -= pathSpeed * fixedDt;
                curPoint = wishPoint;
            }

            if (auto castRes = Utils::SphereCast(curPoint, {0.f, -1.f, 0.f}, TILE_SIZE * 5.f, controller->GetRadius(), PhysGroup::WorldDoorsPlayer))
            {
                if (castRes->userdata && dynamic_cast<Player*>(castRes->userdata->object->GetParent()) && !WorldManager::instance->IsPlayerKilled())
                {
                    willPathKillThePlayer = true;
                    if (!resPath)
                    {
                        break;
                    }
                }
            }

            if ((WorldManager::instance->GetPlayerPos() - wishPoint).Length() <= distanceToExplosionKill)
            {
                willPathKillThePlayer = true;
                if (!resPath)
                {
                    break;
                }
            }
        }

        if (Utils::IsNonZero(curPoint - lastPoint) && resPath)
        {
            resPath->push_back(curPoint);
        }

        return willPathKillThePlayer;
    }

    void SpiderDrone::DoInstantKill()
	{
		Explode(1.f);
	}

    bool SpiderDrone::TryToExplode(float radiusScale)
	{
		if (!WorldManager::instance->IsPlayerKilled() &&
             WorldManager::instance->GetDistanceToPlayer(transform.position) < distanceToExplosionKill * radiusScale)
		{
			Explode(radiusScale);
			return true;
		}

		return false;
	}

	void SpiderDrone::Explode(float radiusScale)
	{
		if (anim)
		{
			anim->anim.GotoNode("Explosion", true);
		}

        if (explosionRadius)
        {
            explosionRadius->SetVisiblity(false);
        }

		Kill({}, DeathSource::Unknown);

        Utils::PlaySoundEvent("event:/Bomb/expl", &transform.position);

		if (!WorldManager::instance->IsPlayerKilled() &&
             WorldManager::instance->GetDistanceToPlayer(transform.position) < distanceToExplosionKill * radiusScale)
		{
			if (!Utils::RayHitLine(transform.position, WorldManager::instance->GetPlayerPos(), PhysGroup::WorldDoors))
			{
				WorldManager::instance->KillPlayer({}, DeathSource::Unknown);
			}
		}
	}

    void SpiderDrone::DoUpdate(float dt)
    {
        const auto curAngle = Utils::Angle::Degrees(transform.rotation.z);
        auto wishAngle = curAngle;

        UpdateSensors(dt);

        auto AimByPathDir = [&]() -> bool
        {
            if (Utils::IsNonZero(lastPathRes.moveDir))
            {
                const Utils::Angle targetAngle = Utils::AdjustAngleByDirection(Utils::Angle::FromDirection(lastPathRes.moveDir), GetDirection());
                wishAngle = curAngle.MoveTo(targetAngle, Utils::ToRadian(rotationSpeed) * dt);

                return wishAngle.IsEqual(targetAngle);
            }
            return true;
        };

        if (state == State::Patrolling)
        {
            lastPathRes = UpdatePath(dt);

            AimByPathDir();

            if (anim)
            {
                anim->anim.ActivateLink((Utils::IsNonZero(lastPathRes.moveDir) && selectNextPointTimer <= 0.f) ? "Run" : "Idle");
            }
        }
        else if (state == State::PrepareAttack)
        {
            if (anim)
            {
                anim->anim.ActivateLink("Activated");
            }

            if (AimByPathDir())
            {
                state = State::Attacking;

                auto wasDir = lastPathRes.moveDir;
                auto wasFlipped = flipped;

                bool willPathKillThePlayer = BuildAttackPath(maxAttackTime, attackSpeed, &attackPathPoints);
                if (!willPathKillThePlayer)
                {
                    lastPathRes.moveDir = -lastPathRes.moveDir;
                    flipped = !flipped;

                    willPathKillThePlayer = BuildAttackPath(maxAttackTime, attackSpeed, &attackPathPoints);
                }

                if (willPathKillThePlayer)
                {
                    prevIndex = 0;
                    nextIndex = 1;
                    pathStep = 1;

                    moveSpeed = attackSpeed;
                    stopAtPathPointsTime = 0.f;
                }
                else
                {
                    lastPathRes.moveDir = wasDir;
                    flipped = wasFlipped;
                    state = State::Patrolling;
                    if (explosionRadius)
                    {
                        explosionRadius->SetVisiblity(false);
                    }
                }
            }
        }
        else if (state == State::Attacking)
        {
            if (anim)
            {
                anim->anim.ActivateLink("Activated");
            }

            if (!isDropAttack)
            {
                if (auto castRes = Utils::SphereCast(transform.position, {0.f, -1.f, 0.f}, TILE_SIZE * 5.f, controller->GetRadius(), PhysGroup::WorldDoorsPlayer))
                {
                    if (castRes->userdata && dynamic_cast<Player*>(castRes->userdata->object->GetParent()) && !WorldManager::instance->IsPlayerKilled())
                    {
                        isDropAttack = true;
                        gravity = 0.f;
                    }
                }
            }

            if (isDropAttack)
            {
                lastPathRes.moveDir = {0.f, -1.f, 0.f};

                gravity = std::min(gravity + dt * gravityAccelSpeed, gravitySpeedMax);
                lastPathRes.wishPos += lastPathRes.moveDir * gravity * dt;

                if (Utils::RayHit(transform.position, {0.f, -1.f, 0.f}, TILE_HALF_SIZE, PhysGroup::WorldDoors))
                {
                    Explode(1.f);
                }
            }
            else
            {
                pathPoints.swap(attackPathPoints);
                lastPathRes = UpdatePath(dt);
                pathPoints.swap(attackPathPoints);
            }

            AimByPathDir();

            auto size = explosionRadius->GetTransform().size;
            auto t = Utils::Saturate((maxAttackTime - attackTime) / maxRadiusScaleTime);

            if (explosionRadius)
            {
                auto targetRadiusScale = 2.0f * distanceToExplosionKill / size.x;
                auto radiusScale = Utils::Lerp(1.f, targetRadiusScale, Math::EaseInQuad(t));
                explosionRadius->GetTransform().scale = Math::Vector3(radiusScale, radiusScale, 0.0f);
            }

            attackTime = std::max(attackTime - dt, 0.f);
            if (attackTime <= 0.f)
            {
                Explode(t);
            }

            if (TryToExplode(t))
            {
                if (explosionRadius)
                {
                    explosionRadius->SetVisiblity(false);
                }
            }
        }

        ApplyTransform(lastPathRes.wishPos, wishAngle.ToSignedDegrees());
    }

    void SpiderDrone::EditorDraw(float dt)
    {
        if (GetScene()->IsPlaying() || !IsVisible())
        {
            return;
        }

        DroneV2::EditorDraw(dt);

        auto arrowDir = Math::Vector3{1.f, 0.f, 0.f} * (moveMode == MoveMode::Left ? -1.f : 1.f);
        arrowDir = transform.GetGlobal().MulNormal(arrowDir);

        Utils::DebugArrow(transform.position, transform.position + arrowDir * 36.f, COLOR_YELLOW_A(0.55f));
    }
}