
#pragma once

#include "Player.h"
#include "Characters/Enemies/Puppet.h"
#include "SceneEntities/Physics/2D/PhysEntity2D.h"
#include "Common/Utils.h"
#include "Objects/Throwable/Throwable.h"
#include "EASTL/bonus/ring_buffer.h"

namespace Orin::Overkill
{
	class Thing : public Player
	{
		friend class WorldManager;

		enum class State
		{
			Normal,
			SurfaceTransition,
			Dash,
			TunnelTransition,
			EnteringPuppet,
			PuppetMaster
		};

		bool onWall = false;
		bool onCeiling = false;
		bool onGround = false;
		bool onSurface = false;

		float moveSpeed;
		float wallMoveSpeed;
		float airMoveSpeed;
		float dashStartSpeed;
		float dashEndSpeed;

		const float shieldImpulse = 350.0f * TILE_SCALE;
		const float dashDistanceMin = 50.0f * TILE_SCALE;
		const float gravityAccelSpeed = 2000.0f * TILE_SCALE;
		const float gravitySpeedMax = 750.0f * TILE_SCALE;
		const float inertiaDecelSpeed = 350.0f * TILE_SCALE;
		const float attackJumpTime = 0.35f;
		const float intoTunnelTime = 0.35f;
		const float rotateTunnelTime = 0.09f;
		const float intoTunnelSpeed = 250.0f * TILE_SCALE;
		const float transitionTotalTime = 0.125f;
		const Math::Vector3 killBoxSize = {24.0f, 18.0f, 8.0f};
		float surfaceMargin = 0.f;

		State state = State::Normal;
		float stateTimer = -1.0f;

		bool keepMoving = false;
		Math::Vector3 keepMovingDir;
		Math::Vector3 accumInputDelta;

		class Enemy* victim = nullptr;

		SoundEvent* footstepsSound = nullptr;

		struct SurfaceTransition
		{
			Math::Vector3 fromUpVector = {0.f, 1.f, 0.f};
			Math::Vector3 toUpVector   = {0.f, 1.f, 0.f};
			Math::Vector3 fromPos = {0.f, 1.f, 0.f};
			Math::Vector3 toPos   = {0.f, 1.f, 0.f};

			bool isVaild = false;
		};

		SurfaceTransition curTransition;
		SurfaceTransition afterDashTransition;

		float gravity  = 0.f;
		float inertia = 0.f;
		Math::Vector3 initialCapsuleOffset;
		Math::Vector3 upVector = {0.f, 1.f, 0.f};
		Math::Vector3 surfaceNormal;
		Math::Vector3 surfaceVel;
		Math::Vector3 curDirAlongSurface;
		float surfaceTraceOffset;
		bool isDashAllowed = false;

		Math::Vector3 dashDir;
		Math::Vector3 aimDir;
		Math::Vector3 aimDirUnclamped;
		Math::Vector3 dashStartPos;
		Math::Vector3 dashEndPos;
		float dashMoveDistance;
		float dashT;
		eastl::vector<Enemy*> hitEnemies;

		PhysEntity2D* hitBox;

		struct DrawDashResult
		{
			Math::Vector3 pos;
			Math::Vector3 normal;
			bool isInAir;
		};
		DrawDashResult drawDashRes;

		struct DebugLine
		{
			Math::Vector3 from;
			Math::Vector3 to;
			Color color;
			float ttl;
			bool alive;
		};
		eastl::vector<DebugLine> debugLines;

		struct DebugSphere
		{
			Math::Vector3 pos;
			float radius;
			Color color;
			float ttl;
			bool alive;
		};
		eastl::vector<DebugSphere> debugSpheres;
		eastl::vector<eastl::string> debugText;

		struct TunnelTransition
		{
			Math::Vector3 tunnelDir;
			Math::Vector3 tunnelEntrancePos;
			Math::Vector3 tunnelExitPos;
			Math::Vector3 tunnelExitDir;
			Math::Vector3 tunnelExitUp;
			Math::Vector3 hintPos;
			class TunnelDoorEntity* door = nullptr;

			float distToTunnelEntrancePos = 0.f;
			float addTunnelSpeed = 0.f;
			float startTime = 0.f;

			bool valid;
		};

		TunnelTransition curTunnelTransition;

		ThrowableDesc* throwableDesc2Pickup = nullptr;
		Math::Vector3 throwableStandPos;
		bool throwableAutoGrab = false;

		ThrowableDesc* throwableDesc2Throw = nullptr;

		Math::Vector3 impulse;
		float impulseTime;

		float tunnelBufferingTime = 0.1f;

		float totalTime = 0.f;
		float inputPressedTime = 0.f;
		float timeInAir = 0.f;
		float lastDashPressedAtTime = -1.f;
		float lastTunnelPressedAtTime = -1.f;
		float dashMaxDistance = 0.f;
		float detachPressedTime = 0.f;

		float innerCornerBeginTransitionDist = 24.f;
		float outerCornerBeginTransitionDist = 24.f;
		float cornerTransitionByKeyDist = 32.f;
		float dashCoyoteTime = 0.1f;
		float dashBufferingTime = 0.1f;
		float dashThresholdDegress = 10.f;
		float cornerLongPress = 0.15f;
		float tunnelActivationDist = 32.f;
		float detachSurfaceLongPress = 0.15f;
		float moveAlongSurfaceTransitionDist = 16.f;

		float lastThrowPressedAtTime = -1.0f;
		float throwBufferingTime = 0.25f;
		bool throwIsBlocked = false;

		Color dashSuccessColor;
		Color dashFailColor;

		int lastTrail = -1;
		SpriteEntity* dashTrail = nullptr;
		SpriteEntity* dashTrailEnd = nullptr;

		bool slashDash = false;

		SpriteEntity* dashTrailAttack = nullptr;
		SpriteEntity* dashTrailAttackEnd = nullptr;

		SpriteEntity* dashSlash = nullptr;
		SpriteEntity* dashSlash2 = nullptr;

		int killAssistanceCount = 0;
		int safeAssistanceCount = 0;

		int aliasMoveHorz;
		int aliasMoveVert;
		int aliasAimHorz;
		int aliasAimVert;
		int aliasAimMainHorz;
		int aliasAimMainVert;
		int aliasDash;
		int aliasUse;
		int aliasDrop;
		int aliasShoot;
		int aliasCursorX;
		int aliasCursorY;

		bool teleportAvailable = false;
		Math::Vector3 teleportDest;
		const float puppetEnteriingTime = 0.3f;
		Math::Vector3 puppetEnteringPos;
		Puppet* owningPuppet = nullptr;

		float teleportDashZoneTime = -1.f;
		Math::Vector3 lastForwardOnSurface;

		Math::Vector3 lastInputMoveVel;
		float nextFootstepAtTime = 0.f;
		float footstepsInterval = 0.1f;

		float invincibilityTime = -1.0f;

		bool debugDash = false;

		float delayedOutro = -1.0f;
		// struct DashRecord
		// {
		// 	int id;
		// 	eastl::string scene;
		// 	Math::Vector2 aimDir;
		// 	Math::Vector2 aimDirUnclamped;
		// 	Math::Vector2 dashStartPos;
		// 	Math::Vector2 dashEndPos;
		// 	float dashLength;
		// 	eastl::string dashMeta;
		// };

		// eastl::vector<DashRecord> records;

	private:

		void SetState(State newState, float timer);

		Utils::MaybeCastRes QueryCorner(Math::Vector3 rayFrom, Math::Vector3 rayDir, float dist);
		Utils::MaybeCastRes QuerySurface(Math::Vector3 pos, uint32_t group, Math::Vector3 *moveDir = nullptr, Math::Vector3 *priorityDir = nullptr);
		bool TryAttachToSurface(uint32_t group, Math::Vector3 *moveDir = nullptr, Math::Vector3 *priorityDir = nullptr);
		void DoAttachToSurface(Math::Vector3 normal);

		enum class SetUpVector { No, Yes };
		void SetSurfaceNormal(Math::Vector3 normal, SetUpVector setUpVector = SetUpVector::No);
		void SetInAirState(Math::Vector3 forward, Math::Vector3 up = {0.f, 1.f, 0.f});

		Utils::MaybeCastRes RayCastCorner(Math::Vector3 rayFrom, Math::Vector3 rayDir, float rayLength, Math::Vector3 cornerDir, uint32_t group);

		void TryKillEnemyInDirection(Math::Vector3 dir);

		float GetDashSpeed(float t, Math::Vector3 dir, float distance) const;

		void DashAssistance();

		void ShowTunnelHint(Math::Vector3 bottomPos, Math::Vector3 hitNormal);
		bool TryToActivateTunnel(bool shouldActivateTunnel);
		bool TryToDash(bool isDashJustPressed);
		void UpdateDash(float dt, bool shouldActivateTunnel, Math::Vector3 &moveVel);
		Utils::MaybeCastRes FindClosestDash(Math::Vector3 endPos, Math::Vector3 endNormal);

		bool TryToReflectFromShield(Math::Vector3 dir, float dist);

		Math::Vector3 SnapToSurface(Math::Vector3 moveVel, Math::Vector3 rayFrom, float rayLength, float dt);

		Math::Vector3 GetPhysPos();

		void UpdateHitBox();

		void HideAllDashTrails();

	public:
		META_DATA_DECL_BASE(Thing)

		void Init() override;
		void Play() override;
		void Update(float dt) override;

		void Kill(Math::Vector3 killDir, DeathSource deathSource) override;

		void Draw(float dt);
		void UpdateThrowable(float dt);

	 	Math::Vector3 GetUp() const override { return upVector; }
		Math::Vector3 GetForward() const override
		{
			return -GetDirection() * Utils::Vector::Rotate90(Utils::Vector::xy(upVector));
		}

		const char* GetBodyPartsTrail() override;

		void Release() override;

		void OnRestart(const EventOnRestart &evt) override;
		void OnFinishLevel(const EventOnFinishLevel &evt) override;
		void OnThrowable2Pickup(ThrowableDesc* throwable, Math::Vector3 standPos, bool autoGrab) override;
		void TeleportState(const CmdTeleportState& evt);
		bool HasPuppet() override;
		void SetPuppet(Puppet* setPuppet) override;
		bool IsTutorialHintAllowed() override { return !owningPuppet || (owningPuppet && !owningPuppet->InCutSceneIntro()); };
		bool CanGrabThrowable() override;
		void MakeInvincibile(float time) override;
	};
}
