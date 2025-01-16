
#pragma once

#include "SceneEntities/2D/ScriptEntity2D.h"
#include "World/WorldManager.h"

namespace Orin::Overkill
{
	class ThrowHint : public ScriptEntity2D
	{		
		AssetTextureRef hint;

		AssetTextureRef hintGP;

		bool needToDraw = false;
		Math::Vector3 pos;

	public:

		META_DATA_DECL_BASE(TunnelHint)

	#ifndef DOXYGEN_SKIP

		ThrowHint() = default;;
		virtual ~ThrowHint() = default;

		void Init() override;
		void Play() override;
		void OnShowHint(const CmdShowThrowHint& evt);
		void Draw(float dt);
	#endif
	};
}