
#pragma once

#include "SceneEntities/2D/ScriptEntity2D.h"
#include "SceneEntities/2D/AnimGraph2D.h"
#include "SceneEntities/2D/SpriteEntity.h"
#include "SceneEntities/Physics/2D/KinematicCapsule2D.h"
#include "SceneEntities/2D/Node2D.h"
#include "SceneEntities/2D/GenericMarker2D.h"
#include "SceneEntities/2D/PointLight2D.h"
#include "Objects/Shield.h"
#include "Common/Utils.h"
#include "World/WorldManager.h"
#include "Enemy.h"

namespace Orin::Overkill
{
	class VFXEmitter;

	class Soldier : public Enemy
	{
		enum class State
		{
			PrepareToPatrolling,
			Patrolling,
			Wondering,
			Attacking,
			HitReaction
		};

		enum class MoveMode
		{
			Left,
			Right,
		};

		MoveMode moveMode;

		AnimGraph2D* arms = nullptr;
		AnimGraph2D* torse = nullptr;
		Node2D* shootNode = nullptr;
		Node2D* redDotNode = nullptr;
		PointLight2D* shootFlash = nullptr;
		SpriteEntity* shootMuzzle = nullptr;
		SpriteEntity* lightBeam = nullptr;
		VFXEmitter* shells = nullptr;

		Math::Vector3 lightBeamPos;

		Shield* shield = nullptr;
		SpriteEntity* shieldSprite = nullptr;
		Math::Vector3 shieldPos;
		
		Math::Vector3 leftPos;
		Math::Vector3 rightPos;

		bool moveLeft = false;
		bool hasPath = false;
		bool keepDead = false;
		float timeToAttackPlayer = -1.0f;
		float timeToAttackPlayerMult = 1.f;
		Utils::Angle aimingAngle;
		Utils::Angle tragetAimingAngle;
		Utils::Angle lastPlayerAngle;
		WorldManager::PlayerSnapshot lastPlayerSnapshot;

		State state = State::Patrolling;

		Math::Vector3 GetAimPos();
		bool IsPlayerVisibleInSector();
		bool CanSeePlayer();
		Utils::Angle GetAngleToPlayer();
		float GetDirectionToPlayer();

		float keepPlayerVisibleTime = -1.f;
		float keepPlayerVisibleMaxTime;

		float reactionToPlayerFromBehindTime = 0.f;
		float reactionToPlayerFromBehindMaxTime;

		float shootTime = 0.f;
		float shootEffectTime = 0.f;
		int shootsBeforeStop = 0;
		int curShootCount = 0;

		float moveT = 0.f;
		float switchMoveDirTime = -1.f;
		float switchLookDirInterval = -1.f;

		const float shootTimeDelay = 0.175f;
		const float shootEffectMaxTime = 0.1f;
		const int shootsMaxBeforeStop = 3;
		const float moveSpeed = 170.0f * TILE_SCALE;
		const float viewDistance = 750.0f * TILE_SCALE;
		const float viewAngle = 45.0f;
		float alwaysViewRadius = 36.0f;
		float stopAtPathPointsTime = 0.5f;
		float speedTransitionTime = 0.3f;
		float startMoveT = 0.f;
		Math::Vector2 alwaysViewBox;

		float hitReactionTime = 0.f;
		float hitReactionDuration = 0.5f;

		float totalTime = 0.f;

		float nextFootstepAtTime = 0.f;
		float footstepsInterval = 0.35f;

		float timeToTriggeredSound = -1.f;
		float maxTimeToTriggeredSound = .25f;

		void DrawRedDot();
		void VisualizeAttackTime();
		void UpdateAnimScale() override;

		float GetTimeToAttack();

	public:
		META_DATA_DECL_BASE(Soldier)

		void Init() override;
		void Release() override;
		void Play() override;

		void Update(float dt) override;

		HitResult Hit(Math::Vector3 hitDir, float damage) override;
		void Kill(Math::Vector3 killDir, DeathSource deathSource) override;

		void EditorDraw(float dt);

		void OnRestart(const EventOnRestart &evt) override;
		void OnCheckpointReached(const EventOnCheckpointReached &evt) override;
		void OnFinishLevel(const EventOnFinishLevel &evt) override;
		void OnThunderHit(Math::Vector3 hitDir, ThunderCharge *source) override;
		void OnShieldTouch();
	};
}
