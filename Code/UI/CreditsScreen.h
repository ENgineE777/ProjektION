
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
	class CreditsScreen : public BaseMenu
	{
		float posY = 0.0f;
		ContainerWidget* credits = nullptr;

	public:

		META_DATA_DECL_BASE(MainMenu)

#ifndef DOXYGEN_SKIP

		CreditsScreen() = default;
		virtual ~CreditsScreen() = default;

		void Play() override;		
		void Draw(float dt) override;
		void OnVisiblityChange(bool set) override;
#endif
	};
}