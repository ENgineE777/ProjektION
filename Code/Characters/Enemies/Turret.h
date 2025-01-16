#pragma once

#include "SceneEntities/2D/PointLight2D.h"
#include "Common/Utils.h"
#include "World/WorldManager.h"
#include "Enemy.h"

namespace Orin::Overkill
{
	class VFXEmitter;

	class Turret : public Enemy
	{
	protected:

		enum class State
		{
			Watching,
			Rotating,
			Wondering,
			Attacking,
			RageAttacking
		};

		float leftAngleVaule = 0.f;
		float rightAngleVaule = 0.f;

		Utils::Angle leftAngle;
		Utils::Angle rightAngle;

		Utils::Angle aimingAngle;
		Utils::Angle tragetAimingAngle;
		Utils::Angle lastPlayerAngle;
		WorldManager::PlayerSnapshot lastPlayerSnapshot;
		bool toLeftAngle = false;

		float waitTime = -1.f;
		float timeToAttackPlayer = -1.0f;
		float shootTime = -1.f;
		float shootEffectTime = 0.f;

		float keepPlayerVisibleTime = -1.f;
		float keepPlayerVisibleMaxTime;

		State state = State::Rotating;

		int shootsBeforeStop = 0;
		int curShootCount = 0;

		bool clampEditorVisualization = true;

		bool isStunned = false;
        float stunTimer = -1.f;
		ThunderCharge *stunSource = nullptr;

		SpriteEntity* baseSprite = nullptr;
		AnimGraph2D* baseElectro = nullptr;
		SpriteEntity* gunSprite = nullptr;
		AnimGraph2D* gunElectro = nullptr;
		Node2D* shootNode = nullptr;
		Node2D* searchLightNode = nullptr;
		PointLight2D* shootFlash = nullptr;
		SpriteEntity* shootMuzzle = nullptr;
		VFXEmitter* shells = nullptr;		
		

		const float shootTimeDelay = 0.175f;
		const float shootEffectMaxTime = 0.1f;
		const int shootsMaxBeforeStop = 3;
		const float viewDistance = 750.0f * TILE_SCALE;
		const float editorViewDistance = 250.0f * TILE_SCALE;
		const float viewAngle = 45.0f;
		float alwaysViewRadius = 90.0f * TILE_SCALE;
		float rotateSpeed = 600.0f;
		float agroTime = 1.0f;

		bool IsPlayerVisibleInSector();
		Utils::Angle GetAngleToPlayer();
		Math::Vector3 GetAimPos();
		void UpdateGunSpriteScale(Utils::Angle centerAngle);
		float GetTimeToAttack();

    public:
		META_DATA_DECL_BASE(Turret)

		void Init() override;
		void Play() override;
		void Update(float dt) override;
		void Kill(Math::Vector3 killDir, DeathSource deathSource) override;

		void OnRestart(const EventOnRestart &evt) override;
		void OnThunderHit(Math::Vector3 hitDir, ThunderCharge *source) override;
		bool IsImmortal() override { return true; }

		void EditorDraw(float dt);
		void StartRageAttack();
    };
}