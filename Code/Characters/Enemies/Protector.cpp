
#include "Protector.h"
#include "Root/Root.h"
#include "World/WorldManager.h"

namespace Orin::Overkill
{
	ENTITYREG(SceneEntity, Protector, "Overkill/Objects", "Protector")

	META_DATA_DESC(Protector::TargetEnemy)
		SCENEOBJECT_PROP(Protector::TargetEnemy, enemy, "Property", "enemy")
	META_DATA_DESC_END()

	META_DATA_DESC(Protector)
		BASE_SCENE_ENTITY_PROP(Protector)
	
		ARRAY_PROP(Protector, targetEnmeies, TargetEnemy, "PropertiesTarget", "enemies")

	META_DATA_DESC_END()

	ORIN_EVENTS(Protector)
		ORIN_EVENT(Protector, EventOnRestart, OnRestart)
	ORIN_EVENTS_END(Protector)

	void Protector::Init()
	{
		Enemy::Init();

		Tasks(false)->AddTask(0, this, (Object::Delegate)&Protector::EditorDraw);
	}

	void Protector::Play()
	{
		Enemy::Play();
	}

	void Protector::OnRestart(const EventOnRestart& evt)
	{
		Enemy::OnRestart(evt);

		if (!addedToTargets)
		{
			for (auto& enemyRef : targetEnmeies)
			{
				auto* enemy = enemyRef.enemy.Get();

				if (enemy)
				{
					enemy->protectors.push_back(this);
				}
			}

			addedToTargets = true;
		}
	}

	void Protector::Update(float dt)
	{
		if (killed)
		{
			return;
		}

		auto protectorPos = Sprite::ToPixels(transform.GetGlobal().Pos());

		for (auto& enemyRef : targetEnmeies)
		{
			auto* enemy = enemyRef.enemy.Get();

			if (enemy && !enemy->IsKilled())
			{
				auto pos = Sprite::ToPixels(enemy->GetTransform().GetGlobal().Pos());
				Sprite::DebugLine(pos, protectorPos, Color(0.75f, 1.0f, 1.0f));
			}
		}
	}

	void Protector::EditorDraw(float dt)
	{
		if (GetScene()->IsPlaying())
		{
			return;
		}

		auto protectorPos = Sprite::ToPixels(transform.GetGlobal().Pos());

		int index = 0;

		for (auto& enemyRef : targetEnmeies)
		{
			auto* enemy = enemyRef.enemy.Get();

			if (enemy)
			{
				auto pos = Sprite::ToPixels(enemy->GetTransform().GetGlobal().Pos());

				Sprite::DebugText({ pos.x, pos.y }, Color(0.75f, 1.0f, 1.0f), "%i: %s", index, enemy->GetName());

				Sprite::DebugLine(pos, protectorPos, Color(0.75f, 1.0f, 1.0f));
			}

			index++;
		}
	}
}