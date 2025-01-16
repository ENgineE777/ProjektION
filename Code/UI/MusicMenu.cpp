
#include "MusicMenu.h"
#include "World/WorldManager.h"
#include "Root/Root.h"

namespace Orin::Overkill
{
	ENTITYREG(SceneEntity, MusicMenu, "Overkill/UI", "MusicMenu")

	META_DATA_DESC(MusicMenu)
		BASE_SCENE_ENTITY_PROP(MusicMenu)

		ENUM_PROP(MusicMenu, horzAlign, 0, "Prop", "Horz align", "Horizontal aligment of a widget")
			ENUM_ELEM("Left", 0)
			ENUM_ELEM("Center", 1)
			ENUM_ELEM("Right", 2)
		ENUM_END
		ENUM_PROP(MusicMenu, vertAlign, 0, "Prop", "Vert align", "Vertical aligment of a widget")
			ENUM_ELEM("Top", 3)
			ENUM_ELEM("Center", 1)
			ENUM_ELEM("Bottom", 4)
		ENUM_END
		ENUM_PROP(MusicMenu, horzSize, 0, "Prop", "Horz size", "Type of width of a widget")
			ENUM_ELEM("Fixed", 0)
			ENUM_ELEM("Fill parent", 1)
			ENUM_ELEM("Wrap content", 2)
		ENUM_END
		ENUM_PROP(MusicMenu, vertSize, 0, "Prop", "Vert size", "Type of height of a widget")
			ENUM_ELEM("Fixed", 0)
			ENUM_ELEM("Fill parent", 1)
			ENUM_ELEM("Wrap content", 2)
		ENUM_END
		FLOAT_PROP(MusicMenu, leftPadding.x, 0.0f, "Prop", "left padding", "Left padding of a widget")
		FLOAT_PROP(MusicMenu, leftPadding.y, 0.0f, "Prop", "top padding", "Top padding of a widget")
		FLOAT_PROP(MusicMenu, rightPadding.x, 0.0f, "Prop", "right padding", "Right padding of a widget")
		FLOAT_PROP(MusicMenu, rightPadding.y, 0.0f, "Prop", "bottom padding", "Bottom padding of a widget")
		COLOR_PROP(MusicMenu, color, COLOR_WHITE, "Prop", "color")

	META_DATA_DESC_END()

	void MusicMenu::OnVisiblityChange(bool set)
	{
		BaseMenu::OnVisiblityChange(set);

		if (set && WorldManager::instance)
		{
			auto& gameState = WorldManager::instance->gameState;
			
			LabelWidget* label = FindChild<LabelWidget>("Lbl_Music_Value");
			label->text = StringUtils::PrintTemp("%i", gameState.musicVolume);

			label = FindChild<LabelWidget>("Lbl_SFX_Value");
			label->text = StringUtils::PrintTemp("%i", gameState.sfxVolume);
		}
	}

	void MusicMenu::Play()
	{
		LabelWidget* label = FindChild<LabelWidget>("Lbl_Music_Volume");

		selectedLabel = label->color;

		items.push_back({ FindChild<LabelWidget>("Lbl_Music_Volume"), false, true, []() {}, [this]() { ChangeMusicVolume(-10); }, [this]() { ChangeMusicVolume(10); } });
		items.push_back({ FindChild<LabelWidget>("Lbl_SFX_Volume"), false, true, []() {}, [this]() { ChangeSFXVolume(-10); }, [this]() { ChangeSFXVolume(10); } });
		items.push_back({ FindChild<LabelWidget>("Lbl_Back"), true, true, []() { WorldManager::instance->PopFrontendState(); }, []() {}, []() {} });
	}

	void MusicMenu::ChangeMusicVolume(int value)
	{
		auto& gameState = WorldManager::instance->gameState;

		gameState.musicVolume = (int)Math::Clamp((float)gameState.musicVolume + value, 0.0f, 100.0f);

		GetRoot()->GetSounds()->SetVCAVolume("vca:/Music", gameState.musicVolume * 0.01f);

		LabelWidget* label = FindChild<LabelWidget>("Lbl_Music_Value");

		label->text = StringUtils::PrintTemp("%i", gameState.musicVolume);

		gameState.SaveGameState();
	}

	void MusicMenu::ChangeSFXVolume(int value)
	{
		auto& gameState = WorldManager::instance->gameState;

		gameState.sfxVolume = (int)Math::Clamp((float)gameState.sfxVolume + value, 0.0f, 100.0f);

		GetRoot()->GetSounds()->SetVCAVolume("vca:/SFX", gameState.sfxVolume * 0.01f);

		LabelWidget* label = FindChild<LabelWidget>("Lbl_SFX_Value");

		label->text = StringUtils::PrintTemp("%i", gameState.sfxVolume);

		gameState.SaveGameState();
	}
}