
#pragma once

#include "SceneEntities/Physics/2D/PhysTriger2D.h"
#include "SceneEntities/2D/PointLight2D.h"
#include "Common/Events.h"
#include "TunnelDoor.h"

namespace Orin::Overkill
{
	class Checkpoint : public PhysTriger2D
	{
		bool isActive = false;
		Math::Vector3 startPos;
		SceneEntityRef<TunnelDoorEntity> door;

		void SetLigtVisibility(bool visible);

	public:

		virtual ~Checkpoint() = default;

		META_DATA_DECL(Checkpoint)

		void Init() override;

		void Release() override;
		void Play() override;
		void ActivateSelf();

		void OnCheckpointReached(const EventOnCheckpointReached& evt);

		void EditorDraw(float dt);
		void OnContactStart(int index, SceneEntity* entity, int contactIndex) override;

		void OnFinishLevel(const EventOnFinishLevel& evt);
	};
}
