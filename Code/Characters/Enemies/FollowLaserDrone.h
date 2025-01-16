#pragma once

#include "LaserDrone.h"

#include "SceneEntities/Physics/2D/PhysTriger2D.h"

namespace Orin::Overkill
{
    class FollowLaserDrone : public LaserDrone
    {
        float initialMoveSpeed = 0.f;

        bool isTriggerActive = false;

        void Activate(bool isActive);

        void OnSelectNextPathPoint() override;

        void ActionOnTrigger() override;

    public:
        META_DATA_DECL_BASE(FollowLaserDrone)

        void Play() override;

        void EditorDraw(float dt) override;

        void Kill(Math::Vector3 killDir, DeathSource deathSource) override;

        void OnRestart(const EventOnRestart &evt) override;
    };
}