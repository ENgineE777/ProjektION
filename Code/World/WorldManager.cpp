
#include "WorldManager.h"
#include "BloodManager.h"
#include "Root/Root.h"
#include "Support/Perforce.h"
#include "Common/Utils.h"
#include "Characters/Player/Thing.h"
#include "Characters/Enemies/Soldier.h"
#include "Root/Sounds/SoundEvent.h"

#include "geometry/PxGeometryQuery.h"
#include "foundation/PxVec3.h"
#include "Objects/TunnelDoor.h"
#include "Objects/InstantPlayerKiller.h"
#include "Objects/Throwable/ThunderCharge.h"
#include "Objects/LevelRating.h"
#include "Objects/ExplosionBarrel.h"
#include "Characters/Enemies/TurretSimple.h"
#include "Effects/VFXEmitterInstantiable.h"

#include "SceneEntities/2D/DefferedLight.h"

#include "Support/Timer.h"

namespace Orin::Overkill
{
	class ShadowCastTechnique : public RenderTechnique
	{
	public:
		virtual const char* GetVsName() { return "cast_shadow_vs.shd"; };
		virtual const char* GetPsName() { return "cast_shadow_ps.shd"; };

		virtual void ApplyStates()
		{
			GetRoot()->GetRender()->GetDevice()->SetDepthTest(false);
			GetRoot()->GetRender()->GetDevice()->SetDepthWriting(false);
			GetRoot()->GetRender()->GetDevice()->SetCulling(CullMode::CullNone);
		};
	};

	WorldManager* WorldManager::instance = nullptr;

	ENTITYREG(SceneEntity, WorldManager, "Overkill/World", "WorldManager")

	META_DATA_DESC(WorldManager::Accuracy)
		FLOAT_PROP(WorldManager::Accuracy, distance, 0.f, "Accuracy", "distance", "Distance to the player")
		FLOAT_PROP(WorldManager::Accuracy, error, 0.f, "Accuracy", "error", "Max shooting error")
	META_DATA_DESC_END()
	

	META_DATA_DESC(WorldManager::Difficulty)
		STRING_PROP(WorldManager::Difficulty, name, "", "Prop", "name")
		FLOAT_PROP(WorldManager::Difficulty, reactionMul, 1.0f, "Prop", "reactionMul", "reactionMul")
		BOOL_PROP(WorldManager::Difficulty, allowCheckPoints, true, "Prop", "allowCheckPoints", "allowCheckPoints")
		FLOAT_PROP(WorldManager::Difficulty, followLaserSpeedMul, 1.0f, "Prop", "followLaserSpeedMul", "followLaserSpeedMul")
		FLOAT_PROP(WorldManager::Difficulty, mineTimerMul, 1.0f, "Prop", "mineTimerMul", "mineTimerMul")
		FLOAT_PROP(WorldManager::Difficulty, scoreMultiplayer, 1.0f, "Prop", "scoreMultiplayer", "scoreMultiplayer")
		INT_PROP(WorldManager::Difficulty, difficultyScore, 0, "Prop", "difficultyScore", "difficultyScore")
	META_DATA_DESC_END()
	

	META_DATA_DESC(WorldManager)
		BASE_SCENE_ENTITY_PROP(WorldManager)
		ASSET_TEXTURE_PROP(WorldManager, coneViewTexture, "View Cone", "Cone View Texture")
		FLOAT_PROP(WorldManager, comboTimeoutMax, 3.0, "Game", "comboTimeoutMax", "Combo Timeout")
		INT_PROP(WorldManager, kills2AbilityMax, 5, "Game", "kills2AbilityMax", "Kills to grant ability")
		FLOAT_PROP(WorldManager, comboInvincibileTime, 3.0, "Game", "comboInvincibileTime", "comboInvincibileTime")
		FLOAT_PROP(WorldManager, maxBulletSpeed, 10000.f, "Game", "maxBulletSpeed", "Max bullet speed")
		INT_PROP(WorldManager, shootCountToMaxAccuracy, 3, "Game", "shootCountToMaxAccuracy", "Shoots count to max accuracy")
		ARRAY_PROP(WorldManager, accuracyTable, Accuracy, "Game", "accuracyTable")
		ARRAY_PROP(WorldManager, difficulties, Difficulty, "Game", "DifficultyTable")
	META_DATA_DESC_END()

	ORIN_EVENTS(WorldManager)		
		ORIN_EVENT(WorldManager, EventOnThingAbilityAvailabiltyChange, OnThingAbilityAvailabiltyChange)
		ORIN_EVENT(WorldManager, EventOnCheatStateChange, OnCheatStateChange)
	ORIN_EVENTS_END(WorldManager)

	void WorldManager::Init()
	{
		ScriptEntity2D::Init();

		eventsQueue.Clear();

		instance = this;

		point2Draw.reserve(64);

		Tasks(true)->AddTask(5, this, (Object::Delegate)&WorldManager::DrawEffetcsDL);
		Tasks(true)->AddTask(9, this, (Object::Delegate)&WorldManager::DrawEffetcs);
		Tasks(true)->AddTask(10, this, (Object::Delegate)&WorldManager::DrawFade);
	}

	void WorldManager::Release()
	{
		RELEASE(ambienceSound);

		GetRoot()->GetSounds()->ClearAllSounds();

		instance = nullptr;

		eventsQueue.Clear();

		ScriptEntity2D::Release();
	}

	void WorldManager::Play()
	{
		// Clear before ScriptEntity2D::Play()
		// ScriptEntity2D::Play() will add substiptions by children

		GetRoot()->GetPhysScene()->SetCollisionFlag(PhysGroup::LevelObjects, PhysGroup::World, false);
		GetRoot()->GetPhysScene()->SetCollisionFlag(PhysGroup::LevelObjects, PhysGroup::TunnelDoor, false);
		GetRoot()->GetPhysScene()->SetCollisionFlag(PhysGroup::LevelObjects, PhysGroup::Enemy_, false);
		GetRoot()->GetPhysScene()->SetCollisionFlag(PhysGroup::LevelObjects, PhysGroup::LevelObjects, false);
		GetRoot()->GetPhysScene()->SetCollisionFlag(PhysGroup::LevelObjects, PhysGroup::DeathZone, false);

		GetRoot()->GetPhysScene()->SetCollisionFlag(PhysGroup::BodyPart, PhysGroup::Player_, false);
		GetRoot()->GetPhysScene()->SetCollisionFlag(PhysGroup::BodyPart, PhysGroup::Enemy_, false);
		GetRoot()->GetPhysScene()->SetCollisionFlag(PhysGroup::BodyPart, PhysGroup::LevelObjects, false);
		GetRoot()->GetPhysScene()->SetCollisionFlag(PhysGroup::BodyPart, PhysGroup::HitBox, false);
		GetRoot()->GetPhysScene()->SetCollisionFlag(PhysGroup::BodyPart, PhysGroup::BodyPart, false);

		GetRoot()->GetPhysScene()->SetCollisionFlag(PhysGroup::HitBox, PhysGroup::World, false);
		GetRoot()->GetPhysScene()->SetCollisionFlag(PhysGroup::HitBox, PhysGroup::TunnelDoor, false);
		GetRoot()->GetPhysScene()->SetCollisionFlag(PhysGroup::HitBox, PhysGroup::Player_, false);
		GetRoot()->GetPhysScene()->SetCollisionFlag(PhysGroup::HitBox, PhysGroup::Enemy_, false);
		GetRoot()->GetPhysScene()->SetCollisionFlag(PhysGroup::HitBox, PhysGroup::LevelObjects, false);

		GetRoot()->GetSounds()->LoadSoundBank("SoundBanks/Desktop/Master.bank");
		GetRoot()->GetSounds()->LoadSoundBank("SoundBanks/Desktop/Master.strings.bank");

		ambienceSound = GetRoot()->GetSounds()->CreateSoundEvent("event:/Ambience/Ambience");
		if (ambienceSound)
		{
			ambienceSound->Play();
		}

		eventsQueue.Clear();
		ORIN_EVENTS_SUBSCRIBE(WorldManager);

		ScriptEntity2D::Play();

		camera = FindChild<Camera2D>();

		testLevelSelector = GetScene()->FindEntity<TestLevelSelector>();
		testLevelSelector->SetVisiblity(false);

		splashScreens = GetScene()->FindEntity<SplashScreens>();
		splashScreens->SetVisiblity(false);

		resultScreen = GetScene()->FindEntity<ResultScreen>();
		resultScreen->SetVisiblity(false);

		resultDemoScreen = GetScene()->FindEntity<ResultDemoScreen>();
		resultDemoScreen->SetVisiblity(false);

		mainMenu = GetScene()->FindEntity<MainMenu>();
		mainMenu->SetVisiblity(false);

		optionsMenu = GetScene()->FindEntity<OptionsMenu>();
		optionsMenu->SetVisiblity(false);

		pauseMenu = GetScene()->FindEntity<PauseMenu>();
		pauseMenu->SetVisiblity(false);

		selectLevelMenu = GetScene()->FindEntity<SelectLevelMenu>();
		selectLevelMenu->SetVisiblity(false);

		selectDifficultyMenu = GetScene()->FindEntity<SelectDifficultyMenu>();
		selectDifficultyMenu->SetVisiblity(false);

		controlsMenu = GetScene()->FindEntity<ControlsMenu>();
		controlsMenu->SetVisiblity(false);

		musicMenu = GetScene()->FindEntity<MusicMenu>();
		musicMenu->SetVisiblity(false);

		creditsScreen = GetScene()->FindEntity<CreditsScreen>();
		creditsScreen->SetVisiblity(false);

		menuLevel = GetScene()->FindEntity<Node2D>("MenuLevel", true);
		menuLevel->SetVisiblity(false);
		
		camTracks.push_back(GetScene()->FindEntity<GenericMarker2D>("Track1"));
		camTracks.push_back(GetScene()->FindEntity<GenericMarker2D>("Track2"));
		camTracks.push_back(GetScene()->FindEntity<GenericMarker2D>("Track3"));

		thunderCharge = GetScene()->FindEntity<ThunderCharge>();

		musicPlayer = GetScene()->FindEntity<DynamicMusicPlayer>();

		bloodManager = GetScene()->FindEntity<BloodManager>();

		dashArrow = GetScene()->FindEntity<SpriteEntity>("DashArrow", true);
		dashTail = GetScene()->FindEntity<SpriteEntity>("DashTail", true);
		dashZone = GetScene()->FindEntity<SpriteEntity>("DashZone", true);
		dashSlash = GetScene()->FindEntity<SpriteEntity>("DashSlash", true);
		killMarker = GetScene()->FindEntity<SpriteEntity>("KillMarker", true);

		playSign = GetScene()->FindEntity<ImageWidget>("PlaySign", true);
		playSign->SetVisiblity(false);

		lightDust = GetScene()->FindEntity<LightDust>();

		if (dashArrow)
		{
			dashArrow->SetVisiblity(false);
		}

		if (dashZone)
		{
			dashZone->SetVisiblity(false);
		}

		bulletTrace = GetScene()->FindEntity<SpriteEntity>("bulletTrace", true);
		if (bulletTrace)
		{
			bulletTrace->SetVisiblity(false);
		}

		bulletTraceTurret = GetScene()->FindEntity<SpriteEntity>("bulletTraceTurret", true);
		if (bulletTraceTurret)
		{
			bulletTraceTurret->SetVisiblity(false);
		}

		redDot = GetScene()->FindEntity<SpriteEntity>("redDot", true);
		if (redDot)
		{
			redDot->SetVisiblity(false);
		}

		surfaceHit = GetScene()->FindEntity<VFXEmitterInstantiable>("surfaceHit", true);
		playerHit = GetScene()->FindEntity<VFXEmitterInstantiable>("playerHit", true);

		currentCL = Perforce::GetRevision();

		occluderRT = GetRoot()->GetRender()->GetDevice()->CreateTexture(512, 512, TextureFormat::FMT_A8R8G8B8, 1, true, TextureType::Tex2D, _FL_);
		occluderRT->SetAdress(TextureAddress::Clamp);
		occluderRT->SetFilters(TextureFilter::Point, TextureFilter::Point);

		shadowRT = GetRoot()->GetRender()->GetDevice()->CreateTexture(360, MAX_CONE_VIEW, TextureFormat::FMT_R32_FLOAT, 1, true, TextureType::Tex2D, _FL_);

		shadowCastTech = GetRoot()->GetRender()->GetRenderTechnique<ShadowCastTechnique>(_FL_);

		SetFrontendState(FrontendState::SplashScreen);

		gameState.LoadGameState();

		MainEventsQueue::PushEvent(EventOnRestart{});

		aliasUIActive = GetRoot()->GetControls()->GetAlias("UI.ACTIVATE");
		aliasUIPause = GetRoot()->GetControls()->GetAlias("UI.PAUSE");
		aliasUIUp = GetRoot()->GetControls()->GetAlias("UI.UP");
		aliasUIDown = GetRoot()->GetControls()->GetAlias("UI.DOWN");
		aliasUILeft = GetRoot()->GetControls()->GetAlias("UI.LEFT");
		aliasUIRight = GetRoot()->GetControls()->GetAlias("UI.RIGHT");
	}

	void WorldManager::InstatiateTunnelDoors()
	{
		if (!player)
		{
			return;
		}

		AssetPrefabRef prefab = GetRoot()->GetAssets()->GetAssetRef<AssetPrefabRef>(eastl::string("Prefabs/LevelObjects/Hangar/TunnelDoor/TunnelDoor.prefab"));
		AssetPrefabRef prefab2Sided = GetRoot()->GetAssets()->GetAssetRef<AssetPrefabRef>(eastl::string("Prefabs/LevelObjects/Hangar/TunnelDoor/TunnelDoor2Sided.prefab"));

		if (!prefab || !prefab2Sided)
		{
			return;
		}

		auto tileMap = player->GetScene()->FindEntity<TileMap>("Map");

		if (!tileMap)
		{
			return;
		}

		for (auto& tile : tileMap->tiles)
		{
			if (tile.index == -1)
			{
				continue;
			}

			int tunnelDoor = -1;

			const char* meta = tileMap->tileSet->tiles[tile.index].meta.c_str();

			if (StringUtils::IsEqual(meta, "TunnelDoorHorz"))
			{
				tunnelDoor = 0;
			}
			else
			if (StringUtils::IsEqual(meta, "TunnelDoorHorzInv"))
			{
				tunnelDoor = 1;
			}
			else
			if (StringUtils::IsEqual(meta, "TunnelDoorVert"))
			{
				tunnelDoor = 2;
			}
			else
			if (StringUtils::IsEqual(meta, "TunnelDoorVertInv"))
			{
				tunnelDoor = 3;
			}
			else
			if (StringUtils::IsEqual(meta, "TunnelDoorHorz2Sided"))
			{
				tunnelDoor = 4;
			}
			else
			if (StringUtils::IsEqual(meta, "TunnelDoorVert2Sided"))
			{
				tunnelDoor = 5;
			}

			if (tunnelDoor != -1)
			{
				auto* instance = tunnelDoor <= 3 ? prefab->CreateInstance<TunnelDoorEntity>(player->GetScene()) : prefab2Sided->CreateInstance<TunnelDoorEntity>(player->GetScene());

				auto& trans = instance->GetTransform();
				trans.position = Math::Vector3(((float)tile.x + 0.5f) * TILE_SIZE, ((float)tile.y - 0.5f) * TILE_SIZE, 0.0f);

				instance->Play();

				if (instance->anim)
				{
					float angles[] = { 90.0f, 270.0f, 0.0f, 180.0f, 90.0f, 0.0f };
					instance->anim->GetTransform().rotation = Utils::Vector::xyV(instance->anim->GetTransform().rotation, angles[tunnelDoor]);
				}

				player->GetScene()->AddEntity(instance);

				tile.index = -1;
			}
		}
	}

	void WorldManager::Update(float dt)
	{
		UpdateCamTrack(dt);
		
		gamepadConnected = GetRoot()->GetControls()->IsGamepadConnected();

		for (auto& t : timers)
		{
			t.timeout -= dt;

			if (t.timeout <= 0.f)
			{
				t.invokeEvent();
			}
		}

		timers.erase(eastl::remove_if(timers.begin(), timers.end(), [](const Timer& t)
			{
				return t.timeout <= 0.f;
			}), timers.end());

		eventsQueue.Dispatch();

		if (pendingFrontendState != FrontendState::Idle)
		{
			FinishFrontendState();

			frontendState = pendingFrontendState;
			pendingFrontendState = FrontendState::Idle;

			StartFrontendState();
		}

		UpdateFrontendState(dt);

		if (!demoMode)
		{			
			if (player)
			{

				eastl::string currentStr, totalStr;
				Utils::FormatTimer(gameState.GetLevelTime(), currentStr);
				Utils::FormatTimer(gameState.GetTotalTime(), totalStr);

				GetRoot()->GetRender()->DebugPrintText({ 10.f, 70.0f }, ScreenCorner::RightBottom, COLOR_GREEN, "globalTimer: %s", Orin::Timer::GetTimeStamp());
				

				GetRoot()->GetRender()->DebugPrintText({ 10.f, 50.0f }, ScreenCorner::RightBottom, COLOR_GREEN, "Current run: %s / Total time: %s", currentStr.c_str(), totalStr.c_str());
				GetRoot()->GetRender()->DebugPrintText({ 10.f, 30.0f }, ScreenCorner::RightBottom, COLOR_GREEN, "Level: %s", player->GetScene()->GetName());
			}

			GetRoot()->GetRender()->DebugPrintText({ 10.f, 10.0f }, ScreenCorner::RightBottom, COLOR_GREEN, "CL: %i", currentCL);
		}
	}

	void WorldManager::IncreaseComboMeter()
	{
		comboMeter++;
		comboTimeout = comboTimeoutMax;

		kills2Ability++;

		if (kills2Ability == kills2AbilityMax)
		{
			if (IsCheatEnabled(Cheat::PlayerAddInvincibile))
			{
				player->MakeInvincibile(comboInvincibileTime);
			}

			if (IsCheatEnabled(Cheat::PlayerAddThunder))
			{
				thunderCharge->NotifyPlayer(true, true);
			}

			kills2Ability = 0;
		}
	}

	void WorldManager::AddScore(int score)
	{
		gameState.AddScore((int)((float)score * (float)comboMeter * curDifficulty->scoreMultiplayer));
	}

	void WorldManager::OnThingAbilityAvailabiltyChange(const EventOnThingAbilityAvailabiltyChange& evt)
	{
		thingAbilitiesState[evt.ability] = evt.value;
	}

	void WorldManager::OnCheatStateChange(const EventOnCheatStateChange& evt)
	{
		cheatsState[evt.cheat] = evt.value;
	}

	void WorldManager::SetPlayer(Player* setPlayer)
	{
		player = setPlayer;		
		SetCameraTarget(player, true);		
	}

	void WorldManager::SetDifficulty(const char* difficulty)
	{
		for (int i = 0; i < difficulties.size(); i++)
		{
			if (StringUtils::IsEqual(difficulties[i].name.c_str(), difficulty))
			{
				curDifficulty = &difficulties[i];
			}
		}		
	}
		
	WorldManager::Difficulty* WorldManager::GetDifficulty()
	{
		return curDifficulty;
	}

	void WorldManager::CenterCamera()
	{
		if (camera)
		{
			camera->CenterCamera();
		}
	}

	void WorldManager::SetCameraTarget(SceneEntity* target, bool centerCamera)
	{
		if (camera)
		{
			camera->targetRef.SetEntity(target ? target : player);

			if (centerCamera)
			{
				CenterCamera();
			}
		}
	}

	bool WorldManager::IsPlayerCanBeDetectd() const
	{
		if (!player || player->IsKilled() || player->HasPuppet() || WorldManager::instance->IsCheatEnabled(Cheat::PlayerInvisibleToEnemies) ||
			frontendState == FrontendState::ResultDemoScreen)
		{
			return false;
		}

		return true;
	}

	bool WorldManager::IsPlayerVisibleInSector(Math::Vector3 pos, float dist, float angle, float cone, float alwaysVisibleDist) const
	{
		if (!IsPlayerCanBeDetectd())
		{
			return false;
		}

		auto targetPos = player->GetTransform().position;

		auto dir = targetPos - pos;
		auto len = dir.Length2();

		if (dir.Length2() < alwaysVisibleDist * alwaysVisibleDist ||
			Math::IsPointInSector(Utils::Vector::xy(targetPos), Utils::Vector::xy(pos), Utils::ToRadian(angle), dist, Utils::ToRadian(cone), showDebug))
		{
			dir.Normalize();

			if (!Utils::RayCastLine(pos, targetPos, PhysGroup::WorldDoors))
			{
				return true;
			}
		}

		return false;
	}

	bool WorldManager::IsPlayerInSphere(Math::Vector3 pos, float radius) const
	{
		if (!IsPlayerCanBeDetectd())
		{
			return false;
		}

		auto targetPos = player->GetTransform().position;

		return ((targetPos - pos).Length2() < radius * radius) && !Utils::RayCastLine(pos, targetPos, PhysGroup::WorldDoors);
	}

	float WorldManager::GetDistanceToPlayer(Math::Vector3 pos)
	{
		return player ? (player->GetTransform().position - pos).Length() : 1000.0f;
	}

	Utils::Angle WorldManager::GetAngleToPlayer(Math::Vector3 pos)
	{			
		const Math::Vector3 dir = player ? (player->GetTransform().position - pos) : Math::Vector3(1.0f, 0.0f, 0.0f);
		return Utils::Angle::Radian(Math::SafeAtan2(dir.y, dir.x));
	}

	float WorldManager::GetDirectionToPlayer(Math::Vector3 pos)
	{
		auto targetPos = player->GetTransform().position;
		auto dir = targetPos - pos;

		return dir.x < 0.0f ? -1.0f : 1.0f;
	}

	WorldManager::PlayerSnapshot WorldManager::GetPlayerSnapshot()
	{
		if (!player)
		{
			return {};
		}

		return { player->GetTransform().position, player->GetForward(), player->GetUp(), player->GetVelocity() };
	}

	Math::Vector3 WorldManager::CalcShootingDirToPlayer(Math::Vector3 pos, int shootCount, const PlayerSnapshot& playerSnapshot)
	{
		if (!player)
		{
			return {};
		}

		const auto playerPos = playerSnapshot.pos;
		const auto playerFwd = playerSnapshot.forward;
		const auto playerUp = playerSnapshot.up;
		const float distance = (pos - playerPos).Length();

		auto futurePlayerPos = playerPos;

		if (!player->IsKilled())
		{
			if (auto* thing = dynamic_cast<Thing*>(player))
			{
				const auto wishFuturePos = playerPos + (distance / maxBulletSpeed) * playerSnapshot.velocity;
				if (thing->state == Thing::State::Dash)
				{
					futurePlayerPos = playerPos + thing->dashDir * std::min((thing->dashEndPos - playerPos).Length(),
						(wishFuturePos - playerPos).Length());
				}
				else
				{
					futurePlayerPos = wishFuturePos;
				}
			}
		}

		float maxShootingError = accuracyTable.empty() ? 0.f : accuracyTable.back().error;
		for (int i = 0, sz = (int)accuracyTable.size(); i < sz; ++i)
		{
			if (distance <= accuracyTable[i].distance)
			{
				maxShootingError = accuracyTable[i].error;
				break;
			}
		}

		const float accuracy = Utils::Saturate(float(shootCount) / float(shootCountToMaxAccuracy));
		const float error = Utils::Lerp(maxShootingError, 0.f, accuracy);
		const float side = Math::Sign(Utils::GetRandom(-100.f, 100.f));

		const auto dirToPlayer = Utils::Vector::Normalize(futurePlayerPos - pos);
		const auto errorDir = std::abs(dirToPlayer.Dot(playerUp)) < 0.2f ? playerUp : playerFwd;

		const auto shootToPos = futurePlayerPos + errorDir * error * side;
		return Utils::Vector::Normalize(shootToPos - pos);
	}

	Player* WorldManager::GetPlayer()
	{
		return player;
	}

	Math::Vector3 WorldManager::GetPlayerPos()
	{
		return player ? player->GetTransform().position : 0.0f;
	}

	Math::Vector3 WorldManager::GetPlayerUp()
	{
		return player ? player->GetUp() : 0.0f;
	}

	bool WorldManager::HasPuppet()
	{
		return player ? player->HasPuppet() : true;
	}

	bool WorldManager::IsPlayerKilled()
	{
		return player ? player->IsKilled() : true;
	}

	void WorldManager::KillPlayer(Math::Vector3 killDir, DeathSource deathSource)
	{
		player->Kill(killDir, deathSource);
	}

	void WorldManager::AddBulletTrace(Math::Vector3 from, Math::Vector3 to)
	{
		BulletTrace trace;
		trace.from = Utils::Vector::xy(from);
		trace.to = Utils::Vector::xy(to);
		trace.timer = 0.1f;
		trace.redDot = false;

		bulletTraces.emplace_back(trace);
	}

	void WorldManager::AddRedDot(Math::Vector3 from, Math::Vector3 to, float alpha)
	{
		BulletTrace trace;
		trace.from = Utils::Vector::xy(from);
		trace.to = Utils::Vector::xy(to);
		trace.timer = 0.1f;
		trace.redDot = true;
		trace.alpha = alpha;

		bulletTraces.emplace_back(trace);
	}

	void WorldManager::AddBulletProjectile(Math::Vector3 from, Math::Vector3 vel, float maxDistance, bool allowFriendlyFire)
	{
		BulletProjectile p;
		p.pos = Utils::Vector::xy(from);
		p.startPos = p.pos;
		p.vel = Utils::Vector::xy(vel);
		p.leftDistance = maxDistance;
		p.time = 0.f;
		p.fadeTimer = -1.f;
		p.allowFriendlyFire = allowFriendlyFire;
		p.alive = true;

		bulletProjectiles.emplace_back(p);
	}

	void WorldManager::AddViewCone(Math::Vector3 from, Utils::Angle angle, Utils::Angle viewAngle, float viewDist, float agrometr)
	{
		ConeView view;
		view.from = Utils::Vector::xy(from);
		view.angle = angle;
		view.halfViewAngle = viewAngle;
		view.viewDist = viewDist;
		view.agrometr = agrometr;

		coneViews.emplace_back(view);
	}

	void WorldManager::DrawTracer(SpriteEntity* tracer, Math::Vector2 from, Math::Vector2 to, float thickness, Color color, Color emessive)
	{
		if (!tracer)
		{
			return;
		}

		auto& tm = tracer->GetTransform();

		Math::Vector2 slizeSz = Utils::Vector::xy(tm.size);
		slizeSz = { (to - from).Length(), slizeSz.y * thickness};

		Transform trans;
		trans.objectType = ObjectType::Object2D;

		const Utils::Angle angle = Utils::Angle::FromDirection(Utils::Vector::Normalize(to - from));
		trans.size = slizeSz;
		trans.offset = tm.offset;
		trans.scale = tm.scale;
		trans.rotation = Math::Vector3{ 0.f, 0.f, angle.ToDegrees() };
		trans.position = Utils::Vector::xyO(to);

		RenderTechniqueRef tech = Sprite::quadPrgNoZ;

		if (DefferedLight::hackStateEnabled && DefferedLight::gbufferTech)
		{
			tech = DefferedLight::gbufferTech;

			tech->SetTexture(ShaderType::Pixel, "materialMap", tracer->material.Get()->texture);
			tech->SetTexture(ShaderType::Pixel, "normalsMap", tracer->normal.Get()->texture);

			Math::Matrix mat = trans.GetGlobal();
			mat.Pos() = 0.0f;

			tech->SetMatrix(ShaderType::Pixel, "normalTrans", &mat, 1);

			Math::Vector4 params;
			params.x = (float)16 / DefferedLight::lightGroupDivider;
			params.y = 0.0f;
			params.z = emessive.a;

			tech->SetVector(ShaderType::Pixel, "params", &params, 1);

			tech->SetVector(ShaderType::Pixel, "emmisive", (Math::Vector4*)&emessive.r, 1);
		}

		tracer->texture.prg = tech;
		tracer->texture.Draw(&trans, color, 0.0f);
	}

	void WorldManager::RenderShadow1D(int index, Math::Vector2 pos, float size, bool needRestoreState)
	{
		Math::Matrix curView;
		Math::Matrix curProj;
		auto camPos = Sprite::GetCamPos();
		auto zoom = Sprite::GetCamZoom();

		if (needRestoreState)
		{
			GetRoot()->GetRender()->GetTransform(TransformStage::View, curView);
			GetRoot()->GetRender()->GetTransform(TransformStage::Projection, curProj);
		}

		Sprite::SetCamPos(pos);
		pos *= Sprite::ToUnits(1.0f);

		const Math::Vector3 upVector{ 0.0f, 1.0f, 0.f };

		float dist = Sprite::ToUnits(size) / (tanf(22.5f * Math::Radian));

		Math::Matrix view;
		view.BuildView(Math::Vector3(pos.x, pos.y, -dist), Math::Vector3(pos.x, pos.y, -dist + 1.0f), upVector);

		GetRoot()->GetRender()->SetTransform(TransformStage::View, view);

		Math::Matrix proj;
		proj.BuildProjection(45.0f * Math::Radian, 1.0f, 1.0f, 1000.0f);
		GetRoot()->GetRender()->SetTransform(TransformStage::Projection, proj);

		GetRoot()->GetRender()->GetDevice()->SetRenderTarget(0, occluderRT);
		GetRoot()->GetRender()->GetDevice()->Clear(true, COLOR_WHITE, false, 1.0f);
		GetRoot()->GetRender()->ExecutePool(500, 0.0f);

		Math::Vector4 params;

		params.x = (float)index;
		params.y = 1.0f / (float)shadowRT->GetHeight();

		shadowCastTech->SetVector(ShaderType::Vertex, "shadowParams", &params, 1);

		params.x = 1.0f / (float)occluderRT->GetWidth();
		params.y = 1.0f / (float)occluderRT->GetHeight();

		shadowCastTech->SetVector(ShaderType::Pixel, "params", &params, 1);

		GetRoot()->GetRender()->GetDevice()->SetRenderTarget(0, shadowRT);
		GetRoot()->GetRender()->GetDevice()->SetDepth(nullptr);

		Sprite::Draw(occluderRT, COLOR_WHITE, Math::Matrix(), 0.0f, 100.0f, 0.0f, 1.0f, shadowCastTech);

		//GetRoot()->GetRender()->DebugSprite(occluderRT, 10.0f, 200.0f);

		if (needRestoreState)
		{
			GetRoot()->GetRender()->SetTransform(TransformStage::View, curView);
			GetRoot()->GetRender()->SetTransform(TransformStage::Projection, curProj);

			Sprite::SetCamPos(camPos);
			Sprite::SetCamZoom(zoom);

			GetRoot()->GetRender()->GetDevice()->RestoreRenderTarget();
		}
	}

	void WorldManager::RenderSpriteWithShadow1D(int index, Color color, Transform trans, float size, AssetTextureRef texture)
	{
		texture.prg = Sprite::quadPrgShdNoZ;

		Math::Vector4 params;
		params.x = trans.size.x * 0.5f * trans.scale.x;
		params.y = trans.size.y * 0.5f * trans.scale.x;
		params.z = -trans.rotation.z * Math::Radian;

		texture.prg->SetVector(ShaderType::Vertex, "rot", &params, 1);

		params.x = (float)index;
		params.y = size;
		texture.prg->SetVector(ShaderType::Pixel, "ps_desc", &params, 1);

		texture.prg->SetTexture(ShaderType::Pixel, "shadowMap", shadowRT);

		texture.Draw(&trans, color, 0.0f);
	}

	void WorldManager::AddThrowable(Math::Vector3 from, Math::Vector3 dir, ThrowableDesc* throwableDesc)
	{
		Throwable throwable;

		throwable.pos = from;
		throwable.dir = dir;
		throwable.desc = throwableDesc;

		throwableDesc->anim->SetVisiblity(true);

		throwables.push_back(throwable);
	}

	void WorldManager::DrawEffetcsDL(float dt)
	{
		for (int i = 0; i < bulletTraces.size(); i++)
		{
			auto& trace = bulletTraces[i];

			trace.timer -= dt;

			if (trace.redDot || trace.timer < 0.0f)
			{
				DrawTracer(redDot, trace.from, trace.to, 0.5f, COLOR_RED_A(0.025f * trace.alpha), COLOR_RED_A(1.25f * trace.alpha));

				bulletTraces.erase(bulletTraces.begin() + i);
				i--;
			}
			else
			{
				const float alpha = trace.timer / 0.1f * 0.65f + 0.1f;
				DrawTracer(redDot, trace.from, trace.to, 1.15f, COLOR_YELLOW_A(alpha), COLOR_YELLOW_A(alpha));

				trace.timer = -0.1f;
			}
		}

		for (int i = (int)bulletProjectiles.size() - 1; i >= 0; --i)
		{
			auto& proj = bulletProjectiles[i];

			if (!proj.alive)
			{
				bulletProjectiles.erase(bulletProjectiles.begin() + i);
				continue;
			}

			const float fadeOutTime = 0.1f;
			const float wishAlpha = proj.fadeTimer >= 0.f ? Utils::Lerp(0.f, 1.f, proj.fadeTimer / fadeOutTime) : 1.f;

			if (proj.fadeTimer > 0.f)
			{
				proj.fadeTimer -= dt;

				if (proj.fadeTimer < 0.f)
				{
					proj.alive = false;
				}
			}
			else
			{
				auto nextPos = proj.pos + proj.vel * dt;

				proj.time += dt;
				proj.leftDistance -= proj.vel.Length() * dt;
				proj.alive = proj.leftDistance > 0.f;

				if (auto castRes = Utils::RayCastLine(proj.pos, nextPos, proj.allowFriendlyFire ? (PhysGroup::WorldDoorsHitBox | PhysGroup::Enemy_) : PhysGroup::WorldDoorsHitBox))
				{
					proj.fadeTimer = fadeOutTime;
					nextPos = Utils::Vector::xy(castRes->hitPos);

					auto hitVFX = surfaceHit;

					if (castRes->userdata)
					{
						if (auto* player = dynamic_cast<Player*>(castRes->userdata->object->GetParent()))
						{
							hitVFX = playerHit;

							if (player->CanReflectProjectile())
							{
								proj.fadeTimer = -1.0f;
								proj.vel = -proj.vel;
							}
							else
							{
								KillPlayer(Utils::Vector::Normalize(proj.vel), DeathSource::Bullet);
							}
						}
						else
						if (auto* soldier = dynamic_cast<Soldier*>(castRes->userdata->object->GetParent()))
						{
							hitVFX = playerHit;
							soldier->Kill(Utils::Vector::Normalize(proj.vel), DeathSource::Bullet);
						}
					}

					if (hitVFX)
					{
						Math::Matrix m;
						m.RotateZ(-Utils::Angle::FromDirection(castRes->hitNormal).ToRadian());
						m.Pos() = Sprite::ToUnits(castRes->hitPos + 1.f * castRes->hitNormal);

						hitVFX->AddInstance(m);

						Utils::PlaySoundEvent("event:/Bullets impact/Wall impact", &castRes->hitPos);
					}
				}

				proj.pos = nextPos;
			}

			const float tracerLen = std::min(82.f, (proj.pos - proj.startPos).Length());
			const float alpha = tracerLen <= 82.f ? Utils::Lerp(0.f, 1.f, Utils::Saturate(tracerLen / 82.f)) : wishAlpha;
			DrawTracer(proj.allowFriendlyFire ? bulletTraceTurret : bulletTrace, proj.pos - Utils::Vector::Normalize(proj.vel) * tracerLen, proj.pos, 1.0f, COLOR_WHITE_A(alpha), COLOR_YELLOW_A(alpha * 1.25f));
		}
	}

	void WorldManager::DrawEffetcs(float dt)
	{
		eastl::vector<ConeView> coneViews2Render;

		for (int i = 0; i < coneViews.size(); i++)
		{
			auto& view = coneViews[i];

			Math::Vector2 halfSize = view.viewDist;
			Math::Vector2 left = view.from + (view.angle + view.halfViewAngle).ToDirection() * view.viewDist;
			Math::Vector2 right = view.from + (view.angle - view.halfViewAngle).ToDirection() * view.viewDist;

			if (!Sprite::IsTrinagleVisibile(view.from, left, right))
			{
				continue;
			}

			coneViews2Render.push_back(view);

			if (coneViews2Render.size() == MAX_CONE_VIEW - 1)
			{
				break;
			}
		}

		Math::Matrix curView;
		Math::Matrix curProj;
		auto camPos = Sprite::GetCamPos();
		auto zoom = Sprite::GetCamZoom();

		GetRoot()->GetRender()->GetTransform(TransformStage::View, curView);
		GetRoot()->GetRender()->GetTransform(TransformStage::Projection, curProj);

		for (int i = 0; i < coneViews2Render.size(); i++)
		{
			auto& viewCone = coneViews2Render[i];

			RenderShadow1D(i, viewCone.from, viewCone.viewDist);
		}

		GetRoot()->GetRender()->SetTransform(TransformStage::View, curView);
		GetRoot()->GetRender()->SetTransform(TransformStage::Projection, curProj);

		Sprite::SetCamPos(camPos);
		Sprite::SetCamZoom(zoom);

		GetRoot()->GetRender()->GetDevice()->RestoreRenderTarget();

		///GetRoot()->GetRender()->DebugSprite(occluderRT, 10.0f, 10.0f);
		//GetRoot()->GetRender()->DebugSprite(shadowRT, 120.0f, 10.0f);

		for (int i = 0; i < coneViews2Render.size(); i++)
		{
			auto& viewCone = coneViews2Render[i];

			Math::Vector2 halfSize = viewCone.viewDist;
			Math::Vector2 left = viewCone.from + (viewCone.angle + viewCone.halfViewAngle).ToDirection() * viewCone.viewDist;
			Math::Vector2 right = viewCone.from + (viewCone.angle - viewCone.halfViewAngle).ToDirection() * viewCone.viewDist;

			Color colorNormal = Color(1.0f, 0.29f, 0.29f, 0.5f);
			Color colorAttack = Color(1.0f, 1.0f, 1.0f, 0.15f);

			Color color;
			color.Lerp(colorNormal, colorAttack, viewCone.agrometr);

			Transform trans;
			trans.objectType = ObjectType::Object2D;

			trans.size.x = viewCone.viewDist;
			trans.size.y = viewCone.viewDist * tanf(viewCone.halfViewAngle.ToRadian());

			trans.size *= 2.0f;

			trans.rotation = Math::Vector3{ 0.f, 0.f, viewCone.angle.ToDegrees() };
			trans.position = Utils::Vector::xyO(viewCone.from);
			
			RenderSpriteWithShadow1D(i, color, trans, viewCone.viewDist, coneViewTexture);
		}

		coneViews.clear();

		//Sprite::DebugTextBottom(1, "coneViews: %i", coneViews2Render.size());		

		for (int i = 0; i < throwables.size(); i++)
		{
			auto& throwable = throwables[i];
			auto* desc = throwable.desc;

			if (throwable.hitTime > 0.f)
			{
				throwable.hitTime -= dt;

				if (throwable.hitTime <= 0.f)
				{
					throwable.hitTime = -1.f;

					desc->anim->SetVisiblity(false);

					throwables.erase(throwables.begin() + i);
					i--;

					if (desc->trail)
					{
						desc->trail->SetVisiblity(false);
					}

					continue;
				}

				if (desc->trail)
				{
					throwable.trailTime -= dt;

					float k = throwable.trailTime / 0.15f;
					auto& tm = desc->trail->GetTransform();
				
					tm.scale = Math::Vector3(k, 1.0f, 1.0f);

					desc->trail->color = COLOR_WHITE_A(k);
				}

				return;
			}

			float delta = desc->speed * dt;

			auto castRes = Utils::RayCast(throwable.pos, throwable.dir, delta, PhysGroup::WorldDoorsEnemies | PhysGroup::MovementBlock);

			SceneEntity* hittedEntity = nullptr;

			if (castRes && castRes->userdata)
			{
				if (castRes->userdata->controller)
				{
					hittedEntity = dynamic_cast<SceneEntity*>(castRes->userdata->object->GetParent());					
				}
				else
				if (castRes->userdata->object)
				{
					hittedEntity = dynamic_cast<ExplosionBarrel*>(castRes->userdata->object->GetParent());
				}

				if (hittedEntity && !hittedEntity->IsVisible())
				{
					castRes = eastl::nullopt;
				}
			}			

			if (!castRes)
			{
				castRes = RayCastKillTriggers(throwable.pos, throwable.dir, delta);

				if (castRes && castRes->userdata)
				{
					hittedEntity = dynamic_cast<TurretSimple*>(castRes->userdata->object->GetParent());				

					if (!hittedEntity)
					{
						hittedEntity = dynamic_cast<SceneEntity*>(castRes->userdata->object);
					}
				}
			}

			if (castRes)
			{				
				if (desc->OnThrowableHit(throwable.pos, throwable.dir, hittedEntity))
				{
					delta = castRes->hitLength;

					desc->anim->anim.ActivateLink("Hit");
					throwable.hitTime = 0.25f;

				}
			}
			else
			{
				eastl::vector<PhysScene::BodyUserData*> hitBodies;
				GetRoot()->GetPhysScene()->OverlapWithSphere(Sprite::ToUnits(throwable.pos), Sprite::ToUnits(desc->hitRadius), PhysGroup::Enemy_, hitBodies);

				for (auto* body : hitBodies)
				{
					if (auto* enemy = dynamic_cast<Enemy*>(body->object->GetParent()))
					{
						if (enemy->IsVisible())
						{
							desc->anim->anim.ActivateLink("Hit");
							throwable.hitTime = 0.25f;

							desc->OnThrowableHit(throwable.pos, throwable.dir, enemy);
							break;
						}
					}
				}
			}

			throwable.pos += throwable.dir * delta;
			throwable.distance += delta;
			throwable.rotation += dt * 1200.0f;

			if (throwable.distance >= desc->maxDistance && !castRes)
			{
				desc->anim->anim.ActivateLink("Hit");
				throwable.hitTime = 0.25f;

				desc->OnThrowableHit(throwable.pos, throwable.dir, nullptr);
			}

			auto& tm = desc->anim->GetTransform();

			Math::Matrix mat;
			mat.Pos() = Sprite::ToUnits(throwable.pos);
			tm.SetGlobal(mat);
			
			tm.rotation = throwable.rotation;

			if (desc->trail)
			{
				desc->trail->SetVisiblity(true);

				auto& tm = desc->trail->GetTransform();

				const auto dashAngle = Utils::AdjustAngleByDirection(Utils::Angle::FromDirection(throwable.dir), 1.0f);

				Math::Matrix mat;
				mat.Pos() = Sprite::ToUnits(throwable.pos);
				tm.SetGlobal(mat);

				tm.rotation = Math::Vector3(0.0f, 0.0f, dashAngle.ToDegrees());

				throwable.trailTime = fmin(throwable.trailTime + dt, 0.15f);

				float k = throwable.trailTime / 0.15f;

				tm.scale = Math::Vector3(k, 1.0f, 1.0f);

				desc->trail->color = COLOR_WHITE_A(k);
			}
		}
	}

	void WorldManager::Detonate(Math::Vector3 pos, float explodeRadius, SceneEntity* skipTarget)
	{
		eastl::vector<PhysScene::BodyUserData*> hitBodies;
		GetRoot()->GetPhysScene()->OverlapWithSphere(Sprite::ToUnits(pos), Sprite::ToUnits(explodeRadius), PhysGroup::Player_ | PhysGroup::Enemy_ | PhysGroup::MovementBlock, hitBodies);

		for (auto* body : hitBodies)
		{
			auto* testBody = body->object->GetParent();

			if (testBody == skipTarget || !testBody)
			{
				continue;
			}

			auto tm = testBody->GetTransform().GetGlobal();
			auto targetPos = Sprite::ToPixels(tm.Pos());

			auto dir = targetPos - pos;
			dir.Normalize();

			if (auto * player = dynamic_cast<Player*>(testBody))
			{
				if (!Utils::RayCastLine(pos, targetPos, PhysGroup::WorldDoors | PhysGroup::MovementBlock))
				{
					KillPlayer(dir, DeathSource::Unknown);
				}
			}
			else
			if (auto* enemy = dynamic_cast<Enemy*>(testBody))
			{
				auto offset = 10.0f * tm.Vy();

				if (!enemy->IsKilled() && !Utils::RayCastLine(pos, targetPos + offset, PhysGroup::WorldDoors | PhysGroup::MovementBlock))
				{
					enemy->Kill(dir, DeathSource::Unknown);
				}
			}
			else
			if (auto* barrel = dynamic_cast<ExplosionBarrel*>(testBody))
			{
				if (!Utils::RayCastLine(pos, targetPos, PhysGroup::WorldDoors | PhysGroup::MovementBlock))
				{
					barrel->Explode();
				}
			}			
		}		
	}

	void WorldManager::StartFade(bool setFadeIn, float setTime, eastl::function<void()> callback)
	{
		fadeIn = setFadeIn;
		fadeCallback = callback;

		if (setTime < 0.01f)
		{
			fadeSpeed = 1.0f;
			fadeAmount = fadeIn ? 1.0f : 0.0f;
		}
		else
		{
			fadeSpeed = 1.0f / setTime;
		}
	}

	void WorldManager::DrawFade(float dt)
	{
		if (!GetScene()->IsPlaying())
		{
			return;
		}

		if (fadeIn && !Math::IsEqual(fadeAmount, 1.0f))
		{
			fadeAmount += dt * fadeSpeed;

			if (fadeAmount > 0.9999f)
			{
				fadeAmount = 1.0f;

				if (fadeCallback)
				{
					auto callback = fadeCallback;
					fadeCallback = eastl::function<void()>();
					callback();
				}
			}
		}
		else
			if (!fadeIn && !Math::IsEqual(fadeAmount, 0.0f))
			{
				fadeAmount -= dt * fadeSpeed;

				if (fadeAmount < 0.001f)
				{
					fadeAmount = 0.0f;

					if (fadeCallback)
					{
						auto callback = fadeCallback;
						fadeCallback = eastl::function<void()>();
						callback();
					}
				}
			}

		if (fadeAmount > 0.01f)
		{
			auto sz = Sprite::GetHalfScreenSize();

			//Sprite::Draw(GetRoot()->GetRender()->GetWhiteTexture(), COLOR_BLACK_A(fadeAmount), Math::Matrix(), Math::Vector2(sz.x, sz.y) + Sprite::GetCamPos(), sz * 2.0f, 0.0f, 1.0f, Sprite::quadPrgNoZ);
			GetRoot()->GetRender()->DebugSprite(nullptr, 0.0f, 2000.f, COLOR_BLACK_A(fadeAmount));

			Transform trans;
			trans.objectType = ObjectType::Object2D;
			trans.size = coneViewTexture.GetSize();
			trans.position = Math::Vector3(Sprite::GetCamPos().x, Sprite::GetCamPos().y, 0.0f);
			//coneViewTexture.Draw(&trans, COLOR_WHITE, dt);
		}
	}

	void WorldManager::SetNextLevel(const eastl::string& levelName, bool resetCheckpoint)
	{
		nextLevel = levelName;
		gameState.MarkStartOfLevel(nextLevel.c_str(), resetCheckpoint);
	}

	void WorldManager::SetFrontendStateViaFade(FrontendState setFrontendState)
	{
		if (frontendState == FrontendState::MainMenu || frontendState == FrontendState::Gameplay)
		{
			SetMusicTheme(MusicTheme::None, 0.5f);
		}

		StartFade(true, 0.5f, [setFrontendState]() { WorldManager::instance->SetFrontendState(setFrontendState); });
	}

	void WorldManager::SetFrontendState(FrontendState frontendState)
	{		
		pendingFrontendState = frontendState;
	}

	void WorldManager::PushFrontendState(FrontendState setFrontendState)
	{
		frontendStakedStates.push_back(frontendState);
		pendingFrontendState = setFrontendState;
	}

	void WorldManager::PopFrontendState()
	{
		int count = (int)frontendStakedStates.size();

		if (count > 0)
		{
			pendingFrontendState = frontendStakedStates[count - 1];
			frontendStakedStates.pop_back();
		}
	}

	void WorldManager::SetMusicTheme(MusicTheme theme, float fadeTime)
	{
		if (musicPlayer)
		{
			musicPlayer->ChangeTheme(theme, fadeTime);
		}
	}

	void WorldManager::ChangeAlarmCount(bool inc)
	{
		alarmCount += inc ? 1 : -1;
	}

	void WorldManager::Clear(bool fullClear)
	{
		if (fullClear)
		{
			bloodManager->Clear();
		}

		bulletProjectiles.clear();
		bulletTraces.clear();

		if (playerHit)
		{
			playerHit->ClearInstances();
		}

		if (surfaceHit)
		{
			surfaceHit->ClearInstances();
		}
	}

	void WorldManager::StartFrontendState()
	{
		switch (frontendState)
		{
		case FrontendState::SplashScreen:
			{
				splashScreens->SetVisiblity(true);
				break;
			}
		case FrontendState::MainMenu:
			{
				StartFade(false, 0.5f);

				SetMusicTheme(MusicTheme::ThemeMain, 0.5f);

				mainMenu->SetVisiblity(true);
				menuLevel->SetVisiblity(true);
				playSign->SetVisiblity(true);

				if (musicPlayer)
				{
					musicPlayer->MuteTheme(false);
				}

				if (player)
				{
					Clear(true /* fullClear */);

					auto* name = player->GetScene()->GetName();
					GetRoot()->GetSceneManager()->UnloadScene(name);
					player = nullptr;
				}

				break;
			}
		case FrontendState::OptionsMenu:
			{
				if (player)
				{
					AnimGraph2D::pause = true;
				}

				StartFade(false, 0.5f);
				optionsMenu->SetVisiblity(true);
				break;
			}
		case FrontendState::MusicMenu:
			{
				if (player)
				{
					AnimGraph2D::pause = true;
				}

				StartFade(false, 0.5f);
				musicMenu->SetVisiblity(true);
				break;
			}
		case FrontendState::ControlsMenu:
			{
				if (player)
				{
					AnimGraph2D::pause = true;
				}

				StartFade(false, 0.5f);
				controlsMenu->SetVisiblity(true);
				break;
			}
		case FrontendState::PauseMenu:
			{
				AnimGraph2D::pause = true;

				StartFade(false, 0.5f);
				pauseMenu->SetVisiblity(true);
				break;
			}
		case FrontendState::SelectLevelMenu:
			{
				StartFade(false, 0.5f);
				selectLevelMenu->SetVisiblity(true);
				break;
			}
		case FrontendState::SelectDifficultyMenu:
		{
			StartFade(false, 0.5f);
			selectDifficultyMenu->SetVisiblity(true);
			break;
		}			
		case FrontendState::CreditsScreen:
			{
				StartFade(false, 0.5f);
				creditsScreen->SetVisiblity(true);
				break;
			}
		case FrontendState::ResultDemoScreen:
			{
				StartFade(false, 0.5f);
				resultDemoScreen->SetVisiblity(true);
				break;
			}			
		case FrontendState::DebugMenu:
			{
				StartFade(false, 0.5f);
				testLevelSelector->SetVisiblity(true);

				break;
			}
		case FrontendState::LoadNextLevel:
			{				
				menuLevel->SetVisiblity(false);
				playSign->SetVisiblity(false);

				if (player)
				{
					gameState.ResetLevelTime();

					Clear(true /* fullClear */);

					auto* name = player->GetScene()->GetName();
					GetRoot()->GetSceneManager()->UnloadScene(name);
				}

				GetRoot()->GetSceneManager()->LoadScene(nextLevel.c_str());
				nextLevel = "";

				StartFade(false, 0.75f);

				SetFrontendState(FrontendState::GameplayRestart);

				break;
			}
		case FrontendState::GameplayRestart:
			{
				alarmCount = 0;
				inNormalTheme = true;
				time2Back2Normal = 0.0f;

				SetMusicTheme(MusicTheme::ThemeNormal, 0.5f);

				MainEventsQueue::InvokeEvent(EventOnRestart{});
				SetFrontendState(FrontendState::Gameplay);

				Clear(false /* fullClear */);

				break;
			}
		case FrontendState::Gameplay:
			{  
				if (musicPlayer)
				{
					musicPlayer->MuteTheme(false);
				}

				if (player)
				{
					auto* defferdLight = player->GetScene()->FindEntity<DefferedLight>();

					if (defferdLight)
					{
						defferdLight->useFilter = false;
					}
				}
				break;
			}
		case FrontendState::GameplayResultWin:
			{				
				gameState.AddScore(curDifficulty->difficultyScore);

				int rating = 0;

				if (auto* levelRating = player->GetScene()->FindEntity<LevelRating>())				
				{
					gameState.AddScore(levelRating->GetTimeScore(gameState.GetLevelTime()));
					rating = levelRating->GetRating(gameState.GetLevelScore());
				}				
				
				gameState.SaveLevelStat();

				resultScreen->SetState(ResultScreen::State::Won, rating);

				break;
			}
		case FrontendState::GameplayResultLost:
			{
				resultScreen->SetState(ResultScreen::State::Lose, 0);
				break;
			}
		}
	}

	void WorldManager::FinishFrontendState()
	{
		switch (frontendState)
		{
		case FrontendState::SplashScreen:
			{
				splashScreens->SetVisiblity(false);
				break;
			}
		case FrontendState::MainMenu:
			{
				mainMenu->SetVisiblity(false);				

				if (player)
				{
					Clear(true /* fullClear */);

					auto* name = player->GetScene()->GetName();
					GetRoot()->GetSceneManager()->UnloadScene(name);
				}
				break;
			}
		case FrontendState::OptionsMenu:
			{
				AnimGraph2D::pause = false;
				optionsMenu->SetVisiblity(false);
				break;
			}
		case FrontendState::PauseMenu:
			{
				AnimGraph2D::pause = false;
				pauseMenu->SetVisiblity(false);
				break;
			}
		case FrontendState::MusicMenu:
			{			
				AnimGraph2D::pause = false;
				musicMenu->SetVisiblity(false);
				break;
			}
		case FrontendState::ControlsMenu:
			{				
				AnimGraph2D::pause = false;
				controlsMenu->SetVisiblity(false);
				break;
			}
		case FrontendState::SelectLevelMenu:
			{
				selectLevelMenu->SetVisiblity(false);
				break;
			}
		case FrontendState::SelectDifficultyMenu:
		{
			selectDifficultyMenu->SetVisiblity(false);
			break;
		}			
		case FrontendState::CreditsScreen:
			{			
				creditsScreen->SetVisiblity(false);				
				break;
			}
		case FrontendState::ResultDemoScreen:
			{			
				resultDemoScreen->SetVisiblity(false);
				break;
			}
		case FrontendState::DebugMenu:
			{
				testLevelSelector->SetVisiblity(false);
				
				break;
			}
		case FrontendState::LoadNextLevel:
			{
				InstatiateTunnelDoors();
				StartFade(false, 0.5f);				
				break;
			}
		case FrontendState::GameplayResultLost:
		case FrontendState::GameplayResultWin:
			{
				resultScreen->SetState(ResultScreen::State::None, 0);
				break;
			}
		}
	}

	void WorldManager::StartNextTrack()
	{
		curCamTrack++;

		if (curCamTrack >= camTracks.size())
		{
			curCamTrack = 0;
		}

		curCamDist = 0.0f;

		auto* track = camTracks[curCamTrack];

		camPos = track->instances[0].GetPosition();
		camDir = track->instances[1].GetPosition() - track->instances[0].GetPosition();
		camDist = camDir.Normalize();
	}		

	void WorldManager::UpdateCamTrack(float dt)
	{		
		if (!menuLevel->IsVisible())
		{
			return;
		}

		float dist = dt * 35.0f;

		curCamDist += dist;
		camPos += camDir * dist;

		if (curCamDist > camDist)
		{
			StartNextTrack();
		}

		camera->GetTransform().position = camPos;
	}

	void WorldManager::UpdateFrontendState(float dt)
	{
		switch (frontendState)
		{
		case FrontendState::Gameplay:
			{				
				gameState.Update(dt);

				if (alarmCount > 0)
				{
					time2Back2Normal = 5.5f;

					if (inNormalTheme)
					{
						inNormalTheme = false;
						SetMusicTheme(MusicTheme::ThemeBattle, 1.0f);
					}
				}
				else
				if (alarmCount == 0 && !inNormalTheme)
				{
					time2Back2Normal -= dt;

					if (time2Back2Normal < 0.0f)
					{
						inNormalTheme = true;
						SetMusicTheme(MusicTheme::ThemeNormal, 1.0f);
					}
				}

				if (GetRoot()->GetControls()->GetAliasState(aliasUIPause, AliasAction::JustPressed))
				{
					if (musicPlayer)
					{
						musicPlayer->MuteTheme(true);
					}

					if (player)
					{
						auto* defferdLight = player->GetScene()->FindEntity<DefferedLight>();

						if (defferdLight)
						{
							defferdLight->useFilter = true;
						}
					}

					SetFrontendState(FrontendState::PauseMenu);
				}

				if (comboTimeout > 0.0f)
				{
					comboTimeout -= dt;

					if (comboTimeout <= 0.0f)
					{
						comboMeter = 0;
						comboTimeout = -1.0f;
						kills2Ability = 0;
					}
				}
				
				//GetRoot()->GetRender()->DebugPrintText({ 10, 10 }, ScreenCorner::LeftTop, COLOR_GREEN, "Score: %i Combo: %i X (%4.3f)", gameState.GetLevelScore(), comboMeter, comboTimeout);

				break;
			}
		}
	}

	Utils::MaybeCastRes WorldManager::RayCastKillTriggers(Math::Vector3 from, Math::Vector3 dir, float length)
	{
		return Utils::RayCast(from, dir, length, PhysGroup::DeathZone);
	}
}