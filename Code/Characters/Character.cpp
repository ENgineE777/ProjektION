
#include "Character.h"
#include "Root/Root.h"
#include "World/WorldManager.h"
#include "World/BloodManager.h"
#include "SceneEntities/Physics/2D/PhysEntity2D.h"
#include "SceneEntities/Physics/2D/Joint2D.h"

namespace Orin::Overkill
{
	ORIN_EVENTS(Character)
		ORIN_EVENT(Character, EventOnRestart, OnRestart)
		ORIN_EVENT(Character, EventOnCheckpointReached, OnCheckpointReached)
		ORIN_EVENT(Character, EventOnFinishLevel, OnFinishLevel)
	ORIN_EVENTS_END(Character)

	void Character::Init()
	{
		ScriptEntity2D::Init();

		transform.objectType = ObjectType::Object2D;
		transform.transformFlag = MoveXYZ | RectMoveXY | RotateZ;
	}

	void Character::ApplyProperties()
	{
		transform.size = { TILE_SIZE, TILE_SIZE, 0.0f };
	}

	void Character::Play()
	{
		ScriptEntity2D::Play();

		initPos = transform.position;
		initRotation = transform.rotation;
		initScale = transform.scale;
		anim = FindChild<AnimGraph2D>();
		controller = FindChild<KinematicCapsule2D>();
		bodyParts = FindChild<Node2D>("body_parts");

		if (bodyParts)
		{
			auto childs = bodyParts->GetChilds();
			bodyPartsMats.resize(childs.size());

			for (int i = 0; i < childs.size(); i++)
			{
				auto* entity = dynamic_cast<PhysEntity2D*>(childs[i]);				
				entity->SetPhysGroup(PhysGroup::BodyPart);

				bodyPartsMats[i] = childs[i]->GetTransform().GetLocal();
			}
		}

		explotion = FindChild<AnimGraph2D>("explosion");

		ORIN_EVENTS_SUBSCRIBE(Character);
	}

	void Character::Release()
	{
		if (WorldManager::instance)
		{
			MainEventsQueue::Unsubscribe(this);
		}

		ScriptEntity2D::Release();
	}

	bool Character::IsKilled()
	{
		return killed;
	}

	void Character::SnapOnSurface()
	{		
		auto pos = (controller->GetTransform().GetPosition());

		if (auto castRes = Utils::RayCast(transform.position + pos * 0.3f, { 0.f, -1.f, 0.f }, 1000.f, PhysGroup::WorldDoors))
		{			
			controller->SetPosition(castRes->hitPos + pos);
		}
	}

	HitResult Character::Hit(Math::Vector3 hitDir, float damage)
	{
		health -= damage;
		if (health <= 0.f)
		{
			Kill(hitDir, DeathSource::Unknown);
			return HitResult::Lethal;
		}
		return HitResult::Effective;
	}

	const char* Character::GetBodyPartsTrail()
	{
		return "BodyPartTrail";
	}

	void Character::Kill(Math::Vector3 killDir, DeathSource deathSource)
	{
		if (killed)
		{
			return;
		}

		if (controller)
		{
			controller->controller->SetActive(false);
		}

		killed = true;
		health = 0.0f;

		if (explotion)
		{
			explotion->anim.GotoNode("StartExplotion", true);
		}

		if (bodyParts)
		{
			if (anim)
			{
				anim->SetVisiblity(false);
			}

			bodyParts->SetVisiblity(true);

			auto mat = bodyParts->GetTransform().GetGlobal();

			auto childs = bodyParts->GetChilds();

			Utils::Angle angle = Utils::Angle::FromDirection(killDir);

			for (int i = 0; i < childs.size(); i++)
			{
				auto* entity = dynamic_cast<PhysEntity2D*>(childs[i]);

				if (entity)
				{
					auto partMat = bodyPartsMats[i] * mat;
					entity->body.body->SetTransform(partMat);

					auto size = entity->GetTransform().size;
					float k = size.x * size.y / 64.0f;

					//HACK
					if (size.x > 40.0f)
					{
						k *= 1.35f;
					}

					Utils::Angle partAngle = angle + Utils::Angle::Degrees(Utils::GetRandom(-0.5f, 20.5f));
					entity->body.body->AddForceAt(partMat.Pos(), partAngle.ToDirection() * Utils::GetRandom(0.0125f, 0.0125f) * k * 1.55f);
				}
			}

			eastl::vector<Joint2D*> joints;
			bodyParts->FindChilds<Joint2D>(joints);

			int count = 1 + (int)Utils::GetRandom(0.0f, 1.8f);

			for (auto& joint : joints)
			{
				bool active = Utils::GetRandom(0.0f, 1.0f) > 0.5f;

				if (!active)
				{
					if (count > 0)
					{
						count--;
					}
					else
					{
						active = true;
					}
				}

				if (joint->joint)
				{
					joint->joint->SetActive(active);
				}

				if (!active && joint->joint)
				{
					auto mat = joint->GetTransform().GetGlobal();
					auto pos = Sprite::ToPixels(mat.Pos());

					SceneEntity* bodyPart = joint->targetRef.Get();

					if (bodyPart && !StringUtils::IsEqual(bodyPart->GetName(), "Gun"))
					{
						WorldManager::instance->GetBlood()->Spawn(GetBodyPartsTrail(), bodyPart, pos, Math::SafeAtan2(-mat.Vx().y, -mat.Vx().x));
					}

					bodyPart = joint->GetParent();

					if (bodyPart && !StringUtils::IsEqual(bodyPart->GetName(), "Gun"))
					{
						WorldManager::instance->GetBlood()->Spawn(GetBodyPartsTrail(), bodyPart, pos, Math::SafeAtan2(mat.Vx().y, mat.Vx().x));
					}
				}
			}
		}
	}

	void Character::OnRestart(const EventOnRestart& evt)
	{
		if (bodyParts)
		{
			bodyParts->SetVisiblity(false);
		}

		if (controller)
		{
			controller->controller->SetActive(true);
			controller->SetPosition(isCheckpointActive ? checkpointPos : initPos);
		}

		if (anim)
		{
			anim->SetVisiblity(true);
			anim->anim.GotoNode("Idle", true);
		}

		if (explotion)
		{
			explotion->anim.Reset();
		}

		flipped = false;
		killed = false;
		health = maxHealth;
	}

	void Character::UpdateAnimScale()
	{
		if (anim)
		{
			auto scale = anim->GetTransform().scale;
			anim->GetTransform().scale = Math::Vector3(fabs(scale.x) * GetDirection(), fabs(scale.y), 1.0f);
		}
	}
}
