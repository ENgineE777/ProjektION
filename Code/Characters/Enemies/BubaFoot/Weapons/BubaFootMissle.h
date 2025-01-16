
#pragma once

#include "SceneEntities/2D/ScriptEntity2D.h"
#include "SceneEntities/2D/AnimGraph2D.h"
#include "SceneEntities/Physics/2D/KinematicCapsule2D.h"
#include "Common/Events.h"
#include "Common/Constants.h"

namespace Orin::Overkill
{
    class VFXEmitter;

	class BubaFootMissle : public ScriptEntity2D
	{
	protected:
        enum class State
        {
            Inactive,
            Active,
            Exploded
        };

        State state = State::Inactive;

        float deleteTimeout = 1.f;

        float timeToActivation = 1.f;
        float timeToExplosion = 0.f;

        float rotationSpeed = 80.f;

        float activeMoveSpeed   = 200.f;
        float activeAcceletaion = 1000.f;

        float inactiveMoveSpeed = 100.f;

        float curMoveSpeed = 0.f;

        float killRadius = 0.f;

        void Explosion();

	public:
        META_DATA_DECL_BASE(BubaFootMissle)

        AnimGraph2D *anim = nullptr;
        KinematicCapsule2D *controller = nullptr;
        VFXEmitter *steamVFX = nullptr;

        Math::Vector3 targetPos;

        class BubaFoot* owner = nullptr;

		void Init() override;
		void Play() override;
		void Release() override;

		void Update(float dt) override;
	};
}
