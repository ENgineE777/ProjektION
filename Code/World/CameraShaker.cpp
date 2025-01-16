
#include "CameraShaker.h"
#include "Root/Root.h"
#include "World/WorldManager.h"

#define ORIN_CONTROLS GetRoot()->GetControls()
#define ORIN_CONTROLS_JUST_PRESSED(alias) ORIN_CONTROLS->GetAliasState(alias, AliasAction::JustPressed)
#define ORIN_CONTROLS_PRESSED(alias) ORIN_CONTROLS->GetAliasState(alias, AliasAction::Pressed)
#define ORIN_CONTROLS_VALUE(alias) ORIN_CONTROLS->GetAliasValue(alias, false)
#define ORIN_CONTROLS_DELTA(alias) ORIN_CONTROLS->GetAliasValue(alias, true)

namespace Orin::Overkill
{
	ENTITYREG(SceneEntity, CameraShaker, "Overkill/World", "CameraShaker")

	META_DATA_DESC(CameraShaker)
		BASE_SCENE_ENTITY_PROP(CameraShaker)
		FLOAT_PROP(CameraShaker, totalTimeMul, 5.f, "Properties", "totalTimeMul", "Speed of shake")
		FLOAT_PROP(CameraShaker, reduceTraumaMul, 2.f, "Properties", "reduceTraumaMul", "Reduce trauma multiplier")
		INT_PROP(CameraShaker, octavesOffset, 2, "Offset", "octaves", "Octaves of offset noise")
		FLOAT_PROP(CameraShaker, persistenceOffset, 0.5f, "Offset", "persistence", "Persistence of offset noise")
		FLOAT_PROP(CameraShaker, maxOffsetX, 16.f, "Offset", "maxOffsetX", "Max offset by X axis")
		FLOAT_PROP(CameraShaker, maxOffsetY, 16.f, "Offset", "maxOffsetY", "Max offset by Y axis")
		INT_PROP(CameraShaker, octavesRotation, 2, "Rotation", "octavesRotation", "Octaves of rotation noise")
		FLOAT_PROP(CameraShaker, persistenceRotation, 0.5f, "Rotation", "persistenceRotation", "Persistence of rotation noise")
		FLOAT_PROP(CameraShaker, maxAngle, 20.f, "Rotation", "maxAngle", "Max rotation shake in degrees")
	META_DATA_DESC_END()

    ORIN_EVENTS(CameraShaker)
        ORIN_EVENT(CameraShaker, CmdAddShakeTrauma, AddShakeTrauma)
        ORIN_EVENT(CameraShaker, CmdAllowCameraMove, AllowCameraMove)
    ORIN_EVENTS_END(CameraShaker)

    void CameraShaker::AddShakeTrauma(const CmdAddShakeTrauma &evt)
    {
        trauma = Utils::Saturate(trauma + evt.trauma);
    }

    void CameraShaker::AllowCameraMove(const CmdAllowCameraMove& evt)
    {
        allowMove = evt.allow;
    }

    void CameraShaker::Play()
    {
        ScriptEntity2D::Play();

        cameraRef.SetEntity(parent);

        for (auto &gen : perlinNoiseGenerators)
        {
            gen.reseed(rndGenerator.NextUInt());
        }

        trauma    = 0.f;
        totalTime = 0.f;

        aliasCameraMoveActive = ORIN_CONTROLS->GetAlias("Camera.MOVE_ACTIVE");
		aliasCameraMoveHorz   = ORIN_CONTROLS->GetAlias("Camera.MOVE_HORZ");
		aliasCameraMoveVert   = ORIN_CONTROLS->GetAlias("Camera.MOVE_VERT");
        aliasCursorX          = ORIN_CONTROLS->GetAlias("Hero.CURSOR_X");
		aliasCursorY          = ORIN_CONTROLS->GetAlias("Hero.CURSOR_Y");

        ORIN_EVENTS_SUBSCRIBE(CameraShaker);
    }

    void CameraShaker::Release()
    {
        MainEventsQueue::Unsubscribe(this);

        ScriptEntity2D::Release();
    }

    void CameraShaker::Update(float dt)
    {
        if (!cameraRef)
        {
            return;
        }

        if (GetRoot()->GetControls()->DebugKeyPressed("KEY_J", AliasAction::JustPressed))
        {
            trauma += 0.25f;
        }
        else if (GetRoot()->GetControls()->DebugKeyPressed("KEY_J", AliasAction::Pressed))
        {
            trauma = 1.f;
        }

        const float shake = Utils::Saturate(Math::EaseInQuad(Utils::Saturate(trauma)));
        trauma = Utils::Saturate(trauma - reduceTraumaMul * dt);

        totalTime += totalTimeMul * dt;

        const float noiseOffsetX  = perlinNoiseGenerators[0].normalizedOctave1D(totalTime, octavesOffset, persistenceOffset);
        const float noiseOffsetY  = perlinNoiseGenerators[1].normalizedOctave1D(totalTime, octavesOffset, persistenceOffset);
        const float noiseRotation = perlinNoiseGenerators[2].normalizedOctave1D(totalTime, octavesRotation, persistenceRotation);

        const float cameraMoveGrid = 8.f;

		if (allowMove && ORIN_CONTROLS_PRESSED(aliasCameraMoveActive))
		{
            Math::Vector2 cameraMoveDir;
            float cameraMoveOffs = 0.f;

            if (WorldManager::instance->gamepadConnected)
            {
                cameraMoveDir = {ORIN_CONTROLS_VALUE(aliasCameraMoveHorz), ORIN_CONTROLS_VALUE(aliasCameraMoveVert)};
                cameraMoveDir.x = std::abs(cameraMoveDir.x) < 0.5f ? 0.f : Math::Sign(cameraMoveDir.x);
                cameraMoveDir.y = std::abs(cameraMoveDir.y) < 0.5f ? 0.f : Math::Sign(cameraMoveDir.y);

                cameraMoveOffs = 1024.f * dt;
            }
            else
            {
                Math::Vector2 cursorPos{ORIN_CONTROLS_VALUE(aliasCursorX), -ORIN_CONTROLS_VALUE(aliasCursorY)};
                cursorPos.x = std::round(cursorPos.x/cameraMoveGrid)*cameraMoveGrid;
                cursorPos.y = std::round(cursorPos.y/cameraMoveGrid)*cameraMoveGrid;
                eastl::tie(cameraMoveDir, cameraMoveOffs) = Utils::Vector::Normalize2(cursorPos - startCameraPos);
                startCameraPos = cursorPos;
            }

            wishCameraMove += cameraMoveDir * cameraMoveOffs;
            auto halfScreen = Sprite::GetHalfScreenSize();

            wishCameraMove = Utils::Clamp(wishCameraMove, -halfScreen, halfScreen);

            cameraMove = Utils::Approach(cameraMove, wishCameraMove, dt, 0.2f);
		}
		else
		{
			cameraMove = Utils::Approach(cameraMove, {}, dt, 0.1f);
            wishCameraMove = {};

            if (!WorldManager::instance->gamepadConnected)
            {
                startCameraPos = {ORIN_CONTROLS_VALUE(aliasCursorX), -ORIN_CONTROLS_VALUE(aliasCursorY)};
                startCameraPos.x = std::round(startCameraPos.x/cameraMoveGrid)*cameraMoveGrid;
                startCameraPos.y = std::round(startCameraPos.y/cameraMoveGrid)*cameraMoveGrid;
            }
		}

        cameraRef->addPosition = shake * Math::Vector2{maxOffsetX * noiseOffsetX, maxOffsetY * noiseOffsetY} + cameraMove;
        cameraRef->addRotation = shake * maxAngle * noiseRotation;

        auto camPos = Sprite::GetCamPos();
        GetRoot()->GetSounds()->SetListenerAttributes({ camPos.x, camPos.y, 0.0f} );
    }
}