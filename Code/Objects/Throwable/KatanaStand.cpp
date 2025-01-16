
#include "KatanaStand.h"
#include "World/WorldManager.h"
#include "Root/Root.h"
#include "SceneEntities/2D/GenericMarker2D.h"
#include "Characters/Enemies/Soldier.h"
#include "Characters/Enemies/Mine.h"
#include "Characters/Enemies/Puppet.h"
#include "Characters/Enemies/ChargeDrone.h"
#include "Characters/Enemies/TurretSimple.h"
#include "Objects/ExplosionBarrel.h"

namespace Orin::Overkill
{
	ENTITYREG(SceneEntity, KatanaStand, "Overkill/Objects", "KatanaStand")

	META_DATA_DESC(KatanaStand)
		BASE_SCENE_ENTITY_PROP(KatanaStand)
		COLOR_PROP(KatanaStand, color, COLOR_YELLOW, "Geometry", "color")
		MARK_DISABLED_FOR_INSTANCE()

		FLOAT_PROP(KatanaStand, desc.hitRadius, 11.0f, "Projectile", "HitRadius", "Hit Radius")
		MARK_DISABLED_FOR_INSTANCE()

		FLOAT_PROP(KatanaStand, desc.speed, 500.0f, "Projectile", "Speed", "Speed")
		MARK_DISABLED_FOR_INSTANCE()

		FLOAT_PROP(KatanaStand, desc.maxDistance, 640.0f, "Projectile", "maxDistance", "Max Distance")
		MARK_DISABLED_FOR_INSTANCE()

		BOOL_PROP(KatanaStand, oneTimeUse, true, "Projectile", "oneTimeUse", "One Time Use")

	META_DATA_DESC_END()

	ORIN_EVENTS(KatanaStand)
		ORIN_EVENT(KatanaStand, EventOnRestart, OnRestart)
	ORIN_EVENTS_END(KatanaStand)

	void KatanaStand::Release()
	{
		MainEventsQueue::Unsubscribe(this);

		PhysTriger2D::Release();
	}

	void KatanaStand::Play()
	{
		physGroup = PhysGroup::LevelObjects;

		PhysTriger2D::Play();

		anim = FindChild<AnimGraph2D>();

		desc.thingSprite = FindChild<SpriteEntity>("KnifeSprite");
		desc.thingSprite->SetVisiblity(false);

		desc.anim = FindChild<AnimGraph2D>("KnifeAnim");
		desc.anim->SetVisiblity(false);

		desc.trail = FindChild<SpriteEntity>("Trail");
		desc.trail->SetVisiblity(false);

		desc.OnPickup = eastl::function<void()>([this]() { OnPicked(); });
		desc.OnThrow = eastl::function<void()>([this]() { });
		desc.OnThrowableHit = eastl::function<bool(Math::Vector3, Math::Vector3, SceneEntity*)>([this](Math::Vector3 pos, Math::Vector3 dir, SceneEntity* entity) { return OnThrowableHit(pos, dir, entity); });

		ORIN_EVENTS_SUBSCRIBE(KatanaStand);

		Tasks(false)->AddTask(-50, this, (Object::Delegate)&KatanaStand::Update);
    }

	void KatanaStand::Update(float dt)
	{
		if (cooldown > 0.0f)
		{
			cooldown -= dt;

			if (cooldown <= 0.0f)
			{
				cooldown = -1.0f;

				throwableAvailable = true;
				anim->anim.GotoNode("Idle", false);
				desc.anim->anim.GotoNode("Fly", true);
			}
		}

		if (playerInside)
		{
			auto* player = WorldManager::instance->GetPlayer();

			if (throwableAvailable && player->CanGrabThrowable())
			{
				NotifyPlayer(true);
			}
		}
	}

	void KatanaStand::NotifyPlayer(bool canGrab)
	{
		auto* player = WorldManager::instance->GetPlayer();
		player->OnThrowable2Pickup(canGrab ? &desc : nullptr, transform.position + Math::Vector3(-TILE_SIZE * 0.5f, 2.0f * TILE_SIZE, 0.0f), false);
	}

	void KatanaStand::OnContactStart(int index, SceneEntity* entity, int contactIndex)
	{
		if (dynamic_cast<Player*>(entity->GetParent()))
		{
			playerInside = true;
		}
	}

	void KatanaStand::OnContactEnd(int index, SceneEntity* entity, int contactIndex)
	{
		playerInside = false;

		NotifyPlayer(false);
	}

	void KatanaStand::OnRestart(const EventOnRestart& evt)
	{
		throwableAvailable = true;
		playerInside = false;
		anim->anim.GotoNode("Idle", false);
		desc.anim->anim.GotoNode("Fly", true);
	}

	void KatanaStand::OnPicked()
	{
		throwableAvailable = false;
		anim->anim.GotoNode("Broken", false);

		if (!oneTimeUse)
		{
			cooldown = 2.0f;
		}
	}

	bool KatanaStand::OnThrowableHit(Math::Vector3 pos, Math::Vector3 dir, SceneEntity* entity)
	{
		if (entity && (dynamic_cast<Enemy*>(entity) == nullptr && dynamic_cast<ExplosionBarrel*>(entity) == nullptr))
		{
			return false;
		}

		if (auto* mine = dynamic_cast<Mine*>(entity))
		{
			mine->Kill(dir, DeathSource::Unknown);
		}

		if (auto* soldier = dynamic_cast<Soldier*>(entity))
		{
			soldier->Hit(dir, 1);
		}

		if (auto* chargeDrone = dynamic_cast<ChargeDrone*>(entity))
		{
			chargeDrone->Agro();
		}

		if (auto* turret = dynamic_cast<TurretSimple*>(entity))
		{
			turret->StartRageAttack();
		}

		if (auto* puppet = dynamic_cast<Puppet*>(entity))
		{
			puppet->Kill(dir, DeathSource::Unknown);
		}

		if (auto* barrel = dynamic_cast<ExplosionBarrel*>(entity))
		{
			if (!barrel->Explode())
			{
				return false;
			}
		}

		return true;
	}
}