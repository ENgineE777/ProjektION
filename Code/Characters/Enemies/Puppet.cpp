
#include "Puppet.h"
#include "Root/Root.h"
#include "World/WorldManager.h"
#include "World/BloodManager.h"
#include "BubaFoot/BubaFoot.h"

namespace Orin::Overkill
{
	ENTITYREG(SceneEntity, Puppet, "Overkill/Characters", "Puppet")

	META_DATA_DESC(Puppet)
		BASE_SCENE_ENTITY_PROP(Puppet)

		ENUM_PROP(Puppet, moveMode, MoveMode::Right, "Puppet", "moveMode", "moveMode")
			ENUM_ELEM("Right", MoveMode::Right)
			ENUM_ELEM("Left", MoveMode::Left)
		ENUM_END

		ENUM_PROP(Puppet, mode, Mode::Normal, "Puppet", "mode", "mode")
			ENUM_ELEM("Normal", Mode::Normal)
			ENUM_ELEM("TutorialInto", Mode::TutorialInto)
			ENUM_ELEM("CutSceneIntro", Mode::CutSceneIntro)
			ENUM_ELEM("PreCutSceneIntro", Mode::PreCutSceneIntro)
		ENUM_END

		FLOAT_PROP(Puppet, stopAtPathPointsTime, 0.5f, "Puppet", "stopAtPathPointsTime", "Stop at path points time")
		FLOAT_PROP(Puppet, switchLookDirInterval, -1.f, "Puppet", "switchLookDirInterval", "Swtich look direction interval")		

		FLOAT_PROP(Puppet, speedTransitionTime, 0.3f, "Puppet", "speedTransitionTime", "Speed transition time")
		MARK_DISABLED_FOR_INSTANCE()

		FLOAT_PROP(Puppet, startMoveT, 0.f, "Puppet", "startMoveT", "Start speed T after transition")
		MARK_DISABLED_FOR_INSTANCE()

		FLOAT_PROP(Puppet, respawnMaxTime, 1.f, "Puppet", "respawnMaxTime", "Respawn animation time")
		MARK_DISABLED_FOR_INSTANCE()

	META_DATA_DESC_END()


	void Puppet::Play()
	{
		Enemy::Play();
		
		GenericMarker2D* path = FindChild<GenericMarker2D>();

		if (path && path->instances.size() >= 2)
		{
			hasPath = true;

			leftPos = path->instances[0].GetPosition();
			rightPos = path->instances[1].GetPosition();

			if (leftPos.x > rightPos.x)
			{
				leftPos = path->instances[1].GetPosition();
				rightPos = path->instances[0].GetPosition();
			}
		}

		moveLeft = moveMode == MoveMode::Left;
		flipped  = moveLeft;

		breahSound = GetRoot()->GetSounds()->CreateSoundEvent("event:/FatGuy/Breathe");

		if (PhysTriger2D* trigger = FindChild<PhysTriger2D>("WaitRocket"))
		{
			trigger->SetPhysGroup(PhysGroup::LevelObjects);
			trigger->AddDelegate<eastl::function<void(int, SceneEntity*, int)>>("OnContactStart", this,
				[this](int index, SceneEntity* entity, int contactIndex)
				{
					if (Player* player = dynamic_cast<Player*>(entity->GetParent()))
					{
						if (state != State::WaitRocket)
						{
							state = State::WaitRocket;
							timeToWaitRocket = 0.8f;

							anim->anim.ActivateLink("Idle");

							auto* buba = GetScene()->FindEntity<BubaFoot>("bubafoot.prefab");

							if (buba)
							{
								buba->LaunchMissle(Sprite::ToPixels(transform.GetGlobal().Pos()));
							}
						}
					}
				});
		}

		if (PhysTriger2D* trigger = FindChild<PhysTriger2D>("Switch2Normal"))
		{
			trigger->SetPhysGroup(PhysGroup::LevelObjects);
			trigger->AddDelegate<eastl::function<void(int, SceneEntity*, int)>>("OnContactStart", this,
				[this](int index, SceneEntity* entity, int contactIndex)
				{
					if (Player* player = dynamic_cast<Player*>(entity->GetParent()))
					{						
						SwitchToNormal();
					}
				});
		}
	}

	void Puppet::Kill(Math::Vector3 killDir, DeathSource deathSource)
	{
		if (killed || state == State::Respawn)
		{
			return;
		}

		if (breahSound)
		{
			breahSound->Stop();
		}

		if (!isCorpse && mode == Mode::Normal)
		{
			const auto corpsePos = transform.position + controller->GetTransform().GetPosition();

			OnRestart({});

			state = State::Respawn;

			respawnPos  = anim->GetTransform().position;
			respawnTime = 0.f;
			++reswapnCount;

			Puppet *corpse = dynamic_cast<Puppet*>(prefabRef->CreateInstance(GetScene()));
			GetScene()->AddEntity(corpse);

			corpse->isCorpse = true;
			corpse->Play();

			corpse->controller->SetPosition(corpsePos);

			corpse->Kill(killDir, deathSource);

			return;
		}

		Enemy::Kill(killDir, deathSource);

		if (deathSource == DeathSource::InnerDash)
		{
			Utils::PlaySoundEvent("event:/FatGuy/Escape");
		}

		anim->anim.GotoNode("Killed", true);

		const float killAngle = Utils::Angle::FromDirection(killDir).ToDegrees();
		WorldManager::instance->GetBlood()->Spawn("Spray", nullptr, transform.position + Math::Vector3{0.f, 36.f, 0.f}, killAngle);
	}

	void Puppet::OnCheckpointReached(const EventOnCheckpointReached &evt)
	{
		keepDead = killed;
	}

	void Puppet::OnFinishLevel(const EventOnFinishLevel &evt)
	{
		keepDead = false;
	}

	void Puppet::OnRestart(const EventOnRestart &evt)
	{
		if (keepDead && (reswapnCount > 0 || mode == Mode::TutorialInto || mode == Mode::CutSceneIntro))
		{
			killed = true;
			anim->SetVisiblity(false);
			return;
		}

		Enemy::OnRestart(evt);

		auto& tm = anim->GetTransform();
		tm.scale = 1.0f;

		SnapOnSurface();

		moveLeft = moveMode == MoveMode::Left;
		flipped  = moveLeft;		

		switchMoveDirTime = switchLookDirInterval;

		killed = false;

		state = State::Patrolling;

		if (mode == Mode::TutorialInto || mode == Mode::PreCutSceneIntro)
		{
			auto* player = WorldManager::instance->GetPlayer();
			player->SetPuppet(this);

			return;
		}

		needDelete = isCorpse;		
	}

	void Puppet::OnThunderHit(Math::Vector3 hitDir, ThunderCharge *source)
	{
		if (IsImmortal())
		{
			return;
		}

		Kill(hitDir, DeathSource::Unknown);
	}
		
	void Puppet::StartAccept()
	{
		state = State::AcceptMaster;
		anim->anim.ActivateLink("Accept");
	}

	void Puppet::StartObey()
	{
		state = State::ObeyMaster;
		anim->anim.ActivateLink("Idle");

		if (mode != Mode::PreCutSceneIntro && breahSound)
		{
			breahSound->Play();
		}
	}

	void Puppet::MoveMaster(bool left)
	{
		moveMaster = left ? -1.0f : 1.0f;
	}

	void Puppet::TeleportMaster(Math::Vector3 pos)
	{
		if (auto castRes = Utils::RayCast(pos, { 0.f, -1.f, 0.f }, 1000.f, PhysGroup::WorldDoors))
		{
			pos = castRes->hitPos;
		}

		controller->SetPosition(pos + controller->GetTransform().GetPosition());

		SnapOnSurface();
	}

	bool Puppet::IsReadyToObey()
	{
		return state != State::Respawn;
	}

	void Puppet::Update(float dt)
	{
		if (needDelete)
		{
			GetScene()->DeleteEntity(this, true);
			return;
		}

		if (!anim || !controller)
		{
			return;
		}

		if (killed)
		{
			return;
		}		

		if (WorldManager::instance->GetPlayer() && WorldManager::instance->GetFrontendState() != FrontendState::Gameplay)
		{
			return;
		}

		Math::Vector2 moveDir = 0.0f;		

		int worldGroup = PhysGroup::WorldDoors;

		if (breahSound && breahSound->IsPlaying())
		{
			breahSound->Set3DAttributes(transform.position);
		}

		if (state == State::Patrolling)
		{
			auto pos = WorldManager::instance->GetPlayerPos();
			if (hasPath)
			{
				moveT = Utils::Saturate(moveT + dt / speedTransitionTime);

				if (switchMoveDirTime > 0.f)
				{
					switchMoveDirTime -= dt;
					moveT = startMoveT;

					if (switchMoveDirTime <= 0.f)
					{
						switchMoveDirTime = -1.f;

						moveLeft = !moveLeft;
					}
				}
				else
				if ((moveLeft && transform.position.x < leftPos.x) ||
					(!moveLeft && transform.position.x > rightPos.x))
				{
					switchMoveDirTime = stopAtPathPointsTime;
				}

				anim->anim.ActivateLink(switchMoveDirTime > 0.f ? "Idle" : "Walk");
			}
			else
			{
				if (switchLookDirInterval > 0.f)
				{
					switchMoveDirTime -= dt;

					if (switchMoveDirTime <= 0.f)
					{
						switchMoveDirTime = switchLookDirInterval;

						moveLeft = !moveLeft;
					}
				}

				anim->anim.ActivateLink("Idle");
			}

			moveDir.x = (moveLeft ? -moveSpeed : moveSpeed) * Math::EaseInQuad(moveT);

			flipped   = moveLeft;

			if (switchMoveDirTime <= 0.0f)
			{
				footstepTimer += dt;

				if (footstepTimer >= footstepsInterval)
				{
					footstepTimer -= footstepsInterval;

					Utils::PlaySoundEvent("event:/FatGuy/Footsteps", &transform.position);
				}
			}
		}
		else
		if (state == State::AcceptMaster)
		{
		}
		else	
		if (state == State::ObeyMaster)
		{
			if (fabs(moveMaster) > 0.0f && mode != Mode::PreCutSceneIntro)
			{
				moveDir.x = (mode == Mode::CutSceneIntro ? moveSpeed : moveMasterSpeed) * moveMaster;
				flipped = moveDir.x < 0.0f;

				worldGroup |= PhysGroup::LevelObjects;

				footstepTimer += dt;

				if (footstepTimer >= footstepsInterval)
				{
					footstepTimer -= footstepsInterval;

					Utils::PlaySoundEvent("event:/FatGuy/Footsteps", &transform.position);
				}
			}
			
			anim->anim.ActivateLink(fabs(moveDir.x) > 0.0f ? "Walk" : "Idle");

			moveMaster = 0.0f;
		}
		else
		if (state == State::Respawn)
		{
			anim->SetVisiblity(true);
			anim->anim.GotoNode("Idle", true);

			const float posT   = Utils::Saturate(respawnTime / respawnMaxTime);
			const float scaleT = Utils::Saturate(0.05f + Utils::Saturate((respawnTime - 0.9f * respawnMaxTime) / respawnMaxTime));

			respawnTime += dt;

			const float spawnHeight = 8.f * TILE_SIZE;

			auto &tm = anim->GetTransform();
			tm.scale    = Utils::Vector::Vyz(tm.scale, Math::EaseOutBack(scaleT));
			tm.scale    = Utils::Vector::xVz(tm.scale, 1.f + .5f * (1.f - Math::EaseOutBack(scaleT)));
			tm.position = Utils::Vector::xVz(tm.position, respawnPos.y + spawnHeight * (1.f - Math::EaseOutQuad(posT)));

			if (scaleT >= 1.f)
			{
				state = State::Patrolling;
			}
		}
		else
		if (state == State::WaitRocket)
		{
			moveMaster = 0.0f;
		}

		UpdateAnimScale();		

		if (hasPath || state == State::ObeyMaster)
		{
			controller->Move(moveDir, 3);
		}
	}

	void Puppet::Release()
	{
		RELEASE(breahSound)

		Enemy::Release();
	}
}