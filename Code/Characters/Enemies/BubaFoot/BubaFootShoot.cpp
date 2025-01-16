
#include "BubaFootShoot.h"
#include "Root/Root.h"
#include "World/WorldManager.h"
#include "World/BloodManager.h"
#include "SceneEntities/Physics/2D/PhysEntity2D.h"

namespace Orin::Overkill
{
	ENTITYREG(SceneEntity, BubaFootShoot, "Overkill/Objects/BubaFoot", "BubaFootShoot")

	META_DATA_DESC(BubaFootShoot)
		BASE_SCENE_ENTITY_PROP(BubaFootShoot)
		MARK_DISABLED_FOR_INSTANCE()
	META_DATA_DESC_END()

	ORIN_EVENTS(BubaFootShoot)
		ORIN_EVENT(BubaFootShoot, EventOnRestart, OnRestart)
	ORIN_EVENTS_END(BubaFootShoot)

	void BubaFootShoot::Init()
	{
		PhysTriger2D::Init();

		if (prefabInstance)
		{
			transform.transformFlag = MoveXYZ | RectMoveXY;
		}

		visibleDuringPlay = false;

		color = COLOR_MAGNETA_A(0.5f);
	}

	void BubaFootShoot::Play()
	{
		physGroup = PhysGroup::LevelObjects;

		PhysTriger2D::Play();

		ORIN_EVENTS_SUBSCRIBE(BubaFootShoot);
	}

	void BubaFootShoot::OnRestart(const EventOnRestart& evt)
	{
		activated = false;
	}

	void BubaFootShoot::OnContactStart(int index, SceneEntity* entity, int contactIndex)
	{
		if (activated)
		{
			return;
		}

		GetRoot()->Log("BubaFootShoot", "OnContactStart");

		if (dynamic_cast<Player*>(entity->GetParent()))
		{
			if (bubaFoot)
			{
				GetRoot()->Log("BubaFootActivator", "bubaFoot");
				bubaFoot->LaunchMissle(Sprite::ToPixels(transform.GetGlobal().Pos()));

				activated = true;
			}
		}
	}

	void BubaFootShoot::Release()
	{
		MainEventsQueue::Unsubscribe(this);

		PhysTriger2D::Release();
	}
}