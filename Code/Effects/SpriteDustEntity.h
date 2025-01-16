
#pragma once

#include "Root/Scenes/SceneEntity.h"
#include "Support/MetaData.h"
#include "root/Assets/AssetTexture.h"
#include "root/Render/RenderTechnique.h"

namespace Orin::Overkill
{
	class SpriteDustEntity : public SceneEntity
	{
	public:

		AssetTextureRef texture;
		Color color;

		RenderTechniqueRef dustedQuad;

		META_DATA_DECL_BASE(SpriteDustEntity)

	#ifndef DOXYGEN_SKIP

		SpriteDustEntity();
		virtual ~SpriteDustEntity() = default;

		void Init() override;
		void ApplyProperties() override;
		void Draw(float dt);
	#endif
	};
}