#pragma once

#include "Characters/Enemies/DroneV2.h"

namespace Orin::Overkill
{
    class RotatingSaw : public DroneV2
    {
    public:
        META_DATA_DECL_BASE(RotatingSaw)

        void Init() override;
        void Release() override;
        void Play() override;

        void DoUpdate(float dt) override;

        void EditorDraw(float dt) override;
    };
}