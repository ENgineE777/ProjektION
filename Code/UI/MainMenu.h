
#pragma once

#include "Root/Scenes/SceneEntity.h"
#include "Support/MetaData.h"
#include "Support/Sprite.h"
#include "root/Assets/AssetTexture.h"
#include "BaseMenu.h"
#include "SceneEntities/UI/LabelWidget.h"

namespace Orin::Overkill
{
	class MainMenu : public BaseMenu
	{
	public:

		META_DATA_DECL_BASE(MainMenu)

	#ifndef DOXYGEN_SKIP

		MainMenu() = default;
		virtual ~MainMenu() = default;

		void Play() override;
		void OnVisiblityChange(bool set) override;
	#endif
	};
}