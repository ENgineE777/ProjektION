
#pragma once

#include "Root/Scenes/SceneEntity.h"
#include "Support/MetaData.h"
#include "Support/Sprite.h"
#include "root/Assets/AssetTexture.h"
#include "BaseMenu.h"
#include "SceneEntities/UI/LabelWidget.h"

namespace Orin::Overkill
{
	class MusicMenu : public BaseMenu
	{
		void ChangeMusicVolume(int value);
		void ChangeSFXVolume(int value);

	public:

		META_DATA_DECL_BASE(MainMenu)

#ifndef DOXYGEN_SKIP

		MusicMenu() = default;
		virtual ~MusicMenu() = default;

		void Play() override;
		void OnVisiblityChange(bool set);
#endif
	};
}