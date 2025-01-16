
#include "Thing.h"
#include "Characters/Enemies/Enemy.h"
#include "Characters/Enemies/Soldier.h"
#include "Root/Root.h"
#include "World/WorldManager.h"
#include "World/BloodManager.h"
#include "Common/Utils.h"
#include "Common/Events.h"
#include "Objects/Shield.h"
#include "Objects/TunnelDoor.h"
#include "Objects/InstantPlayerKiller.h"
#include "Objects/CutSceneOutro.h"

#define ORIN_CONTROLS GetRoot()->GetControls()
#define ORIN_CONTROLS_JUST_PRESSED(alias) ORIN_CONTROLS->GetAliasState(alias, AliasAction::JustPressed)
#define ORIN_CONTROLS_PRESSED(alias) ORIN_CONTROLS->GetAliasState(alias, AliasAction::Pressed)
#define ORIN_CONTROLS_VALUE(alias) ORIN_CONTROLS->GetAliasValue(alias, false)
#define ORIN_CONTROLS_DELTA(alias) ORIN_CONTROLS->GetAliasValue(alias, true)

namespace Orin::Overkill
{
	ENTITYREG(SceneEntity, Thing, "Overkill/Characters", "Thing")

		META_DATA_DESC(Thing)
		BASE_SCENE_ENTITY_PROP(Thing)
		FLOAT_PROP(Thing, innerCornerBeginTransitionDist, 24.f, "Movement", "innerCornerBeginTransitionDist", "Distance to start auto transition in inner corners")
		MARK_DISABLED_FOR_INSTANCE()

		FLOAT_PROP(Thing, outerCornerBeginTransitionDist, 24.f, "Movement", "outerCornerBeginTransitionDist", "Distance to start auto transition in outer corners")
		MARK_DISABLED_FOR_INSTANCE()

		FLOAT_PROP(Thing, cornerTransitionByKeyDist, 32.f, "Movement", "cornerTransitionByKeyDist", "Start corner transition by direction key")
		MARK_DISABLED_FOR_INSTANCE()

		FLOAT_PROP(Thing, cornerLongPress, 0.15f, "Movement", "cornerLongPress", "Auto transition only by long press")
		MARK_DISABLED_FOR_INSTANCE()

		FLOAT_PROP(Thing, detachSurfaceLongPress, 0.15f, "Movement", "detachSurfaceLongPress", "Detach only by long press")
		MARK_DISABLED_FOR_INSTANCE()

		FLOAT_PROP(Thing, moveAlongSurfaceTransitionDist, 16.f, "Movement", "moveAlongSurfaceTransitionDist", "Move along surface after inner transition")
		MARK_DISABLED_FOR_INSTANCE()

		FLOAT_PROP(Thing, moveSpeed, 197.f, "Movement", "moveSpeed", "Move speed on ground and ceiling")
		MARK_DISABLED_FOR_INSTANCE()

		FLOAT_PROP(Thing, wallMoveSpeed, 141.f, "Movement", "wallMoveSpeed", "Move speed on walls")
		MARK_DISABLED_FOR_INSTANCE()

		FLOAT_PROP(Thing, airMoveSpeed, 100.f, "Movement", "airMoveSpeed", "Move speed in air")
		MARK_DISABLED_FOR_INSTANCE()

		FLOAT_PROP(Thing, tunnelActivationDist, 10.f, "Tunnel", "tunnelActivationDist", "Distance to activate a tunnel")
		MARK_DISABLED_FOR_INSTANCE()

		FLOAT_PROP(Thing, tunnelBufferingTime, 0.1f, "Tunnel", "tunnelBufferingTime", "Time to memorize pressed tunnel button before landing and to perform the right after")
		MARK_DISABLED_FOR_INSTANCE()

		FLOAT_PROP(Thing, throwBufferingTime, 0.25f, "Throw", "throwBufferingTime", "Time to memorize pressed throw button during tunnel transition")
		MARK_DISABLED_FOR_INSTANCE()

		FLOAT_PROP(Thing, dashCoyoteTime, 0.1f, "Dash", "dashCoyoteTime", "After detach on the surcafe we have this time to still perform dash movement")
		MARK_DISABLED_FOR_INSTANCE()

		FLOAT_PROP(Thing, dashBufferingTime, 0.1f, "Dash", "dashBufferingTime", "Time to memorize pressed dash button before landing and to perform the right after")
		MARK_DISABLED_FOR_INSTANCE()

		FLOAT_PROP(Thing, dashThresholdDegress, 10.f, "Dash", "dashThresholdDegress", "Angular threshold to perform dash")
		MARK_DISABLED_FOR_INSTANCE()

		FLOAT_PROP(Thing, dashMaxDistance, 4.f * TILE_SIZE, "Dash", "dashMaxDistance", "Max dash distance")
		MARK_DISABLED_FOR_INSTANCE()

		FLOAT_PROP(Thing, dashStartSpeed, 563.f, "Dash", "dashStartSpeed", "Dash start speed")
		MARK_DISABLED_FOR_INSTANCE()

		FLOAT_PROP(Thing, dashEndSpeed, 197.f, "Dash", "dashEndSpeed", "Dash end speed")
		MARK_DISABLED_FOR_INSTANCE()		

		COLOR_PROP(Thing, dashSuccessColor, Color(0.033755f, 1.f, 0.718688f, 1.f), "UI", "dashSuccessColor")
		MARK_DISABLED_FOR_INSTANCE()

		COLOR_PROP(Thing, dashFailColor, Color(.8f, .8f, .8f, .8f), "UI", "dashFailColor")
		MARK_DISABLED_FOR_INSTANCE()

		FLOAT_PROP(Thing, footstepsInterval, .1f, "Sound", "footstepsInterval", "Footsteps interval")
		MARK_DISABLED_FOR_INSTANCE()

		BOOL_PROP(Thing, debugDash, false, "Dash", "debugDash", "Debug Dash")
		MARK_DISABLED_FOR_INSTANCE()

	META_DATA_DESC_END()

	ORIN_EVENTS(Thing)
		ORIN_EVENT(Thing, EventOnFinishLevel, OnFinishLevel)
		ORIN_EVENT(Thing, CmdTeleportState, TeleportState)
	ORIN_EVENTS_END(Thing)

	void Thing::Init()
	{
		Player::Init();

		Tasks(true)->AddTask(10, this, (Object::Delegate)&Thing::Draw);

		aliasMoveHorz = ORIN_CONTROLS->GetAlias("Hero.MOVE_HORZ");
		aliasMoveVert = ORIN_CONTROLS->GetAlias("Hero.MOVE_VERT");
		aliasAimHorz = ORIN_CONTROLS->GetAlias("Hero.AIM_HORZ");
		aliasAimVert = ORIN_CONTROLS->GetAlias("Hero.AIM_VERT");
		aliasAimMainHorz = ORIN_CONTROLS->GetAlias("Hero.AIM_MAIN_HORZ");
		aliasAimMainVert = ORIN_CONTROLS->GetAlias("Hero.AIM_MAIN_VERT");
		aliasDash = ORIN_CONTROLS->GetAlias("Hero.DASH");
		aliasUse = ORIN_CONTROLS->GetAlias("Hero.USE");
		aliasDrop = ORIN_CONTROLS->GetAlias("Hero.DROP");
		aliasShoot = ORIN_CONTROLS->GetAlias("Hero.SHOOT");
		aliasCursorX = ORIN_CONTROLS->GetAlias("Hero.CURSOR_X");
		aliasCursorY = ORIN_CONTROLS->GetAlias("Hero.CURSOR_Y");
	}

	void Thing::Play()
	{
		Player::Play();

		if (controller != nullptr)
		{
			initialCapsuleOffset = controller->GetTransform().GetLocal().Pos();
		}

		footstepsSound = GetRoot()->GetSounds()->CreateSoundEvent("event:/hero/Footsteps");

		ORIN_EVENTS_SUBSCRIBE(Thing);		

		hitBox = FindChild<PhysEntity2D>("hitBox");
		dashTrail = FindChild<SpriteEntity>("dash_trail");

		if (dashTrail)
		{
			dashTrailEnd = dashTrail->FindChild<SpriteEntity>();
		}

		dashTrailAttack = FindChild<SpriteEntity>("dash_trail_attack");

		if (dashTrailAttack)
		{
			dashTrailAttackEnd = dashTrailAttack->FindChild<SpriteEntity>();
		}

		dashSlash = FindChild<SpriteEntity>("dash_slash");
		dashSlash2 = FindChild<SpriteEntity>("dash_slash2");

		const float minTransitionDist = 0.5f * controller->GetHeight() + controller->GetRadius() + 1.f;
		innerCornerBeginTransitionDist = std::max(innerCornerBeginTransitionDist, minTransitionDist);
		surfaceMargin = controller->GetRadius() + 1.f;

		if (PhysTriger2D* trigger = FindChild<PhysTriger2D>("HackToBeContinued"))
		{
			trigger->SetPhysGroup(PhysGroup::LevelObjects);
			trigger->AddDelegate<eastl::function<void(int, SceneEntity*, int)>>("OnContactStart", this,
				[this](int index, SceneEntity* entity, int contactIndex)
				{
					if (Player* player = dynamic_cast<Player*>(entity->GetParent()))
					{
						delayedOutro = 3.5f;						
					}
				});
		}

		// FileInMemory dashLog;
		// if (dashLog.Load("in_dash.txt"))
		// {
		// 	auto strToFloat = [](const char *str) -> float
		// 	{
		// 		float x;
		// 		sscanf_s(str, "%f", &x);
		// 		return x;
		// 	};

		// 	auto strToVec2 = [](const char *str) -> Math::Vector2
		// 	{
		// 		float x, y;
		// 		sscanf_s(str, " (%f, %f)", &x, &y);
		// 		return Math::Vector2{x, y};
		// 	};

		// 	DashRecord record;
		// 	int idx = 0;

		// 	const char* cur = (const char*)dashLog.GetData();
		// 	const char* end = cur + dashLog.GetSize();
		// 	const char* ptr = cur;
		// 	for (; cur != end; ++cur)
		// 	{
		// 		if (*cur == ';' || *cur == '\n')
		// 		{
		// 			if (idx == 0)
		// 			{
		// 				record.scene = eastl::string(ptr, (ptrdiff_t)(cur - ptr));
		// 			}
		// 			else if (idx == 1)
		// 			{
		// 				record.aimDir = strToVec2(ptr);						
		// 			}
		// 			else if (idx == 2)
		// 			{
		// 				record.aimDirUnclamped = strToVec2(ptr);
		// 			}
		// 			else if (idx == 3)
		// 			{
		// 				record.dashStartPos = strToVec2(ptr);
		// 			}
		// 			else if (idx == 4)
		// 			{
		// 				record.dashEndPos = strToVec2(ptr);
		// 			}
		// 			else if (idx == 5)
		// 			{
		// 				record.dashLength = strToFloat(ptr);
		// 			}
		// 			else if (idx == 6)
		// 			{
		// 				record.dashMeta = eastl::string(ptr, (ptrdiff_t)(cur - ptr));
		// 			}

		// 			idx = (idx + 1) % 7;
		// 			ptr = cur + 1;

		// 			if (idx == 0)
		// 			{
		// 				record.id = (int)records.size() + 1;
		// 				records.push_back(record);
		// 				record = DashRecord{};
		// 			}
		// 		}
		// 	}
		// }
	}

	void Thing::OnRestart(const EventOnRestart& evt)
	{
		Player::OnRestart(evt);

		SetState(State::Normal, -1.0f);

		onWall = false;
		onCeiling = false;
		onGround = false;
		anim->SetVisiblity(true);

		upVector = { 0.f, 1.f, 0.f };
		surfaceNormal = { 0.f, 1.f, 0.f };

		totalTime = 0.f;
		lastDashPressedAtTime = -1.f;
		lastTunnelPressedAtTime = -1.f;
		lastThrowPressedAtTime = -1.f;
		detachPressedTime = 0.f;

		nextFootstepAtTime = 0.f;

		killAssistanceCount = 0;
		safeAssistanceCount = 0;

		throwableDesc2Pickup = nullptr;

		if (throwableDesc2Throw)
		{
			throwableDesc2Throw->thingSprite->SetVisiblity(false);
		}

		throwableDesc2Throw = nullptr;

		impulse = {};
		impulseTime = 0.f;

		owningPuppet = nullptr;

		// if (dashTrail2)
		// {
		// 	dashTrail2->color.a = 0.f;
		// }

		debugLines.clear();
		debugSpheres.clear();
		debugText.clear();
	}

	void Thing::OnFinishLevel(const EventOnFinishLevel& evt)
	{
		Player::OnFinishLevel(evt);
	}

	void Thing::OnThrowable2Pickup(ThrowableDesc* throwable, Math::Vector3 standPos, bool autoGrab)
	{
		throwableDesc2Pickup = throwable;
		throwableStandPos = standPos;
		throwableAutoGrab = autoGrab;
	}

	bool Thing::CanGrabThrowable()
	{
		return throwableDesc2Throw == nullptr;
	}

	void Thing::MakeInvincibile(float time)
	{
		invincibilityTime = time;
	}

	void Thing::TeleportState(const CmdTeleportState& evt)
	{
		teleportAvailable = evt.available;
		teleportDest = evt.dest;
	}

	bool Thing::HasPuppet()
	{
		return state == State::PuppetMaster;
	}

	void Thing::SetPuppet(Puppet* setPuppet)
	{
		owningPuppet = setPuppet;

		onSurface = true;
		owningPuppet->StartObey();
		anim->SetVisiblity(false);
		SetState(State::PuppetMaster, 0.3f);
	}

	void Thing::SetState(State newState, float timer)
	{
		if (newState == State::TunnelTransition)
		{
			if (curTunnelTransition.door)
			{				
				curTunnelTransition.door->PlayOpen();
			}
		}
		else if (state == State::Dash && newState != State::Dash)
		{
			slashDash = false;
		}
		else if (state == State::EnteringPuppet)
		{
			Utils::PlaySoundEvent("event:/FatGuy/Penetration");
		}

		state = newState;
		stateTimer = timer;
	}

	void Thing::Kill(Math::Vector3 killDir, DeathSource deathSource)
	{
		if (killed)
		{
			return;
		}

		if (WorldManager::instance->IsCheatEnabled(Cheat::PlayerInvulnerable) || invincibilityTime > 0.0f)
		{
			return;
		}

		Player::Kill(killDir, deathSource);

		if (Utils::IsNonZero(killDir))
		{
			const float killAngle = Utils::Angle::FromDirection(killDir).ToDegrees();
			WorldManager::instance->GetBlood()->Spawn("ThingSpray", nullptr, transform.position, killAngle);
		}

		if (WorldManager::instance->dashZone)
		{
			WorldManager::instance->dashZone->SetVisiblity(false);
		}

		if (WorldManager::instance->dashArrow)
		{
			WorldManager::instance->dashArrow->SetVisiblity(false);
		}

		HideAllDashTrails();

		if (owningPuppet)
		{
			owningPuppet->Kill(killDir, DeathSource::InnerDash);
			owningPuppet = nullptr;
		}

		const char* deathEvent = "event:/hero/FanDeath";

		switch (deathSource)
		{		
			case DeathSource::Fun:
				deathEvent = "event:/hero/FanDeath";
			break;
			case DeathSource::Laser:
				deathEvent = "event:/hero/Laser Death";
			break;
			case DeathSource::Elecro:
				deathEvent = "event:/hero/Turret Death";
			break;
			case DeathSource::Bullet:
				deathEvent = "event:/hero/FanDeath";
			break;
		}

		Utils::PlaySoundEvent(deathEvent, &transform.position);
	}

	void Thing::SetSurfaceNormal(Math::Vector3 normal, SetUpVector setUpVector)
	{
		if (setUpVector == SetUpVector::Yes)
		{
			upVector = normal;
		}

		surfaceNormal = normal;

		eastl::tie(onWall, onCeiling, onGround) = Utils::NormalToFlags(surfaceNormal);

		onSurface = onWall || onCeiling || onGround;
	}

	Utils::MaybeCastRes Thing::RayCastCorner(Math::Vector3 rayFrom, Math::Vector3 rayDir, float rayLength, Math::Vector3 cornerDir, uint32_t group)
	{
		const Math::Vector3 rayFrom1 = rayFrom + rayDir * rayLength;

		if (!Utils::RayHit(rayFrom1, cornerDir, TILE_HALF_SIZE, group))
		{
			// Don't snap to instant death!
			if (WorldManager::instance->RayCastKillTriggers(rayFrom + rayDir * TILE_HALF_SIZE, cornerDir, TILE_SIZE))
			{
				return eastl::nullopt;
			}

			const Math::Vector3 rayFrom2 = rayFrom1 + cornerDir * TILE_HALF_SIZE;
			return Utils::RayCast(rayFrom2, -rayDir, rayLength, group);
		}

		return eastl::nullopt;
	}

	Utils::MaybeCastRes Thing::QueryCorner(Math::Vector3 rayFrom, Math::Vector3 rayDir, float dist)
	{
		if (onCeiling)
		{
			return RayCastCorner(rayFrom, rayDir, dist, { 0.f, 1.f, 0.f }, PhysGroup::WorldDoors);
		}
		else if (onWall)
		{
			if (auto castRes = RayCastCorner(rayFrom, rayDir, dist, { 1.f, 0.f, 0.f }, PhysGroup::WorldDoors))
			{
				return castRes;
			}
			else if (auto castRes = RayCastCorner(rayFrom, rayDir, dist, { -1.f, 0.f, 0.f }, PhysGroup::WorldDoors))
			{
				return castRes;
			}
		}
		else if (onGround)
		{
			return RayCastCorner(rayFrom, rayDir, dist, { 0.f, -1.f, 0.f }, PhysGroup::WorldDoors);
		}

		return eastl::nullopt;
	}

	Utils::MaybeCastRes Thing::QuerySurface(Math::Vector3 pos, uint32_t group, Math::Vector3* moveDir, Math::Vector3 *priorityDir)
	{
		const float attachToSurfaceDist = 24.f * TILE_SCALE;

		auto doRayCast = [&](Math::Vector3 traceDir, Math::Vector3* checkMoveDir) -> Utils::MaybeCastRes
		{
			if (auto castRes = Utils::RayCast(pos, traceDir, attachToSurfaceDist, group))
			{
				if (!checkMoveDir || checkMoveDir->Dot(castRes->hitNormal) < -0.001f)
				{
					return castRes;
				}
			}
			return eastl::nullopt;
		};

		if (priorityDir)
		{
			if (auto castRes = doRayCast(*priorityDir, nullptr))
			{
				return castRes;
			}
		}

		const auto traceDirections = {
			Math::Vector3{ 1.f, 0.f, 0.f},
			Math::Vector3{-1.f, 0.f, 0.f},
			Math::Vector3{0.f,  1.f, 0.f},
			Math::Vector3{0.f, -1.f, 0.f}
		};
		for (Math::Vector3 traceDir : traceDirections)
		{
			if (auto castRes = doRayCast(traceDir, moveDir))
			{
				return castRes;
			}
		}

		return eastl::nullopt;
	}

	void Thing::DoAttachToSurface(Math::Vector3 normal)
	{
		SetSurfaceNormal(normal, SetUpVector::Yes);
	}

	bool Thing::TryAttachToSurface(uint32_t group, Math::Vector3* moveDir, Math::Vector3 *priorityDir)
	{
		if (auto castRes = QuerySurface(transform.position, group, moveDir, priorityDir))
		{
			DoAttachToSurface(castRes->hitNormal);
			return true;
		}

		return false;
	}

	bool Thing::TryToReflectFromShield(Math::Vector3 dir, float dist)
	{
		if (auto castRes = Utils::RayCast(transform.position, dir, dist, PhysGroup::Enemy_))
		{
			if (castRes->userdata)
			{
				if (auto shield = dynamic_cast<Shield*>(castRes->userdata->object))
				{
					if (shield->IsVisible())
					{
						impulse = Utils::Vector::Reflect(dir, castRes->hitNormal) * shieldImpulse;
						impulseTime = 0.5f;

						if (auto soldier = dynamic_cast<Soldier*>(shield->enemyOwner))
						{
							soldier->OnShieldTouch();
						}

						return true;
					}
				}
			}
		}
		return false;
	}

	void Thing::TryKillEnemyInDirection(Math::Vector3 dir)
	{
		Math::Matrix trans;
		trans.Vx() = dir;
		trans.Vy() = { -dir.y, dir.x, dir.z };
		trans.Pos() = Sprite::ToUnits(transform.position + dir * 0.5f);

		eastl::vector<PhysScene::BodyUserData*> bodies;
		GetRoot()->GetPhysScene()->OverlapWithBox(trans, Sprite::ToUnits(killBoxSize), PhysGroup::Enemy_, bodies);

		// The player is moving really fast in dash state
		// We might miss a Shiled with the first check TryToReflectFromShield()
		// So, we have to check if we overlap with a Shield and don't kill Shiled's owner
		eastl::vector<Enemy*> toRemove;
		for (auto* userData : bodies)
		{
			if (auto shield = dynamic_cast<Shield*>(userData->object))
			{
				toRemove.push_back(shield->enemyOwner);
			}
		}

		bodies.erase(eastl::remove_if(bodies.begin(), bodies.end(), [&toRemove](PhysScene::BodyUserData* body)
			{
				Enemy* enemy = dynamic_cast<Enemy*>(body->object->GetParent());
				return enemy && eastl::find(toRemove.begin(), toRemove.end(), enemy) != toRemove.end();
			}), bodies.end());

		for (auto* userData : bodies)
		{
			Enemy* enemy = nullptr;

			if (userData->object->GetParent())
			{
				enemy = dynamic_cast<Enemy*>(userData->object->GetParent());
			}

			if (enemy && enemy->IsVisible() && !enemy->IsImmortal())
			{
				// Do not deal double damage during the same dash attack
				if (eastl::find(hitEnemies.begin(), hitEnemies.end(), enemy) == hitEnemies.end())
				{
					//Never kill puppet by dash
					if (auto newPuppet = dynamic_cast<Puppet*>(enemy))
					{
						if (newPuppet->IsReadyToObey() && owningPuppet != newPuppet)
						{
							owningPuppet = newPuppet;

							puppetEnteringPos = transform.position;
							owningPuppet->StartAccept();
							anim->anim.ActivateLink("Jump");
							SetState(State::EnteringPuppet, puppetEnteriingTime);
						}
					}
					else
					{
						const HitResult hitRes = enemy->Hit(dir, 1.f);
						if (hitRes != HitResult::Nothing)
						{
							hitEnemies.push_back(enemy);
							Utils::AddCameraShake(hitRes == HitResult::Lethal ? 0.5f : 0.25f);
						}
					}
				}
			}
		}
	}

	void Thing::HideAllDashTrails()
	{
		if (dashTrail)
		{
			dashTrail->color = COLOR_WHITE_A(0.0f);

			if (dashTrailEnd)
			{
				dashTrailEnd->color = COLOR_WHITE_A(0.0f);
			}
		}

		if (dashTrailAttack)
		{
			dashTrailAttack->color = COLOR_WHITE_A(0.0f);

			if (dashTrailAttackEnd)
			{
				dashTrailAttackEnd->color = COLOR_WHITE_A(0.0f);
			}

			if (dashSlash)
			{
				dashSlash->color = COLOR_WHITE_A(0.0f);
			}

			if (dashSlash2)
			{
				dashSlash2->color = COLOR_WHITE_A(0.0f);
			}
		}
	}

	void Thing::Draw(float dt)
	{
		/*if (!GetScene()->IsPlaying())
		{
			if (GetRoot()->GetControls()->DebugHotKeyPressed("KEY_LSHIFT", "KEY_1"))
			{
				//Utils::ConvertScene(GetScene());
				Utils::ConvertAllScenes();
			}
		}*/

		if (killed || !WorldManager::instance || !WorldManager::instance->dashZone || !WorldManager::instance->dashArrow || !WorldManager::instance->dashTail)
		{
			return;
		}

		if (owningPuppet && !owningPuppet->AllowInnerDash())
		{
			isDashAllowed = false;
		}

		const bool showForState = state != State::TunnelTransition;

		bool isDashVisible = false;
		bool isDashArrowVisible = isDashAllowed && showForState;// && WorldManager::instance->gamepadConnected;

		const float zoneMoveViscosity = (WorldManager::instance->gamepadConnected && teleportDashZoneTime <= 0.f) ? 0.f : 0.f;
		teleportDashZoneTime = Utils::Saturate(teleportDashZoneTime - dt);

		if (showForState && Utils::IsNonZero(dashDir))
		{
			isDashVisible = !drawDashRes.isInAir;

			WorldManager::instance->dashArrow->color = drawDashRes.isInAir ? dashFailColor : dashSuccessColor;
			WorldManager::instance->dashTail->color  = drawDashRes.isInAir ? dashFailColor : dashSuccessColor;

			{
				auto &tm = WorldManager::instance->dashZone->GetTransform();
				const auto angle = Utils::Angle::FromDirection(Utils::Vector::xy(drawDashRes.normal)) + Utils::Angle::Degrees(90.f);

				tm.position = Utils::Approach(tm.position, drawDashRes.pos, dt, zoneMoveViscosity);
				tm.rotation = Utils::Vector::xyV(tm.rotation, angle.ToSignedDegrees());
			}

			if (isDashArrowVisible)
			{
				auto &tm = WorldManager::instance->dashArrow->GetTransform();

				const Math::Vector3 pos = Utils::Approach(tm.position, drawDashRes.pos, dt, zoneMoveViscosity);
				const Math::Vector3 dir = Utils::Vector::Normalize(pos - transform.position);
				const float length      = (pos - transform.position).Length();

				const auto angle = Utils::Angle::FromDirection(Utils::Vector::xy(dir)) - Utils::Angle::Degrees(90.f);

				tm.position = pos;
				tm.rotation = Utils::Vector::xyV(tm.rotation, angle.ToSignedDegrees());

				WorldManager::instance->dashTail->GetTransform().size.y = std::max(length - TILE_SIZE, 24.f);
			}
		}

		WorldManager::instance->dashZone->SetVisiblity(isDashVisible);
		WorldManager::instance->dashArrow->SetVisiblity(isDashArrowVisible);

		HideAllDashTrails();

		if (slashDash)
		{
			if (dashTrailAttack)
			{
				const float tailK = state == State::Dash ? sinf(Utils::Saturate(dashT) * Math::PI) : 0.f;
				const float alpha = tailK;

				auto& tm = dashTrailAttack->GetTransform();
				const auto dashAngle = Utils::AdjustAngleByDirection(Utils::Angle::FromDirection(dashDir), GetDirection());

				tm.rotation = Math::Vector3(0.0f, 0.0f, dashAngle.ToDegrees());
				tm.scale = Math::Vector3(GetDirection() * tailK, 1.0f, 1.0f);

				dashTrailAttack->color = COLOR_WHITE_A(alpha);

				if (dashTrailAttackEnd)
				{
					dashTrailAttackEnd->color = COLOR_WHITE_A(alpha);

					auto& tmEnd = dashTrailAttackEnd->GetTransform();
					tmEnd.scale = Math::Vector3(tailK, 1.0f, 1.0f);
				}

				if (dashSlash)
				{
					auto& tm = dashSlash->GetTransform();
					tm.scale = Math::Vector3(GetDirection(), 1.0f, 1.0f);
					tm.rotation = Math::Vector3(0.0f, 0.0f, dashAngle.ToDegrees());

					dashSlash->color = COLOR_WHITE_A(dashT < 0.7f ? alpha : 0.0f);
				}

				if (dashSlash2)
				{
					auto& tm = dashSlash2->GetTransform();
					tm.scale = Math::Vector3(GetDirection(), 1.0f, 1.0f);
					tm.rotation = Math::Vector3(0.0f, 0.0f, dashAngle.ToDegrees());

					dashSlash2->color = COLOR_WHITE_A(dashT > 0.7f ? alpha : 0.0f);
				}
			}			
		}
		else
		{
			if (dashTrail)
			{
				const float tailK = state == State::Dash ? sinf(Utils::Saturate(dashT) * Math::PI) : 0.f;
				const float alpha = tailK;

				auto& tm = dashTrail->GetTransform();
				const auto dashAngle = Utils::AdjustAngleByDirection(Utils::Angle::FromDirection(dashDir), GetDirection());

				tm.rotation = Math::Vector3(0.0f, 0.0f, dashAngle.ToDegrees());
				tm.scale = Math::Vector3(GetDirection() * tailK, 1.0f, 1.0f);

				dashTrail->color = COLOR_WHITE_A(alpha);

				if (dashTrailEnd)
				{
					dashTrailEnd->color = COLOR_WHITE_A(alpha);

					auto& tmEnd = dashTrailEnd->GetTransform();
					tmEnd.scale = Math::Vector3(tailK, 1.0f, 1.0f);
				}
			}
		}
	}

	// t = 0 -> 1
	float Thing::GetDashSpeed(float t, Math::Vector3 dir, float distance) const
	{
		// 1. Accelerate to gravitySpeedMax in case of dashing down to the ground
		// 2. Do not decelerate dashes that ends on surface - in order to keep fast pacing movement
		const float distK    = distance >= 0.f ? Utils::Saturate(distance / dashMaxDistance) : 0.f;
		const float endSpeed = distK > 0.99f ? Utils::FSel(dir.y, dashEndSpeed, gravitySpeedMax) : dashStartSpeed;
		return Utils::Lerp(dashStartSpeed, endSpeed, Math::EaseInQuad(t));
	}

	void Thing::DashAssistance()
	{
		if (!WorldManager::instance->gamepadConnected)
		{
			return;
		}

		const auto traceFrom = transform.position + aimDir * 24.f;
		const auto aimAngle  = Utils::Angle::FromDirection(dashDir);

		auto isSafeDirection = [&](float delta) -> bool
		{
			const auto traceDir  = (aimAngle + Utils::Angle::Degrees(delta)).ToDirection();
			if (auto castRes = Utils::SphereCast(traceFrom, traceDir, dashMaxDistance, 8.f, PhysGroup::WorldDoorsDeathZones))
			{
				if (castRes->userdata && dynamic_cast<InstantPlayerKiller*>(castRes->userdata->object))
				{
					// debugSpheres.push_back({castRes->hitPos, 4.f, COLOR_BLUE});
					return false;
				}
			}

			return true;
		};

		// Direction assistance
		float leftSafeDelta = 0.f;
		bool isLeftSafe = false;
		for (float a = 0.f; a <= 15.f; a += 1.f)
		{
			if (isSafeDirection(a))
			{
				isLeftSafe = true;
				leftSafeDelta = a;
				break;
			}
		}
		float rightSafeDelta = 0.f;
		bool isRightSafe = false;
		for (float a = 0.f; a >= -15.f; a -= 1.f)
		{
			if (isSafeDirection(a))
			{
				isRightSafe = true;
				rightSafeDelta = a;
				break;
			}
		}

		if (isLeftSafe && !isRightSafe)
		{
			dashDir = (aimAngle + Utils::Angle::Degrees(leftSafeDelta)).ToDirection();
		}
		else if (!isLeftSafe && isRightSafe)
		{
			dashDir = (aimAngle + Utils::Angle::Degrees(rightSafeDelta)).ToDirection();
		}
		else if (isLeftSafe && isRightSafe)
		{
			if (std::abs(leftSafeDelta) < std::abs(rightSafeDelta))
			{
				dashDir = (aimAngle + Utils::Angle::Degrees(leftSafeDelta)).ToDirection();
			}
			else
			{
				dashDir = (aimAngle + Utils::Angle::Degrees(rightSafeDelta)).ToDirection();
			}
		}

		auto doKillAssistance = [&](float delta) -> bool
		{
			const auto traceDir = (aimAngle + Utils::Angle::Degrees(delta)).ToDirection();
			if (auto castRes = Utils::SphereCast(traceFrom, traceDir, dashMaxDistance, 8.f, PhysGroup::WorldDoorsEnemies))
			{
				if (const auto *enemy = castRes->userdata ? dynamic_cast<Enemy*>(castRes->userdata->object->GetParent()) : nullptr)
				{
					if (delta != 0.f)
					{
						++killAssistanceCount;

						// debugSpheres.push_back({traceFrom, 4.f, COLOR_RED});
						// debugLines.push_back({traceFrom, traceFrom + traceDir * dashMaxDistance, COLOR_GREEN});
						// debugLines.push_back({traceFrom, traceFrom + aimDir * dashMaxDistance, COLOR_RED});
					}

					dashDir = traceDir;

					return true;
				}
			}

			return false;
		};

		if (leftSafeDelta == 0.f && rightSafeDelta == 0.f)
		{
			// Kill assistance
			bool isTargetFound = false;
			for (float a = 0.f; !isTargetFound && a >= -20.f; a -= 5.f)
			{
				isTargetFound |= doKillAssistance(a);
			}
			for (float a = 0.f; !isTargetFound && a <=  20.f; a += 5.f)
			{
				isTargetFound |= doKillAssistance(a);
			}
		}
		else
		{
			++safeAssistanceCount;
			// debugSpheres.push_back({traceFrom, 4.f, COLOR_GREEN});
			// debugLines.push_back({traceFrom, traceFrom + dashDir * dashMaxDistance, COLOR_GREEN});
			// debugLines.push_back({traceFrom, traceFrom + aimDir * dashMaxDistance, COLOR_RED});
		}
	}

	void Thing::ShowTunnelHint(Math::Vector3 bottomPos, Math::Vector3 hitNormal)
	{
		if (bottomPos.x < 0.0f)
		{
			bottomPos.x -= TILE_SIZE;
		}

		if (bottomPos.y > 0.0f)
		{
			bottomPos.y += TILE_SIZE;
		}

		bottomPos.x = (int)(bottomPos.x / TILE_SIZE) * TILE_SIZE;
		bottomPos.y = (int)(bottomPos.y / TILE_SIZE) * TILE_SIZE;

		int hint = 1;

		const float angle = Utils::ToDegrees(Math::SafeAtan2(-hitNormal.y, -hitNormal.x));

		if (Math::IsEqual(angle, 90.0f, 0.001f))
		{
			hint = 0;
		}
		else
		if (Math::IsEqual(angle,-90.0f, 0.001f))
		{
			hint = 2;
		}
		else
		if (Math::IsEqual(angle, 0.0f, 0.001f))
		{
			hint = 3;
		}

		MainEventsQueue::PushEvent(CmdShowTunnelHint{ hint, bottomPos });
	}

	bool Thing::TryToActivateTunnel(bool shouldActivateTunnel)
	{
		const uint32_t worldGroup = PhysGroup::WorldDoors;

		const Math::Vector3 pos = transform.position;
		const Math::Vector3 forward = GetForward() * tunnelActivationDist;
		Math::Vector3 bottomPos = pos - upVector * TILE_HALF_SIZE;

		curTunnelTransition = {};

		auto FindTunnelExitPos = [&]() -> bool
		{
			if (curTunnelTransition.door && !curTunnelTransition.door->IsUnlocked())
			{
				return false;
			}

			const float tunnelSize = TILE_SIZE;
			const auto wishTunnelExitPos = curTunnelTransition.tunnelEntrancePos + curTunnelTransition.tunnelDir * (tunnelSize + TILE_HALF_SIZE);
			if (auto castRes = Utils::RayCast(wishTunnelExitPos, -curTunnelTransition.tunnelDir, TILE_SIZE, PhysGroup::WorldDoors))
			{
				curTunnelTransition.tunnelExitPos = castRes->hitPos + castRes->hitNormal * surfaceMargin;
				curTunnelTransition.tunnelExitDir = GetForward();
				curTunnelTransition.tunnelExitUp  = upVector;
				return true;
			}
			return false;
		};

		auto castRes0 = Utils::RayCastLine(pos + forward, bottomPos + forward, PhysGroup::TunnelDoor);

		if (!castRes0)
		{
			castRes0 = Utils::RayCastLine(pos, pos + forward, worldGroup);
		}

		auto castRes1 = Utils::RayCastLine(pos - forward, bottomPos - forward, PhysGroup::TunnelDoor);

		if (!castRes1)
		{
			castRes1 = Utils::RayCastLine(pos, pos - forward, worldGroup);
		}

		if (castRes0 && castRes1)
		{
			curTunnelTransition.valid                   = true;
			curTunnelTransition.tunnelDir               = -upVector;
			curTunnelTransition.tunnelEntrancePos       = pos;
			curTunnelTransition.distToTunnelEntrancePos = 0.f;
			curTunnelTransition.addTunnelSpeed          = 0.f;
			curTunnelTransition.hintPos                 = bottomPos;

			if (castRes0->userdata)
			{
				curTunnelTransition.door = dynamic_cast<TunnelDoorEntity*>(castRes0->userdata->object);
			}

			if (!curTunnelTransition.door && castRes1->userdata)
			{
				curTunnelTransition.door = dynamic_cast<TunnelDoorEntity*>(castRes1->userdata->object);
			}

			if (FindTunnelExitPos())
			{
				ShowTunnelHint(curTunnelTransition.hintPos, upVector);

				if (shouldActivateTunnel)
				{
					lastTunnelPressedAtTime = -1.f;
					keepMovingDir = {};
					accumInputDelta = {};
					SetState(State::TunnelTransition, intoTunnelTime);

					return true;
				}
			}
			else
			{
				curTunnelTransition = {};
			}
		}

		// Try to find a tunnel underneath the player in advance
		if (!curTunnelTransition.valid && state != State::TunnelTransition)
		{
			const Math::Vector3 checkTunnelFrom = pos + GetForward() * tunnelActivationDist;
			if (!Utils::RayHitLine(pos, checkTunnelFrom, worldGroup))
			{
				if (auto castRes = Utils::RayCast(checkTunnelFrom, -upVector, TILE_HALF_SIZE, PhysGroup::TunnelDoor))
				{
					if (castRes->hitNormal.Dot(surfaceNormal) > 0.f)
					{
						bottomPos = checkTunnelFrom - upVector * TILE_HALF_SIZE;
						curTunnelTransition.hintPos = bottomPos;

						bottomPos.x = roundf(bottomPos.x / TILE_SIZE) * TILE_SIZE;
						bottomPos.y = roundf(bottomPos.y / TILE_SIZE) * TILE_SIZE;

						curTunnelTransition.valid                   = true;
						curTunnelTransition.tunnelDir               = -upVector;
						curTunnelTransition.tunnelEntrancePos       = bottomPos + upVector * (pos - bottomPos).Dot(upVector) + forward;
						curTunnelTransition.distToTunnelEntrancePos = (pos - curTunnelTransition.tunnelEntrancePos).Length();
						curTunnelTransition.addTunnelSpeed          = 0.f;

						if (castRes->userdata)
						{
							curTunnelTransition.door = dynamic_cast<TunnelDoorEntity*>(castRes->userdata->object);
						}

						if (FindTunnelExitPos())
						{
							ShowTunnelHint(curTunnelTransition.hintPos, upVector);

							if (shouldActivateTunnel)
							{
								lastTunnelPressedAtTime = -1.f;
								keepMovingDir = {};
								accumInputDelta = {};
								SetState(State::TunnelTransition, intoTunnelTime);
								return true;
							}
						}
						else
						{
							curTunnelTransition = {};
						}
					}
				}
			}
		}
		
		// Try to find a tunnel in front of the player
		if (!curTunnelTransition.valid && state != State::TunnelTransition)
		{
			for (auto checkDir : {GetForward(), -GetForward()})
			{
				if (auto castRes = Utils::RayCast(pos, checkDir, cornerTransitionByKeyDist, PhysGroup::TunnelDoor))
				{
					bottomPos = castRes->hitPos - castRes->hitNormal * TILE_HALF_SIZE;
					curTunnelTransition.hintPos = bottomPos;

					bottomPos.x = roundf(bottomPos.x / TILE_SIZE) * TILE_SIZE;
					bottomPos.y = roundf(bottomPos.y / TILE_SIZE) * TILE_SIZE;

					curTunnelTransition.valid                   = true;
					curTunnelTransition.tunnelDir               = -castRes->hitNormal;
					curTunnelTransition.tunnelEntrancePos       = pos + 2.f * upVector;
					curTunnelTransition.distToTunnelEntrancePos = (pos - curTunnelTransition.tunnelEntrancePos).Length();
					curTunnelTransition.addTunnelSpeed          = 0.f;

					if (castRes->userdata)
					{
						curTunnelTransition.door = dynamic_cast<TunnelDoorEntity*>(castRes->userdata->object);
					}

					if (FindTunnelExitPos())
					{
						ShowTunnelHint(curTunnelTransition.hintPos, castRes->hitNormal);

						if (shouldActivateTunnel)
						{
							lastTunnelPressedAtTime = -1.f;
							keepMovingDir = {};
							accumInputDelta = {};
							SetState(State::TunnelTransition, intoTunnelTime);

							return true;
						}
					}
					else
					{
						curTunnelTransition = {};
					}

					break;
				}
			}
		}

		return false;
	}

	Utils::MaybeCastRes Thing::FindClosestDash(Math::Vector3 endPos, Math::Vector3 endNormal)
	{
		const auto endFwd = Utils::Vector::xyO(Utils::Vector::Rotate90(Utils::Vector::xy(endNormal)));
		const float step = 1.f;
		for (float offset = step; offset <= TILE_HALF_SIZE; offset += step)
		{
			auto castRes = QuerySurface(endPos + endFwd * offset, PhysGroup::WorldDoors, &dashDir);
			if (!castRes)
			{
				castRes = QuerySurface(endPos - endFwd * offset, PhysGroup::WorldDoors, &dashDir);
			}

			if (castRes)
			{
				return castRes;
			}
		}

		return eastl::nullopt;
	}

	bool Thing::TryToDash(bool isDashJustPressed)
	{
		// In order to draw direction arrow for the gamepad
		drawDashRes.pos = transform.position + aimDir * dashMaxDistance;

		// Allow the dash during Coyote time period
		if (onSurface || timeInAir <= dashCoyoteTime)
		{
			// This string is for debug purposes only
			eastl::string dashMeta;

			#define DASH_DEBUG_META(step) if (debugDash) { dashMeta += StringUtils::PrintTemp("%d, ", step); }

			dashDir          = isDashAllowed ? aimDir : Math::Vector3{};
			dashStartPos     = transform.position;
			dashEndPos       = transform.position + dashDir * dashMaxDistance;
			dashMoveDistance = 0.f;

			afterDashTransition.isVaild = false;

			Utils::MaybeCastRes surfaceRes;

			auto ValidateDashByDistance = [&](Math::Vector3 endPos) -> bool
			{
				if ((endPos - dashStartPos).Length() < TILE_HALF_SIZE)
				{
					isDashAllowed = false;
					drawDashRes.isInAir = true;
					drawDashRes.normal  = {0.f, 1.f, 0.f};
					drawDashRes.pos     = transform.position;
					return false;
				}
				return true;
			};

			if (Utils::IsNonZero(dashDir))
			{
				DASH_DEBUG_META(1);

				// During surface transition the sphere is slightly touching a corner and preventing correct dash
				// So, reduce the radius in order to solve the issue
				const float sphRadius = state == State::SurfaceTransition ? 1.f : 3.f;
				if (surfaceRes = Utils::SphereCast(transform.position, dashDir, dashMaxDistance + TILE_SIZE, sphRadius, PhysGroup::WorldDoors))
				{
					DASH_DEBUG_META(2);

					if (auto castRes = Utils::RayCast(transform.position, dashDir, dashMaxDistance, PhysGroup::WorldDoors))
					{
						DASH_DEBUG_META(3);

						if (castRes->hitLength - surfaceRes->hitLength > TILE_HALF_SIZE)
						{
							DASH_DEBUG_META(4);

							// Override a surface in case of slightest touch
							// It barely blocks the dash
							surfaceRes = castRes;
						}
					}
					// Discard surface only if the dash ended up in air
					// We don't want tunneling through corners
					else if (!QuerySurface(dashEndPos, PhysGroup::WorldDoors))
					{
						DASH_DEBUG_META(5);

						// Discard a surface in case of slightest touch
						// It barely blocks the dash at all
						surfaceRes = eastl::nullopt;
					}

					// Discard invalid hit - the in the middle or really close to collision
					// This could happened after exiting the tunnel
					// The player just exited but not snapped to correct surface distance yet
					if (surfaceRes && surfaceRes->hitLength < 1.f)
					{
						isDashAllowed = false;
						return false;
					}
				}

				if (surfaceRes)
				{
					DASH_DEBUG_META(6);

					auto hitNormal = surfaceRes->hitNormal;
					auto hitPos    = surfaceRes->hitPos;

					dashEndPos = hitPos + hitNormal * surfaceMargin;

					if (!ValidateDashByDistance(dashEndPos))
					{
						return false;
					}

					if (!WorldManager::instance->gamepadConnected)
					{
						DASH_DEBUG_META(7);

						// Project the cursor on a surface and adjust dastEndPos accordingly
						const Math::Vector3 cursorPos = Utils::GetCursorPos(aliasCursorX, aliasCursorY);
						const float proj = (dashEndPos - cursorPos).Dot(hitNormal) - surfaceMargin;
						if (proj > 0.f && proj < TILE_SIZE)
						{
							DASH_DEBUG_META(8);

							auto wishEnd = cursorPos + hitNormal * (proj + surfaceMargin);
							if (!Utils::SphereHitLine(transform.position, wishEnd, sphRadius, PhysGroup::WorldDoors))
							{
								DASH_DEBUG_META(9);

								dashEndPos = wishEnd;

								// Discard the surface: the dash has been redirected along the surface
								surfaceRes = eastl::nullopt;
							}
						}
					}

					dashDir = Utils::Vector::Normalize(dashEndPos - transform.position);
				}
			}

			if (auto castRes = QuerySurface(dashEndPos, PhysGroup::WorldDoors, &dashDir))
			{
				DASH_DEBUG_META(10);

				drawDashRes.isInAir = false;
				drawDashRes.pos     = castRes->hitPos;
				drawDashRes.normal  = castRes->hitNormal;
			}
			else if (surfaceRes)
			{
				DASH_DEBUG_META(11);

				auto closestRes = FindClosestDash(dashEndPos, surfaceRes->hitNormal);
				if (!closestRes)
				{
					DASH_DEBUG_META(12);

					closestRes = surfaceRes;
				}

				drawDashRes.isInAir = false;
				drawDashRes.pos     = closestRes->hitPos;
				drawDashRes.normal  = closestRes->hitNormal;

				const auto wishDashEndPos = closestRes->hitPos + closestRes->hitNormal * surfaceMargin;

				if (auto castRes = QuerySurface(wishDashEndPos, PhysGroup::WorldDoors, &dashDir))
				{
					DASH_DEBUG_META(13);

					dashEndPos = wishDashEndPos;

					drawDashRes.isInAir = false;
					drawDashRes.pos     = castRes->hitPos;
					drawDashRes.normal  = castRes->hitNormal;
				}
				else
				{
					DASH_DEBUG_META(14);

					// Try to solve landing on a corner
					// Pick one of corner surfaces and then do transiton to that surface
					// at the end of the dash
					const auto wishAngle  = std::round(Utils::Angle::FromDirection(surfaceRes->hitNormal).ToRadian() / Math::HalfPI) * Math::HalfPI;
					const auto wishNormal = Utils::Angle::Radian(wishAngle).ToDirection();
					const auto dirAlongSurface = Utils::Vector::Normalize(dashDir - dashDir.Dot(wishNormal) * wishNormal);

					auto wishEndPos = wishDashEndPos + wishNormal * surfaceMargin + dirAlongSurface * moveAlongSurfaceTransitionDist;
					auto wishRes = Utils::RayCast(wishEndPos, -wishNormal, TILE_HALF_SIZE, PhysGroup::WorldDoors);
					if (!wishRes)
					{
						DASH_DEBUG_META(15);

						wishEndPos = wishDashEndPos + wishNormal * surfaceMargin - dirAlongSurface * moveAlongSurfaceTransitionDist;
						wishRes    = Utils::RayCast(wishEndPos, -wishNormal, TILE_HALF_SIZE, PhysGroup::WorldDoors);
					}

					if (wishRes)
					{
						DASH_DEBUG_META(16);

						afterDashTransition.isVaild = true;

						afterDashTransition.fromUpVector = surfaceRes->hitNormal;
						afterDashTransition.toUpVector   = wishNormal;

						afterDashTransition.fromPos = dashEndPos;
						afterDashTransition.toPos   = wishRes->hitPos + wishRes->hitNormal * surfaceMargin;
					}
				}
			}
			else
			{
				DASH_DEBUG_META(17);

				drawDashRes.isInAir = true;
				drawDashRes.normal  = {0.f, 1.f, 0.f};
				drawDashRes.pos     = transform.position + (Utils::IsNonZero(dashDir) ? dashDir : aimDir) * dashMaxDistance;
			}

			if (!ValidateDashByDistance(dashEndPos))
			{
				return false;
			}

			const bool hasBufferedDash = lastDashPressedAtTime > 0.f && totalTime - lastDashPressedAtTime <= dashBufferingTime;
			if (isDashAllowed && (isDashJustPressed || (onSurface && hasBufferedDash)))
			{
				slashDash = false;

				// TODO: Jump from a puppet to another puppet. But we have to igronre the puppet we are currently in in the raycast.
				if (auto castRes = Utils::SphereCastLine(dashStartPos, dashEndPos, 0.5f * killBoxSize.y, PhysGroup::Enemy_))
				{
					DASH_DEBUG_META(18);

					auto *object = castRes->userdata ? castRes->userdata->object : nullptr;
					auto *enemy  = object && object->GetParent() ? dynamic_cast<Enemy*>(object->GetParent()) : nullptr;
					if (enemy)
					{
						DASH_DEBUG_META(19);

						auto newPuppet = dynamic_cast<Puppet*>(enemy);
						if (newPuppet && newPuppet->IsReadyToObey() && owningPuppet != newPuppet)
						{
							owningPuppet = newPuppet;

							puppetEnteringPos = transform.position;
							owningPuppet->StartAccept();
							anim->anim.ActivateLink("Jump");
							SetState(State::EnteringPuppet, puppetEnteriingTime);

							return false;
						}

						slashDash = true;
					}
				}

				anim->anim.ActivateLink(slashDash ? "JumpAttack" : "Jump");

				Utils::PlaySoundEvent(slashDash ? "event:/hero/Dash_enemies" : "event:/hero/Dash_noenemies");

				SetState(State::Dash, -1.f);

				const auto wishUp = std::abs(upVector.y) > std::abs(upVector.x) ? Math::Vector3{0.f, Math::Sign(upVector.y), 0.f} : Math::Vector3{Math::Sign(upVector.x), 0.f, 0.f};
				SetInAirState(dashDir, wishUp);

				if (debugDash)
				{
					GetRoot()->Log("Dash", "%s; (%0.2f, %0.2f); (%0.2f, %0.2f); (%0.2f, %0.2f); (%0.2f, %0.2f); %0.2f; %s",
						GetScene()->GetName(),
						aimDir.x, aimDir.y,
						aimDirUnclamped.x, aimDirUnclamped.y,
						dashStartPos.x, dashStartPos.y,
						dashEndPos.x, dashEndPos.y,
						(dashEndPos - dashStartPos).Length(),
						dashMeta.c_str()
					);
				}

				lastDashPressedAtTime = -1.f;
				keepMovingDir = {};
				accumInputDelta = {};

				lastTrail = -1;
				dashT = 0.f;

				//DashAssistance();

				return true;
			}
		}

		return false;
	}

	void Thing::UpdateDash(float dt, bool shouldActivateTunnel, Math::Vector3 &moveVel)
	{
		onWall = false;
		onCeiling = false;
		onGround = false;
		onSurface = false;

		stateTimer -= dt;

		float angle = Math::SafeAtan2(dashDir.y, dashDir.x);

		if (flipped)
		{
			angle = -Math::SafeAtan2(dashDir.y, -dashDir.x) + Math::TwoPI;
		}

		anim->GetTransform().rotation = Utils::Vector::xyV(transform.rotation, Utils::ToDegrees(angle));

		if (TryToActivateTunnel(shouldActivateTunnel))
		{
			hitEnemies.clear();
			return;
		}

		const float wishDashDist = (dashEndPos - dashStartPos).Length();
		dashT = dashMoveDistance / wishDashDist;

		const float speed = GetDashSpeed(Utils::Saturate(dashT), dashDir, wishDashDist);
		bool isHitObstacle = false;

		// Hit dynamic obstacle
		if (auto castRes = Utils::SphereCast(transform.position, dashDir, speed * dt, 16.f, PhysGroup::WorldDoors))
		{
			if (castRes->hitLength > 0.f)
			{
				if (auto vel = Utils::GetMovingSurfaceVel(castRes))
				{
					const float physDt = 1.f/60.f;
					const float surfaceMoveAlongDashDir = std::max(-vel->Dot(dashDir), 0.f) * physDt;

					// Stop the dash only if the surface is moving towards the players
					if (surfaceMoveAlongDashDir > 0.f)
					{
						const auto spherePos = transform.position + dashDir * castRes->hitLength;
						isHitObstacle = true;
						dashEndPos = spherePos + castRes->hitNormal * surfaceMoveAlongDashDir;
					}
				}
			}
		}
		
		if (dashT >= 1.f || isHitObstacle)
		{
			hitEnemies.clear();

			SetState(State::Normal, -1.0f);
			anim->anim.ActivateLink("Idle");

			controller->SetPosition(dashEndPos);

			auto SetFlipped = [&](Math::Vector3 normal)
			{
				const auto dirAlongSurface = Utils::Vector::Normalize(dashDir - dashDir.Dot(normal) * normal);
				flipped = Utils::Vector::xy(dirAlongSurface).Cross(Utils::Vector::xy(normal)) < -0.01f;
			};

			if (TryAttachToSurface(PhysGroup::WorldDoors, &dashDir))
			{
				SetFlipped(surfaceNormal);
			}
			else
			{
				if (afterDashTransition.isVaild)
				{
					SetFlipped(afterDashTransition.toUpVector);

					SetState(State::SurfaceTransition, 0.f);
					curTransition = afterDashTransition;
				}
				else
				{
					gravity = std::max(-dashEndSpeed * dashDir.y, 0.f);
					inertia = dashEndSpeed * dashDir.x;
					lastForwardOnSurface = dashDir;
				}
			}
		}
		else
		{
			moveVel.x = dashDir.x * speed;
			moveVel.y = dashDir.y * speed;
			lastMoveVel = moveVel;

			dashMoveDistance += speed * dt;

			if (TryToReflectFromShield(dashDir, std::max(24.f, speed * dt)))
			{
				SetState(State::Normal, -1.0f);
				anim->anim.ActivateLink("Idle");
				flipped = Utils::Vector::xy(dashDir).Cross(Utils::Vector::xy(surfaceNormal)) < -0.01f;
			}
			else
			{
				TryKillEnemyInDirection({ dashDir.x, dashDir.y, 0.f });
			}
		}
	}

	void Thing::SetInAirState(Math::Vector3 forward, Math::Vector3 up)
	{
		onSurface = false;
		onWall    = false;
		onCeiling = false;
		onGround  = false;

		surfaceVel = {};

		upVector      = up;
		surfaceNormal = up;

		flipped = Utils::IsFlipped(forward, upVector);
	}

	Math::Vector3 Thing::SnapToSurface(Math::Vector3 moveVel, Math::Vector3 rayFrom, float rayLength, float dt)
	{
		const Math::Vector3 deltaPos = moveVel * dt;
		if (auto castRes = Utils::RayCast(rayFrom, -upVector, rayLength, PhysGroup::WorldDoors))
		{
			lastForwardOnSurface = GetForward();

			const float invDt = Utils::SafeInvert(dt);
			const float distToSurface = castRes->hitLength;

			surfaceNormal = castRes->hitNormal;

			const float moveAlongNormal = deltaPos.Dot(surfaceNormal);
			const float distToSurfaceAfterMove = distToSurface + moveAlongNormal;
			const float distError = surfaceMargin - distToSurfaceAfterMove;

			// Prevent moving along normal below margin
			moveVel += std::max(distError, 0.f) * surfaceNormal * invDt;

			// Snap to a surface
			if (onWall || onCeiling)
			{
				// Move towards a surface along normal
				moveVel += std::min(distError, 0.f) * surfaceNormal * invDt * 0.25f;
				onSurface = true;
			}
			else
			{
				onSurface = (distToSurface - surfaceMargin) < 0.5f;
			}

			// Cancel any impulse
			{
				impulseTime = -1.f;
				impulse = {};
			}

			eastl::tie(onWall, onCeiling, onGround) = Utils::NormalToFlags(surfaceNormal);
		}
		else
		{
			SetInAirState(lastForwardOnSurface);
		}

		return moveVel;
	}

	Math::Vector3 Thing::GetPhysPos()
	{
		return Sprite::ToPixels(controller->controller->GetActorPosition());
	}

	void Thing::UpdateHitBox()
	{
		if (hitBox)
		{
			hitBox->body.body->SetTransform(Utils::MakeMatrix(upVector, transform.GetGlobal().Pos()));
		}
	}

	const char* Thing::GetBodyPartsTrail()
	{
		return "BodyPartThingTrail";
	}

	void Thing::UpdateThrowable(float dt)
	{
		const bool isThrowJustPressed = ORIN_CONTROLS_JUST_PRESSED(aliasShoot);

		if (isThrowJustPressed && throwableDesc2Throw)
		{
			lastThrowPressedAtTime = totalTime;
		}

		const bool shouldActivateThrow = lastThrowPressedAtTime > 0.f && totalTime - lastThrowPressedAtTime <= throwBufferingTime;		

		if (throwableDesc2Throw && shouldActivateThrow && !throwIsBlocked)
		{			
			Math::Vector3 dir = (state == State::Dash) ? aimDirUnclamped : Utils::Vector::Normalize(drawDashRes.pos - transform.position);
			WorldManager::instance->AddThrowable(transform.position, dir, throwableDesc2Throw);

			throwableDesc2Throw->OnThrow();
			throwableDesc2Throw->thingSprite->SetVisiblity(false);
			throwableDesc2Throw = nullptr;
		}

		throwIsBlocked = state == State::TunnelTransition;

		if (throwableDesc2Pickup)
		{
			MainEventsQueue::PushEvent(CmdShowThrowHint{ throwableStandPos });

			if (ORIN_CONTROLS_JUST_PRESSED(aliasShoot) || throwableAutoGrab)
			{
				throwableDesc2Throw = throwableDesc2Pickup;
				throwableDesc2Throw->thingSprite->SetVisiblity(true);
				throwableDesc2Pickup = nullptr;
				throwableAutoGrab = false;

				throwableDesc2Throw->OnPickup();
			}
		}

		if (throwableDesc2Throw)
		{			
			throwableDesc2Throw->thingSprite->SetVisiblity(state != State::TunnelTransition);

			auto& tm = throwableDesc2Throw->thingSprite->GetTransform();

			Math::Matrix mat;
			mat.Pos() = Sprite::ToUnits(transform.position + Math::Vector3(0.0f, TILE_SIZE * 0.5f, 0.0f));
			tm.SetGlobal(mat);			
		}
	}

	void Thing::Update(float dt)
	{
		if (WorldManager::instance->GetFrontendState() != FrontendState::Gameplay)
		{
			return;
		}

		if (delayedOutro > 0.0f)
		{
			float savedDelayedOutro = delayedOutro;
			delayedOutro -= dt;

			if (savedDelayedOutro > 0.5f && delayedOutro <= 0.5f)
			{				
				WorldManager::instance->StartFade(true, 0.5f, [this]() { GetScene()->FindEntity<CutSceneOutro>()->Start(); });
			}
		}
		
		int dbgLine = 1;

		if (!anim || !controller || levelFinished || killed || levelFinished)
		{
			return;
		}
		
		if (invincibilityTime > 0.0f)
		{
			invincibilityTime -= dt;

			if (invincibilityTime <= 0.0f)
			{
				invincibilityTime = -1.0f;
				anim->color = COLOR_WHITE;
			}			
			else
			{
				anim->color = Color(0.1f, 0.1f, 0.1f, sinf(invincibilityTime * 15.0f) * 0.35f + 0.35f);
			}
		}

		totalTime += dt;

		// static int showDashId = -1;
		// int dashCount = 0;
		// int dashRecordId = -1;

		// for (const auto &rec : records)
		// {
		// 	if (rec.scene == GetScene()->GetName())
		// 	{
		// 		if (showDashId < 0 || showDashId == dashCount)
		// 		{
		// 			dashRecordId = rec.id;
		// 			Sprite::DebugLine(rec.dashStartPos, rec.dashEndPos, COLOR_RED);
		// 			Sprite::DebugLine(rec.dashStartPos, rec.dashStartPos + rec.aimDir * TILE_SIZE, COLOR_GREEN);
		// 			Sprite::DebugLine(rec.dashStartPos, rec.dashStartPos + rec.aimDirUnclamped * TILE_HALF_SIZE, COLOR_BLUE);
		// 		}

		// 		++dashCount;
		// 	}
		// }

		// if (showDashId >= 0)
		// {
		// 	Sprite::DebugText(dbgLine++, "ShowDash: %d", dashRecordId);
		// }

		// if (GetRoot()->GetControls()->DebugHotKeyPressed("KEY_LCONTROL", "KEY_J"))
		// {
		// 	if (showDashId < 0)
		// 	{
		// 		showDashId = 0;
		// 	}
		// 	else if (showDashId - 1 < 0)
		// 	{
		// 		showDashId = dashCount - 1;
		// 	}
		// 	else
		// 	{
		// 		showDashId = showDashId - 1;
		// 	}
		// }
		// if (GetRoot()->GetControls()->DebugHotKeyPressed("KEY_LCONTROL", "KEY_K"))
		// {
		// 	showDashId = (showDashId < 0 ? 0 : (showDashId + 1)) % dashCount;
		// }
		// if (GetRoot()->GetControls()->DebugHotKeyPressed("KEY_LCONTROL", "KEY_L"))
		// {
		// 	showDashId = -1;
		// }
		// if (dashRecordId >= 0 && GetRoot()->GetControls()->DebugHotKeyPressed("KEY_LCONTROL", "KEY_H"))
		// {
		// 	const auto &rec = records[dashRecordId-1];

		// 	aimDir = rec.aimDir;
		// 	aimDirUnclamped = rec.aimDirUnclamped;
		// 	dashStartPos = rec.dashStartPos;
		// 	dashEndPos = rec.dashEndPos;

		// 	controller->SetPosition(rec.dashStartPos);

		// 	TryToDash(true);

		// 	return;
		// }

		for (auto &sph : debugSpheres)
		{
			Sprite::DebugSphere(sph.pos, sph.radius, sph.color);
			if (sph.ttl > 0.f)
			{
				sph.ttl -= dt;
				sph.alive = sph.ttl > 0.f;
			}
		}

		for (auto &line : debugLines)
		{
			Sprite::DebugLine(line.from, line.to, line.color);
			if (line.ttl > 0.f)
			{
				line.ttl -= dt;
				line.alive = line.ttl > 0.f;
			}
		}

		debugSpheres.erase(eastl::remove_if(debugSpheres.begin(), debugSpheres.end(), [](const auto &s) { return !s.alive; }), debugSpheres.end());
		debugLines.erase(eastl::remove_if(debugLines.begin(), debugLines.end(), [](const auto &s) { return !s.alive; }), debugLines.end());

		for (const auto &txt : debugText)
		{
			Sprite::DebugText(dbgLine++, "%s", txt.c_str());
		}

		//Determing move vector
		Math::Vector3 moveVel;
		Math::Vector3 inputMoveVel;
		Math::Vector3 wishInputMoveVel;

		const float curMoveSpeed = onSurface ? moveSpeed : airMoveSpeed;
		const float curWallMoveSpeed = wallMoveSpeed;
		const bool wasOnSurface = onSurface;

		bool isJustStartedMoving = false;

		uint32_t worldGroup = PhysGroup::WorldDoors;

		if (WorldManager::instance->gamepadConnected)
		{
			float deadZone = 0.15f;			

			float aimMainHorz = ORIN_CONTROLS_VALUE(aliasAimMainHorz);
			float aimMainVert = ORIN_CONTROLS_VALUE(aliasAimMainVert);
			aimMainHorz = std::abs(aimMainHorz) < deadZone ? 0.f : aimMainHorz;
			aimMainVert = std::abs(aimMainVert) < deadZone ? 0.f : aimMainVert;

			const auto aimMainDir = Utils::Vector::Normalize(Math::Vector3{ aimMainHorz, aimMainVert, 0.f });
			
			aimDir = Utils::IsNonZero(aimMainDir) ? aimMainDir : GetForward();
		}
		else
		{
			const Math::Vector2 cursorPos = Utils::GetCursorPos(aliasCursorX, aliasCursorY);
			aimDir = Math::Vector3(cursorPos.x, cursorPos.y, transform.position.z) - transform.position;
		}

		aimDir.Normalize();
		aimDirUnclamped = aimDir;

		if (state == State::PuppetMaster)
		{
			isDashAllowed = true;
		}
		else
		{
			const float proj = aimDir.Dot(upVector);
			isDashAllowed = (state == State::Normal || state == State::SurfaceTransition) && (proj >= cosf(Utils::ToRadian(dashThresholdDegress + 90.f)));

			// Align aim dir along the surface in case the is blocked by the surface
			{
				bool isAimBlocked = proj < 0.f;
				if (isAimBlocked && state == State::Normal)
				{
					// While staing at the edge of a corner the player still must be able to do dash
					// The player aims down and it looks perfectly reachable
					// So, lets help the player and won't block the dash
					const auto forward      = GetForward();
					const auto traceFromFwd = transform.position + forward * 24.f;
					const auto traceFromBwd = transform.position - forward * 24.f;

					Utils::MaybeCastRes castRes;
					if (QueryCorner(transform.position, forward, TILE_HALF_SIZE) && aimDirUnclamped.Dot(forward) > 0.f)
					{
						castRes = Utils::SphereCast(traceFromFwd, aimDirUnclamped, dashMaxDistance, 1.f, PhysGroup::WorldDoors);
					}

					if (!castRes && QueryCorner(transform.position, -forward, TILE_HALF_SIZE) && aimDirUnclamped.Dot(-forward) > 0.f)
					{
						castRes = Utils::SphereCast(traceFromBwd, aimDirUnclamped, dashMaxDistance, 1.f, PhysGroup::WorldDoors);
					}

					if (castRes && castRes->hitLength > TILE_HALF_SIZE)
					{
						isAimBlocked = false;
					}
				}

				if (isAimBlocked)
				{
					aimDir -= std::min(proj, 0.f) * upVector;
				}
			}
		}

		if (WorldManager::instance->gamepadConnected && !isDashAllowed)
		{
			isDashAllowed = true;
			aimDir = GetForward();
		}

		aimDir.Normalize();
		// debugLines.push_back({transform.position, transform.position + aimDir * 100.f, COLOR_GREEN, 0.25f, true});

		UpdateThrowable(dt);

		bool isDashJustPressed = ORIN_CONTROLS_JUST_PRESSED(aliasDash);

		if (delayedOutro > 0.0f)
		{
			isDashJustPressed = false;
		}

		if (isDashJustPressed)
		{
			lastDashPressedAtTime = totalTime;
		}		

		const bool isTunnelJustPressed = ORIN_CONTROLS_JUST_PRESSED(aliasUse);
		if (isTunnelJustPressed && state != State::TunnelTransition)
		{
			lastTunnelPressedAtTime = totalTime;
		}		

		const bool shouldActivateTunnel = lastTunnelPressedAtTime > 0.f && totalTime - lastTunnelPressedAtTime <= tunnelBufferingTime;

		switch (state)
		{
			case State::TunnelTransition:
			{
				stateTimer -= dt;

				float tunnelAddRot = 90.0f;

				if (stateTimer > intoTunnelTime - rotateTunnelTime)
				{
					tunnelAddRot = (intoTunnelTime - stateTimer) / rotateTunnelTime * 90.0f;
				}
				else
				if (stateTimer < rotateTunnelTime)
				{
					tunnelAddRot = (stateTimer) / rotateTunnelTime * 90.0f;
				}

				if (flipped)
				{
					tunnelAddRot = -tunnelAddRot;
				}

				auto rot = anim->GetTransform().rotation;
				rot.z = Math::SafeAtan2(curTunnelTransition.tunnelDir.y, curTunnelTransition.tunnelDir.x) / Math::Radian + 90.0f - tunnelAddRot;

				anim->GetTransform().rotation = rot;

				if (stateTimer < 0.0f)
				{
					SetState(State::Normal, -1.0f);					

					if (curTunnelTransition.door)
					{
						curTunnelTransition.door->PlayClose();
					}

					Math::Vector3 dir = -curTunnelTransition.tunnelDir;
					Math::Vector3 priorityDir = -curTunnelTransition.tunnelExitUp;
					TryAttachToSurface(worldGroup, &dir, &priorityDir);

					flipped = Utils::IsFlipped(curTunnelTransition.tunnelExitDir, upVector);
					teleportDashZoneTime = 0.25f;

					curTunnelTransition = {};
				}
				else
				{
					anim->anim.ActivateLink("Run");
					worldGroup = PhysGroup::World;

					if (curTunnelTransition.distToTunnelEntrancePos > 0.f)
					{
						curTunnelTransition.distToTunnelEntrancePos -= intoTunnelSpeed * dt;
						moveVel = Utils::Vector::Normalize(curTunnelTransition.tunnelEntrancePos - transform.position) * intoTunnelSpeed;

						if (curTunnelTransition.distToTunnelEntrancePos <= 0.f)
						{
							curTunnelTransition.addTunnelSpeed = intoTunnelSpeed * (1.f - stateTimer / intoTunnelTime);
							curTunnelTransition.startTime = intoTunnelTime - stateTimer;
						}
					}
					else
					{
						const float moveTime = intoTunnelTime - curTunnelTransition.startTime;
						const float t = 1.f - Utils::Saturate(stateTimer / moveTime);
						const auto pos = Utils::Lerp(curTunnelTransition.tunnelEntrancePos, curTunnelTransition.tunnelExitPos, t);

						controller->SetPosition(pos);

						UpdateHitBox();

						return;
					}
				}

				break;
			}
			case State::EnteringPuppet:
			{
				stateTimer -= dt;

				if (stateTimer < 0.0f)
				{
					onSurface = true;
					owningPuppet->StartObey();
					anim->SetVisiblity(false);
					SetState(State::PuppetMaster, 0.3f);

					return;
				}
				else
				{
					const float t = 1.0f - Utils::Saturate(stateTimer / puppetEnteriingTime);
					auto puppetPos = owningPuppet->GetTransform().position + Math::Vector3(0.0, 32.0f, 0.0f);

					const auto pos = Utils::Lerp(puppetEnteringPos, puppetPos, t);

					auto dir = puppetEnteringPos - puppetPos;
					dir.Normalize();

					float angle = Math::SafeAtan2(dir.y, dir.x);

					if (flipped)
					{
						angle = -Math::SafeAtan2(dir.y, -dir.x) + Math::TwoPI;
					}

					anim->GetTransform().rotation = Utils::Vector::xyV(transform.rotation, Utils::ToDegrees(angle));

					controller->SetPosition(pos);

					UpdateHitBox();
				}

				break;
			}
			case State::PuppetMaster:
			{
				float inputMoveVel = ORIN_CONTROLS_VALUE(aliasMoveHorz);

				if (fabs(inputMoveVel) > 0.0f)
				{
					owningPuppet->MoveMaster(inputMoveVel < 0.0f);
				}				

				if (owningPuppet->IsWaitRocket())
				{
					aimDir.x = 1.0f;
					aimDir.y = 0.3f;

					aimDir.Normalize();

					if (TryToDash(true))
					{						
						owningPuppet->Kill(aimDir, DeathSource::InnerDash);
						owningPuppet = nullptr;
						anim->SetVisiblity(true);
						return;
					}
				}

				if (owningPuppet->AllowInnerDash() && TryToDash(isDashJustPressed))
				{					
					owningPuppet->Kill(aimDir, DeathSource::InnerDash);
					owningPuppet = nullptr;
					anim->SetVisiblity(true);
					return;
				}
				else
				if (teleportAvailable && shouldActivateTunnel)
				{
					owningPuppet->TeleportMaster(teleportDest);
					teleportAvailable = false;
					return;
				}

				controller->SetPosition(owningPuppet->GetTransform().position + Math::Vector3(0.0, 32.0f, 0.0f));

				break;
			}
			case State::Dash:
			{
				UpdateDash(dt, shouldActivateTunnel, moveVel);
				UpdateHitBox();

				break;
			}
			case State::SurfaceTransition:
			{
				stateTimer = std::min(stateTimer + dt, transitionTotalTime);

				const float t = stateTimer / transitionTotalTime;

				upVector.SLerp(curTransition.fromUpVector, curTransition.toUpVector, t);

				curTransition.fromPos += surfaceVel * dt;
				curTransition.toPos   += surfaceVel * dt;

				Math::Vector3 pos;
				pos.Lerp(curTransition.fromPos, curTransition.toPos, t);

				const float angle = Utils::ToDegrees(Math::SafeAtan2(upVector.y, upVector.x)) - 90.f;
				anim->GetTransform().rotation = Utils::Vector::xyV(transform.rotation, angle);

				Transform& tm = controller->GetTransform();
				tm.SetRotation(Utils::Vector::xyV(tm.GetRotation(), angle - 90.f));
				tm.SetPosition(anim->GetTransform().GetLocal().MulNormal(initialCapsuleOffset));

				controller->SetPosition(pos);
				controller->controller->SetUpDirection(GetForward());

				UpdateHitBox();

				if (curTunnelTransition.valid)
				{
					ShowTunnelHint(curTunnelTransition.hintPos, -curTunnelTransition.tunnelDir);
				}

				if (TryToDash(isDashJustPressed))
				{
					return;
				}

				if (stateTimer >= transitionTotalTime)
				{
					SetState(State::Normal, -1.0f);

					SetSurfaceNormal(curTransition.toUpVector, SetUpVector::Yes);

					transform.position = curTransition.toPos;
					controller->SetPosition(curTransition.toPos);
				}

				return;
			}
			case State::Normal:
			{
				const bool canJump = onSurface;

				if (TryToDash(isDashJustPressed))
				{
					return;
				}

				const bool isMoveHorzJustPressed = ORIN_CONTROLS_JUST_PRESSED(aliasMoveHorz);
				const bool isMoveVertJustPressed = ORIN_CONTROLS_JUST_PRESSED(aliasMoveVert);

				if (isMoveHorzJustPressed || isMoveVertJustPressed)
				{
					keepMovingDir = {};
					accumInputDelta = {};
				}
			
				inputMoveVel.x = ORIN_CONTROLS_VALUE(aliasMoveHorz);
				inputMoveVel.y = ORIN_CONTROLS_VALUE(aliasMoveVert);
				inputMoveVel.x = std::abs(inputMoveVel.x) < 0.5f ? 0.f : Math::Sign(inputMoveVel.x);
				inputMoveVel.y = std::abs(inputMoveVel.y) < 0.5f ? 0.f : Math::Sign(inputMoveVel.y);

				// Utils::DebugText(dbgLine++, "Dash Assists: kill: %d; safe: %d", killAssistanceCount, safeAssistanceCount);

				inputMoveVel = inputMoveVel * Math::Vector2{curMoveSpeed, curWallMoveSpeed};

				if (WorldManager::instance->gamepadConnected)
				{
					accumInputDelta += Math::Vector2{ORIN_CONTROLS_DELTA(aliasMoveHorz), ORIN_CONTROLS_DELTA(aliasMoveVert)};
					if (std::abs(accumInputDelta.x) > 0.1 || std::abs(accumInputDelta.y) > 0.1)
					{
						keepMovingDir = {};
						accumInputDelta = {};
					}
				}
				else
				{
					accumInputDelta = {};
				}

				if (delayedOutro > 0.0f)
				{
					inputMoveVel = 0.0f;
					keepMovingDir = 0.0f;
					accumInputDelta = 0.0f;
				}

				wishInputMoveVel = inputMoveVel;

				keepMoving = Utils::IsNonZero(inputMoveVel);

				if (keepMoving)
				{
					inputPressedTime += dt;
				}
				else
				{
					keepMovingDir = {};
					accumInputDelta = {};
					inputPressedTime = 0.f;
				}

				if (onSurface)
				{
					if (keepMoving && Utils::IsNonZero(keepMovingDir))
					{
						auto inputMoveVelNorm = inputMoveVel;

						if (Math::AreApproximatelyEqual(keepMovingDir.x, 0.0f))
						{
							inputMoveVelNorm.x = 0.0f;
						}
						else
						{
							inputMoveVelNorm.y = 0.0f;
						}

						inputMoveVelNorm.Normalize();

						if (!Math::AreApproximatelyEqual(inputMoveVelNorm.x, keepMovingDir.x) ||
							!Math::AreApproximatelyEqual(inputMoveVelNorm.y, keepMovingDir.y))
						{
							inputMoveVel = 0.0f;
						}
						else
						{
							inputMoveVel = (onWall ? curWallMoveSpeed : curMoveSpeed) * keepMovingDir;
						}
					}
					else
					{
						const float inputProj = inputMoveVel.Dot(surfaceNormal);
						inputMoveVel -= inputProj * surfaceNormal;
					}
				}

				const char* activateLink = nullptr;

				if (Utils::IsNonZero(inputMoveVel))
				{
					activateLink = "Run";
					flipped = Utils::IsFlipped(inputMoveVel, surfaceNormal);
				}

				isJustStartedMoving = Utils::IsNonZero(inputMoveVel) && !Utils::IsNonZero(lastInputMoveVel);
				lastInputMoveVel = inputMoveVel;

				moveVel += inputMoveVel;
				lastMoveVel = moveVel + surfaceVel;

				// In Air
				if (!onWall && !onCeiling && !onGround)
				{
					upVector = {0.f, 1.f, 0.f};

					if (Utils::IsNonZero(inputMoveVel))
					{
						lastForwardOnSurface = Utils::Vector::Normalize(inputMoveVel);
					}

					flipped = Utils::IsFlipped(lastForwardOnSurface, upVector);
				}

				int surfaceHint = -1;
				Math::Vector3 surfaceHintDir = GetForward();

				if (TryToActivateTunnel(shouldActivateTunnel))
				{
					return;
				}

				if (onSurface && TryToReflectFromShield(Utils::Vector::Normalize(moveVel), std::max(moveVel.Length() * dt, 8.f)))
				{
					return;
				}

				// Trace along moving direction
				{
					auto CalcInnerDirAlongSurface = [&](Math::Vector3 hitPos, Math::Vector3 hitNormal)
					{
						const Math::Vector3 dir = (hitPos + upVector * 24.f) - transform.position;
						return Utils::Vector::Normalize(dir - dir.Dot(hitNormal) * hitNormal);
					};

					auto CalcOuterDirAlongSurface = [&](Math::Vector3 hitPos, Math::Vector3 hitNormal)
					{
						const Math::Vector3 dir = controller->GetPosition() - (hitPos + hitNormal * surfaceMargin);
						return -Utils::Vector::Normalize(dir - dir.Dot(hitNormal) * hitNormal);
					};

					auto BeginInnerTransition = [&](Math::Vector3 hitPos, Math::Vector3 hitNormal)
					{
						if (state == State::SurfaceTransition)
						{
							return;
						}

						const bool wasOnWall    = onWall;
						const bool wasOnCeiling = onCeiling;
						const bool wasOnGround  = onGround;

						eastl::tie(onWall, onCeiling, onGround) = Utils::NormalToFlags(hitNormal);

						if ((!wasOnWall && onWall) || (!wasOnCeiling && onCeiling) || (!wasOnGround && onGround))
						{
							if (onSurface)
							{
								SetState(State::SurfaceTransition, 0.0f);

								curTransition.isVaild      = true;
								curTransition.fromUpVector = upVector;
								curTransition.toUpVector   = hitNormal;

								const Math::Vector3 dirAlongSurface = CalcInnerDirAlongSurface(hitPos, hitNormal);
								curTransition.fromPos = controller->GetPosition();
								curTransition.toPos   = hitPos + hitNormal * surfaceMargin + dirAlongSurface * moveAlongSurfaceTransitionDist;
								keepMovingDir = dirAlongSurface;
								accumInputDelta = {};
								curDirAlongSurface = dirAlongSurface;
								flipped = Utils::IsFlipped(dirAlongSurface, hitNormal);
							}
							else
							{
								upVector      = hitNormal;
								surfaceNormal = hitNormal;
								controller->SetPosition(hitPos + hitNormal * surfaceMargin);
							}
						}
					};

					auto BeginOuterTransition = [&](Math::Vector3 hitPos, Math::Vector3 hitNormal)
					{
						if (state == State::SurfaceTransition)
						{
							return;
						}

						eastl::tie(onWall, onCeiling, onGround) = Utils::NormalToFlags(hitNormal);

						SetState(State::SurfaceTransition, 0.0f);

						curTransition.fromUpVector = upVector;
						curTransition.toUpVector   = hitNormal;

						curTransition.fromPos = controller->GetPosition();
						curTransition.toPos   = hitPos + hitNormal * surfaceMargin;

						const Math::Vector3 dirAlongSurface = CalcOuterDirAlongSurface(hitPos, hitNormal);
						keepMovingDir = dirAlongSurface;
						accumInputDelta = {};
						curDirAlongSurface = dirAlongSurface;
						flipped = Utils::IsFlipped(dirAlongSurface, hitNormal);
					};

					auto QueryInnerTransition = [&](Math::Vector3 queryDir, Math::Vector3 rayDir) -> Utils::MaybeCastRes
					{
						const Math::Vector3 rayFrom = transform.position;

						if (auto castRes = Utils::RayCast(rayFrom, rayDir, cornerTransitionByKeyDist, worldGroup))
						{
							const Math::Vector3 dirAlongSurface = CalcInnerDirAlongSurface(castRes->hitPos, castRes->hitNormal);
							if (onSurface
							    && Math::IsEqual(queryDir.Dot(castRes->hitNormal), 0.f, 0.001f)
								&& queryDir.Dot(dirAlongSurface) > 0.f)
							{
								return castRes;
							}
						}
						return eastl::nullopt;
					};

					auto QueryOutterTransition = [&](Math::Vector3 queryDir, Math::Vector3 rayDir) -> Utils::MaybeCastRes
					{
						const Math::Vector3 rayFrom = transform.position;

						if (auto castRes = QueryCorner(rayFrom, rayDir, cornerTransitionByKeyDist))
						{
							const Math::Vector3 dirAlongSurface = CalcOuterDirAlongSurface(castRes->hitPos, castRes->hitNormal);
							if (onSurface
							    && Math::IsEqual(queryDir.Dot(castRes->hitNormal), 0.f, 0.001f)
								&& queryDir.Dot(dirAlongSurface) > 0.f)
							{
								return castRes;
							}
						}
						return eastl::nullopt;
					};

					if (!Utils::IsNonZero(keepMovingDir))
					{
						if (Math::IsNonZero(wishInputMoveVel.Length2()))
						{
							for (auto checkDir : {GetForward(), -GetForward()})
							{
								if (auto castRes = QueryInnerTransition(wishInputMoveVel, checkDir))
								{
									BeginInnerTransition(castRes->hitPos, castRes->hitNormal);
									break;
								}
								else if (auto castRes = QueryOutterTransition(wishInputMoveVel, checkDir))
								{
									BeginOuterTransition(castRes->hitPos, castRes->hitNormal);
									break;
								}
							}
						}
						else
						{
							for (auto checkDir : {GetForward(), -GetForward()})
							{
								surfaceHintDir = checkDir;
								if (QueryInnerTransition({0.f, 1.f, 0.f}, checkDir) || QueryOutterTransition({0.f, 1.f, 0.f}, checkDir))
								{
									// Up
									surfaceHint = 0;
									break;
								}
								else if (QueryInnerTransition({0.f, -1.f, 0.f}, checkDir) || QueryOutterTransition({0.f, -1.f, 0.f}, checkDir))
								{
									// Down
									surfaceHint = 2;
									break;
								}
								else if (QueryInnerTransition({1.f, 0.f, 0.f}, checkDir) || QueryOutterTransition({1.f, 0.f, 0.f}, checkDir))
								{
									// Right
									surfaceHint = 3;
									break;
								}
								else if (QueryInnerTransition({-1.f, 0.f, 0.f}, checkDir) || QueryOutterTransition({-1.f, 0.f, 0.f}, checkDir))
								{
									// Left
									surfaceHint = 1;
									break;
								}
							}
						}
					}

					if (Math::IsNonZero(moveVel.Length2()))
					{
						const Math::Vector3 rayFrom = transform.position;
						const Math::Vector3 rayDir  = Utils::Vector::Normalize(moveVel);

						const bool isLongInput   = inputPressedTime >= cornerLongPress;
						const bool isAboutToFall = !Utils::RayHit(rayFrom + moveVel * dt, -upVector, TILE_HALF_SIZE, worldGroup);

						if (isLongInput)
						{
							if (auto castRes = Utils::RayCast(rayFrom, rayDir, innerCornerBeginTransitionDist, worldGroup))
							{
								BeginInnerTransition(castRes->hitPos, castRes->hitNormal);
							}
						}

						if (isAboutToFall)
						{
							if (auto castRes = QueryCorner(rayFrom, rayDir, outerCornerBeginTransitionDist))
							{
								BeginOuterTransition(castRes->hitPos, castRes->hitNormal);
							}
						}
					}
				}

				if (!curTunnelTransition.valid && surfaceHint >= 0)
				{
					MainEventsQueue::PushEvent(CmdShowSurfaceHint{ surfaceHint, transform.position + surfaceHintDir * TILE_HALF_SIZE });
				}

				bool isDetachJustPressed = ORIN_CONTROLS_JUST_PRESSED(aliasDrop);
				/*if (!WorldManager::instance->gamepadConnected)
				{
					if (Utils::IsNonZero(wishInputMoveVel) && wishInputMoveVel.Dot(upVector) > 0.f && !Utils::IsNonZero(keepMovingDir))
					{
						if (detachPressedTime >= detachSurfaceLongPress)
						{
							isDetachJustPressed = true;
						}
						detachPressedTime += dt;
					}
					else
					{
						detachPressedTime = 0.f;
					}
				}*/

				if (state == State::Normal && isDetachJustPressed)
				{
					SetInAirState(GetForward());
				}

				if (moveVel.Length2() < 0.1f)
				{
					activateLink = "Idle";
				}

				{
					const float angle = Math::SafeAtan2(upVector.y, upVector.x) - Math::HalfPI;
					anim->GetTransform().rotation = Utils::Vector::xyV(transform.rotation, Utils::ToDegrees(angle));
				}

				if (!canJump && !onWall && !onCeiling)
				{
					activateLink = "Fall";
				}

				if (activateLink)
				{
					anim->anim.ActivateLink(activateLink);
				}

				if (!onWall && !onCeiling && !canJump)
				{
					gravity = std::min(gravity + dt * gravityAccelSpeed, gravitySpeedMax);

					const float intertiaDir = Math::Sign(inertia);
					inertia = intertiaDir * std::max(std::abs(inertia) - dt * inertiaDecelSpeed, 0.f);

					// Try to move in opposite direction of inertia
					if (wishInputMoveVel.x * inertia < 0.f)
					{
						inertia = 0.f;
					}

					moveVel.x = Math::Clamp(moveVel.x + inertia, -airMoveSpeed, airMoveSpeed);
					moveVel.y = -gravity;

					if (auto castRes = QuerySurface(transform.position, worldGroup))
					{
						if (1.f - inputMoveVel.Dot(-castRes->hitNormal) <= 0.05f)
						{
							DoAttachToSurface(castRes->hitNormal);
						}
					}
					// Try to find some surface in advance to land on
					// This pass should help avoid issues like https://electrosoul.atlassian.net/browse/OV-115
					else if (auto castRes = Utils::RayCast(transform.position + GetForward() * 12.f * TILE_SCALE, {0.f, -1.f, 0.f}, TILE_SIZE, worldGroup))
					{
						// Do this only in case of no ground beneath the player
						if (!Utils::RayHit(transform.position, {0.f, -1.f, 0.f}, TILE_SIZE, worldGroup))
						{
							// Push the player toward the surface a little
							moveVel.x += Utils::Vector::Normalize(castRes->hitPos - transform.position).x * Utils::SafeInvert(dt);
						}
					}
					else if (TryToReflectFromShield(Utils::Vector::Normalize(moveVel), 24.f))
					{
					}

					lastMoveVel = moveVel;
				}
				else
				{
					gravity = 0.f;
					inertia = 0.f;
				}

				break;
			}
		}

		// Snap to surface
		if (state == State::Normal)
		{
			surfaceVel = {};

			const float rayLength = 24.f * TILE_SCALE;

			bool willBeOnMovingSurface = false;
			if (auto castRes = Utils::RayCast(transform.position, -upVector, rayLength, worldGroup))
			{
				if (auto res = Utils::GetMovingSurfaceVel(castRes))
				{
					willBeOnMovingSurface = true;
					surfaceVel = *res;
				}
			}


			if (willBeOnMovingSurface)
			{
				const Math::Vector3 physPos = GetPhysPos();
				const float physDt = 1.f / 60.f;
				moveVel = SnapToSurface(moveVel, physPos, rayLength, physDt);

				// Squash the player
				for (float offs : {12.f, 0.f, -12.f})
				{
					if (auto castRes = Utils::RayCast(transform.position + TILE_SCALE * offs * GetForward(), upVector, rayLength, worldGroup))
					{
						if (surfaceVel.Dot(upVector) > 0.f)
						{
							Utils::AddCameraShake(1.f);
							WorldManager::instance->KillPlayer(-upVector, DeathSource::Unknown);
							break;
						}
					}
				}
			}
			else
			{
				moveVel = SnapToSurface(moveVel, transform.position, rayLength, dt);

				// Squash the player
				for (float offs : {12.f, 0.f, -12.f})
				{
					if (auto castRes = Utils::RayCast(transform.position + TILE_SCALE * offs * GetForward(), upVector, rayLength, worldGroup))
					{
						if (auto vel = Utils::GetMovingSurfaceVel(castRes))
						{
							if (vel->Dot(upVector) < 0.f)
							{
								Utils::AddCameraShake(1.f);
								WorldManager::instance->KillPlayer(-upVector, DeathSource::Unknown);
								break;
							}
						}
					}
				}
			}
		}

		// We still want timeInAir = 0.f on the first frame after detach
		timeInAir = (!onSurface && !wasOnSurface) ? timeInAir + dt : 0.f;

		if (!wasOnSurface && onSurface)
		{
			Utils::PlaySoundEvent("event:/hero/land");
		}

		if (onSurface && Utils::IsNonZero(inputMoveVel))
		{
			if (isJustStartedMoving || totalTime >= nextFootstepAtTime)
			{
				nextFootstepAtTime = totalTime + footstepsInterval;

				Utils::PlaySoundEvent("event:/hero/Footsteps");
			}
		}		

		// Capsule
		{
			const float angle = Math::SafeAtan2(upVector.y, upVector.x);

			Transform& tm = controller->GetTransform();
			tm.SetRotation(Utils::Vector::xyV(tm.GetRotation(), Utils::ToDegrees(angle)));

			Transform& animTm = anim->GetTransform();
			tm.SetPosition(animTm.GetLocal().MulNormal(initialCapsuleOffset));

			// Try orient controller's capsule along the player's body.
			const auto contollerUp = controller->controller->GetUpDirection();
			const auto wishUp = GetForward();
			if (!Math::IsEqual(contollerUp.x, wishUp.x, 1e-4f) || !Math::IsEqual(contollerUp.y, wishUp.y, 1e-4f))
			{
				controller->controller->SetUpDirection(wishUp);

				// FIXME: This is a hack: up direction won't be applied until move() is called.
				// The engine won't call move() in case of very low velocity.
				// So, try to simulate fake move() call
				controller->Move(Utils::Vector::xy(-upVector) * 100.f, worldGroup);
				controller->Move(Utils::Vector::xy( upVector) * 100.f, worldGroup);
			}
		}

		if (impulseTime > 0.f)
		{
			if (onSurface)
			{
				impulseTime = -1.f;
				impulse = {};
			}
			else
			{
				impulseTime -= dt;
				if (impulseTime > 0.f)
				{
					moveVel += impulse;
				}
			}
		}

		controller->Move(Utils::Vector::xy(moveVel + surfaceVel), worldGroup | PhysGroup::MovementBlock);

		UpdateAnimScale();

		UpdateHitBox();
	}

	void Thing::Release()
	{
		RELEASE(footstepsSound)

		Tasks(true)->DelTask(10, this);

		Player::Release();
	}
}