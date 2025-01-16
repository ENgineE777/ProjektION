
#pragma once

#include "SceneEntities/2D/ScriptEntity2D.h"
#include "SceneEntities/2D/Camera2D.h"
#include "Common/Utils.h"
#include "Common/Events.h"

#include "EASTL/array.h"

namespace Orin::Overkill
{
	class CameraShaker : public ScriptEntity2D
	{
        SceneEntityRef<Camera2D> cameraRef;

        Utils::RndGenerator rndGenerator;
		eastl::array<Utils::PerlinNoiseGenerator, 3> perlinNoiseGenerators;

        bool allowMove = true;

        // Position
        int octavesOffset;
        float persistenceOffset;

        float maxOffsetX;
        float maxOffsetY;

        // Rotataion
        int octavesRotation;
        float persistenceRotation;

        float maxAngle;

        // Overall shake properties
        float totalTimeMul;
        float reduceTraumaMul;

        float totalTime = 0.f;
        float trauma = 0.f;

        int aliasCameraMoveActive;
		int aliasCameraMoveHorz;
		int aliasCameraMoveVert;
        int aliasCursorX;
		int aliasCursorY;
		Math::Vector2 startCameraPos;
		Math::Vector2 cameraMove;
		Math::Vector2 wishCameraMove;
    public:
        META_DATA_DECL(CameraShaker);

        void Release();
		void Play() override;
		void Update(float dt) override;

        void AddShakeTrauma(const CmdAddShakeTrauma &evt);
        void AllowCameraMove(const CmdAllowCameraMove& evt);
    };
}