
#include "FireExtinguisherStand.h"
#include "World/WorldManager.h"
#include "Root/Root.h"
#include "SceneEntities/2D/GenericMarker2D.h"
#include "Characters/Enemies/Mine.h"
#include "Characters/Enemies/Soldier.h"
#include "Characters/Enemies/Puppet.h"
#include "Characters/Enemies/ChargeDrone.h"
#include "Characters/Enemies/TurretSimple.h"
#include "Objects/FirePlayerKiller.h"
#include "Objects/ExplosionBarrel.h"

namespace Orin::Overkill
{
	ENTITYREG(SceneEntity, FireExtinguisherStand, "Overkill/Objects", "FireExtinguisherStand")

	META_DATA_DESC(FireExtinguisherStand)
		BASE_SCENE_ENTITY_PROP(FireExtinguisherStand)
		COLOR_PROP(FireExtinguisherStand, color, COLOR_YELLOW, "Geometry", "color")
		MARK_DISABLED_FOR_INSTANCE()

		FLOAT_PROP(FireExtinguisherStand, desc.hitRadius, 11.0f, "Projectile", "HitRadius", "Hit Radius")
		MARK_DISABLED_FOR_INSTANCE()

		FLOAT_PROP(FireExtinguisherStand, desc.speed, 500.0f, "Projectile", "Speed", "Speed")
		MARK_DISABLED_FOR_INSTANCE()

		FLOAT_PROP(FireExtinguisherStand, desc.maxDistance, 640.0f, "Projectile", "maxDistance", "Max Distance")
		MARK_DISABLED_FOR_INSTANCE()

		FLOAT_PROP(FireExtinguisherStand, radiusFireOff, 18.0f, "Projectile", "radiusFireOff", "Radius in which kill fire wall after hit")
		MARK_DISABLED_FOR_INSTANCE()

	META_DATA_DESC_END()

	ORIN_EVENTS(FireExtinguisherStand)
		ORIN_EVENT(FireExtinguisherStand, EventOnRestart, OnRestart)
	ORIN_EVENTS_END(FireExtinguisherStand)

	void FireExtinguisherStand::Release()
	{
		MainEventsQueue::Unsubscribe(this);

		PhysTriger2D::Release();
	}

	void FireExtinguisherStand::Play()
	{
		physGroup = PhysGroup::LevelObjects;

		PhysTriger2D::Play();

		anim = FindChild<AnimGraph2D>();

		foam = FindChild<VFXEmitter>();		

		desc.thingSprite = FindChild<SpriteEntity>("KnifeSprite");
		desc.thingSprite->SetVisiblity(false);

		desc.anim = FindChild<AnimGraph2D>("KnifeAnim");
		desc.anim->SetVisiblity(false);

		desc.trail = FindChild<SpriteEntity>("Trail");
		desc.trail->SetVisiblity(false);

		desc.OnPickup = eastl::function<void()>([this]() { OnPicked(); });
		desc.OnThrow = eastl::function<void()>([this]() { });
		desc.OnThrowableHit = eastl::function<bool(Math::Vector3, Math::Vector3, SceneEntity*)>([this](Math::Vector3 pos, Math::Vector3 dir, SceneEntity* entity) { return OnThrowableHit(pos, dir, entity); });

		ORIN_EVENTS_SUBSCRIBE(FireExtinguisherStand);

		Tasks(false)->AddTask(-50, this, (Object::Delegate)&FireExtinguisherStand::Update);
    }

	void FireExtinguisherStand::Update(float dt)
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

	void FireExtinguisherStand::NotifyPlayer(bool canGrab)
	{
		auto* player = WorldManager::instance->GetPlayer();
		player->OnThrowable2Pickup(canGrab ? &desc : nullptr, transform.position + Math::Vector3(-TILE_SIZE * 0.5f, 2.0f * TILE_SIZE, 0.0f), false);
	}

	void FireExtinguisherStand::OnContactStart(int index, SceneEntity* entity, int contactIndex)
	{
		if (dynamic_cast<Player*>(entity->GetParent()))
		{
			playerInside = true;
		}
	}

	void FireExtinguisherStand::OnContactEnd(int index, SceneEntity* entity, int contactIndex)
	{
		playerInside = false;

		NotifyPlayer(false);
	}

	void FireExtinguisherStand::OnRestart(const EventOnRestart& evt)
	{
		throwableAvailable = true;
		playerInside = false;
		anim->anim.GotoNode("Idle", false);
		desc.anim->anim.GotoNode("Fly", true);

		foam->ResetAndStop();		
	}

	void FireExtinguisherStand::OnPicked()
	{
		throwableAvailable = false;
		anim->anim.GotoNode("Broken", false);
	}

	bool FireExtinguisherStand::OnThrowableHit(Math::Vector3 pos, Math::Vector3 dir, SceneEntity* entity)
	{
		if (entity && (dynamic_cast<Enemy*>(entity) == nullptr && dynamic_cast<FirePlayerKiller*>(entity) == nullptr && dynamic_cast<ExplosionBarrel*>(entity) == nullptr))
		{
			return false;
		}

		eastl::vector<PhysScene::BodyUserData*> hitBodies;
		GetRoot()->GetPhysScene()->OverlapWithSphere(Sprite::ToUnits(pos), Sprite::ToUnits(18.0f), PhysGroup::DeathZone, hitBodies);

		for (auto* body : hitBodies)
		{
			if (auto* playerKiller = dynamic_cast<FirePlayerKiller*>(body->object))
			{
				playerKiller->Deactivate();
			}
		}

		auto& tm = foam->GetTransform();

		Math::Matrix mat;
		mat.Pos() = Sprite::ToUnits(pos);
		tm.SetGlobal(mat);

		foam->SetVisiblity(true);
		foam->Start();				

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