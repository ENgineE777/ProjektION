#pragma once

#include "Utils.h"

#include "World/WorldManager.h"

#include <Windows.h>

#include "Objects/TimedInstantPlayerKiller.h"
#include "Objects/Checkpoint.h"
#include "Objects/InstantPlayerKiller.h"
#include "Objects/MovingInstantPlayerKiller.h"
#include "Objects/NextLevelTrigger.h"
#include "Objects/MovingPlatform.h"
#include "Characters/Enemies/Mine.h"
#include "Common/Constants.h"

#include "SceneEntities/2D/GenericMarker2D.h"
#include "SceneEntities/2D/PointLight2D.h"

#include "Root/Assets/AssetTileSet.h"

namespace Orin::Overkill::Utils
{   
    Utils::RndGenerator rndGenerator;

    float GetRandom(float from, float to)
    {
        return from + (to - from) * rndGenerator.NextFloat();
    }

    float GetRandom(Math::Vector2 v)
    {
        return v.x + (v.y - v.x) * rndGenerator.NextFloat();
    }

    eastl::optional<PhysScene::RaycastDesc> RayCast(Math::Vector3 from, Math::Vector3 dir, float length, uint32_t group)
    {
        PhysScene::RaycastDesc rcdesc;

        rcdesc.origin = Sprite::ToUnits(from);
        rcdesc.dir    = dir;
        rcdesc.length = Sprite::ToUnits(length);
        rcdesc.group  = group;

        if (GetRoot()->GetPhysScene()->RayCast(rcdesc))
        {
            rcdesc.hitPos    = Sprite::ToPixels(rcdesc.hitPos);
            rcdesc.hitLength = Sprite::ToPixels(rcdesc.hitLength);
            return eastl::make_optional(rcdesc);
        }

        return eastl::nullopt;
    }

    eastl::optional<PhysScene::RaycastDesc> RayCastLine(Math::Vector3 from, Math::Vector3 to, uint32_t group)
    {
        const Math::Vector3 dir = to - from;
        const float length = dir.Length();
        return RayCast(from, dir * SafeInvert(length), length, group);
    }

    bool RayHit(Math::Vector3 from, Math::Vector3 dir, float length, uint32_t group)
    {
        PhysScene::RaycastDesc rcdesc;

        rcdesc.origin = Sprite::ToUnits(from);
        rcdesc.dir    = dir;
        rcdesc.length = Sprite::ToUnits(length);
        rcdesc.group  = group;

        return GetRoot()->GetPhysScene()->RayCast(rcdesc);
    }

    bool RayHitLine(Math::Vector3 from, Math::Vector3 to, uint32_t group)
    {
        const Math::Vector3 dir = to - from;
        const float length = dir.Length();
        return RayHit(from, dir * SafeInvert(length), length, group);
    }

    eastl::optional<PhysScene::RaycastDesc> SphereCast(Math::Vector3 from, Math::Vector3 dir, float length, float radius, uint32_t group)
    {
        PhysScene::RaycastDesc rcdesc;

        rcdesc.origin = Sprite::ToUnits(from);
        rcdesc.dir    = dir;
        rcdesc.length = Sprite::ToUnits(length);
        rcdesc.group  = group;

        PhysScene *scene = GetRoot()->GetPhysScene();

        if (scene && scene->SphereCast(rcdesc, Sprite::ToUnits(radius)))
        {
            rcdesc.hitPos    = Sprite::ToPixels(rcdesc.hitPos);
            rcdesc.hitLength = Sprite::ToPixels(rcdesc.hitLength);
            return eastl::make_optional(rcdesc);
        }

        return eastl::nullopt;
    }

    eastl::optional<PhysScene::RaycastDesc> SphereCastLine(Math::Vector3 from, Math::Vector3 to, float radius, uint32_t group)
    {
        const Math::Vector3 dir = to - from;
        const float length = dir.Length();
        return SphereCast(from, dir * SafeInvert(length), length, radius, group);
    }

    bool SphereHit(Math::Vector3 from, Math::Vector3 dir, float length, float radius, uint32_t group)
    {
        PhysScene::RaycastDesc rcdesc;

        rcdesc.origin = Sprite::ToUnits(from);
        rcdesc.dir    = dir;
        rcdesc.length = Sprite::ToUnits(length);
        rcdesc.group  = group;

        return GetRoot()->GetPhysScene()->SphereCast(rcdesc, Sprite::ToUnits(radius));
    }

    bool SphereHitLine(Math::Vector3 from, Math::Vector3 to, float radius, uint32_t group)
    {
        const Math::Vector3 dir = to - from;
        const float length = dir.Length();
        return SphereHit(from, dir * SafeInvert(length), length, radius, group);
    }

    MaybeCastRes RayCastBox(Math::Vector3 from, Math::Vector3 dir, float length, Math::Matrix boxTrans, Math::Vector3 boxHalfSize)
    {
        PhysScene::RaycastDesc rcdesc;

        rcdesc.origin = Sprite::ToUnits(from);
        rcdesc.dir    = dir;
        rcdesc.length = Sprite::ToUnits(length);

        if (PhysScene::RayCastBox(rcdesc, boxTrans, boxHalfSize))
        {
            rcdesc.hitPos    = Sprite::ToPixels(rcdesc.hitPos);
            rcdesc.hitLength = Sprite::ToPixels(rcdesc.hitLength);
            return eastl::make_optional(rcdesc);
        }

        return eastl::nullopt;
    }

    void DebugArrow(Math::Vector3 from, Math::Vector3 to, Color color)
    {
        Sprite::DebugLine(from, to, color);

        const auto a = Angle::FromDirection(Vector::Normalize(to - from)) + Angle::Degrees(180.f);
        DebugTrinagle(to, a - Angle::Degrees(10.f), a + Angle::Degrees(10.f), 16.f, color);
    }

    void DebugTrinagle(Math::Vector3 pos, Utils::Angle from, Utils::Angle to, float length, Color color)
    {
        auto p1 = pos + from.ToDirection() * length;
        auto p2 = pos + to.ToDirection() * length;

        Sprite::DebugLine(pos, p1, color);
        Sprite::DebugLine(pos, p2, color);
        Sprite::DebugLine(p1, p2, color);
    }

    void DebugSector(Math::Vector3 pos, Utils::Angle from, Utils::Angle to, Color color, float step, float length)
    {
        float distance   = from.Distance(to).ToDegrees();
        float drawSector = to < from ? 360.f - distance : distance;

        for (Utils::Angle cur = from; drawSector >= 0.f; cur += Utils::Angle::Degrees(step), drawSector -= step)
        {
            Sprite::DebugLine(pos + cur.ToDirection() * 10.0f, pos + cur.ToDirection() * length, color);
        }
    }

    uint64_t UIntNoise1D(uint64_t position)
    {
        const uint64_t BIT_NOISE1 = 0xB5297A4DB5297A4D;
        const uint64_t BIT_NOISE2 = 0x68E31DA468E31DA4;
        const uint64_t BIT_NOISE3 = 0x1B56C4E91B56C4E9;
            
        uint64_t mangled = position;
        mangled *= BIT_NOISE1;
        mangled ^= (mangled >> 8);
        mangled += BIT_NOISE2;
        mangled ^= (mangled << 8);
        mangled *= BIT_NOISE3;
        mangled ^= (mangled >> 8);
        return mangled;
    }

    Math::Vector2 GetCursorPos(int aliasX, int aliasY)
    {
        float msX =  GetRoot()->GetControls()->GetAliasValue(aliasX, false) - (float)GetRoot()->GetRender()->GetDevice()->GetWidth() * 0.5f;
        float msY = -GetRoot()->GetControls()->GetAliasValue(aliasY, false) + (float)GetRoot()->GetRender()->GetDevice()->GetHeight() * 0.5f;

        float scale = Sprite::GetPixelsHeight() / (float)GetRoot()->GetRender()->GetDevice()->GetHeight();

        return Math::Vector2(msX, msY) * scale + Sprite::GetCamPos();
    }

    void AddCameraShake(float addTrauma)
    {
        MainEventsQueue::PushEvent(CmdAddShakeTrauma{addTrauma});
    }

    void FormatTimer(float timer, eastl::string& result)
    {
        int munutes = (int)(timer / 60.0f);
        float seconds = timer - 60.0f * munutes;

        result = StringUtils::PrintTemp("%i : %05.02f", munutes, seconds);
    }

    bool AutoReloadTechnique::TryReload(const char *shader)
	{
		HANDLE hFile = CreateFileA(StringUtils::PrintTemp("%s%s", GetRoot()->GetPath(Root::Path::Assets), shader), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

		uint64_t tm;
		if ((::GetFileTime(hFile, NULL, NULL, (FILETIME*)&tm) && tm != lastWriteTime) || GetRoot()->GetControls()->DebugHotKeyPressed("KEY_U", "KEY_I"))
		{
			CloseHandle(hFile);

			lastWriteTime = tm;

			tech->ReloadShaders();

			return true;
		}

		CloseHandle(hFile);

		return false;
	}

	bool AutoReloadTechnique::TryReload()
	{
		return TryReload(tech->GetPsName())/*  || TryReload(tech->GetVsName()) */;
	}

    void ConvertEntities(const eastl::vector<SceneEntity*>& entities)
    {
        for (auto* entity : entities)
        {
            /*auto* timedInstantPlayerKiller = dynamic_cast<TimedInstantPlayerKiller*>(entity);

            if (timedInstantPlayerKiller)
            {
                if (timedInstantPlayerKiller->overridePreActivationTime > 0.01f)
                {
                    timedInstantPlayerKiller->optPreActivationTime.enabled = true;
                    timedInstantPlayerKiller->optPreActivationTime.value = timedInstantPlayerKiller->overridePreActivationTime;
                }

                if (timedInstantPlayerKiller->overrideTimeActive > 0.01f)
                {
                    timedInstantPlayerKiller->optTimeActive.enabled = true;
                    timedInstantPlayerKiller->optTimeActive.value = timedInstantPlayerKiller->overrideTimeActive;
                }

                if (timedInstantPlayerKiller->overrideTimeInactive > 0.01f)
                {
                    timedInstantPlayerKiller->optTimeInactive.enabled = true;
                    timedInstantPlayerKiller->optTimeInactive.value = timedInstantPlayerKiller->overrideTimeInactive;
                }
            }

            auto* movingInstantPlayerKiller = dynamic_cast<MovingInstantPlayerKiller*>(entity);

            if (movingInstantPlayerKiller)
            {
                if (movingInstantPlayerKiller->overridePreActivationTime > 0.01f)
                {
                    movingInstantPlayerKiller->optPreActivationTime.enabled = true;
                    movingInstantPlayerKiller->optPreActivationTime.value = movingInstantPlayerKiller->overridePreActivationTime;
                }

                if (movingInstantPlayerKiller->overrideTimeActive > 0.01f)
                {
                    movingInstantPlayerKiller->optTimeActive.enabled = true;
                    movingInstantPlayerKiller->optTimeActive.value = movingInstantPlayerKiller->overrideTimeActive;
                }

                if (movingInstantPlayerKiller->overrideTimeInactive > 0.01f)
                {
                    movingInstantPlayerKiller->optTimeInactive.enabled = true;
                    movingInstantPlayerKiller->optTimeInactive.value = movingInstantPlayerKiller->overrideTimeInactive;
                }

                if (movingInstantPlayerKiller->overrideMoveSpeed > 0.01f)
                {
                    movingInstantPlayerKiller->optMoveSpeed.enabled = true;
                    movingInstantPlayerKiller->optMoveSpeed.value = movingInstantPlayerKiller->overrideMoveSpeed;
                }
            }


            /*auto* markers = dynamic_cast<GenericMarker2D*>(entity);

            if (markers)
            {
                for (auto& inst : markers->instances)
                {
                    inst.transform.position *= TILE_SCALE;
                    inst.radius *= TILE_SCALE;
                }
            }

            auto* light = dynamic_cast<PointLight2D*>(entity);

            if (light)
            {
                auto rot = entity->GetTransform().rotation;
                entity->GetTransform().rotation = Math::Vector3(rot.x, rot.y, 360.0f - rot.z);

                light->lineWidth *= TILE_SCALE;
            }

            if (light || dynamic_cast<TimedInstantPlayerKiller*>(entity) || dynamic_cast<NextLevelTrigger*>(entity) ||
                dynamic_cast<MovingInstantPlayerKiller*>(entity) || dynamic_cast<Checkpoint*>(entity) || dynamic_cast<InstantPlayerKiller*>(entity))
            {
                auto sz = entity->GetTransform().size;
                sz.x *= TILE_SCALE;
                sz.y *= TILE_SCALE;

                entity->GetTransform().size = sz;
            }

            auto* drone = dynamic_cast<Drone*>(entity);

            if (drone)
            {
                for (auto& enty : drone->overrides)
                {
                    if (enty.property == Drone::OverrideProperty::MoveSpeed || enty.property == Drone::OverrideProperty::ChargeMaxSpeed)
                    {    
                        enty.value *= TILE_SCALE;
                    }
                }
            }

            auto* movingIPK = dynamic_cast<MovingInstantPlayerKiller*>(entity);

            if (movingIPK)
            {
                if (Math::AreApproximatelyEqual(movingIPK->overrideMoveSpeed, 300.0f * TILE_SCALE))
                {
                    movingIPK->overrideMoveSpeed = -1.0f;
                }
                else
                {
                    movingIPK->overrideMoveSpeed *= TILE_SCALE;
                }
            }
            
            auto* tileMap = dynamic_cast<TileMap*>(entity);

            if (tileMap)
            {
                tileMap->tileSet = GetRoot()->GetAssets()->GetAssetRef<AssetTileSetRef>(eastl::string("prefabs/tilesets/default/map.tls"));
            }

            auto pos = entity->GetTransform().position;

            entity->GetTransform().position = Math::Vector3(pos.x * TILE_SCALE, pos.y * TILE_SCALE, pos.z);

            if (dynamic_cast<InstantPlayerKiller*>(entity))
            {
                entity->GetTransform().offset = Math::Vector3(0.5f, 0.5f, 0.0f);
                entity->GetTransform().position += Math::Vector3(TILE_HALF_SIZE, -TILE_HALF_SIZE, 0.0f);
            }*/

            ConvertEntities(entity->GetChilds());
        }
    }

    void ConvertScene(Scene* scene)
    {
        ConvertEntities(scene->GetEntities());
    }

    void ConvertAllScenes()
    {
        auto assets = GetRoot()->GetAssets()->GetAssetsMap();

        for (auto& entry : assets)
        {
            const char* path = entry.first.c_str();

            if (strstr(path, "/maps") || strstr(path, "/testmaps"))
            {
                auto* assetScene = entry.second->GetAsset<AssetScene>();

                if (assetScene)
                {
                    ConvertScene(assetScene->GetScene());
                    assetScene->Save();
                }
            }
        }
    }

    Angle AdjustAngleByDirection(Angle angle, float direction)
	{
		// [-1; 1] -> [0; 1]
		const float k = (direction + 1.f) * 0.5f;
		return angle - Angle::Degrees(180.f * (1.f - k));
	}

    Math::Matrix MakeMatrix(Math::Vector3 up, Math::Vector3 pos)
    {
        Math::Matrix m;
        m.RotateZ(Math::HalfPI - Angle::FromDirection(up).ToRadian());
        m.Pos() = pos;
        return m;
    }

    eastl::optional<Math::Vector3> GetMovingSurfaceVel(Utils::MaybeCastRes castRes)
	{
		if (castRes->userdata)
		{
			if (MovingPlatform* platform = dynamic_cast<MovingPlatform*>(castRes->userdata->object->GetParent()))
			{
				return eastl::make_optional(platform->velocity);
			}
		}
		return eastl::nullopt;
	}

    void PlaySoundEvent(const char* name, Math::Vector3 *pos)
    {
        GetRoot()->GetSounds()->PlaySoundEvent(name, pos);
    }
}