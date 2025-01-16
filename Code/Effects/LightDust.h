#pragma once

#include "SceneEntities/2D/ScriptEntity2D.h"
#include "SceneEntities/2D/Camera2D.h"
#include "Common/Utils.h"
#include "Common/Events.h"
#include "Effects/BloodEffect.h"

#include "EASTL/array.h"

namespace Orin::Overkill
{
	class LightDust : public SceneEntity
	{
        struct DustDesc
        {
            META_DATA_DECL_BASE(Texture)

            AssetTextureRef texture;
        };

        struct Dust
        {
            Math::Vector2 pos;
            Math::Vector2 dir;
            float timeLife = 1.0f;
            float timeLifeSpeed = 1.0f;
            float speed = 1.0f;
            float scale = 1.0f;
            float alpha = 1.0f;
            float rotation = 0.0f;
            int desc_index = 0;

            void Randomize(int size, int dustDescCount)
            {
                pos = {Utils::GetRandom(0.0f, (float)size), Utils::GetRandom(0.0f, (float)size)};
                dir = { Utils::GetRandom(-1.0f, 1.0f), Utils::GetRandom(-1.0f, 1.0f) };
                dir.Normalize();

                timeLife = 1.0f;
                timeLifeSpeed = 1.0f / Utils::GetRandom(3.0f, 7.0f);
                speed = Utils::GetRandom(5.0f, 12.0f);
                scale = Utils::GetRandom(0.75f, 1.75f);
                alpha = Utils::GetRandom(0.6f, 1.0f);
                rotation = Utils::GetRandom(0.0f, 360.0f);
                desc_index = (int)Utils::GetRandom(0.0f, (float)dustDescCount - 0.01f);
            }
        };

        eastl::vector<DustDesc> dustDescs;
        eastl::vector<Dust> dusts;

        int size = 200;        

    public:

        TextureRef dustRT;

        META_DATA_DECL(LightDust);

        void Init() override;		

        void Draw(float dt);
    };
}