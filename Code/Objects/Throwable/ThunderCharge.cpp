
#include "ThunderCharge.h"
#include "World/WorldManager.h"
#include "Root/Root.h"
#include "SceneEntities/2D/GenericMarker2D.h"

namespace Orin::Overkill
{
	ENTITYREG(SceneEntity, ThunderCharge, "Overkill/Objects", "ThunderCharge")

	META_DATA_DESC(ThunderCharge)
		BASE_SCENE_ENTITY_PROP(ThunderCharge)
		COLOR_PROP(ThunderCharge, color, COLOR_YELLOW, "Geometry", "color")
		MARK_DISABLED_FOR_INSTANCE()

		FLOAT_PROP(ThunderCharge, cooldown, 5.0f, "ThunderCharge", "Cooldown", "Cooldown")
		MARK_DISABLED_FOR_INSTANCE()

		FLOAT_PROP(ThunderCharge, desc.hitRadius, 11.0f, "Projectile", "HitRadius", "Hit Radius")
		MARK_DISABLED_FOR_INSTANCE()

		FLOAT_PROP(ThunderCharge, desc.speed, 500.0f, "Projectile", "Speed", "Speed")
		MARK_DISABLED_FOR_INSTANCE()

		FLOAT_PROP(ThunderCharge, desc.maxDistance, 640.0f, "Projectile", "maxDistance", "Max Distance")
		MARK_DISABLED_FOR_INSTANCE()

	META_DATA_DESC_END()

	ORIN_EVENTS(ThunderCharge)
		ORIN_EVENT(ThunderCharge, EventOnRestart, OnRestart)
	ORIN_EVENTS_END(ThunderCharge)

	void ThunderCharge::Release()
	{
		MainEventsQueue::Unsubscribe(this);

		PhysTriger2D::Release();
	}

	void ThunderCharge::Play()
	{
		physGroup = PhysGroup::LevelObjects;

		PhysTriger2D::Play();

		anim = FindChild<AnimGraph2D>();

		desc.thingSprite = FindChild<SpriteEntity>("ThunderSprite");
		desc.thingSprite->SetVisiblity(false);

		desc.anim = FindChild<AnimGraph2D>("ThunderAnim");
		desc.anim->SetVisiblity(false);

		desc.OnPickup = eastl::function<void()>([this]() { OnPicked(); });
		desc.OnThrow = eastl::function<void()>([this]() { desc.anim->anim.GotoNode("Fly", true); });
		desc.OnThrowableHit = eastl::function<bool(Math::Vector3, Math::Vector3, SceneEntity*)>([this](Math::Vector3 pos, Math::Vector3 dir, SceneEntity* entity) { return OnThrowableHit(pos, dir, entity); });

		ORIN_EVENTS_SUBSCRIBE(ThunderCharge);

		Tasks(false)->AddTask(-50, this, (Object::Delegate)&ThunderCharge::Update);
    }

    void ThunderCharge::Update(float dt)
    {
        if (!thunderActive && cooldownLeft > 0.f)
		{
            cooldownLeft -= dt;
        }

		if (cooldownLeft <= 0.f)
		{
			anim->anim.GotoNode("Idle", false);

			if (playerInside)
			{
				NotifyPlayer(true, false);
			}
		}
    }

	void ThunderCharge::NotifyPlayer(bool canGrab, bool autoGrab)
	{
		auto* player = WorldManager::instance->GetPlayer();
		player->OnThrowable2Pickup(canGrab ? &desc : nullptr, transform.position + Math::Vector3(-TILE_SIZE * 0.5f, 2.0f * TILE_SIZE, 0.0f), autoGrab);
	}

	void ThunderCharge::OnContactStart(int index, SceneEntity* entity, int contactIndex)
	{
		if (dynamic_cast<Player*>(entity->GetParent()))
		{
			playerInside = true;

			auto* player = WorldManager::instance->GetPlayer();

			if (cooldownLeft <= 0.f && player->CanGrabThrowable())
			{
				NotifyPlayer(true, false);
			}
		}
	}

	void ThunderCharge::OnContactEnd(int index, SceneEntity* entity, int contactIndex)
	{
		playerInside = false;

		if (dynamic_cast<Player*>(entity->GetParent()))
		{
			NotifyPlayer(false, false);
		}
	}

	void ThunderCharge::OnRestart(const EventOnRestart& evt)
	{
		cooldownLeft = 0.f;
		playerInside = false;
		thunderActive = false;
		anim->anim.GotoNode("Idle", true);

		desc.thingSprite->SetVisiblity(false);
		desc.anim->SetVisiblity(false);
	}

	void ThunderCharge::OnPicked()
	{
		anim->anim.GotoNode("Broken", false);

		cooldownLeft = cooldown;
		thunderActive = true;
	}

	bool ThunderCharge::OnThrowableHit(Math::Vector3 pos, Math::Vector3 dir, SceneEntity* entity)
	{
		if (entity && dynamic_cast<Enemy*>(entity) == nullptr)
		{
			return false;
		}

		if (auto* enemy = dynamic_cast<Enemy*>(entity))
		{
			enemy->OnThunderHit(dir, this);
		}
		else
		{
			thunderActive = false;
		}

		return true;
	}

	void ThunderCharge::OnThunderDisposed()
	{
		thunderActive = false;

		if (playerInside && cooldownLeft <= 0.f)
		{
			NotifyPlayer(true, false);
		}
	}
}