
#pragma once

#include "SceneEntities/2D/Node2D.h"
#include "SceneEntities/2D/SpriteEntity.h"
#include "Root/Sounds/SoundEvent.h"
#include "Common/Events.h"
#include "Effects/VFXEmitter.h"
#include "SceneEntities/2D/AnimGraph2D.h"

namespace Orin::Overkill
{
	class ExplosionBarrel : public Node2D
	{
		bool active = true;
		SpriteEntity* sprite = nullptr;

		float explotionRadius = 95.0f;
		AnimGraph2D* anim = nullptr;

	public:

		virtual ~ExplosionBarrel() = default;

		META_DATA_DECL(ExplosionBarrel)

		void Init() override;
		void Play() override;
		void Release() override;
		void OnRestart(const EventOnRestart& evt);
		bool Explode();
	};
}
