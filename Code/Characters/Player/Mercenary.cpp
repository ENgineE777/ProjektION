
#include "Mercenary.h"
#include "Characters/Enemies/Enemy.h"
#include "Root/Root.h"
#include "World/WorldManager.h"
#include "Characters/Enemies/Soldier.h"

#define ORIN_CONTROLS GetRoot()->GetControls()
#define ORIN_CONTROLS_JUST_PRESSED(alias) ORIN_CONTROLS->GetAliasState(alias, AliasAction::JustPressed)
#define ORIN_CONTROLS_PRESSED(alias) ORIN_CONTROLS->GetAliasState(alias, AliasAction::Pressed)
#define ORIN_CONTROLS_VALUE(alias) ORIN_CONTROLS->GetAliasValue(alias, false)

namespace Orin::Overkill
{
	ENTITYREG(SceneEntity, Mercenary, "Overkill/Characters", "Mercenary")

	META_DATA_DESC(Mercenary)
		BASE_SCENE_ENTITY_PROP(Mercenary)
	META_DATA_DESC_END()

	void Mercenary::Play()
	{
		Player::Play();

		torso = FindChild<AnimGraph2D>("torso");		
	}

	void Mercenary::OnRestart(const EventOnRestart& evt)
	{
		Player::OnRestart(evt);

		gravity = 0.0f;

		onWall = false;
		shootCooldown = -1.0f;
		wallClimbFall = 0.0f;
		wallClimbJumbTime = -1.0f;

		torso->SetVisiblity(true);
		torso->anim.GotoNode("Idle", false);

		state = State::Normal;
	}

	void Mercenary::OnFinishLevel(const EventOnFinishLevel& evt)
	{
		Player::OnFinishLevel(evt);

		torso->anim.GotoNode("Idle", false);
	}

	void Mercenary::Kill(Math::Vector3 killDir, DeathSource deathSource)
	{
		if (killed)
		{
			return;
		}

		Player::Kill(killDir, deathSource);

		torso->SetVisiblity(false);
	}

	Math::Vector3 Mercenary::GetAimPos()
	{
		return Sprite::ToPixels(torso->GetTransform().GetGlobal().Pos());
	}

	void Mercenary::Init()
	{
		Player::Init();

		aliasMoveHorz    = ORIN_CONTROLS->GetAlias("Hero.MOVE_HORZ");
		aliasMoveVert    = ORIN_CONTROLS->GetAlias("Hero.MOVE_VERT");
		aliasAimHorz     = ORIN_CONTROLS->GetAlias("Hero.AIM_HORZ");
		aliasAimVert     = ORIN_CONTROLS->GetAlias("Hero.AIM_VERT");
		aliasAimMainHorz = ORIN_CONTROLS->GetAlias("Hero.AIM_MAIN_HORZ");
		aliasAimMainVert = ORIN_CONTROLS->GetAlias("Hero.AIM_MAIN_VERT");
		aliasShoot       = ORIN_CONTROLS->GetAlias("Mercenary.SHOOT");
		aliasDash        = ORIN_CONTROLS->GetAlias("Mercenary.DASH");
		aliasCursorX     = ORIN_CONTROLS->GetAlias("Hero.CURSOR_X");
		aliasCursorY     = ORIN_CONTROLS->GetAlias("Hero.CURSOR_Y");
	}

	void Mercenary::Update(float dt)
	{
		if (killed || levelFinished)
		{
			return;
		}

		Math::Vector2 moveDir = 0.0f;
		const char* animNode = "idle";

		auto shootPos = GetAimPos();
		Math::Vector3 shootDir;
		Math::Vector3 wishDashDir;

		if (WorldManager::instance->gamepadConnected)
		{
			float aimHorz = ORIN_CONTROLS_VALUE(aliasAimHorz);
			float aimVert = ORIN_CONTROLS_VALUE(aliasAimVert);
			aimHorz = std::abs(aimHorz) < 0.08f ? 0.f : aimHorz;
			aimVert = std::abs(aimVert) < 0.08f ? 0.f : aimVert;

			float aimMainHorz = ORIN_CONTROLS_VALUE(aliasAimMainHorz);
			float aimMainVert = ORIN_CONTROLS_VALUE(aliasAimMainVert);
			aimMainHorz = std::abs(aimMainHorz) < 0.08f ? 0.f : aimMainHorz;
			aimMainVert = std::abs(aimMainVert) < 0.08f ? 0.f : aimMainVert;

			const auto aimMainDir = Utils::Vector::Normalize(Math::Vector3{aimMainHorz, aimMainVert, 0.f});
			if (Utils::IsNonZero(aimMainDir))
			{
				shootDir = aimMainDir;
			}
			else
			{
				shootDir = Utils::Vector::Normalize(Math::Vector3{aimHorz, aimVert, 0.f});
			}

			if (Utils::IsNonZero(shootDir))
			{
				const float stepDeg = 5.f;
				const float aimAngle = roundf(Utils::Angle::FromDirection(shootDir).ToDegrees() / stepDeg) * stepDeg;
				shootDir = Utils::Angle::Degrees(aimAngle).ToDirection();
			}
			else
			{
				shootDir = Math::Vector3{GetDirection(), 0.f, 0.f};
			}

			wishDashDir = shootDir;
		}
		else
		{
			const Math::Vector3 cursorPos = Utils::Vector::xyV(Utils::GetCursorPos(aliasCursorX, aliasCursorY), transform.position.z);
			shootDir    = cursorPos - shootPos;
			wishDashDir = cursorPos - transform.position;
		}

		shootDir.Normalize();
		wishDashDir.Normalize();

		float vertMoveValue = ORIN_CONTROLS_VALUE(aliasMoveVert);
		vertMoveValue = std::abs(vertMoveValue) < 0.5f ? 0.f : Math::Sign(vertMoveValue);

		torso->GetTransform().rotation = Math::Vector3(0.0f, 0.0f, Utils::ToDegrees(Math::SafeAtan2(shootDir.y, shootDir.x)) + (shootDir.x > 0.0f ? 0.0f : 180.0f));

		{
			float scale = fabs(torso->GetTransform().scale.x);
			torso->GetTransform().scale = Math::Vector3((shootDir.x > 0.0f ? 1.0f : -1.0f) * scale, scale, 1.0f);
		}

		gravity -= dt * 1450.0f;

		if (gravity < -850.0f)
		{
			gravity = -850.0f;
		}

		shootCooldown -= dt;

		if (shootCooldown < 0.0f)
		{
			shootCooldown = -1.0f;

			if (ORIN_CONTROLS_JUST_PRESSED(aliasShoot))
			{
				shootCooldown = shootCooldownTime;

				Math::Vector3 shootToPos;
				if (auto castRes = Utils::RayCast(GetAimPos(), shootDir, 1500.f, PhysGroup::WorldDoorsEnemies))
				{
					shootToPos = castRes->hitPos;

					if (castRes->userdata)
					{
						Soldier* soldier = dynamic_cast<Soldier*>(castRes->userdata->object->GetParent());

						if (soldier)
						{
							soldier->Kill(shootDir, DeathSource::Unknown);
						}
					}
				}
				else
				{
					shootToPos = shootPos + shootDir * 1500.f;
				}

				WorldManager::instance->AddBulletTrace(shootPos, shootToPos);
				//PlaySound("SFX/railgunsound.mp3");
				Utils::AddCameraShake(0.5f);
			}
		}

		switch (state)
		{
		case State::CellAttach:
			animNode = "wallClimb";

			dashDir = wishDashDir;

			if (ORIN_CONTROLS_JUST_PRESSED(aliasDash))
			{
				flipped = dashDir.x < 0.0f;
				state = State::Dash;
				dashTimer = dashTime;
				skipColliderCheck = true;
				torso->SetVisiblity(false);
			}
			else
			if (vertMoveValue < 0.)
			{
				state = State::Normal;
				gravity = 0.0f;
			}

			break;
		case State::Dash:

			animNode = "roll";

			moveDir.x = dashDir.x * dashSpeed;
			moveDir.y = dashDir.y * dashSpeed;

			dashTimer -= dt;

			if (dashTimer < 0.0f)
			{
				gravity = 0.0f;
				state = State::Normal;
			}
			else
			{
				const Math::Vector3 rayFrom = transform.position + Math::Vector3(0.0f, 5.0f, 0.0f);
				const Math::Vector3 rayDir = { GetDirection(), 0.0f, 0.0f };

				auto castResLow = Utils::RayCast(rayFrom - Math::Vector3(0.0f, 25.0f, 0.0f), rayDir, 30.0f, PhysGroup::World);
				auto castResHi = Utils::RayCast(rayFrom + Math::Vector3(0.0f, 17.0f, 0.0f), rayDir, 30.0f, PhysGroup::World);

				if (castResLow || castResHi || (!skipColliderCheck && controller->controller->IsColliding(PhysController::CollideDown)))
				{
					state = State::Normal;
					moveDir = 0.0f;
					gravity = 0.0f;

					torso->SetVisiblity(true);
				}
				else
				if (!skipColliderCheck && controller->controller->IsColliding(PhysController::CollideUp))
				{
					state = State::CellAttach;
					moveDir = 0.0f;
					gravity = 0.0f;

					torso->SetVisiblity(true);
				}

				skipColliderCheck = false;
			}

			break;
		case State::WallClimbJumb:

			wallClimbJumbTime += dt;

			if (wallClimbJumbTime > 0.3f)
			{
				state = State::Normal;
			}
			else
			{
				state = State::WallClimbJumb;

				moveDir.x = GetDirection() * runSpeed;
				moveDir.y = gravity;
			}

			break;
		case State::Normal:
		
			bool onGround = controller->controller->IsColliding(PhysController::CollideDown);

			if (!onWall)
			{
				moveDir.x = ORIN_CONTROLS_VALUE(aliasMoveHorz);
				moveDir.x = std::abs(moveDir.x) < 0.5f ? 0.f : Math::Sign(moveDir.x);

				if (Math::IsNonZero(moveDir.x))
				{
					flipped = moveDir.x < 0.f;
				}
			}

			if (fabs(moveDir.x) > 0.1f)
			{
				float scale = fabs(anim->GetTransform().scale.x);
				anim->GetTransform().scale = Math::Vector3(moveDir.x * scale, scale, 1.0f);
			}

			moveDir.x *= runSpeed;

			if (!onGround || vertMoveValue > 0.f)
			{
				const Math::Vector3 rayFrom = transform.position + Math::Vector3(0.0f, 5.0f, 0.0f);
				const Math::Vector3 rayDir = { GetDirection(), 0.0f, 0.0f };

				auto castResLow = Utils::RayCast(rayFrom - Math::Vector3(0.0f, 25.0f, 0.0f), rayDir, 30.0f, PhysGroup::World);
				auto castResHi = Utils::RayCast(rayFrom + Math::Vector3(0.0f, 17.0f, 0.0f), rayDir, 30.0f, PhysGroup::World);

				if (castResLow || castResHi)
				{
					if (!onWall)
					{
						wallClimbFall = 0.0f;
					}

					onWall = true;
					gravity = 0.0f;
					animNode = "wallClimb";
				}
				else
				{
					onWall = false;
				}
			}

			if (!onWall)
			{
				moveDir.y = gravity;

				if (gravity > 0.0f)
				{
					animNode = "jump";
				}
				else
				{
					animNode = "fall";
				}
			}
			else
			{
				if (vertMoveValue > 0.f)
				{
					wallClimbFall = 0.0f;
					animNode = "roll";
					moveDir.y = runSpeed;
					moveDir.x = GetDirection() * slideSpeed;
					torso->SetVisiblity(false);
				}
				else
				{
					wallClimbFall += dt;

					moveDir.y = -(Math::Clamp(wallClimbFall, 0.1f, 0.5f) - 0.1f) / 0.4f * 455.0f;

					torso->SetVisiblity(true);
				}
			}			

			if (gravity < 0.0f && onGround)
			{
				gravity = 0.0f;
				onWall = false;

				torso->SetVisiblity(true);

				if (fabs(moveDir.x) > 0.1f)
				{
					animNode = "run";
				}
				else
				{
					animNode = "idle";
				}

				/*if (GetRoot()->GetControls()->DebugKeyPressed("KEY_SPACE", AliasAction::JustPressed))
				{
					gravity = 575.0f;
				}*/
			}

			if (onWall)
			{
				/*if (GetRoot()->GetControls()->DebugKeyPressed("KEY_SPACE", AliasAction::JustPressed))
				{
					state = State::WallClimbJumb;
					wallClimbJumbTime = 0.0f;
					flipped = !flipped;
					gravity = 575.0f;

					torso->SetVisiblity(true);
				}*/
			}

			{
				dashDir = wishDashDir;

				if (ORIN_CONTROLS_JUST_PRESSED(aliasDash))
				{
					flipped = dashDir.x < 0.0f;
					state = State::Dash;
					dashTimer = dashTime;
					skipColliderCheck = true;
					torso->SetVisiblity(false);
				}
			}
			
			break;
		}

		int dbgLine = 1;
		//Utils::DebugText(dbgLine++, "dashTimer: %4.3f", dashTimer);

		float offset = GetRoot()->GetRender()->GetDevice()->GetWidth() * 0.5f - 50.0f;
		if (shootCooldown < -0.1f)
		{
			GetRoot()->GetRender()->DebugSprite(nullptr, { offset, 10.0f }, { 100.0f, 15.0f }, COLOR_GREEN);
		}
		else
		{
			GetRoot()->GetRender()->DebugSprite(nullptr, { offset, 10.0f }, { 100.0f, 15.0f }, COLOR_CYAN);
			GetRoot()->GetRender()->DebugSprite(nullptr, { offset, 10.0f }, { 100.0f * (1.0f - shootCooldown / shootCooldownTime), 15.0f }, COLOR_YELLOW);
		}

		controller->Move(moveDir, 1);

		anim->anim.GotoNode(animNode, false);
		torso->anim.GotoNode(animNode, false);
	}
}