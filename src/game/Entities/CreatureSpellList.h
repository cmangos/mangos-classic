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

#ifndef CREATURE_SPELL_LIST_H
#define CREATURE_SPELL_LIST_H

#include "Platform/Define.h"
#include <map>
#include <string>

enum SpellListTargetingType
{
    SPELL_LIST_TARGETING_HARDCODED = 0,
    SPELL_LIST_TARGETING_ATTACK    = 1,
    SPELL_LIST_TARGETING_SUPPORT   = 2,
    SPELL_LIST_TARGETING_MAX
};

enum SpellListTargetingHardcoded
{
    SPELL_LIST_TARGET_NONE    = 0,
    SPELL_LIST_TARGET_CURRENT = 1,
    SPELL_LIST_TARGET_SELF    = 2,
    SPELL_LIST_TARGET_DISPELLABLE_FRIENDLY = 3,
    SPELL_LIST_TARGET_DISPELLABLE_FRIENDLY_NO_SELF = 4,
    SPELL_LIST_HARDCODED_MAX  = 100,
};

struct CreatureSpellListTargeting
{
    uint32 Id;
    uint32 Type;
    uint32 Param1;
    uint32 Param2;
    uint32 Param3;
};

enum SpellListFlags
{
    SPELL_LIST_FLAG_SUPPORT_ACTION  = 1,
    SPELL_LIST_FLAG_RANGED_ACTION   = 2, // previously known as main ranged spell in EAI
};

struct CreatureSpellListSpell
{
    uint32 Id; // list id
    uint32 Position;
    uint32 SpellId;
    uint32 Flags;
    CreatureSpellListTargeting* Target;
    uint32 ScriptId;
    uint32 Availability;
    uint32 Probability;
    uint32 InitialMin;
    uint32 InitialMax;
    uint32 RepeatMin;
    uint32 RepeatMax;
};

struct CreatureSpellList
{
    uint32 Id;
    std::string Name;
    uint32 ChanceSupportAction;
    uint32 ChanceRangedAttack;
    bool Disabled; // Compatibility layer until creature_template_spells is removed
    std::map<uint32, CreatureSpellListSpell> Spells;
};

struct CreatureSpellListContainer
{
    std::map<uint32, CreatureSpellList> spellLists;
    std::map<uint32, CreatureSpellListTargeting> targeting;
};

#endif
