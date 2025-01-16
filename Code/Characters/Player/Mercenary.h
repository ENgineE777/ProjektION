
#pragma once

#include "Characters/Player/Player.h"

namespace Orin::Overkill
{
	class Mercenary : public Player
	{
		enum class State
		{
			Normal,
			WallClimbJumb,
			Dash,
			CellAttach
		};

		State state = State::Normal;

		float gravity = 0.0f;
		AnimGraph2D* torso = nullptr;

		Math::Vector3 GetAimPos();
		bool onWall = false;

		Math::Vector3 dashDir;

		float shootCooldown = -1.0f;
		float wallClimbFall = 0.0f;
		float wallClimbJumbTime = -1.0f;
		float dashTimer = -1.0f;

		const float dashSpeed = 900.0f;
		const float runSpeed = 400.0f;
		const float slideSpeed = 325.0f;
		const float shootCooldownTime = 1.5f;
		const float dashTime = 0.45f;
		bool skipColliderCheck = false;

		int aliasMoveHorz;
		int aliasMoveVert;
		int aliasAimHorz;
		int aliasAimVert;
		int aliasAimMainHorz;
		int aliasAimMainVert;
		int aliasShoot;
		int aliasDash;
		int aliasCursorX;
		int aliasCursorY;

	public:
		META_DATA_DECL_BASE(Mercenary)

		void Init() override;
		void Play() override;
		void OnRestart(const EventOnRestart& evt) override;
		void OnFinishLevel(const EventOnFinishLevel& evt) override;
		void Kill(Math::Vector3 killDir, DeathSource deathSource) override;
		Math::Vector3 GetUp() const override { return Math::Vector3{0.f, 1.f, 0.f}; }
		Math::Vector3 GetForward() const override
		{
			return Math::Vector3{GetDirection(), 0.f, 0.f};
		}
		void Update(float dt) override;
	};
}
