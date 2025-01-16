
#pragma once

#include "SceneEntities/Physics/2D/PhysTriger2D.h"
#include "Common/Events.h"

namespace Orin::Overkill
{
	class NextLevelTrigger : public PhysTriger2D
	{
		bool trigered = false;

	protected:
		eastl::string nextLevel;

	public:

		virtual ~NextLevelTrigger() = default;

		META_DATA_DECL(NextLevelTrigger)

		void Play() override;
		void OnRestart(const EventOnRestart& evt);
		void OnContactStart(int index, SceneEntity* entity, int contactIndex) override;
		void Release() override;
	};
}
