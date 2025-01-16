#pragma once

#include "DroneV2.h"
#include "SceneEntities/2D/PointLight2D.h"

namespace Orin::Overkill
{
    class SpiderDrone : public DroneV2
    {
        SpriteEntity *explosionRadius = nullptr;

        float initialMoveSpeed;
        float initialStopAtPathPointsTime;

        float attackSpeed;
        float attackTime = 0.f;
        float maxAttackTime;
        float distanceToExplosionKill;
        float maxRadiusScaleTime;
        float gravity = 0.f;

        const float gravityAccelSpeed = 2000.0f * TILE_SCALE;
		const float gravitySpeedMax = 750.0f * TILE_SCALE;

        PathRes lastPathRes;

        eastl::vector<Math::Vector3> attackPathPoints;

        bool isDropAttack = false;

        bool onWall = false;
		bool onCeiling = false;
		bool onGround = false;

        void OnPlayerVisiblityChanged(bool isVisible, int sensorId) override;
        void OnSwitchPathDirection() override;

        void DoInstantKill() override;

        bool BuildAttackPath(float pathTime, float pathSpeed, eastl::vector<Math::Vector3> *resPath = nullptr);

        Math::Vector3 CalcOuterDirAlongSurface(Math::Vector3 curPoint, Math::Vector3 hitPos, Math::Vector3 hitNormal);

        Utils::MaybeCastRes RayCastCorner(Math::Vector3 rayFrom, Math::Vector3 rayDir, float rayLength, Math::Vector3 cornerDir, uint32_t group);
        Utils::MaybeCastRes QueryCorner(Math::Vector3 rayFrom, Math::Vector3 rayDir, float dist, Math::Vector3 surfaceNormal);

        bool TryToExplode(float radiusScale);
        void Explode(float radiusScale);

    public:
        META_DATA_DECL_BASE(SpiderDrone)

        void Init() override;
        void Release() override;
        void Play() override;

        void EditorDraw(float dt) override;

        void OnRestart(const EventOnRestart &evt) override;

        void DoUpdate(float dt) override;
    };
}