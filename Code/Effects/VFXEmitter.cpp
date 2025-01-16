#include "Effects/VFXEmitter.h"
#include "World/WorldManager.h"

namespace Orin::Overkill
{
	ENTITYREG(SceneEntity, VFXEmitter, "Overkill/Effects", "VFXEmitter")

	META_DATA_DESC(VFXEmitter::Texture)
		ASSET_TEXTURE_PROP(VFXEmitter::Texture, texture, "Texture", "texture")
		VECTOR2_PROP(VFXEmitter::Texture, scale, Math::Vector2(1.f, 1.f), "Texture", "scale")
		VECTOR2_PROP(VFXEmitter::Texture, offset, Math::Vector2(0.5f, 0.5f), "Texture", "offset")
		INT_PROP(VFXEmitter::Texture, weight, 1, "Texture", "weight", "weight")
	META_DATA_DESC_END()

    META_DATA_DESC(VFXEmitter::Emitter)
        BOOL_PROP(VFXEmitter::Emitter, enabled, true, "Emitter", "enabled", "enabled")
        BOOL_PROP(VFXEmitter::Emitter, matchEntityRotation, false, "Emitter", "matchEntityRotation", "matchEntityRotation")
        BOOL_PROP(VFXEmitter::Emitter, matchVelocityRotation, false, "Emitter", "matchVelocityRotation", "matchVelocityRotation")
        BOOL_PROP(VFXEmitter::Emitter, useParticlePrg, false, "Emitter", "useParticlePrg", "useParticlePrg")
		ARRAY_PROP(VFXEmitter::Emitter, textures, Texture, "Emitter", "textures")
        COLOR_PROP(VFXEmitter::Emitter, color, COLOR_WHITE, "Emitter", "color")
        VECTOR2_PROP(VFXEmitter::Emitter, velocity, Math::Vector2(0.f, 0.f), "Emitter", "velocity")
        VECTOR2_PROP(VFXEmitter::Emitter, omega, Math::Vector2(0.f, 0.f), "Emitter", "omega")
        VECTOR2_PROP(VFXEmitter::Emitter, sector, Math::Vector2(0.f, 0.f), "Emitter", "sector")
        VECTOR2_PROP(VFXEmitter::Emitter, distance, Math::Vector2(0.f, 0.f), "Emitter", "distance")
        VECTOR2_PROP(VFXEmitter::Emitter, fadeIn, Math::Vector2(0.f, 0.f), "Emitter", "fadeIn")
        VECTOR2_PROP(VFXEmitter::Emitter, fadeOut, Math::Vector2(0.7f, 0.9f), "Emitter", "fadeOut")
        VECTOR2_PROP(VFXEmitter::Emitter, scaleIn, Math::Vector2(0.f, 0.f), "Emitter", "scaleIn")
        VECTOR2_PROP(VFXEmitter::Emitter, scaleOut, Math::Vector2(0.f, 0.f), "Emitter", "scaleOut")
        VECTOR2_PROP(VFXEmitter::Emitter, scale, Math::Vector2(1.f, 1.f), "Emitter", "scale")
        VECTOR2_PROP(VFXEmitter::Emitter, particlesCount, Math::Vector2(0.f, 0.f), "Emitter", "particlesCount")
        VECTOR2_PROP(VFXEmitter::Emitter, radius, Math::Vector2(0.f, 0.f), "Emitter", "radius")
        VECTOR2_PROP(VFXEmitter::Emitter, spawnX, Math::Vector2(0.f, 0.f), "Emitter", "spawnX")
        VECTOR2_PROP(VFXEmitter::Emitter, spawnY, Math::Vector2(0.f, 0.f), "Emitter", "spawnY")
        VECTOR2_PROP(VFXEmitter::Emitter, angle, Math::Vector2(0.f, 0.f), "Emitter", "angle")
        VECTOR2_PROP(VFXEmitter::Emitter, bounce, Math::Vector2(0.f, 0.f), "Emitter", "bounce")
        VECTOR2_PROP(VFXEmitter::Emitter, bounceDeviation, Math::Vector2(0.f, 0.f), "Emitter", "bounceDeviation")
        VECTOR2_PROP(VFXEmitter::Emitter, turbulenceX, Math::Vector2(0.f, 0.f), "Emitter", "turbulenceX")
        VECTOR2_PROP(VFXEmitter::Emitter, turbulenceY, Math::Vector2(0.f, 0.f), "Emitter", "turbulenceY")
        FLOAT_PROP(VFXEmitter::Emitter, emitsFreq, 1.f, "Emitter", "emitsFreq", "emitsFreq")    
		FLOAT_PROP(VFXEmitter::Emitter, timeLife, -1.f, "Emitter", "timeLife", "timeLife")
		FLOAT_PROP(VFXEmitter::Emitter, gravity, 0.f, "Emitter", "gravity", "gravity")
		FLOAT_PROP(VFXEmitter::Emitter, affectByPlayerDist, -1.f, "Emitter", "affectByPlayerDist", "affectByPlayerDist")
		FLOAT_PROP(VFXEmitter::Emitter, affectByPlayerFactor, 0.f, "Emitter", "affectByPlayerFactor", "affectByPlayerFactor")
		FLOAT_PROP(VFXEmitter::Emitter, affectByEmitterVelocity, 0.f, "Emitter", "affectByEmitterVelocity", "affectByEmitterVelocity")
		FLOAT_PROP(VFXEmitter::Emitter, noisePersistence, 0.5f, "Emitter", "noisePersistence", "noisePersistence")
		INT_PROP(VFXEmitter::Emitter, noiseOctaves, 0, "Emitter", "noiseOctaves", "noiseOctaves")
    META_DATA_DESC_END()

	META_DATA_DESC(VFXEmitter)
		BASE_SCENE_ENTITY_PROP(VFXEmitter)

		INT_PROP(VFXEmitter, drawLevel, 0, "Visual", "draw_level", "Draw priority")
		MARK_DISABLED_FOR_INSTANCE()

		BOOL_PROP(VFXEmitter, noZ, true, "Visual", "noZ", "no use Z during render")
		MARK_DISABLED_FOR_INSTANCE()

		BOOL_PROP(VFXEmitter, isLooped, false, "VFX", "isLooped", "isLooped")
		MARK_DISABLED_FOR_INSTANCE()

		BOOL_PROP(VFXEmitter, isStarted, true, "VFX", "isStarted", "isStarted")

        ARRAY_PROP(VFXEmitter, emitters, Emitter, "VFX", "emitters")
		MARK_DISABLED_FOR_INSTANCE()
	META_DATA_DESC_END()

	void VFXEmitter::InitEmitterTextures()
	{
		for (auto &e : emitters)
		{
			e.textureIndices.clear();

			int textureIndex = 0;
			for (auto &t : e.textures)
			{
				for (int i = 0; i < t.weight; ++i)
				{
					e.textureIndices.push_back(textureIndex);
				}
				++textureIndex;
			}
		}
	}

	void VFXEmitter::ApplyProperties()
	{
		totalTime = 0.f;

		for (auto& emitter : emitters)
		{
			emitter.state.isDone = false;
			emitter.state.nextEmitAtTime = 0.0f;
		}

		InitEmitterTextures();

#ifdef ORIN_EDITOR
		Tasks(true)->DelAllTasks(this);
#endif

		Tasks(true)->AddTask(0, this, (Object::Delegate)&VFXEmitter::EditorDraw);
        Tasks(true)->AddTask(0 + drawLevel, this, (Object::Delegate)&VFXEmitter::Draw);
	}

    void VFXEmitter::Init()
    {
        ScriptEntity2D::Init();

        transform.objectType = ObjectType::Object2D;
		transform.transformFlag = TransformFlag::MoveXYZ | TransformFlag::RotateZ | TransformFlag::ScaleX;

		particlePrg = GetRoot()->GetRender()->GetRenderTechnique<ParticleRenderTechnique>(_FL_);

        Tasks(true)->AddTask(0, this, (Object::Delegate)&VFXEmitter::EditorDraw);
        Tasks(true)->AddTask(0 + drawLevel, this, (Object::Delegate)&VFXEmitter::Draw);
    }

    void VFXEmitter::Release()
    {
        Tasks(true)->DelTask(0 + drawLevel, this);
        Tasks(true)->DelTask(0, this);

        ScriptEntity2D::Release();
    }

    void VFXEmitter::Play()
    {
        ScriptEntity2D::Play();

		InitEmitterTextures();

		noiseGenerator.reseed(rndGenerator.NextUInt());
    }

    void VFXEmitter::Update(float dt)
    {
		if (AnimGraph2D::pause)
		{
			return;
		}

		dt *= dtScale;

        totalTime += dt;

        UpdateEmitters(dt);
		UpdateParticles(dt);

		if (allowAutodelete && !IsVisible())
		{
			if (GetParent())
			{
				SetParent(nullptr);
				Release();
			}
			else
			{
				GetScene()->DeleteEntity(this, true);
			}
		}
    }

    void VFXEmitter::Draw(float dt)
    {
        if (!IsVisible() || !GetScene()->IsPlaying() || !WorldManager::instance)
		{
			return;
		}

		dt *= dtScale;

		DrawParticles(dt);
    }

    void VFXEmitter::EditorDraw(float dt)
	{
        if (GetScene()->IsPlaying() || !IsVisible())
		{
			return;
		}

        Update(dt);

        DrawParticles(dt);
    }

    int VFXEmitter::SpawnParticles(const Emitter &emitter, Math::Vector3 pos, float rotation, Math::Vector3 vel)
	{
		const int particlesCount = (int)::roundf(rndGenerator.Range(emitter.particlesCount));
		if (particlesCount <= 0)
		{
			return particlesCount;
		}

		const bool isPlaying = GetScene()->IsPlaying();
		const Math::Vector2 spawnSector = rotation + Math::Vector2{ Utils::ToRadian(emitter.sector.x), Utils::ToRadian(emitter.sector.y) };

		particles.reserve(particles.size() + particlesCount);

		for (int i = 0; i < particlesCount; ++i)
		{
			Particle &p = particles.emplace_back();

			if (!emitter.textureIndices.empty())
			{
				const int textureIndex = (int)::roundf(rndGenerator.NextFloat() * float(emitter.textureIndices.size() - 1));
				const auto &texture = emitter.textures[emitter.textureIndices[textureIndex]];
				p.texture       = texture.texture;
				p.textureScale  = texture.scale * transform.scale.x * rndGenerator.Range(emitter.scale);
				p.textureOffset = texture.offset;
			}

			p.vel   = emitter.affectByEmitterVelocity * vel + rndGenerator.Range(emitter.velocity) * Utils::Angle::Radian(rndGenerator.Range(spawnSector)).ToDirection();
			p.omega = rndGenerator.Range(emitter.omega);

			p.pos = pos + rndGenerator.Range(emitter.radius) * Utils::Vector::Normalize(p.vel);
			p.pos += Utils::Vector::Rotate(Math::Vector3{rndGenerator.Range(emitter.spawnX), 0.f, 0.f}, -rotation);
			p.pos += Utils::Vector::Rotate(Math::Vector3{0.f, rndGenerator.Range(emitter.spawnY), 0.f}, -rotation);

			if (emitter.matchEntityRotation)
			{
				p.angle = Utils::ToDegrees(rotation);
			}

			p.angle += rndGenerator.Range(emitter.angle);

			if (isPlaying)
			{
				if (auto castRes = Utils::RayCastLine(pos, p.pos, PhysGroup::WorldDoors))
				{
					p.pos = castRes->hitPos + 1.f * castRes->hitNormal;
				}
			}

			p.ttl    = rndGenerator.Range(emitter.distance) * Utils::SafeInvert(p.vel.Length());
			if (Math::IsEqual(p.ttl, 0.f))
			{
				p.ttl = emitter.timeLife;
			}

			p.color  = emitter.color;
			p.bounce = rndGenerator.Range(emitter.bounce);
			p.bounceDeviation = emitter.bounceDeviation;
			p.gravity = emitter.gravity;
			p.affectByPlayerDist = emitter.affectByPlayerDist;
			p.affectByPlayerFactor = emitter.affectByPlayerFactor;

            p.fadeIn   = emitter.fadeIn;
            p.fadeOut  = emitter.fadeOut;
            p.scaleIn  = emitter.scaleIn;
            p.scaleOut = emitter.scaleOut;

			p.useParticlePrg = emitter.useParticlePrg;
			p.matchVelocityRotation = emitter.matchVelocityRotation;

			p.turbulence       = Math::Vector2{rndGenerator.Range(emitter.turbulenceX), rndGenerator.Range(emitter.turbulenceY)};
			p.noiseOctaves     = emitter.noiseOctaves;
            p.noisePersistence = emitter.noisePersistence;

			p.startTtl = p.ttl;
		}

		return particlesCount;
	}

	void VFXEmitter::Start()
	{
		if (isStarted)
		{
			return;
		}

		isStarted = true;

		totalTime = 0.0f;

		for (auto& emitter : emitters)
		{
			emitter.state.isDone = false;
			emitter.state.nextEmitAtTime = 0.0f;
		}
	}

	void VFXEmitter::Stop()
	{
		isStarted = false;
	}

	void VFXEmitter::ResetAndStop()
	{
		particles.clear();

		Stop();
	}

	void VFXEmitter::UpdateEmitters(float dt)
	{
		if (!isStarted)
		{
			return;
		}

		bool allDone = true;

		auto mat = transform.GetGlobal();
		auto pos = Sprite::ToPixels(mat.Pos());
		auto rotation = Math::SafeAtan2(mat.Vx().y, mat.Vx().x);

		for (auto &emitter : emitters)
        {
			if (!emitter.enabled)
			{
				continue;
			}

			if (!emitter.state.isDone && emitter.timeLife > 0.f && totalTime >= emitter.timeLife)
			{
				emitter.state.isDone = true;
			}

			allDone &= emitter.state.isDone;

            if (!emitter.state.isDone && totalTime >= emitter.state.nextEmitAtTime && Math::IsNonZero(emitter.emitsFreq))
            {
                emitter.state.nextEmitAtTime = totalTime + Utils::SafeInvert(emitter.emitsFreq);
                const int particlesCount = SpawnParticles(emitter, pos, rotation, emitterVelocity);
				if (particlesCount <= 0)
				{
					emitter.state.isDone = true;
				}
            }
        }

		if (allDone && particles.empty())
		{
			if (GetScene()->IsPlaying() && !isLooped)
			{
				SetVisiblity(false);
			}
			else
			{
				totalTime = 0.0f;

				for (auto& emitter : emitters)
				{
					emitter.state.isDone = false;
					emitter.state.nextEmitAtTime = 0.0f;
				}
			}
		}
	}

	void VFXEmitter::DoUpdateParticles(float dt)
	{
		Math::Vector3 playerVel;
		if (WorldManager::instance)
		{
			playerVel     = WorldManager::instance->GetPlayerPos() - lastPlayerPos;
			lastPlayerPos = WorldManager::instance->GetPlayerPos();
		}

		const bool isPlaying = GetScene()->IsPlaying();
		for (auto &p : particles)
		{
			p.vel.y -= p.gravity * dt;
			
			if (p.affectByPlayerDist > 0.f && (lastPlayerPos - p.pos).Length2() <= p.affectByPlayerDist * p.affectByPlayerDist)
			{
				p.vel += playerVel * p.affectByPlayerFactor * dt;
			}

			if (p.noiseOctaves > 0)
			{
				p.vel += p.turbulence * noiseGenerator.normalizedOctave2D_01(p.pos.x, p.pos.y, p.noiseOctaves, p.noisePersistence);
			}

			if (p.matchVelocityRotation)
			{
				p.angle = Utils::Angle::FromDirection(p.vel).ToSignedDegrees();
			}
			else
			{
				p.angle += p.omega * dt;
			}

			auto wishPos = p.pos + p.vel * dt;

			if (isPlaying)
			{
				if (auto castRes = Utils::RayCastLine(p.pos, wishPos, PhysGroup::WorldDoors))
				{
					p.vel = p.bounce * Utils::Vector::Reflect(p.vel, castRes->hitNormal);
					const float deviation = Utils::ToRadian(rndGenerator.Range(p.bounceDeviation));
					if (Math::IsNonZero(deviation))
					{
						p.vel = Utils::Vector::Rotate(p.vel, deviation);
					}

					if (p.vel.Length2() < 1.f)
					{
						p.ttl = 0.f;
					}

					const float distAfterHit = (wishPos - p.pos).Length() - (castRes->hitPos - p.pos).Length();
					wishPos = castRes->hitPos + Utils::Vector::Normalize(p.vel) * std::max(distAfterHit, 0.f);
					p.lastBounceNormal = castRes->hitNormal;
				}
			}

			p.pos  = wishPos;
			p.ttl -= dt;
		}
	}

    void VFXEmitter::UpdateParticles(float dt)
	{
		DoUpdateParticles(dt);

		particles.erase(eastl::remove_if(particles.begin(), particles.end(), [](const Particle &p)
		{
			return p.ttl <= 0.f;
		}), particles.end());
	}

    void VFXEmitter::DrawParticles(float dt)
	{
		Transform trans;
		trans.objectType = ObjectType::Object2D;

		for (auto &p : particles)
		{
			const float t = (p.startTtl - p.ttl) / p.startTtl;

			Color color = p.color;

			if (t <= p.fadeIn.y && Math::IsNonZero(p.fadeIn.y - p.fadeIn.x))
			{
				color.a *= Utils::Smoothstep(p.fadeIn.x, p.fadeIn.y, t);
			}
			else if (Math::IsNonZero(p.fadeOut.y - p.fadeOut.x))
			{
				color.a *= Utils::Smoothstep(p.fadeOut.y, p.fadeOut.x, t);
			}

			trans.size     = p.texture.GetSize();
			trans.offset   = p.textureOffset;
			trans.scale    = p.textureScale;
			trans.position = p.pos;
			trans.rotation = Math::Vector3{0.f, 0.f, p.angle};

			if (t <= p.scaleIn.y && Math::IsNonZero(p.scaleIn.y - p.scaleIn.x))
			{
				trans.scale *= Utils::Smoothstep(p.scaleIn.x, p.scaleIn.y, t);
			}
			else if (Math::IsNonZero(p.scaleOut.y - p.scaleOut.x))
			{
				trans.scale *= Utils::Smoothstep(p.scaleOut.y, p.scaleOut.x, t);
			}

			p.texture.prg = p.useParticlePrg ? particlePrg : (noZ ? Sprite::quadPrgNoZ : Sprite::quadPrg);
			p.texture.Draw(&trans, color, dt);
		}
	}
}