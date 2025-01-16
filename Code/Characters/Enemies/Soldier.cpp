
#include "Soldier.h"
#include "Root/Root.h"
#include "World/WorldManager.h"
#include "World/BloodManager.h"
#include "Effects/VFXEmitter.h"
#include "Objects/Throwable/ThunderCharge.h"

namespace Orin::Overkill
{
	ENTITYREG(SceneEntity, Soldier, "Overkill/Characters", "Soldier")

		META_DATA_DESC(Soldier)
		BASE_SCENE_ENTITY_PROP(Soldier)

		ENUM_PROP(Soldier, moveMode, MoveMode::Right, "Soldier", "moveMode", "moveMode")
		ENUM_ELEM("Right", MoveMode::Right)
		ENUM_ELEM("Left", MoveMode::Left)
		ENUM_END
		
		INT_PROP(Soldier, killScore, 100, "Soldier", "killScore", "Kill Score")
		MARK_DISABLED_FOR_INSTANCE()

		FLOAT_PROP(Soldier, stopAtPathPointsTime, 0.5f, "Soldier", "stopAtPathPointsTime", "Stop at path points time")
		FLOAT_PROP(Soldier, switchLookDirInterval, -1.f, "Soldier", "switchLookDirInterval", "Swtich look direction interval")

		FLOAT_PROP(Soldier, speedTransitionTime, 0.3f, "Soldier", "speedTransitionTime", "Speed transition time")
		MARK_DISABLED_FOR_INSTANCE()

		FLOAT_PROP(Soldier, startMoveT, 0.f, "Soldier", "startMoveT", "Start speed T after transition")
		MARK_DISABLED_FOR_INSTANCE()

		FLOAT_PROP(Soldier, maxHealth, 1.f, "Soldier", "maxHealth", "Health")
		MARK_DISABLED_FOR_INSTANCE()

		FLOAT_PROP(Soldier, hitReactionDuration, 0.5f, "Soldier", "hitReactionDuration", "Hit reaction duration")
		MARK_DISABLED_FOR_INSTANCE()

		FLOAT_PROP(Soldier, keepPlayerVisibleMaxTime, 0.25f, "Soldier", "keepPlayerVisibleMaxTime", "Keep player is sight during time")
		MARK_DISABLED_FOR_INSTANCE()

		FLOAT_PROP(Soldier, alwaysViewRadius, 36.0f, "Soldier", "alwaysViewRadius", "Always View Radius")
		MARK_DISABLED_FOR_INSTANCE()

		VECTOR2_PROP(Soldier, alwaysViewBox, Math::Vector2(72.0f, 36.0f), "Soldier", "alwaysViewBox")
		MARK_DISABLED_FOR_INSTANCE()

		FLOAT_PROP(Soldier, reactionToPlayerFromBehindMaxTime, 0.1f, "Soldier", "reactionToPlayerFromBehindMaxTime", "React to the player from behind during time")
		MARK_DISABLED_FOR_INSTANCE()

		FLOAT_PROP(Soldier, footstepsInterval, .45f, "Sound", "footstepsInterval", "Footsteps interval")
		MARK_DISABLED_FOR_INSTANCE()

		FLOAT_PROP(Soldier, maxTimeToTriggeredSound, .25f, "Sound", "timeToTriggeredSound", "Time to triggered sound")
		MARK_DISABLED_FOR_INSTANCE()

		META_DATA_DESC_END()

		Math::Vector3 Soldier::GetAimPos()
	{
		if (shootNode)
		{
			return Sprite::ToPixels(shootNode->GetTransform().GetGlobal().Pos());
		}
		return transform.position + Math::Vector3{ 0.f, 36.f, 0.f };
	}

	bool Soldier::CanSeePlayer()
	{
		auto pos = transform.position;

		auto playerPos = WorldManager::instance->GetPlayerPos();
		auto halfScreenSize = Sprite::GetHalfScreenSize();
		halfScreenSize.x = Sprite::GetPixelsHeight() * 0.5f * 16.0f / 9.0f - 36.0f * 1.75f;

		float upMargin = 50.0f;
		float downMargin = 5.0f;

		if (playerPos.x - halfScreenSize.x > pos.x || pos.x > playerPos.x + halfScreenSize.x ||
			playerPos.y - halfScreenSize.y + downMargin > pos.y || pos.y > playerPos.y + halfScreenSize.y - upMargin)
		{
			return false;
		}

		return true;
	}

	bool Soldier::IsPlayerVisibleInSector()
	{
		if (!CanSeePlayer() || !WorldManager::instance->IsPlayerCanBeDetectd())
		{
			return false;
		}

		const auto aimPos = GetAimPos();
		const auto playerPos = WorldManager::instance->GetPlayerPos();
		const auto distToPlayer = playerPos - aimPos;
		const float distX = GetDirection() * distToPlayer.x;

		// Visible inside the box in front of the soldier
		if (distX > 0.f && distX < alwaysViewBox.x && std::abs(distToPlayer.y) < alwaysViewBox.y)
		{
			return !Utils::RayHitLine(GetAimPos(), playerPos, PhysGroup::WorldDoors);
		}

		return WorldManager::instance->IsPlayerVisibleInSector(aimPos, viewDistance, aimingAngle.ToSignedDegrees(), 45.0f, alwaysViewRadius);
	}

	Utils::Angle Soldier::GetAngleToPlayer()
	{
		return WorldManager::instance->GetAngleToPlayer(GetAimPos());
	}

	float Soldier::GetDirectionToPlayer()
	{
		return WorldManager::instance->GetDirectionToPlayer(transform.position + Math::Vector3{ 0.f, 36.f, 0.f });
	}

	void Soldier::Init()
	{
		Enemy::Init();

		Tasks(false)->AddTask(0, this, (Object::Delegate)&Soldier::EditorDraw);
	}

	void Soldier::Release()
	{
		Tasks(false)->DelTask(0, this);

		Enemy::Release();
	}

	void Soldier::Play()
	{
		Enemy::Play();

		arms = FindChild<AnimGraph2D>("arms");
		shootNode = FindChild<Node2D>("shoot");
		redDotNode = FindChild<Node2D>("red_dot");
		torse = FindChild<AnimGraph2D>("torse");
		shootFlash = FindChild<PointLight2D>("flash");
		shootMuzzle = FindChild<SpriteEntity>("muzzle");
		lightBeam = FindChild<SpriteEntity>("light_beam");
		shells = FindChild<VFXEmitter>("shells");

		if (lightBeam)
		{
			lightBeamPos = lightBeam->GetTransform().position;
		}

		if (shootFlash)
		{
			shootFlash->SetVisiblity(false);
		}

		if (shootMuzzle)
		{
			shootMuzzle->SetVisiblity(false);
		}

		if (shells)
		{
			shells->ResetAndStop();
		}

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
		flipped = moveLeft;
		aimingAngle = Utils::Angle::Degrees(moveLeft ? 180.f : 0.f);
		tragetAimingAngle = aimingAngle;

		shield = FindChild<Shield>("shield");
		if (shield)
		{
			shield->enemyOwner = this;

			shieldPos = arms->GetTransform().GetGlobal().MulNormal(shield->GetTransform().position);
			shieldPos.x *= GetDirection();

			shieldSprite = shield->FindChild<SpriteEntity>();
		}
	}

	HitResult Soldier::Hit(Math::Vector3 hitDir, float damage)
	{
		const bool isKilled = Enemy::Hit(hitDir, damage) == HitResult::Lethal;
		if (!isKilled)
		{
			WorldManager::instance->ChangeAlarmCount(true);

			state = State::HitReaction;
			hitReactionTime = hitReactionDuration;
			arms->SetVisiblity(false);
			anim->anim.GotoNode("Hit", true);

			if (torse)
			{
				torse->SetVisiblity(false);
			}
		}
		return isKilled ? HitResult::Lethal : HitResult::Effective;
	}

	void Soldier::Kill(Math::Vector3 killDir, DeathSource deathSource)
	{
		if (killed)
		{
			return;
		}

		if (state == State::Wondering || state == State::Attacking || state == State::HitReaction)
		{
			WorldManager::instance->ChangeAlarmCount(false);
		}

		Enemy::Kill(killDir, deathSource);

		anim->anim.GotoNode("Killed", true);
		arms->SetVisiblity(false);

		if (torse)
		{
			torse->SetVisiblity(false);
		}

		const float killAngle = Utils::Angle::FromDirection(killDir).ToDegrees();
		WorldManager::instance->GetBlood()->Spawn("Spray", nullptr, transform.position + Math::Vector3{ 0.f, 36.f, 0.f }, killAngle);

		Utils::PlaySoundEvent("event:/Soldier/Death", &transform.position);

		if (shield)
		{
			shield->SetVisiblity(false);
			shield->body.body->SetActive(false);
		}
	}

	void Soldier::OnRestart(const EventOnRestart& evt)
	{
		if (keepDead)
		{
			return;
		}

		Enemy::OnRestart(evt);

		arms->SetVisiblity(true);

		if (torse)
		{
			torse->SetVisiblity(true);
		}

		moveLeft = moveMode == MoveMode::Left;
		flipped = moveLeft;
		aimingAngle = Utils::Angle::Degrees(moveLeft ? 180.f : 0.f);
		tragetAimingAngle = aimingAngle;

		switchMoveDirTime = switchLookDirInterval;

		killed = false;
		timeToAttackPlayer = -1.0f;
		timeToAttackPlayerMult = 1.f;
		moveT = 0.f;
		shootEffectTime = -1.f;
		keepPlayerVisibleTime = 0.f;
		reactionToPlayerFromBehindTime = 0.f;
		totalTime = 0.f;
		nextFootstepAtTime = 0.f;
		timeToTriggeredSound = -1.f;

		state = State::Patrolling;

		curShootCount = 0;

		if (shootFlash)
		{
			shootFlash->SetVisiblity(false);
		}

		if (shootMuzzle)
		{
			shootMuzzle->SetVisiblity(false);
		}

		if (shells)
		{
			shells->ResetAndStop();
		}

		if (shield)
		{
			auto& shieldTm = shield->GetTransform();

			const auto pos = Utils::Vector::Vyz(shieldPos, shieldPos.x * GetDirection());
			shieldTm.position = pos;

			Math::Matrix tm;
			tm.Pos() = shieldTm.GetGlobal().Pos();
			shield->body.body->SetTransform(tm);

			shield->SetVisiblity(true);
			shield->body.body->SetActive(true);
		}

		SnapOnSurface();
	}

	void Soldier::OnCheckpointReached(const EventOnCheckpointReached& evt)
	{
		keepDead = killed;
	}

	void Soldier::OnFinishLevel(const EventOnFinishLevel& evt)
	{
		keepDead = false;
	}

	void Soldier::OnThunderHit(Math::Vector3 hitDir, ThunderCharge* source)
	{
		if (IsImmortal())
		{
			return;
		}

		const HitResult hitRes = Hit(hitDir, 1.f);
		if (hitRes != HitResult::Nothing)
		{
			Utils::AddCameraShake(hitRes == HitResult::Lethal ? 0.5f : 0.25f);
			source->OnThunderDisposed();
		}
	}

	void Soldier::DrawRedDot()
	{
		const Math::Vector3 shootPos = redDotNode ? Sprite::ToPixels(redDotNode->GetTransform().GetGlobal().Pos()) : GetAimPos();
		Math::Vector3 shootDir = aimingAngle.ToDirection();
		shootDir.Normalize();

		Math::Vector3 shootToPos;
		if (auto castRes = Utils::RayCast(shootPos, shootDir, 1500.f, PhysGroup::WorldDoorsHitBox))
		{
			shootToPos = castRes->hitPos;
		}
		else
		{
			shootToPos = shootPos + shootDir * 1500.f;
		}

		WorldManager::instance->AddRedDot(shootPos, shootToPos, 1.0f - timeToAttackPlayer / GetTimeToAttack());
	}

	void Soldier::VisualizeAttackTime()
	{
		Color lightBeamNormal = Color(1.0f, 1.0f, 0.55f, 0.55f);
		Color lightBeamAttacking = Color(1.0f, 0.29f, 0.29f, 0.85f);

		if (lightBeam)
		{
			lightBeam->GetTransform().position = lightBeamPos - Math::Vector3(arms->anim.GetFrameOffset());
		}

		if (state == State::Patrolling || state == State::PrepareToPatrolling)
		{
			if (lightBeam)
			{
				lightBeam->color = lightBeamNormal;
			}
		}
		else
			if (state == State::Wondering)
			{
				if (lightBeam)
				{
					lightBeam->color.Lerp(lightBeamNormal, lightBeamAttacking, 1.0f - timeToAttackPlayer);
				}
				DrawRedDot();
			}
			else
			{
				if (lightBeam)
				{
					lightBeam->color = lightBeamAttacking;
				}
				DrawRedDot();
			}
	}

	void Soldier::UpdateAnimScale()
	{
		Enemy::UpdateAnimScale();

		if (arms)
		{
			auto scale = arms->GetTransform().scale;
			arms->GetTransform().scale = Math::Vector3(fabs(scale.x) * GetDirection(), fabs(scale.y), 1.0f);

			auto pos = arms->GetTransform().position;
			pos.x = -fabs(pos.x) * GetDirection();

			arms->GetTransform().position = pos;
		}

		if (torse)
		{
			auto scale = torse->GetTransform().scale;
			torse->GetTransform().scale = Math::Vector3(fabs(scale.x) * GetDirection(), fabs(scale.y), 1.0f);
		}
	}

	void Soldier::OnShieldTouch()
	{
		timeToAttackPlayerMult = 10.f;
	}

	float Soldier::GetTimeToAttack()
	{
		return 1.0f * WorldManager::instance->GetDifficulty()->reactionMul;
	}

	void Soldier::Update(float dt)
	{
		if (WorldManager::instance->GetPlayer() && WorldManager::instance->GetFrontendState() != FrontendState::Gameplay)
		{
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

		totalTime += dt;

		Math::Vector2 moveDir = 0.0f;

		if (state != State::Attacking)
		{
			if (shootFlash)
			{
				shootFlash->SetVisiblity(false);
			}

			if (shootMuzzle)
			{
				shootMuzzle->SetVisiblity(false);
			}

			if (shells)
			{
				shells->Stop();
			}
		}	

		bool playerVisibile = IsPlayerVisibleInSector();
		if (playerVisibile || !Utils::RayHitLine(GetAimPos(), WorldManager::instance->GetPlayerPos(), PhysGroup::WorldDoors))
		{
			lastPlayerAngle = GetAngleToPlayer();
			lastPlayerSnapshot = WorldManager::instance->GetPlayerSnapshot();
		}

		// The player is visible, but he is behind the soldier
		if (playerVisibile && GetDirectionToPlayer() * GetDirection() < 0.f)
		{
			reactionToPlayerFromBehindTime += dt;
			if (reactionToPlayerFromBehindTime < reactionToPlayerFromBehindMaxTime)
			{
				playerVisibile = false;
			}
		}
		else
		{
			reactionToPlayerFromBehindTime = 0.f;
		}

		if (playerVisibile)
		{
			keepPlayerVisibleTime = keepPlayerVisibleMaxTime;
		}

		if (keepPlayerVisibleTime > 0.f)
		{
			keepPlayerVisibleTime -= dt;
			playerVisibile = true;
		}

		if (state == State::PrepareToPatrolling)
		{
			timeToAttackPlayerMult = 1.f;

			if (aimingAngle.IsEqual(tragetAimingAngle))
			{
				state = State::Patrolling;
			}

			if (playerVisibile)
			{
				state = State::Wondering;
				timeToAttackPlayer = GetTimeToAttack();

				tragetAimingAngle = lastPlayerAngle;
			}
		}
		else
		if (state == State::Patrolling)
		{
			timeToAttackPlayerMult = 1.f;

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
						aimingAngle = Utils::Angle::Degrees(moveLeft ? 180.f : 0.f);
						tragetAimingAngle = aimingAngle;
					}
				}
				else
				if ((moveLeft && transform.position.x < leftPos.x) ||
					(!moveLeft && transform.position.x > rightPos.x))
				{
					switchMoveDirTime = stopAtPathPointsTime;
				}

				anim->anim.ActivateLink(switchMoveDirTime > 0.f ? "Idle" : "Walk");
				arms->anim.ActivateLink(switchMoveDirTime > 0.f ? "Idle" : "Walk");

				if (torse)
				{
					torse->anim.GotoNode(switchMoveDirTime > 0.f ? "Idle" : "Walk", false);
				}

				if (switchMoveDirTime <= 0.f && totalTime >= nextFootstepAtTime)
				{
					nextFootstepAtTime = totalTime + footstepsInterval;

					Utils::PlaySoundEvent("event:/Soldier/Footsteps", &transform.position);
				}
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
						aimingAngle = Utils::Angle::Degrees(moveLeft ? 180.f : 0.f);
						tragetAimingAngle = aimingAngle;
					}
				}

				anim->anim.ActivateLink("Idle");
				arms->anim.ActivateLink("Idle");

				if (torse)
				{
					torse->anim.GotoNode("Idle", false);
				}
			}

			moveDir.x = (moveLeft ? -moveSpeed : moveSpeed) * Math::EaseInQuad(moveT);
			flipped   = moveLeft;

			if (playerVisibile)
			{
				timeToTriggeredSound = maxTimeToTriggeredSound;

				state = State::Wondering;
				WorldManager::instance->ChangeAlarmCount(true);
				timeToAttackPlayer = GetTimeToAttack();

				tragetAimingAngle = lastPlayerAngle;
				aimingAngle = tragetAimingAngle;
			}
		}
		else
		if (state == State::Wondering)
		{
			if (playerVisibile)
			{
				timeToAttackPlayer -= dt * timeToAttackPlayerMult;

				flipped  = GetDirectionToPlayer() < 0.0f;
				tragetAimingAngle = lastPlayerAngle;

				if (timeToAttackPlayer < 0.0f)
				{
					state = State::Attacking;
					shootTime = shootTimeDelay;
					shootsBeforeStop = shootsMaxBeforeStop;
					curShootCount = 0;
				}
			}
			else
			{
				timeToAttackPlayer += dt * 0.66f;

				if (timeToAttackPlayer > GetTimeToAttack())
				{
					state = State::PrepareToPatrolling;
					WorldManager::instance->ChangeAlarmCount(false);
					moveLeft = flipped;
					tragetAimingAngle = Utils::Angle::Degrees(moveLeft ? 180.f : 0.f);
				}
			}
		}
		else
		if (state == State::Attacking)
		{
			flipped = GetDirectionToPlayer() < 0.0f;

			tragetAimingAngle = lastPlayerAngle;

			shootTime -= dt;

			const bool isShooting = shootTime < 0.01f;
			if (isShooting)
			{
				if (!playerVisibile)
				{
					shootsBeforeStop--;

					if (shootsBeforeStop == 0)
					{
						state = State::Wondering;
						timeToAttackPlayer = 0.0f;
					}
				}
				else
				{
					shootsBeforeStop = shootsMaxBeforeStop;
				}

				shootTime = shootTimeDelay;
				shootEffectTime = shootEffectMaxTime;

				const Math::Vector3 shootPos = GetAimPos();
				const Math::Vector3 shootDir = WorldManager::instance->CalcShootingDirToPlayer(shootPos, curShootCount, lastPlayerSnapshot);
				curShootCount++;

				WorldManager::instance->AddBulletProjectile(shootPos, shootDir * WorldManager::instance->maxBulletSpeed, 1500.f, false);
				
				arms->anim.ActivateLink("Shoot");

				Utils::PlaySoundEvent("event:/Soldier/Shooting", &transform.position);
			}

			const float isShootEffectVisible = shootEffectTime > 0.f;
			shootEffectTime -= dt;

			if (shootFlash)
			{
				shootFlash->SetVisiblity(isShootEffectVisible);
			}

			if (shootMuzzle)
			{
				shootMuzzle->SetVisiblity(isShootEffectVisible);
			}

			if (shells && isShooting)
			{
				shells->Start();
			}
		}
		else
		if (state == State::HitReaction)
		{
			hitReactionTime -= dt;
			if (hitReactionTime <= 0.f)
			{
				anim->anim.GotoNode("Idle", true);
				arms->SetVisiblity(true);
				arms->anim.GotoNode("Idle", true);

				if (torse)
				{
					torse->SetVisiblity(true);
				}

				state = State::Attacking;
				shootTime = shootTimeDelay;
				shootsBeforeStop = shootsMaxBeforeStop;
				curShootCount = 0;

				aimingAngle = lastPlayerAngle;
				tragetAimingAngle = aimingAngle;
			}
		}

		VisualizeAttackTime();

		aimingAngle = aimingAngle.MoveTo(tragetAimingAngle, Utils::ToRadian(400.0f) * dt);

		if (moveDir.Length2() < 0.1f)
		{
			anim->anim.ActivateLink("Idle");
			arms->anim.ActivateLink("Idle");
		}

		UpdateAnimScale();

		if (shieldSprite && arms)
		{
			shieldSprite->GetTransform().position = -Math::Vector3(arms->anim.GetFrameOffset());
		}

		Math::Vector3 rotation  = arms->GetTransform().rotation;
		Utils::Angle worldAngle = Utils::AdjustAngleByDirection(aimingAngle, GetDirection());

		float armsAngle = worldAngle.ToSignedDegrees();
		arms->GetTransform().rotation = Utils::Vector::xyV(rotation, worldAngle.ToSignedDegrees());

		if (torse && state != State::Patrolling)
		{
			if (flipped)
			{
				armsAngle = -armsAngle;
			}

			const char* torseNode = "Torse_90";

			if (armsAngle < -22.5f)
			{
				torseNode = "Torse_315";
			}
			else
			if (armsAngle < 22.5f)
			{
				torseNode = "Torse_0";
			}
			else
			if (armsAngle < 67.5f)
			{
				torseNode = "Torse_45";
			}

			torse->anim.GotoNode(torseNode, false);
		}

		if (timeToTriggeredSound > 0.f)
		{
			timeToTriggeredSound -= dt;
			if (timeToTriggeredSound <= 0.f)
			{
				timeToTriggeredSound = -1.f;

				Utils::PlaySoundEvent("event:/Soldier/Triggered", &transform.position);
			}
		}

		if (shield)
		{
			Math::Matrix physTm = shield->GetTransform().GetGlobal();
			if (GetDirection() < 0.f)
			{
				physTm.Vx() *= -1.f;
			}

			shield->body.body->SetTransform(physTm);
		}

		if (hasPath)
		{
			controller->Move(moveDir, 3);
		}
	}

	void Soldier::EditorDraw(float dt)
	{
		if (GetScene()->IsPlaying() || !IsVisible())
		{
			return;
		}

		Utils::DebugArrow(GetAimPos(), GetAimPos() + Math::Vector3{36.f, 0.f, 0.f} * (moveMode == MoveMode::Left ? -1.f : 1.f), COLOR_YELLOW_A(0.55f));
	}
}