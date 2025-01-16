
#pragma once

#include "SceneEntities/2D/ScriptEntity2D.h"
#include "SceneEntities/2D/Node2D.h"
#include "SceneEntities/2D/Camera2D.h"
#include "SceneEntities/2D/SpriteEntity.h"
#include "SceneEntities/2D/AnimGraph2D.h"
#include "root/Assets/AssetTexture.h"
#include "Root/Sounds/SoundEvent.h"
#include "Root/Fonts/FontRef.h"
#include "Common/Events.h"
#include "Common/Utils.h"

namespace Orin::Overkill
{
	class CutSceneSendJoe : public ScriptEntity2D
	{
		AnimGraph2D* puppet = nullptr;
		Node2D* puppetFrom = nullptr;
		Node2D* puppetTo = nullptr;

		struct StarBright
		{
			SpriteEntity* layer;
			float timer = 0.0f;
			float timerSpeed = 1.0f;
			float intesityPoint = 2.0f;

			StarBright(SpriteEntity* setLayer)
			{
				layer = setLayer;

				timer = Utils::GetRandom(0.0f, 1000.0f);
				timerSpeed = Utils::GetRandom(2.5f, 5.0f);
				intesityPoint = Utils::GetRandom(2.0f, 3.0f);
			}
		};

		eastl::vector<StarBright> starBrights;

		int curScene = -1;
		float timerMax = 0.0f;
		float timer = 0.0f;		
		
		bool puppetWalkWithPlayer = true;
		void NextScene();

		SoundEvent* footstepsSound = nullptr;
		float footstepTimer = 0.f;
		float footstepsInterval = 0.35f;

		Math::Vector3 dialogPos;

		static eastl::string dialogTexts[];

		float letterTime = 0.0f;
		int curDialogLen = 0;
		int curDialog = -1;
		eastl::string curDialogText;

		void SetNextDialog(SpriteEntity* dialog);

		int screenHeight = -1;
		FontRef font;

		void ShowDialogText(float dt);

		void InitializeStars(Node2D* root);
		void UpdateStarBrights(float dt);

	public:

		META_DATA_DECL_BASE(CutSceneSendJoe)

	#ifndef DOXYGEN_SKIP

		CutSceneSendJoe() = default;
		virtual ~CutSceneSendJoe() = default;

		void Init() override;
		void Play() override;
		void Draw(float dt);
		void OnFinishLevel(const EventOnFinishLevel& evt);
		void Release() override;

	#endif
	};
}