#pragma once

#include "DroneV2.h"
#include "SceneEntities/2D/PointLight2D.h"

namespace Orin::Overkill
{
    class ChargeDrone : public DroneV2
    {
        bool shouldStopAttacking = false;

        Math::Vector3 moveDir;
        Math::Vector3 moveStart;

        float speed;
        float maxSpeed;
        float attackTime = 0.f;
        float prepareAttackTime;
        float maxAttackTime;

        bool rotateToPlayer = false;

        float droneSphereRadius;
        Math::Vector3 droneSpherePos;

        float UpdateAgrometer(float dt) override;

        void OnPlayerVisiblityChanged(bool isVisible, int sensorId) override;
        void OnPlayerAppeared() override;

        float GetPrepareAttackTime();

    public:
        META_DATA_DECL_BASE(ChargeDrone)

        void Init() override;
        void Release() override;
        void Play() override;

        void Agro();
        void OnRestart(const EventOnRestart &evt) override;

        void DoUpdate(float dt) override;
    };
}