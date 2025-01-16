
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
	class SelectDifficultyMenu : public BaseMenu
	{
		void SelectDifficulty(const char* diffuculty);
		void SetupDifficultyLabel(int index, const char* diffuculty);

	public:

		eastl::string levelName;

		META_DATA_DECL_BASE(MainMenu)

#ifndef DOXYGEN_SKIP

		SelectDifficultyMenu() = default;
		virtual ~SelectDifficultyMenu() = default;

		void Play() override;		
		void OnVisiblityChange(bool set) override;		
#endif
	};
}