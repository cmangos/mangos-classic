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

#include "Common.h"
#include "Log/Log.h"
#include "Globals/ObjectMgr.h"
#include "Spells/SpellMgr.h"
#include "Entities/Player.h"
#include "Entities/Unit.h"
#include "Spells/Spell.h"
#include "Spells/SpellAuras.h"
#include "Entities/Totem.h"
#include "Entities/Creature.h"
#include "Util/Util.h"

pAuraProcHandler AuraProcHandler[TOTAL_AURAS] =
{
    &Unit::HandleNULLProc,                                  //  0 SPELL_AURA_NONE
    &Unit::HandleNULLProc,                                  //  1 SPELL_AURA_BIND_SIGHT
    &Unit::HandleNULLProc,                                  //  2 SPELL_AURA_MOD_POSSESS
    &Unit::HandleNULLProc,                                  //  3 SPELL_AURA_PERIODIC_DAMAGE
    &Unit::HandleDummyAuraProc,                             //  4 SPELL_AURA_DUMMY
    &Unit::HandleNULLProc,                                  //  5 SPELL_AURA_MOD_CONFUSE
    &Unit::HandleNULLProc,                                  //  6 SPELL_AURA_MOD_CHARM
    &Unit::HandleRemoveByDamageChanceProc,                  //  7 SPELL_AURA_MOD_FEAR
    &Unit::HandleNULLProc,                                  //  8 SPELL_AURA_PERIODIC_HEAL
    &Unit::HandleNULLProc,                                  //  9 SPELL_AURA_MOD_ATTACKSPEED
    &Unit::HandleNULLProc,                                  // 10 SPELL_AURA_MOD_THREAT
    &Unit::HandleNULLProc,                                  // 11 SPELL_AURA_MOD_TAUNT
    &Unit::HandleNULLProc,                                  // 12 SPELL_AURA_MOD_STUN
    &Unit::HandleNULLProc,                                  // 13 SPELL_AURA_MOD_DAMAGE_DONE
    &Unit::HandleNULLProc,                                  // 14 SPELL_AURA_MOD_DAMAGE_TAKEN
    &Unit::HandleNULLProc,                                  // 15 SPELL_AURA_DAMAGE_SHIELD
    &Unit::HandleNULLProc,                                  // 16 SPELL_AURA_MOD_STEALTH
    &Unit::HandleNULLProc,                                  // 17 SPELL_AURA_MOD_STEALTH_DETECT
    &Unit::HandleInvisibilityAuraProc,                      // 18 SPELL_AURA_MOD_INVISIBILITY
    &Unit::HandleNULLProc,                                  // 19 SPELL_AURA_MOD_INVISIBILITY_DETECTION
    &Unit::HandleNULLProc,                                  // 20 SPELL_AURA_OBS_MOD_HEALTH
    &Unit::HandleNULLProc,                                  // 21 SPELL_AURA_OBS_MOD_MANA
    &Unit::HandleModResistanceAuraProc,                     // 22 SPELL_AURA_MOD_RESISTANCE
    &Unit::HandleNULLProc,                                  // 23 SPELL_AURA_PERIODIC_TRIGGER_SPELL
    &Unit::HandleNULLProc,                                  // 24 SPELL_AURA_PERIODIC_ENERGIZE
    &Unit::HandleNULLProc,                                  // 25 SPELL_AURA_MOD_PACIFY
    &Unit::HandleRemoveByDamageChanceProc,                  // 26 SPELL_AURA_MOD_ROOT
    &Unit::HandleNULLProc,                                  // 27 SPELL_AURA_MOD_SILENCE
    &Unit::HandleNULLProc,                                  // 28 SPELL_AURA_REFLECT_SPELLS
    &Unit::HandleNULLProc,                                  // 29 SPELL_AURA_MOD_STAT
    &Unit::HandleNULLProc,                                  // 30 SPELL_AURA_MOD_SKILL
    &Unit::HandleNULLProc,                                  // 31 SPELL_AURA_MOD_INCREASE_SPEED
    &Unit::HandleNULLProc,                                  // 32 SPELL_AURA_MOD_INCREASE_MOUNTED_SPEED
    &Unit::HandleNULLProc,                                  // 33 SPELL_AURA_MOD_DECREASE_SPEED
    &Unit::HandleNULLProc,                                  // 34 SPELL_AURA_MOD_INCREASE_HEALTH
    &Unit::HandleNULLProc,                                  // 35 SPELL_AURA_MOD_INCREASE_ENERGY
    &Unit::HandleNULLProc,                                  // 36 SPELL_AURA_MOD_SHAPESHIFT
    &Unit::HandleNULLProc,                                  // 37 SPELL_AURA_EFFECT_IMMUNITY
    &Unit::HandleNULLProc,                                  // 38 SPELL_AURA_STATE_IMMUNITY
    &Unit::HandleNULLProc,                                  // 39 SPELL_AURA_SCHOOL_IMMUNITY
    &Unit::HandleNULLProc,                                  // 40 SPELL_AURA_DAMAGE_IMMUNITY
    &Unit::HandleNULLProc,                                  // 41 SPELL_AURA_DISPEL_IMMUNITY
    &Unit::HandleProcTriggerSpellAuraProc,                  // 42 SPELL_AURA_PROC_TRIGGER_SPELL
    &Unit::HandleProcTriggerDamageAuraProc,                 // 43 SPELL_AURA_PROC_TRIGGER_DAMAGE
    &Unit::HandleNULLProc,                                  // 44 SPELL_AURA_TRACK_CREATURES
    &Unit::HandleNULLProc,                                  // 45 SPELL_AURA_TRACK_RESOURCES
    &Unit::HandleNULLProc,                                  // 46 SPELL_AURA_46
    &Unit::HandleNULLProc,                                  // 47 SPELL_AURA_MOD_PARRY_PERCENT
    &Unit::HandleNULLProc,                                  // 48 SPELL_AURA_48
    &Unit::HandleNULLProc,                                  // 49 SPELL_AURA_MOD_DODGE_PERCENT
    &Unit::HandleNULLProc,                                  // 50 SPELL_AURA_MOD_BLOCK_SKILL    obsolete?
    &Unit::HandleNULLProc,                                  // 51 SPELL_AURA_MOD_BLOCK_PERCENT
    &Unit::HandleNULLProc,                                  // 52 SPELL_AURA_MOD_CRIT_PERCENT
    &Unit::HandleNULLProc,                                  // 53 SPELL_AURA_PERIODIC_LEECH
    &Unit::HandleNULLProc,                                  // 54 SPELL_AURA_MOD_HIT_CHANCE
    &Unit::HandleNULLProc,                                  // 55 SPELL_AURA_MOD_SPELL_HIT_CHANCE
    &Unit::HandleNULLProc,                                  // 56 SPELL_AURA_TRANSFORM
    &Unit::HandleNULLProc,                                  // 57 SPELL_AURA_MOD_SPELL_CRIT_CHANCE
    &Unit::HandleNULLProc,                                  // 58 SPELL_AURA_MOD_INCREASE_SWIM_SPEED
    &Unit::HandleNULLProc,                                  // 59 SPELL_AURA_MOD_DAMAGE_DONE_CREATURE
    &Unit::HandleRemoveByDamageChanceProc,                  // 60 SPELL_AURA_MOD_PACIFY_SILENCE
    &Unit::HandleNULLProc,                                  // 61 SPELL_AURA_MOD_SCALE
    &Unit::HandleNULLProc,                                  // 62 SPELL_AURA_PERIODIC_HEALTH_FUNNEL
    &Unit::HandleNULLProc,                                  // 63 SPELL_AURA_PERIODIC_MANA_FUNNEL obsolete?
    &Unit::HandleNULLProc,                                  // 64 SPELL_AURA_PERIODIC_MANA_LEECH
    &Unit::HandleModCastingSpeedNotStackAuraProc,           // 65 SPELL_AURA_MOD_CASTING_SPEED_NOT_STACK
    &Unit::HandleNULLProc,                                  // 66 SPELL_AURA_FEIGN_DEATH
    &Unit::HandleNULLProc,                                  // 67 SPELL_AURA_MOD_DISARM
    &Unit::HandleNULLProc,                                  // 68 SPELL_AURA_MOD_STALKED
    &Unit::HandleNULLProc,                                  // 69 SPELL_AURA_SCHOOL_ABSORB
    &Unit::HandleNULLProc,                                  // 70 SPELL_AURA_EXTRA_ATTACKS      Useless, used by only one spell that has only visual effect
    &Unit::HandleNULLProc,                                  // 71 SPELL_AURA_MOD_SPELL_CRIT_CHANCE_SCHOOL
    &Unit::HandleModPowerCostSchoolAuraProc,                // 72 SPELL_AURA_MOD_POWER_COST_SCHOOL_PCT
    &Unit::HandleModPowerCostSchoolAuraProc,                // 73 SPELL_AURA_MOD_POWER_COST_SCHOOL
    &Unit::HandleReflectSpellsSchoolAuraProc,               // 74 SPELL_AURA_REFLECT_SPELLS_SCHOOL
    &Unit::HandleNULLProc,                                  // 75 SPELL_AURA_MOD_LANGUAGE
    &Unit::HandleNULLProc,                                  // 76 SPELL_AURA_FAR_SIGHT
    &Unit::HandleMechanicImmuneResistanceAuraProc,          // 77 SPELL_AURA_MECHANIC_IMMUNITY
    &Unit::HandleNULLProc,                                  // 78 SPELL_AURA_MOUNTED
    &Unit::HandleNULLProc,                                  // 79 SPELL_AURA_MOD_DAMAGE_PERCENT_DONE
    &Unit::HandleNULLProc,                                  // 80 SPELL_AURA_MOD_PERCENT_STAT
    &Unit::HandleNULLProc,                                  // 81 SPELL_AURA_SPLIT_DAMAGE_PCT
    &Unit::HandleNULLProc,                                  // 82 SPELL_AURA_WATER_BREATHING
    &Unit::HandleNULLProc,                                  // 83 SPELL_AURA_MOD_BASE_RESISTANCE
    &Unit::HandleNULLProc,                                  // 84 SPELL_AURA_MOD_REGEN
    &Unit::HandleCantTrigger,                               // 85 SPELL_AURA_MOD_POWER_REGEN
    &Unit::HandleNULLProc,                                  // 86 SPELL_AURA_CHANNEL_DEATH_ITEM
    &Unit::HandleNULLProc,                                  // 87 SPELL_AURA_MOD_DAMAGE_PERCENT_TAKEN
    &Unit::HandleNULLProc,                                  // 88 SPELL_AURA_MOD_HEALTH_REGEN_PERCENT
    &Unit::HandleNULLProc,                                  // 89 SPELL_AURA_PERIODIC_DAMAGE_PERCENT
    &Unit::HandleNULLProc,                                  // 90 SPELL_AURA_MOD_RESIST_CHANCE  Useless
    &Unit::HandleNULLProc,                                  // 91 SPELL_AURA_MOD_DETECT_RANGE
    &Unit::HandleNULLProc,                                  // 92 SPELL_AURA_PREVENTS_FLEEING
    &Unit::HandleNULLProc,                                  // 93 SPELL_AURA_MOD_UNATTACKABLE
    &Unit::HandleNULLProc,                                  // 94 SPELL_AURA_INTERRUPT_REGEN
    &Unit::HandleNULLProc,                                  // 95 SPELL_AURA_GHOST
    &Unit::HandleMagnetAuraProc,                            // 96 SPELL_AURA_SPELL_MAGNET
    &Unit::HandleNULLProc,                                  // 97 SPELL_AURA_MANA_SHIELD
    &Unit::HandleNULLProc,                                  // 98 SPELL_AURA_MOD_SKILL_TALENT
    &Unit::HandleNULLProc,                                  // 99 SPELL_AURA_MOD_ATTACK_POWER
    &Unit::HandleNULLProc,                                  // 100 SPELL_AURA_AURAS_VISIBLE obsolete? all player can see all auras now
    &Unit::HandleNULLProc,                                  // 101 SPELL_AURA_MOD_RESISTANCE_PCT
    &Unit::HandleNULLProc,                                  // 102 SPELL_AURA_MOD_MELEE_ATTACK_POWER_VERSUS
    &Unit::HandleNULLProc,                                  // 103 SPELL_AURA_MOD_TOTAL_THREAT
    &Unit::HandleNULLProc,                                  // 104 SPELL_AURA_WATER_WALK
    &Unit::HandleNULLProc,                                  // 105 SPELL_AURA_FEATHER_FALL
    &Unit::HandleNULLProc,                                  // 106 SPELL_AURA_HOVER
    &Unit::HandleSpellModProc,                              // 107 SPELL_AURA_ADD_FLAT_MODIFIER
    &Unit::HandleSpellModProc,                              // 108 SPELL_AURA_ADD_PCT_MODIFIER
    &Unit::HandleNULLProc,                                  // 109 SPELL_AURA_ADD_TARGET_TRIGGER
    &Unit::HandleNULLProc,                                  // 110 SPELL_AURA_MOD_POWER_REGEN_PERCENT
    &Unit::HandleNULLProc,                                  // 111 SPELL_AURA_ADD_CASTER_HIT_TRIGGER
    &Unit::HandleOverrideClassScriptAuraProc,               // 112 SPELL_AURA_OVERRIDE_CLASS_SCRIPTS
    &Unit::HandleNULLProc,                                  // 113 SPELL_AURA_MOD_RANGED_DAMAGE_TAKEN
    &Unit::HandleNULLProc,                                  // 114 SPELL_AURA_MOD_RANGED_DAMAGE_TAKEN_PCT
    &Unit::HandleNULLProc,                                  // 115 SPELL_AURA_MOD_HEALING
    &Unit::HandleNULLProc,                                  // 116 SPELL_AURA_MOD_REGEN_DURING_COMBAT
    &Unit::HandleMechanicImmuneResistanceAuraProc,          // 117 SPELL_AURA_MOD_MECHANIC_RESISTANCE
    &Unit::HandleNULLProc,                                  // 118 SPELL_AURA_MOD_HEALING_PCT
    &Unit::HandleNULLProc,                                  // 119 SPELL_AURA_SHARE_PET_TRACKING useless
    &Unit::HandleNULLProc,                                  // 120 SPELL_AURA_UNTRACKABLE
    &Unit::HandleNULLProc,                                  // 121 SPELL_AURA_EMPATHY
    &Unit::HandleNULLProc,                                  // 122 SPELL_AURA_MOD_OFFHAND_DAMAGE_PCT
    &Unit::HandleNULLProc,                                  // 123 SPELL_AURA_MOD_TARGET_RESISTANCE
    &Unit::HandleNULLProc,                                  // 124 SPELL_AURA_MOD_RANGED_ATTACK_POWER
    &Unit::HandleNULLProc,                                  // 125 SPELL_AURA_MOD_MELEE_DAMAGE_TAKEN
    &Unit::HandleNULLProc,                                  // 126 SPELL_AURA_MOD_MELEE_DAMAGE_TAKEN_PCT
    &Unit::HandleNULLProc,                                  // 127 SPELL_AURA_RANGED_ATTACK_POWER_ATTACKER_BONUS
    &Unit::HandleNULLProc,                                  // 128 SPELL_AURA_MOD_POSSESS_PET
    &Unit::HandleNULLProc,                                  // 129 SPELL_AURA_MOD_SPEED_ALWAYS
    &Unit::HandleNULLProc,                                  // 130 SPELL_AURA_MOD_MOUNTED_SPEED_ALWAYS
    &Unit::HandleNULLProc,                                  // 131 SPELL_AURA_MOD_RANGED_ATTACK_POWER_VERSUS
    &Unit::HandleNULLProc,                                  // 132 SPELL_AURA_MOD_INCREASE_ENERGY_PERCENT
    &Unit::HandleNULLProc,                                  // 133 SPELL_AURA_MOD_INCREASE_HEALTH_PERCENT
    &Unit::HandleNULLProc,                                  // 134 SPELL_AURA_MOD_MANA_REGEN_INTERRUPT
    &Unit::HandleNULLProc,                                  // 135 SPELL_AURA_MOD_HEALING_DONE
    &Unit::HandleNULLProc,                                  // 136 SPELL_AURA_MOD_HEALING_DONE_PERCENT
    &Unit::HandleNULLProc,                                  // 137 SPELL_AURA_MOD_TOTAL_STAT_PERCENTAGE
    &Unit::HandleHasteAuraProc,                             // 138 SPELL_AURA_MOD_MELEE_HASTE
    &Unit::HandleNULLProc,                                  // 139 SPELL_AURA_FORCE_REACTION
    &Unit::HandleNULLProc,                                  // 140 SPELL_AURA_MOD_RANGED_HASTE
    &Unit::HandleNULLProc,                                  // 141 SPELL_AURA_MOD_RANGED_AMMO_HASTE
    &Unit::HandleNULLProc,                                  // 142 SPELL_AURA_MOD_BASE_RESISTANCE_PCT
    &Unit::HandleNULLProc,                                  // 143 SPELL_AURA_MOD_RESISTANCE_EXCLUSIVE
    &Unit::HandleNULLProc,                                  // 144 SPELL_AURA_SAFE_FALL
    &Unit::HandleNULLProc,                                  // 145 SPELL_AURA_CHARISMA obsolete?
    &Unit::HandleNULLProc,                                  // 146 SPELL_AURA_PERSUADED obsolete?
    &Unit::HandleNULLProc,                                  // 147 SPELL_AURA_MECHANIC_IMMUNITY_MASK
    &Unit::HandleNULLProc,                                  // 148 SPELL_AURA_RETAIN_COMBO_POINTS
    &Unit::HandleCantTrigger,                               // 149 SPELL_AURA_RESIST_PUSHBACK
    &Unit::HandleNULLProc,                                  // 150 SPELL_AURA_MOD_SHIELD_BLOCKVALUE_PCT
    &Unit::HandleNULLProc,                                  // 151 SPELL_AURA_TRACK_STEALTHED
    &Unit::HandleNULLProc,                                  // 152 SPELL_AURA_MOD_DETECTED_RANGE
    &Unit::HandleNULLProc,                                  // 153 SPELL_AURA_SPLIT_DAMAGE_FLAT
    &Unit::HandleNULLProc,                                  // 154 SPELL_AURA_MOD_STEALTH_LEVEL
    &Unit::HandleNULLProc,                                  // 155 SPELL_AURA_MOD_WATER_BREATHING
    &Unit::HandleNULLProc,                                  // 156 SPELL_AURA_MOD_REPUTATION_GAIN
    &Unit::HandleNULLProc,                                  // 157 SPELL_AURA_PET_DAMAGE_MULTI (single test like spell 20782, also single for 214 aura)
    &Unit::HandleNULLProc,                                  // 158 SPELL_AURA_MOD_SHIELD_BLOCKVALUE
    &Unit::HandleNULLProc,                                  // 159 SPELL_AURA_NO_PVP_CREDIT      only for Honorless Target spell
    &Unit::HandleNULLProc,                                  // 160 SPELL_AURA_MOD_AOE_AVOIDANCE
    &Unit::HandleNULLProc,                                  // 161 SPELL_AURA_MOD_HEALTH_REGEN_IN_COMBAT
    &Unit::HandleNULLProc,                                  // 162 SPELL_AURA_POWER_BURN_MANA
    &Unit::HandleNULLProc,                                  // 163 SPELL_AURA_MOD_CRIT_DAMAGE_BONUS
    &Unit::HandleNULLProc,                                  // 164 useless, only one test spell
    &Unit::HandleNULLProc,                                  // 165 SPELL_AURA_MELEE_ATTACK_POWER_ATTACKER_BONUS
    &Unit::HandleNULLProc,                                  // 166 SPELL_AURA_MOD_ATTACK_POWER_PCT
    &Unit::HandleNULLProc,                                  // 167 SPELL_AURA_MOD_RANGED_ATTACK_POWER_PCT
    &Unit::HandleNULLProc,                                  // 168 SPELL_AURA_MOD_DAMAGE_DONE_VERSUS
    &Unit::HandleNULLProc,                                  // 169 SPELL_AURA_MOD_CRIT_PERCENT_VERSUS
    &Unit::HandleNULLProc,                                  // 170 SPELL_AURA_DETECT_AMORE       only for Detect Amore spell
    &Unit::HandleNULLProc,                                  // 171 SPELL_AURA_MOD_SPEED_NOT_STACK
    &Unit::HandleNULLProc,                                  // 172 SPELL_AURA_MOD_MOUNTED_SPEED_NOT_STACK
    &Unit::HandleNULLProc,                                  // 173 SPELL_AURA_ALLOW_CHAMPION_SPELLS  only for Proclaim Champion spell
    &Unit::HandleNULLProc,                                  // 174 SPELL_AURA_MOD_SPELL_DAMAGE_OF_STAT_PERCENT
    &Unit::HandleNULLProc,                                  // 175 SPELL_AURA_MOD_SPELL_HEALING_OF_STAT_PERCENT
    &Unit::HandleNULLProc,                                  // 176 SPELL_AURA_SPIRIT_OF_REDEMPTION   only for Spirit of Redemption spell, die at aura end
    &Unit::HandleNULLProc,                                  // 177 SPELL_AURA_AOE_CHARM
    &Unit::HandleNULLProc,                                  // 178 SPELL_AURA_MOD_DEBUFF_RESISTANCE
    &Unit::HandleNULLProc,                                  // 179 SPELL_AURA_MOD_ATTACKER_SPELL_CRIT_CHANCE
    &Unit::HandleNULLProc,                                  // 180 SPELL_AURA_MOD_FLAT_SPELL_DAMAGE_VERSUS
    &Unit::HandleNULLProc,                                  // 181 SPELL_AURA_MOD_FLAT_SPELL_CRIT_DAMAGE_VERSUS unused
    &Unit::HandleNULLProc,                                  // 182 SPELL_AURA_MOD_RESISTANCE_OF_STAT_PERCENT
    &Unit::HandleNULLProc,                                  // 183 SPELL_AURA_MOD_CRITICAL_THREAT only used in 28746
    &Unit::HandleNULLProc,                                  // 184 SPELL_AURA_MOD_ATTACKER_MELEE_HIT_CHANCE
    &Unit::HandleNULLProc,                                  // 185 SPELL_AURA_MOD_ATTACKER_RANGED_HIT_CHANCE
    &Unit::HandleNULLProc,                                  // 186 SPELL_AURA_MOD_ATTACKER_SPELL_HIT_CHANCE
    &Unit::HandleNULLProc,                                  // 187 SPELL_AURA_MOD_ATTACKER_MELEE_CRIT_CHANCE
    &Unit::HandleNULLProc,                                  // 188 SPELL_AURA_MOD_ATTACKER_RANGED_CRIT_CHANCE
    &Unit::HandleNULLProc,                                  // 189 SPELL_AURA_MOD_RATING
    &Unit::HandleNULLProc,                                  // 190 SPELL_AURA_MOD_FACTION_REPUTATION_GAIN
    &Unit::HandleNULLProc,                                  // 191 SPELL_AURA_USE_NORMAL_MOVEMENT_SPEED
};

struct ProcTriggeredData
{
    ProcTriggeredData(SpellProcEventEntry const* _spellProcEvent, SpellAuraHolder* _triggeredByHolder)
        : spellProcEvent(_spellProcEvent), triggeredByHolder(_triggeredByHolder), canProc{true, true, true}
    {}
    SpellProcEventEntry const* spellProcEvent;
    SpellAuraHolder* triggeredByHolder;
    bool canProc[MAX_EFFECT_INDEX];
};

typedef std::list< ProcTriggeredData > ProcTriggeredList;

uint32 createProcExtendMask(SpellNonMeleeDamage* spellDamageInfo, SpellMissInfo missCondition)
{
    uint32 procEx = PROC_EX_NONE;
    // Check victim state
    if (missCondition != SPELL_MISS_NONE)
        switch (missCondition)
        {
            case SPELL_MISS_MISS:    procEx |= PROC_EX_MISS;   break;
            case SPELL_MISS_RESIST:  procEx |= PROC_EX_RESIST; break;
            case SPELL_MISS_DODGE:   procEx |= PROC_EX_DODGE;  break;
            case SPELL_MISS_PARRY:   procEx |= PROC_EX_PARRY;  break;
            case SPELL_MISS_BLOCK:   procEx |= PROC_EX_BLOCK;  break;
            case SPELL_MISS_EVADE:   procEx |= PROC_EX_EVADE;  break;
            case SPELL_MISS_IMMUNE:  procEx |= PROC_EX_IMMUNE; break;
            case SPELL_MISS_IMMUNE2: procEx |= PROC_EX_IMMUNE; break;
            case SPELL_MISS_DEFLECT: procEx |= PROC_EX_DEFLECT; break;
            case SPELL_MISS_ABSORB:  procEx |= PROC_EX_ABSORB; break;
            case SPELL_MISS_REFLECT: procEx |= PROC_EX_REFLECT; break;
            default:
                break;
        }
    else
    {
        // On block
        if (spellDamageInfo->blocked)
            procEx |= PROC_EX_BLOCK;
        // On absorb
        if (spellDamageInfo->absorb)
            procEx |= PROC_EX_ABSORB;
        // On crit
        if (spellDamageInfo->HitInfo & SPELL_HIT_TYPE_CRIT)
            procEx |= PROC_EX_CRITICAL_HIT;
        else
            procEx |= PROC_EX_NORMAL_HIT;
    }
    return procEx;
}

void Unit::ProcSkillsAndReactives(bool isVictim, Unit* target, uint32 procFlags, uint32 procEx, WeaponAttackType attType)
{
    // For melee/ranged based attack need update skills and set some Aura states
    if (procFlags & MELEE_BASED_TRIGGER_MASK)
    {
        // Update skills here for players
        if (GetTypeId() == TYPEID_PLAYER)
        {
            // On melee based hit/miss/resist need update skill (for victim and attacker)
            if (procEx & (PROC_EX_NORMAL_HIT | PROC_EX_CRITICAL_HIT | PROC_EX_MISS | PROC_EX_DODGE | PROC_EX_PARRY | PROC_EX_BLOCK))
            {
                if (target->GetTypeId() != TYPEID_PLAYER && target->GetCreatureType() != CREATURE_TYPE_CRITTER)
                    ((Player*)this)->UpdateCombatSkills(target, attType, isVictim);
            }
            // Update defence if player is victim and parry/dodge/block
            if (isVictim && procEx & (PROC_EX_DODGE | PROC_EX_PARRY | PROC_EX_BLOCK))
                ((Player*)this)->UpdateDefense();
        }
        // If exist crit/parry/dodge/block need update aura state (for victim and attacker)
        if (procEx & (PROC_EX_CRITICAL_HIT | PROC_EX_PARRY | PROC_EX_DODGE | PROC_EX_BLOCK))
        {
            // for victim
            if (isVictim)
            {
                // if victim and dodge attack
                if (procEx & PROC_EX_DODGE)
                {
                    // Update AURA_STATE on dodge
                    if (getClass() != CLASS_ROGUE) // skip Rogue Riposte
                    {
                        ModifyAuraState(AURA_STATE_DEFENSE, true);
                        StartReactiveTimer(REACTIVE_DEFENSE);
                    }
                }
                // if victim and parry attack
                if (procEx & PROC_EX_PARRY)
                {
                    // For Hunters only Counterattack (skip Mongoose bite)
                    if (getClass() == CLASS_HUNTER)
                    {
                        ModifyAuraState(AURA_STATE_HUNTER_PARRY, true);
                        StartReactiveTimer(REACTIVE_HUNTER_PARRY);
                    }
                    else
                    {
                        ModifyAuraState(AURA_STATE_DEFENSE, true);
                        StartReactiveTimer(REACTIVE_DEFENSE);
                    }
                }
                // if and victim block attack
                if (procEx & PROC_EX_BLOCK)
                {
                    ModifyAuraState(AURA_STATE_DEFENSE, true);
                    StartReactiveTimer(REACTIVE_DEFENSE);
                }
            }
            else // For attacker
            {
                // Overpower on victim dodge
                if (procEx & PROC_EX_DODGE && getClass() == CLASS_WARRIOR)
                {
                    AddComboPoints(target, 1);
                    StartReactiveTimer(REACTIVE_OVERPOWER);
                }
            }
        }
    }
}

void Unit::ProcDamageAndSpell(ProcSystemArguments&& data)
{
    // First lets get skills and reactives out of the way
    bool currentLevel = true;
    if (data.attacker)
    {
        if (data.attacker->m_spellProcsHappening)
            currentLevel = false; // triggered spell in proc system should not make holders ready
        data.attacker->m_spellProcsHappening = true;
        if (data.procFlagsAttacker)
            data.attacker->ProcSkillsAndReactives(false, data.victim, data.procFlagsAttacker, data.procExtra, data.attType);
    }
    bool canProcVictim = data.victim && data.victim->IsAlive() && data.procFlagsVictim;
    if (canProcVictim)
        data.victim->ProcSkillsAndReactives(true, data.attacker, data.procFlagsVictim, data.procExtra, data.attType);

    // Not much to do if no flags are set.
    if (data.attacker && data.procFlagsAttacker)
        data.attacker->ProcDamageAndSpellFor(data, false);

    // Now go on with a victim's events'n'auras
    // Not much to do if no flags are set or there is no victim
    if (canProcVictim)
        data.victim->ProcDamageAndSpellFor(data, true);

    if (data.attacker)
    {
		// trigger weapon enchants for weapon based spells; exclude spells that stop attack, because may break CC
		if (data.attacker->GetTypeId() == TYPEID_PLAYER && (data.procExtra & (PROC_EX_NORMAL_HIT | PROC_EX_CRITICAL_HIT)) != 0)
            if ((data.procFlagsAttacker & PROC_FLAG_DEAL_HARMFUL_PERIODIC) == 0) // do not proc this on DOTs
			    if (!data.spellInfo || (data.spellInfo->EquippedItemClass == ITEM_CLASS_WEAPON && !data.spellInfo->HasAttribute(SPELL_ATTR_CANCELS_AUTO_ATTACK_COMBAT)))
				    static_cast<Player*>(data.attacker)->CastItemCombatSpell(data.victim, data.attType, data.spellInfo ? !IsNextMeleeSwingSpell(data.spellInfo) : false);

        if (currentLevel)
        {
            data.attacker->m_spellProcsHappening = false;

            // Mark auras created during proccing as ready
            for (SpellAuraHolder* holder : data.attacker->m_delayedSpellAuraHolders)
                if (holder->GetState() == SPELLAURAHOLDER_STATE_CREATED) // if deleted by some unknown circumstance
                    holder->SetState(SPELLAURAHOLDER_STATE_READY);

            data.attacker->m_delayedSpellAuraHolders.clear();
        }
    }
}

ProcExecutionData::ProcExecutionData(ProcSystemArguments& data, bool isVictim) : attacker(data.attacker), victim(data.victim),
    isVictim(isVictim), procExtra(data.procExtra), attType(data.attType), damage(data.damage), absorb(data.absorb), spellInfo(data.spellInfo), spell(data.spell), healthGain(data.healthGain), isHeal(data.isHeal),
    triggeredByAura(nullptr), cooldown(0), triggeredSpellId(0), procOnce(false), triggerTarget(nullptr)
{
    if (isVictim)
    {
        source = data.victim;
        target = data.attacker;
        procFlags = data.procFlagsVictim;
    }
    else
    {
        source = data.attacker;
        target = data.victim;
        procFlags = data.procFlagsAttacker;
    }
}

void Unit::ProcDamageAndSpellFor(ProcSystemArguments& argData, bool isVictim)
{
    ProcExecutionData execData(argData, isVictim);

    ProcTriggeredList procTriggered;
    std::vector<SpellAuraHolder*> holdersForDeletion;
    // Fill procTriggered list
    for (SpellAuraHolderMap::const_iterator itr = GetSpellAuraHolderMap().begin(); itr != GetSpellAuraHolderMap().end(); ++itr)
    {
        SpellAuraHolder* holder = itr->second;
        // skip deleted auras (possible at recursive triggered call
        if (holder->GetState() != SPELLAURAHOLDER_STATE_READY || holder->IsDeleted())
            continue;

        ProcTriggeredData procTriggeredData(nullptr, itr->second);

        SpellProcEventTriggerCheck result = IsTriggeredAtSpellProcEvent(execData, holder, procTriggeredData.spellProcEvent, procTriggeredData.canProc);
        if (holder->GetSpellProto()->HasAttribute(SPELL_ATTR_PROC_FAILURE_BURNS_CHARGE) &&
            result == SpellProcEventTriggerCheck::SPELL_PROC_TRIGGER_ROLL_FAILED && holder->GetAuraCharges() > 0)
            holdersForDeletion.push_back(holder);

        if (holder->GetSpellProto()->HasAttribute(SPELL_ATTR_EX2_PROC_COOLDOWN_ON_FAILURE) && result == SpellProcEventTriggerCheck::SPELL_PROC_TRIGGER_ROLL_FAILED)
        {
            uint32 cooldown = 0;
            if (procTriggeredData.spellProcEvent && procTriggeredData.spellProcEvent->cooldown)
                cooldown = procTriggeredData.spellProcEvent->cooldown;
            if (cooldown)
                holder->SetProcCooldown(std::chrono::seconds(cooldown), GetMap()->GetCurrentClockTime());
        }

        if (result != SpellProcEventTriggerCheck::SPELL_PROC_TRIGGER_OK)
            continue;

        procTriggered.emplace_back(procTriggeredData);
    }

    for (SpellAuraHolder* holder : holdersForDeletion)
        if (holder->DropAuraCharge())
            RemoveSpellAuraHolder(holder);

    // Nothing found
    if (procTriggered.empty())
        return;

    // Handle effects proceed this time
    for (ProcTriggeredList::const_iterator itr = procTriggered.begin(); itr != procTriggered.end(); ++itr)
    {
        // Some auras can be deleted in function called in this loop (except first, ofc)
        SpellAuraHolder* triggeredByHolder = itr->triggeredByHolder;
        if (triggeredByHolder->IsDeleted())
            continue;

        SpellProcEventEntry const* spellProcEvent = itr->spellProcEvent;
        bool useCharges = triggeredByHolder->GetAuraCharges() > 0;
        bool procSuccess = true;
        bool anyAuraProc = false;

        execData.cooldown = 0;
        if (spellProcEvent && spellProcEvent->cooldown)
            execData.cooldown = spellProcEvent->cooldown;

        for (int32 i = 0; i < MAX_EFFECT_INDEX; ++i)
        {
            execData.triggeredByAura = triggeredByHolder->GetAuraByEffectIndex(SpellEffectIndex(i));
            if (!execData.triggeredByAura)
                continue;

            Modifier* auraModifier = execData.triggeredByAura->GetModifier();
            if (itr->canProc[i] == false)
                continue;

            execData.triggeredSpellId = 0;
            execData.basepoints = { 0, 0, 0 };
            SpellAuraProcResult procResult = execData.triggeredByAura->OnProc(execData);
            if (procResult == SPELL_AURA_PROC_OK)
                procResult = (*this.*AuraProcHandler[auraModifier->m_auraname])(execData);
            switch (procResult)
            {
                case SPELL_AURA_PROC_CANT_TRIGGER:
                    continue;
                case SPELL_AURA_PROC_FAILED:
                    procSuccess = false;
                    break;
                case SPELL_AURA_PROC_OK:
                    if (execData.procOnce && execData.spell)
                        execData.spell->RegisterAuraProc(execData.triggeredByAura);
                    break;
            }

            anyAuraProc = true;
        }

        if ((procSuccess || triggeredByHolder->GetSpellProto()->HasAttribute(SPELL_ATTR_EX2_PROC_COOLDOWN_ON_FAILURE)) && anyAuraProc && execData.cooldown)
            triggeredByHolder->SetProcCooldown(std::chrono::seconds(execData.cooldown), GetMap()->GetCurrentClockTime());

        // Remove charge (aura can be removed by triggers)
        // Attribute example - drain soul vanilla - third effect fails when not have talent but charge should drop
        if (useCharges && (procSuccess || triggeredByHolder->GetSpellProto()->HasAttribute(SPELL_ATTR_PROC_FAILURE_BURNS_CHARGE)) && anyAuraProc && !triggeredByHolder->IsDeleted())
        {
            // If last charge dropped add spell to remove list
            if (triggeredByHolder->DropAuraCharge())
                RemoveSpellAuraHolder(triggeredByHolder);
        }
    }
}

Unit::SpellProcEventTriggerCheck Unit::IsTriggeredAtSpellProcEvent(ProcExecutionData& data, SpellAuraHolder* holder, SpellProcEventEntry const*& spellProcEvent, bool (&canProc)[MAX_EFFECT_INDEX])
{
    SpellEntry const* spellProto = holder->GetSpellProto();

    // Get proc Event Entry
    spellProcEvent = sSpellMgr.GetSpellProcEvent(spellProto->Id);

    // Get EventProcFlag
    uint32 EventProcFlag;
    if (spellProcEvent && spellProcEvent->procFlags) // if exist get custom spellProcEvent->procFlags
        EventProcFlag = spellProcEvent->procFlags;
    else
        EventProcFlag = spellProto->procFlags;       // else get from spell proto
    // Continue if no trigger exist
    if (!EventProcFlag)
        return SpellProcEventTriggerCheck::SPELL_PROC_TRIGGER_FAILED;

    // Check spellProcEvent data requirements
    if (!SpellMgr::IsSpellProcEventCanTriggeredBy(spellProcEvent, EventProcFlag, data.spellInfo, data.procFlags, data.procExtra))
        return SpellProcEventTriggerCheck::SPELL_PROC_TRIGGER_FAILED;

    // In most cases req get honor or XP from kill
    if (EventProcFlag & PROC_FLAG_KILL && GetTypeId() == TYPEID_PLAYER)
    {
        bool allow = ((Player*)this)->isHonorOrXPTarget(data.target);
        if (!allow)
            return SpellProcEventTriggerCheck::SPELL_PROC_TRIGGER_FAILED;
    }
    // Aura added by spell can`t trigger from self (prevent drop charges/do triggers)
    // But except periodic triggers (can triggered from self)
    if (data.spellInfo && data.spellInfo->Id == spellProto->Id && !(EventProcFlag & PROC_FLAG_TAKE_HARMFUL_PERIODIC))
        return SpellProcEventTriggerCheck::SPELL_PROC_TRIGGER_FAILED;

    // Check if current equipment allows aura to proc
    if (!data.isVictim && GetTypeId() == TYPEID_PLAYER && !spellProto->HasAttribute(SPELL_ATTR_EX3_NO_PROC_EQUIP_REQUIREMENT))
    {
        if (spellProto->EquippedItemClass == ITEM_CLASS_WEAPON)
        {
            Item* item = nullptr;
            switch (data.attType)
            {
                default:
                case BASE_ATTACK:
                    item = ((Player*)this)->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
                    break;
                case OFF_ATTACK:
                    item = ((Player*)this)->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
                    break;
                case RANGED_ATTACK:
                    item = ((Player*)this)->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_RANGED);
                    break;
            }

            if (!CanUseEquippedWeapon(data.attType))
                return SpellProcEventTriggerCheck::SPELL_PROC_TRIGGER_FAILED;

            if (!item || item->IsBroken() || item->GetProto()->Class != ITEM_CLASS_WEAPON || !((1 << item->GetProto()->SubClass) & spellProto->EquippedItemSubClassMask))
                return SpellProcEventTriggerCheck::SPELL_PROC_TRIGGER_FAILED;
        }
        else if (spellProto->EquippedItemClass == ITEM_CLASS_ARMOR)
        {
            // Check if player is wearing shield
            Item* item = ((Player*)this)->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
            if (!item || item->IsBroken() || item->GetProto()->Class != ITEM_CLASS_ARMOR || !((1 << item->GetProto()->SubClass) & spellProto->EquippedItemSubClassMask))
                return SpellProcEventTriggerCheck::SPELL_PROC_TRIGGER_FAILED;
        }
    }

    if (spellProto->HasAttribute(SPELL_ATTR_EX3_ONLY_PROC_OUTDOORS) && !GetTerrain()->IsOutdoors(GetPositionX(), GetPositionY(), GetPositionZ()))
        return SpellProcEventTriggerCheck::SPELL_PROC_TRIGGER_FAILED;

    if (spellProto->HasAttribute(SPELL_ATTR_EX3_ONLY_PROC_ON_CASTER) && holder->GetTarget()->GetObjectGuid() != holder->GetCasterGuid())
        return SpellProcEventTriggerCheck::SPELL_PROC_TRIGGER_FAILED;

    if (IsSitState() && !spellProto->HasAttribute(SPELL_ATTR_ALLOW_WHILE_SITTING))
        return SpellProcEventTriggerCheck::SPELL_PROC_TRIGGER_FAILED;

    // Get chance from spell
    float chance = (float)spellProto->procChance;
    // If in spellProcEvent exist custom chance, chance = spellProcEvent->customChance;
    if (spellProcEvent && spellProcEvent->customChance)
        chance = spellProcEvent->customChance;
    // If PPM exist calculate chance from PPM
    if (!data.isVictim && spellProcEvent && spellProcEvent->ppmRate != 0)
    {
        uint32 WeaponSpeed = GetAttackTime(data.attType);
        chance = GetPPMProcChance(WeaponSpeed, spellProcEvent->ppmRate);
    }
    // Apply chance modifier aura
    if (Player* modOwner = GetSpellModOwner())
        modOwner->ApplySpellMod(spellProto->Id, SPELLMOD_CHANCE_OF_SUCCESS, chance);

    if (data.spell)
    {
        if (data.spell->m_IsTriggeredSpell)
        {
            if (!data.spell->m_spellInfo->HasAttribute(SPELL_ATTR_EX3_NOT_A_PROC) && !spellProto->HasAttribute(SPELL_ATTR_EX3_CAN_PROC_FROM_PROCS))
                return SpellProcEventTriggerCheck::SPELL_PROC_TRIGGER_FAILED;
        }

        for (uint8 i = 0; i < MAX_EFFECT_INDEX; ++i)
            if (Aura* aura = holder->m_auras[i])
                if (data.spell->IsAuraProcced(aura))
                    canProc[i] = false;
    }

    for (uint8 i = 0; i < MAX_EFFECT_INDEX; ++i)
        if (Aura* aura = holder->m_auras[i])
            if (!aura->OnCheckProc(data))
                canProc[i] = false;

    if (data.spellInfo)
    {
        bool useCharges = holder->GetAuraCharges() > 0;
        for (uint8 i = 0; i < MAX_EFFECT_INDEX; ++i)
        {
            if (Aura* aura = holder->m_auras[i])
            {
                if (spellProcEvent)
                {
                    if (spellProcEvent->spellFamilyMask[i])
                    {
                        if (!data.spellInfo->IsFitToFamilyMask(spellProcEvent->spellFamilyMask[i]))
                        {
                            canProc[i] = false;
                            continue;
                        }

                        // don't allow proc from cast end for non modifier spells
                        // unless they have proc ex defined for that
                        if (IsCastEndProcModifierAura(holder->GetSpellProto(), SpellEffectIndex(i), data.spellInfo))
                        {
                            if (useCharges && data.procExtra != PROC_EX_CAST_END && spellProcEvent->procEx == PROC_EX_NONE)
                            {
                                canProc[i] = false;
                                continue;
                            }
                        }
                        else if (spellProcEvent->procEx == PROC_EX_NONE && data.procExtra == PROC_EX_CAST_END)
                        {
                            canProc[i] = false;
                            continue;
                        }
                    }
                    // don't check dbc FamilyFlags if schoolMask exists
                    else if (!aura->CanProcFrom(data.spellInfo, spellProcEvent->procEx, data.procExtra, data.damage != 0, data.absorb != 0, !spellProcEvent->schoolMask))
                    {
                        canProc[i] = false;
                        continue;
                    }
                }
                else if (!aura->CanProcFrom(data.spellInfo, PROC_EX_NONE, data.procExtra, data.damage != 0, data.absorb != 0, true))
                {
                    canProc[i] = false;
                    continue;
                }
            }
            else
                canProc[i] = false;
        }
    }

    if (!canProc[EFFECT_INDEX_0] && !canProc[EFFECT_INDEX_1] && !canProc[EFFECT_INDEX_2])
        return SpellProcEventTriggerCheck::SPELL_PROC_TRIGGER_FAILED;

    if (roll_chance_f(chance))
        return SpellProcEventTriggerCheck::SPELL_PROC_TRIGGER_OK;
    else
        return SpellProcEventTriggerCheck::SPELL_PROC_TRIGGER_ROLL_FAILED;
}

SpellAuraProcResult Unit::TriggerProccedSpell(Unit* target, std::array<int32, MAX_EFFECT_INDEX>& basepoints, uint32 triggeredSpellId, Item* castItem, Aura* triggeredByAura, uint32 cooldown, ObjectGuid originalCaster)
{
    SpellEntry const* triggerEntry = sSpellTemplate.LookupEntry<SpellEntry>(triggeredSpellId);

    if (!triggerEntry)
    {
        sLog.outError("Unit::TriggerProccedSpell: Script has nonexistent triggered spell %u", triggeredSpellId);
        return SPELL_AURA_PROC_FAILED;
    }

    return TriggerProccedSpell(target, basepoints, triggerEntry, castItem, triggeredByAura, cooldown, originalCaster);
}

SpellAuraProcResult Unit::TriggerProccedSpell(Unit* target, std::array<int32, MAX_EFFECT_INDEX>& basepoints, SpellEntry const* spellInfo, Item* castItem, Aura* triggeredByAura, uint32 cooldown, ObjectGuid originalCaster)
{
    // default case
    if (!target && IsSpellRequireTarget(spellInfo))
        return SPELL_AURA_PROC_FAILED;

    if (target && (target != this && !target->IsAlive()))
        return SPELL_AURA_PROC_FAILED;

    if (!triggeredByAura->GetHolder()->IsProcReady(GetMap()->GetCurrentClockTime()))
        return SPELL_AURA_PROC_FAILED;

    if (basepoints[EFFECT_INDEX_0] || basepoints[EFFECT_INDEX_1] || basepoints[EFFECT_INDEX_2])
        CastCustomSpell(target, spellInfo,
            basepoints[EFFECT_INDEX_0] ? &basepoints[EFFECT_INDEX_0] : nullptr,
            basepoints[EFFECT_INDEX_1] ? &basepoints[EFFECT_INDEX_1] : nullptr,
            basepoints[EFFECT_INDEX_2] ? &basepoints[EFFECT_INDEX_2] : nullptr,
            TRIGGERED_OLD_TRIGGERED | TRIGGERED_INSTANT_CAST | TRIGGERED_DO_NOT_RESET_LEASH, castItem, triggeredByAura, originalCaster);
    else
        CastSpell(target, spellInfo, TRIGGERED_OLD_TRIGGERED | TRIGGERED_INSTANT_CAST | TRIGGERED_DO_NOT_RESET_LEASH, castItem, triggeredByAura, originalCaster);

    if (cooldown)
        triggeredByAura->GetHolder()->SetProcCooldown(std::chrono::seconds(cooldown), GetMap()->GetCurrentClockTime());

    return SPELL_AURA_PROC_OK;
}

SpellAuraProcResult Unit::HandleHasteAuraProc(ProcExecutionData& data)
{
    Unit* pVictim = data.target; uint32 damage = data.damage; Aura* triggeredByAura = data.triggeredByAura; uint32 cooldown = data.cooldown;
    SpellEntry const* hasteSpell = triggeredByAura->GetSpellProto();

    Item* castItem = triggeredByAura->GetCastItemGuid() && GetTypeId() == TYPEID_PLAYER
                     ? ((Player*)this)->GetItemByGuid(triggeredByAura->GetCastItemGuid()) : nullptr;

    uint32 triggered_spell_id = data.triggeredSpellId;
    Unit* target = triggered_spell_id ? data.triggerTarget : pVictim;
    std::array<int32, MAX_EFFECT_INDEX>& basepoints = data.basepoints;

    switch (hasteSpell->Id)
    {
        // Blade Flurry
        case 13877:
        {
            target = SelectRandomUnfriendlyTarget(pVictim);
            if (!target)
                return SPELL_AURA_PROC_FAILED;
            basepoints[EFFECT_INDEX_0] = damage;
            triggered_spell_id = 22482;
            break;
        }
        // Flurry - Warrior/Shaman
        case 12966:
        case 12967:
        case 12968:
        case 12969:
        case 12970:
        case 16257:
        case 16277:
        case 16278:
        case 16279:
        case 16280:
            if (pVictim != GetTarget() || m_extraAttacksExecuting) // can only proc on main target
                return SPELL_AURA_PROC_FAILED;
            break;
    }

    // processed charge only counting case
    if (!triggered_spell_id)
        return SPELL_AURA_PROC_OK;

    return TriggerProccedSpell(target, basepoints, triggered_spell_id, castItem, triggeredByAura, cooldown, data.triggerOriginalCaster);
}

SpellAuraProcResult Unit::HandleDummyAuraProc(ProcExecutionData& data)
{
    Unit* pVictim = data.target; uint32 damage = data.damage; Aura* triggeredByAura = data.triggeredByAura; SpellEntry const* spellInfo = data.spellInfo; uint32 procFlags = data.procFlags; uint32 procEx = data.procExtra; uint32 cooldown = data.cooldown;
    SpellEntry const* dummySpell = triggeredByAura->GetSpellProto();
    SpellEffectIndex effIndex = triggeredByAura->GetEffIndex();
    int32  triggerAmount = triggeredByAura->GetModifier()->m_amount;

    Item* castItem = triggeredByAura->GetCastItemGuid() && GetTypeId() == TYPEID_PLAYER
                     ? ((Player*)this)->GetItemByGuid(triggeredByAura->GetCastItemGuid()) : nullptr;

    uint32 triggered_spell_id = data.triggeredSpellId;
    Unit* target = triggered_spell_id ? data.triggerTarget : pVictim;
    std::array<int32, MAX_EFFECT_INDEX>& basepoints = data.basepoints;

    switch (dummySpell->SpellFamilyName)
    {
        case SPELLFAMILY_GENERIC:
        {
            switch (dummySpell->Id)
            {
                // [-ZERO] TODO:  check all spell id ( most of them are different in 1.12 )

                // Eye for an Eye
                case 9799:
                case 25988:
                {
                    // prevent damage back from weapon special attacks
                    if (!spellInfo || spellInfo->DmgClass != SPELL_DAMAGE_CLASS_MAGIC)
                        return SPELL_AURA_PROC_FAILED;

                    // return absorb included damage % to attacker but < 50% own total health
                    basepoints[0] = triggerAmount * int32(data.damage + data.absorb) / 100;
                    if (basepoints[0] > (int32)GetMaxHealth() / 2)
                        basepoints[0] = (int32)GetMaxHealth() / 2;

                    triggered_spell_id = 25997;
                    break;
                }
                // Sweeping Strikes
                case 12292:
                case 18765:
                {
                    // prevent chain of triggered spell from same triggered spell
                    if (spellInfo && (spellInfo->Id == 26654 || spellInfo->Id == 12723))
                        return SPELL_AURA_PROC_FAILED;

                    target = SelectRandomUnfriendlyTarget(pVictim);
                    if (!target)
                        return SPELL_AURA_PROC_FAILED;

                    if (spellInfo)
                    {
                        if (spellInfo->Id == 1680) // whirlwind procs normalized damage
                            triggered_spell_id = 26654;
                        else if (spellInfo->TargetAuraState == AURA_STATE_HEALTHLESS_20_PERCENT)
                        {
                            if (target->HasAuraState(AURA_STATE_HEALTHLESS_20_PERCENT))
                                triggered_spell_id = 12723;
                            else // execute procs normalized damage if target is above 20 percent
                                triggered_spell_id = 26654;
                        }
                        else
                            triggered_spell_id = 12723;
                    }
                    else
                        triggered_spell_id = 12723;

                    if (triggered_spell_id == 12723)
                        basepoints[0] = damage;
                    break;
                }
                // Retaliation
                case 20230:
                {
                    // check attack comes not from behind
                    if (!HasInArc(pVictim))
                        return SPELL_AURA_PROC_FAILED;

                    triggered_spell_id = 22858;
                    break;
                }
                // Unstable Power
                case 24658:
                {
                    if (!spellInfo || spellInfo->Id == 24659)
                        return SPELL_AURA_PROC_FAILED;
                    // Need remove one 24659 aura
                    RemoveAuraHolderFromStack(24659);
                    return SPELL_AURA_PROC_OK;
                }
                // Restless Strength
                case 24661:
                {
                    // Need remove one 24662 aura
                    RemoveAuraHolderFromStack(24662);
                    return SPELL_AURA_PROC_OK;
                }
                // Adaptive Warding (Frostfire Regalia set)
                case 28764:
                {
                    if (!spellInfo)
                        return SPELL_AURA_PROC_FAILED;

                    // find Mage Armor
                    bool found = false;
                    AuraList const& mRegenInterrupt = GetAurasByType(SPELL_AURA_MOD_MANA_REGEN_INTERRUPT);
                    for (auto iter : mRegenInterrupt)
                    {
                        if (SpellEntry const* iterSpellProto = iter->GetSpellProto())
                        {
                            if (iterSpellProto->SpellFamilyName == SPELLFAMILY_MAGE && (iterSpellProto->SpellFamilyFlags & uint64(0x10000000)))
                            {
                                found = true;
                                break;
                            }
                        }
                    }
                    if (!found)
                        return SPELL_AURA_PROC_FAILED;

                    switch (GetFirstSchoolInMask(GetSpellSchoolMask(spellInfo)))
                    {
                        case SPELL_SCHOOL_NORMAL:
                        case SPELL_SCHOOL_HOLY:
                            return SPELL_AURA_PROC_FAILED;  // ignored
                        case SPELL_SCHOOL_FIRE:   triggered_spell_id = 28765; break;
                        case SPELL_SCHOOL_NATURE: triggered_spell_id = 28768; break;
                        case SPELL_SCHOOL_FROST:  triggered_spell_id = 28766; break;
                        case SPELL_SCHOOL_SHADOW: triggered_spell_id = 28769; break;
                        case SPELL_SCHOOL_ARCANE: triggered_spell_id = 28770; break;
                        default:
                            return SPELL_AURA_PROC_FAILED;
                    }

                    target = this;
                    break;
                }
                // Obsidian Armor (Justice Bearer`s Pauldrons shoulder)
                case 27539:
                {
                    if (!spellInfo)
                        return SPELL_AURA_PROC_FAILED;

                    switch (GetFirstSchoolInMask(GetSpellSchoolMask(spellInfo)))
                    {
                        case SPELL_SCHOOL_NORMAL:
                            return SPELL_AURA_PROC_FAILED;  // ignore
                        case SPELL_SCHOOL_HOLY:   triggered_spell_id = 27536; break;
                        case SPELL_SCHOOL_FIRE:   triggered_spell_id = 27533; break;
                        case SPELL_SCHOOL_NATURE: triggered_spell_id = 27538; break;
                        case SPELL_SCHOOL_FROST:  triggered_spell_id = 27534; break;
                        case SPELL_SCHOOL_SHADOW: triggered_spell_id = 27535; break;
                        case SPELL_SCHOOL_ARCANE: triggered_spell_id = 27540; break;
                        default:
                            return SPELL_AURA_PROC_FAILED;
                    }

                    target = this;
                    break;
                }
            }
            break;
        }
        case SPELLFAMILY_MAGE:
        {
            // Magic Absorption
            if (dummySpell->SpellIconID == 459)             // only this spell have SpellIconID == 459 and dummy aura
            {
                if (GetPowerType() != POWER_MANA)
                    return SPELL_AURA_PROC_FAILED;

                // mana reward
                basepoints[0] = (triggerAmount * GetMaxPower(POWER_MANA) / 100);
                target = this;
                triggered_spell_id = 29442;
                break;
            }
            // Master of Elements
            if (dummySpell->SpellIconID == 1920)
            {
                if (!spellInfo)
                    return SPELL_AURA_PROC_FAILED;

                // mana cost save
                int32 cost = spellInfo->manaCost + spellInfo->ManaCostPercentage * GetCreateMana() / 100;
                basepoints[0] = cost * triggerAmount / 100;
                if (basepoints[0] <= 0)
                    return SPELL_AURA_PROC_FAILED;

                target = this;
                triggered_spell_id = 29077;
                data.procOnce = true;
                break;
            }

            switch (dummySpell->Id)
            {
                // Ignite
                case 11119:
                case 11120:
                case 12846:
                case 12847:
                case 12848:
                {
                    switch (dummySpell->Id)
                    {
                        case 11119: basepoints[0] = int32(0.04f * damage); break;
                        case 11120: basepoints[0] = int32(0.08f * damage); break;
                        case 12846: basepoints[0] = int32(0.12f * damage); break;
                        case 12847: basepoints[0] = int32(0.16f * damage); break;
                        case 12848: basepoints[0] = int32(0.20f * damage); break;
                        default:
                            sLog.outError("Unit::HandleDummyAuraProc: non handled spell id: %u (IG)", dummySpell->Id);
                            return SPELL_AURA_PROC_FAILED;
                    }

                    triggered_spell_id = 12654;
                    break;
                }
                // Combustion
                case 11129:
                {
                    // last charge and crit
                    if (triggeredByAura->GetHolder()->GetAuraCharges() <= 1 && (procEx & PROC_EX_CRITICAL_HIT))
                    {
                        RemoveAurasDueToSpell(28682);       //-> remove Combustion auras
                        return SPELL_AURA_PROC_OK;          // charge counting (will removed)
                    }

                    CastSpell(this, 28682, TRIGGERED_OLD_TRIGGERED, castItem, triggeredByAura);
                    return (procEx & PROC_EX_CRITICAL_HIT) ? SPELL_AURA_PROC_OK : SPELL_AURA_PROC_FAILED; // charge update only at crit hits, no hidden cooldowns
                }
            }
            break;
        }
        case SPELLFAMILY_WARLOCK:
        {
            break;
        }
        case SPELLFAMILY_PRIEST:
        {
            switch (dummySpell->Id)
            {
                // Vampiric Embrace
                case 15286:
                {
                    if (!pVictim || !pVictim->IsAlive())
                        return SPELL_AURA_PROC_FAILED;

                    // pVictim is caster of aura
                    if (triggeredByAura->GetCasterGuid() != pVictim->GetObjectGuid())
                        return SPELL_AURA_PROC_FAILED;

                    // heal amount
                    basepoints[0] = triggerAmount * damage / 100;
                    pVictim->CastCustomSpell(nullptr, 15290, &basepoints[0], nullptr, nullptr, TRIGGERED_IGNORE_GCD | TRIGGERED_IGNORE_CURRENT_CASTED_SPELL | TRIGGERED_HIDE_CAST_IN_COMBAT_LOG);
                    return SPELL_AURA_PROC_OK;              // no hidden cooldown
                }
                // Priest Tier 6 Trinket (Ashtongue Talisman of Acumen)
                case 40438:
                {
                    // Shadow Word: Pain
                    if (spellInfo->SpellFamilyFlags & uint64(0x0000000000008000))
                        triggered_spell_id = 40441;
                    // Renew
                    else if (spellInfo->SpellFamilyFlags & uint64(0x0000000000000040))
                        triggered_spell_id = 40440;
                    else
                        return SPELL_AURA_PROC_FAILED;

                    target = this;
                    break;
                }
                // Oracle Healing Bonus ("Garments of the Oracle" set)
                case 26169:
                {
                    // heal amount
                    basepoints[0] = int32(damage * 10 / 100);
                    target = this;
                    triggered_spell_id = 26170;
                    break;
                }
                // Greater Heal (Vestments of Faith (Priest Tier 3) - 4 pieces bonus)
                case 28809:
                {
                    triggered_spell_id = 28810;
                    break;
                }
            }
            break;
        }
        case SPELLFAMILY_DRUID:
        {
            switch (dummySpell->Id)
            {
                // Healing Touch (Dreamwalker Raiment set)
                case 28719:
                {
                    // mana back
                    basepoints[0] = int32(spellInfo->manaCost * 30 / 100);
                    target = this;
                    triggered_spell_id = 28742;
                    break;
                }
                // Healing Touch Refund (Idol of Longevity trinket)
                case 28847:
                {
                    target = this;
                    triggered_spell_id = 28848;
                    break;
                }
            }
            break;
        }
        case SPELLFAMILY_ROGUE:
        {
            break;
        }
        case SPELLFAMILY_HUNTER:
            break;
        case SPELLFAMILY_PALADIN:
        {
            // Seal of Righteousness - melee proc dummy
            if ((dummySpell->SpellFamilyFlags & uint64(0x000000008000000)) && triggeredByAura->GetEffIndex() == EFFECT_INDEX_0)
            {
                if (GetTypeId() != TYPEID_PLAYER)
                    return SPELL_AURA_PROC_FAILED;

                uint32 spellId;
                switch (triggeredByAura->GetId())
                {
                    case 20154: spellId = 25742; break;     // Rank 1
                    case 21084: spellId = 25741; break;     // Rank 1.5
                    case 20287: spellId = 25740; break;     // Rank 2
                    case 20288: spellId = 25739; break;     // Rank 3
                    case 20289: spellId = 25738; break;     // Rank 4
                    case 20290: spellId = 25737; break;     // Rank 5
                    case 20291: spellId = 25736; break;     // Rank 6
                    case 20292: spellId = 25735; break;     // Rank 7
                    case 20293: spellId = 25713; break;     // Rank 8
                    default:
                        sLog.outError("Unit::HandleDummyAuraProc: non handled possibly SoR (Id = %u)", triggeredByAura->GetId());
                        return SPELL_AURA_PROC_FAILED;
                }
                Item* item = ((Player*)this)->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
                float speed = (item ? item->GetProto()->Delay : BASE_ATTACK_TIME) / 1000.0f;

                float damageBasePoints;
                float coeff;
                if (item && item->GetProto()->InventoryType == INVTYPE_2HWEAPON)
                {
                    // two hand weapon
                    damageBasePoints = 1.20f * triggerAmount * 1.2f * 1.03f * speed / 100.0f + 1;
                    coeff = .108f * speed;
                }
                else
                {
                    // one hand weapon/no weapon
                    damageBasePoints = 0.85f * ceil(triggerAmount * 1.2f * 1.03f * speed / 100.0f) - 1;
                    coeff = .092f * speed;
                }

                int32 damagePoint = int32(damageBasePoints + 0.03f * (GetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE) + GetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE)) / 2.0f) + 1;

                // apply damage bonuses manually
                if (damagePoint >= 0)
                {
                    // currently uses same spell damage fetch as flametongue - need to verify whether SP is supposed to be applied pre-triggered spell bonuses or post
                    int32 bonusDamage = SpellBaseDamageBonusDone(GetSpellSchoolMask(dummySpell)) + pVictim->SpellBaseDamageBonusTaken(GetSpellSchoolMask(dummySpell));
                    if (Aura* aura = GetAura(43743, EFFECT_INDEX_0)) // Improved Seal of Righteousness
                        bonusDamage += aura->GetAmount();
                    damagePoint += bonusDamage * coeff * CalculateLevelPenalty(dummySpell);
                }

                CastCustomSpell(pVictim, spellId, &damagePoint, nullptr, nullptr, TRIGGERED_OLD_TRIGGERED, nullptr, triggeredByAura);
                return SPELL_AURA_PROC_OK;                  // no hidden cooldown
            }

            switch (dummySpell->Id)
            {
                // Holy Power (Redemption Armor set)
                case 28789:
                {
                    if (!pVictim)
                        return SPELL_AURA_PROC_FAILED;

                    // Set class defined buff
                    switch (pVictim->getClass())
                    {
                        case CLASS_PALADIN:
                        case CLASS_PRIEST:
                        case CLASS_SHAMAN:
                        case CLASS_DRUID:
                            triggered_spell_id = 28795;     // Increases the friendly target's mana regeneration by $s1 per 5 sec. for $d.
                            break;
                        case CLASS_MAGE:
                        case CLASS_WARLOCK:
                            triggered_spell_id = 28793;     // Increases the friendly target's spell damage and healing by up to $s1 for $d.
                            break;
                        case CLASS_HUNTER:
                        case CLASS_ROGUE:
                            triggered_spell_id = 28791;     // Increases the friendly target's attack power by $s1 for $d.
                            break;
                        case CLASS_WARRIOR:
                            triggered_spell_id = 28790;     // Increases the friendly target's armor
                            break;
                        default:
                            return SPELL_AURA_PROC_FAILED;
                    }
                    break;
                }
            }
            break;
        }
        case SPELLFAMILY_SHAMAN:
        {
            switch (dummySpell->Id)
            {
                // Totemic Power (The Earthshatterer set)
                case 28823:
                {
                    if (!pVictim)
                        return SPELL_AURA_PROC_FAILED;

                    // Set class defined buff
                    switch (pVictim->getClass())
                    {
                        case CLASS_PALADIN:
                        case CLASS_PRIEST:
                        case CLASS_SHAMAN:
                        case CLASS_DRUID:
                            triggered_spell_id = 28824;     // Increases the friendly target's mana regeneration by $s1 per 5 sec. for $d.
                            break;
                        case CLASS_MAGE:
                        case CLASS_WARLOCK:
                            triggered_spell_id = 28825;     // Increases the friendly target's spell damage and healing by up to $s1 for $d.
                            break;
                        case CLASS_HUNTER:
                        case CLASS_ROGUE:
                            triggered_spell_id = 28826;     // Increases the friendly target's attack power by $s1 for $d.
                            break;
                        case CLASS_WARRIOR:
                            triggered_spell_id = 28827;     // Increases the friendly target's armor
                            break;
                        default:
                            return SPELL_AURA_PROC_FAILED;
                    }
                    break;
                }
                // Lesser Healing Wave (Totem of Flowing Water Relic)
                case 28849:
                {
                    target = this;
                    triggered_spell_id = 28850;
                    break;
                }
            }
            break;
        }
        default:
            break;
    }

    // processed charge only counting case
    if (!triggered_spell_id)
        return SPELL_AURA_PROC_OK;

    return TriggerProccedSpell(target, basepoints, triggered_spell_id, castItem, triggeredByAura, cooldown, data.triggerOriginalCaster);
}

SpellAuraProcResult Unit::HandleProcTriggerSpellAuraProc(ProcExecutionData& data)
{
    Unit* pVictim = data.target; uint32 damage = data.damage; Aura* triggeredByAura = data.triggeredByAura; SpellEntry const* spellInfo = data.spellInfo; uint32 procFlags = data.procFlags; uint32 procEx = data.procExtra; uint32 cooldown = data.cooldown;
    // Get triggered aura spell info
    SpellEntry const* auraSpellInfo = triggeredByAura->GetSpellProto();

    // Basepoints of trigger aura
    int32 triggerAmount = triggeredByAura->GetModifier()->m_amount;

    // Set trigger spell id, target, custom basepoints
    uint32 trigger_spell_id = data.triggeredSpellId;
    Unit* target = trigger_spell_id ? data.triggerTarget : nullptr;
    if (!trigger_spell_id)
        trigger_spell_id = auraSpellInfo->EffectTriggerSpell[triggeredByAura->GetEffIndex()];
    std::array<int32, MAX_EFFECT_INDEX>& basepoints = data.basepoints;

    Item* castItem = triggeredByAura->GetCastItemGuid() && GetTypeId() == TYPEID_PLAYER
                     ? ((Player*)this)->GetItemByGuid(triggeredByAura->GetCastItemGuid()) : nullptr;

    // Try handle unknown trigger spells
    // Custom requirements (not listed in procEx) Warning! damage dealing after this
    // Custom triggered spells
    switch (auraSpellInfo->SpellFamilyName)
    {
        case SPELLFAMILY_GENERIC:
            switch (auraSpellInfo->Id)
            {
                // case 5301:  break;                   // Defensive State (DND)
                // case 7137:  break;                   // Shadow Charge (Rank 1)
                // case 7377:  break;                   // Take Immune Periodic Damage <Not Working>
                // case 13358: break;                   // Defensive State (DND)
                // case 16092: break;                   // Defensive State (DND)
                // case 18943: break;                   // Double Attack
                // case 19194: break;                   // Double Attack
                // case 19817: break;                   // Double Attack
                // case 19818: break;                   // Double Attack
                // case 22835: break;                   // Drunken Rage
                //    trigger_spell_id = 14822; break;
                case 23780:                                 // Aegis of Preservation (Aegis of Preservation trinket)
                    trigger_spell_id = 23781;
                    break;
                // case 24949: break;                   // Defensive State 2 (DND)
                case 27522:                                 // Mana Drain Trigger
                {
                    // On successful melee or ranged attack gain $29471s1 mana and if possible drain $27526s1 mana from the target.
                    if (IsAlive())
                        CastSpell(this, 29471, TRIGGERED_OLD_TRIGGERED, castItem, triggeredByAura);
                    if (pVictim && pVictim->IsAlive())
                        CastSpell(pVictim, 27526, TRIGGERED_OLD_TRIGGERED, castItem, triggeredByAura);
                    return SPELL_AURA_PROC_OK;
                }
                case 31255:                                 // Deadly Swiftness (Rank 1)
                {
                    // whenever you deal damage to a target who is below 20% health.
                    if (pVictim->GetHealth() > pVictim->GetMaxHealth() / 5)
                        return SPELL_AURA_PROC_FAILED;

                    target = this;
                    trigger_spell_id = 22588;
                    break;
                }
                break;
            }
            break;
        case SPELLFAMILY_MAGE:
            if (auraSpellInfo->Id == 26467)                 // Persistent Shield (Scarab Brooch trinket)
            {
                // This spell originally trigger 13567 - Dummy Trigger (vs dummy effect)
                basepoints[0] = damage * 15 / 100;
                target = pVictim;
                trigger_spell_id = 26470;
            }
            break;
        case SPELLFAMILY_WARRIOR:
            // Deep Wounds (replace triggered spells to directly apply DoT), dot spell have familyflags
            if (!auraSpellInfo->SpellFamilyFlags && auraSpellInfo->SpellIconID == 243)
            {
                float weaponDamage;
                // DW should benefit of attack power, damage percent mods etc.
                // TODO: check if using offhand damage is correct and if it should be divided by 2
                if (hasOffhandWeaponForAttack() && getAttackTimer(BASE_ATTACK) > getAttackTimer(OFF_ATTACK))
                    weaponDamage = (GetFloatValue(UNIT_FIELD_MINOFFHANDDAMAGE) + GetFloatValue(UNIT_FIELD_MAXOFFHANDDAMAGE)) / 2;
                else
                    weaponDamage = (GetFloatValue(UNIT_FIELD_MINDAMAGE) + GetFloatValue(UNIT_FIELD_MAXDAMAGE)) / 2;

                switch (auraSpellInfo->Id)
                {
                    case 12834: basepoints[0] = int32(weaponDamage * 0.2f); break;
                    case 12849: basepoints[0] = int32(weaponDamage * 0.4f); break;
                    case 12867: basepoints[0] = int32(weaponDamage * 0.6f); break;
                    // Impossible case
                    default:
                        sLog.outError("Unit::HandleProcTriggerSpellAuraProc: DW unknown spell rank %u", auraSpellInfo->Id);
                        return SPELL_AURA_PROC_FAILED;
                }

                // 1 tick/sec * 6 sec = 6 ticks
                basepoints[0] /= 6;

                trigger_spell_id = 12721;
                break;
            }
            break;
        case SPELLFAMILY_WARLOCK:
        {
            // Pyroclasm
            if (auraSpellInfo->SpellIconID == 1137)
            {
                if (!pVictim || !pVictim->IsAlive() || pVictim == this || spellInfo == nullptr)
                    return SPELL_AURA_PROC_FAILED;
                // Calculate spell tick count for spells
                uint32 tick = 1; // Default tick = 1

                // Hellfire have 15 tick
                if (spellInfo->SpellFamilyFlags & uint64(0x0000000000000040))
                    tick = 15;
                // Rain of Fire have 4 tick
                else if (spellInfo->SpellFamilyFlags & uint64(0x0000000000000020))
                    tick = 4;
                else
                    return SPELL_AURA_PROC_FAILED;

                // Calculate chance = baseChance / tick
                float chance = 0;
                switch (auraSpellInfo->Id)
                {
                    case 18096: chance = 13.0f / tick; break;
                    case 18073: chance = 26.0f / tick; break;
                }
                // Roll chance
                if (!roll_chance_f(chance))
                    return SPELL_AURA_PROC_FAILED;

                trigger_spell_id = 18093;
            }
            // Cheat Death
            else if (auraSpellInfo->Id == 28845)
            {
                // When your health drops below 20% ....
                int32 health20 = int32(GetMaxHealth()) / 5;
                if (int32(GetHealth()) - int32(damage) >= health20 || int32(GetHealth()) < health20)
                    return SPELL_AURA_PROC_FAILED;
            }
            break;
        }
        case SPELLFAMILY_PRIEST:
        {
            // Blessed Recovery
            if (auraSpellInfo->SpellIconID == 1875)
            {
                switch (auraSpellInfo->Id)
                {
                    case 27811: trigger_spell_id = 27813; break;
                    case 27815: trigger_spell_id = 27817; break;
                    case 27816: trigger_spell_id = 27818; break;
                    default:
                        sLog.outError("Unit::HandleProcTriggerSpellAuraProc: Spell %u not handled in BR", auraSpellInfo->Id);
                        return SPELL_AURA_PROC_FAILED;
                }
                basepoints[0] = damage * triggerAmount / 100 / 3;
                target = this;
            }
            break;
        }
        case SPELLFAMILY_DRUID:
            break;
        case SPELLFAMILY_HUNTER:
            break;
        case SPELLFAMILY_PALADIN:
        {
            // Illumination
            if (auraSpellInfo->SpellIconID == 241)
            {
                if (!spellInfo)
                    return SPELL_AURA_PROC_FAILED;
                // spellInfo is triggered spell but we need mana cost of original casted spell
                uint32 originalSpellId = spellInfo->Id;
                // Holy Shock
                uint32 manaCost = data.spell ? data.spell->m_currentBasePoints[1] : 0;
                if (spellInfo->SpellFamilyFlags & uint64(0x0000000000200000))
                {
                    switch (spellInfo->Id)
                    {
                        case 25914: originalSpellId = 20473; break;
                        case 25913: originalSpellId = 20929; break;
                        case 25903: originalSpellId = 20930; break;
                        default:
                            sLog.outError("Unit::HandleProcTriggerSpellAuraProc: Spell %u not handled in HShock", spellInfo->Id);
                            return SPELL_AURA_PROC_FAILED;
                    }
                }
                else
                    originalSpellId = data.spell ? data.spell->m_currentBasePoints[1] : 0;
                SpellEntry const* originalSpell = sSpellTemplate.LookupEntry<SpellEntry>(originalSpellId);
                if (!originalSpell)
                {
                    sLog.outError("Unit::HandleProcTriggerSpellAuraProc: Spell %u unknown but selected as original in Illu", originalSpellId);
                    return SPELL_AURA_PROC_FAILED;
                }
                basepoints[0] = originalSpell->manaCost;
                trigger_spell_id = 20272;
                target = this;
            }
            break;
        }
        case SPELLFAMILY_SHAMAN:
        {
            // Lightning Shield (overwrite non existing triggered spell call in spell.dbc
            if (auraSpellInfo->IsFitToFamilyMask(uint64(0x0000000000000400)) && auraSpellInfo->SpellVisual == 37)
            {
                switch (auraSpellInfo->Id)
                {
                    case 324:                           // Rank 1
                        trigger_spell_id = 26364; break;
                    case 325:                           // Rank 2
                        trigger_spell_id = 26365; break;
                    case 905:                           // Rank 3
                        trigger_spell_id = 26366; break;
                    case 945:                           // Rank 4
                        trigger_spell_id = 26367; break;
                    case 8134:                          // Rank 5
                        trigger_spell_id = 26369; break;
                    case 10431:                         // Rank 6
                        trigger_spell_id = 26370; break;
                    case 10432:                         // Rank 7
                        trigger_spell_id = 26363; break;
                    default:
                        sLog.outError("Unit::HandleProcTriggerSpellAuraProc: Spell %u not handled in LShield", auraSpellInfo->Id);
                        return SPELL_AURA_PROC_FAILED;
                }
            }
            // Lightning Shield (The Ten Storms set)
            else if (auraSpellInfo->Id == 23551)
            {
                trigger_spell_id = 23552;
                target = pVictim;
            }
            // Damage from Lightning Shield (The Ten Storms set)
            else if (auraSpellInfo->Id == 23552)
                trigger_spell_id = 27635;
            // Mana Surge (The Earthfury set)
            else if (auraSpellInfo->Id == 23572)
            {
                if (!spellInfo)
                    return SPELL_AURA_PROC_FAILED;
                basepoints[0] = spellInfo->manaCost * 35 / 100;
                trigger_spell_id = 23571;
                target = this;
            }
            break;
        }
        default:
            break;
    }

    // Custom basepoints/target for exist spell
    // dummy basepoints or other customs
    switch (trigger_spell_id)
    {
        // Cast positive spell on enemy target
        case 7099:  // Curse of Mending
        case 29494: // Temptation
        case 20233: // Improved Lay on Hands (cast on target)
        {
            target = pVictim;
            break;
        }
        // Finishing moves that add combo points
        case 14189: // Seal Fate (Netherblade set)
        case 14157: // Ruthlessness
        {
            // Need add combopoint AFTER finishing move (or they get dropped in finish phase)
            if (Spell* spell = GetCurrentSpell(CURRENT_GENERIC_SPELL))
            {
                spell->AddTriggeredSpell(trigger_spell_id);
                return SPELL_AURA_PROC_OK;
            }
            return SPELL_AURA_PROC_FAILED;
        }
    }

    // All ok. Check current trigger spell
    SpellEntry const* triggerEntry = sSpellTemplate.LookupEntry<SpellEntry>(trigger_spell_id);
    if (!triggerEntry)
    {
        // Not cast unknown spell
        // sLog.outError("Unit::HandleProcTriggerSpellAuraProc: Spell %u have 0 in EffectTriggered[%d], not handled custom case?",auraSpellInfo->Id,triggeredByAura->GetEffIndex());
        return SPELL_AURA_PROC_FAILED;
    }

    // try detect target manually if not set
    if (target == nullptr)
        target = !(procFlags & (PROC_FLAG_DEAL_HELPFUL_SPELL | PROC_FLAG_DEAL_HELPFUL_ABILITY)) && IsPositiveSpellTargetMode(triggerEntry, this, pVictim) ? this : pVictim;

    // Quick check for target modes for procs: do not cast offensive procs on friendly targets and in reverse
    if (target != nullptr)
    {
        if (!(procEx & PROC_EX_REFLECT))
        {
            // TODO: add neutral target handling, neutral targets should still be able to go through
            if (!(this == target && IsOnlySelfTargeting(triggerEntry)))
            {
                if (IsPositiveSpellTargetMode(triggerEntry, this, target) != CanAssistSpell(target, triggerEntry))
                    return SPELL_AURA_PROC_FAILED;
            }
        }
    }

    // not allow proc extra attack spell at extra attack
    if (m_extraAttacksExecuting && IsSpellHaveEffect(triggerEntry, SPELL_EFFECT_ADD_EXTRA_ATTACKS))
        return SPELL_AURA_PROC_FAILED;

    return TriggerProccedSpell(target, basepoints, triggerEntry, castItem, triggeredByAura, cooldown, data.triggerOriginalCaster);
}

SpellAuraProcResult Unit::HandleProcTriggerDamageAuraProc(ProcExecutionData& data)
{
    Unit* victim = data.target; Aura* triggeredByAura = data.triggeredByAura; uint32 cooldown = data.cooldown;
    SpellEntry const* spellInfo = triggeredByAura->GetSpellProto();
    DEBUG_FILTER_LOG(LOG_FILTER_SPELL_CAST, "ProcDamageAndSpell: doing %u damage from spell id %u (triggered by auratype %u of spell %u)",
                     triggeredByAura->GetModifier()->m_amount, spellInfo->Id, triggeredByAura->GetModifier()->m_auraname, triggeredByAura->GetId());

    if (!triggeredByAura->GetHolder()->IsProcReady(GetMap()->GetCurrentClockTime()))
        return SPELL_AURA_PROC_FAILED;

    // Trigger damage can be resisted...
    if (SpellMissInfo missInfo = SpellHitResult(this, victim, spellInfo, uint8(1 << triggeredByAura->GetEffIndex()), false))
    {
        SendSpellDamageResist(victim, spellInfo->Id);
        return SPELL_AURA_PROC_OK;
    }

    if (cooldown)
        triggeredByAura->GetHolder()->SetProcCooldown(std::chrono::seconds(cooldown), GetMap()->GetCurrentClockTime());

    SpellNonMeleeDamage spellDamageInfo(this, victim, spellInfo->Id, SpellSchools(spellInfo->School));
    CalculateSpellDamage(&spellDamageInfo, triggeredByAura->GetModifier()->m_amount, spellInfo, triggeredByAura->GetEffIndex());
    spellDamageInfo.target->CalculateAbsorbResistBlock(this, &spellDamageInfo, spellInfo);
    Unit::DealDamageMods(this, spellDamageInfo.target, spellDamageInfo.damage, &spellDamageInfo.absorb, SPELL_DIRECT_DAMAGE);
    SendSpellNonMeleeDamageLog(&spellDamageInfo);
    DealSpellDamage(this, &spellDamageInfo, true, false);
    return SPELL_AURA_PROC_OK;
}

SpellAuraProcResult Unit::HandleOverrideClassScriptAuraProc(ProcExecutionData& data)
{
    Unit* pVictim = data.target; Aura* triggeredByAura = data.triggeredByAura; SpellEntry const* spellInfo = data.spellInfo; uint32 cooldown = data.cooldown;
    int32 scriptId = triggeredByAura->GetModifier()->m_miscvalue;

    if (!pVictim || !pVictim->IsAlive())
        return SPELL_AURA_PROC_FAILED;

    Item* castItem = triggeredByAura->GetCastItemGuid() && GetTypeId() == TYPEID_PLAYER
                     ? ((Player*)this)->GetItemByGuid(triggeredByAura->GetCastItemGuid()) : nullptr;

    // Basepoints of trigger aura
    int32 triggerAmount = triggeredByAura->GetModifier()->m_amount;

    uint32 triggered_spell_id = data.triggeredSpellId;
    Unit* target = triggered_spell_id ? data.triggerTarget : pVictim;
    std::array<int32, MAX_EFFECT_INDEX>& basepoints = data.basepoints;

    switch (scriptId)
    {
        case 3656:                                          // Corrupted Healing (Priest class call in Nefarian encounter)
        {
            // Procced spell can only be triggered by direct heals
            // Heal over time like Renew does not trigger it
            // Check that only priest class can proc it is done in Spell::CheckTargetScript() for aura 23401
            if (IsSpellHaveEffect(spellInfo, SPELL_EFFECT_HEAL))
                triggered_spell_id = 23402;
            break;
        }
        case 4086:                                          // Improved Mend Pet (Rank 1)
        case 4087:                                          // Improved Mend Pet (Rank 2)
        {
            if (!roll_chance_i(triggerAmount))
                return SPELL_AURA_PROC_FAILED;

            triggered_spell_id = 24406;
            break;
        }
        case 4309:
        {
            triggered_spell_id = 17941;                     // Shadow Trance (Warlock Nightfall)
            break;
        }
        case 4533:                                          // Dreamwalker Raiment 2 pieces bonus
        {
            // Chance 50%
            if (!roll_chance_i(50))
                return SPELL_AURA_PROC_FAILED;

            switch (pVictim->GetPowerType())
            {
                case POWER_MANA:   triggered_spell_id = 28722; break;
                case POWER_RAGE:   triggered_spell_id = 28723; break;
                case POWER_ENERGY: triggered_spell_id = 28724; break;
                default:
                    return SPELL_AURA_PROC_FAILED;
            }
            break;
        }
        case 4537:                                          // Dreamwalker Raiment 6 pieces bonus - Regrowth
            triggered_spell_id = 28750;                     // Blessing of the Claw
            break;
    }

    // not processed
    if (!triggered_spell_id)
        return SPELL_AURA_PROC_OK;

    return TriggerProccedSpell(target, basepoints, triggered_spell_id, castItem, triggeredByAura, cooldown, data.triggerOriginalCaster);
}

SpellAuraProcResult Unit::HandleModCastingSpeedNotStackAuraProc(ProcExecutionData& data)
{
    SpellEntry const* spellInfo = data.spellInfo;
    // Skip melee hits or instant cast spells
    return !(spellInfo == nullptr || GetSpellCastTime(spellInfo, this) == 0) ? SPELL_AURA_PROC_OK : SPELL_AURA_PROC_FAILED;
}

SpellAuraProcResult Unit::HandleReflectSpellsSchoolAuraProc(ProcExecutionData& data)
{
    Aura* triggeredByAura = data.triggeredByAura; SpellEntry const* spellInfo = data.spellInfo;
    // Skip Melee hits and spells ws wrong school
    return !(spellInfo == nullptr || (triggeredByAura->GetModifier()->m_miscvalue & GetSchoolMask(spellInfo->School)) == 0) ? SPELL_AURA_PROC_OK : SPELL_AURA_PROC_FAILED;
}

SpellAuraProcResult Unit::HandleModPowerCostSchoolAuraProc(ProcExecutionData& data)
{
    Aura* triggeredByAura = data.triggeredByAura; SpellEntry const* spellInfo = data.spellInfo;
    // Skip melee hits and spells ws wrong school or zero cost
    return !(spellInfo == nullptr ||
             (spellInfo->manaCost == 0 && spellInfo->ManaCostPercentage == 0) ||           // Cost check
             (triggeredByAura->GetModifier()->m_miscvalue & GetSchoolMask(spellInfo->School)) == 0) ? SPELL_AURA_PROC_OK : SPELL_AURA_PROC_FAILED;  // School check
}

SpellAuraProcResult Unit::HandleMechanicImmuneResistanceAuraProc(ProcExecutionData& data)
{
    Aura* triggeredByAura = data.triggeredByAura; SpellEntry const* spellInfo = data.spellInfo;
    // Compare mechanic
    return !(spellInfo == nullptr || spellInfo->Mechanic != triggeredByAura->GetModifier()->m_miscvalue)
           ? SPELL_AURA_PROC_OK : SPELL_AURA_PROC_FAILED;
}

SpellAuraProcResult Unit::HandleMagnetAuraProc(ProcExecutionData& data)
{
    Aura* triggeredByAura = data.triggeredByAura; uint32 procEx = data.procExtra;
    return procEx & PROC_EX_MAGNET && triggeredByAura->IsMagnetUsed() ? SPELL_AURA_PROC_OK : SPELL_AURA_PROC_FAILED;
}

SpellAuraProcResult Unit::HandleModResistanceAuraProc(ProcExecutionData& data)
{
    uint32 damage = data.damage; Aura* triggeredByAura = data.triggeredByAura;
    SpellEntry const* spellInfo = triggeredByAura->GetSpellProto();

    // Inner Fire
    if (spellInfo->IsFitToFamily(SPELLFAMILY_PRIEST, uint64(0x0000000000002)))
    {
        // only at real damage
        if (!damage)
            return SPELL_AURA_PROC_FAILED;
    }

    return SPELL_AURA_PROC_OK;
}

SpellAuraProcResult Unit::HandleRemoveByDamageChanceProc(ProcExecutionData& data)
{
    uint32 damage = data.damage; Aura* triggeredByAura = data.triggeredByAura;
    // The chance to dispel an aura depends on the damage taken with respect to the casters level.
    uint32 max_dmg = GetLevel() > 8 ? 25 * GetLevel() - 150 : 50;
    float chance = float(damage) / max_dmg * 100.0f;
    if (roll_chance_f(chance))
    {
        RemoveAurasByCasterSpell(triggeredByAura->GetId(), triggeredByAura->GetCasterGuid());
        return SPELL_AURA_PROC_OK;
    }

    return SPELL_AURA_PROC_FAILED;
}

SpellAuraProcResult Unit::HandleInvisibilityAuraProc(ProcExecutionData& data)
{
    Aura* triggeredByAura = data.triggeredByAura;
    if (triggeredByAura->GetSpellProto()->HasAttribute(SPELL_ATTR_PASSIVE) || triggeredByAura->GetSpellProto()->HasAttribute(SPELL_ATTR_AURA_IS_DEBUFF))
        return SPELL_AURA_PROC_FAILED;

    RemoveAurasDueToSpell(triggeredByAura->GetId());
    return SPELL_AURA_PROC_OK;
}

SpellAuraProcResult Unit::HandleSpellModProc(ProcExecutionData& /*data*/)
{
    // one day if figured out - remove charges through proc system but currently not reliable enough for edge cases
    return SPELL_AURA_PROC_CANT_TRIGGER;
}
