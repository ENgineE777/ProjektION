
#pragma once

#include "Objects/TimedInstantPlayerKiller.h"
#include "Common/Constants.h"

namespace Orin::Overkill
{
	class MovingInstantPlayerKiller : public TimedInstantPlayerKiller
	{
	public:

		float moveSpeed = 300.f * TILE_SCALE;

		Optional<float> optMoveSpeed;

		enum class State
		{
			PreActivation,
			Moving,
			Waiting
		};

		Math::Vector3 fromPos;
		Math::Vector3 toPos;
		bool hasStartPoint = false;
		Math::Vector3 moveDir;
		Math::Vector3 startPos;
		State state = State::PreActivation;
		bool pathSet = false;

		void SetPosition(Math::Vector3 pos);

	public:

		virtual ~MovingInstantPlayerKiller() = default;

		META_DATA_DECL(MovingInstantPlayerKiller)

		void Play() override;
		void OnRestart(const EventOnRestart& evt) override;
		void Update(float dt) override;
	};
}
