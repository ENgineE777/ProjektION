
#include "TunnelDoor.h"
#include "Root/Root.h"
#include "World/WorldManager.h"

namespace Orin::Overkill
{
	ENTITYREG(SceneEntity, TunnelDoorEntity, "Overkill/Objects", "TunnelDoor")

	META_DATA_DESC(TunnelDoorEntity::TargetEnemy)
		SCENEOBJECT_PROP(TunnelDoorEntity::TargetEnemy, enemy, "Property", "enemy")
	META_DATA_DESC_END()

	META_DATA_DESC(TunnelDoorEntity)
		BASE_SCENE_ENTITY_PROP(TunnelDoorEntity)

		COLOR_PROP(TunnelDoorEntity, color, COLOR_YELLOW_A(0.5f), "Geometry", "color")
		MARK_DISABLED_FOR_INSTANCE()

		BOOL_PROP(TunnelDoorEntity, lockedDoor, false, "PropertiesTarget", "Is door locked", "Is door locked")
		BOOL_PROP(TunnelDoorEntity, oneWay, false, "PropertiesTarget", "oneWay", "oneWay")
		
		ARRAY_PROP(TunnelDoorEntity, targetEnmeies, TargetEnemy, "PropertiesTarget", "enemies")

	META_DATA_DESC_END()

	ORIN_EVENTS(TunnelDoorEntity)
		ORIN_EVENT(TunnelDoorEntity, EventOnRestart, OnRestart)
		ORIN_EVENT(TunnelDoorEntity, EventOnFinishLevel, OnFinishLevel)
	ORIN_EVENTS_END(TunnelDoorEntity)

	void TunnelDoorEntity::Init()
	{
		PhysEntity2D::Init();

		if (prefabInstance)
		{
			transform.transformFlag = TransformFlag::MoveXYZ | TransformFlag::RectMoveXY | TransformFlag::RotateZ;
		}

		Tasks(false)->AddTask(0, this, (Object::Delegate)&TunnelDoorEntity::EditorDraw);
	}

	void TunnelDoorEntity::ApplyProperties()
	{
		bodyType = BodyType::Kinematic;
		visibleDuringPlay = false;

		transform.size = { TILE_SIZE, TILE_SIZE, TILE_SIZE };
	}

	void TunnelDoorEntity::Play()
	{
		physGroup = PhysGroup::TunnelDoor;

		PhysEntity2D::Play();

		anim = FindChild<AnimGraph2D>();

		ORIN_EVENTS_SUBSCRIBE(TunnelDoorEntity);

		Tasks(true)->AddTask(9, this, (Object::Delegate)&TunnelDoorEntity::Update);

		if (lockedDoor)
		{			
			PhysTriger2D* trigger = FindChild<PhysTriger2D>();

			if (trigger)
			{
				trigger->SetPhysGroup(PhysGroup::LevelObjects);
				trigger->AddDelegate<eastl::function<void(int, SceneEntity*, int)>>("OnContactStart", this,
					[this](int index, SceneEntity* entity, int contactIndex)
					{
						if (Player* player = dynamic_cast<Player*>(entity->GetParent()))
						{
							showTargets = true;
						}
					});
			}
		}
	}

	void TunnelDoorEntity::OnRestart(const EventOnRestart& evt)
	{
		anim->anim.GotoNode((lockedDoor || forceToLock) ? "Locked" : "Unlocked", true);

		unlocked = !lockedDoor;

		showTargets = false;

		if ((oneWay && forceToLock) || (forceToLock && !WorldManager::instance->GetDifficulty()->allowCheckPoints))
		{
			waitAnimToLock = -1.0f;
			forceToLock = false;
			anim->anim.GotoNode("Unlocked", true);
		}
	}

	void TunnelDoorEntity::OnFinishLevel(const EventOnFinishLevel& evt)
	{
		forceToLock = false;
	}

	void TunnelDoorEntity::EditorDraw(float dt)
	{
		if (GetScene()->IsPlaying())
		{
			return;
		}

		auto doorPos = Sprite::ToPixels(transform.GetGlobal().Pos());

		int index = 0;

		for (auto& enemyRef : targetEnmeies)
		{
			auto* enemy = enemyRef.enemy.Get();

			if (enemy)
			{
				auto pos = Sprite::ToPixels(enemy->GetTransform().GetGlobal().Pos());

				Sprite::DebugText({ pos.x, pos.y }, Color(1.0f, 0.75f, 1.0f), "%i: %s", index, enemy->GetName());

				Sprite::DebugLine( pos, doorPos, Color(1.0f, 0.75f, 1.0f));
			}

			index++;
		}
	}

	void TunnelDoorEntity::ForceToLock()
	{
		forceToLock = true;
		anim->anim.GotoNode("Locked", true);
	}

	void TunnelDoorEntity::PlayOpen()
	{
		Utils::PlaySoundEvent("event:/Door");
		anim->anim.ActivateLink("Open");
	}

	void TunnelDoorEntity::PlayClose()
	{
		Utils::PlaySoundEvent("event:/Door");
		anim->anim.ActivateLink("Close");

		if (oneWay)
		{
			waitAnimToLock = 0.3f;
		}
	}

	void TunnelDoorEntity::Update(float dt)
	{
		if (unlocked)
		{
			if (waitAnimToLock > 0.0f)
			{
				waitAnimToLock -= dt;

				if (waitAnimToLock <= 0.0f)
				{
					waitAnimToLock = -1.0f;

					ForceToLock();
				}
			}

			return;
		}

		unlocked = true;

		float alpha = 1.0f;
		float size = 30.0f;

		if (showTargets)
		{
			heartTime += dt * 1.25f;

			if (heartTime > 1.0f)
			{
				heartTime -= 1.0f;
			}

			if (heartTime < 0.25f)
			{
				alpha = heartTime * 4.0f;
			}
			else
			if (heartTime < 0.5f)
			{
				alpha = (0.5f - heartTime) * 4.0f;
			}

			//size *= (0.25f + alpha * 0.5f);
		}

		for (auto& enemyRef : targetEnmeies)
		{
			auto* enemy = enemyRef.enemy.Get();

			if (enemy && !enemy->IsKilled())
			{
				unlocked = false;

				if (showTargets && WorldManager::instance->killMarker)
				{
					auto pos = Sprite::ToPixels(enemy->GetTransform().GetGlobal().Pos());
					pos.y += 30.0f;

					auto camPos = Sprite::GetCamPos();
					auto halfScreenSize = Sprite::GetHalfScreenSize() - 10.0f;

					float delta = 0.0f;

					if (camPos.x - halfScreenSize.x < pos.x && pos.x < camPos.x + halfScreenSize.x &&
						camPos.y - halfScreenSize.y < pos.y && pos.y < camPos.y + halfScreenSize.y)
					{
						if (camPos.x - halfScreenSize.x < pos.x && pos.x < camPos.x)
						{
							delta = pos.x - camPos.x + halfScreenSize.x;
						}

						if (camPos.x <= pos.x && pos.x < camPos.x + halfScreenSize.x)
						{
							delta = fmax(camPos.x + halfScreenSize.x - pos.x, delta);
						}

						if (camPos.y - halfScreenSize.y < pos.y && pos.y < camPos.y)
						{
							delta = fmax(pos.y - camPos.y + halfScreenSize.y, delta);
						}

						if (camPos.y <= pos.y && pos.y < camPos.y + halfScreenSize.y)
						{
							delta = fmax(camPos.y + halfScreenSize.y - pos.y, delta);
						}
					}

					float curAlpha = alpha * (1.0f - Utils::Saturate(delta / 20.0f));

					pos.x = fmax(pos.x, camPos.x - halfScreenSize.x);
					pos.x = fmin(pos.x, camPos.x + halfScreenSize.x);

					pos.y = fmax(pos.y, camPos.y - halfScreenSize.y);
					pos.y = fmin(pos.y, camPos.y + halfScreenSize.y);

					auto dir = Math::Vector2(pos.x, pos.y) - camPos;
					dir.Normalize();

					Math::Matrix mat;
					mat.RotateZ(Math::SafeAtan2(-dir.y, dir.x));
					mat.Pos() = Sprite::ToUnits(Math::Vector3(pos.x, pos.y, 0.0f));

					Sprite::Draw(WorldManager::instance->killMarker->texture.Get()->GetTexture(), COLOR_RED_A(curAlpha), mat, { -size * 0.5f, size * 0.5f }, size, 0.0f, 1.0f, Sprite::quadPrgNoZ);
				}
			}
		}

		if (unlocked && !forceToLock)
		{
			anim->anim.GotoNode("Unlocked", true);
		}
	}

	void TunnelDoorEntity::Release()
	{
		MainEventsQueue::Unsubscribe(this);

		PhysEntity2D::Release();
	}
}