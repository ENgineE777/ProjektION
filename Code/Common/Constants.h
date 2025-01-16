#pragma once

namespace Orin::Overkill
{
    static const float TILE_SIZE      = 36.f;
    static const float TILE_HALF_SIZE = TILE_SIZE * 0.5f;
    static const float TILE_SCALE     = 0.5625f; // 36.f/64.f   

	enum PhysGroup : uint32_t
	{
		World = 1,
		TunnelDoor = 2,
		Player_ = 4,
		Enemy_ = 8,
		LevelObjects = 16,
		HitBox = 32,
		DeathZone = 64,
		BodyPart = 128,
		MovementBlock = 256,

		WorldDoors = World | TunnelDoor,
		WorldDoorsPlayer = WorldDoors | Player_,
		WorldDoorsHitBox = WorldDoors | HitBox,
		WorldDoorsEnemies = WorldDoors | Enemy_,
		WorldDoorsDeathZones = WorldDoors | DeathZone
	};
}
