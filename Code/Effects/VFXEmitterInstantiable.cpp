#include "Effects/VFXEmitterInstantiable.h"
#include "World/WorldManager.h"

namespace Orin::Overkill
{
	ENTITYREG(SceneEntity, VFXEmitterInstantiable, "Overkill/Effects", "VFXEmitterInstantiable")

	META_DATA_DESC(VFXEmitterInstantiable)
		BASE_SCENE_ENTITY_PROP(VFXEmitterInstantiable)

		INT_PROP(VFXEmitterInstantiable, drawLevel, 0, "Visual", "draw_level", "Draw priority")
		MARK_DISABLED_FOR_INSTANCE()

		BOOL_PROP(VFXEmitterInstantiable, noZ, false, "Visual", "noZ", "no use Z during render")
		MARK_DISABLED_FOR_INSTANCE()

		BOOL_PROP(VFXEmitterInstantiable, isLooped, false, "VFX", "isLooped", "isLooped")
		MARK_DISABLED_FOR_INSTANCE()

		INT_PROP(VFXEmitterInstantiable, maxInstancesCount, 128, "VFX", "maxInstancesCount", "Max instances count")
		MARK_DISABLED_FOR_INSTANCE()

        ARRAY_PROP(VFXEmitterInstantiable, emitters, Emitter, "VFX", "emitters")
		MARK_DISABLED_FOR_INSTANCE()
	META_DATA_DESC_END()

	void VFXEmitterInstantiable::ApplyProperties()
	{
		VFXEmitter::ApplyProperties();

		for (auto &i : instances)
		{
			for (auto& state : i.emitterStates)
			{
				state.isDone = false;
				state.nextEmitAtTime = 0.0f;
			}
		}
	}

    void VFXEmitterInstantiable::Init()
    {
        VFXEmitter::Init();

		instances.reserve(maxInstancesCount);
    }

    void VFXEmitterInstantiable::Release()
    {
        VFXEmitter::Release();
    }

    void VFXEmitterInstantiable::Play()
    {
        VFXEmitter::Play();

		ClearInstances();

		instances.reserve(maxInstancesCount);
    }

	void VFXEmitterInstantiable::UpdateEmitters(float dt)
	{
		const bool isPlaying = GetScene()->IsPlaying();

		bool allDone = true;

		for (auto &i : instances)
		{
			if (!i.isStarted)
			{
				continue;
			}

			auto pos      = Sprite::ToPixels(i.m.Pos());
			auto rotation = Math::SafeAtan2(i.m.Vx().y, i.m.Vx().x);
			
			const float instanceTime = totalTime - i.startTime;

			int emitterIndex = 0;
			for (const auto &emitter : emitters)
			{
				if (!emitter.enabled)
				{
					continue;
				}

				auto &emitterState = i.emitterStates[emitterIndex++];

				if (!isLooped && !emitterState.isDone && emitter.timeLife > 0.f && instanceTime >= emitter.timeLife)
				{
					emitterState.isDone = true;
				}

				allDone &= emitterState.isDone;

				if (!emitterState.isDone && instanceTime >= emitterState.nextEmitAtTime && Math::IsNonZero(emitter.emitsFreq))
				{
					emitterState.nextEmitAtTime = instanceTime + Utils::SafeInvert(emitter.emitsFreq);
					SpawnParticles(emitter, pos, rotation, i.velocity);
				}
			}
		}

		if (allDone && particles.empty() && !isPlaying)
		{
			totalTime = 0.0f;

			for (auto &i : instances)
			{
				for (auto& state : i.emitterStates)
				{
					state.isDone = false;
					state.nextEmitAtTime = 0.0f;
				}
			}
		}
	}

	void VFXEmitterInstantiable::EditorDraw(float dt)
	{
		if (GetScene()->IsPlaying() || !IsVisible())
		{
			return;
		}

		if (instances.empty())
		{
			AddInstance(transform.GetGlobal());
			return;
		}

		instances.back().m = transform.GetGlobal();

		if (instances.back().emitterStates.size() != emitters.size())
		{
			instances.back().emitterStates.resize(emitters.size());
		}
        
		VFXEmitter::EditorDraw(dt);
	}

	void VFXEmitterInstantiable::ClearInstances()
    {
        instances.clear();
    }

    int VFXEmitterInstantiable::GetInstancesCount() const
    {
        return (int)instances.size();
    }

    int VFXEmitterInstantiable::AddInstance(Math::Matrix &m)
    {
        auto &i = instances.push_back();
		i.m = m;
		i.startTime = totalTime;
		i.isStarted = true;

		i.emitterStates.resize(emitters.size());

		return (int)instances.size() - 1;
    }

    void VFXEmitterInstantiable::UpdateInstance(int id, float dt, Math::Matrix &m)
    {
		instances[id].velocity = (m.Pos() - instances[id].m.Pos()) / dt;
        instances[id].m = m;
    }

	void VFXEmitterInstantiable::StartInstance(int id)
	{
		if (instances[id].isStarted)
		{
			return;
		}

		instances[id].isStarted = true;
		instances[id].startTime = totalTime;
		for (auto& state : instances[id].emitterStates)
		{
			state.isDone = false;
			state.nextEmitAtTime = 0.0f;
		}
	}

	void VFXEmitterInstantiable::StopInstance(int id)
	{
		instances[id].isStarted = false;
	}
}