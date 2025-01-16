#include "BloodManager.h"
#include "Effects/BloodEffect.h"
#include "Effects/VFXEmitter.h"
#include "Root/Root.h"
#include "Common/Constants.h"


namespace Orin::Overkill
{
	ENTITYREG(SceneEntity, BloodManager, "Overkill/World", "BloodManager")

	META_DATA_DESC(BloodManager::Preset)
        STRING_PROP(BloodManager::Preset, name, "", "Preset", "name")
		FILENAME_PROP(BloodManager::Preset, prefab, "", "Preset", "prefab")
    META_DATA_DESC_END()

	META_DATA_DESC(BloodManager::TileTextures::Level)
		ASSET_TEXTURE_PROP(BloodManager::TileTextures::Level, texture, "Level", "texture")
		INT_PROP(BloodManager::TileTextures::Level, stepsToNext, 1, "Level", "stepsToNext", "stepsToNext")
    META_DATA_DESC_END()

	META_DATA_DESC(BloodManager::TileTextures)
		ARRAY_PROP(BloodManager::TileTextures, levels, Level, "Blood", "levels")
    META_DATA_DESC_END()

	META_DATA_DESC(BloodManager)
		BASE_SCENE_ENTITY_PROP(BloodManager)

		INT_PROP(BloodManager, drawLevel, 0, "Visual", "draw_level", "Draw priority")
		MARK_DISABLED_FOR_INSTANCE()

		ARRAY_PROP(BloodManager, presets, Preset, "Blood", "presets")
		ARRAY_PROP(BloodManager, wallsTextures, BloodManager::TileTextures, "Blood", "wallsTextures")
		ARRAY_PROP(BloodManager, floorTextures, BloodManager::TileTextures, "Blood", "floorTextures")
		ARRAY_PROP(BloodManager, ceilingTextures, BloodManager::TileTextures, "Blood", "ceilingTextures")
	META_DATA_DESC_END()

	void BloodManager::ApplyProperties()
	{
		for (auto &preset : presets)
		{
			preset.prefabRef = GetRoot()->GetAssets()->GetAssetRef<AssetPrefabRef>(preset.prefab);	
		}

#ifdef ORIN_EDITOR
		Tasks(true)->DelAllTasks(this);
#endif

        Tasks(true)->AddTask(0 + drawLevel, this, (Object::Delegate)&BloodManager::Draw);
	}

    void BloodManager::Init()
    {
        ScriptEntity2D::Init();

		Tasks(true)->AddTask(0 + drawLevel, this, (Object::Delegate)&BloodManager::Draw);
    }

    void BloodManager::Release()
    {
		Tasks(true)->DelTask(0 + drawLevel, this);

        ScriptEntity2D::Release();
    }

    void BloodManager::Play()
    {
        ScriptEntity2D::Play();
    }

	void BloodManager::Clear()
	{		
		for (auto kv : instances)
		{
			GetScene()->DeleteEntity(kv.second, true);
		}
		
		instances.clear();

		tileMap.clear();
		tileData.clear();
	}
	
    void BloodManager::Update(float dt)
    {
    }

	SceneEntity* BloodManager::Spawn(AssetPrefabRef ref, SceneEntity* parent, Math::Vector3 pos, float angle, bool addToList)
	{
		SceneEntity *effect = ref->CreateInstance(GetScene());
		if (!effect)
		{
			return effect;
		}	

		if (auto* emitter = dynamic_cast<VFXEmitter*>(effect))
		{
			emitter->allowAutodelete = true;
		}

		effect->Play();

		auto &tm = effect->GetTransform();
		const Math::Vector3 offset = Utils::Vector::Rotate(tm.position, angle);
		tm.position = pos + offset;
		tm.rotation = Utils::Vector::xyV(tm.rotation, angle);

		effect->SetVisiblity(true);

		if (parent)
		{
			effect->SetParent(parent);
		}
		else
		{
			GetScene()->AddEntity(effect);
		}		

		return effect;
	}

	SceneEntity* BloodManager::SpawnUnique(AssetPrefabRef ref, Math::Vector3 pos, float angle)
	{
		if (!ref)
		{
			return nullptr;
		}

		auto it = instances.find(ref->GetPath());
		if (it != instances.end())
		{
			return it->second;
		}

		SceneEntity *effect = Spawn(ref, nullptr, pos, angle, false /* addToList */);
		if (effect)
		{
			instances.insert(eastl::make_pair(ref->GetPath(), effect));
		}
		
		return effect;
	}

	SceneEntity* BloodManager::Spawn(const char *name, SceneEntity* parent, Math::Vector3 pos, float angle)
	{
		for (auto &preset : presets)
		{
			if (preset.name == name)
			{
				return Spawn(preset.prefabRef, parent, pos, angle, true /* addToList */);
			}
		}
		return nullptr;
	}

	void BloodManager::AddTileBlood(Math::Vector3 pos, Math::Vector3 normal)
	{
		const auto posInTile = pos - normal * TILE_HALF_SIZE;
		const int tileX = (int)(posInTile.x / TILE_SIZE) + (posInTile.x > 0.f ? 0 : -1);
		const int tileY = (int)(posInTile.y / TILE_SIZE) + (posInTile.y > 0.f ? 1 : 0);

		bool onWall = false;
		bool onCeiling = false;
		bool onGround = false;
		eastl::tie(onWall, onCeiling, onGround) = Utils::NormalToFlags(normal);

		eastl::vector<TileTextures> *textures = nullptr;
		Math::Vector2 posOnTile;
		Math::Vector2 tileOffset;
		Math::Vector2 tileScale{1.f, 1.f};

		int side = 0;
		if (onCeiling)
		{
			side = 0;
			textures = &ceilingTextures;
			posOnTile = {float(tileX) * TILE_SIZE + TILE_HALF_SIZE, float(tileY - 1) * TILE_SIZE};
			tileOffset = {0.5f, 1.f};
		}
		else if (onGround)
		{
			side = 1;
			textures = &floorTextures;
			posOnTile = {float(tileX) * TILE_SIZE + TILE_HALF_SIZE, float(tileY) * TILE_SIZE};
			tileOffset = {0.5f, 0.f};
		}
		else if (onWall)
		{
			// Left, Right
			side = normal.x < 0.f ? 2 : 3;
			textures = &wallsTextures;
			posOnTile = {float(tileX + side - 2) * TILE_SIZE, float(tileY) * TILE_SIZE - TILE_HALF_SIZE};
			tileOffset = {1.f, 0.5f};
			tileScale.x = Math::Sign(normal.x);
		}
		else
		{
			return;
		}

		if (textures->empty())
		{
			return;
		}

		const auto tileId = TileId{tileX, tileY, side};

		auto dataId = tileMap.find(tileId);
		if (dataId == tileMap.end())
		{
			auto &data = tileData.emplace_back();
			data.level = 0;
			data.variant = int(rndGenerator.NextUInt() % uint64_t(textures->size()));
			data.stepsToNext = (*textures)[data.variant].levels[data.level].stepsToNext;
			data.textures = textures;
			data.pos = posOnTile;
			data.offset = tileOffset;
			data.scale = tileScale;
			data.id = tileId;

			tileMap.insert(eastl::make_pair(tileId, int(tileData.size()) - 1));
		}
		else
		{
			auto &data = tileData[dataId->second];
			if (data.stepsToNext > 0)
			{
				data.stepsToNext -= 1;
				if (data.stepsToNext == 0 && data.level + 1 < (*textures)[data.variant].levels.size())
				{
					++data.level;
					data.stepsToNext = (*textures)[data.variant].levels[data.level].stepsToNext;
				}
			}
		}
	}

	void BloodManager::Draw(float dt)
	{
		Transform trans;
		trans.objectType = ObjectType::Object2D;

		for (const auto &d : tileData)
		{
			auto tex = (*d.textures)[d.variant].levels[d.level].texture;

			trans.size     = tex.GetSize();
			trans.scale    = d.scale;
			trans.offset   = d.offset;
			trans.position = d.pos;

			tex.prg = Sprite::quadPrgNoZ;
			tex.Draw(&trans, COLOR_WHITE, dt);
		}
	}
}