
#include "Mine.h"
#include "Root/Root.h"
#include "World/WorldManager.h"
#include "Characters/Enemies/Soldier.h"

namespace Orin::Overkill
{
	ENTITYREG(SceneEntity, Mine, "Overkill/Characters", "Mine")

	META_DATA_DESC(Mine)
		BASE_SCENE_ENTITY_PROP(Mine)
		INT_PROP(Mine, killScore, 50, "Mine", "killScore", "Kill Score")
		MARK_DISABLED_FOR_INSTANCE()
		FLOAT_OPTIONAL_PROP(Mine, overrideExplodeTimer, 0.6f, "Override", "Override time", "Override time to explode")
		FLOAT_OPTIONAL_PROP(Mine, overrideExplodeRadius, 160.0f * TILE_SCALE, "Override", "Override radius", "Override radius")
	META_DATA_DESC_END()

	void Mine::Init()
	{
		Enemy::Init();

		if (prefabInstance)
		{
			Tasks(true)->AddTask(0, this, (Object::Delegate)&Mine::EditorDraw);
		}

		Tasks(true)->AddTask(9, this, (Object::Delegate)&Mine::DrawExplodeRadius);
	}

	void Mine::Play()
	{
		Enemy::Play();

		beepSound = GetRoot()->GetSounds()->CreateSoundEvent("event:/Bomb/trig");

		radiusVizualisation = FindChild<SpriteEntity>("radius");
		radiusVizualisation->SetVisiblity(false);

		radiusVizualisationStatic = FindChild<SpriteEntity>("radiusStatic");
		radiusVizualisationStatic->SetVisiblity(false);

		if (radiusVizualisation && radiusVizualisationStatic)
		{
			auto size = radiusVizualisation->GetTransform().size;

			radiusScale = 2.0f * GetExplodeRadius() / size.x;

			radiusVizualisation->GetTransform().scale = Math::Vector3(radiusScale, radiusScale, 0.0f);
			radiusVizualisationStatic->GetTransform().scale = Math::Vector3(radiusScale, radiusScale, 0.0f);
		}
	}

	void Mine::Kill(Math::Vector3 killDir, DeathSource deathSource)
	{
		if (killed)
		{
			return;
		}

		Enemy::Kill(killDir, deathSource);

		anim->anim.GotoNode("Boom", true);

		if (beepSound)
		{
			beepSound->Stop();
		}

		Utils::PlaySoundEvent("event:/Bomb/expl", &transform.position);

		WorldManager::instance->Detonate(transform.position, GetExplodeRadius(), this);
	}

	void Mine::OnRestart(const EventOnRestart &evt)
	{
		Enemy::OnRestart(evt);

		triggered = false;
		explodeTimer = -1.0f;	

		animTimer = 0.0f;
	}

	void Mine::AnimateRadius(float dt)
	{
		if (radiusVizualisationStatic)
		{
			radiusVizualisationStatic->color.a = 0.5f;
		}

		if (!radiusVizualisation)
		{
			return;
		}

		animTimer += dt * (triggered ? 2.66f : 1.0f) * 2.0f;

		if (animTimer > 1.8f)
		{
			animTimer -= 1.8f;
		}

		float k = animTimer / 1.8f;
		float scale = k * radiusScale;

		radiusVizualisation->GetTransform().scale = Math::Vector3(scale, scale, 0.0f);
		
		radiusVizualisation->color.a = k < 0.75f ? 1.0f : ((1.0f - k) * 4.0f);
	}	

	void Mine::Update(float dt)
	{
		if (killed)
		{
			return;
		}

		if (WorldManager::instance->GetPlayer() && WorldManager::instance->GetFrontendState() != FrontendState::Gameplay)
		{
			return;
		}

		AnimateRadius(dt);

		if (WorldManager::instance->IsPlayerInSphere(transform.position, instantExplodeRadius))
		{
			Kill(0.0f, DeathSource::Unknown);
			return;
		}

		if (triggered)
		{
			explodeTimer -= dt;

			if (explodeTimer < 0.0f)
			{
				Kill(0.0f, DeathSource::Unknown);
			}
		}
		else
		{
			if (WorldManager::instance->IsPlayerInSphere(transform.position, GetExplodeRadius()))
			{
				anim->anim.GotoNode("Activated", true);

				triggered = true;
				explodeTimer = timeToExplode * WorldManager::instance->GetDifficulty()->mineTimerMul;

				if (beepSound)
				{
					beepSound->Set3DAttributes(transform.position);
					beepSound->Play();
				}

				if (overrideExplodeTimer.enabled)
				{
					explodeTimer = overrideExplodeTimer.value * WorldManager::instance->GetDifficulty()->mineTimerMul;
				}
			}
		}
	}

	void Mine::EditorDraw(float dt)
	{
		if (!GetScene()->IsPlaying())
		{
			Sprite::DebugSphere(transform.position, GetExplodeRadius(), COLOR_RED_A(0.25f));
		}
	}

	void Mine::DrawRadius(SpriteEntity* entity, float dt)
	{
		Transform trans = entity->GetTransform();

		//entity->texture.prg = Sprite::quadPrgNoZ;
		//entity->texture.Draw(&trans, entity->color, 0.0f);

		//WorldManager::instance->RenderShadow1D(0, { trans.position.x, trans.position.y }, trans.size.x);
		//WorldManager::instance//->RenderSpriteWithShadow1D(0, entity->color, trans, entity->texture);
	}

	void Mine::DrawExplodeRadius(float dt)
	{
		if (!GetScene()->IsPlaying())
		{
			return;
		}

		if (killed)
		{
			return;
		}

		Transform trans = radiusVizualisationStatic->GetTransform();
		
		auto radiusPos = Sprite::ToPixels(trans.GetGlobal().Pos());

		auto pos = Math::Vector2(radiusPos.x, radiusPos.y);

		if (!Sprite::IsRectVisibile(pos - GetExplodeRadius(), pos + GetExplodeRadius()))
		{
			return;
		}		

		//entity->texture.prg = Sprite::quadPrgNoZ;
		//entity->texture.Draw(&trans, entity->color, 0.0f);

		WorldManager::instance->RenderShadow1D(0, pos, GetExplodeRadius());

		WorldManager::instance->RenderSpriteWithShadow1D(0, radiusVizualisationStatic->color, trans, GetExplodeRadius(), radiusVizualisationStatic->texture);

		trans = radiusVizualisation->GetTransform();
		WorldManager::instance->RenderSpriteWithShadow1D(0, radiusVizualisation->color, trans, GetExplodeRadius(), radiusVizualisation->texture);
	}

	void Mine::Release()
	{
		RELEASE(beepSound);

		Enemy::Release();
	}
}