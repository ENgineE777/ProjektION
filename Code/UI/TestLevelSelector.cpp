
#include "TestLevelSelector.h"
#include "World/WorldManager.h"
#include "Root/Root.h"
#include "Common/Utils.h"

#include <windows.h>
#include <shellapi.h>

namespace Orin::Overkill
{
	ENTITYREG(SceneEntity, TestLevelSelector, "Overkill/World", "TestLevelSelector")

	META_DATA_DESC(TestLevelSelector)
		BASE_SCENE_ENTITY_PROP(TestLevelSelector)
	META_DATA_DESC_END()

	const char* dirNames[] = {"/maps", "/demo", "/testmaps", "/playtest"};
	const int levelTypesCount = sizeof(dirNames) / sizeof(char*);

	void TestLevelSelector::ApplyProperties()
	{
		for (auto& testLevel : levels)
		{
			char filename[128];
			StringUtils::GetFileName(testLevel.path.c_str(), filename);
			StringUtils::RemoveExtension(filename);

			testLevel.name = filename;
		}
	}

	void TestLevelSelector::ScanLevels()
	{
		auto assets = GetRoot()->GetAssets()->GetAssetsMap();	

		const char* dirName = dirNames[scanMapsType];

		levels.clear();

		for (auto& entry : assets)
		{
			const char* path = entry.first.c_str();

			if (strstr(path, dirName))
			{
				TestLevel level;

				level.path = path;

				char name[512];
				StringUtils::GetFileName(path, name);
				StringUtils::RemoveExtension(name);

				level.name = name;

				levels.emplace_back(level);
			}
		}

		selMap = 0;
		currentPage = 0;
		pageCount = (int)((float)levels.size() / (float)levelsPerPage) + 1;
	}

	void TestLevelSelector::Play()
	{
		ScriptEntity2D::Play();

		ScanLevels();

		int count = 0;

		while (levels.size() == 0 && count < levelTypesCount)
		{
			scanMapsType = (scanMapsType + 1) % levelTypesCount;
			count++;
			ScanLevels();
		}
	}
	
	void TestLevelSelector::Update(float dt)
	{
		int dbgLine = 1;
		
		const bool lbPressed = GetRoot()->GetControls()->DebugKeyPressed("JOY_LEFT_SHOULDER", AliasAction::JustPressed);
		const bool rbPressed = GetRoot()->GetControls()->DebugKeyPressed("JOY_RIGHT_SHOULDER", AliasAction::JustPressed);
		if (GetRoot()->GetControls()->DebugKeyPressed("KEY_T", AliasAction::JustPressed) || lbPressed || rbPressed)
		{
			levels.clear();

			int count = 0;
			int dir = lbPressed ? -1 : 1;

			while (levels.size() == 0 && count < levelTypesCount)
			{
				scanMapsType = (scanMapsType + dir);
				if (scanMapsType < 0)
				{
					scanMapsType = levelTypesCount - 1;
				}
				scanMapsType = scanMapsType % levelTypesCount;

				count++;
				ScanLevels();
			}
		}

		dbgLine++;		

		Sprite::DebugText(dbgLine++, "%s (Press T to change): Select Level (%i/%i)", dirNames[scanMapsType], currentPage + 1, pageCount);

		dbgLine++;

		int index = 0;

		for (int i = 0; i < levelsPerPage; i++)
		{
			int index = levelsPerPage * currentPage + i;

			if (index == levels.size())
			{
				break;
			}

			Sprite::DebugText(dbgLine++, "%s %s", index == selMap ? ">" : " ", levels[index].name.c_str());
		}
			
		if (GetRoot()->GetControls()->DebugKeyPressed("KEY_RETURN", AliasAction::JustPressed) ||
			GetRoot()->GetControls()->DebugKeyPressed("JOY_A", AliasAction::JustPressed))
		{
			WorldManager::instance->selectDifficultyMenu->levelName = levels[selMap].name;
			WorldManager::instance->SetFrontendState(FrontendState::SelectDifficultyMenu);
		}
			
		if (GetRoot()->GetControls()->DebugKeyPressed("KEY_LEFT", AliasAction::JustPressed) ||
			GetRoot()->GetControls()->DebugKeyPressed("JOY_DPAD_LEFT", AliasAction::JustPressed))
		{
			if (currentPage > 0)
			{
				selMap -= levelsPerPage;
				currentPage--;
			}
			else
			{
				selMap = (int)levels.size() - 1;
				currentPage = pageCount - 1;
			}
		}

		if (GetRoot()->GetControls()->DebugKeyPressed("KEY_RIGHT", AliasAction::JustPressed) ||
			GetRoot()->GetControls()->DebugKeyPressed("JOY_DPAD_RIGHT", AliasAction::JustPressed))
		{
			if (currentPage < pageCount - 1)
			{
				selMap += levelsPerPage;
				currentPage++;

				if (selMap >= (int)levels.size())
				{
					selMap = (int)levels.size() - 1;
				}
			}
			else
			{
				selMap = 0;
				currentPage = 0;
			}
		}

		if (GetRoot()->GetControls()->DebugKeyPressed("KEY_DOWN", AliasAction::JustPressed) ||
			GetRoot()->GetControls()->DebugKeyPressed("JOY_DPAD_DOWN", AliasAction::JustPressed))
		{
			selMap++;

			if (selMap >= levelsPerPage * (currentPage + 1))
			{
				currentPage++;
			}

			if (selMap == levels.size())
			{
				selMap = 0;
				currentPage = 0;
			}
		}

		if (GetRoot()->GetControls()->DebugKeyPressed("KEY_UP", AliasAction::JustPressed) ||
			GetRoot()->GetControls()->DebugKeyPressed("JOY_DPAD_UP", AliasAction::JustPressed))
		{
			selMap--;

			if (selMap < levelsPerPage * currentPage)
			{
				currentPage--;
			}

			if (selMap < 0)
			{
				selMap = (int)levels.size() - 1;
				currentPage = pageCount - 1;
			}
		}

		if (GetRoot()->GetControls()->DebugKeyPressed("KEY_ESCAPE", AliasAction::JustPressed) ||
			GetRoot()->GetControls()->DebugKeyPressed("JOY_B", AliasAction::JustPressed))
		{
			WorldManager::instance->SetFrontendState(FrontendState::MainMenu);
		}
	}
}