#include "BubaFootFlamethrower.h"
#include "Root/Root.h"
#include "World/WorldManager.h"
#include "Effects/VFXEmitter.h"

namespace Orin::Overkill
{
	ENTITYREG(SceneEntity, BubaFootFlamethrower, "Overkill/Objects/BubaFoot", "BubaFootFlamethrower")

	META_DATA_DESC(BubaFootFlamethrower)
		BASE_SCENE_ENTITY_PROP(BubaFootFlamethrower)

        FLOAT_PROP(BubaFootFlamethrower, timeToActivation, 1.f, "Property", "timeToActivation", "Time to activation")
		MARK_DISABLED_FOR_INSTANCE()

        FLOAT_PROP(BubaFootFlamethrower, timeToExplosion, 5.f, "Property", "timeToExplosion", "Time to explosion")
		MARK_DISABLED_FOR_INSTANCE()

        FLOAT_PROP(BubaFootFlamethrower, rotationSpeed, 80.f, "Property", "rotationSpeed", "Rotation speed")
		MARK_DISABLED_FOR_INSTANCE()

        FLOAT_PROP(BubaFootFlamethrower, activeMoveSpeed, 200.f, "Property", "activeMoveSpeed", "Active move speed")
		MARK_DISABLED_FOR_INSTANCE()

        FLOAT_PROP(BubaFootFlamethrower, activeAcceletaion, 1000.f, "Property", "activeAcceletaion", "Active acceletaion")
		MARK_DISABLED_FOR_INSTANCE()

        FLOAT_PROP(BubaFootFlamethrower, inactiveMoveSpeed, 100.f, "Property", "inactiveMoveSpeed", "Inactive move speed")
		MARK_DISABLED_FOR_INSTANCE()
    META_DATA_DESC_END()

	void BubaFootFlamethrower::Init()
	{
		ScriptEntity2D::Init();

		transform.objectType    = ObjectType::Object2D;
		transform.transformFlag = MoveXYZ | RectMoveXY | RotateZ;
	}

	void BubaFootFlamethrower::Play()
	{
		ScriptEntity2D::Play();

        flameVFX = FindChild<VFXEmitter>("flame");
        if (flameVFX)
        {
            flameVFX->ResetAndStop();
            flameVFX->dtScale = 2.0f;
        }
	}

	void BubaFootFlamethrower::Release()
	{
		ScriptEntity2D::Release();
	}

    void BubaFootFlamethrower::PlaySound(const char* name)
	{
		auto* inst = GetRoot()->GetSounds()->CreateSound(GetScene(), false, name);

		if (inst)
		{
			inst->Play(PlaySoundType::AutoDelete);
		}
	}

    void BubaFootFlamethrower::Start()
    {
        state = State::Active;
    }

    void BubaFootFlamethrower::Stop()
    {
        state = State::Inactive;
    }

    void BubaFootFlamethrower::ResetAndStop()
    {
        Stop();

        if (flameVFX)
        {
            flameVFX->ResetAndStop();
        }
    }

    void BubaFootFlamethrower::Update(float dt)
    {
        if (!flameVFX)
        {
            return;
        }

        if (state == State::Active)
        {
            flameVFX->Start();

            // const auto angleToPlayer = WorldManager::instance->GetAngleToPlayer(Sprite::ToPixels(transform.GetGlobal().Pos()));
            // transform.rotation = Utils::Vector::xyV(transform.rotation, angleToPlayer.ToSignedDegrees());
        }
        else if (state == State::Inactive)
        {
            flameVFX->Stop();
        }
    }
}
