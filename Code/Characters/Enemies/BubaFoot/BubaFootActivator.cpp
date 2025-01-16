
#include "BubaFootActivator.h"
#include "Root/Root.h"
#include "World/WorldManager.h"
#include "World/BloodManager.h"
#include "SceneEntities/Physics/2D/PhysEntity2D.h"

namespace Orin::Overkill
{
	ENTITYREG(SceneEntity, BubaFootActivator, "Overkill/Objects/BubaFoot", "BubaFootActivator")

	META_DATA_DESC(BubaFootActivator)
		BASE_SCENE_ENTITY_PROP(BubaFootActivator)
		MARK_DISABLED_FOR_INSTANCE()

		BOOL_PROP(BubaFootActivator, doNotAttackPlayer, false, "Property", "doNotAttackPlayer", "doNotAttackPlayer")
	META_DATA_DESC_END()

	void BubaFootActivator::Init()
	{
		PhysTriger2D::Init();

		if (prefabInstance)
		{
			transform.transformFlag = MoveXYZ | RectMoveXY;
		}

		visibleDuringPlay = false;

		color = COLOR_CYAN_A(0.5f);
	}

	void BubaFootActivator::Play()
	{
		physGroup = PhysGroup::LevelObjects;

		PhysTriger2D::Play();		
	}

	void BubaFootActivator::OnContactStart(int index, SceneEntity* entity, int contactIndex)
	{		
		GetRoot()->Log("BubaFootActivator", "OnContactStart");

		if (dynamic_cast<Player*>(entity->GetParent()))
		{
			if (bubaFoot)
			{
				GetRoot()->Log("BubaFootActivator", "bubaFoot");
				bubaFoot->Activate();
				bubaFoot->doNotAttackPlayer = doNotAttackPlayer;
			}
		}		
	}
}