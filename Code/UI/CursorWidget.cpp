
#include "CursorWidget.h"
#include "Root/Root.h"
#include "World/WorldManager.h"

namespace Orin::Overkill
{
	ENTITYREG(SceneEntity, CursorWidget, "Overkill/UI", "CursorWidget")

	META_DATA_DESC(CursorWidget)
		BASE_SCENE_ENTITY_PROP(CursorWidget)
		ASSET_TEXTURE_PROP(CursorWidget, texture, "Visual", "Texture")

		ENUM_PROP(CursorWidget, horzAlign, 0, "Prop", "Horz align", "Horizontal aligment of a widget")
			ENUM_ELEM("Left", 0)
			ENUM_ELEM("Center", 1)
			ENUM_ELEM("Right", 2)
		ENUM_END
		ENUM_PROP(CursorWidget, vertAlign, 0, "Prop", "Vert align", "Vertical aligment of a widget")
			ENUM_ELEM("Top", 3)
			ENUM_ELEM("Center", 1)
			ENUM_ELEM("Bottom", 4)
		ENUM_END
		ENUM_PROP(CursorWidget, horzSize, 0, "Prop", "Horz size", "Type of width of a widget")
			ENUM_ELEM("Fixed", 0)
			ENUM_ELEM("Fill parent", 1)
			ENUM_ELEM("Wrap content", 2)
		ENUM_END
		ENUM_PROP(CursorWidget, vertSize, 0, "Prop", "Vert size", "Type of height of a widget")
			ENUM_ELEM("Fixed", 0)
			ENUM_ELEM("Fill parent", 1)
			ENUM_ELEM("Wrap content", 2)
		ENUM_END
		FLOAT_PROP(CursorWidget, leftPadding.x, 0.0f, "Prop", "left padding", "Left padding of a widget")
		FLOAT_PROP(CursorWidget, leftPadding.y, 0.0f, "Prop", "top padding", "Top padding of a widget")
		FLOAT_PROP(CursorWidget, rightPadding.x, 0.0f, "Prop", "right padding", "Right padding of a widget")
		FLOAT_PROP(CursorWidget, rightPadding.y, 0.0f, "Prop", "bottom padding", "Bottom padding of a widget")
		COLOR_PROP(CursorWidget, color, COLOR_WHITE, "Prop", "color")

	META_DATA_DESC_END()

	void CursorWidget::Draw(float dt)
	{
		if (WorldManager::instance && (WorldManager::instance->gamepadConnected || WorldManager::instance->GetFrontendState() != FrontendState::Gameplay || WorldManager::instance->hackDisallowCursor))
		{
			return;
		}

		if (scene->IsPlaying())
		{
			int aliasX = GetRoot()->GetControls()->GetAlias("Hero.CURSOR_X");
			int aliasY = GetRoot()->GetControls()->GetAlias("Hero.CURSOR_Y");

			float msX = GetRoot()->GetControls()->GetAliasValue(aliasX, false);
			float msY = GetRoot()->GetControls()->GetAliasValue(aliasY, false);

			float scale = Sprite::GetPixelsHeight() / (float)GetRoot()->GetRender()->GetDevice()->GetHeight();

			transform.position = Math::Vector3(msX * scale, msY * scale, transform.position.z);
		}

		ImageWidget::Draw(dt);
	}
}