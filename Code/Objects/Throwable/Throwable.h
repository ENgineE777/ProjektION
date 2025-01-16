
#pragma once

#include "SceneEntities/2D/SpriteEntity.h"
#include "SceneEntities/2D/AnimGraph2D.h"

namespace Orin::Overkill
{
	struct ThrowableDesc
	{
		SpriteEntity* thingSprite;
		AnimGraph2D* anim;
		SpriteEntity* trail;

		float hitRadius = 0.0f;
		float speed = 0.0f;
		float maxDistance = 0.0f;		

		eastl::function<void()> OnPickup;
		eastl::function<void()> OnThrow;
		eastl::function<bool(Math::Vector3, Math::Vector3, class SceneEntity*)> OnThrowableHit;
	};

	class ThrowableStand
	{
		ThrowableDesc throwableDesc;
		bool throwableAvailable = true;
	};
}
