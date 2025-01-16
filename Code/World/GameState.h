
#pragma once

#include "Common/Utils.h"
#include "Common/Events.h"

namespace Orin::Overkill
{
	class GameState
	{
		struct LevelStat
		{
			eastl::string name;
			int score = 0;
			float totalTime = 0.0f;
		};

		eastl::vector<LevelStat> levelStats;

		eastl::string curDifficulty;
		eastl::string curLevelName;
		eastl::vector<eastl::string> openedLevels;

		uint32_t checkpointID;
		int killsCout = 0;
		int deathCount = 0;
		int initialScore = 0;
		int totalScore = 0;
		float totalTime = 0.0f;

		float currentLevelTimer = 0.0f;
		float initialLevelTimer = 0.0f;

	public:

		int musicVolume = 100;
		int sfxVolume = 100;
		eastl::string locale;

		void Reset();
		void ResetLevelTime();

		void MarkStartOfLevel(const char* levelName, bool resetCheckpoint);
		bool IsEmpty();

		void LoadGameState();
		void SaveGameState();

		void MarkKill();
		int GetKillsCount();

		void MarkDeath();
		int GetDeathsCount();

		bool IsLevelOpened(const char* levelName);

		const char* GetCurrentLevel();
		uint32_t GetCheckpointID();

		bool IsDifficulty(const char* difficulty);
		const char* GetDifficulty();
		void SetDifficulty(const char* difficulty);

		void Update(float dt);
		float GetLevelTime();
		float GetTotalTime();
		int GetLevelScore();

		void AddScore(int score);

		void OnRestart(const EventOnRestart& evt);
		void OnCheckpointReached(const EventOnCheckpointReached& evt);

		void SaveLevelStat();
	};
}