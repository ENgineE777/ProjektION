
#pragma once

#include "SceneEntities/Physics/2D/PhysTriger2D.h"
#include "SceneEntities/2D/SpriteEntity.h"
#include "Root/Sounds/SoundEvent.h"
#include "Common/Events.h"

namespace Orin::Overkill
{
	class InstantPlayerKiller : public PhysTriger2D
	{
		SpriteEntity* rotor = nullptr;
		SpriteEntity* rotorThingBlood = nullptr;
		SpriteEntity* rotorBlood = nullptr;

		SoundEvent* noiseSound = nullptr;

		const float speedRotation = 300.0f;
		float curSpeedRotation = 0.0f;
		bool broken = false;
		int bloodState = 0;
		void UpdateSpritesVisibility();

	public:

		virtual ~InstantPlayerKiller() = default;

		META_DATA_DECL(InstantPlayerKiller)

		void Init() override;
		void Play() override;
		void Update(float dt);
		void Release() override;
		void OnRestart(const EventOnRestart& evt);
		void OnContactStart(int index, SceneEntity* entity, int contactIndex) override;
	};
}
