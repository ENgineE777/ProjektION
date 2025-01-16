
#include "ControlsMenu.h"
#include "World/WorldManager.h"
#include "Root/Root.h"

namespace Orin::Overkill
{
	ENTITYREG(SceneEntity, ControlsMenu, "Overkill/UI", "ControlsMenu")

	META_DATA_DESC(ControlsMenu)
		BASE_SCENE_ENTITY_PROP(ControlsMenu)

		ENUM_PROP(ControlsMenu, horzAlign, 0, "Prop", "Horz align", "Horizontal aligment of a widget")
			ENUM_ELEM("Left", 0)
			ENUM_ELEM("Center", 1)
			ENUM_ELEM("Right", 2)
		ENUM_END
		ENUM_PROP(ControlsMenu, vertAlign, 0, "Prop", "Vert align", "Vertical aligment of a widget")
			ENUM_ELEM("Top", 3)
			ENUM_ELEM("Center", 1)
			ENUM_ELEM("Bottom", 4)
		ENUM_END
		ENUM_PROP(ControlsMenu, horzSize, 0, "Prop", "Horz size", "Type of width of a widget")
			ENUM_ELEM("Fixed", 0)
			ENUM_ELEM("Fill parent", 1)
			ENUM_ELEM("Wrap content", 2)
		ENUM_END
		ENUM_PROP(ControlsMenu, vertSize, 0, "Prop", "Vert size", "Type of height of a widget")
			ENUM_ELEM("Fixed", 0)
			ENUM_ELEM("Fill parent", 1)
			ENUM_ELEM("Wrap content", 2)
		ENUM_END
		FLOAT_PROP(ControlsMenu, leftPadding.x, 0.0f, "Prop", "left padding", "Left padding of a widget")
		FLOAT_PROP(ControlsMenu, leftPadding.y, 0.0f, "Prop", "top padding", "Top padding of a widget")
		FLOAT_PROP(ControlsMenu, rightPadding.x, 0.0f, "Prop", "right padding", "Right padding of a widget")
		FLOAT_PROP(ControlsMenu, rightPadding.y, 0.0f, "Prop", "bottom padding", "Bottom padding of a widget")
		COLOR_PROP(ControlsMenu, color, COLOR_WHITE, "Prop", "color")

	META_DATA_DESC_END()

	void ControlsMenu::Play()
	{
		gamepad = FindChild<ContainerWidget>("Gamepad");
		mouseKeyboard = FindChild<ContainerWidget>("MouseKeyboard");

		LabelWidget* label = FindChild<LabelWidget>("Lbl_Back");

		selectedLabel = label->color;

		items.push_back({ FindChild<LabelWidget>("Lbl_Back"), true, true, []() { WorldManager::instance->PopFrontendState(); }, []() {}, []() {} });
	}

	void ControlsMenu::Draw(float dt)
	{
		if (gamepad)
		{
			gamepad->SetVisiblity(WorldManager::instance->gamepadConnected);
		}

		if (mouseKeyboard)
		{
			mouseKeyboard->SetVisiblity(!WorldManager::instance->gamepadConnected);
		}

		BaseMenu::Draw(dt);
	}
}