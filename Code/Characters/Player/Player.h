
#pragma once

#include "SceneEntities/2D/ScriptEntity2D.h"
#include "SceneEntities/2D/AnimGraph2D.h"
#include "SceneEntities/Physics/2D/KinematicCapsule2D.h"
#include "SceneEntities/2D/TileMap.h"
#include "Characters/Character.h"
#include "Objects/Throwable/Throwable.h"

namespace Orin::Overkill
{
	class Player : public Character
	{
	protected:

		bool levelFinished = false;

		Math::Vector3 lastMoveVel;

	public:

		void Play() override;
		void OnRestart(const EventOnRestart& evt) override;
		void OnCheckpointReached(const EventOnCheckpointReached &evt) override;
		virtual void OnFinishLevel(const EventOnFinishLevel& evt);

		void Kill(Math::Vector3 killDir, DeathSource deathSource) override;
		virtual Math::Vector3 GetUp() const = 0;
		virtual Math::Vector3 GetForward() const = 0;
		virtual Math::Vector3 GetVelocity() const { return lastMoveVel; }
		virtual bool HasPuppet() { return false; };
		virtual bool CanReflectProjectile() { return false; };
		virtual void SetPuppet(class Puppet* setPuppet) {};
		virtual bool IsTutorialHintAllowed() { return true; };
		virtual void OnThrowable2Pickup(ThrowableDesc* throwable, Math::Vector3 standPos, bool autoGrab) {};
		virtual bool CanGrabThrowable() { return false; };
		virtual void MakeInvincibile(float time) {};
		void Release() override;
	};
}
