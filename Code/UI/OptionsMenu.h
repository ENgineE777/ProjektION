
#pragma once

#include "Root/Scenes/SceneEntity.h"
#include "Support/MetaData.h"
#include "Support/Sprite.h"
#include "root/Assets/AssetTexture.h"
#include "BaseMenu.h"
#include "SceneEntities/UI/LabelWidget.h"

namespace Orin::Overkill
{
	class OptionsMenu : public BaseMenu
	{
		int curLocale = 0;

		void ChangeLocale(int locale);

		static const char* locales[];

	public:

		META_DATA_DECL_BASE(MainMenu)

#ifndef DOXYGEN_SKIP

		OptionsMenu() = default;
		virtual ~OptionsMenu() = default;

		void Play() override;
		void OnVisiblityChange(bool set) override;
#endif
	};
}