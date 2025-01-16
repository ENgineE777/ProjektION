#include "LightDust.h"
#include "World/WorldManager.h"

namespace Orin::Overkill
{
	ENTITYREG(SceneEntity, LightDust, "Overkill/Effects", "LightDust")

    META_DATA_DESC(LightDust::DustDesc)
		ASSET_TEXTURE_PROP(LightDust::DustDesc, texture, "Texture", "texture")
	META_DATA_DESC_END()

	META_DATA_DESC(LightDust)
		BASE_SCENE_ENTITY_PROP(LightDust)
        ARRAY_PROP(LightDust, dustDescs, DustDesc, "Prop", "dustDescs")
	META_DATA_DESC_END()

    void LightDust::Init()
    {
        transform.objectType = ObjectType::Object2D;
		transform.transformFlag = TransformFlag::MoveXYZ | TransformFlag::RotateZ;

        Tasks(true)->AddTask(-15, this, (Object::Delegate)&LightDust::Draw);

        dusts.resize(65);

        for (auto& dust : dusts)
        {
            dust.Randomize(size, (int)dustDescs.size());
        }

        dustRT = GetRoot()->GetRender()->GetDevice()->CreateTexture(size, size, TextureFormat::FMT_A8R8G8B8, 1, true, TextureType::Tex2D, _FL_);
        dustRT->SetFilters(TextureFilter::Point, TextureFilter::Point);
    }

    void LightDust::Draw(float dt)
    {
        if (!IsVisible())
		{
			return;
		}        

        Math::Matrix curView;
        Math::Matrix curProj;
        auto camPos = Sprite::GetCamPos();
        auto zoom = Sprite::GetCamZoom();

        GetRoot()->GetRender()->GetTransform(TransformStage::View, curView);
        GetRoot()->GetRender()->GetTransform(TransformStage::Projection, curProj);

        Math::Matrix view;
        Math::Matrix proj;
        
        Math::Vector2 pos = Math::Vector2((float)size, (float)size) * 0.5f;
        Sprite::SetCamPos({ pos.x, pos.y });

        const Math::Vector3 upVector{ 0.0f, 1.0f, 0.f };

        float dist = size * 0.5f / (tanf(22.5f * Math::Radian));
        view.BuildView(Sprite::ToUnits(Math::Vector3(pos.x, pos.y, -dist)), Sprite::ToUnits(Math::Vector3(pos.x, pos.y, -dist + 1.0f)), upVector);

        GetRoot()->GetRender()->SetTransform(TransformStage::View, view);

        proj.BuildProjection(45.0f * Math::Radian, 1.0f, 1.0f, 1000.0f);
        GetRoot()->GetRender()->SetTransform(TransformStage::Projection, proj);

        GetRoot()->GetRender()->GetDevice()->SetRenderTarget(0, dustRT);
        GetRoot()->GetRender()->GetDevice()->Clear(true, COLOR_BLACK_A(0.0f), false, 1.0f);

        for (auto& dust : dusts)
        {
            dust.pos += dust.dir * dust.speed * dt;

            if (dust.pos.x > size)
            {
                dust.pos.x = dust.pos.x - size;
            }

            if (dust.pos.x < 0.0f)
            {
                dust.pos.x += size;
            }

            if (dust.pos.y > size)
            {
                dust.pos.y = dust.pos.y - size;
            }

            if (dust.pos.y < 0.0f)
            {
                dust.pos.y += size;
            }

            dust.timeLife -= dt * dust.timeLifeSpeed;

            if (dust.timeLife < 0.0f)
            {
                dust.Randomize(size, (int)dustDescs.size());
            }
            else
            {
                Transform trans;
                trans.objectType = ObjectType::Object2D;
                trans.position = { dust.pos.x, dust.pos.y, 0.0f};
                trans.size = 5.0f;
                trans.rotation = { 0.0f, 0.0, dust.rotation };
                trans.scale = { dust.scale,dust.scale, 1.0f };

                float alpha = 1.0f;

                if (dust.timeLife < 0.2f)
                {
                    alpha = dust.timeLife / 0.2f;
                }

                if (dust.timeLife > 0.8f)
                {
                    alpha = 1.0f - (dust.timeLife - 0.8f) / 0.2f;
                }

                dustDescs[dust.desc_index].texture.prg = Sprite::quadPrgNoZ;
                dustDescs[dust.desc_index].texture.Draw(&trans, COLOR_WHITE_A(alpha * dust.alpha), dt);
            }
        }

        GetRoot()->GetRender()->GetDevice()->RestoreRenderTarget();

        GetRoot()->GetRender()->SetTransform(TransformStage::View, curView);
        GetRoot()->GetRender()->SetTransform(TransformStage::Projection, curProj);

        Sprite::SetCamPos(camPos);
        Sprite::SetCamZoom(zoom);

        //GetRoot()->GetRender()->DebugSprite(dustRT, 10.0f, 200.0f * 3, COLOR_WHITE);
    }
}