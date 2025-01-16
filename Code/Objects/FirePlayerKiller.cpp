
#include "FirePlayerKiller.h"
#include "Root/Root.h"
#include "World/WorldManager.h"
#include "World/BloodManager.h"
#include "Characters/Enemies/Soldier.h"
#include "SceneEntities/Physics/2D/PhysEntity2D.h"

namespace Orin::Overkill
{
	ENTITYREG(SceneEntity, FirePlayerKiller, "Overkill/Objects", "FirePlayerKiller")

	META_DATA_DESC(FirePlayerKiller)
		BASE_SCENE_ENTITY_PROP(FirePlayerKiller)
		COLOR_PROP(FirePlayerKiller, color, COLOR_YELLOW_A(0.5f), "Geometry", "color")
		MARK_DISABLED_FOR_INSTANCE()
	META_DATA_DESC_END()

	ORIN_EVENTS(FirePlayerKiller)
		ORIN_EVENT(FirePlayerKiller, EventOnRestart, OnRestart)
	ORIN_EVENTS_END(FirePlayerKiller)

	void FirePlayerKiller::Init()
	{
		PhysTriger2D::Init();

		if (prefabInstance)
		{
			transform.transformFlag = MoveXYZ | RectMoveXY;
		}

		visibleDuringPlay = false;
	}

	void FirePlayerKiller::Play()
	{
		physGroup = PhysGroup::DeathZone;

		PhysTriger2D::Play();

		ORIN_EVENTS_SUBSCRIBE(FirePlayerKiller);

		fire = FindChild<VFXEmitter>();
		fire->ResetAndStop();
	}

	void FirePlayerKiller::OnRestart(const EventOnRestart& evt)
	{
		Active = true;
		fire->Start();
	}	

	void FirePlayerKiller::Release()
	{
		MainEventsQueue::Unsubscribe(this);

		PhysTriger2D::Release();
	}

	void FirePlayerKiller::OnContactStart(int index, SceneEntity* entity, int contactIndex)
	{
		if (!Active)
		{
			return;
		}

		if (dynamic_cast<Player*>(entity->GetParent()))
		{
			auto dir = WorldManager::instance->GetPlayerPos() - transform.position - transform.size * Math::Vector3(0.5f, -0.5f, 0.0f);
			dir.Normalize();			

			WorldManager::instance->KillPlayer(dir, DeathSource::Fun);			
		}		
	}

	void FirePlayerKiller::Deactivate()
	{
		Active = false;
		fire->Stop();
	}
}