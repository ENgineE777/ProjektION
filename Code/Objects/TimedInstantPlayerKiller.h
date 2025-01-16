
#pragma once

#include "SceneEntities/Physics/2D/PhysTriger2D.h"
#include "SceneEntities/2D/AnimGraph2D.h"
#include "Common/Events.h"

namespace Orin::Overkill
{
	class TimedInstantPlayerKiller : public PhysTriger2D
	{
	protected:
		AnimGraph2D* spikes = nullptr;

		bool active = true;
		bool startAsActive = true;

		float timeActive = 0.6f;
		float timeInactive = 0.6f;
		float startTimerOffset = 0.0f;
		float preActivationTime = 0.3f;

		float timer = 0.0f;

	public:

		Optional<float> optTimeActive;
		Optional<float> optTimeInactive;
		Optional<float> optPreActivationTime;

		void SetActive(bool active);

		virtual ~TimedInstantPlayerKiller() = default;

		META_DATA_DECL(TimedInstantPlayerKiller)

		void Init() override;
		void Play() override;
		void Release() override;
		void OnContactStart(int index, SceneEntity* entity, int contactIndex) override;
		virtual void OnRestart(const EventOnRestart& evt);
		virtual void Update(float dt);
		void EditorDraw(float dt);
	};
}
