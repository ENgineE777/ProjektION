#pragma once

#include "Characters/Enemies/DroneV2.h"

namespace Orin::Overkill
{
    class DynamicSaw : public DroneV2
    {
    public:
        META_DATA_DECL_BASE(DynamicSaw)

        void Init() override;
        void Release() override;
        void Play() override;

        void DoUpdate(float dt) override;
    };
}