
#pragma once

#include "SceneEntities/Physics/2D/PhysEntity2D.h"
#include "SceneEntities/2D/AnimGraph2D.h"
#include "Characters/Enemies/Enemy.h"
#include "Common/Events.h"

namespace Orin::Overkill
{
	class Protector : public Enemy
	{
		struct TargetEnemy
		{
			META_DATA_DECL_BASE(TargetEnemy)

			SceneEntityRef<Enemy> enemy;
		};
		
		bool addedToTargets = false;
		eastl::vector<TargetEnemy> targetEnmeies;

	public:

		virtual ~Protector() = default;

		META_DATA_DECL(Protector)

		void Init() override;
		void Play() override;
		void OnRestart(const EventOnRestart& evt);
		void Update(float dt);
		void EditorDraw(float dt);
	};
}
