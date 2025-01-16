
#pragma once

#include "SceneEntities/2D/ScriptEntity2D.h"
#include "root/Assets/AssetTexture.h"

namespace Orin::Overkill
{
	class SplashScreens : public ScriptEntity2D
	{
		struct Screen
		{
			META_DATA_DECL_BASE(Screen)

			float time = 2.0f;
			float scale = 1.0f;
			AssetTextureRef image;
		};

		int curScreen = -1;
		bool timerOn = false;
		float timer = 0.0f;
		eastl::vector<Screen> screens;

		void NextScreen();

	public:

		META_DATA_DECL_BASE(SplashScreens)

	#ifndef DOXYGEN_SKIP

		SplashScreens() = default;;
		virtual ~SplashScreens() = default;

		void Init() override;
		void Draw(float dt);
	#endif
	};
}