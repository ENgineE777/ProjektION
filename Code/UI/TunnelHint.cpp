
#include "TunnelHint.h"

namespace Orin::Overkill
{
	ENTITYREG(SceneEntity, TunnelHint, "Overkill/UI", "TunnelHint")

	META_DATA_DESC(TunnelHint)
		BASE_SCENE_ENTITY_PROP(TunnelHint)

		ASSET_TEXTURE_PROP(TunnelHint, hintUp, "Keyboard", "TextureUp")
		ASSET_TEXTURE_PROP(TunnelHint, hintLeft, "Keyboard", "TextureLeft")
		ASSET_TEXTURE_PROP(TunnelHint, hintDown, "Keyboard", "TextureDown")
		ASSET_TEXTURE_PROP(TunnelHint, hintRight, "Keyboard", "TextureRight")

		ASSET_TEXTURE_PROP(TunnelHint, hintUpGP, "Gamepad", "TextureUpGP")
		ASSET_TEXTURE_PROP(TunnelHint, hintLeftGP, "Gamepad", "TextureLeftGP")
		ASSET_TEXTURE_PROP(TunnelHint, hintDownGP, "Gamepad", "TextureDownGP")
		ASSET_TEXTURE_PROP(TunnelHint, hintRightGP, "Gamepad", "TextureRightGP")

	META_DATA_DESC_END()

	ORIN_EVENTS(TunnelHint)
		ORIN_EVENT(TunnelHint, CmdShowTunnelHint, OnShowHint)
	ORIN_EVENTS_END(TunnelHint)

	void TunnelHint::Init()
	{
		ScriptEntity2D::Init();

		Tasks(true)->AddTask(10, this, (Object::Delegate)&TunnelHint::Draw);
	}

	void TunnelHint::Play()
	{
		ScriptEntity2D::Play();

		ORIN_EVENTS_SUBSCRIBE(TunnelHint);

		textures[0] = hintUp;
		textures[1] = hintLeft;
		textures[2] = hintDown;
		textures[3] = hintRight;

		textures[4] = hintUpGP;
		textures[5] = hintLeftGP;
		textures[6] = hintDownGP;
		textures[7] = hintRightGP;
	}

	void TunnelHint::OnShowHint(const CmdShowTunnelHint& evt)
	{
		needToDraw = true;
		hint = evt.hint;
		pos = evt.pos;
	}

	void TunnelHint::Draw(float dt)
	{
		if (needToDraw)
		{
			Transform trans;
			trans.objectType = ObjectType::Object2D;
			trans.offset = 0.0f;
			trans.size = TILE_SIZE;
			trans.position = pos;

			if (WorldManager::instance->gamepadConnected)
			{
				hint += 4;
			}

			textures[hint].prg = Sprite::quadPrgNoZ;
			textures[hint].Draw(&trans, COLOR_WHITE, dt);

			needToDraw = false;
		}
	}
}