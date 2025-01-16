
#pragma once

#include "Root/Scenes/SceneEntity.h"
#include "Support/MetaData.h"
#include "Support/Sprite.h"
#include "root/Assets/AssetTexture.h"
#include "BaseMenu.h"
#include "SceneEntities/UI/LabelWidget.h"

namespace Orin::Overkill
{
	class ControlsMenu : public BaseMenu
	{
		ContainerWidget* gamepad = nullptr;
		ContainerWidget* mouseKeyboard = nullptr;

	public:

		META_DATA_DECL_BASE(MainMenu)

#ifndef DOXYGEN_SKIP

		ControlsMenu() = default;
		virtual ~ControlsMenu() = default;

		void Play() override;
		void Draw(float dt) override;
#endif
	};
}