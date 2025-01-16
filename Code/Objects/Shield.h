
#pragma once

#include "SceneEntities/Physics/2D/PhysEntity2D.h"

namespace Orin::Overkill
{
	class Enemy;

	class Shield : public PhysEntity2D
	{
	public:
		Enemy *enemyOwner = nullptr;

		virtual ~Shield() = default;

		META_DATA_DECL(Shield)

        Math::Vector3 GetWorldPos()
        {
            return Sprite::ToPixels(transform.GetGlobal().Pos());
        }

        void Init() override;
	};
}
