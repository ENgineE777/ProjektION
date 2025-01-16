
#include "CutSceneOutro.h"
#include "World/WorldManager.h"
#include "Characters/Enemies/Puppet.h"
#include "TutorialHint.h"

namespace Orin::Overkill
{
	ENTITYREG(SceneEntity, CutSceneOutro, "Overkill/CutScene", "CutSceneOutro")

	META_DATA_DESC(CutSceneOutro)
		BASE_SCENE_ENTITY_PROP(CutSceneOutro)

	META_DATA_DESC_END()

	eastl::string CutSceneOutro::dialogTexts[] =
	{
		"Outro_Dialog1",		
		"Outro_Dialog2",
		"Outro_Dialog3",		
		"Outro_Dialog4"		
	};

	void CutSceneOutro::Init()
	{
		ScriptEntity2D::Init();

		Tasks(true)->AddTask(10, this, (Object::Delegate)&CutSceneOutro::Draw);
	}

	void CutSceneOutro::Play()
	{
		ScriptEntity2D::Play();

		footstepsSound = GetRoot()->GetSounds()->CreateSoundEvent("event:/FatGuy/Footsteps");
		
		//NextScene();
	}

	void CutSceneOutro::Start()
	{
		SetVisiblity(true);
		NextScene();

		WorldManager::instance->StartFade(false, 0.5f, nullptr);
	}

	void CutSceneOutro::SetNextDialog(SpriteEntity* dialog)
	{
		curDialog++;

		letterTime = 0.0f;
		curDialogLen = 0;

		dialogPos = Sprite::ToPixels(dialog->GetTransform().GetGlobal().Pos());
		curDialogText = GetRoot()->GetLocalization()->GetLocalized(dialogTexts[curDialog]);
	}

	void CutSceneOutro::ShowDialogText(float dt)
	{
		int height = GetRoot()->GetRender()->GetDevice()->GetHeight();
		float k = (float)height / Sprite::GetPixelsHeight();

		if (height != screenHeight)
		{
			int calculatedFontHeight = (int)(k * 5.25f);
			font = GetRoot()->GetFonts()->LoadFont("UI/conthrax-sb.otf", false, false, calculatedFontHeight, 2.0f, _FL_);

			screenHeight = height;
		}

		float scale = GetRoot()->GetRender()->GetDevice()->GetHeight() / Sprite::GetPixelsHeight();

		auto screenPos = (Sprite::GetCamPos() - Math::Vector2(dialogPos.x, dialogPos.y)) * scale;

		auto halfScreenSize = Sprite::GetHalfScreenSize();

		screenPos.x = halfScreenSize.x * scale - screenPos.x;
		screenPos.y = 2.0f * halfScreenSize.y * scale - (halfScreenSize.y * scale - screenPos.y);
		
		Math::Matrix mat;
		mat.Pos().z = 0.0f;

		mat.Pos().x = screenPos.x;
		mat.Pos().y = screenPos.y;
		
		eastl::wstring bufferU16;
		StringUtils::Utf8toUtf16(bufferU16, curDialogText.c_str());

		if (curDialogLen < bufferU16.size())
		{
			timer = 0.0f;

			letterTime += dt;

			if (letterTime > 0.05f)
			{
				Utils::PlaySoundEvent("event:/Text");

				letterTime -= 0.05f;

				curDialogLen++;

				if (bufferU16[curDialogLen] == '\\')
				{
					curDialogLen += 2;
				}
			}
		}
		else
		{
			timerMax = 1.5f;
		}

		bufferU16.resize(curDialogLen);

		eastl::string bufferU8;
		StringUtils::Utf16toUtf8(bufferU8, bufferU16.c_str());		

		font.Print(mat, 1.0f, COLOR_WHITE, bufferU8.c_str());
	}

	void CutSceneOutro::InitializeStars(Node2D* root)
	{
		starBrights.clear();

		if (auto* stars = root->FindChild<SpriteEntity>("stars1"))
		{
			starBrights.push_back(StarBright(stars));
		}

		if (auto* stars = root->FindChild<SpriteEntity>("stars2"))
		{
			starBrights.push_back(StarBright(stars));
		}

		if (auto* stars = root->FindChild<SpriteEntity>("stars3"))
		{
			starBrights.push_back(StarBright(stars));
		}
	}

	void CutSceneOutro::UpdateStarBrights(float dt)
	{
		for (auto& entry : starBrights)
		{
			entry.timer += entry.timerSpeed * dt;
			entry.layer->emmisiveIntencity = sinf(entry.timer) + entry.intesityPoint;
		}
	}

	void CutSceneOutro::NextScene()
	{		
		curScene++;

		if (curScene == 0)
		{	
			MainEventsQueue::PushEvent(CmdAllowCameraMove{ false });

			WorldManager::instance->hackDisallowCursor = true;
			
			auto* scene = FindChild<Node2D>("captainroom");

			InitializeStars(scene);

			puppet = scene->FindChild<AnimGraph2D>("puppet");
			puppet->SetVisiblity(true);

			puppetFrom = scene->FindChild<Node2D>("puppetFrom");
			puppetTo = scene->FindChild<Node2D>("puppetTo");

			auto* captain = scene->FindChild<SpriteEntity>("captain");
			captain->SetVisiblity(true);

			captain = scene->FindChild<SpriteEntity>("captainBack");
			captain->SetVisiblity(false);

			puppet->anim.GotoNode("walk", true);

			puppet->GetTransform().position = puppetFrom->GetTransform().position;

			WorldManager::instance->SetCameraTarget(scene, true);

			timerMax = 2.0f;
		}
		else
		if (curScene == 1)
		{
			puppet->anim.ActivateLink("idle");

			auto* scene = FindChild<Node2D>("captainroom");

			auto* dialog = scene->FindChild<SpriteEntity>("bubble_left");
			dialog->SetVisiblity(true);

			SetNextDialog(dialog);

			timerMax = 3.0f;
		}
		else
		if (curScene == 2)
		{
			auto* scene = FindChild<Node2D>("captainroom");

			auto* dialog = scene->FindChild<SpriteEntity>("bubble_left");
			dialog->SetVisiblity(false);			

			dialog = scene->FindChild<SpriteEntity>("bubble_right");
			dialog->SetVisiblity(true);

			SetNextDialog(dialog);

			timerMax = 3.0f;
		}
		else
		if (curScene == 3)
		{
			auto* scene = FindChild<Node2D>("captainroom");

			auto* dialog = scene->FindChild<SpriteEntity>("bubble_right");
			dialog->SetVisiblity(false);			

			dialog = scene->FindChild<SpriteEntity>("bubble_left");
			dialog->SetVisiblity(true);

			SetNextDialog(dialog);

			timerMax = 3.0f;
		}
		else
		if (curScene == 4)
		{
			auto* scene = FindChild<Node2D>("captainroom");

			auto* dialog = scene->FindChild<SpriteEntity>("bubble_left");
			dialog->SetVisiblity(false);			

			dialog = scene->FindChild<SpriteEntity>("bubble_right");
			dialog->SetVisiblity(true);

			SetNextDialog(dialog);

			timerMax = 3.0f;
		}
		else
		if (curScene == 5)
		{
			auto* scene = FindChild<Node2D>("captainroom");

			auto* dialog = scene->FindChild<SpriteEntity>("bubble_right");
			dialog->SetVisiblity(false);			

			auto* captain = scene->FindChild<SpriteEntity>("captain");
			captain->SetVisiblity(false);

			captain = scene->FindChild<SpriteEntity>("captainBack");
			captain->SetVisiblity(true);

			timerMax = 2.0f;
		}
		else		
		if (curScene == 6)
		{
			WorldManager::instance->SetFrontendStateViaFade(FrontendState::ResultDemoScreen);
			WorldManager::instance->hackDisallowCursor = false;
		}
		
		timer = 0.0f;
	}

	void CutSceneOutro::Draw(float dt)
	{
		if (WorldManager::instance && WorldManager::instance->GetFrontendState() != FrontendState::Gameplay)
		{
			return;
		}

		if (GetScene()->IsPlaying() && IsVisible())
		{
			UpdateStarBrights(dt);

			bool useFootstepSound = false;

			int device_index = 0;
			if ((curScene == 1 || curScene == 2 || curScene == 3 || curScene == 4) &&
				GetRoot()->GetControls()->GetActivatedKey(device_index))
			{
				NextScene();
			}

			timer += dt;

			if (timer > timerMax)
			{
				NextScene();
			}

			if (curScene == 0)
			{
				WorldManager::instance->SetMusicTheme(MusicTheme::ThemeCaptainRoom, 0.5f);

				auto from = puppetFrom->GetTransform().position;
				auto to = puppetTo->GetTransform().position;
				auto dir = (to - from);
				auto len = dir.Normalize();

				puppet->GetTransform().position = from + dir * len * (timer / timerMax);

				useFootstepSound = true;
			}
			else
			if (curScene == 1 || curScene == 2 || curScene == 3 || curScene == 4 || curScene == 5)
			{
				if (curScene != 5)
				{
					ShowDialogText(dt);
				}
			}

			footstepTimer += dt;

			if (useFootstepSound && footstepTimer >= footstepsInterval)
			{
				footstepTimer -= footstepsInterval;

				Utils::PlaySoundEvent("event:/FatGuy/Footsteps", &transform.position);
			}
		}	
	}

	void CutSceneOutro::Release()
	{
		MainEventsQueue::PushEvent(CmdAllowCameraMove{ true });

		MainEventsQueue::Unsubscribe(this);

		RELEASE(footstepsSound)

		ScriptEntity2D::Release();
	}
}