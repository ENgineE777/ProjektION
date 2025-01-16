
#include "CutSceneIntro.h"
#include "World/WorldManager.h"
#include "Characters/Enemies/Puppet.h"
#include "TutorialHint.h"

namespace Orin::Overkill
{
	ENTITYREG(SceneEntity, CutSceneIntro, "Overkill/CutScene", "IntroCutScene")

	META_DATA_DESC(CutSceneIntro)
		BASE_SCENE_ENTITY_PROP(CutSceneIntro)

	META_DATA_DESC_END()

	ORIN_EVENTS(CutSceneIntro)
		ORIN_EVENT(CutSceneIntro, EventOnFinishLevel, OnFinishLevel)
	ORIN_EVENTS_END(CutSceneIntro)

	eastl::string CutSceneIntro::dialogTexts[] =
	{
		"Intro_Dialog1",		
		"Intro_Dialog2",
		"Intro_Dialog3",		
		"Intro_Dialog4",		
		"Intro_Dialog5",		
		"Intro_Dialog6",		
		"Intro_Dialog7",		
		"Intro_Dialog8",		
		"Intro_Dialog9"		
	};

	void CutSceneIntro::Init()
	{
		ScriptEntity2D::Init();

		Tasks(true)->AddTask(10, this, (Object::Delegate)&CutSceneIntro::Draw);
	}

	void CutSceneIntro::Play()
	{
		ScriptEntity2D::Play();

		ORIN_EVENTS_SUBSCRIBE(CutSceneIntro);

		footstepsSound = GetRoot()->GetSounds()->CreateSoundEvent("event:/FatGuy/Footsteps");

		if (WorldManager::instance->gameState.GetCheckpointID() != -1)
		{
			SetVisiblity(false);

			auto* puppetTutorial = GetScene()->FindEntity<Puppet>("puppetTutorial", true);
			puppetTutorial->SetVisiblity(false);
			puppetTutorial->SwitchToNormal();
		}
		else
		{
			NextScene();
		}
	}	

	void CutSceneIntro::SetNextDialog(SpriteEntity* dialog)
	{
		curDialog++;

		letterTime = 0.0f;
		curDialogLen = 0;

		dialogPos = Sprite::ToPixels(dialog->GetTransform().GetGlobal().Pos());
		curDialogText = GetRoot()->GetLocalization()->GetLocalized(dialogTexts[curDialog]);
	}

	void CutSceneIntro::ShowDialogText(float dt)
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

	void CutSceneIntro::InitializeStars(Node2D* root)
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

	void CutSceneIntro::UpdateStarBrights(float dt)
	{
		for (auto& entry : starBrights)
		{
			entry.timer += entry.timerSpeed * dt;
			entry.layer->emmisiveIntencity = sinf(entry.timer) + entry.intesityPoint;
		}
	}

	void CutSceneIntro::NextScene()
	{		
		curScene++;

		if (curScene == 0)
		{	
			MainEventsQueue::PushEvent(CmdAllowCameraMove{ false });

			Utils::PlaySoundEvent("event:/CutScene/1");

			WorldManager::instance->SetMusicTheme(MusicTheme::ThemeIntro, 0.5f);
			WorldManager::instance->hackDisallowCursor = true;

			auto* scene = FindChild<Node2D>("Scene01");

			InitializeStars(scene);

			pod = scene->FindChild<SpriteEntity>("pod");
			podFrom = scene->FindChild<Node2D>("podFrom");
			podTo = scene->FindChild<SceneEntity>("podTo");

			auto from = podFrom->GetTransform().position;
			podDir = podTo->GetTransform().position - from;
			podDir.Normalize();

			podSpeed = 10.0f;

			WorldManager::instance->SetCameraTarget(scene, true);
			
			timerMax = 4.0f;

			pod->GetTransform().position = podFrom->GetTransform().position;
		}
		else
		if (curScene == 1)
		{
			auto* scene = FindChild<Node2D>("Scene02");

			InitializeStars(scene);

			pod = scene->FindChild<SpriteEntity>("pod");
			podFrom = scene->FindChild<Node2D>("podFrom");
			podTo = scene->FindChild<SceneEntity>("podTo");

			auto from = podFrom->GetTransform().position;
			podDir = podTo->GetTransform().position - from;
			podDir.Normalize();

			WorldManager::instance->SetCameraTarget(scene, true);

			podSpeed = 30.0f;

			timerMax = 5.0f;

			pod->GetTransform().position = podFrom->GetTransform().position;
		}
		else
		if (curScene == 2)
		{
			auto* scene = FindChild<Node2D>("Scene03");

			InitializeStars(scene);

			pod = scene->FindChild<SpriteEntity>("pod");
			podFrom = scene->FindChild<Node2D>("podFrom");
			podTo = scene->FindChild<SceneEntity>("podTo");

			ship = FindChild<SpriteEntity>("ship");
			shipFrom = FindChild<Node2D>("shipFrom");
			shipTo = FindChild<Node2D>("shipTo");


			auto from = podFrom->GetTransform().position;
			podDir = podTo->GetTransform().position - from;
			podDir.Normalize();

			from = shipFrom->GetTransform().position;
			shipDir = shipTo->GetTransform().position - from;
			shipDir.Normalize();

			WorldManager::instance->SetCameraTarget(scene, true);

			timerMax = 3.5f;

			podSpeed = 10.0f;

			pod->GetTransform().position = podFrom->GetTransform().position;
			ship->GetTransform().position = shipFrom->GetTransform().position;
		}
		else
		if (curScene == 3)
		{
			timerMax = 3.5f;
		}
		else
		if (curScene == 4)
		{
			timerMax = 0.5f;
		}
		else
		if (curScene == 5)
		{
			auto* scene = FindChild<Node2D>("Scene04");

			InitializeStars(scene);

			pod = scene->FindChild<SpriteEntity>("pod");

			puppet = scene->FindChild<AnimGraph2D>("puppet");
			puppet->SetVisiblity(true);

			puppetFrom = scene->FindChild<Node2D>("puppetFrom");
			puppetTo = scene->FindChild<Node2D>("puppetTo");

			auto* captain = scene->FindChild<SpriteEntity>("captain");
			captain->SetVisiblity(true);

			captain = scene->FindChild<SpriteEntity>("captainBack");
			captain->SetVisiblity(false);

			puppet->anim.ActivateLink("walk");

			puppet->GetTransform().position = puppetFrom->GetTransform().position;

			WorldManager::instance->SetCameraTarget(scene, true);

			timerMax = 2.0f;
		}
		else
		if (curScene == 6)
		{
			puppet->anim.ActivateLink("idle");

			auto* scene = FindChild<Node2D>("Scene04");

			auto* dialog = scene->FindChild<SpriteEntity>("bubble_left");
			dialog->SetVisiblity(true);

			SetNextDialog(dialog);

			timerMax = 3.0f;
		}
		else
		if (curScene == 7)
		{
			auto* scene = FindChild<Node2D>("Scene04");

			auto* dialog = scene->FindChild<SpriteEntity>("bubble_left");
			dialog->SetVisiblity(false);			

			dialog = scene->FindChild<SpriteEntity>("bubble_right");
			dialog->SetVisiblity(true);

			SetNextDialog(dialog);

			timerMax = 3.0f;
		}
		else
		if (curScene == 8)
		{
			auto* scene = FindChild<Node2D>("Scene04");

			auto* dialog = scene->FindChild<SpriteEntity>("bubble_right");
			dialog->SetVisiblity(false);			

			dialog = scene->FindChild<SpriteEntity>("bubble_left");
			dialog->SetVisiblity(true);

			SetNextDialog(dialog);

			timerMax = 3.0f;
		}
		else
		if (curScene == 9)
		{
			auto* scene = FindChild<Node2D>("Scene04");

			auto* dialog = scene->FindChild<SpriteEntity>("bubble_left");
			dialog->SetVisiblity(false);			

			dialog = scene->FindChild<SpriteEntity>("bubble_right");
			dialog->SetVisiblity(true);

			SetNextDialog(dialog);

			timerMax = 3.0f;
		}
		else
		if (curScene == 10)
		{
			auto* scene = FindChild<Node2D>("Scene04");

			auto* dialog = scene->FindChild<SpriteEntity>("bubble_right");
			dialog->SetVisiblity(false);			

			auto* captain = scene->FindChild<SpriteEntity>("captain");
			captain->SetVisiblity(false);

			captain = scene->FindChild<SpriteEntity>("captainBack");
			captain->SetVisiblity(true);

			timerMax = 2.0f;
		}
		else
		if (curScene == 11)
		{
			Utils::PlaySoundEvent("event:/CutScene/2");

			podRotSpeed = 20.0f;

			auto* scene = FindChild<Node2D>("Scene05");

			InitializeStars(scene);

			pod = scene->FindChild<SpriteEntity>("pod");
			podFrom = scene->FindChild<Node2D>("podFrom");
			podTo = scene->FindChild<SceneEntity>("podTo");

			WorldManager::instance->SetCameraTarget(scene, true);

			timerMax = 5.0f;			
		}
		else
		if (curScene == 12)
		{
			Utils::PlaySoundEvent("event:/CutScene/3");

			auto* scene = FindChild<Node2D>("Scene06");

			puppet = scene->FindChild<AnimGraph2D>("puppet");
			puppet->SetVisiblity(true);
			puppet->GetTransform().scale = Math::Vector3(-1.0f, 1.0f, 1.0f);
			puppet->anim.ActivateLink("idle");

			puppetFrom = scene->FindChild<Node2D>("puppetFrom");

			puppet->GetTransform().position = puppetFrom->GetTransform().position;

			pod = scene->FindChild<SpriteEntity>("pod");
			podFrom = scene->FindChild<Node2D>("podFrom");
			podTo = scene->FindChild<SceneEntity>("podMidlle");
			
			WorldManager::instance->SetCameraTarget(scene, true);
			
			timerMax = 3.0f;

			auto* puppetTutorial = GetScene()->FindEntity<Puppet>("puppetTutorial", true);
			puppetTutorial->SetVisiblity(false);
		}
		else
		if (curScene == 13)
		{
			auto* scene = FindChild<Node2D>("Scene06");

			pod = scene->FindChild<SpriteEntity>("pod");
			podFrom = scene->FindChild<Node2D>("podMidlle");
			podTo = scene->FindChild<SceneEntity>("podTo");

			timerMax = 2.0f;			
		}
		else
		if (curScene == 14)
		{
			auto* puppetTutorial = GetScene()->FindEntity<Puppet>("puppetTutorial", true);
			puppetTutorial->SetVisiblity(true);

			auto* scene = FindChild<Node2D>("Scene06");
			
			puppetTo = scene->FindChild<Node2D>("puppetTo");

			puppet->anim.ActivateLink("walk");

			timerMax = 2.0f;			
		}
		else
		if (curScene == 15)
		{
			puppet->anim.ActivateLink("idle");

			auto* scene = FindChild<Node2D>("Scene06");

			auto* dialog = scene->FindChild<SpriteEntity>("bubble_right");
			dialog->SetVisiblity(true);

			SetNextDialog(dialog);

			timerMax = 3.0f;
		}
		else
		if (curScene == 16)
		{
			auto* scene = FindChild<Node2D>("Scene06");

			auto* dialog = scene->FindChild<SpriteEntity>("bubble_right");
			SetNextDialog(dialog);

			timerMax = 3.0f;
		}
		else
		if (curScene == 17)
		{
			auto* scene = FindChild<Node2D>("Scene06");

			auto* dialog = scene->FindChild<SpriteEntity>("bubble_right");
			dialog->SetVisiblity(false);

			dialog = scene->FindChild<SpriteEntity>("bubble_left");
			dialog->SetVisiblity(true);

			SetNextDialog(dialog);

			timerMax = 3.0f;
		}
		else
		if (curScene == 18)
		{
			auto* scene = FindChild<Node2D>("Scene06");

			auto* dialog = scene->FindChild<SpriteEntity>("bubble_left");
			dialog->SetVisiblity(false);

			dialog = scene->FindChild<SpriteEntity>("bubble_right");
			dialog->SetVisiblity(true);

			SetNextDialog(dialog);

			timerMax = 3.0f;
		}
		else
		if (curScene == 19)
		{
			auto* scene = FindChild<Node2D>("Scene06");

			auto* dioalog = scene->FindChild<SpriteEntity>("bubble_right");
			dioalog->SetVisiblity(false);

			auto* hint = scene->FindChild<TutorialHint>("TutorialMove");
			hint->Acivate(true);

			auto* puppetTutorial = GetScene()->FindEntity<Puppet>("puppetTutorial", true);
			puppetTutorial->SwitchToCutSceneIntro();

			WorldManager::instance->SetCameraTarget(puppetTutorial, false);

			puppetTo = scene->FindChild<Node2D>("puppetEscape");

			podTo = scene->FindChild<SceneEntity>("TutorialHintActivator");

			timerMax = 1.0f;
		}	
		else
		if (curScene == 20)
		{
			auto* scene = FindChild<Node2D>("Scene07");

			puppet = scene->FindChild<AnimGraph2D>("puppet");
			puppet->SetVisiblity(true);

			puppetFrom = scene->FindChild<Node2D>("puppetFrom");
			puppetTo = scene->FindChild<Node2D>("puppetTo");

			puppet->anim.ActivateLink("idle");
			puppet->GetTransform().position = puppetTo->GetTransform().position;

			auto* puppetTutorial = GetScene()->FindEntity<Puppet>("puppetTutorial", true);
			puppetTutorial->TeleportMaster(puppetFrom->GetTransform().position);

			podTo = scene->FindChild<SceneEntity>("puppetStart");

			WorldManager::instance->CenterCamera();

			timerMax = 1.0f;
		}
		else
		if (curScene == 21)
		{
			auto* scene = FindChild<Node2D>("Scene07");

			auto* dialog = scene->FindChild<SpriteEntity>("bubble_left");
			dialog->SetVisiblity(true);

			SetNextDialog(dialog);

			auto* puppetTutorial = GetScene()->FindEntity<Puppet>("puppetTutorial", true);
			puppetTutorial->SwitchToPreCutSceneIntro(false);
		
			timerMax = 3.0f;
		}
		else
		if (curScene == 22)
		{
			auto* scene = FindChild<Node2D>("Scene07");

			auto* dioalog1 = scene->FindChild<SpriteEntity>("bubble_left");
			dioalog1->SetVisiblity(false);

			puppet->anim.ActivateLink("walk");

			timerMax = 3.0f;
		}
		else
		{
			WorldManager::instance->SetCameraTarget(nullptr, false);

			MainEventsQueue::PushEvent(CmdAllowCameraMove{ true });

			auto* hint = GetScene()->FindEntity<TutorialHint>("TutorialDash", true);
			hint->Acivate(true);

			WorldManager::instance->hackDisallowCursor = false;
			SetVisiblity(false);

			auto* puppetTutorial = GetScene()->FindEntity<Puppet>("puppetTutorial", true);
			puppetTutorial->SwitchToTutorialInto();

			WorldManager::instance->SetMusicTheme(MusicTheme::ThemeNormal, 0.5f);
		}

		timer = 0.0f;
	}

	void CutSceneIntro::Draw(float dt)
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
			if ((curScene == 6 || curScene == 7 || curScene == 8 || curScene == 9 ||
				curScene == 15 || curScene == 16 || curScene == 17 || curScene == 18 || curScene == 21) &&
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
				WorldManager::instance->SetMusicTheme(MusicTheme::ThemeIntro, 0.5f);

				pod->GetTransform().position = pod->GetTransform().position + podDir * podSpeed * dt;

				podRotation += dt * podRotSpeed;
				pod->GetTransform().rotation = Math::Vector3(0.0f, 0.0f, podRotation);
			}			
			else
			if (curScene == 1)
			{
				pod->GetTransform().position = pod->GetTransform().position + podDir * podSpeed * dt;
				
				podRotation += dt * podRotSpeed;
				pod->GetTransform().rotation = Math::Vector3(0.0f, 0.0f, podRotation);
			}
			else
			if (curScene == 2)
			{
				pod->GetTransform().position = pod->GetTransform().position + podDir * podSpeed * dt;

				podRotation += dt * podRotSpeed;
				pod->GetTransform().rotation = Math::Vector3(0.0f, 0.0f, podRotation);

				ship->GetTransform().position = ship->GetTransform().position + shipDir * shipSpeed * dt;
			}
			else
			if (curScene == 3)
			{
				float curShipSpeed = (0.2f + (1.0f - Math::EaseOutCubic(timer / timerMax)) * 0.8f) * shipSpeed;

				pod->GetTransform().position = pod->GetTransform().position + podDir * podSpeed * dt;

				podRotation += dt * podRotSpeed;
				pod->GetTransform().rotation = Math::Vector3(0.0f, 0.0f, podRotation);

				ship->GetTransform().position = ship->GetTransform().position + shipDir * curShipSpeed * dt;
			}
			else
			if (curScene == 4)
			{
				pod->GetTransform().position = pod->GetTransform().position + podDir * podSpeed * dt;

				podRotation += dt * podRotSpeed;
				pod->GetTransform().rotation = Math::Vector3(0.0f, 0.0f, podRotation);

				ship->GetTransform().position = ship->GetTransform().position + shipDir * shipSpeed * 0.2f * dt;
			}
			else
			if (curScene == 5)
			{
				podRotation += dt * podRotSpeed;
				pod->GetTransform().rotation = Math::Vector3(0.0f, 0.0f, podRotation);

				auto from = puppetFrom->GetTransform().position;
				auto to = puppetTo->GetTransform().position;
				auto dir = (to - from);
				auto len = dir.Normalize();

				puppet->GetTransform().position = from + dir * len * (timer / timerMax);
				
				useFootstepSound = true;
			}
			else
			if (curScene == 6 || curScene == 7 || curScene == 8 || curScene == 9 || curScene == 10)
			{
				podRotation += dt * podRotSpeed;
				pod->GetTransform().rotation = Math::Vector3(0.0f, 0.0f, podRotation);

				if (curScene != 10)
				{
					ShowDialogText(dt);
				}
			}
			else
			if (curScene == 11 || curScene == 12 || curScene == 13)
			{
				auto from = podFrom->GetTransform().position;
				auto to = podTo->GetTransform().position;
				auto dir = (to - from);
				auto len = dir.Normalize();

				pod->GetTransform().position = from + dir * len * (timer / timerMax);
				
				if (curScene == 11)
				{
					auto* scene = FindChild<Node2D>("Scene05");
					auto* dialog = scene->FindChild<SpriteEntity>("laser_beam");
					dialog->GetTransform().scale = { (1.0f - (timer / timerMax)) * 5.3f, 1.0f, 1.0f };

					podRotation += dt * podRotSpeed;
					pod->GetTransform().rotation = Math::Vector3(0.0f, 0.0f, podRotation);
				}
			}
			else
			if (curScene == 14)
			{				
				auto from = puppetFrom->GetTransform().position;
				auto to = puppetTo->GetTransform().position;
				auto dir = (to - from);
				auto len = dir.Normalize();				

				puppet->GetTransform().position = from + dir * len * (timer / timerMax);

				useFootstepSound = true;
			}
			else
			if (curScene == 15 || curScene == 16 || curScene == 17 || curScene == 18)
			{
				ShowDialogText(dt);
			}
			else
			if (curScene == 19)
			{
				timer = 0.0f;

				auto& puppetTrans = puppet->GetTransform();

				auto playerPos = WorldManager::instance->GetPlayerPos();
				auto pos = Sprite::ToPixels(puppetTrans.GetGlobal().Pos());

				if ((playerPos - pos).Length() > (puppetWalkWithPlayer ? 120.0f : 90.0f))
				{
					puppet->anim.ActivateLink("idle");
					puppetTrans.scale = Math::Vector3(-1.0f, 1.0f, 1.0f);
					puppetWalkWithPlayer = false;
				}
				else
				{
					puppetWalkWithPlayer = true;
					puppet->anim.ActivateLink("walk");
					puppetTrans.scale = Math::Vector3(1.0f, 1.0f, 1.0f);

					puppetTrans.position = puppetTrans.position + Math::Vector3(170.0f * TILE_SCALE * dt, 0.0f, 0.0f);					

					if (puppetTrans.position.x > puppetTo->GetTransform().position.x)
					{
						puppet->SetVisiblity(false);					
					}
					else
					{
						useFootstepSound = true;
					}
				}

				if (playerPos.x > Sprite::ToPixels(podTo->GetTransform().GetGlobal().Pos().x))
				{
					auto* scene = FindChild<Node2D>("Scene06");

					auto* hint = scene->FindChild<TutorialHint>("TutorialMove");
					hint->Acivate(false);
				}

				if (playerPos.x > Sprite::ToPixels(puppetTo->GetTransform().GetGlobal().Pos().x))
				{
					NextScene();
				}
			}
			else
			if (curScene == 20)
			{
				timer = 0.0f;

				auto playerPos = WorldManager::instance->GetPlayerPos();

				if (playerPos.x > Sprite::ToPixels(podTo->GetTransform().GetGlobal().Pos().x))
				{
					NextScene();
				}
			}
			else
			if (curScene == 21)
			{
				ShowDialogText(dt);
			}
			else
			if (curScene == 22)
			{
				timer = 0.0f;

				auto& puppetTrans = puppet->GetTransform();

				puppetTrans.position = puppetTrans.position - Math::Vector3(170.0f * TILE_SCALE * dt, 0.0f, 0.0f);

				useFootstepSound = true;

				if (puppetTrans.position.x < puppetFrom->GetTransform().position.x)
				{
					puppet->SetVisiblity(false);
					NextScene();
				}
			}

			footstepTimer += dt;

			if (useFootstepSound && footstepTimer >= footstepsInterval)
			{
				footstepTimer -= footstepsInterval;

				Utils::PlaySoundEvent("event:/FatGuy/Footsteps2D");
			}
		}	
	}

	void CutSceneIntro::OnFinishLevel(const EventOnFinishLevel& evt)
	{
		letterTime = 0.0f;
		curDialogLen = 0;
		curDialog = -1;
		curDialogText = "";

		curScene = -1;
		timerMax = 0.0f;
		timer = 0.0f;

		SetVisiblity(true);

		auto* puppetTutorial = GetScene()->FindEntity<Puppet>("puppetTutorial", true);
		puppetTutorial->SwitchToPreCutSceneIntro(false);
	}

	void CutSceneIntro::Release()
	{
		MainEventsQueue::PushEvent(CmdAllowCameraMove{ true });

		MainEventsQueue::Unsubscribe(this);

		RELEASE(footstepsSound)

		ScriptEntity2D::Release();
	}
}