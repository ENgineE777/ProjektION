
#include "ResultDemoScreen.h"
#include "World/WorldManager.h"
#include "Root/Root.h"

namespace Orin::Overkill
{
	ENTITYREG(SceneEntity, ResultDemoScreen, "Overkill/UI", "ResultDemoScreen")

	META_DATA_DESC(ResultDemoScreen)
		BASE_SCENE_ENTITY_PROP(ResultDemoScreen)

		ENUM_PROP(ResultDemoScreen, horzAlign, 0, "Prop", "Horz align", "Horizontal aligment of a widget")
			ENUM_ELEM("Left", 0)
			ENUM_ELEM("Center", 1)
			ENUM_ELEM("Right", 2)
		ENUM_END
		ENUM_PROP(ResultDemoScreen, vertAlign, 0, "Prop", "Vert align", "Vertical aligment of a widget")
			ENUM_ELEM("Top", 3)
			ENUM_ELEM("Center", 1)
			ENUM_ELEM("Bottom", 4)
		ENUM_END
		ENUM_PROP(ResultDemoScreen, horzSize, 0, "Prop", "Horz size", "Type of width of a widget")
			ENUM_ELEM("Fixed", 0)
			ENUM_ELEM("Fill parent", 1)
			ENUM_ELEM("Wrap content", 2)
		ENUM_END
		ENUM_PROP(ResultDemoScreen, vertSize, 0, "Prop", "Vert size", "Type of height of a widget")
			ENUM_ELEM("Fixed", 0)
			ENUM_ELEM("Fill parent", 1)
			ENUM_ELEM("Wrap content", 2)
		ENUM_END
		FLOAT_PROP(ResultDemoScreen, leftPadding.x, 0.0f, "Prop", "left padding", "Left padding of a widget")
		FLOAT_PROP(ResultDemoScreen, leftPadding.y, 0.0f, "Prop", "top padding", "Top padding of a widget")
		FLOAT_PROP(ResultDemoScreen, rightPadding.x, 0.0f, "Prop", "right padding", "Right padding of a widget")
		FLOAT_PROP(ResultDemoScreen, rightPadding.y, 0.0f, "Prop", "bottom padding", "Bottom padding of a widget")
		COLOR_PROP(ResultDemoScreen, color, COLOR_WHITE, "Prop", "color")

	META_DATA_DESC_END()

	void ResultDemoScreen::Play()
	{		
		LabelWidget* label = FindChild<LabelWidget>("Lbl_Back");

		selectedLabel = label->color;

		items.push_back({ FindChild<LabelWidget>("Lbl_Back"), true, true, []() { WorldManager::instance->SetFrontendStateViaFade(FrontendState::MainMenu); }, []() {}, []() {} });

		deathsValue = FindChild<LabelWidget>("Label_DeathsValue");

		killsValue = FindChild<LabelWidget>("Label_KillsValue");
		totalTimerValue = FindChild<LabelWidget>("Label_TotalTimerValue");

		toBeContinued = FindChild<ImageWidget>("ToBeContinued");
	}

	void ResultDemoScreen::OnVisiblityChange(bool set)
	{
		if (set && items.size() > 0)
		{
			timerToBe = 3.0f;
			toBeContinued->SetVisiblity(true);

			auto gameState = WorldManager::instance->gameState;
			
			Utils::FormatTimer(gameState.GetTotalTime(), totalTimerValue->text);

			killsValue->text = StringUtils::PrintTemp("%d", gameState.GetKillsCount());
			deathsValue->text = StringUtils::PrintTemp("%d", gameState.GetDeathsCount());
		}
	}

	void ResultDemoScreen::Draw(float dt)
	{
		BaseMenu::Draw(dt);

		if (timerToBe > 0.0f)
		{
			timerToBe -= dt;

			if (timerToBe < 0.0f)
			{
				timerToBe = -1.0f;
				WorldManager::instance->StartFade(true, 0.5f, [this]() { toBeContinued->SetVisiblity(false);
																		 WorldManager::instance->StartFade(false, 0.5f, [this]() { }); });
			}
		}
	}
}