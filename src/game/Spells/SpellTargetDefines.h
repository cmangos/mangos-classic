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

#ifndef MANGOS_SPELLTARGETDEFINES_H
#define MANGOS_SPELLTARGETDEFINES_H

enum SpellTarget
{
    TARGET_NONE                                        = 0,
    TARGET_UNIT_CASTER                                 = 1,
    TARGET_UNIT_ENEMY_NEAR_CASTER                      = 2,
    TARGET_UNIT_FRIEND_NEAR_CASTER                     = 3,
    TARGET_UNIT_NEAR_CASTER                            = 4,
    TARGET_UNIT_CASTER_PET                             = 5,
    TARGET_UNIT_ENEMY                                  = 6,
    TARGET_ENUM_UNITS_SCRIPT_AOE_AT_SRC_LOC            = 7,
    TARGET_ENUM_UNITS_SCRIPT_AOE_AT_DEST_LOC           = 8,
    TARGET_LOCATION_CASTER_HOME_BIND                   = 9,
    TARGET_LOCATION_CASTER_DIVINE_BIND_NYI             = 10,
    TARGET_PLAYER_NYI                                  = 11,
    TARGET_PLAYER_NEAR_CASTER_NYI                      = 12,
    TARGET_PLAYER_ENEMY_NYI                            = 13,
    TARGET_PLAYER_FRIEND_NYI                           = 14,
    TARGET_ENUM_UNITS_ENEMY_AOE_AT_SRC_LOC             = 15,
    TARGET_ENUM_UNITS_ENEMY_AOE_AT_DEST_LOC            = 16,
    TARGET_LOCATION_DATABASE                           = 17,
    TARGET_LOCATION_CASTER_DEST                        = 18,
    TARGET_UNK_19                                      = 19,
    TARGET_ENUM_UNITS_PARTY_WITHIN_CASTER_RANGE        = 20,
    TARGET_UNIT_FRIEND                                 = 21,
    TARGET_LOCATION_CASTER_SRC                         = 22,
    TARGET_GAMEOBJECT                                  = 23,
    TARGET_ENUM_UNITS_ENEMY_IN_CONE_24                 = 24,
    TARGET_UNIT                                        = 25,
    TARGET_LOCKED                                      = 26,
    TARGET_UNIT_CASTER_MASTER                          = 27,
    TARGET_ENUM_UNITS_ENEMY_AOE_AT_DYNOBJ_LOC          = 28,
    TARGET_ENUM_UNITS_FRIEND_AOE_AT_DYNOBJ_LOC         = 29,
    TARGET_ENUM_UNITS_FRIEND_AOE_AT_SRC_LOC            = 30,
    TARGET_ENUM_UNITS_FRIEND_AOE_AT_DEST_LOC           = 31,
    TARGET_LOCATION_UNIT_MINION_POSITION               = 32,
    TARGET_ENUM_UNITS_PARTY_AOE_AT_SRC_LOC             = 33,
    TARGET_ENUM_UNITS_PARTY_AOE_AT_DEST_LOC            = 34,
    TARGET_UNIT_PARTY                                  = 35,
    TARGET_ENUM_UNITS_ENEMY_WITHIN_CASTER_RANGE        = 36, // TODO: only used with dest-effects - reinvestigate naming
    TARGET_UNIT_FRIEND_AND_PARTY                       = 37,
    TARGET_UNIT_SCRIPT_NEAR_CASTER                     = 38,
    TARGET_LOCATION_CASTER_FISHING_SPOT                = 39,
    TARGET_GAMEOBJECT_SCRIPT_NEAR_CASTER               = 40,
    TARGET_LOCATION_CASTER_FRONT_RIGHT                 = 41,
    TARGET_LOCATION_CASTER_BACK_RIGHT                  = 42,
    TARGET_LOCATION_CASTER_BACK_LEFT                   = 43,
    TARGET_LOCATION_CASTER_FRONT_LEFT                  = 44,
    TARGET_UNIT_FRIEND_CHAIN_HEAL                      = 45,
    TARGET_LOCATION_SCRIPT_NEAR_CASTER                 = 46,
    TARGET_LOCATION_CASTER_FRONT                       = 47,
    TARGET_LOCATION_CASTER_BACK                        = 48,
    TARGET_LOCATION_CASTER_LEFT                        = 49,
    TARGET_LOCATION_CASTER_RIGHT                       = 50,
    TARGET_ENUM_GAMEOBJECTS_SCRIPT_AOE_AT_SRC_LOC      = 51,
    TARGET_ENUM_GAMEOBJECTS_SCRIPT_AOE_AT_DEST_LOC     = 52,
    TARGET_LOCATION_CASTER_TARGET_POSITION             = 53,
    TARGET_ENUM_UNITS_ENEMY_IN_CONE_54                 = 54,
    TARGET_LOCATION_CASTER_FRONT_LEAP                  = 55,
    TARGET_ENUM_UNITS_RAID_WITHIN_CASTER_RANGE         = 56,
    TARGET_UNIT_RAID                                   = 57,
    TARGET_UNIT_RAID_NEAR_CASTER                       = 58,
    TARGET_ENUM_UNITS_FRIEND_IN_CONE                   = 59,
    TARGET_ENUM_UNITS_SCRIPT_IN_CONE_60                = 60,
    TARGET_UNIT_RAID_AND_CLASS                         = 61,
    TARGET_PLAYER_RAID_NYI                             = 62,
    TARGET_LOCATION_UNIT_POSITION                      = 63,

    MAX_SPELL_TARGETS
};

enum SpellCastTargetFlags
{
    TARGET_FLAG_SELF            = 0x00000000,
    TARGET_FLAG_UNUSED1         = 0x00000001,               // not used in any spells as of 2.4.3 (can be set dynamically)
    TARGET_FLAG_UNIT            = 0x00000002,               // pguid
    TARGET_FLAG_UNIT_RAID       = 0x00000004,               // not used in any spells as of 2.4.3 (can be set dynamically) - raid member
    TARGET_FLAG_UNIT_PARTY      = 0x00000008,               // not used in any spells as of 2.4.3 (can be set dynamically) - party member
    TARGET_FLAG_ITEM            = 0x00000010,               // pguid
    TARGET_FLAG_SOURCE_LOCATION = 0x00000020,               // 3xfloat
    TARGET_FLAG_DEST_LOCATION   = 0x00000040,               // 3xfloat
    TARGET_FLAG_UNIT_ENEMY      = 0x00000080,               // CanAttack == true
    TARGET_FLAG_UNIT_ALLY       = 0x00000100,               // CanAssist == true
    TARGET_FLAG_CORPSE_ENEMY    = 0x00000200,               // pguid, CanAssist == false
    TARGET_FLAG_UNIT_DEAD       = 0x00000400,               // skinning-like effects
    TARGET_FLAG_GAMEOBJECT      = 0x00000800,               // pguid, 0 spells in 2.4.3
    TARGET_FLAG_TRADE_ITEM      = 0x00001000,               // pguid, 0 spells
    TARGET_FLAG_STRING          = 0x00002000,               // string, 0 spells
    TARGET_FLAG_LOCKED          = 0x00004000,               // 199 spells, opening object/lock
    TARGET_FLAG_CORPSE_ALLY     = 0x00008000,               // pguid, CanAssist == true
    TARGET_FLAG_UNIT_MINIPET    = 0x00010000,               // pguid, not used in any spells as of 2.4.3 (can be set dynamically)
};

enum SpellTargetImplicitType
{
    TARGET_TYPE_UNKNOWN         = 0,
    TARGET_TYPE_LOCATION_SRC,
    TARGET_TYPE_LOCATION_DEST,
    TARGET_TYPE_GAMEOBJECT,
    TARGET_TYPE_UNIT,
    TARGET_TYPE_PLAYER,
    TARGET_TYPE_CORPSE,
    TARGET_TYPE_LOCK,
    // effect custom types
    TARGET_TYPE_NONE,
    TARGET_TYPE_ITEM, // TARGET_FLAG_ITEM
    TARGET_TYPE_UNIT_DEST,
    TARGET_TYPE_DYNAMIC, // changes based on other targets
    TARGET_TYPE_DYNAMIC_ANY, // cant be purely targetless
    TARGET_TYPE_SPECIAL_DEST, // overrides existing targets with custom logic - Persistent AA
    TARGET_TYPE_SPECIAL_UNIT, // Area Auras
    TARGET_TYPE_SPECIAL_SCRIPT,
};

enum SpellTargetEnumerator
{
    TARGET_ENUMERATOR_UNKNOWN   = 0,
    TARGET_ENUMERATOR_SINGLE,
    TARGET_ENUMERATOR_CHAIN,                                  // Sorted list
    TARGET_ENUMERATOR_AOE,                                    // Unsorted list
    TARGET_ENUMERATOR_CONE,                                   // Unsorted list
};

enum SpellTargetFilter
{
    TARGET_NEUTRAL              = 0,
    TARGET_HARMFUL,
    TARGET_HELPFUL,
    TARGET_PARTY,                                            // Same party (subgroup) only
    TARGET_GROUP,                                            // Same group only
    TARGET_SCRIPT,
};

#endif
