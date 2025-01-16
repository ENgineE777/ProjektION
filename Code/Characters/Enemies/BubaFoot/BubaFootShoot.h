
#pragma once

#include "SceneEntities/Physics/2D/PhysTriger2D.h"
#include "Common/Events.h"
#include "BubaFoot.h"

namespace Orin::Overkill
{
	class BubaFootShoot : public PhysTriger2D
	{
		BubaFoot* bubaFoot = nullptr;
		bool activated = false;

	public:

		virtual ~BubaFootShoot() = default;

		META_DATA_DECL(BubaFootActivator)

		void Init() override;
		void Play() override;
		void OnRestart(const EventOnRestart& evt);
		void SetBubaFoot(BubaFoot* setBubaFoot) { bubaFoot = setBubaFoot; };
		void OnContactStart(int index, SceneEntity* entity, int contactIndex) override;
		void Release() override;
	};
}
