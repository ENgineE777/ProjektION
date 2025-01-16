
#pragma once

#include "SceneEntities/Physics/2D/PhysTriger2D.h"
#include "SceneEntities/2D/SpriteEntity.h"
#include "Root/Sounds/SoundEvent.h"
#include "Common/Events.h"
#include "Effects/VFXEmitter.h"

namespace Orin::Overkill
{
	class FirePlayerKiller : public PhysTriger2D
	{
		bool Active = true;
		VFXEmitter* fire = nullptr;

	public:

		virtual ~FirePlayerKiller() = default;

		META_DATA_DECL(FirePlayerKiller)

		void Init() override;
		void Play() override;
		void Release() override;
		void OnRestart(const EventOnRestart& evt);
		void OnContactStart(int index, SceneEntity* entity, int contactIndex) override;
		void Deactivate();
	};
}
