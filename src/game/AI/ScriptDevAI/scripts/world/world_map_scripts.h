/* This file is part of the ScriptDev2 Project. See AUTHORS file for Copyright information
 * This program is free software licensed under GPL version 2
 * Please see the included DOCS/LICENSE.TXT for more information */

#ifndef DEF_WORLD_MAP_SCRIPTS_H
#define DEF_WORLD_MAP_SCRIPTS_H

enum
{
    // Elemental invasions
    NPC_THE_WINDREAVER          = 14454,
    NPC_PRINCESS_TEMPESTRIA     = 14457,
    NPC_BARON_CHARR             = 14461,
    NPC_AVALANCHION             = 14464,

    GO_EARTH_RIFT               = 179664,
    GO_WATER_RIFT               = 179665,
    GO_FIRE_RIFT                = 179666,
    GO_AIR_RIFT                 = 179667,
};

enum ElementalInvasionIndexes
{
    ELEMENTAL_EARTH             = 0,
    ELEMENTAL_AIR               = 1,
    ELEMENTAL_FIRE              = 2,
    ELEMENTAL_WATER             = 3,
    MAX_ELEMENTS
};

#endif
