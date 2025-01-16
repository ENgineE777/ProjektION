#include "Effects/BloodEffect.h"
#include "World/WorldManager.h"

namespace Orin::Overkill
{
    void BloodEffect::Init()
    {
        ScriptEntity2D::Init();

        instances.reserve(32);

        transform.objectType = ObjectType::Object2D;
		transform.transformFlag = TransformFlag::MoveXYZ | TransformFlag::SizeX | TransformFlag::SizeY | TransformFlag::RotateZ;
    }

    TextureRef BloodEffect::CreateNoiseTexture()
    {
        TextureRef noiseRef = GetRoot()->GetRender()->GetDevice()->CreateTexture(256, 256, TextureFormat::FMT_A8R8G8B8, 0, false, TextureType::Tex2D, _FL_);

		uint32_t texData[256 * 256];
		memset(texData, 0, sizeof(texData));

		Utils::RndGenerator rnd;

		for (int y = 0; y < 256; ++y)
		{
			for (int x = 0; x < 256; ++x)
			{
				texData[y * 256 + x] = Color(rnd.NextFloat(), rnd.NextFloat(), rnd.NextFloat(), rnd.NextFloat()).Get();
			}
		}

		noiseRef->Update(0, 0, (uint8_t*)texData, 256 * sizeof(uint32_t));
		noiseRef->SetFilters(TextureFilter::Point, TextureFilter::Point);

        return noiseRef;
    }

    TextureRef BloodEffect::GetNoiseTexture()
    {
        if (!noiseRef)
        {
            noiseRef = CreateNoiseTexture();
        }

        return noiseRef;
    }

    void BloodEffect::Release()
    {
        ScriptEntity2D::Release();
    }

    void BloodEffect::Play()
    {
        ScriptEntity2D::Play();

        localOffset = transform.position;
    }

    void BloodEffect::ClearInstances()
    {
        instances.clear();
    }

    int BloodEffect::GetInstancesCount() const
    {
        return (int)instances.size();
    }

    void BloodEffect::AddInstance(Math::Matrix &m)
    {
        instances.push_back({m, -m.GetRotation().z});
    }
}