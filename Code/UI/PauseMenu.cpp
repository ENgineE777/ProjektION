
#include "PauseMenu.h"
#include "World/WorldManager.h"
#include "Root/Root.h"

namespace Orin::Overkill
{
	ENTITYREG(SceneEntity, PauseMenu, "Overkill/UI", "PauseMenu")

	META_DATA_DESC(PauseMenu)
		BASE_SCENE_ENTITY_PROP(PauseMenu)

		ENUM_PROP(PauseMenu, horzAlign, 0, "Prop", "Horz align", "Horizontal aligment of a widget")
			ENUM_ELEM("Left", 0)
			ENUM_ELEM("Center", 1)
			ENUM_ELEM("Right", 2)
		ENUM_END
		ENUM_PROP(PauseMenu, vertAlign, 0, "Prop", "Vert align", "Vertical aligment of a widget")
			ENUM_ELEM("Top", 3)
			ENUM_ELEM("Center", 1)
			ENUM_ELEM("Bottom", 4)
		ENUM_END
		ENUM_PROP(PauseMenu, horzSize, 0, "Prop", "Horz size", "Type of width of a widget")
			ENUM_ELEM("Fixed", 0)
			ENUM_ELEM("Fill parent", 1)
			ENUM_ELEM("Wrap content", 2)
		ENUM_END
		ENUM_PROP(PauseMenu, vertSize, 0, "Prop", "Vert size", "Type of height of a widget")
			ENUM_ELEM("Fixed", 0)
			ENUM_ELEM("Fill parent", 1)
			ENUM_ELEM("Wrap content", 2)
		ENUM_END
		FLOAT_PROP(PauseMenu, leftPadding.x, 0.0f, "Prop", "left padding", "Left padding of a widget")
		FLOAT_PROP(PauseMenu, leftPadding.y, 0.0f, "Prop", "top padding", "Top padding of a widget")
		FLOAT_PROP(PauseMenu, rightPadding.x, 0.0f, "Prop", "right padding", "Right padding of a widget")
		FLOAT_PROP(PauseMenu, rightPadding.y, 0.0f, "Prop", "bottom padding", "Bottom padding of a widget")
		COLOR_PROP(PauseMenu, color, COLOR_WHITE, "Prop", "color")

	META_DATA_DESC_END()

	void PauseMenu::Play()
	{
		LabelWidget* label = FindChild<LabelWidget>("Lbl_Resume");

		selectedLabel = label->color;

		items.push_back({ FindChild<LabelWidget>("Lbl_Resume"), false, true, []() { WorldManager::instance->SetFrontendState(FrontendState::Gameplay); }, []() { }, []() { } });
		items.push_back({ FindChild<LabelWidget>("Lbl_Options"), false, true, []() { WorldManager::instance->PushFrontendState(FrontendState::OptionsMenu); }, []() {}, []() {} });
		items.push_back({ FindChild<LabelWidget>("Lbl_Back"), true, true, []() { WorldManager::instance->SetFrontendStateViaFade(FrontendState::MainMenu); }, []() {}, []() {} });
	}

	void PauseMenu::ShowAndHandleAbilityChange(int& dbgLine, ThingAbility ability)
	{
		bool hasAbiliity = WorldManager::instance->IsThingAbilityAvailable(ability);
		int index = (int)ability;

		Sprite::DebugText(dbgLine++, "%i - %s (%s)", keyIndex + 1, ThingAbilityNames[index], hasAbiliity ? "On" : "Off");

		if (GetRoot()->GetControls()->DebugKeyPressed(StringUtils::PrintTemp("KEY_%i", keyIndex + 1), AliasAction::JustPressed))
		{
			MainEventsQueue::PushEvent(EventOnThingAbilityAvailabiltyChange{ ability, !hasAbiliity });
		}

		keyIndex++;
	}

	void PauseMenu::ShowAndHandleCheatChange(int& dbgLine, Cheat cheat)
	{
		bool hasAbiliity = WorldManager::instance->IsCheatEnabled(cheat);
		int index = (int)cheat;

		Sprite::DebugText(dbgLine++, "%i - %s (%s)", keyIndex + 1, CheatNames[index], hasAbiliity ? "On" : "Off");

		if (GetRoot()->GetControls()->DebugKeyPressed(StringUtils::PrintTemp("KEY_%i", keyIndex + 1), AliasAction::JustPressed))
		{
			MainEventsQueue::PushEvent(EventOnCheatStateChange{ cheat, !hasAbiliity });
		}

		keyIndex++;
	}

	void PauseMenu::Draw(float dt)
	{
		if (GetScene()->IsPlaying() && !WorldManager::instance->demoMode)
		{
			keyIndex = 0;

			int dbgLine = 1;

			/*Sprite::DebugText(dbgLine++, "Select Ability");
			dbgLine++;

			for (int i = 0; i < ThingAbility::ThingAbilityMaxCount; i++)
			{
				ShowAndHandleAbilityChange(dbgLine, (ThingAbility)i);
			}*/

			dbgLine++;
			Sprite::DebugText(dbgLine++, "Enable Cheat");
			dbgLine++;

			for (int i = 0; i < Cheat::CheatMaxCount; i++)
			{
				ShowAndHandleCheatChange(dbgLine, (Cheat)i);
			}			
		}

		BaseMenu::Draw(dt);
	}
}