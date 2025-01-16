
#pragma once

#include "Root/Scenes/SceneEntity.h"
#include "Support/MetaData.h"
#include "Support/Sprite.h"
#include "root/Assets/AssetTexture.h"
#include "BaseMenu.h"
#include "SceneEntities/UI/LabelWidget.h"
#include "Characters/Player/ThingAbilities.h"
#include "Characters/Player/Cheats.h"

namespace Orin::Overkill
{
	class SelectLevelMenu : public BaseMenu
	{
		void SelectLevel(const char* levelName);

	public:

		META_DATA_DECL_BASE(MainMenu)

#ifndef DOXYGEN_SKIP

		SelectLevelMenu() = default;
		virtual ~SelectLevelMenu() = default;

		void Play() override;		
		void OnVisiblityChange(bool set) override;

		void SetupLevelLabel(int index, const char* levelName);
#endif
	};
}