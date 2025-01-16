
#include "NextLevelTrigger.h"
#include "World/WorldManager.h"
#include "Root/Root.h"

namespace Orin::Overkill
{
	ENTITYREG(SceneEntity, NextLevelTrigger, "Overkill/Objects", "NextLevel")

	META_DATA_DESC(NextLevelTrigger)
		BASE_SCENE_ENTITY_PROP(NextLevelTrigger)
		COLOR_PROP(NextLevelTrigger, color, COLOR_YELLOW, "Geometry", "color")
		STRING_PROP(NextLevelTrigger, nextLevel, "", "Data", "next level")
	META_DATA_DESC_END()

	ORIN_EVENTS(NextLevelTrigger)
		ORIN_EVENT(NextLevelTrigger, EventOnRestart, OnRestart)
	ORIN_EVENTS_END(NextLevelTrigger)

	void NextLevelTrigger::Play()
	{
		physGroup = PhysGroup::DeathZone;

		PhysTriger2D::Play();

		ORIN_EVENTS_SUBSCRIBE(NextLevelTrigger);

		visibleDuringPlay = false;
	}

	void NextLevelTrigger::OnRestart(const EventOnRestart& evt)
	{
		trigered = false;
	}

	void NextLevelTrigger::OnContactStart(int index, SceneEntity* entity, int contactIndex)
	{
		if (trigered)
		{
			return;
		}

		if (dynamic_cast<Player*>(entity->GetParent()))
		{
			GetRoot()->Log("OnContactStart", "NextLevelTrigger");

			Utils::PlaySoundEvent("event:/Level Complete");

			WorldManager::instance->SetNextLevel(nextLevel, true);
			MainEventsQueue::PushEvent(EventOnFinishLevel{});

			trigered = true;
		}
	}

	void NextLevelTrigger::Release()
	{
		MainEventsQueue::Unsubscribe(this);

		PhysTriger2D::Release();
	}
}