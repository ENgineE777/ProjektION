
#include "ThrowHint.h"

namespace Orin::Overkill
{
	ENTITYREG(SceneEntity, ThrowHint, "Overkill/UI", "ThrowHint")

	META_DATA_DESC(ThrowHint)
		BASE_SCENE_ENTITY_PROP(ThrowHint)

		ASSET_TEXTURE_PROP(ThrowHint, hint, "Keyboard", "Texture")

		ASSET_TEXTURE_PROP(ThrowHint, hintGP, "Gamepad", "TextureGP")		

	META_DATA_DESC_END()

	ORIN_EVENTS(ThrowHint)
		ORIN_EVENT(ThrowHint, CmdShowThrowHint, OnShowHint)
	ORIN_EVENTS_END(ThrowHint)

	void ThrowHint::Init()
	{
		ScriptEntity2D::Init();

		Tasks(true)->AddTask(10, this, (Object::Delegate)&ThrowHint::Draw);
	}

	void ThrowHint::Play()
	{
		ORIN_EVENTS_SUBSCRIBE(ThrowHint);
	}

	void ThrowHint::OnShowHint(const CmdShowThrowHint& evt)
	{
		needToDraw = true;
		pos = evt.pos;
	}

	void ThrowHint::Draw(float dt)
	{
		if (needToDraw)
		{
			Transform trans;
			trans.objectType = ObjectType::Object2D;
			trans.offset = 0.0f;
			trans.size = TILE_SIZE;
			trans.position = pos;

			AssetTextureRef texture = (WorldManager::instance->gamepadConnected) ? hintGP : hint;

			texture.prg = Sprite::quadPrgNoZ;
			texture.Draw(&trans, COLOR_WHITE, dt);

			needToDraw = false;
		}
	}
}