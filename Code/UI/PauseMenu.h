
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
	class PauseMenu : public BaseMenu
	{
		int keyIndex = 0;

		void ShowAndHandleAbilityChange(int& dbgLine, ThingAbility ability);
		void ShowAndHandleCheatChange(int& dbgLine, Cheat cheat);

	public:

		META_DATA_DECL_BASE(MainMenu)

#ifndef DOXYGEN_SKIP

		PauseMenu() = default;
		virtual ~PauseMenu() = default;

		void Play() override;		
		void Draw(float dt) override;
#endif
	};
}