
#include "Enemy.h"
#include "Root/Root.h"
#include "World/WorldManager.h"

namespace Orin::Overkill
{
	bool Enemy::IsImmortal()
	{
		for (auto* protector : protectors)
		{
			if (!protector->IsKilled())
			{
				return true;
			}
		}

		return false;
	}

	void Enemy::OnThunderHit(Math::Vector3 hitDir, ThunderCharge* source)
	{
		Kill(hitDir, DeathSource::Unknown);
	}

	void Enemy::Kill(Math::Vector3 killDir, DeathSource deathSource)
	{
		if (!killed)
		{
			WorldManager::instance->gameState.MarkKill();		
			WorldManager::instance->IncreaseComboMeter();
			WorldManager::instance->AddScore(killScore);
		}

		Character::Kill(killDir, deathSource);				
	}

}