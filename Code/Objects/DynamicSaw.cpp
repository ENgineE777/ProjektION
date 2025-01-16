#include "DynamicSaw.h"

#include "Common/Utils.h"
#include "World/WorldManager.h"

namespace Orin::Overkill
{
    ENTITYREG(SceneEntity, DynamicSaw, "Overkill/Objects", "DynamicSaw")

	META_DATA_DESC(DynamicSaw)
		BASE_SCENE_ENTITY_PROP(DynamicSaw)

        FLOAT_OPTIONAL_PROP(DynamicSaw, overrideMoveSpeed, 0.f, "Movement", "overrideMoveSpeed", "Override: Move speed")
        FLOAT_PROP(DynamicSaw, moveSpeed, 100.f, "Movement", "moveSpeed", "Move speed")
        MARK_DISABLED_FOR_INSTANCE()

        FLOAT_OPTIONAL_PROP(DynamicSaw, overrideStopAtPathPointsTime, 0.f, "Movement", "overrideStopAtPathPointsTime", "Override: Stop and wait and path points (seconds)")
        FLOAT_PROP(DynamicSaw, stopAtPathPointsTime, 0.f, "Movement", "stopAtPathPointsTime", "Stop and wait and path points (seconds)")
        MARK_DISABLED_FOR_INSTANCE()

        FLOAT_OPTIONAL_PROP(DynamicSaw, overrideShouldStopOnlyAtEndPoints, 0.f, "Movement", "overrideShouldStopOnlyAtEndPoints", "Override: Stop movement in case the end point has been reached")
        BOOL_PROP(DynamicSaw, shouldStopOnlyAtEndPoints, false, "Movement", "shouldStopOnlyAtEndPoints", "Stop movement in case the end point has been reached")
        MARK_DISABLED_FOR_INSTANCE()

        BOOL_PROP(DynamicSaw, loopPath, false, "Movement", "loopPath", "Move by path points in a loop")
        MARK_DISABLED_FOR_INSTANCE()
	META_DATA_DESC_END()

    void DynamicSaw::Init()
    {
        DroneV2::Init();

        rotationSpeed = 0.f;
        aimSpeed = 0.f;
        distanceToInstantKill = -1.f;
    }

    void DynamicSaw::Release()
    {
        DroneV2::Release();
    }

    void DynamicSaw::Play()
    {
        DroneV2::Play();
    }

    void DynamicSaw::DoUpdate(float dt)
    {
        const PathRes pathRes = UpdatePath(dt);
        ApplyTransform(pathRes.wishPos, transform.rotation.z);
    }
}