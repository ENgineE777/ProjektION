
#pragma once

#include "Root/Scenes/SceneEntity.h"
#include "Support/MetaData.h"
#include "Common/Utils.h"
#include "root/Assets/AssetTexture.h"
#include "root/Render/RenderTechnique.h"

namespace Orin::Overkill
{
	class FogZone : public SceneEntity
	{
		AssetTextureRef texture;
        AssetTextureRef noise;
        RenderTechniqueRef fogQuad;

        float timer = 0.0f;

	public:
		
		META_DATA_DECL_BASE(FogZone)

	#ifndef DOXYGEN_SKIP

		FogZone();
		virtual ~FogZone() = default;

		void Init() override;
		void ApplyProperties() override;
		void Draw(float dt);
	#endif
	};
}