
#pragma once

#include "SceneEntities/2D/SpriteEntity.h"
#include "Common/Events.h"

namespace Orin::Overkill
{
	class TutorialHint : public SceneEntity
	{	
		bool active = false;
		bool activated = false;
		float alpha = 1.0f;

		struct Hint
		{
			META_DATA_DECL_BASE(Hint)

			eastl::string locale;
			AssetTextureRef texture;
			AssetTextureRef textureGamePad;
		};

		eastl::vector<Hint> hints;

	public:

		virtual ~TutorialHint() = default;

		META_DATA_DECL(TutorialHint)

		void Init() override;
		void ApplyProperties() override;
		void Play() override;
		void Draw(float dt);
		void Acivate(bool setActive);
		void Release() override;
		void OnRestart(const EventOnRestart& evt);
	};
}
