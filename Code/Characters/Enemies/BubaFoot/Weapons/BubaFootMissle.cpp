#include "BubaFootMissle.h"
#include "Root/Root.h"
#include "World/WorldManager.h"
#include "Effects/VFXEmitter.h"
#include "Characters/Enemies/Puppet.h"
#include "Characters/Enemies/BubaFoot/BubaFoot.h"

namespace Orin::Overkill
{
	ENTITYREG(SceneEntity, BubaFootMissle, "Overkill/Objects/BubaFoot", "BubaFootMissle")

	META_DATA_DESC(BubaFootMissle)
		BASE_SCENE_ENTITY_PROP(BubaFootMissle)

        FLOAT_PROP(BubaFootMissle, timeToActivation, 1.f, "Property", "timeToActivation", "Time to activation")
		MARK_DISABLED_FOR_INSTANCE()

        FLOAT_PROP(BubaFootMissle, timeToExplosion, 5.f, "Property", "timeToExplosion", "Time to explosion")
		MARK_DISABLED_FOR_INSTANCE()

        FLOAT_PROP(BubaFootMissle, rotationSpeed, 80.f, "Property", "rotationSpeed", "Rotation speed")
		MARK_DISABLED_FOR_INSTANCE()

        FLOAT_PROP(BubaFootMissle, activeMoveSpeed, 200.f, "Property", "activeMoveSpeed", "Active move speed")
		MARK_DISABLED_FOR_INSTANCE()

        FLOAT_PROP(BubaFootMissle, activeAcceletaion, 1000.f, "Property", "activeAcceletaion", "Active acceletaion")
		MARK_DISABLED_FOR_INSTANCE()

        FLOAT_PROP(BubaFootMissle, inactiveMoveSpeed, 100.f, "Property", "inactiveMoveSpeed", "Inactive move speed")
		MARK_DISABLED_FOR_INSTANCE()

        FLOAT_PROP(BubaFootMissle, killRadius, TILE_SIZE, "Property", "killRadius", "Kill radius")
		MARK_DISABLED_FOR_INSTANCE()
    META_DATA_DESC_END()

	void BubaFootMissle::Init()
	{
		ScriptEntity2D::Init();

		transform.objectType    = ObjectType::Object2D;
		transform.transformFlag = MoveXYZ | RectMoveXY | RotateZ;
	}

	void BubaFootMissle::Play()
	{
		ScriptEntity2D::Play();

        anim = FindChild<AnimGraph2D>("anim");
        controller = FindChild<KinematicCapsule2D>("controller");
        steamVFX = FindChild<VFXEmitter>("steam");

        anim->anim.GotoNode("Inactive", true);
	}

	void BubaFootMissle::Release()
	{
		ScriptEntity2D::Release();
	}

    void BubaFootMissle::Explosion()
    {
        state = State::Exploded;

        anim->anim.GotoNode("Explosion", true);

        Utils::PlaySoundEvent("event:/Bomb/expl", &transform.position);

        auto* puppet = GetScene()->FindEntity<Puppet>("puppetIntro");

        if (puppet && puppet->timeToWaitRocket > 0.1f)
        {
            puppet->timeToWaitRocket = -0.1f;
            return;
        }

        WorldManager::instance->Detonate(transform.position, killRadius, owner);
    }

    void BubaFootMissle::Update(float dt)
    {
        if (!IsVisible())
		{
			GetScene()->DeleteEntity(this, true);
			return;
		}

        if (!anim || state == State::Exploded)
        {
            steamVFX->Stop();

            deleteTimeout -= dt;
            if (deleteTimeout <= 0.f)
            {
                SetVisiblity(false);
            }
            return;
        }

        if (timeToActivation > 0.f)
        {
            timeToActivation -= dt;
            if (timeToActivation <= 0.f)
            {
                state = State::Active;

                anim->anim.GotoNode("Active", true);
            }
        }

        Utils::Angle wishAngle = Utils::Angle::Degrees(transform.rotation.z);

        if (state == State::Active)
        {
            const Math::Vector3 dirToTarget  = Utils::Vector::Normalize(targetPos - transform.position);
            const Utils::Angle angleToTarget = Utils::Angle::FromDirection(dirToTarget);
            const Utils::Angle curAngle      = Utils::Angle::Degrees(transform.rotation.z);
            
            wishAngle = curAngle.MoveTo(angleToTarget, Utils::ToRadian(rotationSpeed) * dt);

            curMoveSpeed += activeAcceletaion * dt;

            timeToExplosion -= dt;

            if (timeToExplosion <= 0.f)
            {
                Explosion();
            }            
        }
        else if (state == State::Inactive)
        {
            curMoveSpeed = inactiveMoveSpeed;
        }

        transform.rotation = Utils::Vector::xyV(transform.rotation, wishAngle.ToSignedDegrees());
        if (controller)
        {
            const auto moveDir = wishAngle.ToDirection();

            auto& tm = controller->GetTransform();
			tm.rotation = Utils::Vector::xyV(tm.rotation, wishAngle.ToSignedDegrees());

            controller->controller->SetUpDirection(moveDir);
            controller->Move(moveDir * curMoveSpeed, PhysGroup::WorldDoors);

            if (steamVFX)
            {
                steamVFX->emitterVelocity = moveDir * curMoveSpeed;
            }

            if (controller->controller->IsColliding(PhysController::CollideSides)
                || controller->controller->IsColliding(PhysController::CollideUp)
                || controller->controller->IsColliding(PhysController::CollideDown))
            {
                Explosion();
            }
        }
        else
        {
            transform.position += wishAngle.ToDirection() * curMoveSpeed * dt;
        }
    }
}
