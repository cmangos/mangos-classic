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

#ifndef MANGOS_SPELLTARGETS_H
#define MANGOS_SPELLTARGETS_H

#include <map>

#include "Spells/SpellTargetDefines.h"
#include "Spells/SpellEffectDefines.h"
#include "Server/DBCEnums.h"
#include "Platform/Define.h"

#include <map>
#include <utility>

struct SpellTargetInfo
{
    char const* name;
    SpellTargetImplicitType type;
    SpellTargetFilter filter;
    SpellTargetEnumerator enumerator;
    SpellTargetLOS los;

    SpellTargetInfo(char const* name = "", SpellTargetImplicitType type = TARGET_TYPE_UNKNOWN, SpellTargetFilter filter = TARGET_NEUTRAL, SpellTargetEnumerator enumerator = TARGET_ENUMERATOR_UNKNOWN, SpellTargetLOS los = TARGET_LOS_CASTER);
};

extern SpellTargetInfo SpellTargetInfoTable[MAX_SPELL_TARGETS];

struct SpellTargetingData
{
    SpellTargetImplicitType implicitType[MAX_EFFECT_INDEX];
    uint8 targetMask[MAX_EFFECT_INDEX][2] = { {1, 1}, {2, 2}, {4, 4} };
    std::pair<bool, bool> ignoredTargets[MAX_EFFECT_INDEX] = { {false, false}, {false, false}, {false, false} };
    SpellTargetFilterScheme filteringScheme[MAX_EFFECT_INDEX][2] = { {SCHEME_RANDOM, SCHEME_RANDOM}, {SCHEME_RANDOM, SCHEME_RANDOM}, {SCHEME_RANDOM, SCHEME_RANDOM} };
};

class SpellTargetMgr // thread safe
{
    public:
        static void Initialize(); // precalculates ignored targets and dynamic effect targeting
        static SpellTargetingData& GetSpellTargetingData(uint32 spellId);
        static bool CanEffectBeFilledWithMask(uint32 spellId, uint32 effIdx, uint32 mask);
        static bool CanEffectConsumeTarget(SpellTargetImplicitType targetType, SpellTargetImplicitType effectTargetType);

        // temporary helpers
        static float GetJumpRadius(uint32 spellId);
        static SpellTargetFilterScheme GetSpellTargetingFilterScheme(SpellTargetFilterScheme oldScheme, uint32 spellId);
    private:
        static std::map<uint32, SpellTargetingData> spellTargetingData;
};

#endif
