
#pragma once

#include "SceneEntities/2D/SpriteEntity.h"
#include "Common/Events.h"

namespace Orin::Overkill
{
	class LevelRating : public SceneEntity
	{	
		float scoreTime = 1.0f;

		int scoreA = 0;
		int scoreB = 0;
		int scoreC = 0;
		int scoreD = 0;

	public:

		virtual ~LevelRating() = default;

		META_DATA_DECL(LevelRating)

		void Init() override;
		int GetTimeScore(float time);
		int GetRating(int score);
	};
}
