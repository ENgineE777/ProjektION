
#pragma once

#include "SceneEntities/2D/ScriptEntity2D.h"
#include "SceneEntities/2D/AnimGraph2D.h"
#include "SceneEntities/2D/SpriteEntity.h"
#include "SceneEntities/Physics/2D/KinematicCapsule2D.h"
#include "SceneEntities/2D/Node2D.h"
#include "Characters/Character.h"

namespace Orin::Overkill
{
	class Enemy : public Character
	{
	protected:

		int killScore = 0;

	public:

		eastl::vector<Enemy*> protectors;
		bool IsImmortal();
		void OnThunderHit(Math::Vector3 hitDir, ThunderCharge* source) override;
		void Kill(Math::Vector3 killDir, DeathSource deathSource) override;
	};
}
