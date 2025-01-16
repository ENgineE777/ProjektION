#pragma once

#include "EASTL/string.h"
#include "Characters/Player/ThingAbilities.h"
#include "Characters/Player/Cheats.h"

namespace Orin::Overkill
{
    struct EventOnRestart {};

    struct EventOnFinishLevel {};

    struct EventOnThingAbilityAvailabiltyChange
    {
        ThingAbility ability;
        bool value;
    };

    struct EventOnCheatStateChange
    {
        Cheat cheat;
        bool value;
    };

    struct CmdAllowCameraMove
    {
        bool allow;
    };

    struct CmdAddShakeTrauma
    {
        float trauma;
    };

    struct CmdShowTunnelHint
    {
        int hint;
        Math::Vector3 pos;
    };

    struct CmdShowThrowHint
    {
        Math::Vector3 pos;
    };

    struct CmdShowSurfaceHint
    {
        int hint;
        Math::Vector3 pos;
    };

    struct EventOnCheckpointReached
    {
        Math::Vector3 pos;
        uint32_t checpointID = 0;
    };

    struct CmdTeleportState
    {
        bool available;
        Math::Vector3 dest;
    };
}