
#include "GrenadeStand.h"
#include "World/WorldManager.h"
#include "Root/Root.h"
#include "SceneEntities/2D/GenericMarker2D.h"
#include "Characters/Enemies/Soldier.h"
#include "Characters/Enemies/Mine.h"
#include "Objects/ExplosionBarrel.h"

namespace Orin::Overkill
{
	ENTITYREG(SceneEntity, GrenadeStand, "Overkill/Objects", "GrenadeStand")

	META_DATA_DESC(GrenadeStand)
		BASE_SCENE_ENTITY_PROP(GrenadeStand)
		COLOR_PROP(GrenadeStand, color, COLOR_YELLOW, "Geometry", "color")
		MARK_DISABLED_FOR_INSTANCE()

		FLOAT_PROP(GrenadeStand, desc.hitRadius, 11.0f, "Projectile", "HitRadius", "Hit Radius")
		MARK_DISABLED_FOR_INSTANCE()

		FLOAT_PROP(GrenadeStand, desc.speed, 500.0f, "Projectile", "Speed", "Speed")
		MARK_DISABLED_FOR_INSTANCE()

		FLOAT_PROP(GrenadeStand, desc.maxDistance, 640.0f, "Projectile", "maxDistance", "Max Distance")
		MARK_DISABLED_FOR_INSTANCE()

		FLOAT_PROP(GrenadeStand, explotionRadius, 95.0f, "Projectile", "explotionRadius", "Explotion Radius")
		MARK_DISABLED_FOR_INSTANCE()

	META_DATA_DESC_END()

	ORIN_EVENTS(GrenadeStand)
		ORIN_EVENT(GrenadeStand, EventOnRestart, OnRestart)
	ORIN_EVENTS_END(GrenadeStand)

	void GrenadeStand::Release()
	{
		MainEventsQueue::Unsubscribe(this);

		PhysTriger2D::Release();
	}

	void GrenadeStand::Play()
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

		ORIN_EVENTS_SUBSCRIBE(GrenadeStand);

		Tasks(false)->AddTask(-50, this, (Object::Delegate)&GrenadeStand::Update);
    }

	void GrenadeStand::NotifyPlayer(bool canGrab)
	{
		auto* player = WorldManager::instance->GetPlayer();
		player->OnThrowable2Pickup(canGrab ? &desc : nullptr, transform.position + Math::Vector3(-TILE_SIZE * 0.5f, 2.0f * TILE_SIZE, 0.0f), false);
	}

	void GrenadeStand::Update(float dt)
	{
		if (playerInside)
		{
			auto* player = WorldManager::instance->GetPlayer();

			if (throwableAvailable && player->CanGrabThrowable())
			{
				NotifyPlayer(true);
			}
		}
	}

	void GrenadeStand::OnContactStart(int index, SceneEntity* entity, int contactIndex)
	{
		if (dynamic_cast<Player*>(entity->GetParent()))
		{
			playerInside = true;
		}
	}

	void GrenadeStand::OnContactEnd(int index, SceneEntity* entity, int contactIndex)
	{
		playerInside = false;

		NotifyPlayer(false);
	}

	void GrenadeStand::OnRestart(const EventOnRestart& evt)
	{
		throwableAvailable = true;
		playerInside = false;
		anim->anim.GotoNode("Idle", false);
		desc.anim->anim.GotoNode("Fly", true);
	}

	void GrenadeStand::OnPicked()
	{
		throwableAvailable = false;
		anim->anim.GotoNode("Broken", false);
	}

	bool GrenadeStand::OnThrowableHit(Math::Vector3 pos, Math::Vector3 dir, SceneEntity* entity)
	{
		if (entity && (dynamic_cast<Enemy*>(entity) == nullptr && dynamic_cast<ExplosionBarrel*>(entity) == nullptr))
		{
			return false;
		}

		if (auto* mine = dynamic_cast<Mine*>(entity))
		{
			mine->Kill(dir, DeathSource::Unknown);
		}

		if (auto* barrel = dynamic_cast<ExplosionBarrel*>(entity))
		{
			if (!barrel->Explode())
			{
				return false;
			}
		}

		Utils::PlaySoundEvent("event:/Bomb/expl", &transform.position);

		WorldManager::instance->Detonate(pos, explotionRadius, nullptr);

		return true;
	}
}