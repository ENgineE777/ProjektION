
#include "SplashScreens.h"
#include "World/WorldManager.h"

namespace Orin::Overkill
{
	ENTITYREG(SceneEntity, SplashScreens, "Overkill/UI", "SplashScreens")

	META_DATA_DESC(SplashScreens::Screen)
		FLOAT_PROP(SplashScreens::Screen, time, 2.0f, "Property", "time", "time")
		FLOAT_PROP(SplashScreens::Screen, scale, 1.0f, "Property", "scale", "scale")
		ASSET_TEXTURE_PROP(SplashScreens::Screen, image, "Property", "image")
	META_DATA_DESC_END()

	META_DATA_DESC(SplashScreens)
		BASE_SCENE_ENTITY_PROP(SplashScreens)

		ARRAY_PROP(SplashScreens, screens, Screen, "PropertiesTarget", "screens")

	META_DATA_DESC_END()

	void SplashScreens::Init()
	{
		ScriptEntity2D::Init();

		Tasks(true)->AddTask(10, this, (Object::Delegate)&SplashScreens::Draw);
	}

	void SplashScreens::NextScreen()
	{		
		if (curScreen == screens.size() - 1)
		{
			WorldManager::instance->SetFrontendState(FrontendState::MainMenu);
		}
		else
		{
			curScreen++;
		
			timer = screens[curScreen].time;
			WorldManager::instance->StartFade(false, 0.5f, [this]() { timerOn = true; });
		}
	}

	void SplashScreens::Draw(float dt)
	{
		if (GetScene()->IsPlaying() && IsVisible())
		{
			int device_index = 0;
			if (GetRoot()->GetControls()->GetActivatedKey(device_index))
			{
				NextScreen();
			}

			if (curScreen == -1)
			{
				NextScreen();
			}

			if (timerOn)
			{
				timer -= dt;

				if (timer < 0.0f)
				{
					timerOn = false;

					WorldManager::instance->StartFade(true, 0.5f, [this]() { NextScreen(); });
				}
			}

			Transform trans;
			trans.objectType = ObjectType::Object2D;
			trans.size = screens[curScreen].image.GetSize();
			trans.scale = screens[curScreen].scale;
			trans.position = Math::Vector3(Sprite::GetCamPos().x, Sprite::GetCamPos().y, 0.0f);
			screens[curScreen].image.Draw(&trans, COLOR_WHITE, dt);
		}	
	}
}