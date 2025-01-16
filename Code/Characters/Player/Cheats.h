
#pragma once

namespace Orin::Overkill
{
	enum Cheat
	{
		PlayerInvisibleToEnemies,
		PlayerInvulnerable,
		PlayerAddInvincibile,
		PlayerAddThunder,
		CheatMaxCount
	};

	static const char* CheatNames[] =
	{
		"Player Invisible To Enemies",
		"Player Invulnerable",
		"Player Combo Invincibile",
		"Player Combo Thunder",
		"Empty",
	};
}
