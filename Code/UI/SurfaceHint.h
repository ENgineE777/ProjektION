
#pragma once

#include "SceneEntities/2D/ScriptEntity2D.h"
#include "World/WorldManager.h"

namespace Orin::Overkill
{
	class SurfaceHint : public ScriptEntity2D
	{
		AssetTextureRef hintUp;
		AssetTextureRef hintLeft;
		AssetTextureRef hintDown;
		AssetTextureRef hintRight;

		AssetTextureRef hintUpGP;
		AssetTextureRef hintLeftGP;
		AssetTextureRef hintDownGP;
		AssetTextureRef hintRightGP;

		AssetTextureRef textures[8];

		bool needToDraw = false;
		int hint = 0;
		Math::Vector3 pos;

	public:

		META_DATA_DECL_BASE(SurfaceHint)

	#ifndef DOXYGEN_SKIP

		SurfaceHint() = default;;
		virtual ~SurfaceHint() = default;

		void Init() override;
		void Play() override;
		void OnShowHint(const CmdShowSurfaceHint& evt);
		void Draw(float dt);
	#endif
	};
}