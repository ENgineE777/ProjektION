
#include "ExplosionBarrel.h"
#include "Root/Root.h"
#include "World/WorldManager.h"
#include "World/BloodManager.h"
#include "Characters/Enemies/Soldier.h"
#include "SceneEntities/Physics/2D/PhysEntity2D.h"

namespace Orin::Overkill
{
	ENTITYREG(SceneEntity, ExplosionBarrel, "Overkill/Objects", "ExplosionBarrel")

	META_DATA_DESC(ExplosionBarrel)
		BASE_SCENE_ENTITY_PROP(ExplosionBarrel)

		FLOAT_PROP(ExplosionBarrel, explotionRadius, 95.0f, "Barrel", "explotionRadius", "Explotion Radius")
		MARK_DISABLED_FOR_INSTANCE()

	META_DATA_DESC_END()

	ORIN_EVENTS(ExplosionBarrel)
		ORIN_EVENT(ExplosionBarrel, EventOnRestart, OnRestart)
	ORIN_EVENTS_END(ExplosionBarrel)

	void ExplosionBarrel::Init()
	{
		Node2D::Init();

		if (prefabInstance)
		{
			transform.transformFlag = MoveXYZ | RectMoveXY;
		}
	}

	void ExplosionBarrel::Play()
	{
		Node2D::Play();

		ORIN_EVENTS_SUBSCRIBE(ExplosionBarrel);

		sprite = FindChild<SpriteEntity>();

		anim = FindChild<AnimGraph2D>();

		if (PhysTriger2D* trigger = FindChild<PhysTriger2D>())
		{
			trigger->SetPhysGroup(PhysGroup::Enemy_);
		}
	}

	void ExplosionBarrel::OnRestart(const EventOnRestart& evt)
	{
		active = true;
		sprite->SetVisiblity(true);
		anim->anim.GotoNode("Fly", true);
	}	

	void ExplosionBarrel::Release()
	{
		MainEventsQueue::Unsubscribe(this);

		Node2D::Release();
	}

	bool ExplosionBarrel::Explode()
	{
		if (!active)
		{
			return false;
		}

		active = false;
		sprite->SetVisiblity(false);

		WorldManager::instance->Detonate(transform.position, explotionRadius, this);
		anim->anim.ActivateLink("Hit");
		Utils::PlaySoundEvent("event:/Bomb/expl", &transform.position);

		return true;
	}
}