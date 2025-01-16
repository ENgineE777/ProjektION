
#pragma once

#include "SceneEntities/2D/ScriptEntity2D.h"
#include "SceneEntities/2D/AnimGraph2D.h"
#include "SceneEntities/Physics/2D/KinematicCapsule2D.h"
#include "Common/Events.h"
#include "Common/Constants.h"

namespace Orin::Overkill
{
    class VFXEmitter;

	class BubaFootFlamethrower : public ScriptEntity2D
	{
	protected:
        enum class State
        {
            Inactive,
            Active
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

        void PlaySound(const char* name);

	public:
        META_DATA_DECL_BASE(BubaFootFlamethrower)

        VFXEmitter *flameVFX = nullptr;

        Math::Vector3 targetPos;

		void Init() override;
		void Play() override;
		void Release() override;

		void Update(float dt) override;

        void Start();
        void Stop();
        void ResetAndStop();
	};
}
