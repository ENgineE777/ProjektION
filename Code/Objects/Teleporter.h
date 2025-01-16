
#pragma once

#include "SceneEntities/Physics/2D/PhysTriger2D.h"
#include "SceneEntities/2D/SpriteEntity.h"
#include "SceneEntities/2D/Node2D.h"
#include "Common/Events.h"

namespace Orin::Overkill
{
	class Teleporter : public PhysTriger2D
	{
		bool playerInside = false;
		Node2D* dest = nullptr;

	public:

		virtual ~Teleporter() = default;

		META_DATA_DECL(Teleporter)

		void Init() override;
		void Play() override;
		void Update(float dt);
		void OnContactStart(int index, SceneEntity* entity, int contactIndex) override;
		void OnContactEnd(int index, SceneEntity* entity, int contactIndex) override;
	};
}
