
#include "InstantPlayerKiller.h"
#include "Root/Root.h"
#include "World/WorldManager.h"
#include "World/BloodManager.h"
#include "Characters/Enemies/Soldier.h"
#include "SceneEntities/Physics/2D/PhysEntity2D.h"

namespace Orin::Overkill
{
	ENTITYREG(SceneEntity, InstantPlayerKiller, "Overkill/Objects", "InstantPlayerKiller")

	META_DATA_DESC(InstantPlayerKiller)
		BASE_SCENE_ENTITY_PROP(InstantPlayerKiller)
		COLOR_PROP(InstantPlayerKiller, color, COLOR_YELLOW_A(0.5f), "Geometry", "color")
		MARK_DISABLED_FOR_INSTANCE()
	META_DATA_DESC_END()

	ORIN_EVENTS(InstantPlayerKiller)
		ORIN_EVENT(InstantPlayerKiller, EventOnRestart, OnRestart)
	ORIN_EVENTS_END(InstantPlayerKiller)

	void InstantPlayerKiller::Init()
	{
		PhysTriger2D::Init();

		if (prefabInstance)
		{
			transform.transformFlag = MoveXYZ | RectMoveXY;
		}

		visibleDuringPlay = false;
	}

	void InstantPlayerKiller::Play()
	{
		physGroup = PhysGroup::DeathZone;

		PhysTriger2D::Play();

		ORIN_EVENTS_SUBSCRIBE(InstantPlayerKiller);

		rotor = FindChild<SpriteEntity>("rotor");
		rotorThingBlood = FindChild<SpriteEntity>("rotor_thing_blood");
		rotorBlood = FindChild<SpriteEntity>("rotor_blood");

		Tasks(true)->AddTask(10, this, (Object::Delegate)&InstantPlayerKiller::Update);
	}

	void InstantPlayerKiller::OnRestart(const EventOnRestart& evt)
	{
		curSpeedRotation = speedRotation;
		broken = false;
		bloodState = 0;

		UpdateSpritesVisibility();
	}

	void InstantPlayerKiller::UpdateSpritesVisibility()
	{
		if (rotorThingBlood)
		{
			rotorThingBlood->SetVisiblity(bloodState & 1);
		}

		if (rotorBlood)
		{
			rotorBlood->SetVisiblity(bloodState & 2);
		}
	}

	void InstantPlayerKiller::Update(float dt)
	{
		if (AnimGraph2D::pause)
		{
			return;
		}

		if (rotor)
		{
			if (broken && curSpeedRotation > 0.0f)
			{
				curSpeedRotation = fmax(0.0f, curSpeedRotation - dt * 100.0f);
			}

			auto& trans = rotor->GetTransform();
			auto rot = trans.rotation;

			rot.z += dt * curSpeedRotation;

			trans.rotation = rot;

			if (rotorThingBlood)
			{
				auto& transBlood = rotorThingBlood->GetTransform();
				transBlood.rotation = trans.rotation;
			}

			if (rotorBlood)
			{
				auto& transBlood = rotorBlood->GetTransform();
				transBlood.rotation = trans.rotation;
			}
		}

		if (WorldManager::instance && WorldManager::instance->GetDistanceToPlayer(transform.position) < 10.f * TILE_SIZE)
		{
			if (!noiseSound)
			{
				noiseSound = GetRoot()->GetSounds()->CreateSoundEvent("event:/Fan");
				if (noiseSound)
				{
					noiseSound->Play();
				}
			}
		}
		else
		{
			RELEASE(noiseSound);
		}

		if (noiseSound)
		{
			noiseSound->Set3DAttributes(Sprite::ToPixels(transform.GetGlobal().Pos()));
		}
	}

	void InstantPlayerKiller::Release()
	{
		MainEventsQueue::Unsubscribe(this);

		RELEASE(noiseSound)

		PhysTriger2D::Release();
	}

	void InstantPlayerKiller::OnContactStart(int index, SceneEntity* entity, int contactIndex)
	{
		if (broken)
		{
			return;
		}

		if (dynamic_cast<Player*>(entity->GetParent()))
		{
			auto dir = WorldManager::instance->GetPlayerPos() - transform.position - transform.size * Math::Vector3(0.5f, -0.5f, 0.0f);
			dir.Normalize();			

			WorldManager::instance->KillPlayer(dir, DeathSource::Fun);
			bloodState |= 1;
			broken = true;
		}
		else
		if (auto* physEntity = dynamic_cast<PhysEntity2D*>(entity))
		{
			for (auto *parent = physEntity->GetParent(); parent; parent = parent->GetParent())
			{
				if (dynamic_cast<Soldier*>(parent))
				{
					//Utils::PlaySoundEvent("event:/Soldier/FAN death", &transform.position);
					break;
				}
			}

			auto mat = physEntity->GetTransform().GetGlobal();
			auto pos = Sprite::ToPixels(mat.Pos());
			auto dir = pos - transform.position - transform.size * Math::Vector3(0.5f, -0.5f, 0.0f);
			dir.Normalize();

			bloodState |= 2;

			auto size = physEntity->GetTransform().size;
			float k = size.x * size.y / 64.0f;

			physEntity->body.body->AddForceAt(mat.Pos(), dir * Utils::GetRandom(0.0125f, 0.0125f) * k * 1.75f);

			if (!StringUtils::IsEqual(physEntity->GetName(), "Gun"))
			{
				WorldManager::instance->GetBlood()->Spawn("BodyPartTrail", physEntity, pos, Math::SafeAtan2(-mat.Vx().y, -mat.Vx().x));
			}
		}
			
		UpdateSpritesVisibility();
	}
}