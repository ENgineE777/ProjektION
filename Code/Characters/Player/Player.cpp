
#include "Player.h"
#include "Characters/Enemies/Enemy.h"
#include "Root/Root.h"
#include "World/WorldManager.h"

namespace Orin::Overkill
{
	ORIN_EVENTS(Player)
		ORIN_EVENT(Player, EventOnFinishLevel, OnFinishLevel)
	ORIN_EVENTS_END(Player)

	void Player::Play()
	{
		Character::Play();

		ORIN_EVENTS_SUBSCRIBE(Player);

		WorldManager::instance->SetPlayer(this);
	}

	void Player::Release()
	{
		if (WorldManager::instance)
		{
			WorldManager::instance->SetPlayer(nullptr);
		}

		Character::Release();
	}

	void Player::OnRestart(const EventOnRestart& evt)
	{
		Character::OnRestart(evt);

		WorldManager::instance->CenterCamera();
		levelFinished = false;
	}

	void Player::OnFinishLevel(const EventOnFinishLevel& evt)
	{
		levelFinished = true;
		isCheckpointActive = false;
		WorldManager::instance->SetFrontendState(FrontendState::GameplayResultWin);

		anim->anim.GotoNode("Idle", false);
	}

	void Player::OnCheckpointReached(const EventOnCheckpointReached &evt)
	{
		Character::OnCheckpointReached(evt);

		isCheckpointActive = true;
		checkpointPos = evt.pos;
	}

	void Player::Kill(Math::Vector3 killDir, DeathSource deathSource)
	{
		if (killed)
		{
			return;
		}

		WorldManager::instance->gameState.MarkDeath();		

		Character::Kill(killDir, deathSource);

		anim->anim.GotoNode("Killed", true);

		Utils::AddCameraShake(1.f);

		WorldManager::instance->SetFrontendState(FrontendState::GameplayResultLost);
	}
}