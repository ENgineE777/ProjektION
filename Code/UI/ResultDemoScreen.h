
#pragma once

#include "Root/Scenes/SceneEntity.h"
#include "Support/MetaData.h"
#include "Support/Sprite.h"
#include "root/Assets/AssetTexture.h"
#include "BaseMenu.h"
#include "SceneEntities/UI/LabelWidget.h"
#include "Characters/Player/ThingAbilities.h"
#include "Characters/Player/Cheats.h"
#include "SceneEntities/UI/ImageWidget.h"

namespace Orin::Overkill
{
	class ResultDemoScreen : public BaseMenu
	{
		LabelWidget* totalTimerValue = nullptr;
		LabelWidget* deathsValue = nullptr;
		LabelWidget* killsValue = nullptr;

		ImageWidget* toBeContinued = nullptr;

		float timerToBe = -1.0f;

	public:

		META_DATA_DECL_BASE(MainMenu)

#ifndef DOXYGEN_SKIP

		ResultDemoScreen() = default;
		virtual ~ResultDemoScreen() = default;

		void Play() override;
		void OnVisiblityChange(bool set) override;
		void Draw(float dt) override;
#endif
	};
}