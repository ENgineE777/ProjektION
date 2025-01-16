#pragma once

#include "VFXEmitter.h"

namespace Orin::Overkill
{
	class VFXEmitterInstantiable : public VFXEmitter
	{
        struct Instance
        {
            Math::Matrix m;
            Math::Vector3 velocity;

            float startTime;

            eastl::vector<Emitter::State> emitterStates;

            bool isStarted = false;
        };

        eastl::ring_buffer<Instance> instances;
        int maxInstancesCount = 128;
        
    public:
        META_DATA_DECL(VFXEmitterInstantiable);

        void ApplyProperties() override;
        void Init() override;
        void Release() override;
		void Play() override;

        void UpdateEmitters(float dt) override;
        void EditorDraw(float dt) override;

        void ClearInstances();
        int GetInstancesCount() const;
        int AddInstance(Math::Matrix &m);
        void UpdateInstance(int id, float dt, Math::Matrix &m);
        void StartInstance(int id);
        void StopInstance(int id);
    };
}