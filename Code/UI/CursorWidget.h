
#pragma once

#include "Root/Scenes/SceneEntity.h"
#include "Support/MetaData.h"
#include "Support/Sprite.h"
#include "root/Assets/AssetTexture.h"
#include "SceneEntities/UI/ImageWidget.h"

namespace Orin::Overkill
{
	class CursorWidget : public ImageWidget
	{
	public:

		META_DATA_DECL_BASE(CursorWidget)

	#ifndef DOXYGEN_SKIP

		CursorWidget() = default;;
		virtual ~CursorWidget() = default;

		void Draw(float dt);
	#endif
	};
}