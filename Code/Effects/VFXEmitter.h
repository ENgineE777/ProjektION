#pragma once

#include "SceneEntities/2D/ScriptEntity2D.h"
#include "SceneEntities/2D/Camera2D.h"
#include "Common/Utils.h"
#include "Common/Events.h"

#include "EASTL/array.h"
#include "EASTL/bonus/ring_buffer.h"

namespace Orin::Overkill
{
	class VFXEmitter : public ScriptEntity2D
	{
    protected:
        class ParticleRenderTechnique final : public RenderTechnique
        {
        public:
            virtual const char* GetVsName() { return "sprite_vs.shd"; };
            virtual const char* GetPsName() { return "sprite_ps.shd"; };

            virtual void ApplyStates()
            {
                GetRoot()->GetRender()->GetDevice()->SetDepthTest(false);
                GetRoot()->GetRender()->GetDevice()->SetDepthWriting(false);
                GetRoot()->GetRender()->GetDevice()->SetAlphaBlend(true);
                GetRoot()->GetRender()->GetDevice()->SetBlendFunc(BlendArg::ArgSrcAlpha, BlendArg::ArgOne);
                GetRoot()->GetRender()->GetDevice()->SetCulling(CullMode::CullNone);
            };
        };

        struct Texture
        {
            META_DATA_DECL_BASE(Texture)

            AssetTextureRef texture;
            Math::Vector2 scale;
            Math::Vector2 offset;
            int weight;
        };

        struct Emitter
        {
            META_DATA_DECL_BASE(Emitter)

            struct State
            {
                float nextEmitAtTime = 0.0f;
                bool isDone = false;
            };

            State state;

            eastl::vector<Texture> textures;
            eastl::vector<int> textureIndices;

            Color color;

            Math::Vector2 velocity;
            Math::Vector2 omega;
            Math::Vector2 sector;
            Math::Vector2 distance;
            Math::Vector2 fadeIn;
            Math::Vector2 fadeOut;
            Math::Vector2 scaleIn;
            Math::Vector2 scaleOut;
            Math::Vector2 scale;
            Math::Vector2 particlesCount;
            Math::Vector2 radius;
            Math::Vector2 spawnX;
            Math::Vector2 spawnY;
            Math::Vector2 angle;
            Math::Vector2 bounce;
            Math::Vector2 bounceDeviation;
            Math::Vector2 turbulenceX;
            Math::Vector2 turbulenceY;

            int noiseOctaves;
            float noisePersistence;

            float emitsFreq;
            float timeLife = -1.f;
            float gravity;
            float affectByPlayerDist;
            float affectByPlayerFactor;
            float affectByEmitterVelocity;

            bool enabled = true;
            bool matchEntityRotation = false;
            bool matchVelocityRotation = false;
            bool useParticlePrg = false;
        };

        struct Particle
        {
            AssetTextureRef texture;
            Math::Vector2 textureScale;
            Math::Vector2 textureOffset;

            Color color;

            Math::Vector3 pos;
            Math::Vector3 vel;
            float omega;
            float angle;

            Math::Vector2 fadeIn;
            Math::Vector2 fadeOut;
            Math::Vector2 scaleIn;
            Math::Vector2 scaleOut;

            float bounce;
            Math::Vector2 bounceDeviation;
            Math::Vector3 lastBounceNormal;

            Math::Vector2 turbulence;

            float ttl;
            float startTtl;
            float gravity;
            float affectByPlayerDist;
            float affectByPlayerFactor;

            int noiseOctaves;
            float noisePersistence;

            bool matchVelocityRotation = false;
            bool useParticlePrg = false;
        };

        eastl::vector<Emitter> emitters;
        eastl::vector<Particle> particles;

        Utils::RndGenerator rndGenerator;
        Utils::PerlinNoiseGenerator noiseGenerator;

        RenderTechniqueRef particlePrg;

        float totalTime;

        int drawLevel = 0;
        bool noZ = false;

        Math::Vector3 lastPlayerPos;

        bool isStarted = true;
        bool isLooped  = false;

        void InitEmitterTextures();

    public:

        bool allowAutodelete = false;

        META_DATA_DECL(VFXEmitter);

        Math::Vector3 emitterVelocity;
        float dtScale = 1.0f;

        void ApplyProperties() override;
        void Init() override;
        void Release() override;
		void Play() override;
		void Update(float dt) override;

        virtual void UpdateEmitters(float dt);
        virtual void DoUpdateParticles(float dt);

        void UpdateParticles(float dt);
        void DrawParticles(float dt);
        int SpawnParticles(const Emitter &emitter, Math::Vector3 pos, float rotation, Math::Vector3 vel);

        virtual void Draw(float dt);
        virtual void EditorDraw(float dt);

        void Start();
        void Stop();
        void ResetAndStop();
    };
}