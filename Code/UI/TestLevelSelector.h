
#pragma once

#include "Root/Scenes/SceneEntity.h"
#include "Support/MetaData.h"
#include "SceneEntities/2D/ScriptEntity2D.h"

namespace Orin::Overkill
{
	class TestLevelSelector : public ScriptEntity2D
	{
		const int levelsPerPage = 12;
		int selMap = 0;
		int currentPage = 0;
		int pageCount = 0;

		struct TestLevel
		{
			eastl::string name;
			eastl::string path;
		};

		int scanMapsType = 0;
		eastl::vector<TestLevel> levels;

		void ScanLevels();

	public:

		META_DATA_DECL(TestLevelSelector)

		void Play() override;
		void ApplyProperties() override;
		void Update(float dt) override;
	};
}