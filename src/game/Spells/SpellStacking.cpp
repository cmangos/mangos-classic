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

#include "Spells/SpellStacking.h"
#include "Server/DBCStructure.h"
#include "Entities/Unit.h"
#include "Database/DatabaseEnv.h"
#include "Spells/SpellMgr.h"
#include "Server/SQLStorages.h"

SpellStacker& SpellStacker::Instance()
{
    static SpellStacker spellStacker;
    return spellStacker;
}

void SpellStacker::LoadSpellGroups()
{
    m_spellGroups.clear();
    m_spellGroupSpellData.clear();

    uint32 count = 0;

    auto queryResult = WorldDatabase.Query("SELECT Id, Rule, Name FROM spell_group");

    if (queryResult)
    {
        BarGoLink bar(queryResult->GetRowCount());
        do
        {
            Field* fields = queryResult->Fetch();

            bar.step();

            SpellGroup group;
            group.Id = fields[0].GetUInt32();
            group.rule = (SpellGroupRule)fields[1].GetUInt32();
            group.name = fields[2].GetCppString();

            if (SpellGroupRule::MAX <= group.rule)
            {
                sLog.outErrorDb("spell_group (Id: %u) has invalid rule %u.", group.Id, group.rule);
                continue;
            }

            m_spellGroups.emplace(group.Id, group);
        }
        while (queryResult->NextRow());
    }

    sLog.outString(">> Loaded %u spell groups", count);
    sLog.outString();

    count = 0;

    queryResult = WorldDatabase.Query("SELECT Id, SpellId FROM spell_group_spell");

    if (queryResult)
    {
        BarGoLink bar(queryResult->GetRowCount());
        do
        {
            Field* fields = queryResult->Fetch();

            bar.step();

            uint32 groupId = fields[0].GetUInt32();
            uint32 spellId = fields[1].GetUInt32();

            auto existingItr = m_spellGroups.find(groupId);
            if (existingItr == m_spellGroups.end())
            {
                sLog.outErrorDb("`spell_group_spell` group id %u does not exist", groupId);
                continue;
            }

            SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(spellId);
            if (!spellInfo)
            {
                sLog.outErrorDb("Spell (ID:%u) listed in `spell_group_spell` group id %u does not exist.", spellId, groupId);
                continue;
            }

            existingItr->second.spellIds.push_back(spellId);
        }
        while (queryResult->NextRow());
    }

    for (auto& group : m_spellGroups)
    {
        for (uint32 spellId : group.second.spellIds)
        {
            auto& maskData = m_spellGroupSpellData[spellId];
            maskData.mask |= (uint64(1) << (group.second.Id - 1)); // start from Id 1
            if (maskData.rule == (SpellGroupRule)0)
                maskData.rule = group.second.rule;
        }
    }

    sLog.outString(">> Loaded %u spell group spells", count);
    sLog.outString();
}

bool SpellStacker::IsStackableAuraEffect(SpellEntry const* entry, SpellEntry const* entry2, SpellEffectIndex effIdx, Unit* target) const
{
    const uint32 aura = entry->EffectApplyAuraName[effIdx];
    // Ignore non-aura effects
    if (!aura)
        return true;

    // Get first similar - second spell's same aura with the same sign
    uint32 similar = EFFECT_INDEX_0;
    for (uint32 e = EFFECT_INDEX_0; e < MAX_EFFECT_INDEX; ++e)
    {
        if (IsSimilarAuraEffect(entry, effIdx, entry2, e))
        {
            similar = e;
            break;
        }
        else if (e == (MAX_EFFECT_INDEX - 1))
            return true; // No similarities
    }

    // Special case for potions
    if (entry->SpellFamilyName == SPELLFAMILY_POTION || entry2->SpellFamilyName == SPELLFAMILY_POTION)
        return true;

    // Special rule for food buffs - TODO: Remove one day
    auto firstItr = m_spellGroupSpellData.find(entry->Id);
    auto secondItr = m_spellGroupSpellData.find(entry2->Id);
    if (firstItr != m_spellGroupSpellData.end() && secondItr != m_spellGroupSpellData.end())
    {
        int wellFedIndex = 1 << (((int)SpellGroupId::WELL_FED) - 1);
        if ((firstItr->second.mask & wellFedIndex) != 0 && (secondItr->second.mask & wellFedIndex) == 0)
            return true;
    }

    // Short alias
    const bool positive = (IsPositiveEffect(entry, effIdx));
    const bool related = (entry->SpellFamilyName == entry2->SpellFamilyName);
    const bool siblings = (entry->SpellFamilyFlags == entry2->SpellFamilyFlags);
    const bool player = (entry->SpellFamilyName && !entry->SpellFamilyFlags.Empty());
    const bool multirank = (related && siblings && player);
    const bool instance = (entry->Id == entry2->Id || multirank);
    const bool icon = (entry->SpellIconID == entry2->SpellIconID); // Old bad practice, but a few old spells detection may still depend on it
    const bool visual = (entry->SpellVisual == entry2->SpellVisual); // Old bad practice, but a few old spells detection may still depend on it

    // If aura makes spell not multi-instanceable (do not stack the same spell id or ranks of this spell)
    bool nonmui = false;

    // Stack rules
    switch (aura)
    {
        // Dummy/Triggers
        case SPELL_AURA_DUMMY:
        case SPELL_AURA_PERIODIC_TRIGGER_SPELL:
        {
            if (!related)
                break;
            switch (entry->SpellFamilyName)
            {
                case SPELLFAMILY_SHAMAN:
                    if (entry->IsFitToFamilyMask(uint64(0x200)) && multirank)
                        return true; // Shaman Reincarnation (Passive) and Twisting Nether
                    break;
                case SPELLFAMILY_DRUID:
                    if (entry->IsFitToFamilyMask(uint64(0x80)) && multirank)
                        return true; // Tranquility
                    break;
            }
            break;
        }
        // DoT
        case SPELL_AURA_PERIODIC_LEECH:
        case SPELL_AURA_PERIODIC_MANA_LEECH:
            if (target && target->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED))
                return false;
            break;
        case SPELL_AURA_PERIODIC_DAMAGE:
        case SPELL_AURA_PERIODIC_DAMAGE_PERCENT:
        case SPELL_AURA_POWER_BURN_MANA:
            return true;
            break;
            // HoT
        case SPELL_AURA_OBS_MOD_HEALTH:
        case SPELL_AURA_OBS_MOD_MANA:
            return true;
            break;
            // Raid debuffs: Hunter's Mark and Expose Weakness stack with each other, but not itself
        case SPELL_AURA_RANGED_ATTACK_POWER_ATTACKER_BONUS:
        case SPELL_AURA_MELEE_ATTACK_POWER_ATTACKER_BONUS:
            return (!related || !siblings);
            break;
        case SPELL_AURA_MOD_ATTACK_POWER:
            // Attack Power debuffs logic: Do not stack Curse of Weakness, Demoralizing Roars/Shouts
            if (!positive && entry->EffectBasePoints[effIdx] < 1 && entry2->EffectBasePoints[similar] < 1)
                return (!entry->SpellFamilyName && !entry2->SpellFamilyName);
            break;
            // Armor & Resistance buffs and debuffs logic
        case SPELL_AURA_MOD_RESISTANCE:
        {
            if (entry->EffectMiscValue[effIdx] != entry2->EffectMiscValue[similar])
                break;
            if (positive)
            {
                // Used as a non-zero type in this context
                const bool type = (entry->Dispel && entry->Dispel == entry2->Dispel);
                const bool attacktable = (entry->DmgClass && entry->DmgClass == entry2->DmgClass);
                if ((attacktable || type) && !entry->SpellFamilyName && !entry2->SpellFamilyName)
                    return false; // Do not stack scrolls with other srolls and some procs (such as Hyjal ring)
                if (player && related && siblings && entry->HasAttribute(SPELL_ATTR_EX3_DOT_STACKING_RULE))
                    return true;
            }
            else
            {
                const bool type = (entry->Dispel == entry2->Dispel);
                const bool prevention = (entry->PreventionType && entry->PreventionType == entry2->PreventionType);
                if (type && prevention)
                    return false;
            }
            break;
        }
        case SPELL_AURA_MOD_RESISTANCE_PCT:
        {
            if (entry->EffectMiscValue[effIdx] != entry2->EffectMiscValue[similar])
                break;
            if (positive && entry->Dispel && entry->Dispel == entry2->Dispel)
                return false; // Inspiration / Ancestral Fortitude
            nonmui = true;
            break;
        }
        // By default base stats cannot stack if they're similar
        case SPELL_AURA_MOD_STAT:
        {
            if (entry->Id == 5320 || entry2->Id == 5320) // Echeyakee's Grace - stacks with everything
                return true;
            if (entry->Id == 15366 || entry2->Id == 15366) // Songflower Serenade - stacks with everything
                return true;
            if (entry->EffectMiscValue[effIdx] != entry2->EffectMiscValue[similar])
                break;
            if (positive)
            {
                // Used as a non-zero type in this context
                const bool type = (entry->Dispel && entry->Dispel == entry2->Dispel);
                const bool family = (entry->SpellFamilyName || entry2->SpellFamilyName);
                const bool uncategorized = (!entry->SpellFamilyName || !entry2->SpellFamilyName);
                if (type && family && uncategorized)
                    return false; // Do not stack player buffs with scrolls
                if (!entry->SpellFamilyName && !entry2->SpellFamilyName)
                {
                    if (type)
                        return false; // Do not stack scrolls and other non-player buffs with each other
                    if (entry->HasAttribute(SPELL_ATTR_EX2_NOT_AN_ACTION) && entry2->HasAttribute(SPELL_ATTR_EX2_NOT_AN_ACTION))
                        return false; // FIXME: Cozy fire hack
                }
            }
            else
            {
                // Debuffs are based on patch notes decription of Shrink effects stacking
                // Used as a non-zero type in this context
                const bool type = (entry->Dispel && entry->Dispel == entry2->Dispel);
                if (type)
                    return false;
                nonmui = true;
            }
            break;
        }
        case SPELL_AURA_MOD_TOTAL_STAT_PERCENTAGE:
        case SPELL_AURA_MOD_PERCENT_STAT:
            nonmui = true;
            break;
        case SPELL_AURA_MOD_INCREASE_HEALTH_PERCENT:
            nonmui = true;
            break;
        case SPELL_AURA_MOD_INCREASE_HEALTH:
            break;
        case SPELL_AURA_MOD_HEALING_DONE:
        case SPELL_AURA_MOD_HEALING_PCT:
            // Do not stack similar debuffs: Mortal Strike, Aimed Shot, Hex of Weakness
            if (!positive)
                return (entry->EffectMiscValue[effIdx] == entry2->EffectMiscValue[similar]);
            break;
        case SPELL_AURA_MOD_MELEE_HASTE:
        case SPELL_AURA_MOD_RANGED_HASTE:
        case SPELL_AURA_MOD_DAMAGE_DONE:
        case SPELL_AURA_MOD_DAMAGE_PERCENT_DONE: // Ferocious Inspiration, Shadow Embrace
            if (positive)
                return true;
            nonmui = true;
            break;
        case SPELL_AURA_MOD_SPELL_CRIT_CHANCE:
        case SPELL_AURA_MOD_SPELL_HIT_CHANCE:
            if (player && related && entry->Id == entry2->Id && entry->HasAttribute(SPELL_ATTR_PASSIVE))
                return true; // TBC: Party auras whitelist for Totem of Wrath
            nonmui = true;
            break;
        case SPELL_AURA_MOD_DAMAGE_TAKEN:
        case SPELL_AURA_MOD_DAMAGE_PERCENT_TAKEN:
        case SPELL_AURA_MOD_DECREASE_SPEED: // Bonus stacking handled by core
        case SPELL_AURA_MOD_INCREASE_SPEED: // Bonus stacking handled by core
        case SPELL_AURA_MOD_HEALTH_REGEN_PERCENT:
        case SPELL_AURA_PREVENTS_FLEEING:
            nonmui = true;
            break;
        case SPELL_AURA_PROC_TRIGGER_SPELL:
            if (instance && !icon)
                // Exception: Judgement of Light and Judgement of Wisdom have exact same spell family flags
                // Comparing icons is the fastest (but hacky) way to destinguish between two without poking spell chain
                break;
            nonmui = true;
            break;
        case SPELL_AURA_SPELL_MAGNET: // Party auras whitelist for Grounding Totem
            return true; // Always stacking auras
            break;
        case SPELL_AURA_MOD_POSSESS: // Mind control derrivatives
        case SPELL_AURA_MOD_POSSESS_PET: // Eyes of the beast
        case SPELL_AURA_MOD_CHARM: // Temporary Enslave/Tame derrivatives
        case SPELL_AURA_AOE_CHARM: // Mass charm by Boss/NPC
        case SPELL_AURA_DAMAGE_SHIELD: // Damage shields: Fire Shield, Thorns...
        case SPELL_AURA_MOD_SHAPESHIFT: // Forms and stances
        case SPELL_AURA_MOUNTED: // Mount
        case SPELL_AURA_EMPATHY: // Beast Lore
            // TODO: Make these exclusive rather than unstackable physically and move to nonmui:
        case SPELL_AURA_MOD_CASTING_SPEED_NOT_STACK: // Heroism, Bloodlust, Icy Veins, Power Infusion
        case SPELL_AURA_MOD_MOUNTED_SPEED_NOT_STACK: // Mounted Speed effects
            return false; // Never stacking auras
            break;
        case SPELL_AURA_MOD_DETECT_RANGE: // Never stack
            return false;
        case SPELL_AURA_PERIODIC_ENERGIZE:
            break;
    }
    if (nonmui && instance && !IsChanneledSpell(entry) && !IsChanneledSpell(entry2))
        return false; // Forbids multi-ranking and multi-application on rule, exclude channeled spells (like Mind Flay)

    if (multirank && IsPositiveSpell(entry) && IsPositiveSpell(entry2))
        return false; // Forbids multi-ranking for positive spells

    return true;
}

bool SpellStacker::IsStackableSpell(SpellEntry const* entry, SpellEntry const* entry2, Unit* target) const
{
    for (uint32 i = EFFECT_INDEX_0; i < MAX_EFFECT_INDEX; ++i)
    {
        if (entry->Effect[i] && entry->EffectApplyAuraName[i] && !IsStackableAuraEffect(entry, entry2, SpellEffectIndex(i), target))
            return false;
    }
    return true;
}

bool SpellStacker::IsSpellStackableWithSpell(const SpellEntry* entry1, const SpellEntry* entry2, Unit* target) const
{
    if (!entry1 || !entry2)
        return true;

    if (entry1 == entry2 && entry1->HasAttribute(SPELL_ATTR_EX_AURA_UNIQUE))
        return false;

    // Uncancellable spells are expected to be persistent at all times
    if (entry1->HasAttribute(SPELL_ATTR_NO_AURA_CANCEL) || entry2->HasAttribute(SPELL_ATTR_NO_AURA_CANCEL))
        return true;

    // Allow stacking passive and active spells
    if (entry1->HasAttribute(SPELL_ATTR_PASSIVE) != entry2->HasAttribute(SPELL_ATTR_PASSIVE))
        return true;

    return IsStackableSpell(entry1, entry2, target);
}

bool SpellStacker::IsSpellStackableWithSpellForDifferentCasters(SpellEntry const* entry1, SpellEntry const* entry2, bool isSameChain, Unit* target) const
{
    if (!entry1 || !entry2)
        return true;

    // If spells are two instances of the same spell, check attribute first, and formal aura holder stacking rules after
    if (entry1 == entry2)
        return (entry1->HasAttribute(SPELL_ATTR_EX3_DOT_STACKING_RULE) || IsSpellStackableWithSpell(entry1, entry2, target));

    // If spells are in the same spell chain
    if (isSameChain)
    {
        // Both ranks have attribute, allow stacking
        if (entry1->HasAttribute(SPELL_ATTR_EX3_DOT_STACKING_RULE) && entry2->HasAttribute(SPELL_ATTR_EX3_DOT_STACKING_RULE))
            return true;
    }

    // By default, check formal aura holder stacking rules
    return IsSpellStackableWithSpell(entry1, entry2, target);
}

SpellGroupSpellData const* SpellStacker::GetSpellGroupDataForSpell(uint32 spellId) const
{
    auto itr = m_spellGroupSpellData.find(spellId);
    if (itr == m_spellGroupSpellData.end())
        return nullptr;
    
    return &(itr->second);
}
