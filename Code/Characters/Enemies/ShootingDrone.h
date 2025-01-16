#pragma once

#include "DroneV2.h"

#include "SceneEntities/2D/PointLight2D.h"

namespace Orin::Overkill
{
    class ShootingDrone : public DroneV2
    {
        PointLight2D* shootFlash = nullptr;
		SpriteEntity* shootMuzzle = nullptr;

        float shootTime = 0.f;
        float shootEffectTime = 0.f;

        int shootsToDeadPlayerLeft = 0;
        int shootsBeforeStop = 0;
        int curShootCount = 0;

        const int shootsMaxBeforeStop = 3;
        const float shootTimeDelay = 0.175f;
        const float shootEffectMaxTime = 0.1f;

        void OnPlayerVisiblityChanged(bool isVisible, int sensorId) override;
        void OnPlayerAppeared() override;
        void OnPlayerDisappeared() override;

        void UpdateShooting(float dt);

    public:
        META_DATA_DECL_BASE(ShootingDrone)

        void Init() override;
        void Release() override;
        void Play() override;

        void OnRestart(const EventOnRestart &evt) override;

        void PreUpdate(float dt) override;
        void DoUpdate(float dt) override;
    };
}