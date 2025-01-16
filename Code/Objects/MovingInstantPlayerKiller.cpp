
#include "MovingInstantPlayerKiller.h"
#include "Root/Root.h"
#include "World/WorldManager.h"
#include "SceneEntities/2D/GenericMarker2D.h"

namespace Orin::Overkill
{
	ENTITYREG(SceneEntity, MovingInstantPlayerKiller, "Overkill/Objects", "MovingInstantPlayerKiller")

	META_DATA_DESC(MovingInstantPlayerKiller)
		BASE_SCENE_ENTITY_PROP(MovingInstantPlayerKiller)
		COLOR_PROP(MovingInstantPlayerKiller, color, COLOR_YELLOW_A(0.5f), "Geometry", "color")
		MARK_DISABLED_FOR_INSTANCE()

		FLOAT_PROP(MovingInstantPlayerKiller, timeInactive, 0.6f, "Params", "timeInactive", "Time being inactive")
		MARK_DISABLED_FOR_INSTANCE()

		FLOAT_PROP(MovingInstantPlayerKiller, preActivationTime, 0.3f, "Params", "preActivationTime", "How much show pre activation before activation")
		MARK_DISABLED_FOR_INSTANCE()

		FLOAT_PROP(MovingInstantPlayerKiller, moveSpeed, 128.0f, "Params", "moveSpeed", "Speed of movement")
		MARK_DISABLED_FOR_INSTANCE()

		FLOAT_OPTIONAL_PROP(MovingInstantPlayerKiller, optTimeInactive, 0.6f, "Params", "overrideTimeInactive", "override of TimeInActive")
		FLOAT_OPTIONAL_PROP(MovingInstantPlayerKiller, optPreActivationTime, 0.3f, "Params", "overridePreActivationTime", "override of PreActivationTime")
		FLOAT_OPTIONAL_PROP(MovingInstantPlayerKiller, optMoveSpeed, 300.f * TILE_SCALE, "Params", "overrideMoveSpeed", "override of moveSpeed")

	META_DATA_DESC_END()

	void MovingInstantPlayerKiller::Play()
	{
		physGroup = PhysGroup::DeathZone;

		TimedInstantPlayerKiller::Play();

		GenericMarker2D* path = FindChild<GenericMarker2D>();

		if (path && path->instances.size() >= 2)
		{
			pathSet = true;

			fromPos  = path->instances[0].GetPosition();
			toPos = path->instances[1].GetPosition();		

			if (path->instances.size() > 2)
			{
				startPos = path->instances[2].GetPosition();
				hasStartPoint = true;

				if (Math::IsPointInLineSegment(startPos, fromPos, toPos))
				{
					pathSet = true;
				}
				else
				if (Math::IsPointInLineSegment(toPos, fromPos, startPos))
				{
					eastl::swap(toPos, startPos);
					pathSet = true;
				}
				else
				if (Math::IsPointInLineSegment(fromPos, toPos, startPos))
				{
					eastl::swap(fromPos, startPos);
					eastl::swap(fromPos, toPos);
					pathSet = true;
				}
				else
				{
					pathSet = false;
				}				
			}

			moveDir = toPos - fromPos;
			moveDir.Normalize();
		}
	}

	void MovingInstantPlayerKiller::OnRestart(const EventOnRestart& evt)
	{
		timer = 0.0f;

		if (hasStartPoint)
		{
			state = State::Moving;
			SetPosition(startPos);
			spikes->anim.GotoNode("Active", false);
		}
		else
		{
			state = State::PreActivation;
			SetPosition(fromPos);
		}
	}

	void MovingInstantPlayerKiller::SetPosition(Math::Vector3 pos)
	{
		Math::Matrix mat;
		mat.RotateZ(-transform.rotation.z * Math::Radian);
		mat.Pos() = Sprite::ToUnits(pos);

		transform.SetGlobal(mat);

		auto offset = mat.MulNormal((transform.offset - Math::Vector3{ 0.5f, 0.5f, 0.f }) * transform.size);

		mat.Pos() = mat.Pos() + Sprite::ToUnits(offset);
		body.body->SetTransform(mat);
	}

	void MovingInstantPlayerKiller::Update(float dt)
	{
		if (!GetScene()->IsPlaying())
		{
			color.a = 0.35f;

			return;
		}

		if (!pathSet)
		{
			if (hasStartPoint)
			{
				Sprite::DebugSphere(fromPos, 25.0f, COLOR_WHITE);
				Sprite::DebugSphere(toPos, 25.0f, COLOR_WHITE);
				Sprite::DebugSphere(startPos, 25.0f, COLOR_RED);
			}

			return;
		}

		switch (state)
		{
		case (State::PreActivation):
			{
				timer += dt;

				spikes->anim.GotoNode("PreActivation", false);

				if (timer > (optPreActivationTime.enabled ? optPreActivationTime.value : preActivationTime))
				{
					state = State::Moving;
					SetActive(true);
				}

				break;
			}
		case (State::Moving):
			{		
				auto moveOffset = moveDir * dt * (optMoveSpeed.enabled ? optMoveSpeed.value : moveSpeed);
				auto pos = Sprite::ToPixels(transform.GetGlobal().Pos());

				if (moveOffset.Length2() > (toPos - pos).Length2())
				{
					SetPosition(toPos);

					state = State::Waiting;
					timer = 0.0f;
					SetActive(false);
				}
				else
				{
					SetPosition(pos + moveOffset);

					return;
				}

				break;
			}
		case (State::Waiting):		
			{
				timer += dt;

				float limit = fmax(optTimeInactive.enabled ? optTimeInactive.value : timeInactive, 0.18f);

				if (timer > limit)
				{
					timer -= limit;

					state = State::PreActivation;

					SetPosition(fromPos);
				}

				break;
			}
		}
	}
}