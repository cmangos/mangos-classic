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

#ifndef _SPELLMGR_H
#define _SPELLMGR_H

// For static or at-server-startup loaded spell data
// For more high level function for sSpellTemplate data

#include "Common.h"
#include "Globals/SharedDefines.h"
#include "Spells/SpellAuraDefines.h"
#include "Spells/SpellTargets.h"
#include "Server/DBCStructure.h"
#include "Server/DBCStores.h"
#include "Entities/DynamicObject.h"
#include "Entities/GameObject.h"
#include "Entities/Corpse.h"
#include "Entities/Unit.h"
#include "Entities/Player.h"
#include "Spells/SpellAuras.h"
#include "Server/SQLStorages.h"
#include "Spells/SpellEffectDefines.h"

#include <map>

class Player;
class Spell;
class Unit;
struct SpellModifier;

// only used in code
enum SpellCategories
{
    SPELLCATEGORY_HEALTH_MANA_POTIONS = 4,
    SPELLCATEGORY_DEVOUR_MAGIC        = 12
};

// Spell clasification
enum SpellSpecific
{
    SPELL_NORMAL,
    SPELL_FOOD,
    SPELL_DRINK,
    SPELL_FOOD_AND_DRINK,
    SPELL_WELL_FED,
    SPELL_FLASK_ELIXIR,
    SPELL_SEAL,
    SPELL_JUDGEMENT,
    SPELL_BLESSING,
    SPELL_AURA,
    SPELL_STING,
    SPELL_ASPECT,
    SPELL_TRACKER,
    SPELL_CURSE,
    SPELL_SOUL_CAPTURE,
    SPELL_MAGE_ARMOR,
    SPELL_WARLOCK_ARMOR,
    SPELL_ELEMENTAL_SHIELD,
};

SpellSpecific GetSpellSpecific(uint32 spellId);

// Different spell properties
inline float GetSpellRadius(SpellRadiusEntry const* radius) { return (radius ? radius->Radius : 0); }
uint32 GetSpellCastTime(SpellEntry const* spellInfo, WorldObject* caster, Spell* spell = nullptr);
uint32 GetSpellCastTimeForBonus(SpellEntry const* spellProto, DamageEffectType damagetype);
float CalculateDefaultCoefficient(SpellEntry const* spellProto, DamageEffectType const damagetype);
inline float GetSpellMinRange(SpellRangeEntry const* range) { return (range ? range->minRange : 0); }
inline float GetSpellMaxRange(SpellRangeEntry const* range) { return (range ? range->maxRange : 0); }
inline uint32 GetSpellRecoveryTime(SpellEntry const* spellInfo) { return spellInfo->RecoveryTime > spellInfo->CategoryRecoveryTime ? spellInfo->RecoveryTime : spellInfo->CategoryRecoveryTime; }
int32 GetSpellDuration(SpellEntry const* spellInfo);
int32 GetSpellMaxDuration(SpellEntry const* spellInfo);
int32 CalculateSpellDuration(SpellEntry const* spellInfo, Unit const* caster = nullptr);
uint16 GetSpellAuraMaxTicks(SpellEntry const* spellInfo);
uint16 GetSpellAuraMaxTicks(uint32 spellId);
WeaponAttackType GetWeaponAttackType(SpellEntry const* spellInfo);

inline bool IsSpellHaveEffect(SpellEntry const* spellInfo, SpellEffects effect)
{
    for (unsigned int i : spellInfo->Effect)
        if (SpellEffects(i) == effect)
            return true;
    return false;
}

inline bool IsAuraApplyEffect(SpellEntry const* spellInfo, SpellEffectIndex effecIdx)
{
    switch (spellInfo->Effect[effecIdx])
    {
        case SPELL_EFFECT_APPLY_AURA:
        case SPELL_EFFECT_PERSISTENT_AREA_AURA:
        case SPELL_EFFECT_APPLY_AREA_AURA_PARTY:
        case SPELL_EFFECT_APPLY_AREA_AURA_PET:
            return true;
    }
    return false;
}

inline bool IsAuraApplyEffects(SpellEntry const* entry, SpellEffectIndexMask mask)
{
    if (!entry)
        return false;
    uint32 emptyMask = 0;
    for (uint32 i = EFFECT_INDEX_0; i < MAX_EFFECT_INDEX; ++i)
    {
        const uint32 current = (1 << i);
        if (entry->Effect[i])
        {
            if ((mask & current) && !IsAuraApplyEffect(entry, SpellEffectIndex(i)))
                return false;
        }
        else
            emptyMask |= current;
    }
    // Check if all queried effects are actually empty
    const bool empty = !(mask & ~emptyMask);
    return !empty;
}

inline bool IsSpellAppliesAura(SpellEntry const* spellInfo, uint32 effectMask = ((1 << EFFECT_INDEX_0) | (1 << EFFECT_INDEX_1) | (1 << EFFECT_INDEX_2)))
{
    for (int i = 0; i < MAX_EFFECT_INDEX; ++i)
        if (effectMask & (1 << i))
            if (IsAuraApplyEffect(spellInfo, SpellEffectIndex(i)))
                return true;

    return false;
}

inline bool IsEffectHandledOnDelayedSpellLaunch(SpellEntry const* spellInfo, SpellEffectIndex effecIdx)
{
    switch (spellInfo->Effect[effecIdx])
    {
        case SPELL_EFFECT_SCHOOL_DAMAGE:
        case SPELL_EFFECT_WEAPON_DAMAGE_NOSCHOOL:
        case SPELL_EFFECT_WEAPON_PERCENT_DAMAGE:
        case SPELL_EFFECT_WEAPON_DAMAGE:
        case SPELL_EFFECT_NORMALIZED_WEAPON_DMG:
            return true;
        default:
            return false;
    }
}

inline bool IsPeriodicRegenerateEffect(SpellEntry const* spellInfo, SpellEffectIndex effecIdx)
{
    switch (AuraType(spellInfo->EffectApplyAuraName[effecIdx]))
    {
        case SPELL_AURA_PERIODIC_ENERGIZE:
        case SPELL_AURA_PERIODIC_HEAL:
        case SPELL_AURA_PERIODIC_HEALTH_FUNNEL:
            return true;
        default:
            return false;
    }
}

inline bool IsSpellHaveAura(SpellEntry const* spellInfo, AuraType aura, uint32 effectMask = (1 << EFFECT_INDEX_0) | (1 << EFFECT_INDEX_1) | (1 << EFFECT_INDEX_2))
{
    for (int i = 0; i < MAX_EFFECT_INDEX; ++i)
        if (effectMask & (1 << i))
            if (AuraType(spellInfo->EffectApplyAuraName[i]) == aura)
                return true;
    return false;
}

inline bool IsSpellLastAuraEffect(SpellEntry const* spellInfo, SpellEffectIndex effecIdx)
{
    for (int i = effecIdx + 1; i < MAX_EFFECT_INDEX; ++i)
        if (spellInfo->EffectApplyAuraName[i])
            return false;
    return true;
}

inline bool IsAuraRemoveOnStacking(SpellEntry const* spellInfo, int32 effIdx) // TODO: extend to all effects
{
    switch (spellInfo->EffectApplyAuraName[effIdx])
    {
        case SPELL_AURA_MOD_INCREASE_ENERGY:
        case SPELL_AURA_MOD_POWER_COST_SCHOOL_PCT:
        case SPELL_AURA_MOD_INCREASE_HEALTH:
            return false;
        default:
            return true;
    }
}

inline bool IsAllowingDeadTarget(SpellEntry const* spellInfo)
{
    return spellInfo->HasAttribute(SPELL_ATTR_EX2_CAN_TARGET_DEAD) || spellInfo->HasAttribute(SPELL_ATTR_PASSIVE) || spellInfo->Targets & (TARGET_FLAG_CORPSE_ENEMY | TARGET_FLAG_UNIT_DEAD | TARGET_FLAG_CORPSE_ALLY);
}

inline bool IsSealSpell(SpellEntry const* spellInfo)
{
    // Collection of all the seal family flags. No other paladin spell has any of those.
    return spellInfo->IsFitToFamily(SPELLFAMILY_PALADIN, uint64(0x000000000A000200));
}

inline bool IsSpellMagePolymorph(uint32 spellid)
{
    // Only mage polymorph bears hidden scripted regeneration
    const SpellEntry* entry = sSpellTemplate.LookupEntry<SpellEntry>(spellid);
    return (entry && entry->SpellFamilyName == SPELLFAMILY_MAGE && (entry->SpellFamilyFlags & uint64(0x1000000)) && IsSpellHaveAura(entry, SPELL_AURA_MOD_CONFUSE));
}

inline bool IsSpellEffectTriggerSpell(const SpellEntry* entry, SpellEffectIndex effIndex)
{
    if (!entry)
        return false;

    switch (entry->Effect[effIndex])
    {
        case SPELL_EFFECT_TRIGGER_MISSILE:
        case SPELL_EFFECT_TRIGGER_SPELL:
            return true;
    }
    return false;
}

inline bool IsSpellEffectTriggerSpellByAura(const SpellEntry* entry, SpellEffectIndex effIndex)
{
    if (!entry || !IsAuraApplyEffect(entry, effIndex))
        return false;

    switch (entry->EffectApplyAuraName[effIndex])
    {
        case SPELL_AURA_PERIODIC_TRIGGER_SPELL:
        case SPELL_AURA_PROC_TRIGGER_SPELL:
        case SPELL_AURA_PROC_TRIGGER_DAMAGE:
            return true;
    }
    return false;
}

inline bool IsSpellEffectAbleToCrit(const SpellEntry* entry, SpellEffectIndex index)
{
    if (!entry || entry->HasAttribute(SPELL_ATTR_EX2_CANT_CRIT))
        return false;

    switch (entry->Effect[index])
    {
        case SPELL_EFFECT_SCHOOL_DAMAGE:
        case SPELL_EFFECT_HEAL:
        case SPELL_EFFECT_WEAPON_DAMAGE_NOSCHOOL:
        case SPELL_EFFECT_WEAPON_PERCENT_DAMAGE:
        case SPELL_EFFECT_WEAPON_DAMAGE:
        case SPELL_EFFECT_NORMALIZED_WEAPON_DMG:
            return true;
        case SPELL_EFFECT_ENERGIZE: // Mana Potion and similar spells, Lay on hands
            return (entry->EffectMiscValue[index] == POWER_MANA && entry->SpellFamilyName && entry->DmgClass);
    }
    return false;
}

inline bool IsSpellAbleToCrit(const SpellEntry* entry)
{
    if (!entry || entry->HasAttribute(SPELL_ATTR_EX2_CANT_CRIT))
        return false;

    for (uint32 i = EFFECT_INDEX_0; i < MAX_EFFECT_INDEX; ++i)
    {
        if (entry->Effect[i] && IsSpellEffectAbleToCrit(entry, SpellEffectIndex(i)))
            return true;
    }
    return false;
}

inline bool IsPassiveSpell(SpellEntry const* spellInfo)
{
    return spellInfo->HasAttribute(SPELL_ATTR_PASSIVE);
}

inline bool IsPassiveSpell(uint32 spellId)
{
    const SpellEntry* entry = sSpellTemplate.LookupEntry<SpellEntry>(spellId);
    return (entry && IsPassiveSpell(entry));
}

inline bool IsPassiveSpellStackableWithRanks(SpellEntry const* spellProto)
{
    if (!IsPassiveSpell(spellProto))
        return false;

    return !IsSpellHaveEffect(spellProto, SPELL_EFFECT_APPLY_AURA) && !IsSpellHaveEffect(spellProto, SPELL_EFFECT_APPLY_AREA_AURA_PARTY);
}

inline bool IsAutocastable(SpellEntry const* spellInfo)
{
    return !(spellInfo->HasAttribute(SPELL_ATTR_EX_UNAUTOCASTABLE_BY_CHARMED) || spellInfo->HasAttribute(SPELL_ATTR_PASSIVE));
}

inline bool IsAutocastable(uint32 spellId)
{
    SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(spellId);
    if (!spellInfo)
        return false;
    return IsAutocastable(spellInfo);
}

// TODO: Unify with creature_template_spells so that we can set both attack and pet bar visibility
// If true, only gives access to spellbar, and not states and commands
// Works in connection with AI-CanHandleCharm
inline bool IsPossessCharmType(uint32 spellId)
{
    switch (spellId)
    {
        case 999999: // compilation warning suppression
        default: return false;
    }
}

inline bool IsDeathOnlySpell(SpellEntry const* spellInfo)
{
    return spellInfo->HasAttribute(SPELL_ATTR_EX3_CAST_ON_DEAD) || spellInfo->Id == 2584;
}

inline bool IsDeathPersistentSpell(SpellEntry const* spellInfo)
{
    return spellInfo->HasAttribute(SPELL_ATTR_EX3_DEATH_PERSISTENT);
}

inline bool IsNonCombatSpell(SpellEntry const* spellInfo)
{
    return spellInfo->HasAttribute(SPELL_ATTR_CANT_USED_IN_COMBAT);
}

// some creatures should run immediately after being summoned by spell
inline bool IsSpellSetRun(SpellEntry const* spellInfo)
{
    switch (spellInfo->Id)
    {
        case 39163:    // [DND]Rescue Wyvern
            return true;
        default:
            return false;
    }
}

inline bool IsSpellRemovedOnEvade(SpellEntry const* spellInfo)
{
    //TODO: search for potential correct case for Classic
    /*if (IsSpellHaveAura(spellInfo, SPELL_AURA_FLY))
        return false; */

    switch (spellInfo->Id)
    {
        case 588:           // Inner Fire (Rank 1)
        case 3235:          // Rancid Blood
        case 3284:          // Violent Shield
        case 3417:          // Thrash
        case 3418:          // Improved Blocking
        case 3616:          // Poison Proc
        case 3637:          // Improved Blocking III
        case 5111:          // Living Flame Passive
        case 5301:          // Defensive State (DND)
        case 5680:          // Torch Burn
        case 6718:          // Phasing Stealth
        case 6752:          // Weak Poison Proc
        case 7090:          // Bear Form (Shapeshift)
        case 7276:          // Poison Proc
        case 8247:          // Wandering Plague
        case 8279:          // Stealth Detection
        case 8393:          // Barbs
        case 8601:          // Slowing Poison
        case 8876:          // Thrash
        case 9205:          // Hate to Zero (Hate to Zero)
        case 9460:          // Corrosive Ooze
        case 9941:          // Spell Reflection
        case 10022:         // Deadly Poison
        case 10072:         // Splintered Obsidian
        case 10074:         // Spell Reflection
        case 10095:         // Hate to Zero (Hate to Zero)
        case 11838:         // Hate to Zero (Hate to Zero)
        case 11919:         // Poison Proc
        case 11984:         // Immolate
        case 12099:         // Shield Spike
        case 12246:         // Infected Spine
        case 12529:         // Chilling Touch
        case 12539:         // Ghoul Rot
        case 12546:         // Spitelash (Spitelash)
        case 12556:         // Frost Armor
        case 12627:         // Disease Cloud
        case 12787:         // Thrash
        case 12898:         // Smoke Aura Visual
        case 13299:         // Poison Proc
        case 13767:         // Hate to Zero (Hate to Zero)
        case 16140:         // Exploding Cadaver (Exploding Cadaver)
        case 17327:         // Spirit Particles
        case 17467:         // Unholy Aura
        case 18943:         // Double Attack
        case 19030:         // Bear Form (Shapeshift)
        case 18950:         // Invisibility and Stealth Detection
        case 19194:         // Double Attack
        case 19195:         // Hate to 90% (Hate to 90%)
        case 19396:         // Incinerate (Incinerate)
        case 19626:         // Fire Shield (Fire Shield)
        case 19640:         // Pummel (Pummel)
        case 19817:         // Double Attack
        case 19818:         // Double Attack
        case 21061:         // Putrid Breath
        case 21857:         // Lava Shield
        case 22128:         // Thorns
        case 22735:         // Spirit of Runn Tum
        case 22856:         // Ice Lock (Guard Slip'kik ice trap in Dire Maul)
        case 25592:         // Hate to Zero (Hate to Zero)
        case 26341:         // Saurfang's Rage
        case 27987:         // Unholy Aura
        case 28126:         // Spirit Particles (purple)
        case 29526:         // Hate to Zero (Hate to Zero)
            return false;
        default:
            return true;
    }
}

bool IsExplicitPositiveTarget(uint32 targetA);
bool IsExplicitNegativeTarget(uint32 targetA);

inline bool IsResistableSpell(const SpellEntry* entry)
{
    return (entry->DmgClass != SPELL_DAMAGE_CLASS_NONE && !entry->HasAttribute(SPELL_ATTR_EX4_IGNORE_RESISTANCES));
}

inline bool IsSpellEffectDamage(SpellEntry const& spellInfo, SpellEffectIndex i)
{
    if (!spellInfo.EffectApplyAuraName[i])
    {
        // If its not an aura effect, check for damage effects
        switch (spellInfo.Effect[i])
        {
            case SPELL_EFFECT_SCHOOL_DAMAGE:
            case SPELL_EFFECT_ENVIRONMENTAL_DAMAGE:
            case SPELL_EFFECT_HEALTH_LEECH:
            case SPELL_EFFECT_WEAPON_DAMAGE_NOSCHOOL:
            case SPELL_EFFECT_WEAPON_PERCENT_DAMAGE:
            case SPELL_EFFECT_WEAPON_DAMAGE:
            //   SPELL_EFFECT_POWER_BURN: deals damage for power burned, but its either full damage or resist?
            case SPELL_EFFECT_NORMALIZED_WEAPON_DMG:
                return true;
        }
    }
    else
    {
        // If its an aura effect, check for DoT auras
        switch (spellInfo.EffectApplyAuraName[i])
        {
            case SPELL_AURA_PERIODIC_DAMAGE:
            case SPELL_AURA_PERIODIC_LEECH:
            //   SPELL_AURA_POWER_BURN_MANA: deals damage for power burned, but not really a DoT?
            case SPELL_AURA_PERIODIC_DAMAGE_PERCENT:
                return true;
        }
    }
    return false;
}

inline bool IsSpellEffectDummy(SpellEntry const& spellInfo, SpellEffectIndex i)
{
    if (!spellInfo.EffectApplyAuraName[i])
    {
        switch (spellInfo.Effect[i])
        {
            case SPELL_EFFECT_DUMMY:
                return true;
        }
    }
    else
    {
        switch (spellInfo.EffectApplyAuraName[i])
        {
            case SPELL_AURA_DUMMY:
                return true;
        }
    }
    return false;
}

inline bool IsSpellEffectsDamage(SpellEntry const& spellInfo, uint8 effectMask = EFFECT_MASK_ALL)
{
    for (uint8 i = EFFECT_INDEX_0; (i < MAX_EFFECT_INDEX && effectMask); (++i, (effectMask >>= 1)))
    {
        if (spellInfo.Effect[i] && (effectMask & 1) && !IsSpellEffectDamage(spellInfo, SpellEffectIndex(i)))
            return false;
    }
    return true;
}

inline bool IsBinarySpell(SpellEntry const& spellInfo, uint8 effectMask = EFFECT_MASK_ALL)
{
    // Spell is considered binary if:
    // * (Pre-WotLK): It contains non-damage effects or auras
    // * (WotLK+): It contains no damage effects or auras
    uint8 validmask = 0;    // A bitmask of effects: set bits are valid effects
    uint8 nondmgmask = 0;   // A bitmask of effects: set bits are non-damage effects
    uint8 auramask = 0;     // A bitmask of aura effcts: set bits are auras
    for (uint32 i = EFFECT_INDEX_0; i < MAX_EFFECT_INDEX; ++i)
    {
        const uint8 thisMask = uint8(1 << (i - 1));

        if (!spellInfo.Effect[i] || !(effectMask & thisMask))
            continue;

        if (IsSpellEffectDummy(spellInfo, SpellEffectIndex(i)) || IsSpellEffectTriggerSpell(&spellInfo, SpellEffectIndex(i)))
            continue;

        validmask |= thisMask;

        if (spellInfo.EffectApplyAuraName[i])
            auramask |= thisMask;
        if (!IsSpellEffectDamage(spellInfo, SpellEffectIndex(i)))
            nondmgmask |= thisMask;
    }
    // No valid effects: treat as non-binary
    if (!validmask)
        return false;
    // All effects are non-damage: treat as binary
    // All effects are auras: treat as binary (even pure DoTs are treated as binary on initial application)
    if (nondmgmask == validmask || auramask == validmask)
        return true;
    const uint8 dmgmask = (validmask & ~nondmgmask);
    const uint8 dotmask = (dmgmask & auramask);
    // Just in case, if all damage effects are DoTs: treat as binary
    if (dmgmask == dotmask)
        return true;
    // If we ended up here, we have at least one non-aura damage effect
    // Pre-WotLK: check if at least one non-damage effect hits the same target as damage effect (e.g. Frostbolt) and treat as binary
    if (nondmgmask)
    {
        uint8 directmask = (dmgmask & ~dotmask);
        for (uint8 i = EFFECT_INDEX_0; directmask; (++i, (directmask >>= 1)))
        {
            if (directmask & 1)
            {
                uint8 jmask = nondmgmask;
                for (uint8 j = EFFECT_INDEX_0; jmask; (++j, (jmask >>= 1)))
                {
                    if (jmask & 1)
                    {
                        if (spellInfo.EffectImplicitTargetA[i] == spellInfo.EffectImplicitTargetA[j] &&
                            spellInfo.EffectImplicitTargetB[i] == spellInfo.EffectImplicitTargetB[j])
                        {
                            return true;
                        }
                    }
                }
            }
        }
    }
    return false;
}

inline bool IsCasterSourceTarget(uint32 target)
{
    switch (target)
    {
        case TARGET_UNIT_CASTER:
        case TARGET_UNIT_CASTER_PET:
        case TARGET_ENUM_UNITS_PARTY_WITHIN_CASTER_RANGE:
        case TARGET_ENUM_UNITS_ENEMY_IN_CONE_24:
        case TARGET_UNIT_CASTER_MASTER:
        case TARGET_LOCATION_UNIT_MINION_POSITION:
        case TARGET_ENUM_UNITS_PARTY_AOE_AT_SRC_LOC:
        case TARGET_ENUM_UNITS_PARTY_AOE_AT_DEST_LOC:
        case TARGET_LOCATION_CASTER_FISHING_SPOT:
        case TARGET_LOCATION_CASTER_FRONT_RIGHT:
        case TARGET_LOCATION_CASTER_BACK_RIGHT:
        case TARGET_LOCATION_CASTER_BACK_LEFT:
        case TARGET_LOCATION_CASTER_FRONT_LEFT:
        case TARGET_ENUM_GAMEOBJECTS_SCRIPT_AOE_AT_DEST_LOC:
            return true;
        default:
            break;
    }
    return false;
}

inline bool IsSpellWithScriptUnitTarget(SpellEntry const* spellInfo)
{
    for (int i = 0; i < MAX_EFFECT_INDEX; ++i)
    {
        if (!spellInfo->Effect[i])
            continue;
        switch (spellInfo->EffectImplicitTargetA[i])
        {
            case TARGET_UNIT_SCRIPT_NEAR_CASTER:
                return true;
        }
    }

    return false;
}

inline bool IsSpellWithCasterSourceTargetsOnly(SpellEntry const* spellInfo)
{
    for (int i = 0; i < MAX_EFFECT_INDEX; ++i)
    {
        if (!spellInfo->Effect[i])                          // Skip junk in DBC
            continue;

        uint32 targetA = spellInfo->EffectImplicitTargetA[i];
        if (targetA && !IsCasterSourceTarget(targetA))
            return false;

        uint32 targetB = spellInfo->EffectImplicitTargetB[i];
        if (targetB && !IsCasterSourceTarget(targetB))
            return false;

        if (!targetA && !targetB)
            return false;
    }
    return true;
}

inline bool IsPointEffectTarget(SpellTarget target)
{
    switch (target)
    {
        case TARGET_LOCATION_CASTER_HOME_BIND:
        case TARGET_LOCATION_DATABASE:
        case TARGET_LOCATION_CASTER_SRC:
        case TARGET_LOCATION_SCRIPT_NEAR_CASTER:
        case TARGET_LOCATION_CASTER_TARGET_POSITION:
        case TARGET_LOCATION_UNIT_POSITION:
            return true;
        default:
            break;
    }
    return false;
}

inline bool IsAreaEffectPositiveTarget(SpellTarget target)
{
    switch (target)
    {
        case TARGET_ENUM_UNITS_PARTY_WITHIN_CASTER_RANGE:
        case TARGET_ENUM_UNITS_FRIEND_AOE_AT_SRC_LOC:
        case TARGET_ENUM_UNITS_FRIEND_AOE_AT_DEST_LOC:
        case TARGET_ENUM_UNITS_PARTY_AOE_AT_SRC_LOC:
        case TARGET_ENUM_UNITS_PARTY_AOE_AT_DEST_LOC:
        case TARGET_UNIT_FRIEND_AND_PARTY:
        case TARGET_ENUM_UNITS_RAID_WITHIN_CASTER_RANGE:
        case TARGET_UNIT_RAID_AND_CLASS:
            return true;
        default:
            break;
    }
    return false;
}

inline bool IsAreaEffectTarget(SpellTarget target)
{
    switch (target)
    {
        case TARGET_ENUM_UNITS_SCRIPT_AOE_AT_SRC_LOC:
        case TARGET_ENUM_UNITS_SCRIPT_AOE_AT_DEST_LOC:
        case TARGET_ENUM_UNITS_ENEMY_AOE_AT_SRC_LOC:
        case TARGET_ENUM_UNITS_ENEMY_AOE_AT_DEST_LOC:
        case TARGET_ENUM_UNITS_PARTY_WITHIN_CASTER_RANGE:
        case TARGET_ENUM_UNITS_ENEMY_IN_CONE_24:
        case TARGET_ENUM_UNITS_ENEMY_AOE_AT_DYNOBJ_LOC:
        case TARGET_ENUM_UNITS_FRIEND_AOE_AT_SRC_LOC:
        case TARGET_ENUM_UNITS_FRIEND_AOE_AT_DEST_LOC:
        case TARGET_ENUM_UNITS_PARTY_AOE_AT_SRC_LOC:
        case TARGET_ENUM_UNITS_PARTY_AOE_AT_DEST_LOC:
        case TARGET_UNIT_FRIEND_AND_PARTY:
        case TARGET_ENUM_GAMEOBJECTS_SCRIPT_AOE_AT_DEST_LOC:
        case TARGET_ENUM_UNITS_RAID_WITHIN_CASTER_RANGE:
        case TARGET_UNIT_RAID_AND_CLASS:
            return true;
        default:
            break;
    }
    return false;
}

inline bool IsAreaOfEffectSpell(SpellEntry const* spellInfo)
{
    if (IsAreaEffectTarget(SpellTarget(spellInfo->EffectImplicitTargetA[EFFECT_INDEX_0])) || IsAreaEffectTarget(SpellTarget(spellInfo->EffectImplicitTargetB[EFFECT_INDEX_0])))
        return true;
    if (IsAreaEffectTarget(SpellTarget(spellInfo->EffectImplicitTargetA[EFFECT_INDEX_1])) || IsAreaEffectTarget(SpellTarget(spellInfo->EffectImplicitTargetB[EFFECT_INDEX_1])))
        return true;
    if (IsAreaEffectTarget(SpellTarget(spellInfo->EffectImplicitTargetA[EFFECT_INDEX_2])) || IsAreaEffectTarget(SpellTarget(spellInfo->EffectImplicitTargetB[EFFECT_INDEX_2])))
        return true;
    return false;
}

inline bool IsAreaAuraEffect(uint32 effect)
{
    if (effect == SPELL_EFFECT_APPLY_AREA_AURA_PARTY    ||
            effect == SPELL_EFFECT_APPLY_AREA_AURA_PET)
        return true;
    return false;
}

inline bool HasAreaAuraEffect(SpellEntry const* spellInfo)
{
    for (unsigned int i : spellInfo->Effect)
        if (IsAreaAuraEffect(i))
            return true;
    return false;
}

inline bool IsPersistentAuraEffect(uint32 effect)
{
    return effect == SPELL_EFFECT_PERSISTENT_AREA_AURA;
}

inline bool HasPersistentAuraEffect(SpellEntry const* spellInfo)
{
    for (unsigned int i : spellInfo->Effect)
        if (IsPersistentAuraEffect(i))
            return true;
    return false;
}

inline bool IsOnlySelfTargeting(SpellEntry const* spellInfo)
{
    for (int32 i = 0; i < MAX_EFFECT_INDEX; ++i)
    {
        if (!spellInfo->Effect[i])
            return true;

        switch (spellInfo->EffectImplicitTargetA[i])
        {
            case TARGET_UNIT_CASTER:
                break;
            default:
                return false;
        }
        switch (spellInfo->EffectImplicitTargetB[i])
        {
            case TARGET_UNIT_CASTER:
            case TARGET_NONE:
                break;
            default:
                return false;
        }
    }
    return true;
}

inline bool IsUnitTargetTarget(uint32 target)
{
    switch (target)
    {
        case TARGET_UNIT_ENEMY:
        case TARGET_UNIT:
        case TARGET_UNIT_FRIEND:
        case TARGET_UNIT_FRIEND_CHAIN_HEAL:
            return true;
        default: return false;
    }
}

inline bool HasMissingTargetFromClient(SpellEntry const* spellInfo)
{
    if (IsUnitTargetTarget(spellInfo->EffectImplicitTargetA[EFFECT_INDEX_0]))
        return false;

    if (IsUnitTargetTarget(spellInfo->EffectImplicitTargetA[EFFECT_INDEX_1]) || IsUnitTargetTarget(spellInfo->EffectImplicitTargetA[EFFECT_INDEX_2]))
        return true;

    return false;
}

inline bool IsScriptTarget(uint32 target)
{
    return (target < MAX_SPELL_TARGETS ? (SpellTargetInfoTable[target].filter == TARGET_SCRIPT) : false);
}

inline bool IsNeutralTarget(uint32 target)
{
    return (target < MAX_SPELL_TARGETS ? (SpellTargetInfoTable[target].filter == TARGET_NEUTRAL) : false);
}

inline bool IsFriendlyTarget(uint32 target)
{
    return (target < MAX_SPELL_TARGETS ? (SpellTargetInfoTable[target].filter == TARGET_HELPFUL) : false);
}

inline bool IsHostileTarget(uint32 target)
{
    return (target < MAX_SPELL_TARGETS ? (SpellTargetInfoTable[target].filter == TARGET_HARMFUL) : false);
}

inline bool IsEffectTargetScript(uint32 targetA, uint32 targetB)
{
    return (IsScriptTarget(targetA) || IsScriptTarget(targetB));
}

inline bool IsEffectTargetNeutral(uint32 targetA, uint32 targetB)
{
    return (IsNeutralTarget(targetA) && IsNeutralTarget(targetB));
}

inline bool IsEffectTargetPositive(uint32 targetA, uint32 targetB)
{
    return (IsFriendlyTarget(targetA) || IsFriendlyTarget(targetB));
}

inline bool IsEffectTargetNegative(uint32 targetA, uint32 targetB)
{
    return (IsHostileTarget(targetA) || IsHostileTarget(targetB));
}

inline bool IsNeutralEffectTargetPositive(uint32 etarget, const WorldObject* caster = nullptr, const WorldObject* target = nullptr)
{
    if (etarget < MAX_SPELL_TARGETS && SpellTargetInfoTable[etarget].type != TARGET_TYPE_UNIT)
        return true; // Some gameobjects or coords, who cares

    if (!target || (target->GetTypeId() != TYPEID_PLAYER && target->GetTypeId() != TYPEID_UNIT))
        return true;

    if (caster == target)
        return true; // Early self-cast detection

    if (!caster)
        return true;

    // TODO: Fix it later
    return caster->IsFriend(static_cast<const Unit*>(target));
}

inline bool IsPositiveEffectTargetMode(const SpellEntry* entry, SpellEffectIndex effIndex, const WorldObject* caster = nullptr, const WorldObject* target = nullptr, bool recursive = false)
{
    if (!entry)
        return false;

    // Forces positive targets to be negative TODO: Find out if this is true for neutral targets
    if (entry->HasAttribute(SPELL_ATTR_AURA_IS_DEBUFF))
        return false;

    // Triggered spells case: prefer child spell via IsPositiveSpell()-like scan for triggered spell
    if (IsSpellEffectTriggerSpell(entry, effIndex))
    {
        const uint32 spellid = entry->EffectTriggerSpell[effIndex];
        // Its possible to go infinite cycle with triggered spells. We are interested to peek only at the first layer so far
        if (!recursive && spellid && (spellid != entry->Id))
        {
            if (const SpellEntry* triggered = sSpellTemplate.LookupEntry<SpellEntry>(spellid))
            {
                for (uint32 i = EFFECT_INDEX_0; i < MAX_EFFECT_INDEX; ++i)
                {
                    if (!IsPositiveEffectTargetMode(triggered, SpellEffectIndex(i), caster, target, true))
                        return false;
                }
            }
        }
        // For trigger effects target modes are inconsistent: we have invalid and coflicting ones
        // Let's try to ignore them completely
        return true;
    }

    const uint32 a = entry->EffectImplicitTargetA[effIndex];
    const uint32 b = entry->EffectImplicitTargetB[effIndex];

    if ((!a && !b) || IsEffectTargetPositive(a, b) || IsEffectTargetScript(a, b))
        return true;
    if (IsEffectTargetNegative(a, b))
    {
        // Workaround: Passive talents with negative target modes are getting removed by ice block and similar effects
        // TODO: Fix removal of passives in appropriate places and remove the check below afterwards
        return entry->HasAttribute(SPELL_ATTR_PASSIVE);
    }
    if (IsEffectTargetNeutral(a, b))
        return (IsPointEffectTarget(SpellTarget(b ? b : a)) || IsNeutralEffectTargetPositive((b ? b : a), caster, target));

    // If we ever get to this point, we have unhandled target. Gotta say something about it.
    if (entry->Effect[effIndex])
        DETAIL_LOG("IsPositiveEffectTargetMode: Spell %u's effect %u has unhandled targets (A:%u B:%u)", entry->Id, effIndex,
                   entry->EffectImplicitTargetA[effIndex], entry->EffectImplicitTargetB[effIndex]);
    return true;
}

inline bool IsPositiveEffect(const SpellEntry* spellproto, SpellEffectIndex effIndex, const WorldObject* caster = nullptr, const WorldObject* target = nullptr)
{
    if (!spellproto)
        return false;

    switch (spellproto->Effect[effIndex])
    {
        case SPELL_EFFECT_SEND_TAXI:                // Some NPCs that send taxis are neutral, so target mode fails
        case SPELL_EFFECT_QUEST_COMPLETE:           // TODO: Spells with these effects should be casted by a proper caster to meet target mode.
        case SPELL_EFFECT_TELEPORT_UNITS:           // Until then switch makes them all positive
        case SPELL_EFFECT_CREATE_ITEM:
        case SPELL_EFFECT_SUMMON_CHANGE_ITEM:
            return true;
        case SPELL_EFFECT_APPLY_AREA_AURA_ENEMY:    // Always hostile effects
            return false;
        case SPELL_EFFECT_DUMMY:
            // some explicitly required dummy effect sets
            switch (spellproto->Id)
            {
                case 28441:                                 // AB Effect 000
                    return false;
                case 18153:                                 // Kodo Kombobulator
                    return true;
                default:
                    break;
            }
            break;
        // Aura exceptions:
        case SPELL_EFFECT_APPLY_AURA:
        case SPELL_EFFECT_APPLY_AREA_AURA_FRIEND:
        {
            switch (spellproto->EffectApplyAuraName[effIndex])
            {
                case SPELL_AURA_DUMMY:
                {
                    // dummy aura can be positive or negative dependent from casted spell
                    switch (spellproto->Id)
                    {
                        case 13139:                         // net-o-matic special effect
                            return false;
                        default:
                            break;
                    }
                    break;
                }
                default:
                    break;
            }
            break;
        }
        default:
            break;
    }

    // Generic effect check: negative on negative targets, positive on positive targets
    return IsPositiveEffectTargetMode(spellproto, effIndex, caster, target);
}

inline bool IsPositiveAuraEffect(const SpellEntry* entry, SpellEffectIndex effIndex, const WorldObject* /*caster*/ = nullptr, const WorldObject* /*target*/ = nullptr)
{
    return IsAuraApplyEffect(entry, effIndex) && !IsEffectTargetNegative(entry->EffectImplicitTargetA[effIndex], entry->EffectImplicitTargetB[effIndex])
        && !entry->HasAttribute(SPELL_ATTR_AURA_IS_DEBUFF) && entry->Effect[effIndex] != SPELL_EFFECT_APPLY_AREA_AURA_ENEMY;
}

inline bool IsPositiveSpellTargetModeForSpecificTarget(const SpellEntry* entry, uint8 effectMask, const WorldObject* caster = nullptr, const WorldObject* target = nullptr)
{
    if (!entry)
        return false;
    // spells with at least one negative effect are considered negative
    // some self-applied spells have negative effects but in self casting case negative check ignored.
    for (int i = 0; i < MAX_EFFECT_INDEX; ++i)
        if (effectMask & (1 << i))
            if (entry->Effect[i] && !IsPositiveEffectTargetMode(entry, SpellEffectIndex(i), caster, target))
                return false;
    return true;
}

inline bool IsPositiveSpellTargetModeForSpecificTarget(uint32 spellId, uint8 effectMask, const WorldObject* caster, const WorldObject* target)
{
    if (!spellId)
        return false;
    return IsPositiveSpellTargetModeForSpecificTarget(sSpellTemplate.LookupEntry<SpellEntry>(spellId), effectMask, caster, target);
}

inline bool IsPositiveSpellTargetMode(const SpellEntry* entry, const WorldObject* caster = nullptr, const WorldObject* target = nullptr)
{
    if (!entry)
        return false;
    // spells with at least one negative effect are considered negative
    // some self-applied spells have negative effects but in self casting case negative check ignored.
    for (int i = 0; i < MAX_EFFECT_INDEX; ++i)
        if (entry->Effect[i] && !IsPositiveEffectTargetMode(entry, SpellEffectIndex(i), caster, target))
            return false;
    return true;
}

inline bool IsPositiveSpellTargetMode(uint32 spellId, const WorldObject* caster, const WorldObject* target)
{
    if (!spellId)
        return false;
    return IsPositiveSpellTargetMode(sSpellTemplate.LookupEntry<SpellEntry>(spellId), caster, target);
}

inline bool IsPositiveSpell(const SpellEntry* entry, const WorldObject* caster = nullptr, const WorldObject* target = nullptr)
{
    if (!entry)
        return false;
    // spells with at least one negative effect are considered negative
    // some self-applied spells have negative effects but in self casting case negative check ignored.
    for (int i = 0; i < MAX_EFFECT_INDEX; ++i)
        if (entry->Effect[i] && !IsPositiveEffect(entry, SpellEffectIndex(i), caster, target))
            return false;
    return true;
}

// this is propably the correct check for most positivity/negativity decisions
inline bool IsPositiveEffectMask(const SpellEntry* entry, uint8 effectMask, const WorldObject* caster = nullptr, const WorldObject* target = nullptr)
{
    if (!entry)
        return false;
    // spells with at least one negative effect are considered negative
    // some self-applied spells have negative effects but in self casting case negative check ignored.
    for (int i = 0; i < MAX_EFFECT_INDEX; ++i)
        if (entry->Effect[i] && (effectMask & (1 << i)) && !IsPositiveEffect(entry, SpellEffectIndex(i), caster, target))
            return false;
    return true;
}

inline bool IsPositiveSpell(uint32 spellId, const WorldObject* caster = nullptr, const WorldObject* target = nullptr)
{
    if (!spellId)
        return false;
    return IsPositiveSpell(sSpellTemplate.LookupEntry<SpellEntry>(spellId), caster, target);
}

inline void GetChainJumpRange(SpellEntry const* spellInfo, SpellEffectIndex effIdx, float& minSearchRangeCaster, float& maxSearchRangeTarget, float& /*jumpRadius*/)
{
    const SpellRangeEntry* range = sSpellRangeStore.LookupEntry(spellInfo->rangeIndex);
    if (spellInfo->DmgClass == SPELL_DAMAGE_CLASS_MELEE)
        maxSearchRangeTarget = range->maxRange;
    else
        // FIXME: This very like horrible hack and wrong for most spells
        maxSearchRangeTarget = spellInfo->EffectChainTarget[effIdx] * CHAIN_SPELL_JUMP_RADIUS;

    if (range->ID == 114)   // Hunter Search range
        minSearchRangeCaster = 5;

    switch (spellInfo->Id)
    {
        case 2643:  // Multi-shot
        case 14288:
        case 14289:
        case 14290:
        case 25294:
        case 27021:
            maxSearchRangeTarget = 8.f;
            break;
        default:   // default jump radius
            break;
    }
}

inline bool IsChainAOESpell(SpellEntry const* spellInfo)
{
    switch (spellInfo->Id)
    {
        case 2643:  // Multi-shot
        case 14288:
        case 14289:
        case 14290:
        case 25294:
        case 27021:
            return true;
        default:
            return false;
    }
}

inline bool IsDispelSpell(SpellEntry const* spellInfo)
{
    return IsSpellHaveEffect(spellInfo, SPELL_EFFECT_DISPEL);
}

inline bool isSpellBreakStealth(SpellEntry const* spellInfo)
{
    return !spellInfo->HasAttribute(SPELL_ATTR_EX_NOT_BREAK_STEALTH);
}

inline bool IsAutoRepeatRangedSpell(SpellEntry const* spellInfo)
{
    return spellInfo->HasAttribute(SPELL_ATTR_RANGED) && spellInfo->HasAttribute(SPELL_ATTR_EX2_AUTOREPEAT_FLAG);
}

inline bool IsSpellRequiresRangedAP(SpellEntry const* spellInfo)
{
    return (spellInfo->SpellFamilyName == SPELLFAMILY_HUNTER && spellInfo->DmgClass != SPELL_DAMAGE_CLASS_MELEE);
}

inline uint32 GetAffectedTargets(SpellEntry const* spellInfo)
{
    // custom target amount cases
    switch (spellInfo->SpellFamilyName)
    {
        case SPELLFAMILY_GENERIC:
        {
            switch (spellInfo->Id)
            {
                case 802:                                   // Mutate Bug (AQ40, Emperor Vek'nilash)
                case 804:                                   // Explode Bug (AQ40, Emperor Vek'lor)
                case 23138:                                 // Gate of Shazzrah (MC, Shazzrah)
                case 23173:                                 // Brood Affliction (BWL, Chromaggus)
                case 24019:                                 // Axe Flurry (ZG - Gurubashi Axe Thrower)
                case 24150:                                 // Stinger Charge Primer (AQ20, Hive'Zara Stinger)
                case 24781:                                 // Dream Fog (Emerald Dragons)
                case 26080:                                 // Stinger Charge Primer (AQ40, Vekniss Stinger)
                case 26524:                                 // Sand Trap (AQ20 - Kurinnaxx)
                case 28560:                                 // Summon Blizzard (Naxx, Sapphiron)
                    return 1;
                case 10258:                                 // Awaken Vault Warder (Uldaman)
                case 28542:                                 // Life Drain (Naxx, Sapphiron)
                    return 2;
                case 25676:                                 // Drain Mana (correct number has to be researched)
                case 25754:
                    return 6;
                case 28796:                                 // Poison Bolt Volley (Naxx, Faerlina)
                    return 10;
                case 26457:                                 // Drain Mana (correct number has to be researched)
                case 26559:
                    return 12;
                case 25991:                                 // Poison Bolt Volley (AQ40, Pincess Huhuran)
                    return 15;
                default:
                    break;
            }
            break;
        }
        case SPELLFAMILY_MAGE:
        {
            switch (spellInfo->Id)
            {
                case 23603:                                 // Wild Polymorph (BWL, Nefarian)
                    return 1;
                default:
                    break;
            }
            break;
        }
        default:
            break;
    }

    return spellInfo->MaxAffectedTargets;
}

SpellCastResult GetErrorAtShapeshiftedCast(SpellEntry const* spellInfo, uint32 form);

inline bool IsChanneledSpell(SpellEntry const* spellInfo)
{
    return spellInfo->HasAttribute(SPELL_ATTR_EX_CHANNELED_1) || spellInfo->HasAttribute(SPELL_ATTR_EX_CHANNELED_2);
}

inline bool IsNeedCastSpellAtFormApply(SpellEntry const* spellInfo, ShapeshiftForm form)
{
    if ((!spellInfo->HasAttribute(SPELL_ATTR_PASSIVE) && !spellInfo->HasAttribute(SPELL_ATTR_HIDDEN_CLIENTSIDE)) || !form)
        return false;

    // passive spells with SPELL_ATTR_EX2_NOT_NEED_SHAPESHIFT are already active without shapeshift, do no recast!
    // Feline Swiftness Passive 2a not have 0x1 mask in Stance field in spell data as expected
    return (((spellInfo->Stances & (1 << (form - 1)) || (spellInfo->Id == 24864 && form == FORM_CAT))) &&
            !spellInfo->HasAttribute(SPELL_ATTR_EX2_NOT_NEED_SHAPESHIFT));
}

inline bool IsNeedCastSpellAtOutdoor(SpellEntry const* spellInfo)
{
    return (spellInfo->HasAttribute(SPELL_ATTR_OUTDOORS_ONLY) && spellInfo->HasAttribute(SPELL_ATTR_PASSIVE));
}

inline bool IsReflectableSpell(SpellEntry const* spellInfo)
{
    return spellInfo->DmgClass == SPELL_DAMAGE_CLASS_MAGIC && !spellInfo->HasAttribute(SPELL_ATTR_ABILITY)
           && !spellInfo->HasAttribute(SPELL_ATTR_EX_CANT_BE_REFLECTED) && !spellInfo->HasAttribute(SPELL_ATTR_UNAFFECTED_BY_INVULNERABILITY)
           && !spellInfo->HasAttribute(SPELL_ATTR_PASSIVE) && !IsPositiveSpell(spellInfo);
}

// Mostly required by spells that target a creature inside GO
inline bool IsIgnoreLosSpell(SpellEntry const* spellInfo)
{
    switch (spellInfo->Id)
    {
        case 24742:                                 // Magic Wings
        // case 42867:                              // both need LOS, likely TARGET_UNIT_CASTER should use LOS ignore from normal radius, not per-effect radius WIP
            return true;
        default:
            break;
    }

    return spellInfo->HasAttribute(SPELL_ATTR_EX2_IGNORE_LOS);
}

inline bool IsIgnoreLosSpellEffect(SpellEntry const* spellInfo, SpellEffectIndex effIdx)
{
    // TODO: Move this to target logic
    switch (spellInfo->EffectImplicitTargetA[effIdx])
    {
        case TARGET_UNIT_RAID_AND_CLASS: return true;
        default: break;
    }

    return spellInfo->EffectRadiusIndex[effIdx] == 28 || IsIgnoreLosSpell(spellInfo);
}

inline bool IsIgnoreLosSpellCast(SpellEntry const* spellInfo)
{
    return spellInfo->rangeIndex == 13 || IsIgnoreLosSpell(spellInfo);
}

inline bool NeedsComboPoints(SpellEntry const* spellInfo)
{
    return spellInfo->HasAttribute(SPELL_ATTR_EX_REQ_TARGET_COMBO_POINTS) || spellInfo->HasAttribute(SPELL_ATTR_EX_REQ_COMBO_POINTS);
}

inline SpellSchoolMask GetSpellSchoolMask(SpellEntry const* spellInfo)
{
    return GetSchoolMask(spellInfo->School);
}

inline uint32 GetSpellMechanicMask(SpellEntry const* spellInfo, uint32 effectMask)
{
    uint32 mask = 0;
    if (spellInfo->Mechanic)
        mask |= 1 << (spellInfo->Mechanic - 1);

    for (uint32 i = 0; i < MAX_EFFECT_INDEX; ++i)
    {
        if (!(effectMask & (1 << i)))
            continue;

        if (spellInfo->EffectMechanic[i])
            mask |= 1 << (spellInfo->EffectMechanic[i] - 1);
    }

    return mask;
}

inline uint32 GetAllSpellMechanicMask(SpellEntry const* spellInfo)
{
    uint32 mask = 0;
    if (spellInfo->Mechanic)
        mask |= 1 << (spellInfo->Mechanic - 1);
    for (unsigned int i : spellInfo->EffectMechanic)
        if (i)
            mask |= 1 << (i - 1);
    return mask;
}

inline Mechanics GetEffectMechanic(SpellEntry const* spellInfo, SpellEffectIndex effect)
{
    if (spellInfo->EffectMechanic[effect])
        return Mechanics(spellInfo->EffectMechanic[effect]);
    if (spellInfo->Mechanic)
        return Mechanics(spellInfo->Mechanic);
    return MECHANIC_NONE;
}

inline uint32 GetDispellMask(DispelType dispel)
{
    // If dispell all
    if (dispel == DISPEL_ALL)
        return DISPEL_ALL_MASK;
    return (1 << dispel);
}

inline bool IsAuraAddedBySpell(uint32 auraType, uint32 spellId)
{
    SpellEntry const* spellproto = sSpellTemplate.LookupEntry<SpellEntry>(spellId);
    if (!spellproto) return false;

    for (int i = 0; i < 3; i++)
        if (spellproto->EffectApplyAuraName[i] == auraType)
            return true;
    return false;
}

inline bool IsPartyOrRaidTarget(uint32 target)
{
    switch (target)
    {
        case TARGET_UNIT_FRIEND_NEAR_CASTER:
        case TARGET_ENUM_UNITS_PARTY_WITHIN_CASTER_RANGE:
        case TARGET_ENUM_UNITS_PARTY_AOE_AT_SRC_LOC:
        case TARGET_ENUM_UNITS_PARTY_AOE_AT_DEST_LOC:
        case TARGET_UNIT_PARTY:
        case TARGET_UNIT_FRIEND_AND_PARTY:
        case TARGET_ENUM_UNITS_RAID_WITHIN_CASTER_RANGE:
        case TARGET_UNIT_RAID:
        case TARGET_UNIT_RAID_NEAR_CASTER:
        case TARGET_UNIT_RAID_AND_CLASS:
            return true;
        default:
            return false;
    }
}

inline bool IsGroupBuff(SpellEntry const* spellInfo)
{
    for (unsigned int i : spellInfo->EffectImplicitTargetA)
    {
        if (IsPartyOrRaidTarget(i))
            return true;
    }

    return false;
}

// Caster Centric Specific: A target cannot have more than one instance of specific per caster
// This a relaxed rule and does not automatically exclude multi-ranking, multi-ranking should be handled separately (usually on effect stacking level)
// Example: Curses. One curse per caster, Curse of Agony and Curse of Doom ranks are stackable among casters, the rest of curse stacking logic is handled on effect basis
inline bool IsSpellSpecificUniquePerCaster(SpellSpecific specific)
{
    switch (int32(specific))
    {
        case SPELL_BLESSING:
        case SPELL_AURA:
        case SPELL_STING:
        case SPELL_CURSE:
        case SPELL_ASPECT:
        case SPELL_JUDGEMENT:
        case SPELL_SOUL_CAPTURE:
            return true;
        default:
            break;
    }
    return false;
}

// Target Centric Specific: A target cannot have more than one instance of specific applied to it
// This is a restrictive rule and automatically excludes multi-ranking possibility
// Example: Elemental Shield. No matter who it came from, only last one and the strongest one should stay.
inline bool IsSpellSpecificUniquePerTarget(SpellSpecific specific)
{
    switch (int32(specific))
    {
        case SPELL_SEAL:
        case SPELL_TRACKER:
        case SPELL_WARLOCK_ARMOR:
        case SPELL_MAGE_ARMOR:
        case SPELL_ELEMENTAL_SHIELD:
        case SPELL_WELL_FED:
        case SPELL_FLASK_ELIXIR:
        case SPELL_FOOD:
        case SPELL_DRINK:
        case SPELL_FOOD_AND_DRINK:
            return true;
        default:
            break;
    }
    return false;
}

// Compares two spell specifics
inline bool IsSpellSpecificIdentical(SpellSpecific specific, SpellSpecific specific2)
{
    if (specific == specific2)
        return true;
    // Compare combined specifics
    switch (int32(specific))
    {
        case SPELL_FOOD:
            return specific2 == SPELL_FOOD ||
                   specific2 == SPELL_FOOD_AND_DRINK;
        case SPELL_DRINK:
            return specific2 == SPELL_DRINK ||
                   specific2 == SPELL_FOOD_AND_DRINK;
        case SPELL_FOOD_AND_DRINK:
            return specific2 == SPELL_FOOD ||
                   specific2 == SPELL_DRINK ||
                   specific2 == SPELL_FOOD_AND_DRINK;
        default:
            break;
    }
    return false;
}

inline bool IsSimilarAuraEffect(SpellEntry const* entry, uint32 effect, SpellEntry const* entry2, uint32 effect2)
{
    return (entry2->EffectApplyAuraName[effect2] && entry->EffectApplyAuraName[effect] &&
            entry2->Effect[effect2] == entry->Effect[effect] &&
            entry2->EffectApplyAuraName[effect2] == entry->EffectApplyAuraName[effect] &&
            IsPositiveEffect(entry2, SpellEffectIndex(effect2)) == IsPositiveEffect(entry, SpellEffectIndex(effect)));
}

inline bool IsStackableAuraEffect(SpellEntry const* entry, SpellEntry const* entry2, uint32 i, Unit* pTarget = nullptr)
{
    const uint32 aura = entry->EffectApplyAuraName[i];
    // Ignore non-aura effects
    if (!aura)
        return true;

    // Get first similar - second spell's same aura with the same sign
    uint32 similar = EFFECT_INDEX_0;
    for (uint32 e = EFFECT_INDEX_0; e < MAX_EFFECT_INDEX; ++e)
    {
        if (IsSimilarAuraEffect(entry, i, entry2, e))
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

    // Special rule for food buffs
    if (GetSpellSpecific(entry->Id) == SPELL_WELL_FED && GetSpellSpecific(entry2->Id) != SPELL_WELL_FED)
        return true;

    // Short alias
    const bool positive = (IsPositiveEffect(entry, SpellEffectIndex(i)));
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
                case SPELLFAMILY_GENERIC:
                    if (entry->SpellIconID == 92 && entry->SpellVisual == 99 && icon && visual)
                        return false; // Soulstone Resurrection
                    break;
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
            if (pTarget && pTarget->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED))
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
            if (!positive && entry->EffectBasePoints[i] < 1 && entry2->EffectBasePoints[similar] < 1)
                return (!entry->SpellFamilyName && !entry2->SpellFamilyName);
            break;
        // Armor & Resistance buffs and debuffs logic
        case SPELL_AURA_MOD_RESISTANCE:
        {
            if (entry->EffectMiscValue[i] != entry2->EffectMiscValue[similar])
                break;
            if (positive)
            {
                // Used as a non-zero type in this context
                const bool type = (entry->Dispel && entry->Dispel == entry2->Dispel);
                const bool attacktable = (entry->DmgClass && entry->DmgClass == entry2->DmgClass);
                if ((attacktable || type) && !entry->SpellFamilyName && !entry2->SpellFamilyName)
                    return false; // Do not stack scrolls with other srolls and some procs (such as Hyjal ring)
                if (player && related && siblings && entry->HasAttribute(SPELL_ATTR_EX3_STACK_FOR_DIFF_CASTERS))
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
            if (entry->EffectMiscValue[i] != entry2->EffectMiscValue[similar])
                break;
            if (positive && entry->Dispel && entry->Dispel == entry2->Dispel)
                return false; // Inspiration / Ancestral Fortitude
            nonmui = true;
            break;
        }
        // By default base stats cannot stack if they're similar
        case SPELL_AURA_MOD_STAT:
        {
            if (entry->EffectMiscValue[i] != entry2->EffectMiscValue[similar])
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
                    if (entry->HasAttribute(SPELL_ATTR_EX2_UNK28) && entry2->HasAttribute(SPELL_ATTR_EX2_UNK28))
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
        case SPELL_AURA_MOD_HEALING_DONE:
        case SPELL_AURA_MOD_HEALING_PCT:
            // Do not stack similar debuffs: Mortal Strike, Aimed Shot, Hex of Weakness
            if (!positive)
                return (entry->EffectMiscValue[i] == entry2->EffectMiscValue[similar]);
            break;
        case SPELL_AURA_MOD_MELEE_HASTE:
        case SPELL_AURA_MOD_RANGED_HASTE:
        case SPELL_AURA_MOD_DAMAGE_DONE:
        case SPELL_AURA_MOD_DAMAGE_PERCENT_DONE: // Ferocious Inspiration, Shadow Embrace
            if (positive)
                return true;
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
        case SPELL_AURA_MOD_RATING: // Whitelisted, Rejuvenation has this
        case SPELL_AURA_MOD_SPELL_CRIT_CHANCE: // Party auras whitelist for Totem of Wrath
        case SPELL_AURA_MOD_SPELL_HIT_CHANCE: // Party auras whitelist for Totem of Wrath
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
    }
    if (nonmui && instance && !IsChanneledSpell(entry) && !IsChanneledSpell(entry2))
        return false; // Forbids multi-ranking and multi-application on rule, exclude channeled spells (like Mind Flay)

    if (multirank && IsPositiveSpell(entry) && IsPositiveSpell(entry2))
        return false; // Forbids multi-ranking for positive spells

    return true;
}

inline bool IsStackableSpell(SpellEntry const* entry, SpellEntry const* entry2, Unit* pTarget = nullptr)
{
    for (uint32 i = EFFECT_INDEX_0; i < MAX_EFFECT_INDEX; ++i)
    {
        if (entry->Effect[i] && entry->EffectApplyAuraName[i] && !IsStackableAuraEffect(entry, entry2, i, pTarget))
            return false;
    }
    return true;
}

inline bool IsSimilarExistingAuraStronger(const SpellAuraHolder* holder, const SpellAuraHolder* existing)
{
    if (!holder || !existing)
        return false;
    const SpellEntry* entry = holder->GetSpellProto();
    const SpellEntry* entry2 = existing->GetSpellProto();
    if (!entry || !entry2)
        return false;

    // Already compared effects masks to avoid re-entrance
    uint32 effectmask1 = 0;
    uint32 effectmask2 = 0;

    for (uint32 e = EFFECT_INDEX_0; e < MAX_EFFECT_INDEX; ++e)
    {
        for (uint32 e2 = EFFECT_INDEX_0; e2 < MAX_EFFECT_INDEX; ++e2)
        {
            if (IsSimilarAuraEffect(entry, e, entry2, e2) && !(effectmask1 & (1 << e)) && !(effectmask2 & (1 << e2)))
            {
                effectmask1 |= (1 << e);
                effectmask2 |= (1 << e2);
                Aura* aura1 = holder->GetAuraByEffectIndex(SpellEffectIndex(e));
                Aura* aura2 = existing->GetAuraByEffectIndex(SpellEffectIndex(e2));
                int32 value = aura1 ? (aura1->GetModifier()->m_amount / int32(aura1->GetStackAmount())) : 0;
                int32 value2 = aura2 ? (aura2->GetModifier()->m_amount / int32(aura2->GetStackAmount())) : 0;
                if (value < 0 && value2 < 0)
                {
                    value = abs(value);
                    value2 = abs(value2);
                }
                if (value2 > value)
                    return true;
            }
        }
    }
    return false;
}

inline bool IsSimilarExistingAuraStronger(const Unit* caster, const SpellEntry* entry, const SpellAuraHolder* existing)
{
    if (!caster || !existing)
        return false;
    const SpellEntry* entry2 = existing->GetSpellProto();
    if (!entry || !entry2)
        return false;

    // Already compared effects masks to avoid re-entrance
    uint32 effectmask1 = 0;
    uint32 effectmask2 = 0;

    for (uint32 e = EFFECT_INDEX_0; e < MAX_EFFECT_INDEX; ++e)
    {
        for (uint32 e2 = EFFECT_INDEX_0; e2 < MAX_EFFECT_INDEX; ++e2)
        {
            if (IsSimilarAuraEffect(entry, e, entry2, e2) && !(effectmask1 & (1 << e)) && !(effectmask2 & (1 << e2)))
            {
                effectmask1 |= (1 << e);
                effectmask2 |= (1 << e2);
                Aura* aura = existing->GetAuraByEffectIndex(SpellEffectIndex(e2));
                int32 value = entry->CalculateSimpleValue(SpellEffectIndex(e));
                int32 value2 = aura ? (aura->GetModifier()->m_amount / int32(aura->GetStackAmount())) : 0;
                // FIXME: We need API to peacefully pre-calculate static base spell damage without destroying mods
                // Until then this is a rather lame set of hacks
                // Apply combo points base damage for spells like expose armor
                if (caster->GetTypeId() == TYPEID_PLAYER)
                {
                    const Player* player = (const Player*)caster;
                    const Unit* target = existing->GetTarget();
                    const float comboDamage = entry->EffectPointsPerComboPoint[e];
                    if (player && target && (target->GetObjectGuid() == player->GetComboTargetGuid()))
                        value += int32(comboDamage * player->GetComboPoints());
                }
                if (value < 0 && value2 < 0)
                {
                    value = abs(value);
                    value2 = abs(value2);
                }
                if (value2 > value)
                    return true;
            }
        }
    }
    return false;
}

inline bool IsSimilarExistingAuraStronger(const Unit* caster, uint32 spellid, const SpellAuraHolder* existing)
{
    if (!spellid)
        return false;
    return IsSimilarExistingAuraStronger(caster, sSpellTemplate.LookupEntry<SpellEntry>(spellid), existing);
}

// Diminishing Returns interaction with spells
DiminishingGroup GetDiminishingReturnsGroupForSpell(SpellEntry const* spellproto, bool triggered);
bool IsDiminishingReturnsGroupDurationLimited(DiminishingGroup group);
DiminishingReturnsType GetDiminishingReturnsGroupType(DiminishingGroup group);
bool IsCreatureDRSpell(SpellEntry const* spellInfo);

// Spell affects related declarations (accessed using SpellMgr functions)
typedef std::map<uint32, uint64> SpellAffectMap;

// Spell proc event related declarations (accessed using SpellMgr functions)
enum ProcFlags
{
    PROC_FLAG_NONE                          = 0x00000000,

    PROC_FLAG_KILLED                        = 0x00000001,   // 00 Killed by aggressor
    PROC_FLAG_KILL                          = 0x00000002,   // 01 Kill target (in most cases need XP/Honor reward, see Unit::IsTriggeredAtSpellProcEvent for additinoal check)

    PROC_FLAG_SUCCESSFUL_MELEE_HIT          = 0x00000004,   // 02 Successful melee auto attack
    PROC_FLAG_TAKEN_MELEE_HIT               = 0x00000008,   // 03 Taken damage from melee auto attack hit

    PROC_FLAG_SUCCESSFUL_MELEE_SPELL_HIT    = 0x00000010,   // 04 Successful attack by Spell that use melee weapon
    PROC_FLAG_TAKEN_MELEE_SPELL_HIT         = 0x00000020,   // 05 Taken damage by Spell that use melee weapon

    PROC_FLAG_SUCCESSFUL_RANGED_HIT         = 0x00000040,   // 06 Successful Ranged auto attack
    PROC_FLAG_TAKEN_RANGED_HIT              = 0x00000080,   // 07 Taken damage from ranged auto attack

    PROC_FLAG_SUCCESSFUL_RANGED_SPELL_HIT   = 0x00000100,   // 08 Successful Ranged attack by Spell that use ranged weapon
    PROC_FLAG_TAKEN_RANGED_SPELL_HIT        = 0x00000200,   // 09 Taken damage by Spell that use ranged weapon

    PROC_FLAG_DONE_SPELL_NONE_DMG_CLASS_POS     = 0x00000400,  // 10 Done positive spell that has dmg class none
    PROC_FLAG_TAKEN_SPELL_NONE_DMG_CLASS_POS    = 0x00000800,  // 11 Taken positive spell that has dmg class none

    PROC_FLAG_DONE_SPELL_NONE_DMG_CLASS_NEG     = 0x00001000,  // 12 Done negative spell that has dmg class none
    PROC_FLAG_TAKEN_SPELL_NONE_DMG_CLASS_NEG    = 0x00002000,  // 13 Taken negative spell that has dmg class none

    PROC_FLAG_DONE_SPELL_MAGIC_DMG_CLASS_POS    = 0x00004000,  // 14 Successful cast positive spell (by default only on healing)
    PROC_FLAG_TAKEN_SPELL_MAGIC_DMG_CLASS_POS   = 0x00008000,  // 15 Taken positive spell hit (by default only on healing)

    PROC_FLAG_DONE_SPELL_MAGIC_DMG_CLASS_NEG    = 0x00010000,  // 16 Successful negative spell cast (by default only on damage)
    PROC_FLAG_TAKEN_SPELL_MAGIC_DMG_CLASS_NEG   = 0x00020000,  // 17 Taken negative spell (by default only on damage)

    PROC_FLAG_ON_DO_PERIODIC                = 0x00040000,   // 18 Successful do periodic (damage / healing, determined by PROC_EX_PERIODIC_POSITIVE or negative if no procEx)
    PROC_FLAG_ON_TAKE_PERIODIC              = 0x00080000,   // 19 Taken spell periodic (damage / healing, determined by PROC_EX_PERIODIC_POSITIVE or negative if no procEx)

    PROC_FLAG_TAKEN_ANY_DAMAGE              = 0x00100000,   // 20 Taken any damage
    PROC_FLAG_ON_TRAP_ACTIVATION            = 0x00200000,   // 21 On trap activation

    PROC_FLAG_TAKEN_OFFHAND_HIT             = 0x00400000,   // 22 Taken off-hand melee attacks(not used)
    PROC_FLAG_SUCCESSFUL_OFFHAND_HIT        = 0x00800000    // 23 Successful off-hand melee attacks
};

#define MELEE_BASED_TRIGGER_MASK (PROC_FLAG_SUCCESSFUL_MELEE_HIT        | \
                                  PROC_FLAG_TAKEN_MELEE_HIT             | \
                                  PROC_FLAG_SUCCESSFUL_MELEE_SPELL_HIT  | \
                                  PROC_FLAG_TAKEN_MELEE_SPELL_HIT       | \
                                  PROC_FLAG_SUCCESSFUL_RANGED_HIT       | \
                                  PROC_FLAG_TAKEN_RANGED_HIT            | \
                                  PROC_FLAG_SUCCESSFUL_RANGED_SPELL_HIT | \
                                  PROC_FLAG_TAKEN_RANGED_SPELL_HIT)

#define NEGATIVE_TRIGGER_MASK (MELEE_BASED_TRIGGER_MASK                | \
                               PROC_FLAG_DONE_SPELL_NONE_DMG_CLASS_NEG      | \
                               PROC_FLAG_TAKEN_SPELL_NONE_DMG_CLASS_NEG           | \
                               PROC_FLAG_DONE_SPELL_MAGIC_DMG_CLASS_NEG | \
                               PROC_FLAG_TAKEN_SPELL_MAGIC_DMG_CLASS_NEG)

enum ProcFlagsEx
{
    PROC_EX_NONE                = 0x0000000,                // If none can tigger on Hit/Crit only (passive spells MUST defined by SpellFamily flag)
    PROC_EX_NORMAL_HIT          = 0x0000001,                // If set only from normal hit (only damage spells)
    PROC_EX_CRITICAL_HIT        = 0x0000002,
    PROC_EX_MISS                = 0x0000004,
    PROC_EX_RESIST              = 0x0000008,
    PROC_EX_DODGE               = 0x0000010,
    PROC_EX_PARRY               = 0x0000020,
    PROC_EX_BLOCK               = 0x0000040,
    PROC_EX_EVADE               = 0x0000080,
    PROC_EX_IMMUNE              = 0x0000100,
    PROC_EX_DEFLECT             = 0x0000200,
    PROC_EX_ABSORB              = 0x0000400,
    PROC_EX_REFLECT             = 0x0000800,
    PROC_EX_INTERRUPT           = 0x0001000,                // Melee hit result can be Interrupt (not used)
    PROC_EX_RESERVED1           = 0x0002000,
    PROC_EX_RESERVED2           = 0x0004000,
    PROC_EX_RESERVED3           = 0x0008000,
    PROC_EX_EX_TRIGGER_ALWAYS   = 0x0010000,                // If set trigger always ( no matter another flags) used for drop charges
    PROC_EX_EX_ONE_TIME_TRIGGER = 0x0020000,                // If set trigger always but only one time (not used)
    PROC_EX_PERIODIC_POSITIVE   = 0x0040000,                // For periodic heal
    PROC_EX_MAGNET              = 0x0080000,                // For grounding totem hit

    // Flags for internal use - do not use these in db!
    PROC_EX_INTERNAL_HOT        = 0x2000000
};

struct SpellProcEventEntry
{
    uint32      schoolMask;
    uint32      spellFamilyName;                            // if nonzero - for matching proc condition based on candidate spell's SpellFamilyNamer value
    ClassFamilyMask spellFamilyMask[MAX_EFFECT_INDEX];      // if nonzero - for matching proc condition based on candidate spell's SpellFamilyFlags  (like auras 107 and 108 do)
    uint32      procFlags;                                  // bitmask for matching proc event
    uint32      procEx;                                     // proc Extend info (see ProcFlagsEx)
    float       ppmRate;                                    // for melee (ranged?) damage spells - proc rate per minute. if zero, falls back to flat chance from Spell.dbc
    float       customChance;                               // Owerride chance (in most cases for debug only)
    uint32      cooldown;                                   // hidden cooldown used for some spell proc events, applied to _triggered_spell_
};

struct SpellBonusEntry
{
    float  direct_damage;
    float  dot_damage;
    float  ap_bonus;
    float  ap_dot_bonus;
};

typedef std::unordered_map<uint32, SpellProcEventEntry> SpellProcEventMap;
typedef std::unordered_map<uint32, SpellBonusEntry>     SpellBonusMap;

#define ELIXIR_FLASK_MASK     0x03                          // 2 bit mask for batter compatibility with more recent client version, flaks must have both bits set
#define ELIXIR_WELL_FED       0x10                          // Some foods have SPELLFAMILY_POTION

struct SpellThreatEntry
{
    uint16 threat;
    float multiplier;
    float ap_bonus;
};

typedef std::map<uint32, uint8> SpellElixirMap;
typedef std::map<uint32, float> SpellProcItemEnchantMap;
typedef std::map<uint32, SpellThreatEntry> SpellThreatMap;

// Spell script target related declarations (accessed using SpellMgr functions)
enum SpellTargetType
{
    SPELL_TARGET_TYPE_GAMEOBJECT    = 0,
    SPELL_TARGET_TYPE_CREATURE      = 1,
    SPELL_TARGET_TYPE_DEAD          = 2,
    SPELL_TARGET_TYPE_CREATURE_GUID = 3,
};

#define MAX_SPELL_TARGET_TYPE 4

// pre-defined targeting for spells
struct SpellTargetEntry
{
    uint32 spellId;
    uint32 type;
    uint32 targetEntry;
    uint32 inverseEffectMask;

    bool CanNotHitWithSpellEffect(SpellEffectIndex effect) const { return (inverseEffectMask & (1 << effect)) != 0; }
};

// coordinates for spells (accessed using SpellMgr functions)
struct SpellTargetPosition
{
    uint32 target_mapId;
    float  target_X;
    float  target_Y;
    float  target_Z;
    float  target_Orientation;
};

struct SpellCone
{
    uint32 spellId;
    int32 coneAngle;
};

typedef std::unordered_map<uint32, SpellTargetPosition> SpellTargetPositionMap;

// Spell pet auras
class PetAura
{
    public:
        PetAura() :
            removeOnChangePet(false),
            damage(0)
        {
        }

        PetAura(uint32 petEntry, uint32 aura, bool _removeOnChangePet, int _damage) :
            removeOnChangePet(_removeOnChangePet), damage(_damage)
        {
            auras[petEntry] = aura;
        }

        uint32 GetAura(uint32 petEntry) const
        {
            std::map<uint32, uint32>::const_iterator itr = auras.find(petEntry);
            if (itr != auras.end())
                return itr->second;
            std::map<uint32, uint32>::const_iterator itr2 = auras.find(0);
            if (itr2 != auras.end())
                return itr2->second;
            return 0;
        }

        void AddAura(uint32 petEntry, uint32 aura)
        {
            auras[petEntry] = aura;
        }

        bool IsRemovedOnChangePet() const
        {
            return removeOnChangePet;
        }

        int32 GetDamage() const
        {
            return damage;
        }

    private:
        std::map<uint32, uint32> auras;
        bool removeOnChangePet;
        int32 damage;
};
typedef std::map<uint16, PetAura> SpellPetAuraMap;

struct SpellArea
{
    uint32 spellId;
    uint32 areaId;                                          // zone/subzone/or 0 is not limited to zone
    uint32 questStart;                                      // quest start (quest must be active or rewarded for spell apply)
    uint32 questEnd;                                        // quest end (quest don't must be rewarded for spell apply)
    uint16 conditionId;                                     // conditionId - will replace questStart, questEnd, raceMask, gender and questStartCanActive
    int32  auraSpell;                                       // spell aura must be applied for spell apply )if positive) and it don't must be applied in other case
    uint32 raceMask;                                        // can be applied only to races
    Gender gender;                                          // can be applied only to gender
    bool questStartCanActive;                               // if true then quest start can be active (not only rewarded)
    bool autocast;                                          // if true then auto applied at area enter, in other case just allowed to cast

    // helpers
    bool IsFitToRequirements(Player const* player, uint32 newZone, uint32 newArea) const;
    void ApplyOrRemoveSpellIfCan(Player* player, uint32 newZone, uint32 newArea, bool onlyApply) const;
};

typedef std::multimap<uint32 /*applySpellId*/, SpellArea> SpellAreaMap;
typedef std::multimap<uint32 /*auraSpellId*/, SpellArea const*> SpellAreaForAuraMap;
typedef std::multimap<uint32 /*areaOrZoneId*/, SpellArea const*> SpellAreaForAreaMap;
typedef std::pair<SpellAreaMap::const_iterator, SpellAreaMap::const_iterator> SpellAreaMapBounds;
typedef std::pair<SpellAreaForAuraMap::const_iterator, SpellAreaForAuraMap::const_iterator>  SpellAreaForAuraMapBounds;
typedef std::pair<SpellAreaForAreaMap::const_iterator, SpellAreaForAreaMap::const_iterator>  SpellAreaForAreaMapBounds;


// Spell rank chain  (accessed using SpellMgr functions)
struct SpellChainNode
{
    uint32 prev;
    uint32 first;
    uint32 req;
    uint8  rank;
};

typedef std::unordered_map<uint32, SpellChainNode> SpellChainMap;
typedef std::multimap<uint32, uint32> SpellChainMapNext;

// Spell learning properties (accessed using SpellMgr functions)
struct SpellLearnSkillNode
{
    uint16 skill;
    uint16 step;
    SpellEffects effect;
};

typedef std::map<uint32, SpellLearnSkillNode> SpellLearnSkillMap;

struct SpellLearnSpellNode
{
    uint32 spell;
    bool active;                                            // show in spellbook or not
    bool autoLearned;
};

typedef std::multimap<uint32, SpellLearnSpellNode> SpellLearnSpellMap;
typedef std::pair<SpellLearnSpellMap::const_iterator, SpellLearnSpellMap::const_iterator> SpellLearnSpellMapBounds;

typedef std::multimap<uint32, SkillLineAbilityEntry const*> SkillLineAbilityMap;
typedef std::pair<SkillLineAbilityMap::const_iterator, SkillLineAbilityMap::const_iterator> SkillLineAbilityMapBounds;

typedef std::multimap<uint32, SkillRaceClassInfoEntry const*> SkillRaceClassInfoMap;
typedef std::pair<SkillRaceClassInfoMap::const_iterator, SkillRaceClassInfoMap::const_iterator> SkillRaceClassInfoMapBounds;

bool IsPrimaryProfessionSkill(uint32 skill);

inline bool IsProfessionSkill(uint32 skill)
{
    return  IsPrimaryProfessionSkill(skill) || skill == SKILL_FISHING || skill == SKILL_COOKING || skill == SKILL_FIRST_AID;
}

inline bool IsProfessionOrRidingSkill(uint32 skill)
{
    return  IsProfessionSkill(skill) || skill == SKILL_RIDING;
}

typedef std::map<uint32, uint32> SpellFacingFlagMap;

class SpellMgr
{
        friend struct DoSpellBonuses;
        friend struct DoSpellProcEvent;
        friend struct DoSpellProcItemEnchant;

        // Constructors
    public:
        SpellMgr();
        ~SpellMgr();

        // Accessors (const or static functions)
    public:
        // Spell affects
        ClassFamilyMask GetSpellAffectMask(uint32 spellId, SpellEffectIndex effectId) const
        {
            SpellAffectMap::const_iterator itr = mSpellAffectMap.find((spellId << 8) + effectId);
            if (itr != mSpellAffectMap.end())
                return ClassFamilyMask(itr->second);
            if (SpellEntry const* spellEntry = sSpellTemplate.LookupEntry<SpellEntry>(spellId))
                return ClassFamilyMask(spellEntry->EffectItemType[effectId]);
            return ClassFamilyMask();
        }

        SpellElixirMap const& GetSpellElixirMap() const { return mSpellElixirs; }

        uint32 GetSpellElixirMask(uint32 spellid) const
        {
            SpellElixirMap::const_iterator itr = mSpellElixirs.find(spellid);
            if (itr == mSpellElixirs.end())
                return 0x0;

            return itr->second;
        }

        SpellSpecific GetSpellElixirSpecific(uint32 spellid) const
        {
            uint32 mask = GetSpellElixirMask(spellid);

            // flasks must have all bits set from ELIXIR_FLASK_MASK
            if ((mask & ELIXIR_FLASK_MASK) == ELIXIR_FLASK_MASK)
                return SPELL_FLASK_ELIXIR;
            else if (mask & ELIXIR_WELL_FED)
                return SPELL_WELL_FED;
            return SPELL_NORMAL;
        }

        SpellSpecific GetSpellFoodSpecific(const SpellEntry* entry) const
        {
            if (!entry)
                return SPELL_NORMAL;
            // Food / Drinks (mostly)
            if (entry->AuraInterruptFlags & AURA_INTERRUPT_FLAG_NOT_SEATED)
            {
                if (entry->SpellFamilyName == SPELLFAMILY_GENERIC)
                {
                    bool food = false;
                    bool drink = false;
                    for (int i = 0; i < MAX_EFFECT_INDEX; ++i)
                    {
                        switch (entry->EffectApplyAuraName[i])
                        {
                            // Food
                            case SPELL_AURA_MOD_REGEN:
                            case SPELL_AURA_OBS_MOD_HEALTH:
                                food = true;
                                break;
                            // Drink
                            case SPELL_AURA_MOD_POWER_REGEN:
                            case SPELL_AURA_OBS_MOD_MANA:
                                drink = true;
                                break;
                            default:
                                break;
                        }
                    }

                    if (food && drink)
                        return SPELL_FOOD_AND_DRINK;
                    else if (food)
                        return SPELL_FOOD;
                    else if (drink)
                        return SPELL_DRINK;
                }
            }
            // Well Fed buffs (must be exclusive with Food / Drink replenishment effects, or else Well Fed will cause them to be removed)
            else if (entry->HasAttribute(SPELL_ATTR_EX2_FOOD_BUFF))
                return SPELL_WELL_FED;

            // Spells without attributes, but classified as well fed
            // Multi-family check
            switch (entry->Id)
            {
                // Food buffs without attribute: instantly applied ones
                // Parent spell contains the attribute for them (TODO: add a query for parent spell in the future?)
                case 18125: // Blessed Sunfruit
                case 18141: // Blessed Sunfruit Juice
                case 18191: // Windblossom Berries
                case 18192: // Grilled Squid
                case 18193: // Marsh Lichen
                case 22730: // Runn Tum Tuber Surprise
                case 25661: // Dirge's Kickin' Chimaerok Chops
                // Alcohol: instant application, no attribute
                case 5020:  // Stormstout
                case 5021:  // Trogg Ale
                case 5257:  // Thunderbrew Lager
                case 5909:  // Watered-down Beer
                case 6114:  // Raptor Punch
                case 8553:  // Barleybrew Scalder
                case 20875: // Rumsey Rum
                case 22789: // Gordok Green Grog
                case 22790: // Kreeg's Stout Beatdown
                case 25037: // Rumsey Rum Light
                case 25722: // Rumsey Rum Dark
                case 25804: // Rumsey Rum Black Label
                    return SPELL_WELL_FED;
            }
            return SPELL_NORMAL;
        }

        // Reverse engineered from binary: do not alter
        static inline bool IsSpellTargetHarmfulAtClient(uint32 target)
        {
            switch (target)
            {
                case TARGET_UNIT_ENEMY_NEAR_CASTER:
                case TARGET_UNIT_ENEMY:
                case TARGET_ENUM_UNITS_ENEMY_AOE_AT_SRC_LOC:
                case TARGET_ENUM_UNITS_ENEMY_AOE_AT_DEST_LOC:
                case TARGET_ENUM_UNITS_ENEMY_IN_CONE_24:
                case TARGET_ENUM_UNITS_ENEMY_AOE_AT_DYNOBJ_LOC:
                case TARGET_LOCATION_CASTER_TARGET_POSITION:
                case TARGET_ENUM_UNITS_ENEMY_IN_CONE_54:
                    return true;
                default:
                    return false;
            }
        }

        // Reverse engineered from binary: do not alter
        static inline bool IsSpellTargetHelpfulAtClient(uint32 target)
        {
            switch (target)
            {
                case TARGET_UNIT_CASTER:
                case TARGET_UNIT_FRIEND_NEAR_CASTER:
                case TARGET_UNIT_NEAR_CASTER:
                case TARGET_UNIT_CASTER_PET:
                case TARGET_ENUM_UNITS_PARTY_WITHIN_CASTER_RANGE:
                case TARGET_UNIT_FRIEND:
                case TARGET_UNIT_CASTER_MASTER:
                case TARGET_ENUM_UNITS_FRIEND_AOE_AT_DYNOBJ_LOC:
                case TARGET_ENUM_UNITS_FRIEND_AOE_AT_SRC_LOC:
                case TARGET_ENUM_UNITS_FRIEND_AOE_AT_DEST_LOC:
                case TARGET_ENUM_UNITS_PARTY_AOE_AT_SRC_LOC:
                case TARGET_ENUM_UNITS_PARTY_AOE_AT_DEST_LOC:
                case TARGET_UNIT_PARTY:
                case TARGET_UNIT_FRIEND_CHAIN_HEAL:
                case TARGET_ENUM_UNITS_RAID_WITHIN_CASTER_RANGE:
                case TARGET_UNIT_RAID:
                case TARGET_UNIT_RAID_NEAR_CASTER:
                case TARGET_ENUM_UNITS_FRIEND_IN_CONE:
                case TARGET_UNIT_RAID_AND_CLASS:
                case TARGET_PLAYER_RAID_NYI:
                    return true;
                default:
                    return false;
            }
        }

        // Reverse engineered from binary: do not alter
        static inline bool IsSpellEffectHarmfulAtClient(const SpellEntry &entry, SpellEffectIndex effIndex)
        {
            return (IsSpellTargetHarmfulAtClient(entry.EffectImplicitTargetA[effIndex]) || IsSpellTargetHarmfulAtClient(entry.EffectImplicitTargetB[effIndex]));
        }

        // Reverse engineered from binary: do not alter
        static inline bool IsSpellEffectHelpfulAtClient(const SpellEntry &entry, SpellEffectIndex effIndex)
        {
            if (IsSpellTargetHelpfulAtClient(entry.EffectImplicitTargetA[effIndex]))
            {
                if (entry.EffectImplicitTargetA[effIndex] != TARGET_UNIT_CASTER || entry.EffectApplyAuraName[effIndex] != SPELL_AURA_DUMMY)
                    return true;
            }

            if (IsSpellTargetHelpfulAtClient(entry.EffectImplicitTargetB[effIndex]))
            {
                if (entry.EffectImplicitTargetB[effIndex] != TARGET_UNIT_CASTER || entry.EffectApplyAuraName[effIndex] != SPELL_AURA_DUMMY)
                    return true;
            }

            return false;
        }

        // Reverse engineered from binary: do not alter
        enum SpellFaction
        {
            SPELL_NEUTRAL = 0,
            SPELL_HELPFUL = 1,
            SPELL_HARMFUL = 2,
        };

        // Reverse engineered from binary: do not alter
        static inline SpellFaction GetSpellFactionAtClient(const SpellEntry &entry, SpellEffectIndexMask mask = EFFECT_MASK_ALL)
        {
            if (entry.Targets & TARGET_FLAG_UNIT_ALLY)
                return SPELL_HELPFUL;

            if (entry.Targets & TARGET_FLAG_UNIT_ENEMY)
                return SPELL_HARMFUL;

            for (uint32 i = 0; i < MAX_EFFECT_INDEX; ++i)
            {
                // Customization: skip junk, skip by mask
                if (!entry.Effect[i] || !(mask & (1 << i)))
                    continue;

                if (IsSpellEffectHarmfulAtClient(entry, SpellEffectIndex(i)))
                    return SPELL_HARMFUL;

                if (IsSpellEffectHelpfulAtClient(entry, SpellEffectIndex(i)))
                    return SPELL_HELPFUL;
            }

            return SPELL_NEUTRAL;
        }

        SpellThreatEntry const* GetSpellThreatEntry(uint32 spellid) const
        {
            SpellThreatMap::const_iterator itr = mSpellThreatMap.find(spellid);
            if (itr != mSpellThreatMap.end())
                return &itr->second;

            return nullptr;
        }

        float GetSpellThreatMultiplier(SpellEntry const* spellInfo) const
        {
            if (!spellInfo)
                return 1.0f;

            if (SpellThreatEntry const* entry = GetSpellThreatEntry(spellInfo->Id))
                return entry->multiplier;

            return 1.0f;
        }

        // Spell proc events
        SpellProcEventEntry const* GetSpellProcEvent(uint32 spellId) const
        {
            SpellProcEventMap::const_iterator itr = mSpellProcEventMap.find(spellId);
            if (itr != mSpellProcEventMap.end())
                return &itr->second;
            return nullptr;
        }

        // Spell procs from item enchants
        float GetItemEnchantProcChance(uint32 spellid) const
        {
            SpellProcItemEnchantMap::const_iterator itr = mSpellProcItemEnchantMap.find(spellid);
            if (itr == mSpellProcItemEnchantMap.end())
                return 0.0f;

            return itr->second;
        }

        static bool IsSpellProcEventCanTriggeredBy(SpellProcEventEntry const* spellProcEvent, uint32 EventProcFlag, SpellEntry const* procSpell, uint32 procFlags, uint32 procExtra);

        // Spell bonus data
        SpellBonusEntry const* GetSpellBonusData(uint32 spellId) const
        {
            // Lookup data
            SpellBonusMap::const_iterator itr = mSpellBonusMap.find(spellId);
            if (itr != mSpellBonusMap.end())
                return &itr->second;

            return nullptr;
        }

        uint32 GetSpellFacingFlag(uint32 spellId) const
        {
            SpellFacingFlagMap::const_iterator itr =  mSpellFacingFlagMap.find(spellId);
            if (itr != mSpellFacingFlagMap.end())
                return itr->second;
            return 0x0;
        }

        // Spell target coordinates
        SpellTargetPosition const* GetSpellTargetPosition(uint32 spell_id) const
        {
            SpellTargetPositionMap::const_iterator itr = mSpellTargetPositions.find(spell_id);
            if (itr != mSpellTargetPositions.end())
                return &itr->second;
            return nullptr;
        }

        // Spell ranks chains
        SpellChainNode const* GetSpellChainNode(uint32 spell_id) const
        {
            SpellChainMap::const_iterator itr = mSpellChains.find(spell_id);
            if (itr == mSpellChains.end())
                return nullptr;

            return &itr->second;
        }

        uint32 GetFirstSpellInChain(uint32 spell_id) const
        {
            if (SpellChainNode const* node = GetSpellChainNode(spell_id))
                return node->first;

            return spell_id;
        }

        uint32 GetPrevSpellInChain(uint32 spell_id) const
        {
            if (SpellChainNode const* node = GetSpellChainNode(spell_id))
                return node->prev;

            return 0;
        }

        uint32 GetNextSpellInChain(uint32 spell_id) const
        {
            SpellChainMapNext const& nextMap = GetSpellChainNext();

            for (SpellChainMapNext::const_iterator itr = nextMap.lower_bound(spell_id); itr != nextMap.upper_bound(spell_id); ++itr)
            {
                SpellChainNode const* node = GetSpellChainNode(itr->second);

                // If next spell is a requirement for this one then skip it
                if (node->req == spell_id)
                    continue;

                if (node->prev == spell_id)
                    return itr->second;
            }

            return 0;
        }

        SpellChainMapNext const& GetSpellChainNext() const { return mSpellChainsNext; }

        template<typename Worker>
        void doForHighRanks(uint32 spellid, Worker& worker)
        {
            SpellChainMapNext const& nextMap = GetSpellChainNext();
            for (SpellChainMapNext::const_iterator itr = nextMap.lower_bound(spellid); itr != nextMap.upper_bound(spellid); ++itr)
            {
                worker(itr->second);
                doForHighRanks(itr->second, worker);
            }
        }

        // Note: not use rank for compare to spell ranks: spell chains isn't linear order
        // Use IsSpellHigherRankOfSpell instead
        uint8 GetSpellRank(uint32 spell_id) const
        {
            if (SpellChainNode const* node = GetSpellChainNode(spell_id))
                return node->rank;

            return 0;
        }

        bool IsSpellHigherRankOfSpell(uint32 spellId1, uint32 spellId2) const
        {
            if (spellId1 == spellId2)
                return false;

            SpellChainMap::const_iterator itr = mSpellChains.find(spellId1);

            uint32 rank2 = GetSpellRank(spellId2);

            // not ordered correctly by rank value
            if (itr == mSpellChains.end() || !rank2 || itr->second.rank <= rank2)
                return false;

            // check present in same rank chain
            for (; itr != mSpellChains.end(); itr = mSpellChains.find(itr->second.prev))
                if (itr->second.prev == spellId2)
                    return true;

            return false;
        }

        inline bool IsSpellAnotherRankOfSpell(uint32 spellId1, uint32 spellId2) const
        {
            return (spellId1 != spellId2 && GetFirstSpellInChain(spellId1) == GetFirstSpellInChain(spellId2));
        }

        bool IsSingleTargetSpell(SpellEntry const* entry) const
        {
            // Pre-TBC: SPELL_ATTR_EX5_SINGLE_TARGET_SPELL substitute code
            // Not AoE
            if (IsAreaOfEffectSpell(entry))
                return false;

            // Mechanics
            switch (entry->Mechanic)
            {
                case MECHANIC_FEAR:         // Includes: Warlock's Fear, Scare Beast
                case MECHANIC_TURN:         // Turn Undead
                    // Always single-target in classic
                    return true;
                case MECHANIC_ROOT:
                case MECHANIC_SLEEP:        // Includes: Hibernate, Wyvern Sting
                case MECHANIC_KNOCKOUT:     // Includes: Sap, Gouge
                case MECHANIC_POLYMORPH:
                case MECHANIC_BANISH:
                case MECHANIC_SHACKLE:
                    // Only spells used by players seem to be subjects to single target mechanics in classic
                    return (entry->SpellFamilyName && entry->SpellFamilyFlags.Flags);
            }

            // Hunter's Mark mechanics
            if (entry->SpellFamilyName == SPELLFAMILY_HUNTER && IsSpellHaveAura(entry, SPELL_AURA_MOD_STALKED))
                return true;

            return false;
        }

        bool IsSingleTargetSpells(SpellEntry const* entry1, SpellEntry const* entry2) const
        {
            if (!IsSingleTargetSpell(entry1) || !IsSingleTargetSpell(entry2))
                return false;

            // Early instance of same spell check
            if (entry1 == entry2)
                return true;

            // One spell is a rank of another spell (same spell chain)
            if (GetFirstSpellInChain(entry1->Id) == GetFirstSpellInChain(entry2->Id))
                return true;

            return false;
        }

        bool IsSpellRankOfSpell(SpellEntry const* spellInfo_1, uint32 spellId_2) const;
        bool IsSpellStackableWithSpell(const SpellEntry* entry1, const SpellEntry* entry2) const
        {
            if (!entry1 || !entry2)
                return true;

            // Uncancellable spells are expected to be persistent at all times
            if (entry1->HasAttribute(SPELL_ATTR_CANT_CANCEL) || entry2->HasAttribute(SPELL_ATTR_CANT_CANCEL))
                return true;

            // Allow stacking passive and active spells
            if (entry1->HasAttribute(SPELL_ATTR_PASSIVE) != entry2->HasAttribute(SPELL_ATTR_PASSIVE))
                return true;

            return IsStackableSpell(entry1, entry2);
        }

        bool IsSpellStackableWithSpellForDifferentCasters(const SpellEntry* entry1, const SpellEntry* entry2) const
        {
            if (!entry1 || !entry2)
                return true;

            // If spells are two instances of the same spell, check attribute first, and formal aura holder stacking rules after
            if (entry1 == entry2)
                return (entry1->HasAttribute(SPELL_ATTR_EX3_STACK_FOR_DIFF_CASTERS) || IsSpellStackableWithSpell(entry1, entry2));

            // If spells are in the same spell chain
            if (IsSpellAnotherRankOfSpell(entry1->Id, entry2->Id))
            {
                // Both ranks have attribute, allow stacking
                if (entry1->HasAttribute(SPELL_ATTR_EX3_STACK_FOR_DIFF_CASTERS) && entry2->HasAttribute(SPELL_ATTR_EX3_STACK_FOR_DIFF_CASTERS))
                    return true;
            }

            // By default, check formal aura holder stacking rules
            return IsSpellStackableWithSpell(entry1, entry2);
        }

        uint32 GetSpellBookSuccessorSpellId(uint32 spellId)
        {
            SkillLineAbilityMapBounds bounds = GetSkillLineAbilityMapBoundsBySpellId(spellId);
            for (SkillLineAbilityMap::const_iterator itr = bounds.first; itr != bounds.second; ++itr)
            {
                if (SkillLineAbilityEntry const* pAbility = itr->second)
                {
                    if (pAbility->forward_spellid)
                        return pAbility->forward_spellid;
                }
            }
            return 0;
        }

        // return true if spell1 can affect spell2
        bool IsSpellCanAffectSpell(SpellEntry const* spellInfo_1, SpellEntry const* spellInfo_2) const;

        SpellEntry const* SelectAuraRankForLevel(SpellEntry const* spellInfo, uint32 level) const;

        // Spell learning
        SpellLearnSkillNode const* GetSpellLearnSkill(uint32 spell_id) const
        {
            SpellLearnSkillMap::const_iterator itr = mSpellLearnSkills.find(spell_id);
            if (itr != mSpellLearnSkills.end())
                return &itr->second;
            return nullptr;
        }

        bool IsSpellLearnSpell(uint32 spell_id) const
        {
            return mSpellLearnSpells.find(spell_id) != mSpellLearnSpells.end();
        }

        SpellLearnSpellMapBounds GetSpellLearnSpellMapBounds(uint32 spell_id) const
        {
            return mSpellLearnSpells.equal_range(spell_id);
        }

        bool IsSpellLearnToSpell(uint32 parent, uint32 child) const
        {
            SpellLearnSpellMapBounds bounds = GetSpellLearnSpellMapBounds(parent);
            for (SpellLearnSpellMap::const_iterator i = bounds.first; i != bounds.second; ++i)
                if (i->second.spell == child)
                    return true;
            return false;
        }

        static bool IsProfessionOrRidingSpell(uint32 spellId);
        static bool IsProfessionSpell(uint32 spellId);
        static bool IsPrimaryProfessionSpell(uint32 spellId);
        bool IsPrimaryProfessionFirstRankSpell(uint32 spellId) const;

        bool IsSkillBonusSpell(uint32 spellId) const;

        // Spell correctness for client using
        static bool IsSpellValid(SpellEntry const* spellInfo, Player* pl = nullptr, bool msg = true);

        SkillLineAbilityMapBounds GetSkillLineAbilityMapBoundsBySpellId(uint32 spellId) const
        {
            return mSkillLineAbilityMapBySpellId.equal_range(spellId);
        }

        SkillLineAbilityMapBounds GetSkillLineAbilityMapBoundsBySkillId(uint32 skillId) const
        {
            return mSkillLineAbilityMapBySkillId.equal_range(skillId);
        }

        SkillRaceClassInfoMapBounds GetSkillRaceClassInfoMapBounds(uint32 skill_id) const
        {
            return mSkillRaceClassInfoMap.equal_range(skill_id);
        }

        PetAura const* GetPetAura(uint32 spell_id)
        {
            SpellPetAuraMap::const_iterator itr = mSpellPetAuraMap.find(spell_id);
            if (itr != mSpellPetAuraMap.end())
                return &itr->second;
            else
                return nullptr;
        }

        SpellCastResult GetSpellAllowedInLocationError(SpellEntry const* spellInfo, uint32 map_id, uint32 zone_id, uint32 area_id, Player const* player = nullptr) const;

        SpellAreaMapBounds GetSpellAreaMapBounds(uint32 spell_id) const
        {
            return mSpellAreaMap.equal_range(spell_id);
        }

        SpellAreaForAuraMapBounds GetSpellAreaForAuraMapBounds(uint32 spell_id) const
        {
            return mSpellAreaForAuraMap.equal_range(spell_id);
        }

        SpellAreaForAreaMapBounds GetSpellAreaForAreaMapBounds(uint32 area_id) const
        {
            return mSpellAreaForAreaMap.equal_range(area_id);
        }

        // Modifiers
    public:
        static SpellMgr& Instance();

        void CheckUsedSpells(char const* table) const;

        // Loading data at server startup
        void LoadSpellChains();
        void LoadSpellLearnSkills();
        void LoadSpellLearnSpells();
        void LoadSpellScriptTarget();
        void LoadSpellAffects();
        void LoadSpellElixirs();
        void LoadSpellProcEvents();
        void LoadSpellProcItemEnchant();
        void LoadSpellBonuses();
        void LoadSpellTargetPositions();
        void LoadSpellThreats();
        void LoadSkillLineAbilityMaps();
        void LoadSkillRaceClassInfoMap();
        void LoadSpellPetAuras();
        void LoadSpellAreas();
        void LoadFacingCasterFlags();

    private:
        SpellChainMap      mSpellChains;
        SpellChainMapNext  mSpellChainsNext;
        SpellLearnSkillMap mSpellLearnSkills;
        SpellLearnSpellMap mSpellLearnSpells;
        SpellTargetPositionMap mSpellTargetPositions;
        SpellAffectMap     mSpellAffectMap;
        SpellElixirMap     mSpellElixirs;
        SpellThreatMap     mSpellThreatMap;
        SpellProcEventMap  mSpellProcEventMap;
        SpellProcItemEnchantMap mSpellProcItemEnchantMap;
        SpellBonusMap      mSpellBonusMap;
        SkillLineAbilityMap mSkillLineAbilityMapBySpellId;
        SkillLineAbilityMap mSkillLineAbilityMapBySkillId;
        SkillRaceClassInfoMap mSkillRaceClassInfoMap;
        SpellPetAuraMap     mSpellPetAuraMap;
        SpellAreaMap         mSpellAreaMap;
        SpellAreaForAuraMap  mSpellAreaForAuraMap;
        SpellAreaForAreaMap  mSpellAreaForAreaMap;
        SpellFacingFlagMap  mSpellFacingFlagMap;
};

#define sSpellMgr SpellMgr::Instance()
#endif
