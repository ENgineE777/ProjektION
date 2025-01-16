
#include "Teleporter.h"
#include "Root/Root.h"
#include "World/WorldManager.h"
#include "SceneEntities/Physics/2D/PhysEntity2D.h"

namespace Orin::Overkill
{
	ENTITYREG(SceneEntity, Teleporter, "Overkill/Objects", "Teleporter")

	META_DATA_DESC(Teleporter)
		BASE_SCENE_ENTITY_PROP(Teleporter)
		COLOR_PROP(Teleporter, color, COLOR_YELLOW_A(0.5f), "Geometry", "color")
		MARK_DISABLED_FOR_INSTANCE()
	META_DATA_DESC_END()

	void Teleporter::Init()
	{
		PhysTriger2D::Init();

		if (prefabInstance)
		{
			transform.transformFlag = MoveXYZ | RectMoveXY;
		}

		visibleDuringPlay = false;

		Tasks(true)->AddTask(10, this, (Object::Delegate)&Teleporter::Update);
	}

	void Teleporter::Play()
	{
		physGroup = PhysGroup::LevelObjects;

		PhysTriger2D::Play();

		dest = FindChild<Node2D>();
	}

	void Teleporter::Update(float dt)
	{
		if (GetScene()->IsPlaying())
		{
			if (playerInside)
			{
				MainEventsQueue::PushEvent(CmdShowTunnelHint{ 2, transform.position + Math::Vector3(-18.0f, 18.0f, 0.0f) });
			}

			
			return;
		}

		if (!dest)
		{
			dest = FindChild<Node2D>();
		}

		if (dest)
		{
			auto pos = Sprite::ToPixels(transform.GetGlobal().Pos());

			auto destPos = Sprite::ToPixels(dest->GetTransform().GetGlobal().Pos());

			Sprite::DebugLine(pos, destPos, Color(1.0f, 0.75f, 1.0f));
		}
	}

	void Teleporter::OnContactStart(int index, SceneEntity* entity, int contactIndex)
	{
		auto* payer = dynamic_cast<Player*>(entity->GetParent());

		if (payer && payer->HasPuppet())
		{
			playerInside = true;

			if (dest)
			{
				auto destPos = Sprite::ToPixels(dest->GetTransform().GetGlobal().Pos());
				MainEventsQueue::PushEvent(CmdTeleportState{ true, destPos });
			}
		}		
	}

	void Teleporter::OnContactEnd(int index, SceneEntity* entity, int contactIndex)
	{
		if (dynamic_cast<Player*>(entity->GetParent()))
		{
			playerInside = false;
			MainEventsQueue::PushEvent(CmdTeleportState{ false, 0.0f });
		}
	}
}