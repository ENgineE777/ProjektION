
#pragma once

#include "SceneEntities/2D/ScriptEntity2D.h"
#include "SceneEntities/2D/Camera2D.h"
#include "Support/MetaData.h"
#include "Support/Sprite.h"
#include "SceneEntities/2D/Node2D.h"
#include "SceneEntities/UI/ContainerWidget.h"
#include "root/Assets/AssetTexture.h"
#include "Characters/Player/Player.h"
#include "Characters/Enemies/Enemy.h"
#include "SceneEntities/Common/MusicPlayer.h"
#include "SceneEntities/2D/GenericMarker2D.h"
#include "Common/EventQueue.h"
#include "Common/Events.h"
#include "Common/Utils.h"
#include "Common/Constants.h"
#include "UI/CreditsScreen.h"
#include "UI/MainMenu.h"
#include "UI/OptionsMenu.h"
#include "UI/ResultScreen.h"
#include "UI/ResultDemoScreen.h"
#include "UI/PauseMenu.h"
#include "UI/ControlsMenu.h"
#include "UI/MusicMenu.h"
#include "UI/SelectDifficultyMenu.h"
#include "UI/SelectLevelMenu.h"
#include "UI/SplashScreens.h"
#include "UI/TestLevelSelector.h"
#include "Objects/InstantPlayerKiller.h"
#include "Objects/Throwable/Throwable.h"
#include "DynamicMusicPlayer.h"
#include "GameState.h"
#include "Effects/LightDust.h"

namespace Orin::Overkill
{
	class BloodManager;
	class VFXEmitterInstantiable;

	enum class FrontendState
	{
		Idle,
		SplashScreen,
		MainMenu,
		OptionsMenu,
		MusicMenu,
		ControlsMenu,
		PauseMenu,
		SelectDifficultyMenu,
		SelectLevelMenu,		
		DebugMenu,
		CreditsScreen,
		LoadNextLevel,
		GameplayRestart,
		Gameplay,
		GameplayResultWin,
		GameplayResultLost,
		ResultDemoScreen
	};

	class WorldManager : public ScriptEntity2D
	{
		friend class Player;

		Camera2D* camera = nullptr;
		Player* player = nullptr;
		ResultScreen* resultScreen = nullptr;
		ResultDemoScreen* resultDemoScreen = nullptr;
		TestLevelSelector* testLevelSelector = nullptr;
		SplashScreens* splashScreens = nullptr;
		BloodManager *bloodManager = nullptr;
		MainMenu* mainMenu = nullptr;
		OptionsMenu* optionsMenu = nullptr;
		PauseMenu* pauseMenu = nullptr;
		ControlsMenu* controlsMenu = nullptr;
		MusicMenu* musicMenu = nullptr;
		SelectLevelMenu* selectLevelMenu = nullptr;		
		CreditsScreen* creditsScreen = nullptr;
		ImageWidget* playSign = nullptr;

		Node2D* menuLevel = nullptr;

		int curCamTrack = -1;
		float curCamDist = 0.0f;
		float camDist = 0.0f;
		Math::Vector3 camPos = 0.0f;
		Math::Vector3 camDir = 0.0f;
		eastl::vector<GenericMarker2D*> camTracks;

		eastl::vector<Enemy*> enemies;
		DynamicMusicPlayer* musicPlayer;		

		bool fadeIn = true;
		float fadeSpeed = 1.0f;
		float fadeAmount = 1.0f;
		eastl::function<void()> fadeCallback;

		constexpr static int MAX_CONE_VIEW = 32;

		TextureRef occluderRT;
		TextureRef shadowRT;

		RenderTechniqueRef shadowCastTech;

		bool showDebug = false;

		int currentCL = 0;
		int alarmCount = 0;
		bool inNormalTheme = true;
		float time2Back2Normal = 0.0f;		

		EventsQueue eventsQueue;

		bool thingAbilitiesState[ThingAbility::ThingAbilityMaxCount];
		bool cheatsState[Cheat::CheatMaxCount];

		struct ConeView
		{
			Math::Vector2 from;
			Utils::Angle angle;
			Utils::Angle halfViewAngle;
			float viewDist;
			float agrometr;
		};

		AssetTextureRef coneViewTexture;
		eastl::vector<ConeView> coneViews;

		eastl::vector<Sprite::PolygonVertex> point2Draw;

		struct BulletTrace
		{
			bool redDot = false;
			float alpha = 1.0f;
			Math::Vector2 from;
			Math::Vector2 to;
			float timer = -1.0f;
		};

		eastl::vector<BulletTrace> bulletTraces;

		SpriteEntity* bulletTrace = nullptr;
		SpriteEntity* bulletTraceTurret = nullptr;
		SpriteEntity* redDot = nullptr;

		VFXEmitterInstantiable *surfaceHit = nullptr;
		VFXEmitterInstantiable *playerHit = nullptr;

		struct BulletProjectile
		{
			Math::Vector2 pos;
			Math::Vector2 startPos;
			Math::Vector2 vel;
			float leftDistance;
			float time;
			float fadeTimer;
			bool alive;
			bool allowFriendlyFire;
		};		

		eastl::vector<BulletProjectile> bulletProjectiles;

		eastl::string nextLevel;
		FrontendState pendingFrontendState = FrontendState::Idle;
		FrontendState frontendState = FrontendState::Idle;
		eastl::vector<FrontendState> frontendStakedStates;		

		struct Accuracy
		{
			META_DATA_DECL_BASE(Accuracy)

			float distance;
			float error;
		};

		eastl::vector<Accuracy> accuracyTable;

		int shootCountToMaxAccuracy;

		struct Throwable
		{
			ThrowableDesc* desc;

			Math::Vector3 pos;
			Math::Vector3 dir;
			float rotation = 0.0f;
			float trailTime = 0.0f;

			float hitTime = -1.f;
			float distance = 0.0f;
		};

		eastl::vector<Throwable> throwables;

		void Clear(bool fullClear);
		void StartFrontendState();
		void FinishFrontendState();
		void UpdateFrontendState(float dt);

		void StartNextTrack();
		void UpdateCamTrack(float dt);

		void DrawTracer(SpriteEntity* tracer, Math::Vector2 from, Math::Vector2 to, float thickness, Color color, Color emessive);

		void InstatiateTunnelDoors();

		float toBeContinuedTimer = 0.0f;		

		int   comboMeter = 0;
		int   kills2Ability = 0;
		int   kills2AbilityMax = 0;
		float comboTimeout = -1.0f;
		float comboTimeoutMax = 1.0f;
		float comboInvincibileTime = -1.0f;
		ThunderCharge* thunderCharge = nullptr;

	public:

		int aliasUIActive = -1;
		int aliasUIPause = -1;
		int aliasUIUp = -1;
		int aliasUIDown = -1;
		int aliasUILeft = -1;
		int aliasUIRight = -1;

		LightDust* lightDust = nullptr;

		void IncreaseComboMeter();
		void AddScore(int score);

		SelectDifficultyMenu* selectDifficultyMenu = nullptr;

		struct Difficulty
		{
			META_DATA_DECL_BASE(Difficulty)

			eastl::string name;
			float reactionMul = 1.0f;
			bool allowCheckPoints = true;
			float followLaserSpeedMul = 1.0f;
			float mineTimerMul = 1.0f;
			float scoreMultiplayer = 1.0f;
			int difficultyScore = 0;
		};

		Difficulty* curDifficulty = nullptr;
		eastl::vector<Difficulty> difficulties;

		void SetDifficulty(const char* difficulty);
		Difficulty* GetDifficulty();

		GameState gameState;

		SpriteEntity *dashArrow = nullptr;
		SpriteEntity *dashTail = nullptr;
		SpriteEntity *dashZone = nullptr;
		SpriteEntity *dashSlash = nullptr;
		SpriteEntity *killMarker = nullptr;

		SoundEvent *ambienceSound = nullptr;

		bool hackDisallowCursor = false;

		bool gamepadConnected = false;

		float maxBulletSpeed;

		bool demoMode = false;

		WorldManager()
		{
			memset(thingAbilitiesState, 0, sizeof(thingAbilitiesState));
			memset(cheatsState, 0, sizeof(cheatsState));
		}

		META_DATA_DECL(WorldManager);

		void Init() override;
		void Release() override;
		void Play() override;
		void Update(float dt) override;

		static WorldManager* instance;

		EventsQueue& GetEventsQueue() { return eventsQueue; }

		void Subscribe();		
		void OnThingAbilityAvailabiltyChange(const EventOnThingAbilityAvailabiltyChange& evt);
		void OnCheatStateChange(const EventOnCheatStateChange& evt);

		void SetPlayer(Player* setPlayer);

		void CenterCamera();
		void SetCameraTarget(SceneEntity* target, bool centerCamera);

		BloodManager* GetBlood() { return bloodManager; }

		bool inline IsThingAbilityAvailable(ThingAbility ability)
		{
			return thingAbilitiesState[(int)ability];
		}

		bool inline IsCheatEnabled(Cheat cheat)
		{
			return cheatsState[(int)cheat];
		}

		bool IsPlayerCanBeDetectd() const;

		bool IsPlayerVisibleInSector(Math::Vector3 pos, float dist, float angle, float cone, float alwaysVisibleDist) const;

		bool IsPlayerInSphere(Math::Vector3 pos, float radius) const;

		Player* GetPlayer();
		Math::Vector3 GetPlayerPos();
		Math::Vector3 GetPlayerUp();
		bool HasPuppet();
		Utils::Angle GetAngleToPlayer(Math::Vector3 pos);
		float GetDistanceToPlayer(Math::Vector3 pos);
		float GetDirectionToPlayer(Math::Vector3 pos);		

		struct PlayerSnapshot
		{
			Math::Vector3 pos;
			Math::Vector3 forward;
			Math::Vector3 up;
			Math::Vector3 velocity;
		};

		PlayerSnapshot GetPlayerSnapshot();
		Math::Vector3 CalcShootingDirToPlayer(Math::Vector3 pos, int shootCount, const PlayerSnapshot &playerSnapshot);

		bool IsPlayerKilled();
		void KillPlayer(Math::Vector3 killDir, DeathSource deathSource);

		void AddBulletTrace(Math::Vector3 from, Math::Vector3 to);
		void AddRedDot(Math::Vector3 from, Math::Vector3 to, float alpha);
		void AddBulletProjectile(Math::Vector3 from, Math::Vector3 vel, float maxDistance, bool allowFriendlyFire);
		void AddViewCone(Math::Vector3 from, Utils::Angle angle, Utils::Angle viewAngle, float viewDist, float agrometr);
		void AddThrowable(Math::Vector3 from, Math::Vector3 dir, ThrowableDesc* throwableDesc);
		void DrawEffetcsDL(float dt);
		void DrawEffetcs(float dt);

		void Detonate(Math::Vector3 pos, float radius, SceneEntity* skipTarget);

		void StartFade(bool fadeIn, float time, eastl::function<void()> callback = eastl::function<void()>());
		void DrawFade(float dt);

		Utils::MaybeCastRes RayCastKillTriggers(Math::Vector3 from, Math::Vector3 dir, float length);

		using TimerID = int;

		struct Timer
		{
			eastl::fixed_function<sizeof(void*), void()> invokeEvent;
			float timeout;
			TimerID id;
		};

		eastl::vector<Timer> timers;
		TimerID nextTimerId = 0;

		template <typename EventType>
		TimerID StartTimer(EventType &&evt, float timeout)
		{
			auto timer = [timerEvent=eastl::move(evt)]()
			{
				WorldManager::instance->GetEventsQueue().InvokeEvent<EventType>(eastl::move(timerEvent));
			};

			const TimerID timerId = ++nextTimerId;
			timers.push_back({eastl::move(timer), timeout, timerId});

			return timerId;
		}

		void StopTimer(TimerID id)
		{
			timers.erase(eastl::remove_if(timers.begin(), timers.end(), [id](const Timer &t)
			{
				return t.id == id;
			}), timers.end());
		}

		void SetNextLevel(const eastl::string& levelName, bool resetCheckpoint);
		FrontendState GetFrontendState() { return frontendState; };
		void SetFrontendStateViaFade(FrontendState frontendState);
		void SetFrontendState(FrontendState frontendState);

		void PushFrontendState(FrontendState frontendState);
		void PopFrontendState();

		void SetMusicTheme(MusicTheme theme, float fadeTime);
		void ChangeAlarmCount(bool inc);

		void RenderShadow1D(int index, Math::Vector2 pos, float size, bool needRestoreState = true);
		void RenderSpriteWithShadow1D(int index, Color color, Transform trans, float size, AssetTextureRef texture);
	};

	namespace Timers
	{
		template <typename Event>
		inline WorldManager::TimerID Start(Event &&evt, float timeout)
		{
			return WorldManager::instance ? WorldManager::instance->StartTimer<Event>(eastl::move(evt), timeout) : -1;
		}

		inline void Stop(WorldManager::TimerID id)
		{
			if (WorldManager::instance != nullptr)
			{
				WorldManager::instance->StopTimer(id);
			}
		}

		template <typename Event>
		inline WorldManager::TimerID Restart(Event &&evt, float timeout, WorldManager::TimerID prevId = -1)
		{
			if (prevId != -1)
			{
				Timers::Stop(prevId);
			}
			return Timers::Start(eastl::move(evt), timeout);
		}
	}

	namespace MainEventsQueue
	{
		template<typename Event>
		inline void PushEvent(Event&& inEvent)
		{
			if (WorldManager::instance != nullptr)
			{
				WorldManager::instance->GetEventsQueue().PushEvent<Event>(eastl::move(inEvent));
			}
		}

		template<typename Event, typename... Args>
		inline void PushEvent(Args&&... args)
		{
			if (WorldManager::instance != nullptr)
			{
				WorldManager::instance->GetEventsQueue().PushEvent<Event>(eastl::forward<Args>(args)...);
			}
		}

		template<typename Event>
		inline void InvokeEvent(Event&& event)
		{
			if (WorldManager::instance != nullptr)
			{
				WorldManager::instance->GetEventsQueue().InvokeEvent(eastl::move(event));
			}
		}

		template<typename Event, typename... Args>
		inline void InvokeEvent(Args&&... args)
		{
			if (WorldManager::instance != nullptr)
			{
				WorldManager::instance->GetEventsQueue().InvokeEvent<Event>(eastl::forward<Args>(args)...);
			}
		}

		template<typename Event, typename Object, typename Method>
		inline void Subscribe(Object *object, Method method)
		{
			if (WorldManager::instance != nullptr)
			{
				WorldManager::instance->GetEventsQueue().Subscribe<Event>(object, method);
			}
		}

		inline void Unsubscribe(EventsQueue::HandlerObject object)
		{
			if (WorldManager::instance != nullptr)
			{
				WorldManager::instance->GetEventsQueue().Unsubscribe(object);
			}
		}
	}
}