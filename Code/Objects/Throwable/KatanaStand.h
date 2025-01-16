
#pragma once

#include "SceneEntities/Physics/2D/PhysTriger2D.h"
#include "Common/Events.h"
#include "SceneEntities/2D/AnimGraph2D.h"
#include "Throwable.h"

namespace Orin::Overkill
{
	class KatanaStand : public PhysTriger2D
	{
		ThrowableDesc desc;

		bool playerInside = false;
		bool throwableAvailable = true;

		AnimGraph2D* anim = nullptr;
		void NotifyPlayer(bool canGrab);

		bool oneTimeUse = true;
		float cooldown = -1.0f;

	public:

		virtual ~KatanaStand() = default;

		META_DATA_DECL(KatanaStand)

		void Release() override;
		void Play() override;

		void Update(float dt);

		void OnContactStart(int index, SceneEntity* entity, int contactIndex) override;
		void OnContactEnd(int index, SceneEntity* entity, int contactIndex) override;

		void OnRestart(const EventOnRestart& evt);

		void OnPicked();
		bool OnThrowableHit(Math::Vector3 pos, Math::Vector3 dir, SceneEntity* enemy);
	};
}
