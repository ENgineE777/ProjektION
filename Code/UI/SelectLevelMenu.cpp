
#include "SelectLevelMenu.h"
#include "World/WorldManager.h"
#include "Root/Root.h"

namespace Orin::Overkill
{
	ENTITYREG(SceneEntity, SelectLevelMenu, "Overkill/UI", "SelectLevel")

	META_DATA_DESC(SelectLevelMenu)
		BASE_SCENE_ENTITY_PROP(SelectLevelMenu)

		ENUM_PROP(SelectLevelMenu, horzAlign, 0, "Prop", "Horz align", "Horizontal aligment of a widget")
			ENUM_ELEM("Left", 0)
			ENUM_ELEM("Center", 1)
			ENUM_ELEM("Right", 2)
		ENUM_END
		ENUM_PROP(SelectLevelMenu, vertAlign, 0, "Prop", "Vert align", "Vertical aligment of a widget")
			ENUM_ELEM("Top", 3)
			ENUM_ELEM("Center", 1)
			ENUM_ELEM("Bottom", 4)
		ENUM_END
		ENUM_PROP(SelectLevelMenu, horzSize, 0, "Prop", "Horz size", "Type of width of a widget")
			ENUM_ELEM("Fixed", 0)
			ENUM_ELEM("Fill parent", 1)
			ENUM_ELEM("Wrap content", 2)
		ENUM_END
		ENUM_PROP(SelectLevelMenu, vertSize, 0, "Prop", "Vert size", "Type of height of a widget")
			ENUM_ELEM("Fixed", 0)
			ENUM_ELEM("Fill parent", 1)
			ENUM_ELEM("Wrap content", 2)
		ENUM_END
		FLOAT_PROP(SelectLevelMenu, leftPadding.x, 0.0f, "Prop", "left padding", "Left padding of a widget")
		FLOAT_PROP(SelectLevelMenu, leftPadding.y, 0.0f, "Prop", "top padding", "Top padding of a widget")
		FLOAT_PROP(SelectLevelMenu, rightPadding.x, 0.0f, "Prop", "right padding", "Right padding of a widget")
		FLOAT_PROP(SelectLevelMenu, rightPadding.y, 0.0f, "Prop", "bottom padding", "Bottom padding of a widget")
		COLOR_PROP(SelectLevelMenu, color, COLOR_WHITE, "Prop", "color")

	META_DATA_DESC_END()

	void SelectLevelMenu::OnVisiblityChange(bool set)
	{
		BaseMenu::OnVisiblityChange(set);

		if (set && items.size() > 0)
		{
			SetupLevelLabel(1, "Level_02");
			SetupLevelLabel(2, "Level_03");
			SetupLevelLabel(3, "Level_04");
			SetupLevelLabel(4, "Level_05");
			SetupLevelLabel(5, "Level_06");
			SetupLevelLabel(6, "Level_07");
		}
	}

	void SelectLevelMenu::SetupLevelLabel(int index, const char* levelName)
	{
		items[index].enabled = WorldManager::instance->gameState.IsLevelOpened(levelName);
		items[index].label->color = items[index].enabled ? COLOR_WHITE : COLOR_LIGHT_GRAY;
	}

	void SelectLevelMenu::SelectLevel(const char* levelName)
	{
		WorldManager::instance->selectDifficultyMenu->levelName = levelName;
		WorldManager::instance->SetFrontendState(FrontendState::SelectDifficultyMenu);
	}

	void SelectLevelMenu::Play()
	{
		LabelWidget* label = FindChild<LabelWidget>("Lbl_Level1");
		
		selectedLabel = label->color;

		items.push_back({ FindChild<LabelWidget>("Lbl_Level1"), false, true, [this]() { SelectLevel("Level_01"); }, []() {}, []() {}});
		items.push_back({ FindChild<LabelWidget>("Lbl_Level2"), false, true, [this]() { SelectLevel("Level_02"); }, []() {}, []() {} });
		items.push_back({ FindChild<LabelWidget>("Lbl_Level3"), false, true, [this]() { SelectLevel("Level_03"); }, []() {}, []() {} });
		items.push_back({ FindChild<LabelWidget>("Lbl_Level4"), false, true, [this]() { SelectLevel("Level_04"); }, []() {}, []() {} });
		items.push_back({ FindChild<LabelWidget>("Lbl_Level5"), false, true, [this]() { SelectLevel("Level_05"); }, []() {}, []() {} });
		items.push_back({ FindChild<LabelWidget>("Lbl_Level6"), false, true, [this]() { SelectLevel("Level_06"); }, []() {}, []() {} });
		items.push_back({ FindChild<LabelWidget>("Lbl_Level7"), false, true, [this]() { SelectLevel("Level_07"); }, []() {}, []() {} });

		items.push_back({ FindChild<LabelWidget>("Lbl_Back"), true, true, []() { WorldManager::instance->SetFrontendState(FrontendState::MainMenu); }, []() {}, []() {} });
	}
}