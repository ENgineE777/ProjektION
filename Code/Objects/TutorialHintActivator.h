
#pragma once

#include "SceneEntities/Physics/2D/PhysTriger2D.h"
#include "Common/Events.h"
#include "TutorialHint.h"

namespace Orin::Overkill
{
	class TutorialHintActivator : public PhysTriger2D
	{		
		friend class TutorialHint;

		TutorialHint* hint = nullptr;
		bool activator = true;

	public:

		virtual ~TutorialHintActivator() = default;

		META_DATA_DECL(TutorialHintActivator)

		void Init() override;
		void Play() override;
		void SetHint(TutorialHint* setHint) { hint = setHint; };
		void OnContactStart(int index, SceneEntity* entity, int contactIndex) override;
	};
}
