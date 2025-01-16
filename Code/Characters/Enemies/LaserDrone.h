#pragma once

#include "DroneV2.h"

#include "SceneEntities/2D/PointLight2D.h"
#include "Root/Sounds/SoundEvent.h"

namespace Orin::Overkill
{
    class VFXEmitterInstantiable;

    class LaserDrone : public DroneV2
    {
    protected:
        // TODO: Add intervals for enabling/disabling rays
        struct LaserRay
		{
			META_DATA_DECL_BASE(LaserRay)

            SoundEvent* noiseSound = nullptr;

            Math::Vector3 hitPos;

			float directionAngle = 0.f;
			float maxLength = -1.f;

            bool isActive = false;
		};

        eastl::vector<LaserRay> laserRays;

        Node2D *laserCenter = nullptr;
        VFXEmitterInstantiable *laserHit = nullptr;
        AssetTextureRef laserBeam;

        void UpdateLasers(float dt);
        void ClearLaserHit();

        bool activeOnReset = true;
        bool active = true;

        virtual void ActionOnTrigger();

        void MakeActive(bool active);

    public:
        META_DATA_DECL_BASE(LaserDrone)

        void Init() override;
        void Release() override;
        void Play() override;

        void Draw(float dt) override;
        void EditorDraw(float dt) override;

        void Kill(Math::Vector3 killDir, DeathSource deathSource) override;

        void OnRestart(const EventOnRestart &evt) override;
        void OnThunderHit(Math::Vector3 hitDir, ThunderCharge* source) override;
        void OnCheckpointReached(const EventOnCheckpointReached& evt) override;
        void DoUpdate(float dt) override;
    };
}