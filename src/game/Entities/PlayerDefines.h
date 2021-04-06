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

#ifndef _PLAYER_DEF_H
#define _PLAYER_DEF_H

#include "Globals/SharedDefines.h"
#include "Server/DBCStructure.h"
#include <set>
#include <list>
#include <vector>
#include <string>

class Aura;

enum SpellModOp
{
    SPELLMOD_DAMAGE                     = 0,
    SPELLMOD_DURATION                   = 1,
    SPELLMOD_THREAT                     = 2,
    SPELLMOD_ATTACK_POWER               = 3,
    SPELLMOD_CHARGES                    = 4,
    SPELLMOD_RANGE                      = 5,
    SPELLMOD_RADIUS                     = 6,
    SPELLMOD_CRITICAL_CHANCE            = 7,
    SPELLMOD_ALL_EFFECTS                = 8,
    SPELLMOD_NOT_LOSE_CASTING_TIME      = 9,
    SPELLMOD_CASTING_TIME               = 10,
    SPELLMOD_COOLDOWN                   = 11,
    SPELLMOD_SPEED                      = 12,
    SPELLMOD_UNK1                       = 13, // unused
    SPELLMOD_COST                       = 14,
    SPELLMOD_CRIT_DAMAGE_BONUS          = 15,
    SPELLMOD_RESIST_MISS_CHANCE         = 16,
    SPELLMOD_JUMP_TARGETS               = 17,
    SPELLMOD_CHANCE_OF_SUCCESS          = 18, // Only used with SPELL_AURA_ADD_FLAT_MODIFIER and affects proc spells
    SPELLMOD_ACTIVATION_TIME            = 19,
    SPELLMOD_EFFECT_PAST_FIRST          = 20,
    SPELLMOD_GLOBAL_COOLDOWN            = 21,
    SPELLMOD_DOT                        = 22,
    SPELLMOD_HASTE                      = 23,
    SPELLMOD_SPELL_BONUS_DAMAGE         = 24,
    SPELLMOD_UNK2                       = 25, // unused
    // SPELLMOD_FREQUENCY_OF_SUCCESS    = 26,                // not used in 2.4.3
    SPELLMOD_MULTIPLE_VALUE             = 27,
    SPELLMOD_RESIST_DISPEL_CHANCE       = 28,
    MAX_SPELLMOD                        = 32,
};

// Note: SPELLMOD_* values is aura types in fact
enum SpellModType
{
    SPELLMOD_FLAT = 107,                      // SPELL_AURA_ADD_FLAT_MODIFIER
    SPELLMOD_PCT = 108                       // SPELL_AURA_ADD_PCT_MODIFIER
};

// Spell modifier (used for modify other spells)
struct SpellModifier
{
    static uint64 modCounter;

    uint64 GetModId() { return modCounter++; }

    SpellModifier() : charges(0), priority(0), isFinite(false) {}

    SpellModifier(SpellModOp _op, SpellModType _type, int32 _value, uint32 _spellId, uint64 _mask, int32 _priority = 0, int16 _charges = 0)
        : op(_op), type(_type), charges(_charges), value(_value), mask(_mask), spellId(_spellId), priority(_priority), isFinite(_charges>0), modId(GetModId())
    {}

    SpellModifier(SpellModOp _op, SpellModType _type, int32 _value, uint32 _spellId, ClassFamilyMask _mask, int32 _priority = 0, int16 _charges = 0)
        : op(_op), type(_type), charges(_charges), value(_value), mask(_mask), spellId(_spellId), priority(_priority), isFinite(_charges>0), modId(GetModId())
    {}

    SpellModifier(SpellModOp _op, SpellModType _type, int32 _value, SpellEntry const* spellEntry, SpellEffectIndex eff, int32 _priority = 0, int16 _charges = 0);

    SpellModifier(SpellModOp _op, SpellModType _type, int32 _value, Aura const* aura, int32 _priority = 0, int16 _charges = 0);

    bool isAffectedOnSpell(SpellEntry const* spell) const;

    SpellModOp   op : 8;
    SpellModType type : 8;
    int16 charges : 16;
    int32 value;
    ClassFamilyMask mask;
    uint32 spellId;
    int32 priority;
    bool isFinite;
    uint64 modId;
};

struct SpellModifierComparator
{
    bool operator() (SpellModifier* a, SpellModifier* b) { return (a->priority > b->priority); }
};

typedef std::list<SpellModifier*> SpellModList;
typedef std::vector<SpellModifier*> SpellModVector;

struct SpellModifierPair
{
    uint32 spellId;
    uint64 modId;
    SpellModifierPair() : spellId(0), modId(0) {}
    SpellModifierPair(uint32 _spellId, uint64 _modifierId) : spellId(_spellId), modId(_modifierId) {}
    bool operator<(const SpellModifierPair &rhs) const
    {
        return spellId < rhs.spellId;
    }
    bool operator>(const SpellModifierPair &rhs) const
    {
        return spellId > rhs.spellId;
    }
};

// typedef std::set<SpellModifierPair>  ConsumedSpellModifiers;

#endif