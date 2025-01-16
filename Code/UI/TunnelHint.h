
#pragma once

#include "SceneEntities/2D/ScriptEntity2D.h"
#include "World/WorldManager.h"

namespace Orin::Overkill
{
	class TunnelHint : public ScriptEntity2D
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

		META_DATA_DECL_BASE(TunnelHint)

	#ifndef DOXYGEN_SKIP

		TunnelHint() = default;;
		virtual ~TunnelHint() = default;

		void Init() override;
		void Play() override;
		void OnShowHint(const CmdShowTunnelHint& evt);
		void Draw(float dt);
	#endif
	};
}