#include "MovingPlatform.h"

#include "Common/Utils.h"
#include "World/WorldManager.h"

namespace Orin::Overkill
{
    ENTITYREG(SceneEntity, MovingPlatform, "Overkill/Objects", "MovingPlatform")

	META_DATA_DESC(MovingPlatform)
		BASE_SCENE_ENTITY_PROP(MovingPlatform)

        FLOAT_OPTIONAL_PROP(MovingPlatform, overrideMoveSpeed, 0.f, "Movement", "overrideMoveSpeed", "Override: Move speed")
        FLOAT_PROP(MovingPlatform, moveSpeed, 100.f, "Movement", "moveSpeed", "Move speed")
        MARK_DISABLED_FOR_INSTANCE()

        FLOAT_OPTIONAL_PROP(MovingPlatform, overrideStopAtPathPointsTime, 0.f, "Movement", "overrideStopAtPathPointsTime", "Override: Stop and wait and path points (seconds)")
        FLOAT_PROP(MovingPlatform, stopAtPathPointsTime, 0.f, "Movement", "stopAtPathPointsTime", "Stop and wait and path points (seconds)")
        MARK_DISABLED_FOR_INSTANCE()

        FLOAT_OPTIONAL_PROP(MovingPlatform, overrideShouldStopOnlyAtEndPoints, 0.f, "Movement", "overrideShouldStopOnlyAtEndPoints", "Override: Stop movement in case the end point has been reached")
        BOOL_PROP(MovingPlatform, shouldStopOnlyAtEndPoints, false, "Movement", "shouldStopOnlyAtEndPoints", "Stop movement in case the end point has been reached")
        MARK_DISABLED_FOR_INSTANCE()

        BOOL_PROP(MovingPlatform, loopPath, false, "Movement", "loopPath", "Move by path points in a loop")
        MARK_DISABLED_FOR_INSTANCE()
	META_DATA_DESC_END()

    void MovingPlatform::Init()
    {
        DroneV2::Init();

        rotationSpeed = 0.f;
        aimSpeed = 0.f;
        distanceToInstantKill = -1.f;
    }

    void MovingPlatform::Release()
    {
        DroneV2::Release();
    }

    void MovingPlatform::Play()
    {
        DroneV2::Play();

        velocity = {};
    }

    void MovingPlatform::DoUpdate(float dt)
    {
        const PathRes pathRes = UpdatePath(dt);
        velocity = selectNextPointTimer > 0.f ? Math::Vector3{} : (pathRes.moveDir * moveSpeed);
        ApplyTransform(pathRes.wishPos, transform.rotation.z);
    }
}