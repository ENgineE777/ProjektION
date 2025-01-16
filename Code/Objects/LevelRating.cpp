
#include "LevelRating.h"
#include "Root/Root.h"
#include "World/WorldManager.h"


namespace Orin::Overkill
{
	ENTITYREG(SceneEntity, LevelRating, "Overkill/Objects", "LevelRating")

	META_DATA_DESC(LevelRating)
		BASE_SCENE_ENTITY_PROP(LevelRating)
		FLOAT_PROP(LevelRating, scoreTime, 60.0f, "Game", "scoreTime", "scoreTime")
		INT_PROP(LevelRating, scoreA, 10000, "Game", "scoreA", "scoreA")
		INT_PROP(LevelRating, scoreB, 8000, "Game", "scoreB", "scoreB")
		INT_PROP(LevelRating, scoreC, 6000, "Game", "scoreC", "scoreC")
		INT_PROP(LevelRating, scoreD, 4000, "Game", "scoreD", "scoreD")
		
	META_DATA_DESC_END()

	void LevelRating::Init()
	{
		transform.objectType = ObjectType::Object2D;
		transform.transformFlag = (TransformFlag)(TransformFlag::SpriteTransformFlags ^ TransformFlag::RectSizeXY);	
	}

	int LevelRating::GetTimeScore(float time)
	{
		if (time < scoreTime)
		{
			return (int)(3000.0f + (scoreTime - time) * 100);
		}

		if (time - scoreTime > 30.0f)
		{
			return 0;
		}

		return (int)(3000.0f - (time - scoreTime) * 100.0f);

	}

	int LevelRating::GetRating(int score)
	{
		if (score > scoreA)
		{
			return 0;
		}

		if (score > scoreB)
		{
			return 1;
		}

		if (score > scoreC)
		{
			return 2;
		}

		return 3;
	}
}