
#pragma once

#include "SceneEntities/Physics/2D/PhysTriger2D.h"
#include "Common/Events.h"
#include "BubaFoot.h"

namespace Orin::Overkill
{
	class BubaFootActivator : public PhysTriger2D
	{		
		BubaFoot* bubaFoot = nullptr;
		bool doNotAttackPlayer = false;

	public:

		virtual ~BubaFootActivator() = default;

		META_DATA_DECL(BubaFootActivator)

		void Init() override;
		void Play() override;
		void SetBubaFoot(BubaFoot* setBubaFoot) { bubaFoot = setBubaFoot; };
		void OnContactStart(int index, SceneEntity* entity, int contactIndex) override;		
	};
}
