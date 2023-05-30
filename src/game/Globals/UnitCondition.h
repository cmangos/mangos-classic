/*
 * This file is part of the CMaNGOS Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef MANGOS_UNIT_CONDITION_H
#define MANGOS_UNIT_CONDITION_H

#include "Common.h"
#include <map>
#include <memory>

enum class UnitCondition
{
    NONE                                     = 0,
    RACE                                     = 1,
    CLASS                                    = 2,
    LEVEL                                    = 3,
    IS_SELF                                  = 4,
    IS_MY_PET                                = 5,
    IS_MASTER                                = 6,
    IS_TARGET                                = 7,
    CAN_ASSIST                               = 8,
    CAN_ATTACK                               = 9,
    HAS_PET                                  = 10,
    HAS_WEAPON                               = 11,
    HEALTH_PERCENT                           = 12,
    MANA_PERCENT                             = 13,
    RAGE_PERCENT                             = 14,
    ENERGY_PERCENT                           = 15,
    COMBO_POINTS                             = 16,
    HAS_HELPFUL_AURA_SPELL                   = 17,
    HAS_HELPFUL_AURA_DISPEL_TYPE             = 18,
    HAS_HELPFUL_AURA_MECHANIC                = 19,
    HAS_HARMFUL_AURA_SPELL                   = 20,
    HAS_HARMFUL_AURA_DISPEL_TYPE             = 21,
    HAS_HARMFUL_AURA_MECHANIC                = 22,
    HAS_HARMFUL_AURA_SCHOOL                  = 23,
    DAMAGE_PHYSICAL                          = 24,
    DAMAGE_HOLY                              = 25,
    DAMAGE_FIRE                              = 26,
    DAMAGE_NATURE                            = 27,
    DAMAGE_FROST                             = 28,
    DAMAGE_SHADOW                            = 29,
    DAMAGE_ARCANE                            = 30,
    IN_COMBAT                                = 31,
    IS_MOVING                                = 32,
    IS_CASTING                               = 33,
    IS_CASTING_SPELL                         = 34,
    IS_CHANNELING                            = 35,
    IS_CHANNELING_SPELL                      = 36,
    NUMBER_OF_MELEE_ATTACKERS                = 37,
    IS_ATTACKING_ME                          = 38,
    RANGE                                    = 39,
    IN_MELEE_RANGE                           = 40,
    PURSUIT_TIME                             = 41,
    HAS_HARMFUL_AURA_CANCELLED_BY_DAMAGE     = 42,
    HAS_HARMFUL_AURA_WITH_PERIODIC_DAMAGE    = 43,
    NUMBER_OF_ENEMIES                        = 44,
    NUMBER_OF_FRIENDS                        = 45,
    THREAT_PHYSICAL                          = 46,
    THREAT_HOLY                              = 47,
    THREAT_FIRE                              = 48,
    THREAT_NATURE                            = 49,
    THREAT_FROST                             = 50,
    THREAT_SHADOW                            = 51,
    THREAT_ARCANE                            = 52,
    IS_INTERRUPTIBLE                         = 53,
    NUMBER_OF_ATTACKERS                      = 54,
    NUMBER_OF_RANGED_ATTACKERS               = 55,
    CREATURE_TYPE                            = 56,
    IS_MELEE_ATTACKING                       = 57,
    IS_RANGED_ATTACKING                      = 58,
    HEALTH                                   = 59,
    SPELL_KNOWN                              = 60,
    HAS_HARMFUL_AURA_EFFECT                  = 61,
    IS_IMMUNE_TO_AREA_OF_EFFECT              = 62,
    IS_PLAYER                                = 63,
    DAMAGE_MAGIC                             = 64,
    DAMAGE_TOTAL                             = 65,
    THREAT_MAGIC                             = 66,
    THREAT_TOTAL                             = 67,
    HAS_CRITTER                              = 68,
    HAS_TOTEM_IN_SLOT_1                      = 69,
    HAS_TOTEM_IN_SLOT_2                      = 70,
    HAS_TOTEM_IN_SLOT_3                      = 71,
    HAS_TOTEM_IN_SLOT_4                      = 72,
    HAS_TOTEM_IN_SLOT_5                      = 73,
    CREATURE_ID                              = 74,
    STRING_ID                                = 75,
    HAS_AURA                                 = 76,
    IS_ENEMY                                 = 77,
    IS_SPEC_MELEE                            = 78,
    IS_SPEC_TANK                             = 79,
    IS_SPEC_RANGED                           = 80,
    IS_SPEC_HEALER                           = 81,
    IS_PLAYER_CONTROLLED_NPC                 = 82,
    IS_DYING                                 = 83,
    PATH_FAIL_COUNT                          = 84,
    UNUSED                                   = 85,
    LABEL                                    = 86,
    MAX
};

enum UnitConditionFlags
{
    UNIT_CONDITION_FLAG_OR = 0x1,
};

class Unit;

struct UnitConditionEntry
{
    int32 Id;
    uint32 Flags;
    uint32 Variable[8];
    uint32 Operation[8];
    int32 Value[8];
};

class UnitConditionMgr
{
    public:
        std::shared_ptr<std::map<int32, UnitConditionEntry>> Load();

        bool Meets(Unit const* source, Unit const* target, int32 Id);

        bool Exists(int32 Id);

        std::shared_ptr<std::map<int32, UnitConditionEntry>> Get() { return m_unitConditions; }
    private:
        bool meets(Unit const* source, Unit const* target, UnitCondition conditionType, uint32 operation, int32 value);
        int32 getConditionValue(Unit const* source, Unit const* target, UnitCondition conditionType, int32 value);

        std::shared_ptr<std::map<int32, UnitConditionEntry>> m_unitConditions;
};

#endif