
#include "ResultScreen.h"
#include "World/WorldManager.h"
#include "Root/Root.h"

namespace Orin::Overkill
{
	ENTITYREG(SceneEntity, ResultScreen, "Overkill/UI", "ResultScreen")

	META_DATA_DESC(ResultScreen)
		BASE_SCENE_ENTITY_PROP(ResultScreen)
		ASSET_TEXTURE_PROP(ResultScreen, texture, "Visual", "Texture")

		ENUM_PROP(ResultScreen, horzAlign, 0, "Prop", "Horz align", "Horizontal aligment of a widget")
			ENUM_ELEM("Left", 0)
			ENUM_ELEM("Center", 1)
			ENUM_ELEM("Right", 2)
		ENUM_END
		ENUM_PROP(ResultScreen, vertAlign, 0, "Prop", "Vert align", "Vertical aligment of a widget")
			ENUM_ELEM("Top", 3)
			ENUM_ELEM("Center", 1)
			ENUM_ELEM("Bottom", 4)
		ENUM_END
		ENUM_PROP(ResultScreen, horzSize, 0, "Prop", "Horz size", "Type of width of a widget")
			ENUM_ELEM("Fixed", 0)
			ENUM_ELEM("Fill parent", 1)
			ENUM_ELEM("Wrap content", 2)
		ENUM_END
		ENUM_PROP(ResultScreen, vertSize, 0, "Prop", "Vert size", "Type of height of a widget")
			ENUM_ELEM("Fixed", 0)
			ENUM_ELEM("Fill parent", 1)
			ENUM_ELEM("Wrap content", 2)
		ENUM_END
		FLOAT_PROP(ResultScreen, leftPadding.x, 0.0f, "Prop", "left padding", "Left padding of a widget")
		FLOAT_PROP(ResultScreen, leftPadding.y, 0.0f, "Prop", "top padding", "Top padding of a widget")
		FLOAT_PROP(ResultScreen, rightPadding.x, 0.0f, "Prop", "right padding", "Right padding of a widget")
		FLOAT_PROP(ResultScreen, rightPadding.y, 0.0f, "Prop", "bottom padding", "Bottom padding of a widget")
		COLOR_PROP(ResultScreen, imageColor, COLOR_WHITE, "Prop", "image color")
		COLOR_PROP(ResultScreen, color, COLOR_WHITE, "Prop", "color")

	META_DATA_DESC_END()

	void ResultScreen::Init()
	{
		ImageWidget::Init();
	}

	void ResultScreen::Play()
	{
		wonLabel = FindChild<LabelWidget>("Label_won");
		loseLabel = FindChild<LabelWidget>("Label_lose");

		timerValue = FindChild<LabelWidget>("Label_TimerValue");
		deathsValue = FindChild<LabelWidget>("Label_DeathsValue");		
		
		killsValue = FindChild<LabelWidget>("Label_KillsValue");
		totalTimerValue = FindChild<LabelWidget>("Label_TotalTimerValue");

		scoreContainer = FindChild<ContainerWidget>("TotalScore");
		scoreValue = FindChild<LabelWidget>("Label_TotalScoreValue");

		continueLabel = FindChild<LabelWidget>("Label_Continue");
		continueLabelGamepad = FindChild<LabelWidget>("Label_Continue_Gamepad");

		restartLabel = FindChild<LabelWidget>("Label_Restart");
		restartLabelGamepad = FindChild<LabelWidget>("Label_Restart_Gamepad");
	}

	void ResultScreen::SetState(ResultScreen::State newState, int rating)
	{
		state = newState;

		SetVisiblity(state != State::None);

		if (state == State::None)
		{
			return;
		}

		auto gameState = WorldManager::instance->gameState;

		Utils::FormatTimer(gameState.GetLevelTime(), timerValue->text);
		Utils::FormatTimer(gameState.GetTotalTime(), totalTimerValue->text);

		killsValue->text = StringUtils::PrintTemp("%d", gameState.GetKillsCount());
		deathsValue->text = StringUtils::PrintTemp("%d", gameState.GetDeathsCount());

		const char* ratings[] = { "A", "B", "C", "D" };

		scoreValue->text = StringUtils::PrintTemp("%d %s", gameState.GetLevelScore(), ratings[rating]);

		scoreContainer->SetVisiblity(false);

		wonLabel->SetVisiblity(state == State::Won);
		loseLabel->SetVisiblity(state == State::Lose);

		restartLabel->SetVisiblity(state == State::Lose);
		restartLabelGamepad->SetVisiblity(state == State::Lose);

		continueLabel->SetVisiblity(state == State::Won);		
		continueLabelGamepad->SetVisiblity(state == State::Won);
	}

	void ResultScreen::Draw(float dt)
	{
		if (!GetScene()->IsPlaying())
		{
			return;
		}

		if (state == State::Lose)
		{
			restartLabel->SetVisiblity(!WorldManager::instance->gamepadConnected);
			restartLabelGamepad->SetVisiblity(WorldManager::instance->gamepadConnected);
		}

		if (state == State::Won)
		{
			continueLabel->SetVisiblity(!WorldManager::instance->gamepadConnected);
			continueLabelGamepad->SetVisiblity(WorldManager::instance->gamepadConnected);
		}

		if (aliasContinue == -1)
		{
			aliasContinue = GetRoot()->GetControls()->GetAlias("UI.CONTINUE");
			aliasRestart = GetRoot()->GetControls()->GetAlias("UI.RESTART");
		}

		if (state == State::Won && GetRoot()->GetControls()->GetAliasState(aliasContinue, AliasAction::JustPressed))
		{
			WorldManager::instance->SetFrontendStateViaFade(FrontendState::LoadNextLevel);
			return;
		}
		
		if (state == State::Lose && GetRoot()->GetControls()->GetAliasState(aliasRestart, AliasAction::JustPressed))
		{
			WorldManager::instance->SetFrontendState(FrontendState::GameplayRestart);
		}

		ImageWidget::Draw(dt);
	}
}