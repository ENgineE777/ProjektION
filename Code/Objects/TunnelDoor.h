
#pragma once

#include "SceneEntities/Physics/2D/PhysEntity2D.h"
#include "SceneEntities/2D/AnimGraph2D.h"
#include "Characters/Enemies/Enemy.h"
#include "Common/Events.h"

namespace Orin::Overkill
{
	class TunnelDoorEntity : public PhysEntity2D
	{
		bool lockedDoor = false;
		bool unlocked = true;
		bool showTargets = false;
		bool forceToLock = false;
		bool oneWay = false;
		float waitAnimToLock = -1.0f;

		struct TargetEnemy
		{
			META_DATA_DECL_BASE(TargetEnemy)

			SceneEntityRef<Enemy> enemy;
		};

		float heartTime = 0.0f;
		eastl::vector<TargetEnemy> targetEnmeies;

	public:

		AnimGraph2D* anim = nullptr;

		virtual ~TunnelDoorEntity() = default;

		META_DATA_DECL(TunnelDoorEntity)

		void Init() override;
		void ApplyProperties() override;
		void Play() override;
		void OnRestart(const EventOnRestart& evt);
		void OnFinishLevel(const EventOnFinishLevel& evt);
		void Update(float dt);
		void EditorDraw(float dt);
		void Release() override;
		bool IsUnlocked() { return unlocked && !forceToLock && (waitAnimToLock < 0.0f); };
		void ForceToLock();
		void PlayOpen();
		void PlayClose();
	};
}
