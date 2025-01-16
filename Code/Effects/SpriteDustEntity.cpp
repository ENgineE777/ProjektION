
#include "SpriteDustEntity.h"
#include "World/WorldManager.h"
#include "Root/Root.h"

namespace Orin::Overkill
{
	class QuadDustedTechnique : public RenderTechnique
	{
	public:
		virtual const char* GetVsName() { return "Shaders/dusted_vs.shd"; };
		virtual const char* GetPsName() { return "Shaders/dusted_ps.shd"; };

		virtual void ApplyStates()
		{
			GetRoot()->GetRender()->GetDevice()->SetDepthWriting(false);
			GetRoot()->GetRender()->GetDevice()->SetAlphaBlend(true);
			GetRoot()->GetRender()->GetDevice()->SetBlendFunc(BlendArg::ArgSrcAlpha, BlendArg::ArgOne);
			GetRoot()->GetRender()->GetDevice()->SetCulling(CullMode::CullNone);
		};
	};

	ENTITYREG(SceneEntity, SpriteDustEntity, "Overkill/Effects", "SpriteDustEntity")

	META_DATA_DESC(SpriteDustEntity)
		BASE_SCENE_ENTITY_PROP(SpriteDustEntity)
		ASSET_TEXTURE_PROP(SpriteDustEntity, texture, "Visual", "Texture")
		COLOR_PROP(SpriteDustEntity, color, COLOR_WHITE, "Visual", "Color")
	META_DATA_DESC_END()

	SpriteDustEntity::SpriteDustEntity() : SceneEntity()
	{
	}

	void SpriteDustEntity::Init()
	{
		transform.objectType = ObjectType::Object2D;
		transform.transformFlag = (TransformFlag)(TransformFlag::SpriteTransformFlags ^ TransformFlag::RectSizeXY);

		dustedQuad = GetRoot()->GetRender()->GetRenderTechnique<QuadDustedTechnique>(_FL_);
	}

	void SpriteDustEntity::ApplyProperties()
	{
		Math::Vector2 size = texture.GetSize();
		transform.size = Math::Vector3(size.x, size.y, 0.0f);

#ifdef ORIN_EDITOR
		Tasks(true)->DelAllTasks(this);
#endif

		Tasks(true)->AddTask(7, this, (Object::Delegate)&SpriteDustEntity::Draw);
	}

	void SpriteDustEntity::Draw(float dt)
	{
		if (IsVisible())
		{
			if (GetScene()->IsPlaying() && WorldManager::instance)
			{				
				dustedQuad->SetTexture(ShaderType::Pixel, "dustMap", WorldManager::instance->lightDust->dustRT);
				texture.prg = dustedQuad;
			}
			else
			{
				texture.prg = Sprite::quadLightenPrg;
			}

			Transform trans = transform;		
			texture.Draw(&trans, color, dt);
		}
	}
}