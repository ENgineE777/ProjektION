
#include "OptionsMenu.h"
#include "World/WorldManager.h"
#include "Root/Root.h"

namespace Orin::Overkill
{
	ENTITYREG(SceneEntity, OptionsMenu, "Overkill/UI", "OptionsMenu")

	META_DATA_DESC(OptionsMenu)
		BASE_SCENE_ENTITY_PROP(OptionsMenu)

		ENUM_PROP(OptionsMenu, horzAlign, 0, "Prop", "Horz align", "Horizontal aligment of a widget")
			ENUM_ELEM("Left", 0)
			ENUM_ELEM("Center", 1)
			ENUM_ELEM("Right", 2)
		ENUM_END
		ENUM_PROP(OptionsMenu, vertAlign, 0, "Prop", "Vert align", "Vertical aligment of a widget")
			ENUM_ELEM("Top", 3)
			ENUM_ELEM("Center", 1)
			ENUM_ELEM("Bottom", 4)
		ENUM_END
		ENUM_PROP(OptionsMenu, horzSize, 0, "Prop", "Horz size", "Type of width of a widget")
			ENUM_ELEM("Fixed", 0)
			ENUM_ELEM("Fill parent", 1)
			ENUM_ELEM("Wrap content", 2)
		ENUM_END
		ENUM_PROP(OptionsMenu, vertSize, 0, "Prop", "Vert size", "Type of height of a widget")
			ENUM_ELEM("Fixed", 0)
			ENUM_ELEM("Fill parent", 1)
			ENUM_ELEM("Wrap content", 2)
		ENUM_END
		FLOAT_PROP(OptionsMenu, leftPadding.x, 0.0f, "Prop", "left padding", "Left padding of a widget")
		FLOAT_PROP(OptionsMenu, leftPadding.y, 0.0f, "Prop", "top padding", "Top padding of a widget")
		FLOAT_PROP(OptionsMenu, rightPadding.x, 0.0f, "Prop", "right padding", "Right padding of a widget")
		FLOAT_PROP(OptionsMenu, rightPadding.y, 0.0f, "Prop", "bottom padding", "Bottom padding of a widget")
		COLOR_PROP(OptionsMenu, color, COLOR_WHITE, "Prop", "color")

	META_DATA_DESC_END()

	const char* OptionsMenu::locales[] = { "en", "ru" };

	void OptionsMenu::OnVisiblityChange(bool set)
	{
		BaseMenu::OnVisiblityChange(set);

		int count = sizeof(locales) / sizeof(const char*);

		if (!WorldManager::instance)
		{
			return;
		}

		auto& gameState = WorldManager::instance->gameState;

		if (gameState.locale.empty())
		{
			gameState.locale = GetRoot()->GetLocalization()->GetCurrentLocale();
		}

		curLocale = -1;

		for (int i = 0; i < count; i++)
		{
			if (StringUtils::IsEqual(gameState.locale.c_str(), locales[i]))
			{
				curLocale = i;
			}
		}

		if (curLocale == -1)
		{
			gameState.locale = "en";
			curLocale = 0;
		}
	}

	void OptionsMenu::ChangeLocale(int locale)
	{
		int count = sizeof(locales) / sizeof(const char*);

		curLocale = locale;

		if (curLocale < 0)
		{
			curLocale = count - 1;
		}

		if (curLocale >= count)
		{
			curLocale = 0;
		}

		GetRoot()->GetLocalization()->SetCurrentLocale(locales[curLocale]);

		auto& gameState = WorldManager::instance->gameState;

		gameState.locale = locales[curLocale];
		gameState.SaveGameState();
	}

	void OptionsMenu::Play()
	{
		LabelWidget* label = FindChild<LabelWidget>("Lbl_language");

		selectedLabel = label->color;

		items.push_back({ FindChild<LabelWidget>("Lbl_language"), false, true, []() { }, [this]() { ChangeLocale(curLocale - 1); }, [this]() { ChangeLocale(curLocale + 1); } });
		items.push_back({ FindChild<LabelWidget>("Lbl_Music"), false, true, []() { WorldManager::instance->PushFrontendState(FrontendState::MusicMenu); }, [this]() {}, [this]() {} });
		items.push_back({ FindChild<LabelWidget>("Lbl_Controls"), false, true, []() { WorldManager::instance->PushFrontendState(FrontendState::ControlsMenu); }, [this]() {}, [this]() {} });
		items.push_back({ FindChild<LabelWidget>("Lbl_Back"), true, true, []() { WorldManager::instance->PopFrontendState(); }, []() {}, []() {} });
	}
}