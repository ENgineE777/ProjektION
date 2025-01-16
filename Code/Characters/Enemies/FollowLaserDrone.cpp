#include "FollowLaserDrone.h"

#include "Common/Utils.h"
#include "World/WorldManager.h"

namespace Orin::Overkill
{
    ENTITYREG(SceneEntity, FollowLaserDrone, "Overkill/Characters", "FollowLaserDrone")

	META_DATA_DESC(FollowLaserDrone)
		BASE_SCENE_ENTITY_PROP(FollowLaserDrone)

        ARRAY_PROP(FollowLaserDrone, laserRays, LaserRay, "Laser", "laserRays")
        MARK_DISABLED_FOR_INSTANCE()

        ASSET_TEXTURE_PROP(FollowLaserDrone, laserBeam, "Laser", "laserBeam")
        MARK_DISABLED_FOR_INSTANCE()

        FLOAT_PROP(FollowLaserDrone, distanceToInstantKill, 18.f, "Drone", "distanceToInstantKill", "Distance to instant kill the player")
		MARK_DISABLED_FOR_INSTANCE()

        FLOAT_PROP(FollowLaserDrone, moveSpeed, 100.f, "Movement", "moveSpeed", "Move speed")
        MARK_DISABLED_FOR_INSTANCE()

        FLOAT_PROP(FollowLaserDrone, stopAtPathPointsTime, 0.f, "Movement", "stopAtPathPointsTime", "Stop and wait and path points (seconds)")
        MARK_DISABLED_FOR_INSTANCE()

        BOOL_PROP(FollowLaserDrone, shouldStopOnlyAtEndPoints, false, "Movement", "shouldStopOnlyAtEndPoints", "Stop movement in case the end point has been reached")
        MARK_DISABLED_FOR_INSTANCE()

        BOOL_PROP(FollowLaserDrone, loopPath, false, "Movement", "loopPath", "Move by path points in a loop")
        MARK_DISABLED_FOR_INSTANCE()

        FLOAT_PROP(FollowLaserDrone, rotationSpeed, 100.f, "Rotation", "rotationSpeed", "Rotation speed")
        MARK_DISABLED_FOR_INSTANCE()

        DRONE_MOVEMENT_OVERRIDES(FollowLaserDrone)
        DRONE_ROTATION_OVERRIDES(FollowLaserDrone)
	META_DATA_DESC_END()

    void FollowLaserDrone::Play()
    {
        LaserDrone::Play();

        initialMoveSpeed = moveSpeed;

        Activate(false);
    }

    void FollowLaserDrone::ActionOnTrigger()
    {
        if (totalTime > 0.25f)
        {
            Activate(true);
        }
    }

    void FollowLaserDrone::OnRestart(const EventOnRestart &evt)
    {
        LaserDrone::OnRestart(evt);
        bodySprite->SetVisiblity(true);
        Activate(false);
    }

    void FollowLaserDrone::Kill(Math::Vector3 killDir, DeathSource deathSource)
    {
        Enemy::Kill(killDir, deathSource);
        bodySprite->SetVisiblity(false);
    }

    void FollowLaserDrone::Activate(bool isActive)
    {
        isTriggerActive = isActive;
        moveSpeed = isActive ? initialMoveSpeed * (WorldManager::instance->GetDifficulty()->followLaserSpeedMul) : 0.f;

        for (auto &ray : laserRays)
        {
            ray.isActive = isActive;
        }

        if (trigger)
        {
            trigger->SetVisiblity(!isActive);
        }
    }

    void FollowLaserDrone::OnSelectNextPathPoint()
    {
        if (nextIndex == 1)
        {
            Activate(false);
        }
    }

    void FollowLaserDrone::EditorDraw(float dt)
    {
        if (GetScene()->IsPlaying() || !IsVisible())
		{
			return;
		}

        LaserDrone::EditorDraw(dt);

        if (PhysTriger2D *trigger = FindChild<PhysTriger2D>())
        {
            Sprite::DebugLine(transform.position, Sprite::ToPixels(trigger->GetTransform().GetGlobal().Pos()), COLOR_YELLOW);
        }
    }
}