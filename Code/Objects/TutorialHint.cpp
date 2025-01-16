
#include "TutorialHint.h"
#include "Root/Root.h"
#include "World/WorldManager.h"
#include "World/BloodManager.h"
#include "SceneEntities/Physics/2D/PhysEntity2D.h"
#include "TutorialHintActivator.h"

namespace Orin::Overkill
{
	ENTITYREG(SceneEntity, TutorialHint, "Overkill/Objects", "TutorialHint")


	META_DATA_DESC(TutorialHint::Hint)
		STRING_PROP(TutorialHint::Hint, locale, "", "Visual", "locale")
		ASSET_TEXTURE_PROP(TutorialHint::Hint, texture, "Visual", "Texture")
		ASSET_TEXTURE_PROP(TutorialHint::Hint, textureGamePad, "Visual", "TextureGamePad")
	META_DATA_DESC_END()		

	META_DATA_DESC(TutorialHint)
		BASE_SCENE_ENTITY_PROP(TutorialHint)		

		ARRAY_PROP(TutorialHint, hints, Hint, "Properties", "hints")

	META_DATA_DESC_END()

	ORIN_EVENTS(TutorialHint)
		ORIN_EVENT(TutorialHint, EventOnRestart, OnRestart)
	ORIN_EVENTS_END(TutorialHint)

	void TutorialHint::Init()
	{
		transform.objectType = ObjectType::Object2D;
		transform.transformFlag = (TransformFlag)(TransformFlag::SpriteTransformFlags ^ TransformFlag::RectSizeXY);	
	}

	void TutorialHint::ApplyProperties()
	{
		Math::Vector2 size = 100.0f;
		transform.size = Math::Vector3(size.x, size.y, 0.0f);

#ifdef ORIN_EDITOR
		Tasks(true)->DelAllTasks(this);
#endif

		Tasks(true)->AddTask(7, this, (Object::Delegate)&TutorialHint::Draw);
	}

	void TutorialHint::Play()
	{
		SceneEntity::Play();

		ORIN_EVENTS_SUBSCRIBE(TutorialHint);

		GetRoot()->Log("TutorialHint", "Play");

		if (childs.size() > 0)
		{
			auto* activator = dynamic_cast<TutorialHintActivator*>(childs[0]);

			if (activator)
			{
				GetRoot()->Log("TutorialHint", "activator");
				activator->SetHint(this);
			}
		}

		if (childs.size() > 1)
		{
			auto* deactivator = dynamic_cast<TutorialHintActivator*>(childs[1]);

			if (deactivator)
			{
				GetRoot()->Log("TutorialHint", "deactivator");
				deactivator->SetHint(this);
			}
		}
	}

	void TutorialHint::Acivate(bool setActive)
	{
		if (activated && setActive)
		{
			return;
		}

		active = setActive;

		if (active)
		{
			activated = true;
		}
	}

	void TutorialHint::OnRestart(const EventOnRestart& evt)
	{
		activated = false;
		active = false;
		alpha = 0.0f;
	}

	void TutorialHint::Draw(float dt)
	{
		if (!GetScene()->IsPlaying())
		{
			auto hintPos = Sprite::ToPixels(transform.GetGlobal().Pos());

			if (childs.size() > 0)
			{
				auto* activator = dynamic_cast<TutorialHintActivator*>(childs[0]);

				if (activator)
				{
					auto pos = Sprite::ToPixels(activator->GetTransform().GetGlobal().Pos());

					Sprite::DebugText({ pos.x, pos.y }, Color(1.0f, 0.75f, 1.0f), activator->activator ? "Activator" : "Deactivator");
					Sprite::DebugLine(pos, hintPos, Color(1.0f, 0.75f, 1.0f));
				}
			}

			if (childs.size() > 1)
			{
				auto* activator = dynamic_cast<TutorialHintActivator*>(childs[1]);

				if (activator)
				{
					auto pos = Sprite::ToPixels(activator->GetTransform().GetGlobal().Pos());

					Sprite::DebugText({ pos.x, pos.y }, Color(1.0f, 0.75f, 1.0f), activator->activator ? "Activator" : "Deactivator");
					Sprite::DebugLine(pos, hintPos, Color(1.0f, 0.75f, 1.0f));
				}
			}
		}
		else
		{
			alpha += dt * 2.5f * (active ? 1.0f : -1.0f);
			alpha = Math::Clamp(alpha, 0.0f, 1.0f);
		}

		if (alpha > 0.01f)
		{
			const char* locale = "en";

			if (GetScene()->IsPlaying())
			{
				locale = WorldManager::instance->gameState.locale.c_str();
			}
			

			for (auto& hint : hints)
			{
				if (StringUtils::IsEqual(hint.locale.c_str(), locale))
				{
					if (hint.textureGamePad && WorldManager::instance && WorldManager::instance->gamepadConnected)
					{
						Math::Vector2 size = hint.textureGamePad.GetSize();
						transform.size = Math::Vector3(size.x, size.y, 0.0f);
						hint.textureGamePad.Draw(&transform, COLOR_WHITE_A(alpha), dt);
					}
					else
					{
						Math::Vector2 size = hint.texture.GetSize();
						transform.size = Math::Vector3(size.x, size.y, 0.0f);
						hint.texture.Draw(&transform, COLOR_WHITE_A(alpha), dt);
					}
				}
			}			
		}
	}

	void TutorialHint::Release()
	{
		MainEventsQueue::Unsubscribe(this);

		SceneEntity::Release();
	}
}