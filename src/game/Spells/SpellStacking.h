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

#ifndef _SPELLSTACKING_H
#define _SPELLSTACKING_H

#include "Server/DBCEnums.h"
#include "Platform/Define.h"
#include <map>
#include <vector>
#include <string>

struct SpellEntry;
class Unit;

enum class SpellGroupRule
{
    UNIQUE              = 1,
    UNIQUE_PER_CASTER   = 2,
    MAX                 = 3
};

enum class SpellGroupId
{
    WELL_FED = 3, // hardcoded for current compliance with old code
    MAX = 64,
};

struct SpellGroup
{
    uint32 Id;
    SpellGroupRule rule;
    std::string name;
    std::vector<uint32> spellIds;
};

struct SpellGroupSpellData
{
    uint64 mask;
    SpellGroupRule rule;

    SpellGroupSpellData() : mask(0), rule((SpellGroupRule)0) {}
};

class SpellStacker
{
    public:
        SpellStacker() {}
        ~SpellStacker() {}

        static SpellStacker& Instance();

        void LoadSpellGroups();

        bool IsStackableAuraEffect(SpellEntry const* entry, SpellEntry const* entry2, SpellEffectIndex effIdx, Unit* target) const;
        bool IsStackableSpell(SpellEntry const* entry, SpellEntry const* entry2, Unit* target) const;
        bool IsSpellStackableWithSpell(SpellEntry const* entry1, SpellEntry const* entry2, Unit* target) const;
        bool IsSpellStackableWithSpellForDifferentCasters(SpellEntry const* entry1, SpellEntry const* entry2, bool isSameChain, Unit* target) const;

        SpellGroupSpellData const* GetSpellGroupDataForSpell(uint32 spellId) const;

    private:
        std::map<uint32, SpellGroup> m_spellGroups;
        std::map<uint32, SpellGroupSpellData> m_spellGroupSpellData;
};

#define sSpellStacker SpellStacker::Instance()

#endif