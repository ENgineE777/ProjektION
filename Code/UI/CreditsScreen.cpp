
#include "CreditsScreen.h"
#include "World/WorldManager.h"
#include "Root/Root.h"

namespace Orin::Overkill
{
	ENTITYREG(SceneEntity, CreditsScreen, "Overkill/UI", "CreditsScreen")

	META_DATA_DESC(CreditsScreen)
		BASE_SCENE_ENTITY_PROP(CreditsScreen)

		ENUM_PROP(CreditsScreen, horzAlign, 0, "Prop", "Horz align", "Horizontal aligment of a widget")
			ENUM_ELEM("Left", 0)
			ENUM_ELEM("Center", 1)
			ENUM_ELEM("Right", 2)
		ENUM_END
		ENUM_PROP(CreditsScreen, vertAlign, 0, "Prop", "Vert align", "Vertical aligment of a widget")
			ENUM_ELEM("Top", 3)
			ENUM_ELEM("Center", 1)
			ENUM_ELEM("Bottom", 4)
		ENUM_END
		ENUM_PROP(CreditsScreen, horzSize, 0, "Prop", "Horz size", "Type of width of a widget")
			ENUM_ELEM("Fixed", 0)
			ENUM_ELEM("Fill parent", 1)
			ENUM_ELEM("Wrap content", 2)
		ENUM_END
		ENUM_PROP(CreditsScreen, vertSize, 0, "Prop", "Vert size", "Type of height of a widget")
			ENUM_ELEM("Fixed", 0)
			ENUM_ELEM("Fill parent", 1)
			ENUM_ELEM("Wrap content", 2)
		ENUM_END
		FLOAT_PROP(CreditsScreen, leftPadding.x, 0.0f, "Prop", "left padding", "Left padding of a widget")
		FLOAT_PROP(CreditsScreen, leftPadding.y, 0.0f, "Prop", "top padding", "Top padding of a widget")
		FLOAT_PROP(CreditsScreen, rightPadding.x, 0.0f, "Prop", "right padding", "Right padding of a widget")
		FLOAT_PROP(CreditsScreen, rightPadding.y, 0.0f, "Prop", "bottom padding", "Bottom padding of a widget")
		COLOR_PROP(CreditsScreen, color, COLOR_WHITE, "Prop", "color")

	META_DATA_DESC_END()

	void CreditsScreen::Play()
	{
		credits = FindChild<ContainerWidget>("Credits");

		LabelWidget* label = FindChild<LabelWidget>("Lbl_Back");

		selectedLabel = label->color;

		items.push_back({ FindChild<LabelWidget>("Lbl_Back"), true, true, []() { WorldManager::instance->SetFrontendStateViaFade(FrontendState::MainMenu); }, []() {}, []() {} });
	}

	void CreditsScreen::Draw(float dt)
	{
		if (GetScene()->IsPlaying())
		{
			posY += dt * 30.0f;

			credits->GetTransform().position = { 0.0f, posY, 0.0f };

			if (posY > Sprite::GetPixelsHeight())
			{
				WorldManager::instance->SetFrontendStateViaFade(FrontendState::MainMenu);
			}
		}

		BaseMenu::Draw(dt);
	}

	void CreditsScreen::OnVisiblityChange(bool set)
	{
		if (set && credits)
		{
			auto& trans = credits->GetTransform();
			posY = -trans.size.y;

			trans.position = { 0.0f, posY, 0.0f };
		}
	}
}