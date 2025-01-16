
#pragma once

#include "SceneEntities/Physics/2D/PhysTriger2D.h"
#include "Common/Events.h"
#include "Throwable.h"

namespace Orin::Overkill
{
	class ThunderCharge : public PhysTriger2D
	{
		ThrowableDesc desc;

		bool playerInside = false;
		bool thunderActive = false;
		float cooldown;
		float cooldownLeft = 0.f;

		AnimGraph2D* anim = nullptr;		

	public:

		virtual ~ThunderCharge() = default;

		META_DATA_DECL(ThunderCharge)

		void Release() override;
		void Play() override;

		void Update(float dt);

		void OnContactStart(int index, SceneEntity* entity, int contactIndex) override;
		void OnContactEnd(int index, SceneEntity* entity, int contactIndex) override;

		void OnRestart(const EventOnRestart& evt);
		void OnPicked();
		bool OnThrowableHit(Math::Vector3 pos, Math::Vector3 dir, SceneEntity* enemy);

		void OnThunderDisposed();
		void NotifyPlayer(bool canGrab, bool autoGrab);
	};
}
