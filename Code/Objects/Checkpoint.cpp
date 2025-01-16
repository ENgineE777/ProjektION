
#include "Checkpoint.h"
#include "World/WorldManager.h"
#include "Root/Root.h"
#include "SceneEntities/2D/GenericMarker2D.h"

namespace Orin::Overkill
{
	ENTITYREG(SceneEntity, Checkpoint, "Overkill/Objects", "Checkpoint")

	META_DATA_DESC(Checkpoint)
		BASE_SCENE_ENTITY_PROP(Checkpoint)
		COLOR_PROP(Checkpoint, color, COLOR_YELLOW, "Geometry", "color")
		SCENEOBJECT_PROP(Checkpoint, door, "Property", "door")
	META_DATA_DESC_END()

	ORIN_EVENTS(Checkpoint)
		ORIN_EVENT(Checkpoint, EventOnCheckpointReached, OnCheckpointReached)
		ORIN_EVENT(Checkpoint, EventOnFinishLevel, OnFinishLevel)
	ORIN_EVENTS_END(Checkpoint)
		
	void Checkpoint::Init()
	{
		PhysTriger2D::Init();

		Tasks(false)->AddTask(0, this, (Object::Delegate)&Checkpoint::EditorDraw);
		visibleDuringPlay = false;
	}

	void Checkpoint::Release()
	{
		MainEventsQueue::Unsubscribe(this);

		PhysTriger2D::Release();
	}

	void Checkpoint::SetLigtVisibility(bool visible)
	{
		eastl::vector<PointLight2D*> lights;

		FindChilds<PointLight2D>(lights);

		for (auto* light : lights)
		{
			light->SetVisiblity(visible);
		}
	}

	void Checkpoint::Play()
	{
		physGroup = PhysGroup::LevelObjects;

		PhysTriger2D::Play();

		ORIN_EVENTS_SUBSCRIBE(Checkpoint);

		SetLigtVisibility(false);

		if (auto *p = FindChild<GenericMarker2D>())
		{
			startPos = Sprite::ToPixels(p->GetTransform().GetGlobal().Pos());
		}
		else
		{
			startPos = transform.position;
		}

		if (WorldManager::instance->gameState.GetCheckpointID() == GetUID())
		{
			GetScene()->AddPostPlay(0, this, (Object::DelegateSimple)&Checkpoint::ActivateSelf);
		}
	}

	void Checkpoint::ActivateSelf()
	{
		if (WorldManager::instance->GetDifficulty()->allowCheckPoints)
		{
			MainEventsQueue::PushEvent(EventOnCheckpointReached{ startPos, GetUID() });
		}
	}

	void Checkpoint::OnCheckpointReached(const EventOnCheckpointReached& evt)
	{
		if (evt.checpointID != GetUID())
		{
			isActive = false;

			SetLigtVisibility(false);
		}
	}	

	void Checkpoint::OnContactStart(int index, SceneEntity* entity, int contactIndex)
	{
		if (dynamic_cast<Player*>(entity->GetParent()))
		{
			if (!isActive)
			{
				isActive = true;

				SetLigtVisibility(true);

				GetRoot()->Log("OnContactStart", "Checkpoint has been reached!");
				
				if (door)
				{
					door->ForceToLock();
				}
			}

			ActivateSelf();
		}
	}

	void Checkpoint::EditorDraw(float dt)
	{
		if (GetScene()->IsPlaying())
		{
			return;
		}

		if (door)
		{
			auto doorPos = Sprite::ToPixels(transform.GetGlobal().Pos());

			auto pos = Sprite::ToPixels(door->GetTransform().GetGlobal().Pos());

			Sprite::DebugLine(pos, doorPos, Color(1.0f, 0.15f, 0.15f));
		}
	}

	void Checkpoint::OnFinishLevel(const EventOnFinishLevel& evt)
	{
		isActive = false;

		SetLigtVisibility(false);
	}
}