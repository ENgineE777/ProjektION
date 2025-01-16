#include "Effects/BloodEmitter.h"
#include "Effects/BloodEffect.h"
#include "World/WorldManager.h"

namespace Orin::Overkill
{
	ENTITYREG(SceneEntity, BloodEmitter, "Overkill/Effects", "BloodEmitter")

	META_DATA_DESC(BloodEmitter)
		BASE_SCENE_ENTITY_PROP(BloodEmitter)

        FILENAME_PROP(BloodEmitter, smallPuddle, "", "Blood", "smallPuddle")
		MARK_DISABLED_FOR_INSTANCE()

        FLOAT_PROP(BloodEmitter, addSmallPuddleProbability, 0.05f, "Blood", "addSmallPuddleProbability", "addSmallPuddleProbability")
		MARK_DISABLED_FOR_INSTANCE()

        FLOAT_PROP(BloodEmitter, addPuddleProbability, 0.05f, "Blood", "addPuddleProbability", "addPuddleProbability")
		MARK_DISABLED_FOR_INSTANCE()

		INT_PROP(BloodEmitter, drawLevel, 0, "Visual", "draw_level", "Draw priority")
		MARK_DISABLED_FOR_INSTANCE()

        ARRAY_PROP(BloodEmitter, emitters, Emitter, "VFX", "emitters")
		MARK_DISABLED_FOR_INSTANCE()
	META_DATA_DESC_END()

	void BloodEmitter::ApplyProperties()
	{
		VFXEmitter::ApplyProperties();

		smallPuddlePrefabRef = GetRoot()->GetAssets()->GetAssetRef<AssetPrefabRef>(smallPuddle);	
	}

    void BloodEmitter::Init()
    {
        VFXEmitter::Init();
    }

    void BloodEmitter::Release()
    {
        VFXEmitter::Release();
    }

    void BloodEmitter::Play()
    {
        VFXEmitter::Play();
    }

    void BloodEmitter::DoUpdateParticles(float dt)
	{
		VFXEmitter::DoUpdateParticles(dt);

		if (GetScene()->IsPlaying())
		{
			if (BloodManager *bloodManager = WorldManager::instance ? WorldManager::instance->GetBlood() : nullptr)
			{
				for (auto &p : particles)
				{
					if (p.ttl <= 0.f && addPuddleProbability > 0.f && rndGenerator.NextFloat() < addPuddleProbability)
					{
						bloodManager->AddTileBlood(p.pos, p.lastBounceNormal);
					}
					else if (p.ttl <= 0.f && addSmallPuddleProbability > 0.f && rndGenerator.NextFloat() < addSmallPuddleProbability)
					{
						auto *entity = bloodManager->SpawnUnique(smallPuddlePrefabRef, p.pos, 0.f);
						if (BloodEffect *effect = dynamic_cast<BloodEffect*>(entity))
						{
							Math::Matrix m;
							m.Pos() = Sprite::ToUnits(p.pos);
							effect->AddInstance(m);
						}
					}
				}
			}
		}
	}
}