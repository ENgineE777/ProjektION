
#include "SurfaceHint.h"

namespace Orin::Overkill
{
	ENTITYREG(SceneEntity, SurfaceHint, "Overkill/UI", "SurfaceHint")

	META_DATA_DESC(SurfaceHint)
		BASE_SCENE_ENTITY_PROP(SurfaceHint)

		ASSET_TEXTURE_PROP(SurfaceHint, hintUp, "Keyboard", "TextureUp")
		ASSET_TEXTURE_PROP(SurfaceHint, hintLeft, "Keyboard", "TextureLeft")
		ASSET_TEXTURE_PROP(SurfaceHint, hintDown, "Keyboard", "TextureDown")
		ASSET_TEXTURE_PROP(SurfaceHint, hintRight, "Keyboard", "TextureRight")

		ASSET_TEXTURE_PROP(SurfaceHint, hintUpGP, "Gamepad", "TextureUpGP")
		ASSET_TEXTURE_PROP(SurfaceHint, hintLeftGP, "Gamepad", "TextureLeftGP")
		ASSET_TEXTURE_PROP(SurfaceHint, hintDownGP, "Gamepad", "TextureDownGP")
		ASSET_TEXTURE_PROP(SurfaceHint, hintRightGP, "Gamepad", "TextureRightGP")

	META_DATA_DESC_END()

	ORIN_EVENTS(SurfaceHint)
		ORIN_EVENT(SurfaceHint, CmdShowSurfaceHint, OnShowHint)
	ORIN_EVENTS_END(SurfaceHint)

	void SurfaceHint::Init()
	{
		ScriptEntity2D::Init();

		Tasks(true)->AddTask(10, this, (Object::Delegate)&SurfaceHint::Draw);
	}

	void SurfaceHint::Play()
	{
		ScriptEntity2D::Play();

		ORIN_EVENTS_SUBSCRIBE(SurfaceHint);

		textures[0] = hintUp;
		textures[1] = hintLeft;
		textures[2] = hintDown;
		textures[3] = hintRight;

		textures[4] = hintUpGP;
		textures[5] = hintLeftGP;
		textures[6] = hintDownGP;
		textures[7] = hintRightGP;
	}

	void SurfaceHint::OnShowHint(const CmdShowSurfaceHint& evt)
	{
		//needToDraw = true;
		hint = evt.hint;
		pos = evt.pos;
	}

	void SurfaceHint::Draw(float dt)
	{
		if (needToDraw)
		{
			Transform trans;
			trans.objectType = ObjectType::Object2D;
			trans.offset = 0.5f;
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