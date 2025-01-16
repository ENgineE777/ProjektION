
#pragma once

#include "Common/Utils.h"
#include "Enemy.h"

namespace Orin::Overkill
{
	class Mine : public Enemy
	{
		bool triggered = false;
		float explodeTimer = -1.0f;

		SoundEvent* beepSound = nullptr;
		SpriteEntity* radiusVizualisation = nullptr;
		SpriteEntity* radiusVizualisationStatic = nullptr;

		const float timeToExplode = 0.6f;
		const float explodeRadius = 160.0f * TILE_SCALE;
		const float instantExplodeRadius = 40.0f * TILE_SCALE;

		float animTimer = 0.0f;
		float radiusScale = 1.0f;

		inline float GetExplodeRadius() { return overrideExplodeRadius.enabled ? overrideExplodeRadius.value : explodeRadius; };
		void AnimateRadius(float dt);
		void DrawRadius(SpriteEntity* entity, float dt);

	public:
		
		Optional<float> overrideExplodeTimer;
		Optional<float> overrideExplodeRadius;

		META_DATA_DECL_BASE(Mine)

		void Init() override;
		void Update(float dt) override;
		void Play() override;

		void Kill(Math::Vector3 killDir, DeathSource deathSource) override;
		void EditorDraw(float dt);
		void DrawExplodeRadius(float dt);

		void OnRestart(const EventOnRestart &evt) override;
		bool IsImmortal() override { return true; }

		void Release() override;
	};
}
