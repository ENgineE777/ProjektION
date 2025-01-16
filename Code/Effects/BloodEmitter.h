#pragma once

#include "SceneEntities/2D/ScriptEntity2D.h"
#include "SceneEntities/2D/Camera2D.h"
#include "Common/Utils.h"
#include "Common/Events.h"
#include "World/BloodManager.h"
#include "Effects/VFXEmitter.h"

#include "EASTL/array.h"

namespace Orin::Overkill
{
	class BloodEmitter : public VFXEmitter
	{
        eastl::string smallPuddle;
        AssetPrefabRef smallPuddlePrefabRef;

        float addSmallPuddleProbability;
        float addPuddleProbability;

    public:
        META_DATA_DECL(BloodEmitter);

        void ApplyProperties() override;
        void Init() override;
        void Release() override;
		void Play() override;

        void DoUpdateParticles(float dt) override;
    };
}