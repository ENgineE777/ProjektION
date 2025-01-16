
#include "SelectDifficultyMenu.h"
#include "World/WorldManager.h"
#include "Root/Root.h"

namespace Orin::Overkill
{
	ENTITYREG(SceneEntity, SelectDifficultyMenu, "Overkill/UI", "SelectLevel")

	META_DATA_DESC(SelectDifficultyMenu)
		BASE_SCENE_ENTITY_PROP(SelectDifficultyMenu)

		ENUM_PROP(SelectDifficultyMenu, horzAlign, 0, "Prop", "Horz align", "Horizontal aligment of a widget")
			ENUM_ELEM("Left", 0)
			ENUM_ELEM("Center", 1)
			ENUM_ELEM("Right", 2)
		ENUM_END
		ENUM_PROP(SelectDifficultyMenu, vertAlign, 0, "Prop", "Vert align", "Vertical aligment of a widget")
			ENUM_ELEM("Top", 3)
			ENUM_ELEM("Center", 1)
			ENUM_ELEM("Bottom", 4)
		ENUM_END
		ENUM_PROP(SelectDifficultyMenu, horzSize, 0, "Prop", "Horz size", "Type of width of a widget")
			ENUM_ELEM("Fixed", 0)
			ENUM_ELEM("Fill parent", 1)
			ENUM_ELEM("Wrap content", 2)
		ENUM_END
		ENUM_PROP(SelectDifficultyMenu, vertSize, 0, "Prop", "Vert size", "Type of height of a widget")
			ENUM_ELEM("Fixed", 0)
			ENUM_ELEM("Fill parent", 1)
			ENUM_ELEM("Wrap content", 2)
		ENUM_END
		FLOAT_PROP(SelectDifficultyMenu, leftPadding.x, 0.0f, "Prop", "left padding", "Left padding of a widget")
		FLOAT_PROP(SelectDifficultyMenu, leftPadding.y, 0.0f, "Prop", "top padding", "Top padding of a widget")
		FLOAT_PROP(SelectDifficultyMenu, rightPadding.x, 0.0f, "Prop", "right padding", "Right padding of a widget")
		FLOAT_PROP(SelectDifficultyMenu, rightPadding.y, 0.0f, "Prop", "bottom padding", "Bottom padding of a widget")
		COLOR_PROP(SelectDifficultyMenu, color, COLOR_WHITE, "Prop", "color")

	META_DATA_DESC_END()

	void SelectDifficultyMenu::OnVisiblityChange(bool set)
	{
		BaseMenu::OnVisiblityChange(set);

		if (set && items.size() > 0)
		{
			SetupDifficultyLabel(1, "Lbl_LevelNormal");
			SetupDifficultyLabel(2, "Lbl_LevelHard");
			SetupDifficultyLabel(3, "Lbl_LevelInsane");
		}
	}
	
	void SelectDifficultyMenu::SetupDifficultyLabel(int index, const char* diffuculty)
	{
		items[index].enabled = WorldManager::instance->gameState.IsDifficulty(diffuculty);
		items[index].label->color = items[index].enabled ? COLOR_WHITE : COLOR_LIGHT_GRAY;
	}

	void SelectDifficultyMenu::SelectDifficulty(const char* diffuculty)
	{
		WorldManager::instance->gameState.Reset();
		WorldManager::instance->gameState.SetDifficulty(diffuculty);
		WorldManager::instance->SetNextLevel(levelName, true);
		WorldManager::instance->SetFrontendStateViaFade(FrontendState::LoadNextLevel);
	}

	void SelectDifficultyMenu::Play()
	{
		LabelWidget* label = FindChild<LabelWidget>("Lbl_DiffEasy");
		
		selectedLabel = label->color;

		items.push_back({ FindChild<LabelWidget>("Lbl_DiffEasy"), false, true, [this]() { SelectDifficulty("Easy"); }, []() {}, []() {}});
		items.push_back({ FindChild<LabelWidget>("Lbl_DiffNormal"), false, true, [this]() { SelectDifficulty("Normal"); }, []() {}, []() {} });
		items.push_back({ FindChild<LabelWidget>("Lbl_DiffHard"), false, true, [this]() { SelectDifficulty("Hard"); }, []() {}, []() {} });
		items.push_back({ FindChild<LabelWidget>("Lbl_DiffInsane"), false, true, [this]() { SelectDifficulty("Insane"); }, []() {}, []() {} });

		items.push_back({ FindChild<LabelWidget>("Lbl_Back"), true, true, []() { WorldManager::instance->SetFrontendState(FrontendState::MainMenu); }, []() {}, []() {} });
	}
}