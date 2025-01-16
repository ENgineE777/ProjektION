
#pragma once

#include "Root/Scenes/SceneEntity.h"
#include "Support/MetaData.h"
#include "Common/Utils.h"
#include "root/Assets/AssetTexture.h"
#include "root/Render/RenderTechnique.h"

namespace Orin::Overkill
{
	class Rotator : public SceneEntity
	{
	public:
		
		META_DATA_DECL_BASE(Rotator)

	#ifndef DOXYGEN_SKIP

		Rotator();
		virtual ~Rotator() = default;

		void Init() override;
		void Update(float dt);
	#endif
	};
}