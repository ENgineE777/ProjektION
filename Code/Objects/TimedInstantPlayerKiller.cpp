
#include "TimedInstantPlayerKiller.h"
#include "Root/Root.h"
#include "World/WorldManager.h"
#include "SceneEntities/2D/GenericMarker2D.h"

namespace Orin::Overkill
{
	ENTITYREG(SceneEntity, TimedInstantPlayerKiller, "Overkill/Objects", "TimedInstantPlayerKiller")

	META_DATA_DESC(TimedInstantPlayerKiller)
		BASE_SCENE_ENTITY_PROP(TimedInstantPlayerKiller)
		COLOR_PROP(TimedInstantPlayerKiller, color, COLOR_YELLOW_A(0.5f), "Geometry", "color")
		MARK_DISABLED_FOR_INSTANCE()

		BOOL_PROP(TimedInstantPlayerKiller, startAsActive, true, "Params", "startAsActive", "Should start as active")

		FLOAT_PROP(TimedInstantPlayerKiller, timeActive, 0.6f, "Params", "timeActive", "Time being active")
		MARK_DISABLED_FOR_INSTANCE()

		FLOAT_PROP(TimedInstantPlayerKiller, timeInactive, 0.6f, "Params", "timeInactive", "Time being inactive")
		MARK_DISABLED_FOR_INSTANCE()

		FLOAT_PROP(TimedInstantPlayerKiller, preActivationTime, 0.3f, "Params", "preActivationTime", "How much show pre activation before activation")
		MARK_DISABLED_FOR_INSTANCE()

		FLOAT_PROP(TimedInstantPlayerKiller, startTimerOffset, 0.0f, "Params", "startTimerOffset", "Left Angle")

		FLOAT_OPTIONAL_PROP(TimedInstantPlayerKiller, optTimeActive, 0.6f, "Params", "overrideTimeActive", "override of timeActive")
		FLOAT_OPTIONAL_PROP(TimedInstantPlayerKiller, optTimeInactive, 0.6f, "Params", "overrideTimeInactive", "override of timeInactive")
		FLOAT_OPTIONAL_PROP(TimedInstantPlayerKiller, optPreActivationTime, 0.3f, "Params", "overridePreActivationTime", "override of PreActivationTime")

	META_DATA_DESC_END()

	ORIN_EVENTS(TimedInstantPlayerKiller)
		ORIN_EVENT(TimedInstantPlayerKiller, EventOnRestart, OnRestart)
	ORIN_EVENTS_END(TimedInstantPlayerKiller)

	void TimedInstantPlayerKiller::Init()
	{
		PhysTriger2D::Init();

		visibleDuringPlay = false;

		transform.offset = 0.0f;

		if (prefabInstance)
		{
			transform.transformFlag = MoveXYZ | RectMoveXY | TransformFlag::RotateZ;
		}

		physGroup = PhysGroup::DeathZone;

		Tasks(true)->AddTask(10, this, (Object::Delegate)&TimedInstantPlayerKiller::Update);
	}

	void TimedInstantPlayerKiller::SetActive(bool setActive)
	{
		active = setActive;

		spikes->anim.GotoNode(active ? "Activation" : "Deactivation", false);
		body.body->SetActive(active);
	}

	void TimedInstantPlayerKiller::Play()
	{
		PhysTriger2D::Play();

		spikes = FindChild<AnimGraph2D>("Spikes");

		ORIN_EVENTS_SUBSCRIBE(TimedInstantPlayerKiller);
	}

	void TimedInstantPlayerKiller::Release()
	{
		Tasks(true)->DelTask(10, this);

		MainEventsQueue::Unsubscribe(this);

		PhysTriger2D::Release();
	}

	void TimedInstantPlayerKiller::OnRestart(const EventOnRestart& evt)
	{
		timer = startTimerOffset;
		SetActive(startAsActive);
	}

	void TimedInstantPlayerKiller::OnContactStart(int index, SceneEntity* entity, int contactIndex)
	{
		if (active && dynamic_cast<Player*>(entity->GetParent()))
		{
			auto mat = transform.GetGlobal();
			auto dir = WorldManager::instance->GetPlayerPos() - transform.position - mat.MulNormal(transform.size * Math::Vector3(0.5f, -0.5f, 0.0f));
			dir.Normalize();

			WorldManager::instance->KillPlayer(dir, DeathSource::Fun);
		}
	}

	void TimedInstantPlayerKiller::Update(float dt)
	{
		if (!GetScene()->IsPlaying())
		{
			color.a = startAsActive ? 0.35f : 0.1f;

			return;
		}

		timer += dt;

		float limit = active ? (optTimeActive.enabled ? optTimeActive.value : timeActive) : (optTimeInactive.enabled ? optTimeInactive.value : timeInactive);

		if (!active && limit - timer < (optPreActivationTime.enabled ? optPreActivationTime.value : preActivationTime))
		{
			spikes->anim.GotoNode("PreActivation", false);
		}

		if (timer > limit)
		{
			timer -= limit;

			SetActive(!active);
		}
	}
}