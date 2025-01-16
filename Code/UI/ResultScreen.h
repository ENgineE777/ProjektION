
#pragma once

#include "Root/Scenes/SceneEntity.h"
#include "Support/MetaData.h"
#include "Support/Sprite.h"
#include "root/Assets/AssetTexture.h"
#include "SceneEntities/UI/ImageWidget.h"
#include "SceneEntities/UI/LabelWidget.h"

namespace Orin::Overkill
{
	class ResultScreen : public ImageWidget
	{
	public:

		enum class State
		{
			None,
			Won,
			Lose
		};

	private:
		LabelWidget* timerValue = nullptr;
		LabelWidget* totalTimerValue = nullptr;
		ContainerWidget* scoreContainer = nullptr;
		LabelWidget* scoreValue = nullptr;
		LabelWidget* deathsValue = nullptr;
		LabelWidget* killsValue = nullptr;
		LabelWidget* wonLabel = nullptr;
		LabelWidget* loseLabel = nullptr;

		LabelWidget* restartLabel = nullptr;
		LabelWidget* restartLabelGamepad = nullptr;
		LabelWidget* continueLabel = nullptr;
		LabelWidget* continueLabelGamepad = nullptr;

		State state = State::Won;

		int aliasContinue = -1;
		int aliasRestart = -1;
	public:

		META_DATA_DECL_BASE(ResultSreen)

	#ifndef DOXYGEN_SKIP

		ResultScreen() = default;
		virtual ~ResultScreen() = default;

		void SetState(ResultScreen::State newState, int rating);
		void Init() override;
		void Play() override;
		void Draw(float dt);
	#endif
	};
}