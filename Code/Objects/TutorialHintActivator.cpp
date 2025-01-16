
#include "TutorialHintActivator.h"
#include "Root/Root.h"
#include "World/WorldManager.h"
#include "World/BloodManager.h"
#include "SceneEntities/Physics/2D/PhysEntity2D.h"

namespace Orin::Overkill
{
	ENTITYREG(SceneEntity, TutorialHintActivator, "Overkill/Objects", "TutorialHintActivator")

	META_DATA_DESC(TutorialHintActivator)
		BASE_SCENE_ENTITY_PROP(TutorialHintActivator)
		BOOL_PROP(TutorialHintActivator, activator, true, "Property", "is activator", "is activator")
		MARK_DISABLED_FOR_INSTANCE()
	META_DATA_DESC_END()

	void TutorialHintActivator::Init()
	{
		PhysTriger2D::Init();

		if (prefabInstance)
		{
			transform.transformFlag = MoveXYZ | RectMoveXY;
		}

		visibleDuringPlay = false;

		color = COLOR_CYAN_A(0.5f);
	}

	void TutorialHintActivator::Play()
	{
		physGroup = PhysGroup::LevelObjects;

		PhysTriger2D::Play();		
	}

	void TutorialHintActivator::OnContactStart(int index, SceneEntity* entity, int contactIndex)
	{		
		GetRoot()->Log("TutorialHintActivator", "OnContactStart");

		if (auto* player = dynamic_cast<Player*>(entity->GetParent()))
		{
			if (player->IsTutorialHintAllowed() && hint)
			{
				GetRoot()->Log("TutorialHintActivator", "hint->SetVisiblity");
				hint->Acivate(activator);
			}
		}		
	}
}