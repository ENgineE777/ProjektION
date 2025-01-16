
#include "Root/Root.h"
#include "World/WorldManager.h"

namespace Orin::Overkill
{
	ORIN_EVENTS(GameState)
		ORIN_EVENT(GameState, EventOnRestart, OnRestart)
		ORIN_EVENT(GameState, EventOnCheckpointReached, OnCheckpointReached)
	ORIN_EVENTS_END(GameState)

	void GameState::Reset()
	{
		killsCout = 0;
		deathCount = 0;		
		totalTime = 0.0f;
		initialLevelTimer = 0.0f;
		currentLevelTimer = 0.0f;
		initialScore = 0;
		totalScore = 0;

		curDifficulty = "easy";
	}

	void GameState::ResetLevelTime()
	{
		initialLevelTimer = 0.0f;
		currentLevelTimer = 0.0f;
		initialScore = 0;
		totalScore = 0;
	}

	void GameState::MarkStartOfLevel(const char* levelName, bool resetCheckpoint)
	{
		curLevelName = levelName;

		if (resetCheckpoint)
		{
			checkpointID = -1;
		}			

		if (!IsLevelOpened(levelName))
		{
			openedLevels.push_back(levelName);
		}

		WorldManager::instance->SetDifficulty(curDifficulty.c_str());

		SaveGameState();
	}

	bool GameState::IsLevelOpened(const char* levelName)
	{
		auto iter = eastl::find_if(openedLevels.begin(), openedLevels.end(), [this, levelName](const eastl::string& level) { return StringUtils::IsEqual(level.c_str(), levelName); });

		return iter != openedLevels.end();		
	}

	uint32_t GameState::GetCheckpointID()
	{
		return checkpointID;
	}

	bool GameState::IsDifficulty(const char* difficulty)
	{
		return true;
	}

	const char* GameState::GetDifficulty()
	{
		return curDifficulty.c_str();
	}

	void GameState::SetDifficulty(const char* difficulty)
	{
		curDifficulty = difficulty;
	}

	void GameState::LoadGameState()
	{
		auto path = GetRoot()->GetPath(IRoot::Path::Project) + eastl::string("//gameState");

		JsonReader reader;

		if (reader.ParseFile(path.c_str()))
		{
			reader.Read("curDifficulty", curDifficulty);
			reader.Read("curLevelName", curLevelName);
			reader.Read("checkpointID", checkpointID);
			reader.Read("killsCout", killsCout);
			reader.Read("deathCount", deathCount);
			reader.Read("score", initialScore);
			reader.Read("totalTime", totalTime);
			reader.Read("initialLevelTimer", initialLevelTimer);

			reader.Read("musicVolume", musicVolume);
			reader.Read("sfxVolume", sfxVolume);
			reader.Read("locale", locale);

			while (reader.EnterBlock("openedLevels"))
			{
				eastl::string name;
				reader.Read("name", name);

				openedLevels.push_back(name);
				reader.LeaveBlock();
			}

			while (reader.EnterBlock("levelStats"))
			{
				LevelStat levelStat;

				reader.Read("name", levelStat.name);
				reader.Read("score", levelStat.score);
				reader.Read("totalTime", levelStat.totalTime);

				levelStats.push_back(levelStat);
				reader.LeaveBlock();
			}
		}

		ORIN_EVENTS_SUBSCRIBE(GameState);

		GetRoot()->GetSounds()->SetVCAVolume("vca:/Music", musicVolume * 0.01f);		
		GetRoot()->GetSounds()->SetVCAVolume("vca:/SFX", sfxVolume * 0.01f);

		GetRoot()->GetLocalization()->SetCurrentLocale(locale.c_str());
	}

	void GameState::SaveGameState()
	{
		auto path = GetRoot()->GetPath(IRoot::Path::Project) + eastl::string("//gameState");

		JsonWriter writer;

		if (writer.Start(path.c_str()))
		{
			writer.Write("curDifficulty", curDifficulty);
			writer.Write("curLevelName", curLevelName);
			writer.Write("checkpointID", checkpointID);
			writer.Write("killsCout", killsCout);
			writer.Write("deathCount", deathCount);
			writer.Write("score", initialScore);
			writer.Write("totalTime", totalTime);
			writer.Write("initialLevelTimer", initialLevelTimer);

			writer.Write("musicVolume", musicVolume);
			writer.Write("sfxVolume", sfxVolume);
			writer.Write("locale", locale);

			writer.StartArray("openedLevels");

			for (auto& name : openedLevels)
			{
				writer.StartBlock(nullptr);
				writer.Write("name", name);
				writer.FinishBlock();
			}

			writer.FinishArray();

			writer.StartArray("levelStats");

			for (auto& levelStat : levelStats)
			{
				writer.StartBlock(nullptr);
				writer.Write("name", levelStat.name);
				writer.Write("score", levelStat.score);
				writer.Write("totalTime", levelStat.totalTime);
				writer.FinishBlock();
			}

			writer.FinishArray();
		}
	}

	bool GameState::IsEmpty()
	{
		return curLevelName.empty();
	}

	void GameState::MarkKill()
	{
		killsCout++;
	}
	
	int GameState::GetKillsCount()
	{
		return killsCout;
	}

	void GameState::MarkDeath()
	{
		deathCount++;
	}

	int GameState::GetDeathsCount()
	{
		return deathCount;
	}

	const char* GameState::GetCurrentLevel()
	{
		return curLevelName.c_str();
	}

	void GameState::Update(float dt)
	{
		currentLevelTimer += dt;
		totalTime += dt;
	}

	float GameState::GetLevelTime()
	{
		return currentLevelTimer;
	}

	float GameState::GetTotalTime()
	{
		return totalTime;
	}

	int GameState::GetLevelScore()
	{
		return totalScore;
	}

	void GameState::AddScore(int score)
	{
		totalScore += score;
	}

	void GameState::OnRestart(const EventOnRestart& evt)
	{
		currentLevelTimer = initialLevelTimer;
		totalScore = initialScore;

		SaveGameState();
	}

	void GameState::OnCheckpointReached(const EventOnCheckpointReached& evt)
	{
		initialLevelTimer = currentLevelTimer;
		initialScore = totalScore;
		checkpointID = evt.checpointID;

		SaveGameState();
	}

	void GameState::SaveLevelStat()
	{		
		for (int i = 0; i < levelStats.size(); i++)
		{
			if (StringUtils::IsEqual(levelStats[i].name.c_str(), curLevelName.c_str()))
			{
				if (levelStats[i].score < totalScore)
				{
					levelStats[i].score = totalScore;
					levelStats[i].totalTime = currentLevelTimer;
				}

				return;
			}
		}

		LevelStat levelStat;
		levelStat.name = curLevelName;
		levelStat.score = totalScore;
		levelStat.totalTime = currentLevelTimer;

		levelStats.push_back(levelStat);

		SaveGameState();
	}
}