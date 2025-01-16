#pragma once

#include "SceneEntities/2D/ScriptEntity2D.h"
#include "SceneEntities/2D/Camera2D.h"
#include "Common/Utils.h"
#include "Common/Events.h"
#include "Effects/BloodEffect.h"

#include "EASTL/array.h"

namespace Orin::Overkill
{
	class BloodPuddle : public BloodEffect
	{
        struct Texture
        {
            META_DATA_DECL_BASE(Texture)

            AssetTextureRef texture;
        };

        eastl::vector<Texture> textures;
        eastl::vector<AssetTextureRef> textureForInstances;

        Utils::RndGenerator rndGenerator;

    public:
        META_DATA_DECL(BloodPuddle);

        void Init() override;
        void Release() override;
		void Play() override;
		void Update(float dt) override;

        void Draw(float dt);
    };
}