
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
#include "Enemy.h"

namespace Orin::Overkill
{
	class Puppet : public Enemy
	{
		enum class State
		{
			Patrolling,
			AcceptMaster,
			ObeyMaster,
			Respawn,
			WaitRocket
		};

		enum class MoveMode
		{
			Left,
			Right,
		};

		enum class Mode
		{
			Normal,
			TutorialInto,
			CutSceneIntro,
			PreCutSceneIntro
		};

		Mode mode = Mode::Normal;

		MoveMode moveMode;

		Math::Vector3 leftPos;
		Math::Vector3 rightPos;

		bool moveLeft = false;
		bool hasPath = false;
		bool isCorpse = false;
		bool needDelete = false;
		bool keepDead = false;
			
		float moveT = 0.f;
		float switchMoveDirTime = -1.f;
		float switchLookDirInterval = -1.f;

		float stopAtPathPointsTime = 0.5f;
		float speedTransitionTime = 0.3f;
		float startMoveT = 0.f;

		State state = State::Patrolling;

		const float moveSpeed = 170.0f * TILE_SCALE;
		const float moveMasterSpeed = 300.0f * TILE_SCALE;
		float moveMaster = 0.0f;

		float respawnTime = 0.f;
		float respawnMaxTime;
		Math::Vector3 respawnPos;
		int reswapnCount = 0;

		float footstepTimer = 0.f;
		float footstepsInterval = 0.35f;

		SoundEvent* breahSound = nullptr;		

	public:

		float timeToWaitRocket = -1.0f;

		META_DATA_DECL_BASE(Puppet)

		void Play() override;

		void Update(float dt) override;

		void Kill(Math::Vector3 killDir, DeathSource deathSource) override;

		void OnRestart(const EventOnRestart &evt) override;
		void OnCheckpointReached(const EventOnCheckpointReached &evt) override;
		void OnFinishLevel(const EventOnFinishLevel &evt) override;
		void OnThunderHit(Math::Vector3 hitDir, ThunderCharge *source) override;
		void StartAccept();
		void StartObey();
		void MoveMaster(bool left);
		void TeleportMaster(Math::Vector3 pos);

		bool IsReadyToObey();
		bool IsWaitRocket() { return state == State::WaitRocket && timeToWaitRocket < 0.0f; };
		bool AllowInnerDash() { return (mode == Mode::Normal || mode == Mode::TutorialInto); };
		void SwitchToNormal() { mode = Mode::Normal; };
		void SwitchToTutorialInto() { mode = Mode::TutorialInto; };
		void SwitchToCutSceneIntro() { mode = Mode::CutSceneIntro; };
		bool InCutSceneIntro() { return (mode == Mode::CutSceneIntro || mode == Mode::PreCutSceneIntro); };
		void SwitchToPreCutSceneIntro(bool setFlipped) { flipped = setFlipped;  mode = Mode::PreCutSceneIntro; };

		void Release() override;
	};
}
