#pragma once

#include "SceneEntities/2D/ScriptEntity2D.h"
#include "SceneEntities/2D/Camera2D.h"
#include "Common/Utils.h"
#include "Common/Events.h"

#include "EASTL/array.h"
#include "EASTL/hash_map.h"

namespace Orin::Overkill
{
	class BloodManager : public ScriptEntity2D
	{
        struct Preset
        {
            META_DATA_DECL_BASE(Preset)

            eastl::string name;
            eastl::string prefab;

            AssetPrefabRef prefabRef;
        };

        struct BloodEffectCell
        {
            Math::Vector2 from;
            Math::Vector2 to;
        };

        struct TileTextures
        {
            META_DATA_DECL_BASE(TileTextures)

            struct Level
            {
                META_DATA_DECL_BASE(Level)

                AssetTextureRef texture;
                int stepsToNext;
            };

            eastl::vector<Level> levels;
        };

        struct TileId
        {
            int x;
            int y;
            int side;

            bool operator==(const TileId &other) const
            {
                return x == other.x && y == other.y && side == other.side;
            }
        };

        struct TileIdHash
        {
            size_t operator()(TileId id) const
            {
                return Utils::UIntNoise1D((uint64_t(id.x) << 32ull) | uint64_t(id.y)) * uint64_t(id.side + 1);
            }
        };

        struct TileData
        {
            TileId id;

            eastl::vector<TileTextures> *textures = nullptr;

            Math::Vector2 pos;
            Math::Vector2 offset;
            Math::Vector2 scale;

            int variant;
            int level;
            int stepsToNext;
        };

        eastl::hash_map<TileId, int, TileIdHash> tileMap;
        eastl::vector<TileData> tileData;

        eastl::vector<TileTextures> wallsTextures;
        eastl::vector<TileTextures> floorTextures;
        eastl::vector<TileTextures> ceilingTextures;

        eastl::vector<Preset> presets;
        //eastl::vector<SceneEntity*> effects;
        eastl::hash_map<eastl::string, SceneEntity*> instances;

        Utils::RndGenerator rndGenerator;

        int drawLevel = 0;

        SceneEntity* Spawn(AssetPrefabRef ref, SceneEntity* parent, Math::Vector3 pos, float angle, bool addToList);

    public:
        META_DATA_DECL(BloodManager);

        void ApplyProperties() override;
        void Init() override;
        void Release() override;
		void Play() override;
		void Update(float dt) override;

		void Draw(float dt);

        void Clear();

        SceneEntity* Spawn(const char *name, SceneEntity* parent, Math::Vector3 pos, float angle);
        SceneEntity* SpawnUnique(AssetPrefabRef ref, Math::Vector3 pos, float angle);

        void AddTileBlood(Math::Vector3 pos, Math::Vector3 normal);
    };
}