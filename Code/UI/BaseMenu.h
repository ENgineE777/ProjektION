
#pragma once

#include "Root/Scenes/SceneEntity.h"
#include "Support/MetaData.h"
#include "Support/Sprite.h"
#include "root/Assets/AssetTexture.h"
#include "SceneEntities/UI/ContainerWidget.h"
#include "SceneEntities/UI/LabelWidget.h"

namespace Orin::Overkill
{
	class BaseMenu : public ContainerWidget
	{
	protected:

		enum SoundType
		{
			ChangeSelection,
			Activate,
			GoBack
		};

		Color selectedLabel;
		
		struct Item
		{
			LabelWidget* label = nullptr;
			bool goBack = false;
			bool enabled = true;
			eastl::function<void()> callback;
			eastl::function<void()> callbackLeft;
			eastl::function<void()> callbackRight;
		};

		int curItem = 0;
		eastl::vector<Item> items;
		void PlaySound(SoundType soundType);

	public:

	#ifndef DOXYGEN_SKIP

		BaseMenu() = default;
		virtual ~BaseMenu() = default;

		void Draw(float dt) override;

		void OnVisiblityChange(bool set) override;
	#endif
	};
}