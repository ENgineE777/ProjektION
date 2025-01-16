#pragma once

#include "SceneEntities/2D/ScriptEntity2D.h"
#include "SceneEntities/2D/Camera2D.h"
#include "Common/Utils.h"
#include "Common/Events.h"

#include "EASTL/bonus/ring_buffer.h"

namespace Orin::Overkill
{
    class BloodEffect : public ScriptEntity2D
	{
        friend class BloodManager;

    protected:
        struct Instance
        {
            Math::Matrix m;
            float angle;
        };

        eastl::ring_buffer<Instance> instances;

        TextureRef noiseRef;
        TextureRef GetNoiseTexture();

    public:

        static TextureRef CreateNoiseTexture();

        Math::Vector3 localOffset;

        void Init() override;
        void Release() override;
		void Play() override;

        void ClearInstances();
        int GetInstancesCount() const;
        void AddInstance(Math::Matrix &m);
    };
}