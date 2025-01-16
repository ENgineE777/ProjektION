#pragma once

#include "Enemy.h"
#include "SceneEntities/Physics/2D/PhysEntity2D.h"
#include "SceneEntities/Physics/2D/PhysTriger2D.h"

#include "Common/Utils.h"
#include "World/WorldManager.h"

namespace Orin::Overkill
{
    #define DRONE_MOVEMENT_OVERRIDES(cls)\
        FLOAT_OPTIONAL_PROP(cls, overrideMoveSpeed, 0.f, "Movement", "overrideMoveSpeed", "Override: Move speed")\
        FLOAT_OPTIONAL_PROP(cls, overrideStopAtPathPointsTime, 0.f, "Movement", "overrideStopAtPathPointsTime", "Override: Stop and wait and path points (seconds)")\
        FLOAT_OPTIONAL_PROP(cls, overrideShouldStopOnlyAtEndPoints, 0.f, "Movement", "overrideShouldStopOnlyAtEndPoints", "Override: Stop movement in case the end point has been reached")\

    #define DRONE_ROTATION_OVERRIDES(cls)\
        FLOAT_OPTIONAL_PROP(cls, overrideRotationSpeed, 0.f, "Rotation", "overrideRotationSpeed", "Override: Rotation speed")\
        FLOAT_OPTIONAL_PROP(cls, overrideAimSpeed, 0.f, "Rotation", "overrideAimSpeed", "Override: Aim speed")\
        FLOAT_OPTIONAL_PROP(cls, overrideReturnSpeed, 0.f, "Rotation", "overrideReturnSpeed", "Override: Aim return speed")\

    #define DRONE_SENSOR_OVERRIDES(cls)\
        FLOAT_OPTIONAL_PROP(cls, overrideTimeToPlayerAppear, 0.f, "Sensor", "overrideTimeToPlayerAppear", "Override: Duration of wondering state")\

    #define DRONE_ALL_OVERRIDES(cls)\
        DRONE_MOVEMENT_OVERRIDES(cls)\
        DRONE_ROTATION_OVERRIDES(cls)\
        DRONE_SENSOR_OVERRIDES(cls)\

    class DroneV2 : public Enemy
    {
    protected:
        enum class State
        {
            PrepareToPatrolling,
            Patrolling,
            AimingToPlayer,
            Wondering,
            PrepareAttack,
            Attacking,
            Cooldown,
        };

        State state;

        enum class MoveMode
		{
			Right,
			Left,
		};

		MoveMode moveMode;

        float totalTime = 0.f;

        bool isStunned = false;
        float stunTimer = -1.f;
        ThunderCharge *stunSource = nullptr;

        SpriteEntity *bodySprite = nullptr;
        AnimGraph2D *bodyElectro = nullptr;
        PhysEntity2D *phys = nullptr;
        PhysTriger2D *trigger = nullptr;
        Node2D *sensorCenter = nullptr;        

        struct Sensor
        {
            META_DATA_DECL_BASE(Sensor)

            float coneAngle = 0.f;
            float directionAngle = 0.f;
            float distance = 0.f;
            float alwaysVisibleDistance = 0.f;
        };

        eastl::vector<Sensor> sensors;

        float maxTimeToPlayerAppear = 1.0f;
        float timeToPlayerAppear = -1.0f;

        bool isPlayerVisible = false;
        int playerVisibleInSensorId = -1;

        int prevIndex = 0;
        int nextIndex = 0;
        int pathStep = 1;

        float moveSpeed = 0.f;
        float rotationSpeed = 0.f;
        float aimSpeed = 0.f;
        float returnSpeed = 0.f;
        float selectNextPointTimer = -1.f;
        float stopAtPathPointsTime = 0.f;

        bool loopPath = false;
        bool shouldStopOnlyAtEndPoints = false;

        eastl::vector<Math::Vector3> pathPoints;

        Utils::Angle lastAngleToPlayer;
        WorldManager::PlayerSnapshot lastPlayerSnapshot;

        float distanceToInstantKill = -1.f;

        bool isAlarmStarted = false;

        Optional<float> overrideMoveSpeed;
        Optional<float> overrideRotationSpeed;
        Optional<float> overrideAimSpeed;
        Optional<float> overrideReturnSpeed;
        Optional<float> overrideTimeToPlayerAppear;
        Optional<float> overrideStopAtPathPointsTime;
        Optional<float> overrideShouldStopOnlyAtEndPoints;

        void ApplyTransform(Math::Vector3 wishPosition, float withAngle);
        void ApplyTransform(Math::Matrix wishTm);

        struct PathRes
        {
            Math::Vector3 moveDir;
            Math::Vector3 wishPos;
        };

        PathRes UpdatePath(float dt);
        void UpdateSensors(float dt);
        void UpdateInstantKill(float dt);

        void SelectNextPathPoint();

        virtual float UpdateAgrometer(float dt);

        virtual void DoInstantKill();

        virtual void OnPlayerVisiblityChanged(bool isVisible, int sensorId) {}
        virtual void OnPlayerAppeared() {}
        virtual void OnPlayerDisappeared() {}
        virtual void OnSelectNextPathPoint() {}
        virtual void OnSwitchPathDirection() {}

    public:
        void Init() override;
        void Release() override;
        void Play() override;
        void Update(float dt) override;

        virtual void Draw(float dt) {}
        virtual void EditorDraw(float dt);

        virtual void PreUpdate(float dt) {}
        virtual void DoUpdate(float dt) {}

        void StartAlarm();
        void StopAlarm();

        void Kill(Math::Vector3 killDir, DeathSource deathSource) override;

        void OnRestart(const EventOnRestart &evt) override;
        void OnThunderHit(Math::Vector3 hitDir, ThunderCharge *source) override;
        bool IsImmortal() override { return true; }
    };
};