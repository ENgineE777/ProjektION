#include "Effects/BloodPuddle.h"
#include "World/WorldManager.h"

namespace Orin::Overkill
{
	ENTITYREG(SceneEntity, BloodPuddle, "Overkill/Effects", "BloodPuddle")

    META_DATA_DESC(BloodPuddle::Texture)
		ASSET_TEXTURE_PROP(BloodPuddle::Texture, texture, "Texture", "texture")
	META_DATA_DESC_END()

	META_DATA_DESC(BloodPuddle)
		BASE_SCENE_ENTITY_PROP(BloodPuddle)
        ARRAY_PROP(BloodPuddle, textures, Texture, "Puddle", "textures")
	META_DATA_DESC_END()

    void BloodPuddle::Init()
    {
        BloodEffect::Init();

        transform.objectType = ObjectType::Object2D;
		transform.transformFlag = TransformFlag::MoveXYZ | TransformFlag::RotateZ;

        Tasks(true)->AddTask(1, this, (Object::Delegate)&BloodPuddle::Draw);
    }

    void BloodPuddle::Release()
    {
        Tasks(true)->DelTask(1, this);

        BloodEffect::Release();
    }

    void BloodPuddle::Play()
    {
        BloodEffect::Play();
    }

    void BloodPuddle::Update(float dt)
    {
    }

    void BloodPuddle::Draw(float dt)
    {
        if (!IsVisible() || !GetScene()->IsPlaying() || !WorldManager::instance)
		{
			return;
		}

        if (!textures.empty())
        {
            if (textureForInstances.size() != instances.size())
            {
                const int from = (int)textureForInstances.size();
                textureForInstances.resize(instances.size());

                for (int i = from, sz = (int)textureForInstances.size(); i < sz; ++i)
                {
                    textureForInstances[i] = textures[int(rndGenerator.NextUInt() % uint64_t(textures.size()))].texture;
                }
            }
        }

        Transform trans = transform;

        int instanceIdx = 0;
        for (auto &i : instances)
        {
            auto tex = textureForInstances[instanceIdx++];

            trans.size     = tex.GetSize();
            trans.offset   = {0.5f, 0.5f, 0.f};
            trans.position = Sprite::ToPixels(i.m.Pos());

            tex.prg = Sprite::quadPrgNoZ;
            tex.Draw(&trans, COLOR_WHITE, dt);
        }
    }
}