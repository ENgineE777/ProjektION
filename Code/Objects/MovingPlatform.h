#pragma once

#include "Characters/Enemies/DroneV2.h"

namespace Orin::Overkill
{
    class MovingPlatform : public DroneV2
    {
    public:
        Math::Vector3 velocity;

        META_DATA_DECL_BASE(MovingPlatform)

        void Init() override;
        void Release() override;
        void Play() override;

        void DoUpdate(float dt) override;
    };
}