
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
#include "../Enemy.h"

namespace Orin::Overkill
{
	class BubaFootFlamethrower;

	class BubaFoot : public Enemy
	{
		friend class BubaFootActivator;

		enum class State
		{
			Inactive,
			Following,
			WaitingAttack
		};
										
		State state = State::Inactive;

		float waveK = 0.0f;
		float moveSpeed = 120.0f;
		float curMoveSpeed = 0.0f;
		float moveToZoneAccel = 1000.f;

		float timeToStopAfterKill = 1.f;

		Math::Vector2 redZone;
		Math::Vector2 greenZone;

		int curPointSegment = -1;
		float lenSegment = 0.0f;
		Math::Vector2 dirSegment = 0.0f;

		Node2D* shootNode = nullptr;
		SpriteEntity* arm = nullptr;
		Node2D* aimPoint = nullptr;
		GenericMarker2D* path;
		float timeToNextFlamethrowerSFX = 0.0f;
		BubaFootFlamethrower* flamethrower = nullptr;

		float deathRayLength = 0.f;

		bool doNotAttackPlayer = false;
		bool debugDrawZones = false;

		SceneEntityRef<Node2D> data;

		Utils::RndGenerator rndGen;

		AssetPrefabRef misslePrefab;

		SoundEvent* jetpackSound = nullptr;

		void SetNextPoint();

	public:
		META_DATA_DECL_BASE(Puppet)

		void Init() override;
		void Release() override;
		void Play() override;

		void Update(float dt) override;

		void EditorDraw(float dt);

		void Kill(Math::Vector3 killDir, DeathSource deathSource) override;

		void OnRestart(const EventOnRestart &evt) override;		

		void Activate();
		void LaunchMissle(Math::Vector3 dest);
	};
}
