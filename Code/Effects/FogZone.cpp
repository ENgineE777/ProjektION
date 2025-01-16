
#include "FogZone.h"
#include "World/WorldManager.h"
#include "Root/Root.h"

namespace Orin::Overkill
{
	class QuadFogTechnique : public RenderTechnique
	{
	public:
		virtual const char* GetVsName() { return "Shaders/fog_vs.shd"; };
		virtual const char* GetPsName() { return "Shaders/fog_ps.shd"; };

		virtual void ApplyStates()
		{
			GetRoot()->GetRender()->GetDevice()->SetDepthWriting(false);
			GetRoot()->GetRender()->GetDevice()->SetAlphaBlend(true);
			GetRoot()->GetRender()->GetDevice()->SetBlendFunc(BlendArg::ArgSrcAlpha, BlendArg::ArgOne);
			GetRoot()->GetRender()->GetDevice()->SetCulling(CullMode::CullNone);
		};
	};

	ENTITYREG(SceneEntity, FogZone, "Overkill/Effects", "FogZone")

	META_DATA_DESC(FogZone)
		BASE_SCENE_ENTITY_PROP(FogZone)
	META_DATA_DESC_END()

		FogZone::FogZone() : SceneEntity()
	{
	}

	void FogZone::Init()
	{
		transform.objectType = ObjectType::Object2D;
		transform.transformFlag = (TransformFlag)(TransformFlag::SpriteTransformFlags);
		transform.size = 100.0f;

		eastl::string path = "Prefabs/Effects/Glow/Fog.png";
		texture = GetRoot()->GetAssets()->GetAssetRef<AssetTextureRef>(path);

		path = "Prefabs/Effects/Glow/noise.png";
		noise = GetRoot()->GetAssets()->GetAssetRef<AssetTextureRef>(path);
		
		fogQuad = GetRoot()->GetRender()->GetRenderTechnique<QuadFogTechnique>(_FL_);
		fogQuad->SetTexture(ShaderType::Pixel, "fogMap", noise->GetTexture());

		texture.prg = fogQuad;
	}

	void FogZone::ApplyProperties()
	{
#ifdef ORIN_EDITOR
		Tasks(true)->DelAllTasks(this);
#endif

		Tasks(true)->AddTask(7, this, (Object::Delegate)&FogZone::Draw);

		timer = 0.0f;
	}

	void FogZone::Draw(float dt)
	{
		if (IsVisible())
		{
			timer += dt * 0.2f;
			
			auto pos = Sprite::ToPixels(transform.GetGlobal().Pos());
			auto size = transform.size * 0.5f;

			if (Sprite::IsRectVisibile(Math::Vector2(pos.x, pos.y) - Math::Vector2(size.x, size.y), Math::Vector2(pos.x, pos.y) + Math::Vector2(size.x, size.y)))
			{
				texture.Draw(&transform, Color(1.0f, 1.0f, timer, 0.7f), dt);
			}
		}
	}
}