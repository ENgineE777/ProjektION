
#pragma once

#include "SceneEntities/2D/ScriptEntity2D.h"
#include "SceneEntities/2D/AnimGraph2D.h"
#include "SceneEntities/2D/SpriteEntity.h"
#include "SceneEntities/Physics/2D/KinematicCapsule2D.h"
#include "SceneEntities/2D/Node2D.h"
#include "Common/Events.h"
#include "Common/Constants.h"

namespace Orin::Overkill
{
	class ThunderCharge;

	enum class HitResult
	{
		Nothing,
		Effective,
		Lethal
	};

	enum class DeathSource
	{
		Unknown,
		Elecro,
		Laser,
		Fun,
		InnerDash,
		Bullet
	};

	class Character : public ScriptEntity2D
	{
	protected:
		AnimGraph2D* anim = nullptr;
		KinematicCapsule2D* controller = nullptr;
		Node2D* bodyParts = nullptr;
		eastl::vector<Math::Matrix> bodyPartsMats;
		Math::Vector3 initPos;
		Math::Vector3 initRotation;
		// TODO: Remove
		Math::Vector3 initScale;
		Math::Vector3 checkpointPos;
		bool flipped = false;
		bool flippedV = false;
		bool killed = false;
		bool isCheckpointActive = false;

		float health = 1.f;
		float maxHealth = 1.f;

		inline float GetDirection() const
		{
			return flipped ? -1.0f : 1.0f;
		}

		AnimGraph2D* explotion = nullptr;

		virtual void UpdateAnimScale();

		void SnapOnSurface();
		
	public:

		void Init() override;
		void ApplyProperties() override;
		void Play() override;
		void Release() override;

		virtual const char* GetBodyPartsTrail();

		virtual bool IsKilled();
		virtual HitResult Hit(Math::Vector3 hitDir, float damage);
		virtual void Kill(Math::Vector3 killDir, DeathSource deathSource);
		virtual void OnRestart(const EventOnRestart &evt);
		virtual void OnCheckpointReached(const EventOnCheckpointReached &evt) {}
		virtual void OnFinishLevel(const EventOnFinishLevel &evt) {}
		virtual void OnThunderHit(Math::Vector3 hitDir, ThunderCharge *source) {}
		virtual bool IsImmortal() { return false; }
	};
}
