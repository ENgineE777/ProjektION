#include "RotatingSaw.h"

#include "Common/Utils.h"
#include "World/WorldManager.h"

namespace Orin::Overkill
{
    ENTITYREG(SceneEntity, RotatingSaw, "Overkill/Objects", "RotatingSaw")

	META_DATA_DESC(RotatingSaw)
		BASE_SCENE_ENTITY_PROP(RotatingSaw)

        FLOAT_OPTIONAL_PROP(RotatingSaw, overrideRotationSpeed, -1.f, "Rotation", "overrideRotationSpeed", "Override: Rotation speed")
        FLOAT_PROP(RotatingSaw, rotationSpeed, 100.f, "Rotation", "rotationSpeed", "Rotation speed")
        MARK_DISABLED_FOR_INSTANCE()
	META_DATA_DESC_END()

    void RotatingSaw::Init()
    {
        DroneV2::Init();

        aimSpeed = 0.f;
        distanceToInstantKill = -1.f;
    }

    void RotatingSaw::Release()
    {
        DroneV2::Release();
    }

    void RotatingSaw::Play()
    {
        DroneV2::Play();
    }

    void RotatingSaw::DoUpdate(float dt)
    {
        auto wishAngle = Utils::Angle::Degrees(transform.rotation.z).Add(rotationSpeed * dt, Utils::Angle::TDegrees{});
        ApplyTransform(Utils::Vector::Rotate(initPos, -wishAngle.ToRadian()), wishAngle.ToDegrees());
    }

    void RotatingSaw::EditorDraw(float dt)
	{
		if (GetScene()->IsPlaying() || !IsVisible())
		{
			return;
		}

        DroneV2::EditorDraw(dt);

        if (parent)
        {
            const auto center = Sprite::ToPixels(parent->GetTransform().GetGlobal().Pos());
            const auto pos    = Sprite::ToPixels(transform.GetGlobal().Pos());
            Utils::DebugArrow(pos, center, COLOR_YELLOW_A(0.55f));
        }
	}
}