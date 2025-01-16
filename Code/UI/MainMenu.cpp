
#include "MainMenu.h"
#include "World/WorldManager.h"
#include "Root/Root.h"

namespace Orin::Overkill
{
	ENTITYREG(SceneEntity, MainMenu, "Overkill/UI", "MainMenu")

	META_DATA_DESC(MainMenu)
		BASE_SCENE_ENTITY_PROP(MainMenu)

		ENUM_PROP(MainMenu, horzAlign, 0, "Prop", "Horz align", "Horizontal aligment of a widget")
			ENUM_ELEM("Left", 0)
			ENUM_ELEM("Center", 1)
			ENUM_ELEM("Right", 2)
		ENUM_END
		ENUM_PROP(MainMenu, vertAlign, 0, "Prop", "Vert align", "Vertical aligment of a widget")
			ENUM_ELEM("Top", 3)
			ENUM_ELEM("Center", 1)
			ENUM_ELEM("Bottom", 4)
		ENUM_END
		ENUM_PROP(MainMenu, horzSize, 0, "Prop", "Horz size", "Type of width of a widget")
			ENUM_ELEM("Fixed", 0)
			ENUM_ELEM("Fill parent", 1)
			ENUM_ELEM("Wrap content", 2)
		ENUM_END
		ENUM_PROP(MainMenu, vertSize, 0, "Prop", "Vert size", "Type of height of a widget")
			ENUM_ELEM("Fixed", 0)
			ENUM_ELEM("Fill parent", 1)
			ENUM_ELEM("Wrap content", 2)
		ENUM_END
		FLOAT_PROP(MainMenu, leftPadding.x, 0.0f, "Prop", "left padding", "Left padding of a widget")
		FLOAT_PROP(MainMenu, leftPadding.y, 0.0f, "Prop", "top padding", "Top padding of a widget")
		FLOAT_PROP(MainMenu, rightPadding.x, 0.0f, "Prop", "right padding", "Right padding of a widget")
		FLOAT_PROP(MainMenu, rightPadding.y, 0.0f, "Prop", "bottom padding", "Bottom padding of a widget")
		COLOR_PROP(MainMenu, color, COLOR_WHITE, "Prop", "color")

	META_DATA_DESC_END()

	void MainMenu::OnVisiblityChange(bool set)
	{
		if (set && items.size() > 0)
		{
			items[1].enabled = !WorldManager::instance->gameState.IsEmpty();
			items[1].label->color = items[1].enabled ? COLOR_WHITE : COLOR_LIGHT_GRAY;

			if (items[1].enabled)
			{
				items[curItem].label->color = COLOR_WHITE;
				curItem = 1;
				items[curItem].label->color = selectedLabel;
			}
		}
	}

	void MainMenu::Play()
	{
		LabelWidget* label = FindChild<LabelWidget>("Lbl_Start");

		selectedLabel = label->color;

		items.push_back({ FindChild<LabelWidget>("Lbl_Start"), false, true, []()
			{
				WorldManager::instance->selectDifficultyMenu->levelName = "Level_01";
				WorldManager::instance->SetFrontendStateViaFade(FrontendState::SelectDifficultyMenu);
			}, []() {}, []() {} });
		items.push_back({ FindChild<LabelWidget>("Lbl_Continue"), false, true, []()
			{				
				WorldManager::instance->SetNextLevel(WorldManager::instance->gameState.GetCurrentLevel(), false);
				WorldManager::instance->SetFrontendStateViaFade(FrontendState::LoadNextLevel);
			}, []() {}, []() {} });
		items.push_back({ FindChild<LabelWidget>("Lbl_Debug_Menu"), false, true, []() { WorldManager::instance->SetFrontendState(WorldManager::instance->demoMode ? FrontendState::SelectLevelMenu : FrontendState::DebugMenu); }, []() {}, []() {} });
		items.push_back({ FindChild<LabelWidget>("Lbl_Options"), false, true, []() { WorldManager::instance->PushFrontendState(FrontendState::OptionsMenu); }, []() {}, []() {} });
		items.push_back({ FindChild<LabelWidget>("Lbl_Credits"), false, true, []() { WorldManager::instance->SetFrontendStateViaFade(FrontendState::CreditsScreen); }, []() {}, []() {} });
		items.push_back({ FindChild<LabelWidget>("Lbl_Exit"), true, true, []() { GetRoot()->SetQuitRequest(true); }, []() {}, []() {} });
	}
}