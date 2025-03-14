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
#include "Database/DatabaseEnv.h"
#include "Server/WorldPacket.h"
#include "Server/WorldSession.h"
#include "Server/Opcodes.h"
#include "Log/Log.h"
#include "World/World.h"
#include "Globals/ObjectMgr.h"
#include "Spells/SpellMgr.h"
#include "Entities/Player.h"
#include "Entities/Unit.h"
#include "Spells/Spell.h"
#include "Entities/DynamicObject.h"
#include "Groups/Group.h"
#include "Entities/UpdateData.h"
#include "Entities/UpdateMask.h"
#include "Globals/ObjectAccessor.h"
#include "Policies/Singleton.h"
#include "Entities/Totem.h"
#include "Entities/Creature.h"
#include "BattleGround/BattleGround.h"
#include "OutdoorPvP/OutdoorPvP.h"
#include "AI/BaseAI/CreatureAI.h"
#include "AI/ScriptDevAI/ScriptDevAIMgr.h"
#include "Util/Util.h"
#include "Grids/GridNotifiers.h"
#include "Grids/GridNotifiersImpl.h"
#include "Grids/CellImpl.h"
#include "Tools/Language.h"
#include "Maps/MapManager.h"
#include "Loot/LootMgr.h"
#include "AI/ScriptDevAI/include/sc_grid_searchers.h"

#define NULL_AURA_SLOT 0xFF

/**
 * An array with all the different handlers for taking care of
 * the various aura types that are defined in AuraType.
 */
pAuraHandler AuraHandler[TOTAL_AURAS] =
{
    &Aura::HandleNULL,                                      //  0 SPELL_AURA_NONE
    &Aura::HandleBindSight,                                 //  1 SPELL_AURA_BIND_SIGHT
    &Aura::HandleModPossess,                                //  2 SPELL_AURA_MOD_POSSESS
    &Aura::HandlePeriodicDamage,                            //  3 SPELL_AURA_PERIODIC_DAMAGE
    &Aura::HandleAuraDummy,                                 //  4 SPELL_AURA_DUMMY
    &Aura::HandleModConfuse,                                //  5 SPELL_AURA_MOD_CONFUSE
    &Aura::HandleModCharm,                                  //  6 SPELL_AURA_MOD_CHARM
    &Aura::HandleModFear,                                   //  7 SPELL_AURA_MOD_FEAR
    &Aura::HandlePeriodicHeal,                              //  8 SPELL_AURA_PERIODIC_HEAL
    &Aura::HandleModAttackSpeed,                            //  9 SPELL_AURA_MOD_ATTACKSPEED
    &Aura::HandleModThreat,                                 // 10 SPELL_AURA_MOD_THREAT
    &Aura::HandleModTaunt,                                  // 11 SPELL_AURA_MOD_TAUNT
    &Aura::HandleAuraModStun,                               // 12 SPELL_AURA_MOD_STUN
    &Aura::HandleModDamageDone,                             // 13 SPELL_AURA_MOD_DAMAGE_DONE
    &Aura::HandleNoImmediateEffect,                         // 14 SPELL_AURA_MOD_DAMAGE_TAKEN   implemented in Unit::MeleeDamageBonusTaken and Unit::SpellBaseDamageBonusTaken
    &Aura::HandleNoImmediateEffect,                         // 15 SPELL_AURA_DAMAGE_SHIELD      implemented in Unit::DealMeleeDamage
    &Aura::HandleModStealth,                                // 16 SPELL_AURA_MOD_STEALTH
    &Aura::HandleModStealthDetect,                          // 17 SPELL_AURA_MOD_STEALTH_DETECT
    &Aura::HandleInvisibility,                              // 18 SPELL_AURA_MOD_INVISIBILITY
    &Aura::HandleInvisibilityDetect,                        // 19 SPELL_AURA_MOD_INVISIBILITY_DETECTION
    &Aura::HandleAuraModTotalHealthPercentRegen,            // 20 SPELL_AURA_OBS_MOD_HEALTH
    &Aura::HandleAuraModTotalManaPercentRegen,              // 21 SPELL_AURA_OBS_MOD_MANA
    &Aura::HandleAuraModResistance,                         // 22 SPELL_AURA_MOD_RESISTANCE
    &Aura::HandlePeriodicTriggerSpell,                      // 23 SPELL_AURA_PERIODIC_TRIGGER_SPELL
    &Aura::HandlePeriodicEnergize,                          // 24 SPELL_AURA_PERIODIC_ENERGIZE
    &Aura::HandleAuraModPacify,                             // 25 SPELL_AURA_MOD_PACIFY
    &Aura::HandleAuraModRoot,                               // 26 SPELL_AURA_MOD_ROOT
    &Aura::HandleAuraModSilence,                            // 27 SPELL_AURA_MOD_SILENCE
    &Aura::HandleNoImmediateEffect,                         // 28 SPELL_AURA_REFLECT_SPELLS        implement in Unit::SpellHitResult
    &Aura::HandleAuraModStat,                               // 29 SPELL_AURA_MOD_STAT
    &Aura::HandleAuraModSkill,                              // 30 SPELL_AURA_MOD_SKILL
    &Aura::HandleAuraModIncreaseSpeed,                      // 31 SPELL_AURA_MOD_INCREASE_SPEED
    &Aura::HandleAuraModIncreaseMountedSpeed,               // 32 SPELL_AURA_MOD_INCREASE_MOUNTED_SPEED
    &Aura::HandleAuraModDecreaseSpeed,                      // 33 SPELL_AURA_MOD_DECREASE_SPEED
    &Aura::HandleAuraModIncreaseHealth,                     // 34 SPELL_AURA_MOD_INCREASE_HEALTH
    &Aura::HandleAuraModIncreaseEnergy,                     // 35 SPELL_AURA_MOD_INCREASE_ENERGY
    &Aura::HandleAuraModShapeshift,                         // 36 SPELL_AURA_MOD_SHAPESHIFT
    &Aura::HandleAuraModEffectImmunity,                     // 37 SPELL_AURA_EFFECT_IMMUNITY
    &Aura::HandleAuraModStateImmunity,                      // 38 SPELL_AURA_STATE_IMMUNITY
    &Aura::HandleAuraModSchoolImmunity,                     // 39 SPELL_AURA_SCHOOL_IMMUNITY
    &Aura::HandleAuraModDmgImmunity,                        // 40 SPELL_AURA_DAMAGE_IMMUNITY
    &Aura::HandleAuraModDispelImmunity,                     // 41 SPELL_AURA_DISPEL_IMMUNITY
    &Aura::HandleAuraProcTriggerSpell,                      // 42 SPELL_AURA_PROC_TRIGGER_SPELL  implemented in Unit::ProcDamageAndSpellFor and Unit::HandleProcTriggerSpell
    &Aura::HandleNoImmediateEffect,                         // 43 SPELL_AURA_PROC_TRIGGER_DAMAGE implemented in Unit::ProcDamageAndSpellFor
    &Aura::HandleAuraTrackCreatures,                        // 44 SPELL_AURA_TRACK_CREATURES
    &Aura::HandleAuraTrackResources,                        // 45 SPELL_AURA_TRACK_RESOURCES
    &Aura::HandleUnused,                                    // 46 SPELL_AURA_46
    &Aura::HandleAuraModParryPercent,                       // 47 SPELL_AURA_MOD_PARRY_PERCENT
    &Aura::HandleUnused,                                    // 48 SPELL_AURA_48
    &Aura::HandleAuraModDodgePercent,                       // 49 SPELL_AURA_MOD_DODGE_PERCENT
    &Aura::HandleUnused,                                    // 50 SPELL_AURA_MOD_BLOCK_SKILL    obsolete?
    &Aura::HandleAuraModBlockPercent,                       // 51 SPELL_AURA_MOD_BLOCK_PERCENT
    &Aura::HandleAuraModCritPercent,                        // 52 SPELL_AURA_MOD_CRIT_PERCENT
    &Aura::HandlePeriodicLeech,                             // 53 SPELL_AURA_PERIODIC_LEECH
    &Aura::HandleModHitChance,                              // 54 SPELL_AURA_MOD_HIT_CHANCE
    &Aura::HandleModSpellHitChance,                         // 55 SPELL_AURA_MOD_SPELL_HIT_CHANCE
    &Aura::HandleAuraTransform,                             // 56 SPELL_AURA_TRANSFORM
    &Aura::HandleModSpellCritChance,                        // 57 SPELL_AURA_MOD_SPELL_CRIT_CHANCE
    &Aura::HandleAuraModIncreaseSwimSpeed,                  // 58 SPELL_AURA_MOD_INCREASE_SWIM_SPEED
    &Aura::HandleNoImmediateEffect,                         // 59 SPELL_AURA_MOD_DAMAGE_DONE_CREATURE implemented in Unit::MeleeDamageBonusDone and Unit::SpellDamageBonusDone
    &Aura::HandleAuraModPacifyAndSilence,                   // 60 SPELL_AURA_MOD_PACIFY_SILENCE
    &Aura::HandleAuraModScale,                              // 61 SPELL_AURA_MOD_SCALE
    &Aura::HandlePeriodicHealthFunnel,                      // 62 SPELL_AURA_PERIODIC_HEALTH_FUNNEL
    &Aura::HandleUnused,                                    // 63 SPELL_AURA_PERIODIC_MANA_FUNNEL obsolete?
    &Aura::HandlePeriodicManaLeech,                         // 64 SPELL_AURA_PERIODIC_MANA_LEECH
    &Aura::HandleModCastingSpeed,                           // 65 SPELL_AURA_MOD_CASTING_SPEED_NOT_STACK
    &Aura::HandleFeignDeath,                                // 66 SPELL_AURA_FEIGN_DEATH
    &Aura::HandleAuraModDisarm,                             // 67 SPELL_AURA_MOD_DISARM
    &Aura::HandleAuraModStalked,                            // 68 SPELL_AURA_MOD_STALKED
    &Aura::HandleNoImmediateEffect,                         // 69 SPELL_AURA_SCHOOL_ABSORB implemented in Unit::CalculateAbsorbAndResist
    &Aura::HandleUnused,                                    // 70 SPELL_AURA_EXTRA_ATTACKS      Useless, used by only one spell that has only visual effect
    &Aura::HandleModSpellCritChanceShool,                   // 71 SPELL_AURA_MOD_SPELL_CRIT_CHANCE_SCHOOL
    &Aura::HandleModPowerCostPCT,                           // 72 SPELL_AURA_MOD_POWER_COST_SCHOOL_PCT
    &Aura::HandleModPowerCost,                              // 73 SPELL_AURA_MOD_POWER_COST_SCHOOL
    &Aura::HandleNoImmediateEffect,                         // 74 SPELL_AURA_REFLECT_SPELLS_SCHOOL  implemented in Unit::SpellHitResult
    &Aura::HandleNoImmediateEffect,                         // 75 SPELL_AURA_MOD_LANGUAGE           implemented in WorldSession::HandleMessagechatOpcode
    &Aura::HandleFarSight,                                  // 76 SPELL_AURA_FAR_SIGHT
    &Aura::HandleModMechanicImmunity,                       // 77 SPELL_AURA_MECHANIC_IMMUNITY
    &Aura::HandleAuraMounted,                               // 78 SPELL_AURA_MOUNTED
    &Aura::HandleModDamagePercentDone,                      // 79 SPELL_AURA_MOD_DAMAGE_PERCENT_DONE
    &Aura::HandleModPercentStat,                            // 80 SPELL_AURA_MOD_PERCENT_STAT
    &Aura::HandleNoImmediateEffect,                         // 81 SPELL_AURA_SPLIT_DAMAGE_PCT       implemented in Unit::CalculateAbsorbAndResist
    &Aura::HandleWaterBreathing,                            // 82 SPELL_AURA_WATER_BREATHING
    &Aura::HandleModBaseResistance,                         // 83 SPELL_AURA_MOD_BASE_RESISTANCE
    &Aura::HandleModRegen,                                  // 84 SPELL_AURA_MOD_REGEN
    &Aura::HandleModPowerRegen,                             // 85 SPELL_AURA_MOD_POWER_REGEN
    &Aura::HandleChannelDeathItem,                          // 86 SPELL_AURA_CHANNEL_DEATH_ITEM
    &Aura::HandleNoImmediateEffect,                         // 87 SPELL_AURA_MOD_DAMAGE_PERCENT_TAKEN implemented in Unit::MeleeDamageBonusTaken and Unit::SpellDamageBonusTaken
    &Aura::HandleNoImmediateEffect,                         // 88 SPELL_AURA_MOD_HEALTH_REGEN_PERCENT implemented in Player::RegenerateHealth
    &Aura::HandlePeriodicDamagePCT,                         // 89 SPELL_AURA_PERIODIC_DAMAGE_PERCENT
    &Aura::HandleUnused,                                    // 90 SPELL_AURA_MOD_RESIST_CHANCE  Useless
    &Aura::HandleNoImmediateEffect,                         // 91 SPELL_AURA_MOD_DETECT_RANGE implemented in Creature::GetAttackDistance
    &Aura::HandlePreventFleeing,                            // 92 SPELL_AURA_PREVENTS_FLEEING
    &Aura::HandleModUnattackable,                           // 93 SPELL_AURA_MOD_UNATTACKABLE
    &Aura::HandleInterruptRegen,                            // 94 SPELL_AURA_INTERRUPT_REGEN implemented in Player::RegenerateAll
    &Aura::HandleAuraGhost,                                 // 95 SPELL_AURA_GHOST
    &Aura::HandleNoImmediateEffect,                         // 96 SPELL_AURA_SPELL_MAGNET implemented in Unit::SelectMagnetTarget
    &Aura::HandleManaShield,                                // 97 SPELL_AURA_MANA_SHIELD implemented in Unit::CalculateAbsorbAndResist
    &Aura::HandleAuraModSkill,                              // 98 SPELL_AURA_MOD_SKILL_TALENT
    &Aura::HandleAuraModAttackPower,                        // 99 SPELL_AURA_MOD_ATTACK_POWER
    &Aura::HandleAurasVisible,                              // 100 SPELL_AURA_AURAS_VISIBLE
    &Aura::HandleModResistancePercent,                      // 101 SPELL_AURA_MOD_RESISTANCE_PCT
    &Aura::HandleNoImmediateEffect,                         // 102 SPELL_AURA_MOD_MELEE_ATTACK_POWER_VERSUS implemented in Unit::MeleeDamageBonusDone
    &Aura::HandleAuraModTotalThreat,                        // 103 SPELL_AURA_MOD_TOTAL_THREAT
    &Aura::HandleAuraWaterWalk,                             // 104 SPELL_AURA_WATER_WALK
    &Aura::HandleAuraFeatherFall,                           // 105 SPELL_AURA_FEATHER_FALL
    &Aura::HandleAuraHover,                                 // 106 SPELL_AURA_HOVER
    &Aura::HandleAddModifier,                               // 107 SPELL_AURA_ADD_FLAT_MODIFIER
    &Aura::HandleAddModifier,                               // 108 SPELL_AURA_ADD_PCT_MODIFIER
    &Aura::HandleNoImmediateEffect,                         // 109 SPELL_AURA_ADD_TARGET_TRIGGER
    &Aura::HandleModPowerRegenPCT,                          // 110 SPELL_AURA_MOD_POWER_REGEN_PERCENT
    &Aura::HandleUnused,                                    // 111 SPELL_AURA_ADD_CASTER_HIT_TRIGGER
    &Aura::HandleOverrideClassScript,                       // 112 SPELL_AURA_OVERRIDE_CLASS_SCRIPTS implemented in diff functions.
    &Aura::HandleNoImmediateEffect,                         // 113 SPELL_AURA_MOD_RANGED_DAMAGE_TAKEN implemented in Unit::MeleeDamageBonusTaken
    &Aura::HandleNoImmediateEffect,                         // 114 SPELL_AURA_MOD_RANGED_DAMAGE_TAKEN_PCT implemented in Unit::MeleeDamageBonusTaken
    &Aura::HandleNoImmediateEffect,                         // 115 SPELL_AURA_MOD_HEALING                 implemented in Unit::SpellBaseHealingBonusTaken
    &Aura::HandleNoImmediateEffect,                         // 116 SPELL_AURA_MOD_REGEN_DURING_COMBAT     imppemented in Player::RegenerateAll and Player::RegenerateHealth
    &Aura::HandleNoImmediateEffect,                         // 117 SPELL_AURA_MOD_MECHANIC_RESISTANCE     implemented in Unit::MagicSpellHitResult
    &Aura::HandleNoImmediateEffect,                         // 118 SPELL_AURA_MOD_HEALING_PCT             implemented in Unit::SpellHealingBonusTaken
    &Aura::HandleUnused,                                    // 119 SPELL_AURA_SHARE_PET_TRACKING useless
    &Aura::HandleAuraUntrackable,                           // 120 SPELL_AURA_UNTRACKABLE
    &Aura::HandleAuraEmpathy,                               // 121 SPELL_AURA_EMPATHY
    &Aura::HandleModOffhandDamagePercent,                   // 122 SPELL_AURA_MOD_OFFHAND_DAMAGE_PCT
    &Aura::HandleNoImmediateEffect,                         // 123 SPELL_AURA_MOD_TARGET_RESISTANCE  implemented in Unit::CalculateAbsorbAndResist and Unit::CalcArmorReducedDamage
    &Aura::HandleAuraModRangedAttackPower,                  // 124 SPELL_AURA_MOD_RANGED_ATTACK_POWER
    &Aura::HandleNoImmediateEffect,                         // 125 SPELL_AURA_MOD_MELEE_DAMAGE_TAKEN implemented in Unit::MeleeDamageBonusTaken
    &Aura::HandleNoImmediateEffect,                         // 126 SPELL_AURA_MOD_MELEE_DAMAGE_TAKEN_PCT implemented in Unit::MeleeDamageBonusTaken
    &Aura::HandleNoImmediateEffect,                         // 127 SPELL_AURA_RANGED_ATTACK_POWER_ATTACKER_BONUS implemented in Unit::MeleeDamageBonusDone
    &Aura::HandleModPossessPet,                             // 128 SPELL_AURA_MOD_POSSESS_PET
    &Aura::HandleAuraModIncreaseSpeed,                      // 129 SPELL_AURA_MOD_SPEED_ALWAYS
    &Aura::HandleAuraModIncreaseMountedSpeed,               // 130 SPELL_AURA_MOD_MOUNTED_SPEED_ALWAYS
    &Aura::HandleNoImmediateEffect,                         // 131 SPELL_AURA_MOD_RANGED_ATTACK_POWER_VERSUS implemented in Unit::MeleeDamageBonusDone
    &Aura::HandleAuraModIncreaseEnergyPercent,              // 132 SPELL_AURA_MOD_INCREASE_ENERGY_PERCENT
    &Aura::HandleAuraModIncreaseHealthPercent,              // 133 SPELL_AURA_MOD_INCREASE_HEALTH_PERCENT
    &Aura::HandleAuraModRegenInterrupt,                     // 134 SPELL_AURA_MOD_MANA_REGEN_INTERRUPT
    &Aura::HandleModHealingDone,                            // 135 SPELL_AURA_MOD_HEALING_DONE
    &Aura::HandleNoImmediateEffect,                         // 136 SPELL_AURA_MOD_HEALING_DONE_PERCENT   implemented in Unit::SpellHealingBonusDone
    &Aura::HandleModTotalPercentStat,                       // 137 SPELL_AURA_MOD_TOTAL_STAT_PERCENTAGE
    &Aura::HandleModMeleeSpeedPct,                          // 138 SPELL_AURA_MOD_MELEE_HASTE
    &Aura::HandleForceReaction,                             // 139 SPELL_AURA_FORCE_REACTION
    &Aura::HandleAuraModRangedHaste,                        // 140 SPELL_AURA_MOD_RANGED_HASTE
    &Aura::HandleRangedAmmoHaste,                           // 141 SPELL_AURA_MOD_RANGED_AMMO_HASTE
    &Aura::HandleAuraModBaseResistancePercent,              // 142 SPELL_AURA_MOD_BASE_RESISTANCE_PCT
    &Aura::HandleAuraModResistanceExclusive,                // 143 SPELL_AURA_MOD_RESISTANCE_EXCLUSIVE
    &Aura::HandleAuraSafeFall,                              // 144 SPELL_AURA_SAFE_FALL                  implemented in WorldSession::HandleMovementOpcodes
    &Aura::HandleUnused,                                    // 145 SPELL_AURA_CHARISMA obsolete?
    &Aura::HandleUnused,                                    // 146 SPELL_AURA_PERSUADED obsolete?
    &Aura::HandleModMechanicImmunityMask,                   // 147 SPELL_AURA_MECHANIC_IMMUNITY_MASK     implemented in Unit::IsImmuneToSpell and Unit::IsImmuneToSpellEffect (check part)
    &Aura::HandleAuraRetainComboPoints,                     // 148 SPELL_AURA_RETAIN_COMBO_POINTS
    &Aura::HandleNoImmediateEffect,                         // 149 SPELL_AURA_RESIST_PUSHBACK            implemented in Spell::Delayed and Spell::DelayedChannel
    &Aura::HandleShieldBlockValue,                          // 150 SPELL_AURA_MOD_SHIELD_BLOCKVALUE_PCT
    &Aura::HandleAuraTrackStealthed,                        // 151 SPELL_AURA_TRACK_STEALTHED
    &Aura::HandleNoImmediateEffect,                         // 152 SPELL_AURA_MOD_DETECTED_RANGE         implemented in Creature::GetAttackDistance
    &Aura::HandleNoImmediateEffect,                         // 153 SPELL_AURA_SPLIT_DAMAGE_FLAT          implemented in Unit::CalculateAbsorbAndResist
    &Aura::HandleModStealthLevel,                           // 154 SPELL_AURA_MOD_STEALTH_LEVEL          implemented in Unit::isVisibleForOrDetect
    &Aura::HandleModWaterBreathing,                         //155 SPELL_AURA_MOD_WATER_BREATHING
    &Aura::HandleNoImmediateEffect,                         // 156 SPELL_AURA_MOD_REPUTATION_GAIN        implemented in Player::CalculateReputationGain
    &Aura::HandleUnused,                                    // 157 SPELL_AURA_PET_DAMAGE_MULTI (single test like spell 20782, also single for 214 aura)
    &Aura::HandleShieldBlockValue,                          // 158 SPELL_AURA_MOD_SHIELD_BLOCKVALUE
    &Aura::HandleNoImmediateEffect,                         // 159 SPELL_AURA_NO_PVP_CREDIT              implemented in Player::RewardHonor
    &Aura::HandleNoImmediateEffect,                         // 160 SPELL_AURA_MOD_AOE_AVOIDANCE          implemented in Unit::MagicSpellHitResult
    &Aura::HandleNoImmediateEffect,                         // 161 SPELL_AURA_MOD_HEALTH_REGEN_IN_COMBAT implemented in Player::RegenerateAll and Player::RegenerateHealth
    &Aura::HandleAuraPowerBurn,                             // 162 SPELL_AURA_POWER_BURN_MANA
    &Aura::HandleUnused,                                    // 163 SPELL_AURA_MOD_CRIT_DAMAGE_BONUS
    &Aura::HandleUnused,                                    // 164 useless, only one test spell
    &Aura::HandleNoImmediateEffect,                         // 165 SPELL_AURA_MELEE_ATTACK_POWER_ATTACKER_BONUS implemented in Unit::MeleeDamageBonusDone
    &Aura::HandleAuraModAttackPowerPercent,                 // 166 SPELL_AURA_MOD_ATTACK_POWER_PCT
    &Aura::HandleAuraModRangedAttackPowerPercent,           // 167 SPELL_AURA_MOD_RANGED_ATTACK_POWER_PCT
    &Aura::HandleNoImmediateEffect,                         // 168 SPELL_AURA_MOD_DAMAGE_DONE_VERSUS            implemented in Unit::SpellDamageBonusDone, Unit::MeleeDamageBonusDone
    &Aura::HandleNoImmediateEffect,                         // 169 SPELL_AURA_MOD_CRIT_PERCENT_VERSUS           implemented in Unit::DealDamageBySchool, Unit::DoAttackDamage, Unit::SpellCriticalBonus
    &Aura::HandleDetectAmore,                               // 170 SPELL_AURA_DETECT_AMORE       only for Detect Amore spell
    &Aura::HandleAuraModIncreaseSpeed,                      // 171 SPELL_AURA_MOD_SPEED_NOT_STACK
    &Aura::HandleAuraModIncreaseMountedSpeed,               // 172 SPELL_AURA_MOD_MOUNTED_SPEED_NOT_STACK
    &Aura::HandleUnused,                                    // 173 SPELL_AURA_ALLOW_CHAMPION_SPELLS  only for Proclaim Champion spell
    &Aura::HandleModSpellDamagePercentFromStat,             // 174 SPELL_AURA_MOD_SPELL_DAMAGE_OF_STAT_PERCENT  implemented in Unit::SpellBaseDamageBonusDone (in 1.12.* only spirit)
    &Aura::HandleModSpellHealingPercentFromStat,            // 175 SPELL_AURA_MOD_SPELL_HEALING_OF_STAT_PERCENT implemented in Unit::SpellBaseHealingBonusDone (in 1.12.* only spirit)
    &Aura::HandleSpiritOfRedemption,                        // 176 SPELL_AURA_SPIRIT_OF_REDEMPTION   only for Spirit of Redemption spell, die at aura end
    &Aura::HandleAoECharm,                                  // 177 SPELL_AURA_AOE_CHARM
    &Aura::HandleNoImmediateEffect,                         // 178 SPELL_AURA_MOD_DEBUFF_RESISTANCE          implemented in Unit::MagicSpellHitResult
    &Aura::HandleNoImmediateEffect,                         // 179 SPELL_AURA_MOD_ATTACKER_SPELL_CRIT_CHANCE implemented in Unit::SpellCriticalBonus
    &Aura::HandleNoImmediateEffect,                         // 180 SPELL_AURA_MOD_FLAT_SPELL_DAMAGE_VERSUS   implemented in Unit::SpellDamageBonusDone
    &Aura::HandleUnused,                                    // 181 SPELL_AURA_MOD_FLAT_SPELL_CRIT_DAMAGE_VERSUS unused
    &Aura::HandleAuraModResistenceOfStatPercent,            // 182 SPELL_AURA_MOD_RESISTANCE_OF_STAT_PERCENT
    &Aura::HandleNoImmediateEffect,                         // 183 SPELL_AURA_MOD_CRITICAL_THREAT only used in 28746, implemented in ThreatCalcHelper::CalcThreat
    &Aura::HandleNoImmediateEffect,                         // 184 SPELL_AURA_MOD_ATTACKER_MELEE_HIT_CHANCE  implemented in Unit::CalculateEffectiveMissChance
    &Aura::HandleNoImmediateEffect,                         // 185 SPELL_AURA_MOD_ATTACKER_RANGED_HIT_CHANCE implemented in Unit::CalculateEffectiveMissChance
    &Aura::HandleNoImmediateEffect,                         // 186 SPELL_AURA_MOD_ATTACKER_SPELL_HIT_CHANCE  implemented in Unit::MagicSpellHitResult
    &Aura::HandleNoImmediateEffect,                         // 187 SPELL_AURA_MOD_ATTACKER_MELEE_CRIT_CHANCE  implemented in Unit::CalculateEffectiveCritChance
    &Aura::HandleNoImmediateEffect,                         // 188 SPELL_AURA_MOD_ATTACKER_RANGED_CRIT_CHANCE implemented in Unit::CalculateEffectiveCritChance
    &Aura::HandleUnused,                                    // 189 SPELL_AURA_MOD_RATING (not used in 1.12.1)
    &Aura::HandleNoImmediateEffect,                         // 190 SPELL_AURA_MOD_FACTION_REPUTATION_GAIN     implemented in Player::CalculateReputationGain
    &Aura::HandleAuraModUseNormalSpeed,                     // 191 SPELL_AURA_USE_NORMAL_MOVEMENT_SPEED
};

static AuraType const frozenAuraTypes[] = { SPELL_AURA_MOD_ROOT, SPELL_AURA_MOD_STUN, SPELL_AURA_NONE };

Aura::Aura(SpellEntry const* spellproto, SpellEffectIndex eff, int32 const* currentDamage, int32 const* currentBasePoints, SpellAuraHolder* holder, Unit* target, Unit* caster, Item* castItem) :
    m_spellmod(nullptr), m_periodicTimer(0), m_periodicTick(0), m_removeMode(AURA_REMOVE_BY_DEFAULT),
    m_effIndex(eff), m_positive(false), m_isPeriodic(false), m_isAreaAura(false),
    m_isPersistent(false), m_magnetUsed(false), m_spellAuraHolder(holder),
    m_scriptValue(0), m_storage(nullptr), m_affectOverriden(false), m_scriptRef(this, NoopAuraDeleter())
{
    MANGOS_ASSERT(target);
    MANGOS_ASSERT(spellproto && spellproto == sSpellTemplate.LookupEntry<SpellEntry>(spellproto->Id) && "`info` must be pointer to sSpellTemplate element");

    m_currentBasePoints = currentBasePoints ? *currentBasePoints : spellproto->CalculateSimpleValue(eff);

    m_positive = IsPositiveAuraEffect(spellproto, m_effIndex, caster, target);

    m_applyTime = time(nullptr);

    int32 damage = currentDamage ? *currentDamage : m_currentBasePoints;
    if (caster)
    {
        damage = OnAuraValueCalculate(caster, damage, castItem);
    }

    DEBUG_FILTER_LOG(LOG_FILTER_SPELL_CAST, "Aura: construct Spellid : %u, Aura : %u Target : %d Damage : %d", spellproto->Id, spellproto->EffectApplyAuraName[eff], spellproto->EffectImplicitTargetA[eff], damage);

    SetModifier(AuraType(spellproto->EffectApplyAuraName[eff]), damage, spellproto->EffectAmplitude[eff], spellproto->EffectMiscValue[eff]);

    Player* modOwner = caster ? caster->GetSpellModOwner() : nullptr;

    // Apply periodic time mod
    if (modOwner && m_modifier.periodictime)
        modOwner->ApplySpellMod(spellproto->Id, SPELLMOD_ACTIVATION_TIME, m_modifier.periodictime);

    // Start periodic on next tick
    m_periodicTimer += m_modifier.periodictime;
}

Aura::~Aura()
{
    delete m_storage;
    delete m_spellmod;
}

AreaAura::AreaAura(SpellEntry const* spellproto, SpellEffectIndex eff, int32 const* currentDamage, int32 const* currentBasePoints, SpellAuraHolder* holder, Unit* target,
                   Unit* caster, Item* castItem, uint32 originalRankSpellId)
    : Aura(spellproto, eff, currentDamage, currentBasePoints, holder, target, caster, castItem), m_originalRankSpellId(originalRankSpellId)
{
    m_isAreaAura = true;

    // caster==nullptr in constructor args if target==caster in fact
    Unit* caster_ptr = caster ? caster : target;

    m_radius = GetSpellRadius(sSpellRadiusStore.LookupEntry(spellproto->EffectRadiusIndex[m_effIndex]));
    if (Player* modOwner = caster_ptr->GetSpellModOwner())
        modOwner->ApplySpellMod(spellproto->Id, SPELLMOD_RADIUS, m_radius);

    switch (spellproto->Effect[eff])
    {
        case SPELL_EFFECT_APPLY_AREA_AURA_PARTY:
            m_areaAuraType = AREA_AURA_PARTY;
            break;
        case SPELL_EFFECT_APPLY_AREA_AURA_PET:
            m_areaAuraType = AREA_AURA_PET;
            break;
        default:
            sLog.outError("Wrong spell effect in AreaAura constructor");
            MANGOS_ASSERT(false);
            break;
    }

    // totems are immune to any kind of area auras
    if (target->GetTypeId() == TYPEID_UNIT && ((Creature*)target)->IsTotem())
        m_modifier.m_auraname = SPELL_AURA_NONE;
}

AreaAura::~AreaAura()
{
}

bool AreaAura::OnAreaAuraCheckTarget(Unit* target) const
{
    if (AuraScript* script = GetAuraScript())
        return script->OnAreaAuraCheckTarget(this, target);
    return true;
}

PersistentAreaAura::PersistentAreaAura(SpellEntry const* spellproto, SpellEffectIndex eff, int32 const* currentDamage, int32 const* currentBasePoints, SpellAuraHolder* holder, Unit* target,
                                       Unit* caster, Item* castItem) : Aura(spellproto, eff, currentDamage, currentBasePoints, holder, target, caster, castItem)
{
    m_isPersistent = true;
}

PersistentAreaAura::~PersistentAreaAura()
{
}

SingleEnemyTargetAura::SingleEnemyTargetAura(SpellEntry const* spellproto, SpellEffectIndex eff, int32 const* currentDamage, int32 const* currentBasePoints, SpellAuraHolder* holder, Unit* target,
        Unit* caster, Item* castItem) : Aura(spellproto, eff, currentDamage, currentBasePoints, holder, target, caster, castItem)
{
    if (caster)
        m_castersTargetGuid = caster->GetSelectionGuid();
}

SingleEnemyTargetAura::~SingleEnemyTargetAura()
{
}

Unit* SingleEnemyTargetAura::GetTriggerTarget() const
{
    return ObjectAccessor::GetUnit(*(m_spellAuraHolder->GetTarget()), m_castersTargetGuid);
}

Aura* CreateAura(SpellEntry const* spellproto, SpellEffectIndex eff, int32 const* currentDamage, int32 const* currentBasePoints, SpellAuraHolder* holder, Unit* target, Unit* caster, Item* castItem, uint64 scriptValue)
{
    Aura* aura = nullptr;
    if (IsAreaAuraEffect(spellproto->Effect[eff]))
        aura = new AreaAura(spellproto, eff, currentDamage, currentBasePoints, holder, target, caster, castItem);

    if (!aura)
        aura = new Aura(spellproto, eff, currentDamage, currentBasePoints, holder, target, caster, castItem);

    aura->SetScriptValue(scriptValue); // must be done before OnAuraInit
    aura->OnAuraInit();
    return aura;
}

SpellAuraHolder* CreateSpellAuraHolder(SpellEntry const* spellproto, Unit* target, WorldObject* caster, Item* castItem /*= nullptr*/, SpellEntry const* triggeredBy /*= nullptr*/)
{
    return new SpellAuraHolder(spellproto, target, caster, castItem, triggeredBy);
}

void Aura::SetModifier(AuraType type, int32 amount, uint32 periodicTime, int32 miscValue)
{
    m_modifier.m_auraname = type;
    m_modifier.m_amount = amount * GetStackAmount();
    m_modifier.m_baseAmount = amount;
    m_modifier.m_miscvalue = miscValue;
    m_modifier.periodictime = periodicTime;
}

void Aura::Update(uint32 diff)
{
    if (m_isPeriodic)
    {
        m_periodicTimer -= diff;
        if (m_periodicTimer <= 0) // tick also at m_periodicTimer==0 to prevent lost last tick in case max m_duration == (max m_periodicTimer)*N
        {
            // update before applying (aura can be removed in TriggerSpell or PeriodicTick calls)
            m_periodicTimer += m_modifier.periodictime;
            ++m_periodicTick;                               // for some infinity auras in some cases can overflow and reset
            PeriodicTick();
        }
    }
}

void AreaAura::Update(uint32 diff)
{
    // update for the caster of the aura
    if (GetCasterGuid() == GetTarget()->GetObjectGuid())
    {
        Unit* caster = GetTarget();

        if (!caster->hasUnitState(UNIT_STAT_ISOLATED))
        {
            Unit* owner = caster->GetMaster();
            if (!owner)
                owner = caster;
            UnitList targets;

            switch (m_areaAuraType)
            {
                case AREA_AURA_PARTY:
                {
                    Group* pGroup = nullptr;

                    // Handle aura party for players
                    if (owner->GetTypeId() == TYPEID_PLAYER)
                    {
                        pGroup = ((Player*)owner)->GetGroup();

                        if (pGroup)
                        {
                            uint8 subgroup = ((Player*)owner)->GetSubGroup();
                            for (GroupReference* itr = pGroup->GetFirstMember(); itr != nullptr; itr = itr->next())
                            {
                                Player* Target = itr->getSource();
                                if (Target && Target->IsAlive() && Target->GetSubGroup() == subgroup && caster->CanAssistSpell(Target, GetSpellProto()))
                                {
                                    if (caster->IsWithinDistInMap(Target, m_radius))
                                        targets.push_back(Target);
                                    Pet* pet = Target->GetPet();
                                    if (pet && pet->IsAlive() && caster->IsWithinDistInMap(pet, m_radius))
                                        targets.push_back(pet);
                                }
                            }
                            break;
                        }
                    }
                    else    // handle aura party for creatures
                    {
                        // Get all creatures in spell radius
                        std::list<Creature*> nearbyTargets;
                        MaNGOS::AnyUnitInObjectRangeCheck u_check(owner, m_radius);
                        MaNGOS::CreatureListSearcher<MaNGOS::AnyUnitInObjectRangeCheck> searcher(nearbyTargets, u_check);
                        Cell::VisitGridObjects(owner, searcher, m_radius);

                        for (auto target : nearbyTargets)
                        {
                            // Due to the lack of support for NPC groups or formations, are considered of the same party NPCs with same faction than caster
                            if (target != owner && target->IsAlive() && target->GetFaction() == ((Creature*)owner)->GetFaction())
                                targets.push_back(target);
                        }
                    }

                    // add owner
                    if (owner != caster && caster->IsWithinDistInMap(owner, m_radius))
                        targets.push_back(owner);
                    // add caster's pet
                    Unit* pet = caster->GetPet();
                    if (pet && caster->IsWithinDistInMap(pet, m_radius))
                        targets.push_back(pet);

                    break;
                }
                case AREA_AURA_PET:
                {
                    if (owner != caster && caster->IsWithinDistInMap(owner, m_radius) && caster->CanAssistSpell(owner, GetSpellProto()))
                        targets.push_back(owner);
                    break;
                }
            }

            for (auto& target : targets)
            {
                if (!OnAreaAuraCheckTarget(target))
                    continue;

                // flag for selection is need apply aura to current iteration target
                bool apply = true;

                SpellEntry const* actualSpellInfo;
                if (GetCasterGuid() == target->GetObjectGuid()) // if caster is same as target then no need to change rank of the spell
                    actualSpellInfo = GetSpellProto();
                else
                    actualSpellInfo = sSpellMgr.SelectAuraRankForLevel(GetSpellProto(), target->GetLevel()); // use spell id according level of the target
                if (!actualSpellInfo)
                    continue;

                Unit::SpellAuraHolderBounds spair = target->GetSpellAuraHolderBounds(actualSpellInfo->Id);
                // we need ignore present caster self applied are auras sometime
                // in cases if this only auras applied for spell effect
                for (Unit::SpellAuraHolderMap::const_iterator i = spair.first; i != spair.second; ++i)
                {
                    if (i->second->IsDeleted())
                        continue;

                    Aura* aur = i->second->GetAuraByEffectIndex(m_effIndex);

                    if (!aur)
                        continue;

                    switch (m_areaAuraType)
                    {
                        case AREA_AURA_PARTY:
                            // non caster self-casted auras (stacked from diff. casters)
                            if (aur->GetModifier()->m_auraname != SPELL_AURA_NONE && i->second->GetCasterGuid() != GetCasterGuid())
                            {
                                apply = IsStackableSpell(actualSpellInfo, i->second->GetSpellProto(), target);
                                break;
                            }
                            if (aur->GetModifier()->m_auraname != SPELL_AURA_NONE || i->second->GetCasterGuid() == GetCasterGuid())
                                apply = false;
                            break;
                        default:
                            // in generic case not allow stacking area auras
                            apply = false;
                            break;
                    }

                    if (!apply)
                        break;
                }

                if (!apply)
                    continue;

                // Skip some targets (TODO: Might require better checks, also unclear how the actual caster must/can be handled)
                if (actualSpellInfo->HasAttribute(SPELL_ATTR_EX3_ONLY_ON_PLAYER) && target->GetTypeId() != TYPEID_PLAYER)
                    continue;

                int32 actualBasePoints = m_currentBasePoints;
                int32 actualDamage = m_modifier.m_baseAmount;
                // recalculate basepoints for lower rank (all AreaAura spell not use custom basepoints?)
                if (actualSpellInfo != GetSpellProto())
                {
                    actualBasePoints = actualSpellInfo->CalculateSimpleValue(m_effIndex);
                    actualDamage = caster->CalculateSpellEffectValue(target, actualSpellInfo, m_effIndex, &actualBasePoints);
                }

                SpellAuraHolder* holder = target->GetSpellAuraHolder(actualSpellInfo->Id, GetCasterGuid());

                bool addedToExisting = true;
                if (!holder)
                {
                    holder = CreateSpellAuraHolder(actualSpellInfo, target, caster);
                    addedToExisting = false;
                }

                holder->SetAuraDuration(GetAuraDuration());

                AreaAura* aur = new AreaAura(actualSpellInfo, m_effIndex, &actualDamage, &actualBasePoints, holder, target, caster, nullptr, GetSpellProto()->Id);
                holder->AddAura(aur, m_effIndex);

                if (addedToExisting)
                {
                    target->AddAuraToModList(aur);
                    aur->ApplyModifier(true, true);
                }
                else
                {
                    if (target->AddSpellAuraHolder(holder))
                        holder->SetState(SPELLAURAHOLDER_STATE_READY);
                    else
                        delete holder;
                }
            }
        }
        Aura::Update(diff);
    }
    else                                                    // aura at non-caster
    {
        Unit* caster = GetCaster();
        Unit* target = GetTarget();
        uint32 originalRankSpellId = m_originalRankSpellId ? m_originalRankSpellId : GetId(); // caster may have different spell id if target has lower level

        Aura::Update(diff);

        // remove aura if out-of-range from caster (after teleport for example)
        // or caster is isolated or caster no longer has the aura
        // or caster is (no longer) friendly
        bool needFriendly = true;
        if (!caster ||
                caster->hasUnitState(UNIT_STAT_ISOLATED)               ||
                !caster->HasAura(originalRankSpellId, GetEffIndex())   ||
                !caster->IsWithinDistInMap(target, m_radius)           ||
                caster->CanAssistSpell(target, GetSpellProto()) != needFriendly
           )
        {
            target->RemoveSingleAuraFromSpellAuraHolder(GetId(), GetEffIndex(), GetCasterGuid());
        }
        else if (m_areaAuraType == AREA_AURA_PARTY)         // check if in same sub group
        {
            // Do not check group if target == owner or target == pet
            // or if caster is a not player (as NPCs do not support group so aura is only removed by moving out of range)
            if (caster->GetMasterGuid() != target->GetObjectGuid()  &&
                caster->GetObjectGuid() != target->GetMasterGuid()  &&
                caster->IsPlayerControlled())
            {
                Player* check = caster->GetBeneficiaryPlayer();

                Group* pGroup = check ? check->GetGroup() : nullptr;
                if (pGroup)
                {
                    Player* checkTarget = target->GetBeneficiaryPlayer();
                    if (!checkTarget || !pGroup->SameSubGroup(check, checkTarget))
                        target->RemoveSingleAuraFromSpellAuraHolder(GetId(), GetEffIndex(), GetCasterGuid());
                }
                else
                    target->RemoveSingleAuraFromSpellAuraHolder(GetId(), GetEffIndex(), GetCasterGuid());
            }
        }
        else if (m_areaAuraType == AREA_AURA_PET)
        {
            if (target->GetObjectGuid() != caster->GetMasterGuid())
                target->RemoveSingleAuraFromSpellAuraHolder(GetId(), GetEffIndex(), GetCasterGuid());
        }
    }
}

void PersistentAreaAura::Update(uint32 diff)
{
    bool remove = true;
    AuraRemoveMode removeMode = AURA_REMOVE_BY_EXPIRE;

    // remove the aura if its caster or the dynamic object causing it was removed
    // or if the target moves too far from the dynamic object
    if (Unit* caster = GetCaster())
    {
        if (DynamicObject* dynObj = caster->GetDynObject(GetId(), GetEffIndex(), GetTarget()))
        {
            if (GetTarget()->GetDistance(dynObj, true, DIST_CALC_COMBAT_REACH) > dynObj->GetRadius())
            {
                removeMode = AURA_REMOVE_BY_DEFAULT;
                dynObj->RemoveAffected(GetTarget()); // let later reapply if target return to range
            }
            else
                remove = false;
        }
    }

    if (remove)
    {
        GetTarget()->RemoveSingleAuraFromSpellAuraHolder(GetHolder(), GetEffIndex(), removeMode);
        return;
    }

    Aura::Update(diff);
}

bool Aura::IsSaveToDbAura() const
{
    if (IsAreaAura() && GetHolder()->GetCasterGuid() != GetTarget()->GetObjectGuid())
        return false;

    if (IsPersistent())
        return false;

    return true;
}

void Aura::ApplyModifier(bool apply, bool Real)
{
    AuraType aura = m_modifier.m_auraname;

    if (apply)
        OnApply(apply);
    if (!apply)
        OnAfterApply(apply);
    if (aura < TOTAL_AURAS)
        (*this.*AuraHandler [aura])(apply, Real);
    if (apply)
        OnAfterApply(apply);
    if (!apply)
        OnApply(apply);
}

ClassFamilyMask Aura::GetAuraSpellClassMask() const
{
    return sSpellMgr.GetSpellAffectMask(GetId(), GetEffIndex());
}

bool Aura::isAffectedOnSpell(SpellEntry const* spellProto) const
{
    if (m_affectOverriden)
        return OnAffectCheck(spellProto);

    if (!spellProto)
        return false;

    if (m_spellmod)
        return m_spellmod->isAffectedOnSpell(spellProto);

    // Check family name
    if (spellProto->SpellFamilyName != GetSpellProto()->SpellFamilyName)
        return false;

    ClassFamilyMask mask = sSpellMgr.GetSpellAffectMask(GetId(), GetEffIndex());
    return spellProto->IsFitToFamilyMask(mask);
}

bool Aura::CanProcFrom(SpellEntry const* spell, uint32 EventProcEx, uint32 procEx, bool damaging, bool absorbing, bool useClassMask) const
{
    // Check EffectClassMask
    ClassFamilyMask mask = GetAuraSpellClassMask();

    // allow proc for modifier auras with charges
    if (IsCastEndProcModifierAura(GetSpellProto(), GetEffIndex(), spell))
    {
        if (GetHolder()->GetAuraCharges() > 0)
        {
            if (procEx != PROC_EX_CAST_END && EventProcEx == PROC_EX_NONE)
                return false;
        }
    }
    else if (EventProcEx == PROC_EX_NONE && procEx == PROC_EX_CAST_END)
        return false;

    // if no class mask defined, or spell_proc_event has SpellFamilyName=0 - allow proc
    if (!useClassMask || !mask)
    {
        if (!(EventProcEx & PROC_EX_EX_TRIGGER_ON_NO_DAMAGE))
        {
            // Check for extra req (if none) and hit/crit
            if (EventProcEx == PROC_EX_NONE)
            {
                if ((procEx & PROC_EX_CAST_END) != 0) // cast end does not care about damage/healing
                    return true;
                if ((procEx & PROC_EX_ABSORB) && absorbing) // trigger ex absorb procs even if no damage is dealt
                    return true;

                // No extra req, so can trigger only for active (damage/healing present) and hit/crit
                return ((procEx & (PROC_EX_NORMAL_HIT | PROC_EX_CRITICAL_HIT)) && damaging);
            }
            // Passive spells hits here only if resist/reflect/immune/evade
            // Passive spells can`t trigger if need hit (exclude cases when procExtra include non-active flags)
            if ((EventProcEx & (PROC_EX_NORMAL_HIT | PROC_EX_CRITICAL_HIT) & procEx) && !damaging)
                return (EventProcEx & (PROC_EX_ABSORB) & procEx) && !absorbing; // but allow full absorb case when specified
        }
        return true;
    }
    // SpellFamilyName check is performed in SpellMgr::IsSpellProcEventCanTriggeredBy and it is done once for whole holder
    // note: SpellFamilyName is not checked if no spell_proc_event is defined
    return mask.IsFitToFamilyMask(spell->SpellFamilyFlags);
}

void Aura::ReapplyAffectedPassiveAuras(Unit* target)
{
    // we need store cast item guids for self casted spells
    // expected that not exist permanent auras from stackable auras from different items
    std::map<uint32, ObjectGuid> affectedSelf;

    for (Unit::SpellAuraHolderMap::const_iterator itr = target->GetSpellAuraHolderMap().begin(); itr != target->GetSpellAuraHolderMap().end(); ++itr)
    {
        // permanent passive
        // passive spells can be affected only by own or owner spell mods)
        if (itr->second->IsPassive() && itr->second->IsPermanent() &&
                // non deleted and not same aura (any with same spell id)
                !itr->second->IsDeleted() && itr->second->GetId() != GetId() &&
                // and affected by aura
                itr->second->GetCasterGuid() == target->GetObjectGuid() &&
                // and affected by spellmod
                isAffectedOnSpell(itr->second->GetSpellProto()))
        {
            affectedSelf[itr->second->GetId()] = itr->second->GetCastItemGuid();
        }
    }

    if (!affectedSelf.empty())
    {
        Player* pTarget = target->GetTypeId() == TYPEID_PLAYER ? (Player*)target : nullptr;

        for (std::map<uint32, ObjectGuid>::const_iterator map_itr = affectedSelf.begin(); map_itr != affectedSelf.end(); ++map_itr)
        {
            Item* item = pTarget && map_itr->second ? pTarget->GetItemByGuid(map_itr->second) : nullptr;
            target->RemoveAurasDueToSpell(map_itr->first);
            target->CastSpell(target, map_itr->first, TRIGGERED_OLD_TRIGGERED, item);
        }
    }
}

struct ReapplyAffectedPassiveAurasHelper
{
    explicit ReapplyAffectedPassiveAurasHelper(Aura* _aura) : aura(_aura) {}
    void operator()(Unit* unit) const { aura->ReapplyAffectedPassiveAuras(unit); }
    Aura* aura;
};

void Aura::ReapplyAffectedPassiveAuras()
{
    // not reapply spell mods with charges (use original value because processed and at remove)
    if (GetSpellProto()->procCharges)
        return;

    // not reapply some spell mods ops (mostly speedup case)
    switch (m_modifier.m_miscvalue)
    {
        case SPELLMOD_DURATION:
        case SPELLMOD_CHARGES:
        case SPELLMOD_NOT_LOSE_CASTING_TIME:
        case SPELLMOD_CASTING_TIME:
        case SPELLMOD_COOLDOWN:
        case SPELLMOD_COST:
        case SPELLMOD_ACTIVATION_TIME:
        case SPELLMOD_GLOBAL_COOLDOWN:
        case SPELLMOD_SPEED:
        case SPELLMOD_HASTE:
        case SPELLMOD_ATTACK_POWER:
            return;
    }

    // reapply talents to own passive persistent auras
    ReapplyAffectedPassiveAuras(GetTarget());

    // re-apply talents/passives/area auras applied to pet/totems (it affected by player spellmods)
    GetTarget()->CallForAllControlledUnits(ReapplyAffectedPassiveAurasHelper(this), CONTROLLED_PET | CONTROLLED_TOTEMS);
}

void Aura::PickTargetsForSpellTrigger(Unit*& triggerCaster, Unit*& triggerTarget, WorldObject*& triggerTargetObject, SpellEntry const* spellInfo)
{
    // automatic caster - target resolution
    switch (spellInfo->EffectImplicitTargetA[0])
    {
        case TARGET_UNIT_ENEMY:
        case TARGET_UNIT:
            triggerCaster = GetCaster();
            if (!triggerCaster)
                triggerCaster = triggerTarget;
            triggerTarget = triggerCaster->GetTarget(); // This will default to channel target for channels
            break;
        case TARGET_UNIT_FRIEND: // Abolish Disease / Poison confirms this
        case TARGET_UNIT_CASTER:
            triggerCaster = GetTarget();
            triggerTarget = GetTarget();
            break;
        case TARGET_LOCATION_CASTER_SRC: // TODO: this needs to be done whenever target isnt important, doing it per case for safety
            //[[fallthrough]]
        case TARGET_LOCATION_CASTER_DEST:
        case TARGET_UNIT_SCRIPT_NEAR_CASTER:
        default:
            triggerTarget = nullptr;
            break;
    }
}

void Aura::CastTriggeredSpell(PeriodicTriggerData& data)
{
    Spell* spell = new Spell(data.trueCaster ? data.trueCaster : data.caster, data.spellInfo, data.triggerFlags, data.trueCaster ? data.trueCaster->GetObjectGuid() : data.caster->GetObjectGuid(), GetSpellProto());
    if (data.spellInfo->HasAttribute(SPELL_ATTR_EX2_RETAIN_ITEM_CAST)) // forward guid to at least spell go
        spell->SetForwardedCastItem(GetCastItemGuid());
    if (data.trueCaster && data.caster)
        spell->SetFakeCaster(data.caster);
    SpellCastTargets targets;
    if (data.spellInfo->Targets & TARGET_FLAG_DEST_LOCATION)
    {
        if (data.targetObject)
            targets.setDestination(data.targetObject->GetPositionX(), data.targetObject->GetPositionY(), data.targetObject->GetPositionZ());
        else if (data.target)
            targets.setDestination(data.target->GetPositionX(), data.target->GetPositionY(), data.target->GetPositionZ());
        else
            targets.setDestination(data.caster->GetPositionX(), data.caster->GetPositionY(), data.caster->GetPositionZ());
    }
    if (data.spellInfo->Targets & TARGET_FLAG_SOURCE_LOCATION)
    {
        if (data.targetObject)
            targets.setSource(data.targetObject->GetPositionX(), data.targetObject->GetPositionY(), data.targetObject->GetPositionZ());
        else if (data.target)
            targets.setSource(data.target->GetPositionX(), data.target->GetPositionY(), data.target->GetPositionZ());
        else
            targets.setSource(data.caster->GetPositionX(), data.caster->GetPositionY(), data.caster->GetPositionZ());
    }
    if (data.target)
        targets.setUnitTarget(data.target);
    if (data.basePoints[0])
        spell->m_currentBasePoints[EFFECT_INDEX_0] = data.basePoints[0];
    if (data.basePoints[1])
        spell->m_currentBasePoints[EFFECT_INDEX_1] = data.basePoints[1];
    if (data.basePoints[2])
        spell->m_currentBasePoints[EFFECT_INDEX_2] = data.basePoints[2];
    spell->SpellStart(&targets, this);
}

/*********************************************************/
/***               BASIC AURA FUNCTION                 ***/
/*********************************************************/
void Aura::HandleAddModifier(bool apply, bool Real)
{
    if (GetTarget()->GetTypeId() != TYPEID_PLAYER || !Real)
        return;

    if (m_modifier.m_miscvalue >= MAX_SPELLMOD)
        return;

    if (apply)
    {
        SpellEntry const* spellProto = GetSpellProto();
        uint32 priority = spellProto->spellPriority;

        // Add custom charges for some mod aura
        switch (spellProto->Id)
        {
            case 17941:                                     // Shadow Trance
            case 22008:                                     // Netherwind Focus
                GetHolder()->SetAuraCharges(1);
                break;
            case 16246:                                     // Clearcasting - Shaman - need to be applied before elemental mastery
                priority = 100;
                break;
        }

        m_spellmod = new SpellModifier(
            SpellModOp(m_modifier.m_miscvalue),
            SpellModType(m_modifier.m_auraname),            // SpellModType value == spell aura types
            m_modifier.m_amount,
            this,
            priority,
            // prevent expire spell mods with (charges > 0 && m_stackAmount > 1)
            // all this spell expected expire not at use but at spell proc event check
            spellProto->StackAmount > 1 ? 0 : GetHolder()->GetAuraCharges());
    }

    static_cast<Player*>(GetTarget())->AddSpellMod(m_spellmod, apply);
    if (!apply)
        m_spellmod = nullptr;

    ReapplyAffectedPassiveAuras();
}

void Aura::TriggerSpell()
{
    ObjectGuid casterGUID = GetCasterGuid();
    Unit* triggerTarget = GetTriggerTarget();

    if (!casterGUID || !triggerTarget)
        return;

    // generic casting code with custom spells and target/caster customs
    uint32 trigger_spell_id = GetSpellProto()->EffectTriggerSpell[m_effIndex];

    SpellEntry const* triggeredSpellInfo = sSpellTemplate.LookupEntry<SpellEntry>(trigger_spell_id);
    SpellEntry const* auraSpellInfo = GetSpellProto();
    uint32 auraId = auraSpellInfo->Id;
    Unit* target = GetTarget();
    Unit* triggerCaster = triggerTarget;
    WorldObject* triggerTargetObject = nullptr;

    // specific code for cases with no trigger spell provided in field
    if (triggeredSpellInfo == nullptr)
    {
        switch (auraSpellInfo->SpellFamilyName)
        {
            case SPELLFAMILY_GENERIC:
            {
                switch (auraId)
                {
//                    // Polymorphic Ray
//                    case 6965: break;
                    case 9712:                              // Thaumaturgy Channel
                        if (Unit* caster = GetCaster())
                            caster->CastSpell(caster, 21029, TRIGGERED_OLD_TRIGGERED);
                        return;
                    case 23170:                             // Brood Affliction: Bronze
                    {
                        // Only 10% chance of triggering spell, return for the remaining 90%
                        if (urand(0, 9) >= 1)
                            return;
                        target->CastSpell(target, 23171, TRIGGERED_OLD_TRIGGERED, nullptr, this);
                        return;
                    }
                    case 23493:                             // Restoration
                    {
                        // sniff confirmed to send no packets
                        uint32 heal = triggerTarget->GetMaxHealth() / 10;
                        triggerTarget->ModifyHealth(int32(heal));
                        if (int32 mana = triggerTarget->GetMaxPower(POWER_MANA))
                        {
                            mana /= 10;
                            triggerTarget->ModifyPower(POWER_MANA, mana);
                        }
                        return;
                    }
//                    // Restoration
//                    case 24379: break;
                    case 24743:                             // Cannon Prep
                    case 24832:                             // Cannon Prep
                        trigger_spell_id = 24731;
                        break;
                    case 24834:                             // Shadow Bolt Whirl
                    {
                        uint32 spellForTick[8] = { 24820, 24821, 24822, 24823, 24835, 24836, 24837, 24838 };
                        uint32 tick = (GetAuraTicks() + 7/*-1*/) % 8;

                        // casted in left/right (but triggered spell have wide forward cone)
                        float forward = target->GetOrientation();
                        if (tick <= 3)
                            target->SetOrientation(forward + 0.75f * M_PI_F - tick * M_PI_F / 8);       // Left
                        else
                            target->SetOrientation(forward - 0.75f * M_PI_F + (8 - tick) * M_PI_F / 8); // Right

                        triggerTarget->CastSpell(triggerTarget, spellForTick[tick], TRIGGERED_OLD_TRIGGERED, nullptr, this, casterGUID);
                        target->SetOrientation(forward);
                        return;
                    }
//                    // Stink Trap
//                    case 24918: break;
                    case 25371:                             // Consume
                    {
                        int32 bpDamage = triggerTarget->GetMaxHealth() * 10 / 100;
                        triggerTarget->CastCustomSpell(triggerTarget, 25373, &bpDamage, nullptr, nullptr, TRIGGERED_OLD_TRIGGERED, nullptr, this, casterGUID);
                        return;
                    }
//                    // Consume
//                    case 26196: break;
//                    // Defile
//                    case 27177: break;
//                    // Five Fat Finger Exploding Heart Technique
//                    case 27673: break;
                    case 27746:                             // Nitrous Boost
                    {
                        if (triggerTarget->GetPower(POWER_MANA) >= 10)
                        {
                            triggerTarget->ModifyPower(POWER_MANA, -10);
                            triggerTarget->SendEnergizeSpellLog(triggerTarget, 27746, -10, POWER_MANA);
                        }
                        else
                        {
                            triggerTarget->RemoveAurasDueToSpell(27746);
                        }
                        return;
                    }
                    case 27747:                             // Steam Tank Passive
                    {
                        if (triggerTarget->GetPower(POWER_MANA) <= 90)
                        {
                            triggerTarget->ModifyPower(POWER_MANA, 10);
                            triggerTarget->SendEnergizeSpellLog(triggerTarget, 27747, 10, POWER_MANA);
                        }
                        return;
                    }
                    // Detonate Mana
                    case 27819:
                    {
                        // 50% Mana Burn
                        int32 bpDamage = (int32)triggerTarget->GetPower(POWER_MANA) / 2;
                        triggerTarget->ModifyPower(POWER_MANA, -bpDamage);
                        triggerTarget->CastCustomSpell(triggerTarget, 27820, &bpDamage, nullptr, nullptr, TRIGGERED_OLD_TRIGGERED, nullptr, this, triggerTarget->GetObjectGuid());
                        return;
                    }
                    // Stalagg Chain and Feugen Chain
                    case 28096:
                    case 28111:
//                    // Guardian of Icecrown Passive
//                    case 29897: break;
//                    // Mind Exhaustion Passive
//                    case 30025: break;
                    default:
                        break;
                }
                break;
            }
//            case SPELLFAMILY_MAGE:
//            {
//                switch (auraId)
//                {
//                    default:
//                        break;
//                }
//                break;
//            }
            case SPELLFAMILY_WARRIOR:
            {
                switch(auraId)
                {
                    case 23410:                             // Wild Magic (Mage class call in Nefarian encounter)
                    {
                        trigger_spell_id = 23603;
                        break;
                    }
                    default:
                        break;
                }
                break;
            }
//            case SPELLFAMILY_PRIEST:
//            {
//                switch(auraId)
//                {
//                    default:
//                        break;
//                }
//                break;
//            }
            case SPELLFAMILY_DRUID:
            {
                switch (auraId)
                {
                    case 768:                               // Cat Form
                        // trigger_spell_id not set and unknown effect triggered in this case, ignoring for while
                        return;
                    case 22842:                             // Frenzied Regeneration
                    case 22895:
                    case 22896:
                    {
                        int32 LifePerRage = GetModifier()->m_amount;

                        int32 lRage = target->GetPower(POWER_RAGE);
                        if (lRage > 100)                    // rage stored as rage*10
                            lRage = 100;
                        target->ModifyPower(POWER_RAGE, -lRage);
                        int32 FRTriggerBasePoints = int32(lRage * LifePerRage / 10);
                        target->CastCustomSpell(target, 22845, &FRTriggerBasePoints, nullptr, nullptr, TRIGGERED_OLD_TRIGGERED, nullptr, this);
                        return;
                    }
                    default:
                        break;
                }
                break;
            }
//            case SPELLFAMILY_HUNTER:
//            {
//                switch(auraId)
//                {
//                    default:
//                        break;
//                }
//                break;
//            }
//            case SPELLFAMILY_SHAMAN:
//            {
//                switch (auraId)
//                {
//                    default:
//                        break;
//                }
//                break;
//            }
            default:
                break;
        }

        // Reget trigger spell proto
        triggeredSpellInfo = sSpellTemplate.LookupEntry<SpellEntry>(trigger_spell_id);
    }
    else 
    {
        // for channeled spell cast applied from aura owner to channel target (persistent aura affects already applied to true target)
        // come periodic casts applied to targets, so need select proper caster (ex. 15790)
        // interesting 2 cases: periodic aura at caster of channeled spell
        if (target->GetObjectGuid() == casterGUID)
            triggerCaster = target;

        PickTargetsForSpellTrigger(triggerCaster, triggerTarget, triggerTargetObject, triggeredSpellInfo);

        // Spell exist but require custom code
        switch (auraId)
        {
            case 9347:                                      // Mortal Strike
            {
                if (target->GetTypeId() != TYPEID_UNIT)
                    return;
                // expected selection current fight target
                triggerTarget = ((Creature*)target)->SelectAttackingTarget(ATTACKING_TARGET_TOPAGGRO, 0, triggeredSpellInfo);
                if (!triggerTarget)
                    return;

                break;
            }
            case 1010:                                      // Curse of Idiocy
            {
                // TODO: spell casted by result in correct way mostly
                // BUT:
                // 1) target show casting at each triggered cast: target don't must show casting animation for any triggered spell
                //      but must show affect apply like item casting
                // 2) maybe aura must be replace by new with accumulative stat mods instead stacking

                // prevent cast by triggered auras
                if (casterGUID == triggerTarget->GetObjectGuid())
                    return;

                // stop triggering after each affected stats lost > 90
                int32 intelectLoss = 0;
                int32 spiritLoss = 0;

                Unit::AuraList const& mModStat = triggerTarget->GetAurasByType(SPELL_AURA_MOD_STAT);
                for (auto i : mModStat)
                {
                    if (i->GetId() == 1010)
                    {
                        switch (i->GetModifier()->m_miscvalue)
                        {
                            case STAT_INTELLECT: intelectLoss += i->GetModifier()->m_amount; break;
                            case STAT_SPIRIT:    spiritLoss   += i->GetModifier()->m_amount; break;
                            default: break;
                        }
                    }
                }

                if (intelectLoss <= -90 && spiritLoss <= -90)
                    return;

                break;
            }
            case 16191:                                     // Mana Tide
            {
                triggerCaster->CastCustomSpell(nullptr, trigger_spell_id, &m_modifier.m_amount, nullptr, nullptr, TRIGGERED_OLD_TRIGGERED, nullptr, this);
                return;
            }
        }
    }
    int32 basePoints[] = { 0,0,0 };
    PeriodicTriggerData data(nullptr, triggerCaster, triggerTarget, triggerTargetObject, triggeredSpellInfo, basePoints);
    OnPeriodicTrigger(data);

    // All ok cast by default case
    if (data.spellInfo)
    {
        CastTriggeredSpell(data);
    }
    else if (!GetAuraScript()) // if scripter scripted spell, it is handled somehow
    {
        if (Unit* caster = GetCaster())
        {
            if (triggerTarget->GetTypeId() != TYPEID_UNIT || !sScriptDevAIMgr.OnEffectDummy(caster, GetId(), GetEffIndex(), (Creature*)triggerTarget, ObjectGuid()))
                sLog.outError("Aura::TriggerSpell: Spell %u have 0 in EffectTriggered[%d], not handled custom case?", GetId(), GetEffIndex());
        }
    }
}

/*********************************************************/
/***                  AURA EFFECTS                     ***/
/*********************************************************/

void Aura::HandleAuraDummy(bool apply, bool Real)
{
    // spells required only Real aura add/remove
    if (!Real)
        return;

    Unit* target = GetTarget();

    // AT APPLY
    if (apply)
    {
        switch (GetSpellProto()->SpellFamilyName)
        {
            case SPELLFAMILY_GENERIC:
            {
                switch (GetId())
                {
                    case 126:                               // Eye of Kilrogg
                    {
                        if (target->IsPlayer())
                        {
                            Unit* pet = static_cast<Player*>(target)->GetCharm();
                            if (pet && pet->GetEntry() == 4277)
                            {
                                pet->CastSpell(pet, 2585, TRIGGERED_OLD_TRIGGERED);
                            }
                        }
                        break;
                    }
                    case 6946:                              // Curse of the Bleakheart
                    case 41170:
                        m_isPeriodic = true;
                        m_modifier.periodictime = 5 * IN_MILLISECONDS;
                        m_periodicTimer = m_modifier.periodictime;
                        return;
                    case 7057:                              // Haunting Spirits
                        // expected to tick with 30 sec period (tick part see in Aura::PeriodicTick)
                        m_isPeriodic = true;
                        m_modifier.periodictime = 30 * IN_MILLISECONDS;
                        m_periodicTimer = m_modifier.periodictime;
                        return;
                    case 7054:                              // Forsaken Skill
                        m_isPeriodic = true;
                        m_modifier.periodictime = 1 * IN_MILLISECONDS;
                        m_periodicTimer = m_modifier.periodictime;
                        break;
                    case 13139:                             // net-o-matic
                        // root to self part of (root_target->charge->root_self sequence
                        if (Unit* caster = GetCaster())
                            caster->CastSpell(caster, 13138, TRIGGERED_OLD_TRIGGERED, nullptr, this);
                        return;
                    case 21094:                             // Separation Anxiety (Majordomo Executus)
                    case 23487:                             // Separation Anxiety (Garr)
                    {
                        // expected to tick with 5 sec period (tick part see in Aura::PeriodicTick)
                        m_isPeriodic = true;
                        m_modifier.periodictime = 5 * IN_MILLISECONDS;
                        m_periodicTimer = m_modifier.periodictime;
                        return;
                    }
                    case 23183:                             // Mark of Frost
                    {
                        if (target->HasAura(23182))
                            target->CastSpell(target, 23186, TRIGGERED_OLD_TRIGGERED, nullptr, nullptr, GetCasterGuid());
                        return;
                    }
                    case 25042:                             // Mark of Nature
                    {
                        if (target->HasAura(25040))
                            target->CastSpell(target, 25043, TRIGGERED_OLD_TRIGGERED, nullptr, nullptr, GetCasterGuid());
                        return;
                    }
                    case 25471:                             // Attack Order
                    {
                        target->CastSpell(nullptr, 25473, TRIGGERED_OLD_TRIGGERED);
                        return;
                    }
                    case 26681:                             // Cologne
                    {
                        if (Unit* target = GetTarget())
                        {
                            if (target->HasAura(26682))
                                target->RemoveAurasDueToSpell(26682);
                        }
                        return;
                    }
                    case 26682:                             // Perfume
                    {
                        if (Unit* target = GetTarget())
                        {
                            if (target->HasAura(26681))
                                target->RemoveAurasDueToSpell(26681);
                        }
                        return;
                    }
                    case 28832:                             // Mark of Korth'azz
                    case 28833:                             // Mark of Blaumeux
                    case 28834:                             // Mark of Rivendare
                    case 28835:                             // Mark of Zeliek
                    {
                        int32 damage;
                        switch (GetStackAmount())
                        {
                            case 1:
                                return;
                            case 2: damage =   250; break;
                            case 3: damage =  1000; break;
                            case 4: damage =  3000; break;
                            default:
                                damage = 1000 * GetStackAmount();
                                break;
                        }

                        if (Unit* caster = GetCaster())
                            caster->CastCustomSpell(target, 28836, &damage, nullptr, nullptr, TRIGGERED_OLD_TRIGGERED, nullptr, this);
                        return;
                    }
                }
                break;
            }
            case SPELLFAMILY_WARRIOR:
            {
                switch (GetId())
                {
                    case 23427:             // Summon Infernals (Warlock class call in Nefarian encounter)
                    {
                        if (Unit* target = GetTarget())
                            target->CastSpell(target, 23426, TRIGGERED_OLD_TRIGGERED);
                        return;
                    }
                }
                break;
            }
            case SPELLFAMILY_PRIEST:
            {
                switch (GetId())
                {
                    case 30238:             // Lordaeron's Blessing
                    {
                        target->CastSpell(target, 31906, TRIGGERED_OLD_TRIGGERED);
                        return;
                    }
                }
                break;
            }
        }
    }
    // AT REMOVE
    else
    {
        if (IsQuestTameSpell(GetId()) && target->IsAlive())
        {
            Unit* caster = GetCaster();
            if (!caster || !caster->IsAlive())
                return;

            uint32 finalSpellId = 0;
            switch (GetId())
            {
                case 19548: finalSpellId = 19597; break;
                case 19674: finalSpellId = 19677; break;
                case 19687: finalSpellId = 19676; break;
                case 19688: finalSpellId = 19678; break;
                case 19689: finalSpellId = 19679; break;
                case 19692: finalSpellId = 19680; break;
                case 19693: finalSpellId = 19684; break;
                case 19694: finalSpellId = 19681; break;
                case 19696: finalSpellId = 19682; break;
                case 19697: finalSpellId = 19683; break;
                case 19699: finalSpellId = 19685; break;
                case 19700: finalSpellId = 19686; break;
            }

            if (finalSpellId)
                caster->CastSpell(target, finalSpellId, TRIGGERED_OLD_TRIGGERED, nullptr, this);

            return;
        }

        switch (GetId())
        {
            case 11129:                                     // Combustion
                target->RemoveAurasDueToSpell(28682); // on Combustion removal remove crit % stacks
                return;
            case 11826:                                     // Electromagnetic Gigaflux Reactivator
                if (m_removeMode != AURA_REMOVE_BY_EXPIRE)
                    return;

                if (Unit* caster = GetCaster())
                {
                    if (caster->GetTypeId() == TYPEID_PLAYER)
                        caster->CastSpell(target, 11828, TRIGGERED_OLD_TRIGGERED, ((Player*)caster)->GetItemByGuid(this->GetCastItemGuid()), this);
                }
                return;
            case 12774:                                     // (DND) Belnistrasz Idol Shutdown Visual
            {
                if (m_removeMode == AURA_REMOVE_BY_DEATH)
                    return;

                // Idom Rool Camera Shake <- wtf, don't drink while making spellnames?
                if (Unit* caster = GetCaster())
                    caster->CastSpell(caster, 12816, TRIGGERED_OLD_TRIGGERED);

                return;
            }
            case 17189:                                     // Frostwhisper's Lifeblood
                // Ras Frostwhisper gets back to full health when turned to his human form
                if (Unit* caster = GetCaster())
                    caster->ModifyHealth(caster->GetMaxHealth() - caster->GetHealth());
                return;
            case 25185:                                     // Itch
            {
                GetCaster()->CastSpell(target, 25187, TRIGGERED_OLD_TRIGGERED);
                return;
            }
            case 26077:                                     // Itch
            {
                GetCaster()->CastSpell(target, 26078, TRIGGERED_OLD_TRIGGERED);
                return;
            }
            case 29104:                                     // Anub'Rekhan Aura
            {
                if (m_removeMode == AURA_REMOVE_BY_DEATH && target->GetTypeId() == TYPEID_PLAYER)
                    target->CastSpell(target, 29105, TRIGGERED_OLD_TRIGGERED, nullptr, this);
                return;
            }
            case 30238:                                     // Lordaeron's Blessing
            {
                target->RemoveAurasDueToSpell(31906);
                return;
            }
        }

        if (m_removeMode == AURA_REMOVE_BY_DEATH)
        {
            // Stop caster Arcane Missle chanelling on death
            if (GetSpellProto()->SpellFamilyName == SPELLFAMILY_MAGE && (GetSpellProto()->SpellFamilyFlags & uint64(0x0000000000000800)))
            {
                if (Unit* caster = GetCaster())
                    caster->InterruptSpell(CURRENT_CHANNELED_SPELL);

                return;
            }
        }
    }

    // AT APPLY & REMOVE

    switch (GetSpellProto()->SpellFamilyName)
    {
        case SPELLFAMILY_GENERIC:
        {
            switch (GetId())
            {
                case 6606:                                  // Self Visual - Sleep Until Cancelled (DND)
                {
                    if (apply)
                    {
                        target->SetStandState(UNIT_STAND_STATE_SLEEP);
                        target->addUnitState(UNIT_STAT_ROOT);
                    }
                    else
                    {
                        target->clearUnitState(UNIT_STAT_ROOT);
                        target->SetStandState(UNIT_STAND_STATE_STAND);
                    }

                    return;
                }
                case 16093:                                  // Self Visual - Sleep Until Cancelled (DND)
                {
                    if (apply)
                    {
                        target->SetStandState(UNIT_STAND_STATE_SLEEP);
                        target->addUnitState(UNIT_STAT_ROOT);
                    }
                    else
                    {
                        target->clearUnitState(UNIT_STAT_ROOT);
                        target->SetStandState(UNIT_STAND_STATE_STAND);
                    }

                    return;
                }
                case 24658:                                 // Unstable Power
                {
                    if (apply)
                    {
                        Unit* caster = GetCaster();
                        if (!caster)
                            return;

                        caster->CastSpell(target, 24659, TRIGGERED_OLD_TRIGGERED, nullptr, nullptr, GetCasterGuid());
                    }
                    else
                        target->RemoveAurasDueToSpell(24659);
                    return;
                }
                case 24661:                                 // Restless Strength
                {
                    if (apply)
                    {
                        Unit* caster = GetCaster();
                        if (!caster)
                            return;

                        caster->CastSpell(target, 24662, TRIGGERED_OLD_TRIGGERED, nullptr, nullptr, GetCasterGuid());
                    }
                    else
                        target->RemoveAurasDueToSpell(24662);
                    return;
                }
                case 28819:                             // Submerge Visual
                {
                    if (apply)
                    {
                        target->SetStandState(UNIT_STAND_STATE_CUSTOM);
                        if (target->GetTypeId() == TYPEID_UNIT && target->GetEntry() == 16236)
                            ((Creature*)target)->ForcedDespawn(700);
                    }
                    else
                        target->SetStandState(UNIT_STAND_STATE_STAND);
                    return;
                }
                case 29266:                                 // Permanent Feign Death
                {
                    // Unclear what the difference really is between them.
                    // Some has effect1 that makes the difference, however not all.
                    // Some appear to be used depending on creature location, in water, at solid ground, in air/suspended, etc
                    // For now, just handle all the same way
                    //if (target->GetTypeId() == TYPEID_UNIT)
                    target->SetFeignDeath(apply, GetCasterGuid(), GetId());

                    return;
                }
            }
            break;
        }
        case SPELLFAMILY_DRUID:
        {
            // Predatory Strikes
            if (target->GetTypeId() == TYPEID_PLAYER && GetSpellProto()->SpellIconID == 1563)
            {
                ((Player*)target)->UpdateAttackPowerAndDamage();
                return;
            }
            break;
        }
        case SPELLFAMILY_ROGUE:
            break;
        case SPELLFAMILY_HUNTER:
            break;
        case SPELLFAMILY_PALADIN:
        {
            switch (GetId())
            {
                case 21082:                                 // Seal of the Crusader, rank 1
                case 20162:                                 // rank 2
                case 20305:                                 // rank 3
                case 20306:                                 // rank 4
                case 20307:                                 // rank 5
                case 20308:                                 // rank 6
                    ApplyPercentModFloatVar(target->m_modAttackBaseDPSPct[BASE_ATTACK], 40, !apply);
                    target->UpdateDamagePhysical(BASE_ATTACK);
                    return;
            }
            break;
        }
    }

    // pet auras
    if (PetAura const* petSpell = sSpellMgr.GetPetAura(GetId()))
    {
        if (apply)
            target->AddPetAura(petSpell);
        else
            target->RemovePetAura(petSpell);
        return;
    }

    if (target->GetTypeId() == TYPEID_PLAYER)
    {
        SpellAreaForAreaMapBounds saBounds = sSpellMgr.GetSpellAreaForAuraMapBounds(GetId());
        if (saBounds.first != saBounds.second)
        {
            uint32 zone, area;
            target->GetZoneAndAreaId(zone, area);
            std::set<uint32> appliedSpells;
            std::set<uint32> removedSpells;

            Player* player = static_cast<Player*>(target);

            // custom loop for evaluation reasons
            for (SpellAreaForAreaMap::const_iterator itr = saBounds.first; itr != saBounds.second; ++itr)
            {
                auto data = (*itr).second;
                uint32 spellId = data->spellId;
                // one success trumps multiple failures
                if (appliedSpells.find(spellId) != appliedSpells.end())
                    continue;

                if (data->IsFitToRequirements(player, zone, area))
                {
                    if (data->autocast && !player->HasAura(spellId))
                    {
                        appliedSpells.insert(spellId);
                        removedSpells.erase(spellId);
                    }
                }
                else if (player->HasAura(spellId))
                    removedSpells.insert(spellId);
            }

            for (auto spellId : appliedSpells)
                player->CastSpell(player, spellId, TRIGGERED_OLD_TRIGGERED);

            for (auto spellId : removedSpells)
                player->RemoveAurasDueToSpell(spellId);
        }
    }

    // script has to "handle with care", only use where data are not ok to use in the above code.
    if (target->GetTypeId() == TYPEID_UNIT)
        sScriptDevAIMgr.OnAuraDummy(this, apply);
}

void Aura::HandleAuraMounted(bool apply, bool Real)
{
    // only at real add/remove aura
    if (!Real)
        return;

    Unit* target = GetTarget();

    if (apply)
    {
        CreatureInfo const* ci = ObjectMgr::GetCreatureTemplate(m_modifier.m_miscvalue);
        if (!ci)
        {
            sLog.outErrorDb("AuraMounted: `creature_template`='%u' not found in database (only need it modelid)", m_modifier.m_miscvalue);
            return;
        }

        uint32 display_id = Creature::ChooseDisplayId(ci);
        CreatureModelInfo const* minfo = sObjectMgr.GetCreatureModelRandomGender(display_id);
        if (minfo)
            display_id = minfo->modelid;

        m_modifier.m_amount = display_id;

        target->Mount(display_id, this);
    }
    else
    {
        target->Unmount(this);
    }
}

void Aura::HandleAuraWaterWalk(bool apply, bool Real)
{
    // only at real add/remove aura
    if (!Real)
        return;

    GetTarget()->SetWaterWalk(apply);
}

void Aura::HandleAuraFeatherFall(bool apply, bool Real)
{
    // only at real add/remove aura
    if (!Real)
        return;

    GetTarget()->SetFeatherFall(apply);
}

void Aura::HandleAuraHover(bool apply, bool Real)
{
    // only at real add/remove aura
    if (!Real)
        return;

    GetTarget()->SetHover(apply);
}

void Aura::HandleWaterBreathing(bool apply, bool /*Real*/)
{
    Unit* target = GetTarget();

    if (target->GetTypeId() == TYPEID_PLAYER)
    {
        if (apply || target->HasAuraType(SPELL_AURA_WATER_BREATHING))
            static_cast<Player*>(target)->SetWaterBreathingIntervalMultiplier(0);
        else
            static_cast<Player*>(target)->SetWaterBreathingIntervalMultiplier(target->GetTotalAuraMultiplier(SPELL_AURA_MOD_WATER_BREATHING));
    }
}

void Aura::HandleModWaterBreathing(bool /*apply*/, bool /*Real*/)
{
    Unit* target = GetTarget();

    if (target->GetTypeId() == TYPEID_PLAYER)
        static_cast<Player*>(target)->SetWaterBreathingIntervalMultiplier(target->GetTotalAuraMultiplier(SPELL_AURA_MOD_WATER_BREATHING));
}

void Aura::HandleAuraModShapeshift(bool apply, bool Real)
{
    if (!Real)
        return;

    ShapeshiftForm form = ShapeshiftForm(m_modifier.m_miscvalue);

    SpellShapeshiftFormEntry const* ssEntry = sSpellShapeshiftFormStore.LookupEntry(form);
    if (!ssEntry)
    {
        sLog.outError("Unknown shapeshift form %u in spell %u", form, GetId());
        return;
    }

    Powers PowerType = POWER_MANA;
    Unit* target = GetTarget();
    uint32 displayId = 0;

    switch (form)
    {
        case FORM_CAT:
            if (Player::TeamForRace(target->getRace()) == ALLIANCE)
                displayId = 892;
            else
                displayId = 8571;
            PowerType = POWER_ENERGY;
            break;
        case FORM_TRAVEL:
            displayId = 632;
            break;
        case FORM_AQUA:
            displayId = 2428;
            break;
        case FORM_BEAR:
            if (Player::TeamForRace(target->getRace()) == ALLIANCE)
                displayId = 2281;
            else
                displayId = 2289;
            PowerType = POWER_RAGE;
            break;
        case FORM_GHOUL:
            if (Player::TeamForRace(target->getRace()) == ALLIANCE)
                displayId = 10045;
            break;
        case FORM_DIREBEAR:
            if (Player::TeamForRace(target->getRace()) == ALLIANCE)
                displayId = 2281;
            else
                displayId = 2289;
            PowerType = POWER_RAGE;
            break;
        case FORM_CREATUREBEAR:
            displayId = 902;
            break;
        case FORM_GHOSTWOLF:
            displayId = 4613;
            break;
        case FORM_MOONKIN:
            if (GetId() == 24809)       // Specific case for spell Spirit Shade Visual in Lethon encounter (spell use moonkin form value for some reason)
            {
                Unit* caster = GetCaster();
                if (caster->GetTypeId() != TYPEID_PLAYER)
                    return;

                displayId = caster->GetNativeDisplayId();
            }
            else
            {
                if (Player::TeamForRace(target->getRace()) == ALLIANCE)
                    displayId = 15374;
                else
                    displayId = 15375;
            }
            break;
        case FORM_AMBIENT:
        case FORM_SHADOW:
        case FORM_STEALTH:
            break;
        case FORM_TREE:
            displayId = 864;
            break;
        case FORM_BATTLESTANCE:
        case FORM_BERSERKERSTANCE:
        case FORM_DEFENSIVESTANCE:
            PowerType = POWER_RAGE;
            break;
        case FORM_SPIRITOFREDEMPTION:
            displayId = 16031;
            break;
        default:
            break;
    }

    // remove polymorph before changing display id to keep new display id
    switch (form)
    {
        case FORM_CAT:
        case FORM_TREE:
        case FORM_TRAVEL:
        case FORM_AQUA:
        case FORM_BEAR:
        case FORM_DIREBEAR:
        case FORM_MOONKIN:
        {
            // remove movement affects
            target->RemoveSpellsCausingAura(SPELL_AURA_MOD_ROOT, GetHolder(), true);
            Unit::AuraList const& slowingAuras = target->GetAurasByType(SPELL_AURA_MOD_DECREASE_SPEED);
            for (Unit::AuraList::const_iterator iter = slowingAuras.begin(); iter != slowingAuras.end();)
            {
                SpellEntry const* aurSpellInfo = (*iter)->GetSpellProto();

                uint32 aurMechMask = GetAllSpellMechanicMask(aurSpellInfo);

                if (aurMechMask == 0) // all shapeshift removing spells have some mechanic mask. 33572 and 38132 confirmed not removed on poly
                {
                    ++iter;
                    continue;
                }

                // If spell that caused this aura has Croud Control or Daze effect
                if ((aurMechMask & MECHANIC_NOT_REMOVED_BY_SHAPESHIFT) ||
                        // some Daze spells have these parameters instead of MECHANIC_DAZE (skip snare spells)
                        (aurSpellInfo->SpellIconID == 15 && aurSpellInfo->Dispel == 0 &&
                         (aurMechMask & (1 << (MECHANIC_SNARE - 1))) == 0))
                {
                    ++iter;
                    continue;
                }

                // All OK, remove aura now
                target->RemoveAurasDueToSpellByCancel(aurSpellInfo->Id);
                iter = slowingAuras.begin();
            }

            //no break here
        }
        case FORM_GHOSTWOLF:
        {
            // remove water walk aura. TODO:: there is probably better way to do this
            target->RemoveSpellsCausingAura(SPELL_AURA_WATER_WALK);

            break;
        }
        default:
            break;
    }

    if (apply)
    {
        // remove other shapeshift before applying a new one
        target->RemoveSpellsCausingAura(SPELL_AURA_MOD_SHAPESHIFT, GetHolder());

        if (displayId > 0)
            target->SetDisplayId(displayId);

        if (PowerType != POWER_MANA)
        {
            // reset power to default values only at power change
            if (target->GetPowerType() != PowerType)
                target->SetPowerType(PowerType);

            switch (form)
            {
                case FORM_BATTLESTANCE:
                case FORM_DEFENSIVESTANCE:
                case FORM_BERSERKERSTANCE:
                {
                    ShapeshiftForm previousForm = target->GetShapeshiftForm();
                    uint32 remainingRage = 0;
                    if (previousForm == FORM_DEFENSIVESTANCE)
                        if (Aura* aura = target->GetOverrideScript(831))
                            remainingRage += aura->GetModifier()->m_amount * 10;

                    // Tactical mastery
                    if (target->IsPlayer())
                    {
                        Unit::AuraList const& aurasOverrideClassScripts = target->GetAurasByType(SPELL_AURA_OVERRIDE_CLASS_SCRIPTS);
                        for (Unit::AuraList::const_iterator iter = aurasOverrideClassScripts.begin(); iter != aurasOverrideClassScripts.end(); ++iter)
                        {
                            uint32 rageVal = 0;
                            // select by script id
                            switch ((*iter)->GetModifier()->m_miscvalue)
                            {
                                case 831: rageVal =  50; break;
                                case 832: rageVal = 100; break;
                                case 833: rageVal = 150; break;
                                case 834: rageVal = 200; break;
                                case 835: rageVal = 250; break;
                            }
                            if (rageVal != 0)
                            {
                                remainingRage += rageVal;
                                break;
                            }
                        }
                    }

                    if (target->GetPower(POWER_RAGE) > remainingRage)
                        target->SetPower(POWER_RAGE, remainingRage);
                    break;
                }
                default:
                    break;
            }
        }

        target->SetShapeshiftForm(form);

        target->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_SHAPESHIFTING);
    }
    else
    {
        target->RestoreDisplayId();

        if (target->getClass() == CLASS_DRUID)
            target->SetPowerType(POWER_MANA);

        target->SetShapeshiftForm(FORM_NONE);
    }

    // adding/removing linked auras
    // add/remove the shapeshift aura's boosts
    HandleShapeshiftBoosts(apply);

    if (target->GetTypeId() == TYPEID_PLAYER)
        ((Player*)target)->InitDataForForm();

    if (apply)
    {
        switch (form)
        {
            case FORM_CAT: // need to cast Track Humanoids if no other tracking is on
                if (target->HasSpell(5225) && !target->HasAura(2383) && !target->HasAura(2580))
                    target->CastSpell(nullptr, 5225, TRIGGERED_OLD_TRIGGERED);
                // no break
            case FORM_BEAR:
            case FORM_DIREBEAR:
            {
                // get furor proc chance
                int32 furorChance = 0;
                Unit::AuraList const& mDummy = target->GetAurasByType(SPELL_AURA_DUMMY);
                for (auto i : mDummy)
                {
                    if (i->GetSpellProto()->SpellIconID == 238)
                    {
                        furorChance = i->GetModifier()->m_amount;
                        break;
                    }
                }

                if (m_modifier.m_miscvalue == FORM_CAT)
                {
                    target->SetPower(POWER_ENERGY, 0);
                    if (irand(1, 100) <= furorChance)
                        target->CastSpell(target, 17099, TRIGGERED_OLD_TRIGGERED, nullptr, this);
                }
                else
                {
                    target->SetPower(POWER_RAGE, 0);
                    if (irand(1, 100) <= furorChance)
                        target->CastSpell(target, 17057, TRIGGERED_OLD_TRIGGERED, nullptr, this);
                }
                break;
            }
            default:
                break;
        }
    }
}

void Aura::HandleAuraTransform(bool apply, bool /*Real*/)
{
    Unit* target = GetTarget();
    bool skipDisplayUpdate = false;
    if (apply)
    {
        if (uint32 overrideDisplayId = GetAuraScriptCustomizationValue()) // from script
            m_modifier.m_amount = overrideDisplayId;
        // special case (spell specific functionality)
        else if (m_modifier.m_miscvalue == 0)
        {
//             switch (GetId())
//             {
//                 default:
                    if (!m_modifier.m_amount) // can be set through script
                        sLog.outError("Aura::HandleAuraTransform, spell %u does not have creature entry defined, need custom defined model.", GetId());
//                    break;
//             }
        }
        else                                                // m_modifier.m_miscvalue != 0
        {
            CreatureInfo const* cInfo = ObjectMgr::GetCreatureTemplate(m_modifier.m_miscvalue);
            if (!cInfo)
            {
                m_modifier.m_amount = 16358;                           // pig pink ^_^
                sLog.outError("Auras: unknown creature id = %d (only need its modelid) Form Spell Aura Transform in Spell ID = %d", m_modifier.m_miscvalue, GetId());
            }
            else
                m_modifier.m_amount = Creature::ChooseDisplayId(cInfo);   // Will use the default model here

            // creature case, need to update equipment if additional provided
            if (cInfo && target->GetTypeId() == TYPEID_UNIT)
            {
                skipDisplayUpdate = ((Creature*)target)->IsTotem();
                ((Creature*)target)->LoadEquipment(cInfo->EquipmentTemplateId, false);
            }

            if (cInfo && cInfo->Scale != 1.f)
                target->SetObjectScale(cInfo->Scale * target->GetObjectScaleMod());
        }

        if (!skipDisplayUpdate)
            target->SetDisplayId(m_modifier.m_amount);
    }
    else                                                    // !apply
    {
        // ApplyModifier(true) will reapply it if need
        target->RestoreDisplayId();

        // apply default equipment for creature case
        if (target->GetTypeId() == TYPEID_UNIT)
            ((Creature*)target)->LoadEquipment(((Creature*)target)->GetCreatureInfo()->EquipmentTemplateId, true);
    }
}

void Aura::HandleForceReaction(bool apply, bool Real)
{
    if (GetTarget()->GetTypeId() != TYPEID_PLAYER)
        return;

    if (!Real)
        return;

    Player* player = (Player*)GetTarget();

    uint32 faction_id = m_modifier.m_miscvalue;
    ReputationRank faction_rank = ReputationRank(m_modifier.m_amount);

    player->GetReputationMgr().ApplyForceReaction(faction_id, faction_rank, apply);
    player->GetReputationMgr().SendForceReactions();

    // stop fighting if at apply forced rank friendly or at remove real rank friendly
    if ((apply && faction_rank >= REP_FRIENDLY) || (!apply && player->GetReputationRank(faction_id) >= REP_FRIENDLY))
        player->StopAttackFaction(faction_id);
}

void Aura::HandleAuraModSkill(bool apply, bool /*Real*/)
{
    if (GetTarget()->GetTypeId() != TYPEID_PLAYER)
        return;

    Player* target = static_cast<Player*>(GetTarget());

    Modifier const* mod = GetModifier();
    const uint16 skillId = uint16(GetSpellProto()->EffectMiscValue[m_effIndex]);
    const int16 amount = int16(mod->m_amount);
    const bool permanent = (mod->m_auraname == SPELL_AURA_MOD_SKILL_TALENT);

    target->ModifySkillBonus(skillId, (apply ? amount : -amount), permanent);
}

void Aura::HandleChannelDeathItem(bool apply, bool Real)
{
    if (Real && !apply)
    {
        if (m_removeMode != AURA_REMOVE_BY_DEATH)
            return;
        // Item amount
        if (m_modifier.m_amount <= 0)
            return;

        SpellEntry const* spellInfo = GetSpellProto();
        if (spellInfo->EffectItemType[m_effIndex] == 0)
            return;

        Unit* victim = GetTarget();
        Unit* caster = GetCaster();
        if (!caster || caster->GetTypeId() != TYPEID_PLAYER)
            return;

        uint32 itemType = spellInfo->EffectItemType[m_effIndex];

        // Soul Shard (target req.)
        if (itemType == 6265)
        {
            // Only from non-grey units
            if (!((Player*)caster)->isHonorOrXPTarget(victim))
                return;
            // Only if the creature is tapped by the player or his group
            if (victim->GetTypeId() == TYPEID_UNIT && !((Creature*)victim)->IsTappedBy((Player*)caster))
                return;
            // Avoid awarding multiple souls on the same target
            // 1.11.0: If you cast Drain Soul while shadowburn is on the victim, you will no longer receive two soul shards upon the victim's death.
            for (auto const& aura : victim->GetAurasByType(SPELL_AURA_CHANNEL_DEATH_ITEM))
                if (aura != this && caster->GetObjectGuid() == aura->GetCasterGuid() && aura->GetSpellProto()->EffectItemType[aura->GetEffIndex()] == itemType)
                    return;
        }

        // Adding items
        uint32 noSpaceForCount = 0;
        uint32 count = m_modifier.m_amount;

        ItemPosCountVec dest;
        InventoryResult msg = ((Player*)caster)->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, itemType, count, &noSpaceForCount);
        if (msg != EQUIP_ERR_OK)
        {
            count -= noSpaceForCount;
            ((Player*)caster)->SendEquipError(msg, nullptr, nullptr, itemType);
            if (count == 0)
                return;
        }

        Item* newitem = ((Player*)caster)->StoreNewItem(dest, itemType, true);
        ((Player*)caster)->SendNewItem(newitem, count, true, true);
    }
}

void Aura::HandleBindSight(bool apply, bool /*Real*/)
{
    Unit* caster = GetCaster();
    if (!caster || caster->GetTypeId() != TYPEID_PLAYER)
        return;

    Camera& camera = ((Player*)caster)->GetCamera();
    if (apply)
        camera.SetView(GetTarget());
    else
        camera.ResetView();
}

void Aura::HandleFarSight(bool apply, bool /*Real*/)
{
    Unit* caster = GetCaster();
    if (!caster || caster->GetTypeId() != TYPEID_PLAYER)
        return;

    Camera& camera = ((Player*)caster)->GetCamera();
    if (apply)
        camera.SetView(GetTarget());
    else
        camera.ResetView();
}

void Aura::HandleAuraTrackCreatures(bool apply, bool /*Real*/)
{
    if (GetTarget()->GetTypeId() != TYPEID_PLAYER)
        return;

    if (apply)
        GetTarget()->SetFlag(PLAYER_TRACK_CREATURES, uint32(1) << (m_modifier.m_miscvalue - 1));
    else
        GetTarget()->RemoveFlag(PLAYER_TRACK_CREATURES, uint32(1) << (m_modifier.m_miscvalue - 1));
}

void Aura::HandleAuraTrackResources(bool apply, bool /*Real*/)
{
    if (GetTarget()->GetTypeId() != TYPEID_PLAYER)
        return;

    if (apply)
        GetTarget()->SetFlag(PLAYER_TRACK_RESOURCES, uint32(1) << (m_modifier.m_miscvalue - 1));
    else
        GetTarget()->RemoveFlag(PLAYER_TRACK_RESOURCES, uint32(1) << (m_modifier.m_miscvalue - 1));
}

void Aura::HandleAuraTrackStealthed(bool apply, bool /*Real*/)
{
    if (GetTarget()->GetTypeId() != TYPEID_PLAYER)
        return;

    GetTarget()->ApplyModByteFlag(PLAYER_FIELD_BYTES, 0, PLAYER_FIELD_BYTE_TRACK_STEALTHED, apply);
}

void Aura::HandleAuraModScale(bool apply, bool /*Real*/)
{
    GetTarget()->ApplyPercentModFloatValue(OBJECT_FIELD_SCALE_X, float(m_modifier.m_amount), apply);
    GetTarget()->UpdateModelData();
}

void Aura::HandleModPossess(bool apply, bool Real)
{
    if (!Real)
        return;

    Unit* target = GetTarget();

    // not possess yourself
    if (GetCasterGuid() == target->GetObjectGuid())
        return;

    Unit* caster = GetCaster();
    if (!caster || caster->GetTypeId() != TYPEID_PLAYER) // TODO:: well i know some bosses can take control of player???
        return;

    if (apply)
    {
        if (const SpellEntry* spellInfo = GetSpellProto())
        {
            switch (spellInfo->Id)
            {
                case 24937: // Using Control Console
                {
                    static const uint32 spells[] = { 25026, 25024, 27759, 25027 };
                    if (target->GetTypeId() == TYPEID_UNIT)
                        static_cast<Creature*>(target)->UpdateSpell(5, spells[urand(0, 3)]);
                }
                break;
            }
        }

        // Possess: advertised type of charm (unique) - remove existing advertised charm
        caster->BreakCharmOutgoing(true);

        caster->TakePossessOf(target);

        if (Player* playerCaster = caster->IsPlayer() ? static_cast<Player*>(caster) : nullptr)
        {
            UpdateData newData;
            target->BuildValuesUpdateBlockForPlayerWithFlags(newData, playerCaster, UF_FLAG_OWNER_ONLY);
            if (newData.HasData())
            {
                WorldPacket newDataPacket = newData.BuildPacket(0, false);
                playerCaster->SendDirectMessage(newDataPacket);
            }
        }
    }
    else
        caster->Uncharm(target);

    if (const SpellEntry* spellInfo = GetSpellProto())
    {
        switch (spellInfo->Id)
        {
            case 24937: // Using Control Console
                if (!apply)
                {
                    caster->CastSpell(caster, 24938, TRIGGERED_OLD_TRIGGERED);      // Close Control Console
                    caster->CastSpell(caster, 27880, TRIGGERED_OLD_TRIGGERED);      // Stun
                    if (target->GetTypeId() == TYPEID_UNIT)
                    {
                        target->CastSpell(target, 3617, TRIGGERED_OLD_TRIGGERED);   // Quiet Suicide
                        static_cast<Creature*>(target)->ForcedDespawn(5000);
                    }
                }
                break;
        }
    }
}

void Aura::HandleModPossessPet(bool apply, bool Real)
{
    if (!Real)
        return;

    Unit* caster = GetCaster();
    if (!caster || caster->GetTypeId() != TYPEID_PLAYER)
        return;

    Unit* target = GetTarget();
    if (target->GetTypeId() != TYPEID_UNIT || !static_cast<Creature*>(target)->IsPet())
        return;

    if (apply)
    {
        // Possess pet: advertised type of charm (unique) - remove existing advertised charm
        caster->BreakCharmOutgoing(true);

        caster->TakePossessOf(target);
    }
    else
        caster->Uncharm(target);
}

void Aura::HandleModCharm(bool apply, bool Real)
{
    if (!Real)
        return;

    Unit* target = GetTarget();

    // not charm yourself
    if (GetCasterGuid() == target->GetObjectGuid())
        return;

    Unit* caster = GetCaster();
    if (!caster)
        return;

    Player* playerCaster = caster->GetTypeId() == TYPEID_PLAYER ? static_cast<Player*>(caster) : nullptr;

    if (apply)
    {
        // Charm: normally advertised type of charm (unique), but with notable exceptions:
        // * Seems to be non-unique for NPCs - allows overwriting advertised charm by offloading existing one (e.g. Chromatic Mutation)
        // * Seems to be always unique for players - remove player's existing advertised charm (no evidence against this found yet)
        if (playerCaster)
            caster->BreakCharmOutgoing(true);

        caster->TakeCharmOf(target, GetId());

        if (playerCaster)
        {
            UpdateMask updateMask;
            updateMask.SetCount(target->GetValuesCount());
            target->MarkUpdateFieldsWithFlagForUpdate(updateMask, UF_FLAG_OWNER_ONLY);
            if (updateMask.HasData())
            {
                UpdateData newData;
                target->BuildValuesUpdateBlockForPlayer(newData, updateMask, playerCaster);

                if (newData.HasData())
                {
                    WorldPacket newDataPacket = newData.BuildPacket(0, false);
                    playerCaster->SendDirectMessage(newDataPacket);
                }
            }
        }
    }
    else
        caster->Uncharm(target, GetId());
}


void Aura::HandleAoECharm(bool apply, bool Real)
{
    if (!Real)
        return;

    Unit* target = GetTarget();

    // not charm yourself
    if (GetCasterGuid() == target->GetObjectGuid())
        return;

    Unit* caster = GetCaster();
    if (!caster)
        return;

    if (apply)
        // AoE charm: non-advertised type of charm - co-exists with other charms
        caster->TakeCharmOf(target, GetId(), false);
    else
        caster->Uncharm(target, GetId());
}

void Aura::HandleModConfuse(bool apply, bool Real)
{
    if (!Real)
        return;

    // Do not remove it yet if more effects are up, do it for the last effect
    if (!apply && GetTarget()->HasAuraType(SPELL_AURA_MOD_CONFUSE))
        return;

    GetTarget()->SetConfused(apply, GetCasterGuid(), GetId());

    GetTarget()->getHostileRefManager().HandleSuppressed(apply);
}

void Aura::HandleModFear(bool apply, bool Real)
{
    if (!Real)
        return;

    // Do not remove it yet if more effects are up, do it for the last effect
    if (!apply && GetTarget()->HasAuraType(SPELL_AURA_MOD_FEAR))
        return;

    GetTarget()->SetFleeing(apply, GetCasterGuid(), GetId());

    GetTarget()->getHostileRefManager().HandleSuppressed(apply);
}

void Aura::HandleFeignDeath(bool apply, bool Real)
{
    if (!Real)
        return;

    Unit* target = GetTarget();

    // Do not remove it yet if more effects are up, do it for the last effect
    if (!apply && target->HasAuraType(SPELL_AURA_FEIGN_DEATH))
        return;

    if (apply)
    {
        bool success = true;

        if (target->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED))
        {
            // Players and player-controlled units do an additional success roll for this aura on application
            const SpellEntry* entry = GetSpellProto();
            const SpellSchoolMask schoolMask = GetSpellSchoolMask(entry);

            float resist = 0;

            for (auto attacker : target->getAttackers())
            {
                if (attacker && !attacker->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED))
                {
                    const float chance = target->CalculateSpellMissChance(attacker, schoolMask, entry);
                    resist = ((chance > resist) ? chance : resist);
                }
            }

            success = !roll_chance_f(resist);
        }

        if (success)
            target->InterruptSpellsCastedOnMe();

        target->SetFeignDeath(apply, GetCasterGuid(), GetId(), true, success);
    }
    else
        target->SetFeignDeath(false);
}

void Aura::HandleAuraModDisarm(bool apply, bool Real)
{
    if (!Real)
        return;

    Unit* target = GetTarget();

    if (!apply && target->HasAuraType(GetModifier()->m_auraname))
        return;

    target->ApplyModFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISARMED, apply);

    // main-hand attack speed already set to special value for feral form already and don't must change and reset at remove.
    if (target->IsNoWeaponShapeShift())
        return;

    if (target->GetTypeId() == TYPEID_PLAYER)
    {
        if (apply)
            target->SetAttackTime(BASE_ATTACK, BASE_ATTACK_TIME);
        else
            ((Player*)target)->SetRegularAttackTime();
    }

    target->UpdateDamagePhysical(BASE_ATTACK);
}

void Aura::HandleAuraModStun(bool apply, bool Real)
{
    if (!Real)
        return;

    Unit* target = GetTarget();

    if (apply)
    {
        // Frost stun aura -> freeze/unfreeze target
        if (GetSpellSchoolMask(GetSpellProto()) & SPELL_SCHOOL_MASK_FROST)
            target->ModifyAuraState(AURA_STATE_FROZEN, apply);

        // Creature specific
        if (target->IsInWorld() && !target->IsPlayer() && !target->IsStunned())
            target->SetFacingTo(target->GetOrientation());

        Unit* caster = GetCaster();
        target->SetStunned(true, (caster ? caster->GetObjectGuid() : ObjectGuid()), GetSpellProto()->Id);

        if (caster)
            if (UnitAI* ai = caster->AI())
                ai->JustStunnedTarget(GetSpellProto(), target);

        if (GetSpellProto()->AuraInterruptFlags & AURA_INTERRUPT_FLAG_DAMAGE)
            target->getHostileRefManager().HandleSuppressed(apply);
    }
    else
    {
        // Frost stun aura -> freeze/unfreeze target
        if (GetSpellSchoolMask(GetSpellProto()) & SPELL_SCHOOL_MASK_FROST)
        {
            bool found_another = false;
            for (AuraType const* itr = &frozenAuraTypes[0]; *itr != SPELL_AURA_NONE; ++itr)
            {
                Unit::AuraList const& auras = target->GetAurasByType(*itr);
                for (auto aura : auras)
                {
                    if (GetSpellSchoolMask(aura->GetSpellProto()) & SPELL_SCHOOL_MASK_FROST)
                    {
                        found_another = true;
                        break;
                    }
                }
                if (found_another)
                    break;
            }

            if (!found_another)
                target->ModifyAuraState(AURA_STATE_FROZEN, apply);
        }

        if (GetSpellProto()->AuraInterruptFlags & AURA_INTERRUPT_FLAG_DAMAGE)
            target->getHostileRefManager().HandleSuppressed(apply);

        // Real remove called after current aura remove from lists, check if other similar auras active
        if (target->HasAuraType(SPELL_AURA_MOD_STUN))
            return;

        target->SetStunned(false);

        // Wyvern Sting
        if (GetSpellProto()->SpellFamilyName == SPELLFAMILY_HUNTER)
        {
            Unit* caster = GetCaster();
            if (GetSpellProto()->SpellFamilyFlags & uint64(0x00010000))
            {
                if (!caster || caster->GetTypeId() != TYPEID_PLAYER)
                    return;

                uint32 spell_id;
                switch (GetId())
                {
                    case 19386: spell_id = 24131; break;
                    case 24132: spell_id = 24134; break;
                    case 24133: spell_id = 24135; break;
                    default:
                        sLog.outError("Spell selection called for unexpected original spell %u, new spell for this spell family?", GetId());
                        return;
                }

                SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(spell_id);

                if (!spellInfo)
                    return;

                caster->CastSpell(target, spellInfo, TRIGGERED_OLD_TRIGGERED, nullptr, this);
            }
            else    // Wyvern Sting for bosses
            {
                if (!caster)
                    return;

                switch (GetId())
                {
                    case 26180:     // Wyvern Sting (Princess Huhuran)
                    {
                        int32 natureDamage = (m_removeMode == AURA_REMOVE_BY_DISPEL) ? 3000 : 500;  // Deal 500 nature damage on spell expire but 3000 if dispelled
                        GetCaster()->CastCustomSpell(target, 26233, &natureDamage, nullptr, nullptr, TRIGGERED_OLD_TRIGGERED);
                        return;
                    }
                    default:
                        sLog.outError("Spell selection called for unexpected original spell %u, new spell for this spell family?", GetId());
                        return;
                }
            }
        }
    }
}

void Aura::HandleModStealth(bool apply, bool Real)
{
    Unit* target = GetTarget();
    // TODO: add mask
    target->GetVisibilityData().AddStealthStrength(StealthType(m_modifier.m_miscvalue), apply ? m_modifier.m_amount : -m_modifier.m_amount);

    if (apply)
    {
        // drop flag at stealth in bg
        target->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_STEALTH_INVIS_CANCELS);

        // only at real aura add
        if (Real)
        {
            target->SetVisFlags(UNIT_VIS_FLAG_CREEP);

            if (target->GetTypeId() == TYPEID_PLAYER)
                target->SetByteFlag(PLAYER_FIELD_BYTES2, 1, PLAYER_FIELD_BYTE2_STEALTH);

            // apply only if not in GM invisibility (and overwrite invisibility state)
            if (target->GetVisibility() != VISIBILITY_OFF)
            {
                target->SetVisibility(VISIBILITY_GROUP_NO_DETECT);
                target->SetVisibility(VISIBILITY_GROUP_STEALTH);
            }
        }
    }
    else
    {
        // only at real aura remove of _last_ SPELL_AURA_MOD_STEALTH
        if (Real && !target->HasAuraType(SPELL_AURA_MOD_STEALTH))
        {
            // if no GM invisibility
            if (target->GetVisibility() != VISIBILITY_OFF)
            {
                target->RemoveVisFlags(UNIT_VIS_FLAG_CREEP);

                if (target->GetTypeId() == TYPEID_PLAYER)
                    target->RemoveByteFlag(PLAYER_FIELD_BYTES2, 1, PLAYER_FIELD_BYTE2_STEALTH);

                // restore invisibility if any
                if (target->HasAuraType(SPELL_AURA_MOD_INVISIBILITY))
                {
                    target->SetVisibility(VISIBILITY_GROUP_NO_DETECT);
                    target->SetVisibility(VISIBILITY_GROUP_INVISIBILITY);
                }
                else
                    target->SetVisibility(VISIBILITY_ON);
            }
        }
    }
}

void Aura::HandleModStealthDetect(bool apply, bool Real)
{
    GetTarget()->GetVisibilityData().AddStealthDetectionStrength(StealthType(m_modifier.m_miscvalue), apply ? m_modifier.m_amount : -m_modifier.m_amount);
}

void Aura::HandleModStealthLevel(bool apply, bool Real)
{
    GetTarget()->GetVisibilityData().AddStealthStrength(StealthType(m_modifier.m_miscvalue), apply ? m_modifier.m_amount : -m_modifier.m_amount);
}

void Aura::HandleInvisibility(bool apply, bool Real)
{
    Unit* target = GetTarget();

    target->GetVisibilityData().AddInvisibilityValue(m_modifier.m_miscvalue, apply ? m_modifier.m_amount : -m_modifier.m_amount);
    int32 value = target->GetVisibilityData().GetInvisibilityValue(m_modifier.m_miscvalue);
    bool trueApply = value > 0;
    target->GetVisibilityData().SetInvisibilityMask(m_modifier.m_miscvalue, trueApply);
    if (trueApply)
    {
        target->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_STEALTH_INVIS_CANCELS);

        if (Real && target->GetTypeId() == TYPEID_PLAYER)
        {
            if (Player* player = (Player*)target)
            {
                if (player->GetMover() == nullptr) // check if the player doesnt have a mover, when player is hidden during MC of creature
                {
                    // apply glow vision
                    target->SetByteFlag(PLAYER_FIELD_BYTES2, 1, PLAYER_FIELD_BYTE2_INVISIBILITY_GLOW);
                }
            }
        }

        // apply only if not in GM invisibility and not stealth
        if (target->GetVisibility() == VISIBILITY_ON)
            target->SetVisibilityWithoutUpdate(VISIBILITY_GROUP_INVISIBILITY);
    }
    else
    {
        // only at real aura remove and if not have different invisibility auras.
        if (Real && target->GetVisibilityData().GetInvisibilityMask() == 0)
        {
            // remove glow vision
            if (target->GetTypeId() == TYPEID_PLAYER)
                target->RemoveByteFlag(PLAYER_FIELD_BYTES2, 1, PLAYER_FIELD_BYTE2_INVISIBILITY_GLOW);

            // apply only if not in GM invisibility & not stealthed while invisible
            if (target->GetVisibility() != VISIBILITY_OFF)
            {
                // if have stealth aura then already have stealth visibility
                if (!target->HasAuraType(SPELL_AURA_MOD_STEALTH))
                    target->SetVisibilityWithoutUpdate(VISIBILITY_ON);
            }
        }
    }

    if (target->IsInWorld())
        target->UpdateVisibilityAndView();
}

void Aura::HandleInvisibilityDetect(bool apply, bool Real)
{
    // In Classic: multiple invisibility detection auras can apply at the same type but only the highest rank one for a given invisibility type (mask) is used
    Unit* target = GetTarget();
    int32 detectValue = m_modifier.m_amount;
    // Aura appliance
    if (apply)
    {
        // Update invisibility detection value for this invisibility mask only if new value if higher than current
        // to prevent lower detection ranks from erasing higher ranks
        target->GetVisibilityData().SetInvisibilityDetectMask(m_modifier.m_miscvalue, apply);
        if (target->GetVisibilityData().GetInvisibilityDetectValue(m_modifier.m_miscvalue) <= detectValue)
            target->GetVisibilityData().SetInvisibilityDetectValue(m_modifier.m_miscvalue, apply ? m_modifier.m_amount : -m_modifier.m_amount);
    }
    // Aura removal
    else
    {
        // Check if other auras for the same invisibility mask exist and update invisibility detection value to the highest one
        int32 maxDetectValue = 0;
        Unit::AuraList const& auras = target->GetAurasByType(SPELL_AURA_MOD_INVISIBILITY_DETECTION);
        for (auto aura : auras)
        {
            if (aura->GetModifier()->m_miscvalue == m_modifier.m_miscvalue
                    && aura->GetModifier()->m_amount != detectValue
                    && aura->GetModifier()->m_amount > maxDetectValue)
                maxDetectValue = aura->GetModifier()->m_amount;
        }

        if (maxDetectValue)
            target->GetVisibilityData().SetInvisibilityDetectValue(m_modifier.m_miscvalue, maxDetectValue);
        // No other aura: remove invisibility detection for this invisibility mask
        else
        {
            target->GetVisibilityData().SetInvisibilityDetectMask(m_modifier.m_miscvalue, false);
            target->GetVisibilityData().SetInvisibilityDetectValue(m_modifier.m_miscvalue, 0);
        }
    }

    if (Real && target->GetTypeId() == TYPEID_PLAYER)
        ((Player*)target)->GetCamera().UpdateVisibilityForOwner();
}

void Aura::HandleDetectAmore(bool apply, bool /*real*/)
{
    if (!GetTarget()->IsPlayer())
        return;

    GetTarget()->ApplyModByteFlag(PLAYER_FIELD_BYTES2, 1, 1 << (GetMiscValue() - 1), apply);
}

void Aura::HandleAuraModRoot(bool apply, bool Real)
{
    // only at real add/remove aura
    if (!Real)
        return;

    Unit* target = GetTarget();

    if (apply)
    {
        // Frost root aura -> freeze/unfreeze target
        if (GetSpellSchoolMask(GetSpellProto()) & SPELL_SCHOOL_MASK_FROST)
            target->ModifyAuraState(AURA_STATE_FROZEN, apply);

        if (Unit* caster = GetCaster())
            if (UnitAI* ai = caster->AI())
                ai->JustRootedTarget(GetSpellProto(), target);
    }
    else
    {
        // Frost root aura -> freeze/unfreeze target
        if (GetSpellSchoolMask(GetSpellProto()) & SPELL_SCHOOL_MASK_FROST)
        {
            bool found_another = false;
            for (AuraType const* itr = &frozenAuraTypes[0]; *itr != SPELL_AURA_NONE; ++itr)
            {
                Unit::AuraList const& auras = target->GetAurasByType(*itr);
                for (auto aura : auras)
                {
                    if (GetSpellSchoolMask(aura->GetSpellProto()) & SPELL_SCHOOL_MASK_FROST)
                    {
                        found_another = true;
                        break;
                    }
                }
                if (found_another)
                    break;
            }

            if (!found_another)
                target->ModifyAuraState(AURA_STATE_FROZEN, apply);
        }

        // Real remove called after current aura remove from lists, check if other similar auras active
        if (target->HasAuraType(SPELL_AURA_MOD_ROOT))
            return;
    }

    target->SetImmobilizedState(apply);
}

void Aura::HandleAuraModSilence(bool apply, bool Real)
{
    // only at real add/remove aura
    if (!Real)
        return;

    Unit* target = GetTarget();

    if (apply)
    {
        target->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SILENCED);
        // Stop cast only spells vs PreventionType == SPELL_PREVENTION_TYPE_SILENCE
        for (uint32 i = CURRENT_MELEE_SPELL; i < CURRENT_MAX_SPELL; ++i)
            if (Spell* spell = target->GetCurrentSpell(CurrentSpellTypes(i)))
                if (spell->m_spellInfo->PreventionType == SPELL_PREVENTION_TYPE_SILENCE)
                    // Stop spells on prepare or casting state
                    target->InterruptSpell(CurrentSpellTypes(i), false);
    }
    else
    {
        // Real remove called after current aura remove from lists, check if other similar auras active
        if (target->HasAuraType(SPELL_AURA_MOD_SILENCE))
            return;

        target->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SILENCED);
    }
}

void Aura::HandleModThreat(bool apply, bool Real)
{
    // only at real add/remove aura
    if (!Real)
        return;

    Unit* target = GetTarget();

    if (!target->IsAlive())
        return;

    int level_diff = 0;
    int multiplier = 0;
    switch (GetId())
    {
        // Arcane Shroud
        case 26400:
            level_diff = target->GetLevel() - 60;
            multiplier = 2;
            break;
        // The Eye of Diminution
        case 28862:
            level_diff = target->GetLevel() - 60;
            multiplier = 1;
            break;
    }

    if (level_diff > 0)
        m_modifier.m_amount += multiplier * level_diff;

    for (int8 x = 0; x < MAX_SPELL_SCHOOL; ++x)
        if (m_modifier.m_miscvalue & int32(1 << x))
            ApplyPercentModFloatVar(target->m_threatModifier[x], float(m_modifier.m_amount), apply);
}

void Aura::HandleAuraModTotalThreat(bool apply, bool Real)
{
    // only at real add/remove aura
    if (!Real)
        return;

    Unit* target = GetTarget();

    if (!target->IsAlive() || target->GetTypeId() != TYPEID_PLAYER)
        return;

    Unit* caster = GetCaster();

    if (!caster || !caster->IsAlive())
        return;

    target->getHostileRefManager().threatTemporaryFade(caster, m_modifier.m_amount, apply);
}

void Aura::HandleModTaunt(bool /*apply*/, bool Real)
{
    // only at real add/remove aura
    if (!Real)
        return;

    Unit* target = GetTarget();

    if (!target->IsAlive() || !target->CanHaveThreatList())
        return;

    target->TauntUpdate();

    if (target->AI())
        target->AI()->OnTaunt();
}

/*********************************************************/
/***                  MODIFY SPEED                     ***/
/*********************************************************/
void Aura::HandleAuraModIncreaseSpeed(bool /*apply*/, bool Real)
{
    // all applied/removed only at real aura add/remove
    if (!Real)
        return;

    GetTarget()->UpdateSpeed(MOVE_RUN, true);
}

void Aura::HandleAuraModIncreaseMountedSpeed(bool /*apply*/, bool Real)
{
    // all applied/removed only at real aura add/remove
    if (!Real)
        return;

    GetTarget()->UpdateSpeed(MOVE_RUN, true);
}

void Aura::HandleAuraModIncreaseSwimSpeed(bool /*apply*/, bool Real)
{
    // all applied/removed only at real aura add/remove
    if (!Real)
        return;

    GetTarget()->UpdateSpeed(MOVE_SWIM, true);
}

void Aura::HandleAuraModDecreaseSpeed(bool /*apply*/, bool Real)
{
    // all applied/removed only at real aura add/remove
    if (!Real)
        return;

    Unit* target = GetTarget();

    target->UpdateSpeed(MOVE_RUN, true);
    target->UpdateSpeed(MOVE_RUN_BACK, true);
    target->UpdateSpeed(MOVE_SWIM, true);
}

void Aura::HandleAuraModUseNormalSpeed(bool /*apply*/, bool Real)
{
    // all applied/removed only at real aura add/remove
    if (!Real)
        return;

    Unit* target = GetTarget();

    target->UpdateSpeed(MOVE_RUN, true);
    target->UpdateSpeed(MOVE_SWIM, true);
}

/*********************************************************/
/***                     IMMUNITY                      ***/
/*********************************************************/

void Aura::HandleModMechanicImmunity(bool apply, bool /*Real*/)
{
    uint32 misc  = m_modifier.m_miscvalue;
    Unit* target = GetTarget();

    if (apply && GetSpellProto()->HasAttribute(SPELL_ATTR_EX_IMMUNITY_PURGES_EFFECT))
    {
        uint32 mechanic = 1 << (misc - 1);

        target->RemoveAurasAtMechanicImmunity(mechanic, GetId());
    }

    target->ApplySpellImmune(this, IMMUNITY_MECHANIC, misc, apply);
}

void Aura::HandleModMechanicImmunityMask(bool apply, bool /*Real*/)
{
    uint32 mechanic  = m_modifier.m_miscvalue;

    if (apply && GetSpellProto()->HasAttribute(SPELL_ATTR_EX_IMMUNITY_PURGES_EFFECT))
        GetTarget()->RemoveAurasAtMechanicImmunity(mechanic, GetId());

    // check implemented in Unit::IsImmuneToSpell and Unit::IsImmuneToSpellEffect
}

// this method is called whenever we add / remove aura which gives m_target some imunity to some spell effect
void Aura::HandleAuraModEffectImmunity(bool apply, bool /*Real*/)
{
    Unit* target = GetTarget();

    target->ApplySpellImmune(this, IMMUNITY_EFFECT, m_modifier.m_miscvalue, apply);

    if (apply && IsPositive())
        target->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_INVULNERABILITY_BUFF_CANCELS, GetHolder());
}

void Aura::HandleAuraModStateImmunity(bool apply, bool Real)
{
    if (apply && Real && GetSpellProto()->HasAttribute(SPELL_ATTR_EX_IMMUNITY_PURGES_EFFECT))
    {
        Unit::AuraList const& auraList = GetTarget()->GetAurasByType(AuraType(m_modifier.m_miscvalue));
        for (Unit::AuraList::const_iterator itr = auraList.begin(); itr != auraList.end();)
        {
            if (auraList.front() != this)                   // skip itself aura (it already added)
            {
                GetTarget()->RemoveAurasDueToSpell(auraList.front()->GetId());
                itr = auraList.begin();
            }
            else
                ++itr;
        }
    }

    GetTarget()->ApplySpellImmune(this, IMMUNITY_STATE, m_modifier.m_miscvalue, apply);
}

void Aura::HandleAuraModSchoolImmunity(bool apply, bool Real)
{
    Unit* target = GetTarget();
    target->ApplySpellImmune(this, IMMUNITY_SCHOOL, m_modifier.m_miscvalue, apply);

    // remove all flag auras (they are positive, but they must be removed when you are immune)
    if (GetSpellProto()->HasAttribute(SPELL_ATTR_EX_IMMUNITY_PURGES_EFFECT))
        target->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_INVULNERABILITY_BUFF_CANCELS);

    // TODO: optimalize this cycle - use RemoveAurasWithInterruptFlags call or something else
    if (Real && apply
            && GetSpellProto()->HasAttribute(SPELL_ATTR_EX_IMMUNITY_PURGES_EFFECT)
            && IsPositiveSpell(GetId(), GetCaster(), target))                    // Only positive immunity removes auras
    {
        uint32 school_mask = m_modifier.m_miscvalue;
        Unit::SpellAuraHolderMap& Auras = target->GetSpellAuraHolderMap();
        for (Unit::SpellAuraHolderMap::iterator iter = Auras.begin(), next; iter != Auras.end(); iter = next)
        {
            next = iter;
            ++next;
            SpellEntry const* spell = iter->second->GetSpellProto();
            if ((GetSpellSchoolMask(spell) & school_mask)   // Check for school mask
                    && !spell->HasAttribute(SPELL_ATTR_NO_IMMUNITIES)   // Spells unaffected by invulnerability
                    && !iter->second->IsPositive()          // Don't remove positive spells
                    && spell->Id != GetId())                // Don't remove self
            {
                target->RemoveAurasDueToSpell(spell->Id);
                if (Auras.empty())
                    break;
                next = Auras.begin();
            }
        }
    }
    if (Real && GetSpellProto()->Mechanic == MECHANIC_BANISH)
    {
        if (apply)
            target->addUnitState(UNIT_STAT_ISOLATED);
        else
            target->clearUnitState(UNIT_STAT_ISOLATED);
    }

    GetTarget()->getHostileRefManager().HandleSuppressed(apply, true);

    if (apply)
        target->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE);
    else
    {
        // do not remove unit flag if there are more than this auraEffect of that kind on unit
        if (target->HasAuraType(SPELL_AURA_SCHOOL_IMMUNITY))
            return;
        target->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE);
    }
}

void Aura::HandleAuraModDmgImmunity(bool apply, bool /*Real*/)
{
    GetTarget()->ApplySpellImmune(this, IMMUNITY_DAMAGE, m_modifier.m_miscvalue, apply);

    GetTarget()->getHostileRefManager().HandleSuppressed(apply, true);
}

void Aura::HandleAuraModDispelImmunity(bool apply, bool Real)
{
    // all applied/removed only at real aura add/remove
    if (!Real)
        return;

    GetTarget()->ApplySpellDispelImmunity(this, DispelType(m_modifier.m_miscvalue), apply);
}

void Aura::HandleAuraProcTriggerSpell(bool apply, bool Real)
{
    if (!Real)
        return;

    switch (GetId())
    {
        // some spell have charges by functionality not have its in spell data
        case 28200:                                         // Ascendance (Talisman of Ascendance trinket)
            if (apply)
                GetHolder()->SetAuraCharges(6);
            break;
        default:
            break;
    }
}

void Aura::HandleAuraModStalked(bool apply, bool /*Real*/)
{
    // used by spells: Hunter's Mark, Mind Vision, Syndicate Tracker (MURP) DND
    if (apply)
        GetTarget()->SetFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_TRACK_UNIT);
    else
        GetTarget()->RemoveFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_TRACK_UNIT);
}

/*********************************************************/
/***                   PERIODIC                        ***/
/*********************************************************/

void Aura::HandlePeriodicTriggerSpell(bool apply, bool /*Real*/)
{
    m_isPeriodic = apply;

    Unit* target = GetTarget();

    if (!apply)
    {
        switch (GetId())
        {
            case 18173:                                     // Burning Adrenaline (Main Target version)
            case 23620:                                     // Burning Adrenaline (Caster version)
                // On aura removal, the target deals AoE damage to friendlies and kills himself/herself (prevent durability loss)
                target->CastSpell(target, 23478, TRIGGERED_OLD_TRIGGERED, nullptr, this);
                target->CastSpell(target, 23644, TRIGGERED_OLD_TRIGGERED, nullptr, this);
                return;
            case 29213:                                     // Curse of the Plaguebringer
                if (m_removeMode != AURA_REMOVE_BY_DISPEL)
                    // Cast Wrath of the Plaguebringer if not dispelled
                    target->CastSpell(target, 29214, TRIGGERED_OLD_TRIGGERED, 0, this);
                return;
            default:
                break;
        }
    }
}

void Aura::HandlePeriodicTriggerSpellWithValue(bool apply, bool /*Real*/)
{
    m_isPeriodic = apply;
}

void Aura::HandlePeriodicEnergize(bool apply, bool /*Real*/)
{
    m_isPeriodic = apply;
}

void Aura::HandleAuraPowerBurn(bool apply, bool /*Real*/)
{
    m_isPeriodic = apply;
}

void Aura::HandlePeriodicHeal(bool apply, bool /*Real*/)
{
    m_isPeriodic = apply;

    Unit* target = GetTarget();

    // For prevent double apply bonuses
    bool loading = (target->GetTypeId() == TYPEID_PLAYER && ((Player*)target)->GetSession()->PlayerLoading());

    // Custom damage calculation after
    if (apply)
    {
        if (loading)
            return;

        Unit* caster = GetCaster();
        if (!caster)
            return;

        switch (GetSpellProto()->Id)
        {
            case 12939: m_modifier.m_amount = target->GetMaxHealth() / 10; break; // Polymorph Heal Effect
            default: m_modifier.m_amount = caster->SpellHealingBonusDone(target, GetSpellProto(), GetEffIndex(), m_modifier.m_amount, DOT, GetStackAmount()); break;
        }
    }
}

void Aura::HandlePeriodicDamage(bool apply, bool Real)
{
    // spells required only Real aura add/remove
    if (!Real)
        return;

    m_isPeriodic = apply;

    Unit* target = GetTarget();
    SpellEntry const* spellProto = GetSpellProto();

    // For prevent double apply bonuses
    bool loading = (target->GetTypeId() == TYPEID_PLAYER && ((Player*)target)->GetSession()->PlayerLoading());

    // Custom damage calculation after
    if (apply)
    {
        if (loading)
            return;

        Unit* caster = GetCaster();
        if (!caster)
            return;

        switch (spellProto->SpellFamilyName)
        {
            case SPELLFAMILY_DRUID:
            {
                // Rip
                if (spellProto->SpellFamilyFlags & uint64(0x000000000000800000))
                {
                    // $AP * min(0.06*$cp, 0.24)/6 [Yes, there is no difference, whether 4 or 5 CPs are being used]
                    if (caster->GetTypeId() == TYPEID_PLAYER)
                    {
                        uint8 cp = caster->GetComboPoints();

                        if (cp > 4) cp = 4;
                        m_modifier.m_amount += int32(caster->GetTotalAttackPowerValue(BASE_ATTACK) * cp / 100);
                    }
                }
                break;
            }
            case SPELLFAMILY_ROGUE:
            {
                // Rupture
                if (spellProto->SpellFamilyFlags & uint64(0x000000000000100000))
                {
                    if (caster->GetTypeId() != TYPEID_PLAYER)
                        break;
                    // Dmg/tick = $AP*min(0.01*$cp, 0.03) [Like Rip: only the first three CP increase the contribution from AP]
                    uint8 cp = caster->GetComboPoints();
                    if (cp > 3) cp = 3;
                    m_modifier.m_amount += int32(caster->GetTotalAttackPowerValue(BASE_ATTACK) * cp / 100);
                }
                break;
            }
            default:
                break;
        }

        if (m_modifier.m_auraname == SPELL_AURA_PERIODIC_DAMAGE)
        {
            // SpellDamageBonusDone for magic spells
            if (spellProto->DmgClass == SPELL_DAMAGE_CLASS_NONE || spellProto->DmgClass == SPELL_DAMAGE_CLASS_MAGIC)
                m_modifier.m_amount = caster->SpellDamageBonusDone(target, GetSpellSchoolMask(GetSpellProto()), GetSpellProto(), GetEffIndex(), m_modifier.m_amount, DOT, GetStackAmount());
            // MeleeDamagebonusDone for weapon based spells
            else
            {
                WeaponAttackType attackType = GetWeaponAttackType(GetSpellProto());
                m_modifier.m_amount = caster->MeleeDamageBonusDone(target, m_modifier.m_amount, attackType, GetSpellSchoolMask(spellProto), spellProto, GetEffIndex(), DOT, GetStackAmount());
            }
        }
    }
    // remove time effects
    else
    {
        switch (spellProto->Id)
        {
            case 23155: // Chromaggus - Brood Affliction: Red
                if (m_removeMode == AURA_REMOVE_BY_DEATH)
                    target->CastSpell(nullptr, 23168, TRIGGERED_OLD_TRIGGERED); // Heal Chromaggus
                break;
        }
    }
}

void Aura::HandlePeriodicDamagePCT(bool apply, bool /*Real*/)
{
    m_isPeriodic = apply;
}

void Aura::HandlePeriodicLeech(bool apply, bool /*Real*/)
{
    m_isPeriodic = apply;

    // For prevent double apply bonuses
    bool loading = (GetTarget()->GetTypeId() == TYPEID_PLAYER && ((Player*)GetTarget())->GetSession()->PlayerLoading());

    // Custom damage calculation after
    if (apply)
    {
        if (loading)
            return;

        Unit* caster = GetCaster();
        if (!caster)
            return;

        m_modifier.m_amount = caster->SpellDamageBonusDone(GetTarget(), GetSpellSchoolMask(GetSpellProto()), GetSpellProto(), GetEffIndex(), m_modifier.m_amount, DOT, GetStackAmount());
    }
}

void Aura::HandlePeriodicManaLeech(bool apply, bool /*Real*/)
{
    m_isPeriodic = apply;

    if (!apply)
    {
        switch (GetId())
        {
            case 24002: // Tranquilizing Poison
            case 24003: // Tranquilizing Poison
                if (m_removeMode == AURA_REMOVE_BY_EXPIRE)
                    GetTarget()->CastSpell(nullptr, 24004, TRIGGERED_OLD_TRIGGERED); // Sleep
                break;
        }
    }
}

void Aura::HandlePeriodicHealthFunnel(bool apply, bool /*Real*/)
{
    m_isPeriodic = apply;

    // For prevent double apply bonuses
    bool loading = (GetTarget()->GetTypeId() == TYPEID_PLAYER && ((Player*)GetTarget())->GetSession()->PlayerLoading());

    // Custom damage calculation after
    if (apply)
    {
        if (loading)
            return;

        Unit* caster = GetCaster();
        if (!caster)
            return;

        m_modifier.m_amount = caster->SpellDamageBonusDone(GetTarget(), GetSpellSchoolMask(GetSpellProto()), GetSpellProto(), GetEffIndex(), m_modifier.m_amount, DOT, GetStackAmount());
    }
}

/*********************************************************/
/***                  MODIFY STATS                     ***/
/*********************************************************/

/********************************/
/***        RESISTANCE        ***/
/********************************/

void Aura::HandleAuraModResistanceExclusive(bool apply, bool /*Real*/)
{
    if (!m_modifier.m_amount)
        return;

    Unit* target = GetTarget();

    for (uint32 i = SPELL_SCHOOL_NORMAL; i < MAX_SPELL_SCHOOL; ++i)
    {
        if (m_modifier.m_miscvalue & (1 << i))
        {
            // Apply the value exclusively for each school
            const int32 amount = m_modifier.m_amount;
            int32 bonusMax = 0;
            int32 malusMax = 0;

            for (Aura* aura : target->GetAurasByType(m_modifier.m_auraname))
            {
                if (aura == this || !(aura->GetMiscValue() & (1 << i)))
                    continue;

                const int32 mod = aura->GetModifier()->m_amount;

                if (mod > bonusMax)
                    bonusMax = mod;
                else if (mod < malusMax)
                    malusMax = mod;
            }

            float oldResist = target->GetTotalResistanceValue(SpellSchools(i));
            if (amount > bonusMax)
            {
                target->HandleStatModifier(UnitMods(UNIT_MOD_RESISTANCE_START + i), TOTAL_VALUE, float(apply ? bonusMax : amount), false);
                target->HandleStatModifier(UnitMods(UNIT_MOD_RESISTANCE_START + i), TOTAL_VALUE, float(apply ? amount : bonusMax), true);
            }
            else if (amount < malusMax)
            {
                target->HandleStatModifier(UnitMods(UNIT_MOD_RESISTANCE_START + i), TOTAL_VALUE, float(apply ? malusMax : amount), false);
                target->HandleStatModifier(UnitMods(UNIT_MOD_RESISTANCE_START + i), TOTAL_VALUE, float(apply ? amount : malusMax), true);
            }
            float change = target->GetTotalResistanceValue(SpellSchools(i)) - oldResist;
            // UI malus info:
            if (target->GetTypeId() == TYPEID_PLAYER)
                static_cast<Player*>(target)->ApplyResistanceBuffModsMod(SpellSchools(i), (m_modifier.m_amount > 0), apply ? change : -change, apply);
        }
    }
}

void Aura::HandleAuraModResistance(bool apply, bool /*Real*/)
{
    if (!m_modifier.m_amount)
        return;


    Unit* target = GetTarget();

    for (uint32 i = SPELL_SCHOOL_NORMAL; i < MAX_SPELL_SCHOOL; ++i)
    {
        if (m_modifier.m_miscvalue & (1 << i))
        {
            float oldResist = target->GetTotalResistanceValue(SpellSchools(i));
            target->HandleStatModifier(UnitMods(UNIT_MOD_RESISTANCE_START + i), TOTAL_VALUE, float(m_modifier.m_amount), apply);
            float change = target->GetTotalResistanceValue(SpellSchools(i)) - oldResist;
            // UI bonus/malus info:
            if (target->GetTypeId() == TYPEID_PLAYER)
                static_cast<Player*>(target)->ApplyResistanceBuffModsMod(SpellSchools(i), (m_modifier.m_amount > 0), apply ? change : -change, apply);
        }
    }
}
void Aura::HandleAurasVisible(bool apply, bool /*Real*/)
{
    GetTarget()->ApplyModFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_AURAS_VISIBLE, apply);
}

void Aura::HandleModResistancePercent(bool apply, bool /*Real*/)
{
    if (!m_modifier.m_amount)
        return;

    Unit* target = GetTarget();

    for (uint32 i = SPELL_SCHOOL_NORMAL; i < MAX_SPELL_SCHOOL; ++i)
    {
        if (m_modifier.m_miscvalue & (1 << i))
        {
            float oldResist = target->GetTotalResistanceValue(SpellSchools(i));
            target->HandleStatModifier(UnitMods(UNIT_MOD_RESISTANCE_START + i), TOTAL_PCT, float(m_modifier.m_amount), apply);
            float change = target->GetTotalResistanceValue(SpellSchools(i)) - oldResist;
            // UI bonus/malus info:
            if (target->GetTypeId() == TYPEID_PLAYER)
            {
                static_cast<Player*>(target)->ApplyResistanceBuffModsMod(SpellSchools(i), (m_modifier.m_amount > 0), apply ? change : -change, apply);
            }
        }
    }
}

void Aura::HandleModBaseResistance(bool apply, bool /*Real*/)
{
    if (!m_modifier.m_amount)
        return;

    Unit* target = GetTarget();

    for (uint32 i = SPELL_SCHOOL_NORMAL; i < MAX_SPELL_SCHOOL; ++i)
    {
        if (m_modifier.m_miscvalue & (1 << i))
            target->HandleStatModifier(UnitMods(UNIT_MOD_RESISTANCE_START + i), BASE_VALUE, float(m_modifier.m_amount), apply);
    }
}

void Aura::HandleAuraModBaseResistancePercent(bool apply, bool /*Real*/)
{
    if (!m_modifier.m_amount)
        return;

    Unit* target = GetTarget();

    for (uint32 i = SPELL_SCHOOL_NORMAL; i < MAX_SPELL_SCHOOL; ++i)
    {
        if (m_modifier.m_miscvalue & (1 << i))
            target->HandleStatModifier(UnitMods(UNIT_MOD_RESISTANCE_START + i), BASE_PCT, float(m_modifier.m_amount), apply);
    }
}

/********************************/
/***           STAT           ***/
/********************************/

void Aura::HandleAuraModStat(bool apply, bool /*Real*/)
{
    if (m_modifier.m_miscvalue < -2 || m_modifier.m_miscvalue > 4)
    {
        sLog.outError("WARNING: Spell %u effect %u have unsupported misc value (%i) for SPELL_AURA_MOD_STAT ", GetId(), GetEffIndex(), m_modifier.m_miscvalue);
        return;
    }

    Unit* target = GetTarget();

    if (GetSpellProto()->IsFitToFamilyMask(0x0000000000008000)) // improved scorpid sting
    {
        if (apply)
        {
            int32 staminaToRemove = 0;
            Unit::AuraList const& auraClassScripts = target->GetAurasByType(SPELL_AURA_OVERRIDE_CLASS_SCRIPTS);
            for (Unit::AuraList::const_iterator itr = auraClassScripts.begin(); itr != auraClassScripts.end(); ++itr)
            {
                switch ((*itr)->GetModifier()->m_miscvalue)
                {
                    case 2388: staminaToRemove = m_modifier.m_amount * 10 / 100; break;
                    case 2389: staminaToRemove = m_modifier.m_amount * 20 / 100; break;
                    case 2390: staminaToRemove = m_modifier.m_amount * 30 / 100; break;
                }
            }
            if (staminaToRemove)
                GetCaster()->CastCustomSpell(target, 19486, &staminaToRemove, nullptr, nullptr, TRIGGERED_OLD_TRIGGERED);
        }
        else
            target->RemoveAurasTriggeredBySpell(GetId(), GetCasterGuid()); // just do it every time, lookup is too time consuming
    }

    for (int32 i = STAT_STRENGTH; i < MAX_STATS; ++i)
    {
        // -1 or -2 is all stats ( misc < -2 checked in function beginning )
        if (m_modifier.m_miscvalue < 0 || m_modifier.m_miscvalue == i)
        {
            float amount = m_removeMode == AURA_REMOVE_BY_GAINED_STACK ? float(m_modifier.m_recentAmount) : float(m_modifier.m_amount);
            // m_target->ApplyStatMod(Stats(i), m_modifier.m_amount,apply);
            target->HandleStatModifier(UnitMods(UNIT_MOD_STAT_START + i), TOTAL_VALUE, amount, apply);
            if (target->GetTypeId() == TYPEID_PLAYER)
                static_cast<Player*>(target)->ApplyStatBuffMod(Stats(i), amount, apply);
        }
    }
}

void Aura::HandleModPercentStat(bool apply, bool /*Real*/)
{
    if (m_modifier.m_miscvalue < -1 || m_modifier.m_miscvalue > 4)
    {
        sLog.outError("WARNING: Misc Value for SPELL_AURA_MOD_PERCENT_STAT not valid");
        return;
    }

    // only players have base stats
    if (GetTarget()->GetTypeId() != TYPEID_PLAYER)
        return;

    for (int32 i = STAT_STRENGTH; i < MAX_STATS; ++i)
    {
        if (m_modifier.m_miscvalue == i || m_modifier.m_miscvalue == -1)
            GetTarget()->HandleStatModifier(UnitMods(UNIT_MOD_STAT_START + i), BASE_PCT, float(m_modifier.m_amount), apply);
    }
}

void Aura::HandleModSpellDamagePercentFromStat(bool /*apply*/, bool /*Real*/)
{
    if (GetTarget()->GetTypeId() != TYPEID_PLAYER)
        return;

    // Magic damage modifiers implemented in Unit::SpellDamageBonusDone
    // This information for client side use only
    // Recalculate bonus
    ((Player*)GetTarget())->UpdateSpellDamageBonus();
}

void Aura::HandleModSpellHealingPercentFromStat(bool /*apply*/, bool /*Real*/)
{
    if (GetTarget()->GetTypeId() != TYPEID_PLAYER)
        return;

    // Recalculate bonus
    //((Player*)GetTarget())->UpdateSpellHealingBonus(); //Not implemented on classic
}

void Aura::HandleModHealingDone(bool /*apply*/, bool /*Real*/)
{
    if (GetTarget()->GetTypeId() != TYPEID_PLAYER)
        return;
    // implemented in Unit::SpellHealingBonusDone
    // this information is for client side only
    //((Player*)GetTarget())->UpdateSpellHealingBonus(); //Not implemented on classic
}

void Aura::HandleModTotalPercentStat(bool apply, bool /*Real*/)
{
    if (m_modifier.m_miscvalue < -1 || m_modifier.m_miscvalue > 4)
    {
        sLog.outError("WARNING: Misc Value for SPELL_AURA_MOD_PERCENT_STAT not valid");
        return;
    }

    Unit* target = GetTarget();

    // save current and max HP before applying aura
    uint32 curHPValue = target->GetHealth();
    uint32 maxHPValue = target->GetMaxHealth();

    for (int32 i = STAT_STRENGTH; i < MAX_STATS; ++i)
    {
        if (m_modifier.m_miscvalue == i || m_modifier.m_miscvalue == -1)
        {
            target->HandleStatModifier(UnitMods(UNIT_MOD_STAT_START + i), TOTAL_PCT, float(m_modifier.m_amount), apply);
            if (target->GetTypeId() == TYPEID_PLAYER)
                ((Player*)target)->ApplyStatPercentBuffMod(Stats(i), float(m_modifier.m_amount), apply);
        }
    }

    // recalculate current HP/MP after applying aura modifications (only for spells with 0x10 flag)
    if (m_modifier.m_miscvalue == STAT_STAMINA && maxHPValue > 0 && GetSpellProto()->HasAttribute(SPELL_ATTR_IS_ABILITY) && target->GetHealth() > 0)
    {
        // newHP = (curHP / maxHP) * newMaxHP = (newMaxHP * curHP) / maxHP -> which is better because no int -> double -> int conversion is needed
        uint32 newHPValue = std::max(1u, (target->GetMaxHealth() * curHPValue) / maxHPValue);
        target->SetHealth(newHPValue);
    }
}

void Aura::HandleAuraModResistenceOfStatPercent(bool /*apply*/, bool /*Real*/)
{
    if (GetTarget()->GetTypeId() != TYPEID_PLAYER)
        return;

    if (m_modifier.m_miscvalue != SPELL_SCHOOL_MASK_NORMAL)
    {
        // support required adding replace UpdateArmor by loop by UpdateResistence at intellect update
        // and include in UpdateResistence same code as in UpdateArmor for aura mod apply.
        sLog.outError("Aura SPELL_AURA_MOD_RESISTANCE_OF_STAT_PERCENT(182) need adding support for non-armor resistances!");
        return;
    }

    // Recalculate Armor
    GetTarget()->UpdateArmor();
}

/********************************/
/***      HEAL & ENERGIZE     ***/
/********************************/
void Aura::HandleAuraModTotalHealthPercentRegen(bool apply, bool /*Real*/)
{
    m_isPeriodic = apply;
}

void Aura::HandleAuraModTotalManaPercentRegen(bool apply, bool /*Real*/)
{
    if (m_modifier.periodictime == 0)
        m_modifier.periodictime = 1000;

    m_periodicTimer = m_modifier.periodictime;
    m_isPeriodic = apply;

    if (GetId() == 30024 && !apply && m_removeMode == AURA_REMOVE_BY_DEFAULT) // Shade of Aran drink on interrupt
    {
        Unit* target = GetTarget();
        UnitAI* ai = GetTarget()->AI();
        if (ai && target->GetTypeId() == TYPEID_UNIT)
            ai->SendAIEvent(AI_EVENT_CUSTOM_A, target, static_cast<Creature*>(target));
    }
}

void Aura::HandleModRegen(bool apply, bool /*Real*/)        // eating
{
    if (m_modifier.periodictime == 0)
        m_modifier.periodictime = 5000;

    m_periodicTimer = 5000;
    m_isPeriodic = apply;
}

void Aura::HandleModPowerRegen(bool apply, bool Real)       // drinking
{
    if (!Real)
        return;

    Powers powerType = GetTarget()->GetPowerType();
    if (m_modifier.periodictime == 0)
    {
        // Anger Management (only spell use this aura for rage)
        if (powerType == POWER_RAGE)
            m_modifier.periodictime = 3000;
        else
            m_modifier.periodictime = 2000;
    }

    m_periodicTimer = 5000;

    if (GetTarget()->GetTypeId() == TYPEID_PLAYER && m_modifier.m_miscvalue == POWER_MANA)
        ((Player*)GetTarget())->UpdateManaRegen();

    m_isPeriodic = apply;
}

void Aura::HandleModPowerRegenPCT(bool /*apply*/, bool Real)
{
    // spells required only Real aura add/remove
    if (!Real)
        return;

    if (GetTarget()->GetTypeId() != TYPEID_PLAYER)
        return;

    // Update manaregen value
    if (m_modifier.m_miscvalue == POWER_MANA)
        ((Player*)GetTarget())->UpdateManaRegen();
}

void Aura::HandleAuraModIncreaseHealth(bool apply, bool Real)
{
    Unit* target = GetTarget();

    switch (GetId())
    {
        // Special case with temporary increase max/current health
        // Cases where we need to manually calculate the amount for the spell (by percentage)
        // recalculate to full amount at apply for proper remove
        // Backport notive TBC: no cases yet
        // no break here

        // Cases where m_amount already has the correct value (spells cast with CastCustomSpell or absolute values)
        case 4148:                                          // Growl of Fortitude
        case 12976:                                         // Warrior Last Stand triggered spell (Cast with percentage-value by CastCustomSpell)
        case 16609:                                         // Warchief's Blessing
        {
            if (Real)
            {
                if (apply)
                {
                    target->HandleStatModifier(UNIT_MOD_HEALTH, TOTAL_VALUE, float(m_modifier.m_amount), apply);
                    target->ModifyHealth(m_modifier.m_amount);
                }
                else
                {
                    if (m_removeMode != AURA_REMOVE_BY_DEATH)
                    {
                        if (int32(target->GetHealth()) > m_modifier.m_amount)
                            target->ModifyHealth(-m_modifier.m_amount);
                        else
                            target->SetHealth(1);
                    }

                    target->HandleStatModifier(UNIT_MOD_HEALTH, TOTAL_VALUE, float(m_modifier.m_amount), apply);
                }
            }
            return;
        }
        // Case with temp increase health, where total percentage is kept
        case 1178:                                          // Bear Form (Passive)
        case 9635:                                          // Dire Bear Form (Passive)
        {
            if (Real)
            {
                float pct = target->GetHealthPercent();
                target->HandleStatModifier(UNIT_MOD_HEALTH, TOTAL_VALUE, float(m_modifier.m_amount), apply);
                target->SetHealthPercent(pct);
            }
            return;
        }
        // generic case
        default:
            if (m_removeMode != AURA_REMOVE_BY_GAINED_STACK)
                target->HandleStatModifier(UNIT_MOD_HEALTH, TOTAL_VALUE, float(m_modifier.m_amount), apply);
            else
                target->HandleStatModifier(UNIT_MOD_HEALTH, TOTAL_VALUE, float(m_modifier.m_recentAmount), apply);
            break;
    }
}

void Aura::HandleAuraModIncreaseEnergy(bool apply, bool /*Real*/)
{
    Unit* target = GetTarget();
    Powers powerType = Powers(m_modifier.m_miscvalue);

    UnitMods unitMod = UnitMods(UNIT_MOD_POWER_START + powerType);

    target->HandleStatModifier(unitMod, TOTAL_VALUE, float(m_removeMode == AURA_REMOVE_BY_GAINED_STACK ? m_modifier.m_recentAmount : m_modifier.m_amount), apply);
}

void Aura::HandleAuraModIncreaseEnergyPercent(bool apply, bool /*Real*/)
{
    Unit* target = GetTarget();
    Powers powerType = Powers(m_modifier.m_miscvalue);

    UnitMods unitMod = UnitMods(UNIT_MOD_POWER_START + powerType);

    target->HandleStatModifier(unitMod, TOTAL_PCT, float(m_modifier.m_amount), apply);
    target->ModifyPower(powerType, apply ? m_modifier.m_amount : -m_modifier.m_amount);
}

void Aura::HandleAuraModIncreaseHealthPercent(bool apply, bool /*Real*/)
{
    Unit* target = GetTarget();

    uint32 oldMaxHealth = target->GetMaxHealth();
    target->HandleStatModifier(UNIT_MOD_HEALTH, TOTAL_PCT, float(m_modifier.m_amount), apply);

    switch (GetId())
    {
        case 802:                           // Mutate Bug
        {
            if (apply)
                target->ModifyHealth(target->GetMaxHealth() - oldMaxHealth);
            break;
        }
    }
}

/********************************/
/***          FIGHT           ***/
/********************************/

void Aura::HandleAuraModParryPercent(bool apply, bool /*Real*/)
{
    Unit* target = GetTarget();

    if (target->GetTypeId() != TYPEID_PLAYER)
    {
        target->m_modParryChance += (apply ? m_modifier.m_amount : -m_modifier.m_amount);
        return;
    }

    ((Player*)target)->UpdateParryPercentage();
}

void Aura::HandleAuraModDodgePercent(bool apply, bool /*Real*/)
{
    Unit* target = GetTarget();

    if (target->GetTypeId() != TYPEID_PLAYER)
    {
        target->m_modDodgeChance += (apply ? m_modifier.m_amount : -m_modifier.m_amount);
        return;
    }

    ((Player*)target)->UpdateDodgePercentage();
    // sLog.outError("BONUS DODGE CHANCE: + %f", float(m_modifier.m_amount));
}

void Aura::HandleAuraModBlockPercent(bool apply, bool /*Real*/)
{
    Unit* target = GetTarget();

    if (target->GetTypeId() != TYPEID_PLAYER)
    {
        target->m_modBlockChance += (apply ? m_modifier.m_amount : -m_modifier.m_amount);
        return;
    }

    ((Player*)target)->UpdateBlockPercentage();
    // sLog.outError("BONUS BLOCK CHANCE: + %f", float(m_modifier.m_amount));
}

void Aura::HandleAuraModRegenInterrupt(bool /*apply*/, bool Real)
{
    // spells required only Real aura add/remove
    if (!Real)
        return;

    if (GetTarget()->GetTypeId() != TYPEID_PLAYER)
        return;

    ((Player*)GetTarget())->UpdateManaRegen();
}

void Aura::HandleAuraModCritPercent(bool apply, bool Real)
{
    Unit* target = GetTarget();

    if (target->GetTypeId() != TYPEID_PLAYER)
    {
        for (float& i : target->m_modCritChance)
            i += (apply ? m_modifier.m_amount : -m_modifier.m_amount);
        return;
    }

    // apply item specific bonuses for already equipped weapon
    if (Real)
    {
        for (int i = 0; i < MAX_ATTACK; ++i)
            if (Item* pItem = ((Player*)target)->GetWeaponForAttack(WeaponAttackType(i), true, false))
                ((Player*)target)->_ApplyWeaponDependentAuraCritMod(pItem, WeaponAttackType(i), this, apply);
    }

    // mods must be applied base at equipped weapon class and subclass comparison
    // with spell->EquippedItemClass and  EquippedItemSubClassMask and EquippedItemInventoryTypeMask
    // m_modifier.m_miscvalue comparison with item generated damage types

    if (GetSpellProto()->EquippedItemClass == -1)
    {
        ((Player*)target)->HandleBaseModValue(CRIT_PERCENTAGE,         FLAT_MOD, float(m_modifier.m_amount), apply);
        ((Player*)target)->HandleBaseModValue(OFFHAND_CRIT_PERCENTAGE, FLAT_MOD, float(m_modifier.m_amount), apply);
        ((Player*)target)->HandleBaseModValue(RANGED_CRIT_PERCENTAGE,  FLAT_MOD, float(m_modifier.m_amount), apply);
    }
    else
    {
        // done in Player::_ApplyWeaponDependentAuraMods
    }
}

void Aura::HandleModHitChance(bool apply, bool /*Real*/)
{
    Unit* target = GetTarget();

    if (target->IsPlayer())
    {
        static_cast<Player*>(target)->UpdateWeaponHitChances(BASE_ATTACK);
        static_cast<Player*>(target)->UpdateWeaponHitChances(OFF_ATTACK);
        static_cast<Player*>(target)->UpdateWeaponHitChances(RANGED_ATTACK);
    }
    else
    {
        target->m_modWeaponHitChance[BASE_ATTACK] += (apply ? m_modifier.m_amount : -m_modifier.m_amount);
        target->m_modWeaponHitChance[OFF_ATTACK] += (apply ? m_modifier.m_amount : -m_modifier.m_amount);
        target->m_modWeaponHitChance[RANGED_ATTACK] += (apply ? m_modifier.m_amount : -m_modifier.m_amount);
    }
}

void Aura::HandleModSpellHitChance(bool apply, bool /*Real*/)
{
    GetTarget()->m_modSpellHitChance += (apply ? m_modifier.m_amount : -m_modifier.m_amount);
}

void Aura::HandleModSpellCritChance(bool apply, bool Real)
{
    // spells required only Real aura add/remove
    if (!Real)
        return;

    Unit* target = GetTarget();

    if (target->GetTypeId() == TYPEID_UNIT)
    {
        for (uint8 school = SPELL_SCHOOL_NORMAL; school < MAX_SPELL_SCHOOL; ++school)
            target->m_modSpellCritChance[school] += (apply ? m_modifier.m_amount : -m_modifier.m_amount);
        return;
    }

    ((Player*)target)->UpdateAllSpellCritChances();
}

void Aura::HandleModSpellCritChanceShool(bool apply, bool Real)
{
    // spells required only Real aura add/remove
    if (!Real)
        return;

    Unit* target = GetTarget();

    for (uint8 school = SPELL_SCHOOL_NORMAL; school < MAX_SPELL_SCHOOL; ++school)
    {
        if (m_modifier.m_miscvalue & (1 << school))
        {
            if (target->GetTypeId() == TYPEID_UNIT)
                target->m_modSpellCritChance[school] += (apply ? m_modifier.m_amount : -m_modifier.m_amount);
            else
                ((Player*)target)->UpdateSpellCritChance(school);
        }
    }
}

/********************************/
/***         ATTACK SPEED     ***/
/********************************/

void Aura::HandleModCastingSpeed(bool apply, bool /*Real*/)
{
    float amount = m_modifier.m_amount;

    GetTarget()->ApplyCastTimePercentMod(amount, apply);
}

void Aura::HandleModAttackSpeed(bool apply, bool /*Real*/)
{
    float amount = m_modifier.m_amount;

    GetTarget()->ApplyAttackTimePercentMod(BASE_ATTACK, amount, apply);
}

void Aura::HandleModMeleeSpeedPct(bool apply, bool /*Real*/)
{
    float amount = m_modifier.m_amount;

    Unit* target = GetTarget();
    target->ApplyAttackTimePercentMod(BASE_ATTACK, amount, apply);
    target->ApplyAttackTimePercentMod(OFF_ATTACK, amount, apply);
}

void Aura::HandleAuraModRangedHaste(bool apply, bool /*Real*/)
{
    float amount = m_modifier.m_amount;

    GetTarget()->ApplyAttackTimePercentMod(RANGED_ATTACK, amount, apply);
}

void Aura::HandleRangedAmmoHaste(bool apply, bool /*Real*/)
{
    if (GetTarget()->GetTypeId() != TYPEID_PLAYER)
        return;

    float amount = m_modifier.m_amount;

    GetTarget()->ApplyAttackTimePercentMod(RANGED_ATTACK, amount, apply);
}

/********************************/
/***        ATTACK POWER      ***/
/********************************/

void Aura::HandleAuraModAttackPower(bool apply, bool /*Real*/)
{
    float amount = m_modifier.m_amount;

    GetTarget()->HandleStatModifier(UNIT_MOD_ATTACK_POWER, TOTAL_VALUE, amount, apply);
}

void Aura::HandleAuraModRangedAttackPower(bool apply, bool /*Real*/)
{
    if ((GetTarget()->getClassMask() & CLASSMASK_WAND_USERS) != 0)
        return;

    float amount = m_modifier.m_amount;

    GetTarget()->HandleStatModifier(UNIT_MOD_ATTACK_POWER_RANGED, TOTAL_VALUE, amount, apply);
}

void Aura::HandleAuraModAttackPowerPercent(bool apply, bool /*Real*/)
{
    float amount = m_modifier.m_amount;

    // UNIT_FIELD_ATTACK_POWER_MULTIPLIER = multiplier - 1
    GetTarget()->HandleStatModifier(UNIT_MOD_ATTACK_POWER, TOTAL_PCT, amount, apply);
}

void Aura::HandleAuraModRangedAttackPowerPercent(bool apply, bool /*Real*/)
{
    if ((GetTarget()->getClassMask() & CLASSMASK_WAND_USERS) != 0)
        return;

    float amount = m_modifier.m_amount;

    // UNIT_FIELD_RANGED_ATTACK_POWER_MULTIPLIER = multiplier - 1
    GetTarget()->HandleStatModifier(UNIT_MOD_ATTACK_POWER_RANGED, TOTAL_PCT, amount, apply);
}

/********************************/
/***        DAMAGE BONUS      ***/
/********************************/
void Aura::HandleModDamageDone(bool apply, bool Real)
{
    Unit* target = GetTarget();

    // apply item specific bonuses for already equipped weapon
    if (Real && target->GetTypeId() == TYPEID_PLAYER)
    {
        for (int i = 0; i < MAX_ATTACK; ++i)
            if (Item* pItem = ((Player*)target)->GetWeaponForAttack(WeaponAttackType(i), true, false))
                ((Player*)target)->_ApplyWeaponDependentAuraDamageMod(pItem, WeaponAttackType(i), this, apply);
    }

    // m_modifier.m_miscvalue is bitmask of spell schools
    // 1 ( 0-bit ) - normal school damage (SPELL_SCHOOL_MASK_NORMAL)
    // 126 - full bitmask all magic damages (SPELL_SCHOOL_MASK_MAGIC) including wands
    // 127 - full bitmask any damages
    //
    // mods must be applied base at equipped weapon class and subclass comparison
    // with spell->EquippedItemClass and  EquippedItemSubClassMask and EquippedItemInventoryTypeMask
    // m_modifier.m_miscvalue comparison with item generated damage types

    if ((m_modifier.m_miscvalue & SPELL_SCHOOL_MASK_NORMAL) != 0)
    {
        // apply generic physical damage bonuses including wand case
        if (GetSpellProto()->EquippedItemClass == -1 || target->GetTypeId() != TYPEID_PLAYER)
        {
            target->HandleStatModifier(UNIT_MOD_DAMAGE_MAINHAND, TOTAL_VALUE, float(m_modifier.m_amount), apply);
            target->HandleStatModifier(UNIT_MOD_DAMAGE_OFFHAND, TOTAL_VALUE, float(m_modifier.m_amount), apply);
            target->HandleStatModifier(UNIT_MOD_DAMAGE_RANGED, TOTAL_VALUE, float(m_modifier.m_amount), apply);
        }
        else
        {
            // done in Player::_ApplyWeaponDependentAuraMods
        }

        if (target->GetTypeId() == TYPEID_PLAYER)
        {
            if (m_positive)
                target->ApplyModUInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS, m_modifier.m_amount, apply);
            else
                target->ApplyModUInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_NEG, m_modifier.m_amount, apply);
        }
    }

    // Skip non magic case for speedup
    if ((m_modifier.m_miscvalue & SPELL_SCHOOL_MASK_MAGIC) == 0)
        return;

    if (GetSpellProto()->EquippedItemClass != -1 || GetSpellProto()->EquippedItemInventoryTypeMask != 0)
    {
        // wand magic case (skip generic to all item spell bonuses)
        // done in Player::_ApplyWeaponDependentAuraMods

        // Skip item specific requirements for not wand magic damage
        return;
    }

    // Magic damage modifiers implemented in Unit::SpellDamageBonusDone
    // This information for client side use only
    if (target->GetTypeId() == TYPEID_PLAYER)
    {
        if (m_positive)
        {
            for (int i = SPELL_SCHOOL_HOLY; i < MAX_SPELL_SCHOOL; ++i)
            {
                if ((m_modifier.m_miscvalue & (1 << i)) != 0)
                    target->ApplyModUInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + i, m_modifier.m_amount, apply);
            }
        }
        else
        {
            for (int i = SPELL_SCHOOL_HOLY; i < MAX_SPELL_SCHOOL; ++i)
            {
                if ((m_modifier.m_miscvalue & (1 << i)) != 0)
                    target->ApplyModUInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_NEG + i, m_modifier.m_amount, apply);
            }
        }
        Pet* pet = target->GetPet();
        if (pet)
            pet->UpdateAttackPowerAndDamage();
    }
}

void Aura::HandleModDamagePercentDone(bool apply, bool Real)
{
    DEBUG_FILTER_LOG(LOG_FILTER_SPELL_CAST, "AURA MOD DAMAGE type:%u negative:%u", m_modifier.m_miscvalue, m_positive ? 0 : 1);
    Unit* target = GetTarget();

    // apply item specific bonuses for already equipped weapon
    if (Real && target->GetTypeId() == TYPEID_PLAYER)
    {
        for (int i = 0; i < MAX_ATTACK; ++i)
            if (Item* pItem = ((Player*)target)->GetWeaponForAttack(WeaponAttackType(i), true, false))
                ((Player*)target)->_ApplyWeaponDependentAuraDamageMod(pItem, WeaponAttackType(i), this, apply);
    }

    // m_modifier.m_miscvalue is bitmask of spell schools
    // 1 ( 0-bit ) - normal school damage (SPELL_SCHOOL_MASK_NORMAL)
    // 126 - full bitmask all magic damages (SPELL_SCHOOL_MASK_MAGIC) including wand
    // 127 - full bitmask any damages
    //
    // mods must be applied base at equipped weapon class and subclass comparison
    // with spell->EquippedItemClass and  EquippedItemSubClassMask and EquippedItemInventoryTypeMask
    // m_modifier.m_miscvalue comparison with item generated damage types

    if ((m_modifier.m_miscvalue & SPELL_SCHOOL_MASK_NORMAL) != 0)
    {
        // apply generic physical damage bonuses including wand case
        if (GetSpellProto()->EquippedItemClass == -1 || target->GetTypeId() != TYPEID_PLAYER)
        {
            target->HandleStatModifier(UNIT_MOD_DAMAGE_MAINHAND, TOTAL_PCT, float(m_modifier.m_amount), apply);
            target->HandleStatModifier(UNIT_MOD_DAMAGE_OFFHAND, TOTAL_PCT, float(m_modifier.m_amount), apply);
            target->HandleStatModifier(UNIT_MOD_DAMAGE_RANGED, TOTAL_PCT, float(m_modifier.m_amount), apply);

            // For show in client
            if (target->GetTypeId() == TYPEID_PLAYER)
                target->ApplyModSignedFloatValue(PLAYER_FIELD_MOD_DAMAGE_DONE_PCT, m_modifier.m_amount / 100.0f, apply);
        }
        else
        {
            // done in Player::_ApplyWeaponDependentAuraMods
        }
    }

    // Skip non magic case for speedup
    if ((m_modifier.m_miscvalue & SPELL_SCHOOL_MASK_MAGIC) == 0)
        return;

    if (GetSpellProto()->EquippedItemClass != -1 || GetSpellProto()->EquippedItemInventoryTypeMask != 0)
    {
        // wand magic case (skip generic to all item spell bonuses)
        // done in Player::_ApplyWeaponDependentAuraMods

        // Skip item specific requirements for not wand magic damage
        return;
    }

    // Magic damage percent modifiers implemented in Unit::SpellDamageBonusDone
    // Send info to client
    if (target->GetTypeId() == TYPEID_PLAYER)
        for (int i = SPELL_SCHOOL_HOLY; i < MAX_SPELL_SCHOOL; ++i)
            if (m_modifier.m_miscvalue & (1 << i))
                target->ApplyModSignedFloatValue(PLAYER_FIELD_MOD_DAMAGE_DONE_PCT + i, m_modifier.m_amount / 100.0f, apply);

    if (!apply && m_removeMode == AURA_REMOVE_BY_EXPIRE)
        if (GetId() == 30423)
            target->CastSpell(target, 38639, TRIGGERED_OLD_TRIGGERED);
}

void Aura::HandleModOffhandDamagePercent(bool apply, bool Real)
{
    // spells required only Real aura add/remove
    if (!Real)
        return;

    DEBUG_FILTER_LOG(LOG_FILTER_SPELL_CAST, "AURA MOD OFFHAND DAMAGE");

    GetTarget()->HandleStatModifier(UNIT_MOD_DAMAGE_OFFHAND, TOTAL_PCT, float(m_modifier.m_amount), apply);
}

/********************************/
/***        POWER COST        ***/
/********************************/

void Aura::HandleModPowerCostPCT(bool apply, bool Real)
{
    // spells required only Real aura add/remove
    if (!Real)
        return;

    Unit* target = GetTarget();

    float amount = (m_removeMode == AURA_REMOVE_BY_GAINED_STACK ? m_modifier.m_recentAmount : m_modifier.m_amount) / 100.0f;
    for (int i = 0; i < MAX_SPELL_SCHOOL; ++i)
        if (m_modifier.m_miscvalue & (1 << i))
            target->ApplyModSignedFloatValue(UNIT_FIELD_POWER_COST_MULTIPLIER + i, amount, apply);

    if (!apply && m_removeMode == AURA_REMOVE_BY_EXPIRE)
        if (GetId() == 30422)
            target->CastSpell(target, 38638, TRIGGERED_OLD_TRIGGERED);
}

void Aura::HandleModPowerCost(bool apply, bool Real)
{
    // spells required only Real aura add/remove
    if (!Real)
        return;

    for (int i = 0; i < MAX_SPELL_SCHOOL; ++i)
        if (m_modifier.m_miscvalue & (1 << i))
            GetTarget()->ApplyModInt32Value(UNIT_FIELD_POWER_COST_MODIFIER + i, m_modifier.m_amount, apply);
}

/*********************************************************/
/***                    OTHERS                         ***/
/*********************************************************/

void Aura::HandleShapeshiftBoosts(bool apply)
{
    uint32 spellId1 = 0;
    uint32 spellId2 = 0;
    uint32 HotWSpellId = 0;

    ShapeshiftForm form = ShapeshiftForm(GetModifier()->m_miscvalue);

    Unit* target = GetTarget();

    switch (form)
    {
        case FORM_CAT:
            spellId1 = 3025;
            HotWSpellId = 24900;
            break;
        case FORM_TREE:
            spellId1 = 5420;
            break;
        case FORM_TRAVEL:
            spellId1 = 5419;
            break;
        case FORM_AQUA:
            spellId1 = 5421;
            break;
        case FORM_BEAR:
            spellId1 = 1178;
            spellId2 = 21178;
            HotWSpellId = 24899;
            break;
        case FORM_DIREBEAR:
            spellId1 = 9635;
            spellId2 = 21178;
            HotWSpellId = 24899;
            break;
        case FORM_BATTLESTANCE:
            spellId1 = 21156;
            break;
        case FORM_DEFENSIVESTANCE:
            spellId1 = 7376;
            break;
        case FORM_BERSERKERSTANCE:
            spellId1 = 7381;
            break;
        case FORM_MOONKIN:
            spellId1 = 24905;
            break;
        case FORM_SPIRITOFREDEMPTION:
            spellId1 = 27792;
            spellId2 = 27795;                               // must be second, this important at aura remove to prevent to early iterator invalidation.
            break;
        case FORM_GHOSTWOLF:
        case FORM_AMBIENT:
        case FORM_GHOUL:
        case FORM_SHADOW:
        case FORM_STEALTH:
        case FORM_CREATURECAT:
        case FORM_CREATUREBEAR:
            break;
        default:
            break;
    }

    if (apply)
    {
        if (spellId1)
            target->CastSpell(nullptr, spellId1, TRIGGERED_OLD_TRIGGERED, nullptr, this);
        if (spellId2)
            target->CastSpell(nullptr, spellId2, TRIGGERED_OLD_TRIGGERED, nullptr, this);

        if (target->GetTypeId() == TYPEID_PLAYER)
        {
            const PlayerSpellMap& sp_list = ((Player*)target)->GetSpellMap();
            for (const auto& itr : sp_list)
            {
                if (itr.second.state == PLAYERSPELL_REMOVED) continue;
                if (itr.first == spellId1 || itr.first == spellId2) continue;
                SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(itr.first);
                if (!spellInfo || !IsNeedCastSpellAtFormApply(spellInfo, form))
                    continue;
                target->CastSpell(nullptr, itr.first, TRIGGERED_OLD_TRIGGERED, nullptr, this);
            }

            // Leader of the Pack
            if (((Player*)target)->HasSpell(17007))
            {
                SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(24932);
                if (spellInfo && spellInfo->Stances & (1 << (form - 1)))
                    target->CastSpell(nullptr, 24932, TRIGGERED_OLD_TRIGGERED, nullptr, this);
            }

            // Heart of the Wild
            if (HotWSpellId)
            {
                Unit::AuraList const& mModTotalStatPct = target->GetAurasByType(SPELL_AURA_MOD_TOTAL_STAT_PERCENTAGE);
                for (auto i : mModTotalStatPct)
                {
                    if (i->GetSpellProto()->SpellIconID == 240 && i->GetModifier()->m_miscvalue == 3)
                    {
                        int32 HotWMod = i->GetModifier()->m_amount;
                        target->CastCustomSpell(nullptr, HotWSpellId, &HotWMod, nullptr, nullptr, TRIGGERED_OLD_TRIGGERED, nullptr, this);
                        break;
                    }
                }
            }
        }
    }
    else
    {
        if (spellId1)
            target->RemoveAurasDueToSpell(spellId1);
        if (spellId2)
            target->RemoveAurasDueToSpell(spellId2);

        Unit::SpellAuraHolderMap& tAuras = target->GetSpellAuraHolderMap();
        for (Unit::SpellAuraHolderMap::iterator itr = tAuras.begin(); itr != tAuras.end();)
        {
            if (itr->second->IsRemovedOnShapeLost())
            {
                target->RemoveAurasDueToSpell(itr->second->GetId());
                itr = tAuras.begin();
            }
            else
                ++itr;
        }
    }
}

void Aura::HandleAuraEmpathy(bool apply, bool /*Real*/)
{
    Unit* target = GetTarget();

    // This aura is expected to only work with CREATURE_TYPE_BEAST or players
    CreatureInfo const* ci = ObjectMgr::GetCreatureTemplate(target->GetEntry());
    if (target->GetTypeId() == TYPEID_PLAYER || (target->GetTypeId() == TYPEID_UNIT && ci && ci->CreatureType == CREATURE_TYPE_BEAST))
        target->ApplyModUInt32Value(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_SPECIALINFO, apply);

    if (apply)
    {
        if (Unit* caster = GetCaster())
        {
            if (Player* playerCaster = caster->IsPlayer() ? static_cast<Player*>(caster) : nullptr)
            {
                UpdateData updateData;
                target->BuildValuesUpdateBlockForPlayerWithFlags(updateData, playerCaster, UpdateFieldFlags(UF_FLAG_SPECIAL_INFO | UF_FLAG_DYNAMIC));
                updateData.SendData(*playerCaster->GetSession());
            }
        }
    }
}

void Aura::HandleAuraUntrackable(bool apply, bool /*Real*/)
{
    if (apply)
        GetTarget()->SetVisFlags(UNIT_VIS_FLAG_UNTRACKABLE);
    else
        GetTarget()->RemoveVisFlags(UNIT_VIS_FLAG_UNTRACKABLE);
}

void Aura::HandleAuraModPacify(bool apply, bool /*Real*/)
{
    if (apply)
        GetTarget()->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PACIFIED);
    else
        GetTarget()->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PACIFIED);
}

void Aura::HandleAuraModPacifyAndSilence(bool apply, bool Real)
{
    HandleAuraModPacify(apply, Real);
    HandleAuraModSilence(apply, Real);
}

void Aura::HandleAuraGhost(bool apply, bool /*Real*/)
{
    Unit* target = GetTarget();
    Player* player = target->IsPlayer() ? (Player*)target : nullptr;

    if (apply)
    {
        target->SetVisFlags(UNIT_VIS_FLAG_GHOST);
        if (player)
        {
            player->SetFlag(PLAYER_FLAGS, PLAYER_FLAGS_GHOST);
            if (!player->HasAuraType(SPELL_AURA_WATER_WALK))
                player->SetWaterWalk(true);
        }
    }
    else
    {
        target->RemoveVisFlags(UNIT_VIS_FLAG_GHOST);
        if (player)
        {
            player->RemoveFlag(PLAYER_FLAGS, PLAYER_FLAGS_GHOST);
            if (!player->HasAuraType(SPELL_AURA_WATER_WALK))
                player->SetWaterWalk(false);
        }
    }

    if (player && player->GetGroup())
        player->SetGroupUpdateFlag(GROUP_UPDATE_FLAG_STATUS);
}

void Aura::HandleShieldBlockValue(bool apply, bool /*Real*/)
{
    BaseModType modType = FLAT_MOD;
    if (m_modifier.m_auraname == SPELL_AURA_MOD_SHIELD_BLOCKVALUE_PCT)
        modType = PCT_MOD;

    if (GetTarget()->GetTypeId() == TYPEID_PLAYER)
        ((Player*)GetTarget())->HandleBaseModValue(SHIELD_BLOCK_VALUE, modType, float(m_modifier.m_amount), apply);
}

void Aura::HandleAuraRetainComboPoints(bool apply, bool Real)
{
    // spells required only Real aura add/remove
    if (!Real)
        return;

    if (GetTarget()->GetTypeId() != TYPEID_PLAYER)
        return;

    Player* target = (Player*)GetTarget();

    // combo points was added in SPELL_EFFECT_ADD_COMBO_POINTS handler
    // remove only if aura expire by time (in case combo points amount change aura removed without combo points lost)
    if (!apply && m_removeMode == AURA_REMOVE_BY_EXPIRE && target->GetComboTargetGuid())
        if (Unit* unit = ObjectAccessor::GetUnit(*GetTarget(), target->GetComboTargetGuid()))
            target->AddComboPoints(unit, -m_modifier.m_amount);
}

void Aura::HandleModUnattackable(bool Apply, bool Real)
{
    if (Real && Apply)
        GetTarget()->CombatStop();

    GetTarget()->ApplyModFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_UNTARGETABLE, Apply);
}

void Aura::HandleSpiritOfRedemption(bool apply, bool Real)
{
    // spells required only Real aura add/remove
    if (!Real)
        return;

    Unit* target = GetTarget();

    target->ApplySpellImmune(this, IMMUNITY_DAMAGE, 255, apply); // wotlk style hack - immunity against all damage

    // prepare spirit state
    if (apply)
    {
        if (target->GetTypeId() == TYPEID_PLAYER)
        {
            // set stand state (expected in this form)
            if (!target->IsStandState())
                target->SetStandState(UNIT_STAND_STATE_STAND);
        }

        // interrupt casting when entering Spirit of Redemption
        if (target->IsNonMeleeSpellCasted(false))
            target->InterruptNonMeleeSpells(false);

        // set health and mana to maximum        
        target->SetPower(POWER_MANA, target->GetMaxPower(POWER_MANA));
    }
    // die at aura end
    else
        target->CastSpell(nullptr, 27965, TRIGGERED_OLD_TRIGGERED); // Suicide
}

void Aura::PeriodicTick()
{
    Unit* target = GetTarget();
    // passive periodic trigger spells should not be updated when dead, only death persistent should
    if (!target->IsAlive() && GetHolder()->IsPassive())
        return;

    SpellEntry const* spellProto = GetSpellProto();

    switch (m_modifier.m_auraname)
    {
        case SPELL_AURA_PERIODIC_DAMAGE:
        case SPELL_AURA_PERIODIC_DAMAGE_PERCENT:
        {
            // don't damage target if not alive, possible death persistent effects
            if (!target->IsAlive())
                break;

            Unit* caster = GetCaster();

            uint32 absorb = 0;
            int32 resist = 0;

            // ignore non positive values (can be result apply spellmods to aura damage
            uint32 amount = (m_modifier.m_amount > 0 ? uint32(m_modifier.m_amount) : 0);

            uint32 pdamage;

            if (m_modifier.m_auraname == SPELL_AURA_PERIODIC_DAMAGE)
                pdamage = amount;
            else
                pdamage = uint32(target->GetMaxHealth() * amount / 100);

            OnPeriodicCalculateAmount(pdamage);

            if (spellProto->Effect[GetEffIndex()] == SPELL_EFFECT_PERSISTENT_AREA_AURA) // safe case - caster always will exist
            {
                if (!caster)
                {
                    sLog.outCustomLog("Spell ID: %u Caster guid %lu", spellProto->Id, GetCasterGuid().GetRawValue());
                    MANGOS_ASSERT(caster);
                }
                if (Unit::SpellHitResult(caster, target, spellProto, (1 << GetEffIndex()), false) != SPELL_MISS_NONE)
                    break;
            }

            // Check for immune (not use charges)
            if (!spellProto->HasAttribute(SPELL_ATTR_NO_IMMUNITIES)) // confirmed Impaling spine goes through immunity
            {
                if (target->IsImmuneToDamage(GetSpellSchoolMask(spellProto)))
                {
                    Unit::SendSpellOrDamageImmune(GetCasterGuid(), target, spellProto->Id);
                    break;
                }
            }

            bool isNotBleed = GetEffectMechanic(spellProto, m_effIndex) != MECHANIC_BLEED;
            // SpellDamageBonus for magic spells
            if ((spellProto->DmgClass == SPELL_DAMAGE_CLASS_NONE && isNotBleed) || spellProto->DmgClass == SPELL_DAMAGE_CLASS_MAGIC)
                pdamage = target->SpellDamageBonusTaken(caster, GetSpellSchoolMask(spellProto), spellProto, GetEffIndex(), pdamage, DOT, GetStackAmount());
            // MeleeDamagebonus for weapon based spells
            else
            {
                WeaponAttackType attackType = GetWeaponAttackType(spellProto);
                pdamage = target->MeleeDamageBonusTaken(caster, pdamage, attackType, GetSpellSchoolMask(spellProto), spellProto, GetEffIndex(), DOT, GetStackAmount());
            }

            target->CalculateDamageAbsorbAndResist(caster, GetSpellSchoolMask(spellProto), DOT, pdamage, &absorb, &resist, IsReflectableSpell(spellProto), IsResistableSpell(spellProto));

            DETAIL_FILTER_LOG(LOG_FILTER_PERIODIC_AFFECTS, "PeriodicTick: %s attacked %s for %u dmg inflicted by %u",
                              GetCasterGuid().GetString().c_str(), target->GetGuidStr().c_str(), pdamage, GetId());

            Unit::DealDamageMods(caster, target, pdamage, &absorb, DOT, spellProto);

            // Set trigger flag
            uint32 procAttacker = PROC_FLAG_DEAL_HARMFUL_PERIODIC; //  | PROC_FLAG_SUCCESSFUL_HARMFUL_SPELL_HIT;
            uint32 procVictim   = PROC_FLAG_TAKE_HARMFUL_PERIODIC;// | PROC_FLAG_TAKEN_HARMFUL_SPELL_HIT;

            const uint32 bonus = (resist < 0 ? uint32(std::abs(resist)) : 0);
            pdamage += bonus;
            const uint32 malus = (resist > 0 ? (absorb + uint32(resist)) : absorb);
            pdamage = (pdamage <= malus ? 0 : (pdamage - malus));

            SpellPeriodicAuraLogInfo pInfo(this, pdamage, absorb, resist, 0.0f);
            target->SendPeriodicAuraLog(&pInfo);

            if (pdamage)
                procVictim |= PROC_FLAG_TAKE_ANY_DAMAGE;

            CleanDamage cleanDamage(pdamage, BASE_ATTACK, MELEE_HIT_NORMAL, pdamage || absorb);
            Unit::DealDamage(caster, target, pdamage, &cleanDamage, DOT, GetSpellSchoolMask(spellProto), spellProto, true);

            Unit::ProcDamageAndSpell(ProcSystemArguments(caster, target, procAttacker, procVictim, PROC_EX_NORMAL_HIT, pdamage, absorb, BASE_ATTACK, spellProto));

            if (GetId() == 13493)
                m_scriptValue += pdamage;
            break;
        }
        case SPELL_AURA_PERIODIC_LEECH:
        case SPELL_AURA_PERIODIC_HEALTH_FUNNEL:
        {
            // don't damage target if not alive, possible death persistent effects
            if (!target->IsAlive())
                break;

            Unit* pCaster = GetCaster();
            if (!pCaster)
                break;

            if (!pCaster->IsAlive())
                break;

            uint32 pdamage = (m_modifier.m_amount > 0 ? uint32(m_modifier.m_amount) : 0);
            OnPeriodicCalculateAmount(pdamage);

            if (spellProto->Effect[GetEffIndex()] == SPELL_EFFECT_PERSISTENT_AREA_AURA &&
                    Unit::SpellHitResult(pCaster, target, spellProto, (1 << GetEffIndex()), false) != SPELL_MISS_NONE)
                break;

            // Check for immune
            if (!spellProto->HasAttribute(SPELL_ATTR_NO_IMMUNITIES))
            {
                if (target->IsImmuneToDamage(GetSpellSchoolMask(spellProto)))
                {
                    Unit::SendSpellOrDamageImmune(GetCasterGuid(), target, spellProto->Id);
                    break;
                }
            }

            uint32 absorb = 0;
            int32 resist = 0;

            pdamage = target->SpellDamageBonusTaken(pCaster, GetSpellSchoolMask(spellProto), spellProto, GetEffIndex(), pdamage, DOT, GetStackAmount());

            target->CalculateDamageAbsorbAndResist(pCaster, GetSpellSchoolMask(spellProto), DOT, pdamage, &absorb, &resist, IsReflectableSpell(spellProto), IsResistableSpell(spellProto));

            DETAIL_FILTER_LOG(LOG_FILTER_PERIODIC_AFFECTS, "PeriodicTick: %s health leech of %s for %u dmg inflicted by %u abs is %u",
                              GetCasterGuid().GetString().c_str(), target->GetGuidStr().c_str(), pdamage, GetId(), absorb);

            Unit::DealDamageMods(pCaster, target, pdamage, &absorb, DOT, spellProto);

            Unit::SendSpellNonMeleeDamageLog(pCaster, target, GetId(), pdamage, GetSpellSchoolMask(spellProto), absorb, resist, true, 0);

            float multiplier = spellProto->EffectMultipleValue[GetEffIndex()] > 0 ? spellProto->EffectMultipleValue[GetEffIndex()] : 1;

            // Set trigger flag
            uint32 procAttacker = PROC_FLAG_DEAL_HARMFUL_PERIODIC; // | PROC_FLAG_SUCCESSFUL_HARMFUL_SPELL_HIT;
            uint32 procVictim   = PROC_FLAG_TAKE_HARMFUL_PERIODIC;// | PROC_FLAG_TAKEN_HARMFUL_SPELL_HIT;

            const uint32 bonus = (resist < 0 ? uint32(std::abs(resist)) : 0);
            pdamage += bonus;
            const uint32 malus = (resist > 0 ? (absorb + uint32(resist)) : absorb);
            pdamage = (pdamage <= malus ? 0 : (pdamage - malus));

            pdamage = std::min(pdamage, target->GetHealth());

            if (pdamage)
                procVictim |= PROC_FLAG_TAKE_ANY_DAMAGE;

            CleanDamage cleanDamage(pdamage, BASE_ATTACK, MELEE_HIT_NORMAL, pdamage || absorb);
            int32 new_damage = Unit::DealDamage(pCaster, target, pdamage, &cleanDamage, DOT, GetSpellSchoolMask(spellProto), spellProto, false);
            Unit::ProcDamageAndSpell(ProcSystemArguments(pCaster, target, procAttacker, procVictim, PROC_EX_NORMAL_HIT, pdamage, absorb, BASE_ATTACK, spellProto));

            if (!target->IsAlive() && pCaster->IsNonMeleeSpellCasted(false))
                for (uint32 i = CURRENT_FIRST_NON_MELEE_SPELL; i < CURRENT_MAX_SPELL; ++i)
                    if (Spell* spell = pCaster->GetCurrentSpell(CurrentSpellTypes(i)))
                        if (spell->m_spellInfo->Id == GetId())
                            spell->cancel();

            if (Player* modOwner = pCaster->GetSpellModOwner())
            {
                modOwner->ApplySpellMod(GetId(), SPELLMOD_ALL_EFFECTS, new_damage);
                modOwner->ApplySpellMod(GetId(), SPELLMOD_MULTIPLE_VALUE, multiplier);
            }

            uint32 heal = pCaster->SpellHealingBonusTaken(pCaster, spellProto, GetEffIndex(), int32(new_damage * multiplier), DOT, GetStackAmount());

            int32 gain = pCaster->DealHeal(pCaster, heal, spellProto);
            // Health Leech effects do not generate healing aggro
            if (m_modifier.m_auraname == SPELL_AURA_PERIODIC_LEECH)
                break;
            pCaster->getHostileRefManager().threatAssist(pCaster, gain * 0.5f * sSpellMgr.GetSpellThreatMultiplier(spellProto), spellProto, false, true);
            break;
        }
        case SPELL_AURA_PERIODIC_HEAL:
        case SPELL_AURA_OBS_MOD_HEALTH:
        {
            Unit* pCaster = GetCaster();
            if (!pCaster)
                break;

            // don't heal target if max health or if not alive, mostly death persistent effects from items
            if (!target->IsAlive() || (target->GetHealth() == target->GetMaxHealth()))
                break;

            // heal for caster damage (must be alive)
            if (target != pCaster && spellProto->SpellVisual == 163 && !pCaster->IsAlive())
                break;

            // ignore non positive values (can be result apply spellmods to aura damage
            uint32 amount = m_modifier.m_amount > 0 ? m_modifier.m_amount : 0;

            uint32 pdamage;

            if (m_modifier.m_auraname == SPELL_AURA_OBS_MOD_HEALTH)
                pdamage = uint32(target->GetMaxHealth() * amount / 100);
            else
                pdamage = amount;

            OnPeriodicCalculateAmount(pdamage);

            // first aid confirmed to work through immune when selfcast
            if (pCaster != target && target->IsImmuneToSchool(spellProto, (1 << GetEffIndex()))) // TODO: Check if we need to check for avoiding immune
            {
                Unit::SendSpellOrDamageImmune(GetCasterGuid(), target, spellProto->Id);
                break;
            }

            pdamage = target->SpellHealingBonusTaken(pCaster, spellProto, GetEffIndex(), pdamage, DOT, GetStackAmount());

            DETAIL_FILTER_LOG(LOG_FILTER_PERIODIC_AFFECTS, "PeriodicTick: %s heal of %s for %u health inflicted by %u",
                GetCasterGuid().GetString().c_str(), target->GetGuidStr().c_str(), pdamage, GetId());

            int32 gain = target->ModifyHealth(pdamage);
            SpellPeriodicAuraLogInfo pInfo(this, pdamage, 0, 0, 0.0f);
            target->SendPeriodicAuraLog(&pInfo);

            // Set trigger flag
            uint32 procAttacker = PROC_FLAG_DEAL_HARMFUL_PERIODIC;
            uint32 procVictim = PROC_FLAG_TAKE_HARMFUL_PERIODIC;
            uint32 procEx = PROC_EX_NORMAL_HIT | PROC_EX_INTERNAL_HOT;

            if (pCaster->IsInCombat() && !pCaster->IsCrowdControlled())
                target->getHostileRefManager().threatAssist(pCaster, float(gain) * 0.5f * sSpellMgr.GetSpellThreatMultiplier(spellProto), spellProto, false, true);

            Unit::ProcDamageAndSpell(ProcSystemArguments(pCaster, target, procAttacker, procVictim, procEx, gain, 0, BASE_ATTACK, spellProto, nullptr, gain, true));
            break;
        }
        case SPELL_AURA_PERIODIC_MANA_LEECH:
        {
            // don't damage target if not alive, possible death persistent effects
            if (!target->IsAlive())
                break;

            if (m_modifier.m_miscvalue < 0 || m_modifier.m_miscvalue >= MAX_POWERS)
                break;

            Powers power = Powers(m_modifier.m_miscvalue);

            // power type might have changed between aura applying and tick (druid's shapeshift)
            if (target->GetPowerType() != power)
                break;

            Unit* pCaster = GetCaster();
            if (!pCaster)
                break;

            if (!pCaster->IsAlive())
                break;

            // ignore non positive values (can be result apply spellmods to aura damage
            uint32 pdamage = m_modifier.m_amount > 0 ? m_modifier.m_amount : 0;
            OnPeriodicCalculateAmount(pdamage);

            if (GetSpellProto()->Effect[GetEffIndex()] == SPELL_EFFECT_PERSISTENT_AREA_AURA &&
                    Unit::SpellHitResult(pCaster, target, spellProto, (1 << GetEffIndex()), false) != SPELL_MISS_NONE)
                break;

            // Check for immune (not use charges)
            if (!spellProto->HasAttribute(SPELL_ATTR_NO_IMMUNITIES)) // confirmed Impaling spine goes through immunity
            {
                if (target->IsImmuneToDamage(GetSpellSchoolMask(spellProto)))
                {
                    Unit::SendSpellOrDamageImmune(GetCasterGuid(), target, spellProto->Id);
                    break;
                }
            }

            DETAIL_FILTER_LOG(LOG_FILTER_PERIODIC_AFFECTS, "PeriodicTick: %s power leech of %s for %u dmg inflicted by %u",
                              GetCasterGuid().GetString().c_str(), target->GetGuidStr().c_str(), pdamage, GetId());

            int32 drain_amount = target->GetPower(power) > pdamage ? pdamage : target->GetPower(power);

            target->ModifyPower(power, -drain_amount);

            float gain_multiplier = 0;

            if (pCaster->GetMaxPower(power) > 0)
            {
                gain_multiplier = spellProto->EffectMultipleValue[GetEffIndex()];

                if (Player* modOwner = pCaster->GetSpellModOwner())
                    modOwner->ApplySpellMod(GetId(), SPELLMOD_MULTIPLE_VALUE, gain_multiplier);
            }

            SpellPeriodicAuraLogInfo pInfo(this, drain_amount, 0, 0, gain_multiplier);
            target->SendPeriodicAuraLog(&pInfo);

            int32 gain_amount = int32(drain_amount * gain_multiplier);

            if (gain_amount)
            {
                int32 gain = pCaster->ModifyPower(power, gain_amount);
                target->AddThreat(pCaster, float(gain) * 0.5f, false, GetSpellSchoolMask(spellProto), spellProto);
            }
            break;
        }
        case SPELL_AURA_PERIODIC_ENERGIZE:
        {
            // don't energize target if not alive, possible death persistent effects
            if (!target->IsAlive())
                break;

            Unit* pCaster = GetCaster();

            // ignore non positive values (can be result apply spellmods to aura damage
            uint32 pdamage = m_modifier.m_amount > 0 ? m_modifier.m_amount : 0;
            OnPeriodicCalculateAmount(pdamage);

            if (pCaster)
            {
                if (target->IsImmuneToSchool(spellProto, (1 << GetEffIndex())))
                {
                    Unit::SendSpellOrDamageImmune(GetCasterGuid(), target, spellProto->Id);
                    break;
                }
            }

            DETAIL_FILTER_LOG(LOG_FILTER_PERIODIC_AFFECTS, "PeriodicTick: %s energize %s for %u dmg inflicted by %u",
                              GetCasterGuid().GetString().c_str(), target->GetGuidStr().c_str(), pdamage, GetId());

            if (m_modifier.m_miscvalue < 0 || m_modifier.m_miscvalue >= MAX_POWERS)
                break;

            Powers power = Powers(m_modifier.m_miscvalue);

            if (target->GetMaxPower(power) == 0)
                break;

            SpellPeriodicAuraLogInfo info(this, pdamage, 0, 0, 0.0f);
            target->SendPeriodicAuraLog(&info);

            int32 gain = target->ModifyPower(power, pdamage);

            if (pCaster)
                target->getHostileRefManager().threatAssist(pCaster, float(gain) * 0.5f * sSpellMgr.GetSpellThreatMultiplier(spellProto), spellProto, false, true);

            if (GetId() == 25685) // Moam - Energize
                if (target->GetPower(POWER_MANA) > 24000)
                    target->RemoveAurasDueToSpell(GetId());
            break;
        }
        case SPELL_AURA_OBS_MOD_MANA:
        {
            // don't energize target if not alive, possible death persistent effects
            if (!target->IsAlive())
                break;

            Unit* pCaster = GetCaster();

            // ignore non positive values (can be result apply spellmods to aura damage
            uint32 amount = m_modifier.m_amount > 0 ? m_modifier.m_amount : 0;

            uint32 pdamage = uint32(target->GetMaxPower(POWER_MANA) * amount / 100);
            OnPeriodicCalculateAmount(pdamage);

            if (pCaster)
            {
                if (target->IsImmuneToSchool(spellProto, (1 << GetEffIndex())))
                {
                    Unit::SendSpellOrDamageImmune(GetCasterGuid(), target, spellProto->Id);
                    break;
                }
            }

            DETAIL_FILTER_LOG(LOG_FILTER_PERIODIC_AFFECTS, "PeriodicTick: %s energize %s for %u mana inflicted by %u",
                              GetCasterGuid().GetString().c_str(), target->GetGuidStr().c_str(), pdamage, GetId());

            if (target->GetMaxPower(POWER_MANA) == 0)
                break;

            SpellPeriodicAuraLogInfo pInfo(this, pdamage, 0, 0, 0.0f);
            target->SendPeriodicAuraLog(&pInfo);

            int32 gain = target->ModifyPower(POWER_MANA, pdamage);

            if (pCaster)
                target->getHostileRefManager().threatAssist(pCaster, float(gain) * 0.5f * sSpellMgr.GetSpellThreatMultiplier(spellProto), spellProto, false, true);
            break;
        }
        case SPELL_AURA_POWER_BURN_MANA:
        {
            // don't mana burn target if not alive, possible death persistent effects
            if (!target->IsAlive())
                break;

            Unit* pCaster = GetCaster();
            if (!pCaster)
                break;

            uint32 pdamage = m_modifier.m_amount > 0 ? m_modifier.m_amount : 0;
            OnPeriodicCalculateAmount(pdamage);

            // Check for immune (not use charges)
            if (!spellProto->HasAttribute(SPELL_ATTR_NO_IMMUNITIES)) // confirmed Impaling spine goes through immunity
            {
                if (target->IsImmuneToDamage(GetSpellSchoolMask(spellProto)))
                {
                    Unit::SendSpellOrDamageImmune(GetCasterGuid(), target, spellProto->Id);
                    break;
                }
            }

            Powers powerType = Powers(m_modifier.m_miscvalue);

            if (!target->IsAlive() || target->GetPowerType() != powerType)
                break;

            uint32 gain = uint32(-target->ModifyPower(powerType, -int32(pdamage)));

            gain = uint32(gain * spellProto->EffectMultipleValue[GetEffIndex()]);

            // maybe has to be sent different to client, but not by SMSG_PERIODICAURALOG
            SpellNonMeleeDamage spellDamageInfo(pCaster, target, spellProto->Id, SpellSchools(spellProto->School));
            spellDamageInfo.periodicLog = true;

            pCaster->CalculateSpellDamage(&spellDamageInfo, gain, spellProto, GetEffIndex());

            spellDamageInfo.target->CalculateAbsorbResistBlock(pCaster, &spellDamageInfo, spellProto);

            Unit::DealDamageMods(pCaster, spellDamageInfo.target, spellDamageInfo.damage, &spellDamageInfo.absorb, SPELL_DIRECT_DAMAGE, spellProto);

            pCaster->SendSpellNonMeleeDamageLog(&spellDamageInfo);

            // Set trigger flag
            uint32 procAttacker = PROC_FLAG_DEAL_HARMFUL_PERIODIC; //  | PROC_FLAG_SUCCESSFUL_HARMFUL_SPELL_HIT;
            uint32 procVictim   = PROC_FLAG_TAKE_HARMFUL_PERIODIC;// | PROC_FLAG_TAKEN_HARMFUL_SPELL_HIT;
            uint32 procEx       = createProcExtendMask(&spellDamageInfo, SPELL_MISS_NONE);
            if (spellDamageInfo.damage)
                procVictim |= PROC_FLAG_TAKE_ANY_DAMAGE;

            Unit::DealSpellDamage(pCaster, &spellDamageInfo, true, false);

            Unit::ProcDamageAndSpell(ProcSystemArguments(pCaster, spellDamageInfo.target, procAttacker, procVictim, procEx, spellDamageInfo.damage, spellDamageInfo.absorb, BASE_ATTACK, spellProto));
            break;
        }
        case SPELL_AURA_MOD_REGEN:
        {
            // don't heal target if not alive, possible death persistent effects
            if (!target->IsAlive())
                break;

            int32 gain = target->ModifyHealth(m_modifier.m_amount);
            if (Unit* caster = GetCaster())
                target->getHostileRefManager().threatAssist(caster, float(gain) * 0.5f  * sSpellMgr.GetSpellThreatMultiplier(spellProto), spellProto, false, true);
            break;
        }
        case SPELL_AURA_MOD_POWER_REGEN:
        {
            // don't energize target if not alive, possible death persistent effects
            if (!target->IsAlive())
                break;

            Powers powerType = target->GetPowerType();
            if (int32(powerType) != m_modifier.m_miscvalue)
                break;

            if (spellProto->AuraInterruptFlags & AURA_INTERRUPT_FLAG_STANDING_CANCELS)
            {
                // eating anim
                target->HandleEmoteCommand(EMOTE_ONESHOT_EAT);
            }
            else if (GetId() == 20577)
            {
                // cannibalize anim
                target->HandleEmoteCommand(EMOTE_STATE_CANNIBALIZE);
            }

            // Anger Management
            // amount = 1+ 16 = 17 = 3,4*5 = 10,2*5/3
            // so 17 is rounded amount for 5 sec tick grow ~ 1 range grow in 3 sec
            if (powerType == POWER_RAGE && target->IsInCombat())
                target->ModifyPower(powerType, m_modifier.m_amount * 3 / 5);
            break;
        }
        // Here tick dummy auras
        case SPELL_AURA_DUMMY:                              // some spells have dummy aura
        {
            PeriodicDummyTick();
            break;
        }
        case SPELL_AURA_PERIODIC_TRIGGER_SPELL:
        {
            TriggerSpell();
            break;
        }
        default:
            break;
    }
    OnPeriodicTickEnd();
}

void Aura::PeriodicDummyTick()
{
    SpellEntry const* spell = GetSpellProto();
    Unit* target = GetTarget();
    switch (spell->SpellFamilyName)
    {
        case SPELLFAMILY_GENERIC:
        {
            switch (spell->Id)
            {
                case 6946:                                  // Curse of the Bleakheart
                case 41170:
                {
                    if (roll_chance_i(5))
                    {
                        int32 damageValue = target->CalculateSpellEffectValue(target, spell, EFFECT_INDEX_1);
                        target->CastCustomSpell(nullptr, spell->Id == 6946 ? 6945 : 41356, nullptr, &damageValue, nullptr, TRIGGERED_OLD_TRIGGERED);
                    }
                    return;
                }
                case 7057:                                  // Haunting Spirits
                    if (roll_chance_i(33))
                        target->CastSpell(target, m_modifier.m_amount, TRIGGERED_OLD_TRIGGERED, nullptr, this);
                    return;
                case 21094:                                 // Separation Anxiety (Majordomo Executus)
                case 23487:                                 // Separation Anxiety (Garr)
                    if (Unit* caster = GetCaster())
                    {
                        float m_radius = GetSpellRadius(sSpellRadiusStore.LookupEntry(spell->EffectRadiusIndex[m_effIndex]));
                        if (caster->IsAlive() && !caster->IsWithinDistInMap(target, m_radius))
                            target->CastSpell(target, (spell->Id == 21094 ? 21095 : 23492), TRIGGERED_OLD_TRIGGERED, nullptr);      // Spell 21095: Separation Anxiety for Majordomo Executus' adds, 23492: Separation Anxiety for Garr's adds
                    }
                    return;
            }
            break;
        }
        default:
            break;
    }

    OnPeriodicDummy();

    if (Unit* caster = GetCaster())
    {
        if (target && target->GetTypeId() == TYPEID_UNIT)
            sScriptDevAIMgr.OnEffectDummy(caster, GetId(), GetEffIndex(), (Creature*)target, ObjectGuid());
    }
}

void Aura::HandlePreventFleeing(bool apply, bool Real)
{
    if (!Real)
        return;

    Unit::AuraList const& fearAuras = GetTarget()->GetAurasByType(SPELL_AURA_MOD_FEAR);
    if (!fearAuras.empty())
    {
        const Aura* first = fearAuras.front();
        if (apply)
            GetTarget()->SetFleeing(false);
        else
            GetTarget()->SetFleeing(true, first->GetCasterGuid(), first->GetId());
    }
}

void Aura::HandleManaShield(bool apply, bool Real)
{
    if (!Real)
        return;

    // prevent double apply bonuses
    if (apply && (GetTarget()->GetTypeId() != TYPEID_PLAYER || !((Player*)GetTarget())->GetSession()->PlayerLoading()))
    {
        if (Unit* caster = GetCaster())
        {
            float DoneActualBenefit = 0.0f;
            switch (GetSpellProto()->SpellFamilyName)
            {
                case SPELLFAMILY_MAGE:
                    if (GetSpellProto()->SpellFamilyFlags & uint64(0x0000000000008000))
                    {
                        // Mana Shield
                        // +50% from +spd bonus
                        DoneActualBenefit = caster->SpellBaseDamageBonusDone(GetSpellSchoolMask(GetSpellProto())) * 0.5f;
                        break;
                    }
                    break;
                default:
                    break;
            }

            DoneActualBenefit *= caster->CalculateLevelPenalty(GetSpellProto());

            m_modifier.m_amount += (int32)DoneActualBenefit;
        }
    }
}

void Aura::HandleOverrideClassScript(bool apply, bool real)
{
    GetTarget()->RegisterOverrideScriptAura(this, m_modifier.m_miscvalue, apply);
}

bool Aura::IsLastAuraOnHolder()
{
    for (int32 i = 0; i < MAX_EFFECT_INDEX; ++i)
        if (i != GetEffIndex() && GetHolder()->m_auras[i])
            return false;
    return true;
}

void Aura::HandleInterruptRegen(bool apply, bool Real)
{
    if (!Real)
        return;

    if (GetSpellProto()->Id != 5229 && GetSpellProto()->Id != 29131)
        return;

    GetTarget()->SetInDummyCombatState(apply);
}

inline bool IsRemovedOnShapeshiftLost(SpellEntry const* spellproto, ObjectGuid const& casterGuid, ObjectGuid const& targetGuid)
{
    if (casterGuid == targetGuid)
    {
        if (spellproto->HasAttribute(SPELL_ATTR_AURA_IS_DEBUFF))
            return false;

        if (spellproto->Stances)
        {
            switch (spellproto->Id)
            {
                case 11327: // vanish stealth aura improvements are removed on stealth removal
                case 11329: // but they have attribute SPELL_ATTR_NOT_SHAPESHIFT
                    // maybe relic from when they had Effect1?
                    return true;
                default:
                    break;
            }

            if (!spellproto->HasAttribute(SPELL_ATTR_EX2_ALLOW_WHILE_NOT_SHAPESHIFTED) && !spellproto->HasAttribute(SPELL_ATTR_NOT_SHAPESHIFT))
                return true;
        }
        else if (spellproto->SpellFamilyName == SPELLFAMILY_DRUID && spellproto->EffectApplyAuraName[0] == SPELL_AURA_MOD_DODGE_PERCENT)
            return true;
    }

    return false;
    /*TODO: investigate spellid 24864  or (SpellFamilyName = 7 and EffectApplyAuraName_1 = 49 and stances = 0)*/
}

SpellAuraHolder::SpellAuraHolder(SpellEntry const* spellproto, Unit* target, WorldObject* caster, Item* castItem, SpellEntry const* triggeredBy) :
    m_spellProto(spellproto), m_target(target),
    m_castItemGuid(castItem ? castItem->GetObjectGuid() : ObjectGuid()), m_triggeredBy(triggeredBy),
    m_spellAuraHolderState(SPELLAURAHOLDER_STATE_CREATED), m_auraSlot(MAX_AURAS),
    m_auraLevel(1), m_procCharges(0),
    m_stackAmount(1), m_timeCla(1000), m_removeMode(AURA_REMOVE_BY_DEFAULT),
    m_AuraDRGroup(DIMINISHING_NONE), m_permanent(false), m_isRemovedOnShapeLost(true),
    m_heartbeatResistChance(0), m_heartbeatResistTimer(0), m_heartbeatResistInterval(0),
    m_deleted(false), m_skipUpdate(false),
    m_auraScript(SpellScriptMgr::GetAuraScript(spellproto->Id))
{
    MANGOS_ASSERT(target);
    MANGOS_ASSERT(spellproto && spellproto == sSpellTemplate.LookupEntry<SpellEntry>(spellproto->Id) && "`info` must be pointer to sSpellTemplate element");

    if (!caster)
        m_casterGuid = target->GetObjectGuid();
    else
        m_casterGuid = caster->GetObjectGuid();

    m_applyTime      = time(nullptr);
    m_applyMSTime    = target->GetMap()->GetCurrentMSTime();
    m_isPassive      = IsPassiveSpell(spellproto);
    m_isDeathPersist = IsDeathPersistentSpell(spellproto);
    m_trackedAuraType = sSpellMgr.IsSingleTargetSpell(spellproto) ? TRACK_AURA_TYPE_SINGLE_TARGET : TRACK_AURA_TYPE_NOT_TRACKED;
    m_procCharges    = spellproto->procCharges;

    m_isRemovedOnShapeLost = IsRemovedOnShapeshiftLost(m_spellProto, GetCasterGuid(), target->GetObjectGuid());

    Unit* unitCaster = caster && caster->isType(TYPEMASK_UNIT) ? (Unit*)caster : nullptr;

    m_duration = m_maxDuration = CalculateSpellDuration(spellproto, unitCaster, target, m_auraScript);

    if (m_maxDuration == -1 || (m_isPassive && spellproto->DurationIndex == 0))
        m_permanent = true;

    if (unitCaster)
    {
        if (Player* modOwner = unitCaster->GetSpellModOwner())
            modOwner->ApplySpellMod(GetId(), SPELLMOD_CHARGES, m_procCharges);
    }

    // some custom stack values at aura holder create
    switch (m_spellProto->Id)
    {
        // some auras applied with max stack
        case 24575:                                         // Brittle Armor
        case 24659:                                         // Unstable Power
        case 24662:                                         // Restless Strength
        case 26464:                                         // Mercurial Shield
            m_stackAmount = m_spellProto->StackAmount;
            break;
    }

    for (auto& m_aura : m_auras)
        m_aura = nullptr;

    OnHolderInit(caster);
}

void SpellAuraHolder::AddAura(Aura* aura, SpellEffectIndex index)
{
    m_auras[index] = aura;
}

void SpellAuraHolder::RemoveAura(SpellEffectIndex index)
{
    m_auras[index] = nullptr;
}

void SpellAuraHolder::ApplyAuraModifiers(bool apply, bool real)
{
    for (int32 i = 0; i < MAX_EFFECT_INDEX && !IsDeleted(); ++i)
        if (Aura* aur = GetAuraByEffectIndex(SpellEffectIndex(i)))
            aur->ApplyModifier(apply, real);
}

void SpellAuraHolder::_AddSpellAuraHolder()
{
    if (!GetId())
        return;
    if (!m_target)
        return;

    // Try find slot for aura
    uint8 slot = NULL_AURA_SLOT;
    Unit* caster = GetCaster();

    // Lookup free slot
    // will be < MAX_AURAS slot (if find free) with !secondaura
    if (IsNeedVisibleSlot(caster))
    {
        if (IsPositive()) // empty positive slot
        {
            for (uint8 i = 0; i < MAX_POSITIVE_AURAS; i++)
            {
                if (m_target->GetUInt32Value((uint16)(UNIT_FIELD_AURA + i)) == 0)
                {
                    slot = i;
                    break;
                }
            }
        }
        else                                                // empty negative slot
        {
            for (uint8 i = MAX_POSITIVE_AURAS; i < MAX_AURAS; i++)
            {
                if (m_target->GetUInt32Value((uint16)(UNIT_FIELD_AURA + i)) == 0)
                {
                    slot = i;
                    break;
                }
            }
        }
    }

    SetAuraSlot(slot);

    // Not update fields for not first spell's aura, all data already in fields
    if (slot < MAX_AURAS)                                   // slot found
    {
        SetAura(slot, false);
        SetAuraFlag(slot, true);
        SetAuraLevel(slot, caster ? caster->GetLevel() : sWorld.getConfig(CONFIG_UINT32_MAX_PLAYER_LEVEL));
        UpdateAuraApplication();

        // update for out of range group members
        m_target->UpdateAuraForGroup(slot);

        UpdateAuraDuration();
    }

    //*****************************************************
    // Update target aura state flag (at 1 aura apply)
    // TODO: Make it easer
    //*****************************************************
    // Sitdown on apply aura req seated
    if (m_spellProto->AuraInterruptFlags & AURA_INTERRUPT_FLAG_STANDING_CANCELS && !m_target->IsSitState())
        m_target->SetStandState(UNIT_STAND_STATE_SIT);

    if (m_spellProto->HasAttribute(SPELL_ATTR_EX_PREVENTS_ANIM))
        m_target->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PREVENT_ANIM);

    // register aura diminishing on apply
    if (getDiminishGroup() != DIMINISHING_NONE)
        m_target->ApplyDiminishingAura(getDiminishGroup(), true);

    // Update Seals information
    if (IsSealSpell(GetSpellProto()))
        m_target->ModifyAuraState(AURA_STATE_JUDGEMENT, true);
}

void SpellAuraHolder::_RemoveSpellAuraHolder()
{
    // Remove all triggered by aura spells vs unlimited duration
    // except same aura replace case
    if (m_removeMode != AURA_REMOVE_BY_STACK)
        CleanupTriggeredSpells();

    Unit* caster = GetCaster();

    if (caster && IsPersistent())
    {
        DynamicObject* dynObj = caster->GetDynObject(GetId());
        if (dynObj)
            dynObj->RemoveAffected(m_target);
    }

    // passive auras do not get put in slots - said who? ;)
    // Note: but totem can be not accessible for aura target in time remove (to far for find in grid)
    // if(m_isPassive && !(caster && caster->GetTypeId() == TYPEID_UNIT && ((Creature*)caster)->IsTotem()))
    //    return;

    uint8 slot = GetAuraSlot();

    if (slot >= MAX_AURAS)                                  // slot not set
        return;

    if (m_target->GetUInt32Value((uint16)(UNIT_FIELD_AURA + slot)) == 0)
        return;

    // unregister aura diminishing (and store last time)
    if (getDiminishGroup() != DIMINISHING_NONE)
        m_target->ApplyDiminishingAura(getDiminishGroup(), false);

    SetAura(slot, true);
    SetAuraFlag(slot, false);
    SetAuraLevel(slot, caster ? caster->GetLevel() : sWorld.getConfig(CONFIG_UINT32_MAX_PLAYER_LEVEL));

    m_procCharges = 0;
    m_stackAmount = 1;
    UpdateAuraApplication();

    if (m_removeMode != AURA_REMOVE_BY_DELETE)
    {
        // update for out of range group members
        m_target->UpdateAuraForGroup(slot);

        //*****************************************************
        // Update target aura state flag (at last aura remove)
        //*****************************************************
        uint32 removeState = 0;
        ClassFamilyMask removeFamilyFlag = m_spellProto->SpellFamilyFlags;
        switch (m_spellProto->SpellFamilyName)
        {
            case SPELLFAMILY_PALADIN:
                if (IsSealSpell(m_spellProto))
                    removeState = AURA_STATE_JUDGEMENT;     // Update Seals information
                break;
        }

        // Remove state (but need check other auras for it)
        if (removeState)
            // this has been last aura
            if (!m_target->HasAuraWithCondition([spellProto = m_spellProto, removeFamilyFlag](SpellAuraHolder* holder)
            {
                SpellEntry const* auraSpellInfo = holder->GetSpellProto();
                if (auraSpellInfo->IsFitToFamily(SpellFamily(spellProto->SpellFamilyName), removeFamilyFlag))
                    return true;
                return false;
            }))
                m_target->ModifyAuraState(AuraState(removeState), false);

        if (m_spellProto->HasAttribute(SPELL_ATTR_EX_PREVENTS_ANIM))
            if (!m_target->HasAuraWithCondition([](SpellAuraHolder* holder)
            {
                if (holder->GetSpellProto()->HasAttribute(SPELL_ATTR_EX_PREVENTS_ANIM))
                    return true;
                return false;
            }))
                m_target->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PREVENT_ANIM);

        // reset cooldown state for spells
        if (caster && GetSpellProto()->HasAttribute(SPELL_ATTR_COOLDOWN_ON_EVENT))
        {
            // some spells need to start cooldown at aura fade (like stealth)
            caster->AddCooldown(*GetSpellProto());
        }
    }
}

void SpellAuraHolder::CleanupTriggeredSpells()
{
    for (int32 i = 0; i < MAX_EFFECT_INDEX; ++i)
    {
        if (!m_spellProto->EffectApplyAuraName[i])
            continue;

        uint32 tSpellId = m_spellProto->EffectTriggerSpell[i];
        if (!tSpellId)
            continue;

        SpellEntry const* tProto = sSpellTemplate.LookupEntry<SpellEntry>(tSpellId);
        if (!tProto)
            continue;

        if (GetSpellDuration(tProto) != -1)
            continue;

        // needed for spell 43680, maybe others
        // TODO: is there a spell flag, which can solve this in a more sophisticated way?
        if (m_spellProto->EffectApplyAuraName[i] == SPELL_AURA_PERIODIC_TRIGGER_SPELL &&
                GetSpellDuration(m_spellProto) == int32(m_spellProto->EffectAmplitude[i]))
            continue;

        m_target->RemoveAurasDueToSpell(tSpellId);
    }
}

bool SpellAuraHolder::ModStackAmount(int32 num, Unit* newCaster)
{
    uint32 protoStackAmount = m_spellProto->StackAmount;

    // Can`t mod
    if (!protoStackAmount)
        return true;

    // Modify stack but limit it
    int32 stackAmount = m_stackAmount + num;
    if (stackAmount > (int32)protoStackAmount)
        stackAmount = protoStackAmount;
    else if (stackAmount <= 0) // Last aura from stack removed
    {
        m_stackAmount = 0;
        return true; // need remove aura
    }

    // Update stack amount
    SetStackAmount(stackAmount, newCaster);
    return false;
}

void SpellAuraHolder::SetStackAmount(uint32 stackAmount, Unit* newCaster)
{
    Unit* target = GetTarget();
    if (!target)
        return;

    bool refresh = false;
    if (stackAmount >= m_stackAmount)
    {
        // Change caster
        Unit* oldCaster = GetCaster();
        if (newCaster && oldCaster != newCaster)
        {
            m_casterGuid = newCaster->GetObjectGuid();
            // New caster duration sent for owner in RefreshHolder
        }

        refresh = true;
    }

    int32 oldStackAmount = m_stackAmount;
    if (stackAmount != m_stackAmount)
    {
        m_stackAmount = stackAmount;
        UpdateAuraApplication();
    }

    for (auto aur : m_auras)
    {
        if (aur)
        {
            int32 baseAmount = aur->GetModifier()->m_baseAmount;
            int32 amount = m_stackAmount * baseAmount;
            amount = aur->OnAuraValueCalculate(newCaster, amount, nullptr);
            // Reapply if amount change
            if (!baseAmount || amount != aur->GetModifier()->m_amount)
            {
                aur->SetRemoveMode(AURA_REMOVE_BY_GAINED_STACK);
                if (IsAuraRemoveOnStacking(this->GetSpellProto(), aur->GetEffIndex()))
                    aur->ApplyModifier(false, true);
                aur->GetModifier()->m_amount = amount;
                aur->GetModifier()->m_recentAmount = baseAmount * (stackAmount - oldStackAmount);
                aur->ApplyModifier(true, true);
            }
        }
    }

    if (refresh) // Stack increased refresh duration
        RefreshHolder();
}

Unit* SpellAuraHolder::GetCaster() const
{
    if (GetCasterGuid() == m_target->GetObjectGuid())
        return m_target;

    if (m_casterGuid.IsGameObject())
        return nullptr;

    if (!m_target->IsInWorld())
        return nullptr;

    return m_target->GetMap()->GetUnit(m_casterGuid);
}

bool SpellAuraHolder::IsWeaponBuffCoexistableWith(SpellAuraHolder const* ref) const
{
    // only item casted spells
    if (!GetCastItemGuid())
        return false;

    // Exclude Debuffs
    if (!IsPositive())
        return false;

    // Exclude Non-generic Buffs and Executioner-Enchant
    if (GetSpellProto()->SpellFamilyName != SPELLFAMILY_GENERIC)
        return false;

    // Exclude Stackable Buffs [ie: Blood Reserve]
    if (GetSpellProto()->StackAmount)
        return false;

    // only self applied player buffs
    if (m_target->GetTypeId() != TYPEID_PLAYER || m_target->GetObjectGuid() != GetCasterGuid())
        return false;

    Item* castItem = ((Player*)m_target)->GetItemByGuid(GetCastItemGuid());
    if (!castItem)
        return false;

    // Limit to Weapon-Slots
    if (!castItem->IsEquipped() ||
            (castItem->GetSlot() != EQUIPMENT_SLOT_MAINHAND && castItem->GetSlot() != EQUIPMENT_SLOT_OFFHAND))
        return false;

    // from different weapons
    return ref->GetCastItemGuid() && ref->GetCastItemGuid() != GetCastItemGuid();
}

bool SpellAuraHolder::IsNeedVisibleSlot(Unit const* caster) const
{
    bool totemAura = caster && caster->GetTypeId() == TYPEID_UNIT && ((Creature*)caster)->IsTotem();

    for (int i = 0; i < MAX_EFFECT_INDEX; ++i)
    {
        if (!m_auras[i])
            continue;

        // special area auras cases
        switch (m_spellProto->Effect[i])
        {
            case SPELL_EFFECT_APPLY_AREA_AURA_PARTY: // passive party auras shown even on caster
                return m_auras[i]->GetModifier()->m_auraname != SPELL_AURA_NONE;
            case SPELL_EFFECT_APPLY_AREA_AURA_PET:
                // passive auras (except totem auras) do not get placed in caster slot
                return (m_target != caster || totemAura || !m_isPassive) && m_auras[i]->GetModifier()->m_auraname != SPELL_AURA_NONE;
            default:
                break;
        }
    }

    // passive auras (except totem auras) do not get placed in the slots
    return !m_isPassive || totemAura;
}

void SpellAuraHolder::HandleSpellSpecificBoosts(bool apply)
{
    std::vector<uint32> boostSpells;

    switch (GetSpellProto()->SpellFamilyName)
    {
        case SPELLFAMILY_GENERIC:
        {
            switch (GetId())
            {
                case 20594: // Stoneform (dwarven racial)
                    boostSpells.push_back(20612);
                    break;
                default:
                    return;
            }
            break;
        }
        case SPELLFAMILY_MAGE:
        {
            //switch (GetId())
            //{
            //    default:
            //        break; // Break here for poly below - 2.4.2+ only player poly regens
            //}
            break;
        }
        case SPELLFAMILY_HUNTER:
        {
            switch (GetId())
            {
                // The Beast Within and Bestial Wrath - immunity
                case 19574:
                {
                    boostSpells.push_back(24395);
                    boostSpells.push_back(24396);
                    boostSpells.push_back(24397);
                    boostSpells.push_back(26592);
                    break;
                }
                default:
                    return;
            }
            break;
        }
        default:
            return;
    }

    if (GetSpellProto()->Mechanic == MECHANIC_POLYMORPH)
        boostSpells.push_back(12939); // Just so that this doesnt conflict with others

    if (boostSpells.empty())
        return;

    for (uint32 spellId : boostSpells)
    {
        Unit* boostCaster = m_target;
        Unit* boostTarget = nullptr;
        ObjectGuid casterGuid = m_target->GetObjectGuid(); // caster can be nullptr, but guid is still valid for removal
        SpellEntry const* boostEntry = sSpellTemplate.LookupEntry<SpellEntry>(spellId);
        for (uint32 target : boostEntry->EffectImplicitTargetA)
        {
            switch (target)
            {
                case TARGET_UNIT_ENEMY:
                case TARGET_UNIT:
                    if (apply) // optimization
                        boostCaster = GetCaster();
                    else
                        casterGuid = GetCasterGuid();
                    boostTarget = m_target;
                    break;
            }
        }
        if (apply)
            boostCaster->CastSpell(boostTarget, boostEntry, TRIGGERED_OLD_TRIGGERED);
        else
            m_target->RemoveAurasByCasterSpell(spellId, casterGuid);
    }
}

void Aura::HandleAuraSafeFall(bool /*Apply*/, bool /*Real*/)
{
    // implemented in WorldSession::HandleMovementOpcodes
}

SpellAuraHolder::~SpellAuraHolder()
{
    // note: auras in delete list won't be affected since they clear themselves from holder when adding to deletedAuraslist
    for (int32 i = 0; i < MAX_EFFECT_INDEX; ++i)
        delete m_auras[i];
}

void SpellAuraHolder::Update(uint32 diff)
{
    if (m_skipUpdate)
    {
        m_skipUpdate = false;
        if (m_applyMSTime == GetTarget()->GetMap()->GetCurrentMSTime()) // do not tick in same tick as created
            return;
    }

    for (auto aura : m_auras)
        if (aura)
            aura->UpdateAura(diff);

    if (m_duration > 0)
    {
        m_duration -= diff;
        if (m_duration < 0)
            m_duration = 0;

        m_timeCla -= diff;

        if (m_timeCla <= 0)
        {
            if (!GetSpellProto()->HasAttribute(SPELL_ATTR_EX2_NO_TARGET_PER_SECOND_COSTS) || GetCasterGuid() == GetTarget()->GetObjectGuid())
            {
                Powers powertype = Powers(GetSpellProto()->powerType);
                int32 manaPerSecond = GetSpellProto()->manaPerSecond + GetSpellProto()->manaPerSecondPerLevel * GetTarget()->GetLevel();
                m_timeCla = 1 * IN_MILLISECONDS;

                if (manaPerSecond)
                {
                    if (powertype == POWER_HEALTH)
                    {
                        if (GetTarget()->GetHealth() <= (uint32)manaPerSecond)
                        {
                            // cannot apply damage part so we have to cancel responsible aura
                            GetTarget()->RemoveAurasDueToSpell(GetId());

                            // finish current generic/channeling spells, don't affect autorepeat
                            GetTarget()->FinishSpell(CURRENT_GENERIC_SPELL);
                            GetTarget()->FinishSpell(CURRENT_CHANNELED_SPELL);
                        }
                        else
                            GetTarget()->ModifyHealth(-manaPerSecond);
                    }
                    else
                        GetTarget()->ModifyPower(powertype, -manaPerSecond);
                }
            }
        }

        if (m_duration && !IsDeleted())
            UpdateHeartbeatResist(diff);
    }
}

void SpellAuraHolder::RefreshHolder()
{
    SetAuraDuration(GetAuraMaxDuration());
    UpdateAuraDuration();
}

void SpellAuraHolder::SetAuraMaxDuration(int32 duration)
{
    m_maxDuration = duration;

    // possible overwrite persistent state
    if (duration > 0)
    {
        if (!(IsPassive() && GetSpellProto()->DurationIndex == 0))
            SetPermanent(false);
    }
}

void SpellAuraHolder::SetAuraCharges(uint32 charges, bool update)
{
    if (m_procCharges == charges)
        return;
    m_procCharges = charges;

    for (uint8 i = 0; i < MAX_EFFECT_INDEX; i++)
        if (m_auras[i])
            if (SpellModifier* spellMod = m_auras[i]->GetSpellModifier())
                spellMod->charges = charges;

    if (update)
        UpdateAuraApplication();
}

bool SpellAuraHolder::DropAuraCharge()
{
    if (m_procCharges == 0)
        return false;

    --m_procCharges;

    for (uint8 i = 0; i < MAX_EFFECT_INDEX; i++)
        if (m_auras[i])
            if (SpellModifier* spellMod = m_auras[i]->GetSpellModifier())
                spellMod->charges = m_procCharges;

    UpdateAuraApplication();

    if (GetCasterGuid() != m_target->GetObjectGuid() && IsAreaAura())
        if (Unit* caster = GetCaster())
            caster->RemoveAuraCharge(m_spellProto->Id);

    return m_procCharges == 0;
}

void SpellAuraHolder::ResetSpellModCharges()
{
    for (uint8 i = 0; i < MAX_EFFECT_INDEX; i++)
        if (m_auras[i])
            if (SpellModifier* spellMod = m_auras[i]->GetSpellModifier())
                spellMod->charges = m_procCharges;
}

bool SpellAuraHolder::HasModifier(const uint64& modId) const
{
    for (uint8 i = 0; i < MAX_EFFECT_INDEX; i++)
        if (m_auras[i] && m_auras[i]->GetSpellModifier())
            if (m_auras[i]->GetSpellModifier()->modId == modId)
                return true;
    return false;
}

bool SpellAuraHolder::HasMechanic(uint32 mechanic) const
{
    if (mechanic == m_spellProto->Mechanic)
        return true;

    for (int32 i = 0; i < MAX_EFFECT_INDEX; ++i)
        if (m_auras[i] && m_spellProto->EffectMechanic[i] == mechanic)
            return true;
    return false;
}

bool SpellAuraHolder::HasMechanicMask(uint32 mechanicMask) const
{
    if (mechanicMask & (1 << (m_spellProto->Mechanic - 1)))
        return true;

    for (int32 i = 0; i < MAX_EFFECT_INDEX; ++i)
        if (m_auras[i] && m_spellProto->EffectMechanic[i] && ((1 << (m_spellProto->EffectMechanic[i] - 1)) & mechanicMask))
            return true;
    return false;
}

bool SpellAuraHolder::IsDispellableByMask(uint32 dispelMask, Unit const* caster, SpellEntry const* spellInfo) const
{
    if ((1 << GetSpellProto()->Dispel) & dispelMask)
    {
        if (GetSpellProto()->Dispel == DISPEL_MAGIC)
        {
            // do not remove positive auras if friendly target
            //               negative auras if non-friendly target
            bool positive = IsPositive();
            if (positive == caster->CanAssistSpell(GetTarget(), spellInfo))
                if (positive || !IsCharm())
                    return false;
        }
        return true;
    }
    return false;
}

bool SpellAuraHolder::IsPersistent() const
{
    for (auto aur : m_auras)
        if (aur)
            if (aur->IsPersistent())
                return true;
    return false;
}

bool SpellAuraHolder::IsAreaAura() const
{
    for (auto aur : m_auras)
        if (aur)
            if (aur->IsAreaAura())
                return true;
    return false;
}

bool SpellAuraHolder::IsPositive() const
{
    for (auto aur : m_auras)
        if (aur)
            if (!aur->IsPositive())
                return false;
    return true;
}

bool SpellAuraHolder::IsEmptyHolder() const
{
    for (auto m_aura : m_auras)
        if (m_aura)
            return false;
    return true;
}

bool SpellAuraHolder::IsSaveToDbHolder() const
{
    if (IsPassive() || IsChanneledSpell(GetSpellProto()))
        return false;

    if (GetTrackedAuraType() == TRACK_AURA_TYPE_SINGLE_TARGET)
        return false;

    if (m_spellProto->AuraInterruptFlags & AURA_INTERRUPT_FLAG_LEAVE_WORLD)
        return false;

    if (IsPermanent())
    {
        // world buff auras should not be saved
        switch (m_spellProto->Id)
        {
            case 11413: // echoes of lordaeron
            case 11414: // echoes of lordaeron
            case 11415: // echoes of lordaeron
            case 1386: // echoes of lordaeron
            case 30880: // echoes of lordaeron
            case 30683: // echoes of lordaeron
            case 30682: // echoes of lordaeron
            case 29520: // echoes of lordaeron
                return false;
        }
    }
    return true;
}

bool SpellAuraHolder::IsCharm() const
{
    for (auto m_aura : m_auras)
        if (m_aura && IsCharmAura(m_spellProto, m_aura->GetEffIndex()))
            return true;
    return false;
}

void SpellAuraHolder::UnregisterAndCleanupTrackedAuras()
{
    TrackedAuraType trackedType = GetTrackedAuraType();
    if (!trackedType)
        return;

    if (trackedType == TRACK_AURA_TYPE_SINGLE_TARGET)
    {
        if (Unit* caster = GetCaster())
            caster->GetTrackedAuraTargets(trackedType).erase(GetSpellProto());
    }

    m_trackedAuraType = TRACK_AURA_TYPE_NOT_TRACKED;
}

void SpellAuraHolder::SetAuraFlag(uint32 slot, bool add)
{
    uint32 index    = slot >> 3;
    uint32 byte     = (slot & 7) << 2;
    uint32 val      = m_target->GetUInt32Value(UNIT_FIELD_AURAFLAGS + index);
    val &= ~(uint32(AFLAG_MASK_ALL) << byte);
    if (add)
    {
        uint32 flags = AFLAG_NONE;

        if (IsPositive())
        {
            if (!m_spellProto->HasAttribute(SPELL_ATTR_NO_AURA_CANCEL))
                flags |= AFLAG_CANCELABLE;
            flags |= AFLAG_UNK3;
        }
        else
            flags |= AFLAG_UNK4;

        val |= (flags << byte);
    }
    m_target->SetUInt32Value(UNIT_FIELD_AURAFLAGS + index, val);
}

void SpellAuraHolder::SetAuraLevel(uint32 slot, uint32 level)
{
    uint32 index    = slot / 4;
    uint32 byte     = (slot % 4) * 8;
    uint32 val      = m_target->GetUInt32Value(UNIT_FIELD_AURALEVELS + index);
    val &= ~(0xFF << byte);
    val |= (level << byte);
    m_target->SetUInt32Value(UNIT_FIELD_AURALEVELS + index, val);
}

void SpellAuraHolder::SetCreationDelayFlag()
{
    m_skipUpdate = true;
}

void SpellAuraHolder::UpdateAuraApplication()
{
    if (m_auraSlot >= MAX_AURAS)
        return;

    uint32 stackCount = m_procCharges > 0 ? m_procCharges * m_stackAmount : m_stackAmount;

    uint32 index    = m_auraSlot / 4;
    uint32 byte     = (m_auraSlot % 4) * 8;
    uint32 val      = m_target->GetUInt32Value(UNIT_FIELD_AURAAPPLICATIONS + index);
    val &= ~(0xFF << byte);
    // field expect count-1 for proper amount show, also prevent overflow at client side
    val |= ((uint8(stackCount <= 255 ? stackCount - 1 : 255 - 1)) << byte);
    m_target->SetUInt32Value(UNIT_FIELD_AURAAPPLICATIONS + index, val);
}

void SpellAuraHolder::UpdateAuraDuration()
{
    if (GetAuraSlot() >= MAX_AURAS || m_isPassive)
        return;

    if (m_target->GetTypeId() == TYPEID_PLAYER)
    {
        WorldPacket data(SMSG_UPDATE_AURA_DURATION, 5);
        data << uint8(GetAuraSlot());
        data << uint32(GetAuraDuration());
        ((Player*)m_target)->SendDirectMessage(data);
    }
}

bool SpellAuraHolder::IsProcReady(TimePoint const& now) const
{
    return m_procCooldown < now;
}

void SpellAuraHolder::SetProcCooldown(std::chrono::milliseconds cooldown, TimePoint const& now)
{
    m_procCooldown = now + cooldown;
}

void SpellAuraHolder::SetHeartbeatResist(uint32 chance, int32 originalDuration, uint32 drLevel)
{
    // NOTE: This is an experimental approximation of heartbeat resist mechanics, more research is required
    // Main points in common cited by independent sources:
    // * Break attempts become more frequent as hit count rises
    // * Break chance becomes higher as hit count rises
    m_heartbeatResistChance = (0.01f * chance * (1 + drLevel));
    m_heartbeatResistInterval = std::max(1000, int32(uint32(originalDuration) / (2 + drLevel)));
    m_heartbeatResistTimer = m_heartbeatResistInterval;
}

void SpellAuraHolder::UpdateHeartbeatResist(uint32 diff)
{
    if (m_heartbeatResistChance == 0.0f || !m_heartbeatResistInterval || m_heartbeatResistTimer <= 0)
        return;

    m_heartbeatResistTimer -= diff;

    while (m_heartbeatResistTimer < 0)
    {
        m_heartbeatResistTimer += m_heartbeatResistInterval;

        DEBUG_FILTER_LOG(LOG_FILTER_COMBAT, "UpdateHeartbeatResist: Update tick for spell %u with %i ms interval", m_spellProto->Id, m_heartbeatResistInterval);

        const bool resist = roll_chance_f(m_heartbeatResistChance);

        DEBUG_FILTER_LOG(LOG_FILTER_COMBAT, "UpdateHeartbeatResist: Result: %s (chance %.2f)", (resist ? "RESIST" : "HIT"), double(m_heartbeatResistChance));

        if (resist)
        {
            if (Unit* target = GetTarget())
            {
                target->RemoveSpellAuraHolder(this, AURA_REMOVE_BY_CANCEL);
                return;
            }
        }
    }
}

void SpellAuraHolder::OnHolderInit(WorldObject* caster)
{
    if (AuraScript* script = GetAuraScript())
        script->OnHolderInit(this, caster);
}

void SpellAuraHolder::OnDispel(Unit* dispeller, uint32 dispellingSpellId, uint32 originalStacks)
{
    if (AuraScript* script = GetAuraScript())
        script->OnDispel(this, dispeller, dispellingSpellId, originalStacks);
}

uint32 Aura::CalculateAuraEffectValue(Unit* caster, Unit* target, SpellEntry const* spellProto, SpellEffectIndex effIdx, uint32 value)
{
    switch (spellProto->EffectApplyAuraName[effIdx])
    {
        case SPELL_AURA_SCHOOL_ABSORB:
        {
            float DoneActualBenefit = 0.0f;
            if (spellProto->effectBonusCoefficient[effIdx] > 0)
            {
                DoneActualBenefit = caster->SpellBaseHealingBonusDone(GetSpellSchoolMask(spellProto)) * spellProto->effectBonusCoefficient[effIdx];
                DoneActualBenefit *= caster->CalculateLevelPenalty(spellProto);

                value += (int32)DoneActualBenefit;
            }
            return value;
        }
        case SPELL_AURA_MOD_DECREASE_SPEED:
        case SPELL_AURA_MOD_INCREASE_SPEED:
        {
            if (caster)
                if (Player* modOwner = caster->GetSpellModOwner())
                    modOwner->ApplySpellMod(spellProto->Id, SPELLMOD_SPEED, value);
            return value;
        }
        case SPELL_AURA_MOD_CASTING_SPEED_NOT_STACK:
        case SPELL_AURA_MOD_ATTACKSPEED:
        case SPELL_AURA_MOD_MELEE_HASTE:
        case SPELL_AURA_MOD_RANGED_HASTE:
        case SPELL_AURA_MOD_RANGED_AMMO_HASTE:
        {
            if (caster)
                if (Player* modOwner = caster->GetSpellModOwner())
                    modOwner->ApplySpellMod(spellProto->Id, SPELLMOD_HASTE, value);
            return value;
        }
        case SPELL_AURA_MOD_ATTACK_POWER:
        case SPELL_AURA_MOD_ATTACK_POWER_PCT:
        case SPELL_AURA_MOD_RANGED_ATTACK_POWER:
        case SPELL_AURA_MOD_RANGED_ATTACK_POWER_PCT:
        {
            if (caster)
                if (Player* modOwner = caster->GetSpellModOwner())
                    modOwner->ApplySpellMod(spellProto->Id, SPELLMOD_ATTACK_POWER, value);
            return value;
        }
    }
    return value;
}

void Aura::OnAuraInit()
{
    if (AuraScript* script = GetAuraScript())
        script->OnAuraInit(this);
}

int32 Aura::OnAuraValueCalculate(Unit* caster, int32 currentValue, Item* castItem)
{
    if (AuraScript* script = GetAuraScript())
    {
        AuraCalcData data(this, caster, GetTarget(), GetSpellProto(), GetEffIndex(), castItem);
        return script->OnAuraValueCalculate(data, currentValue);
    }
    return currentValue;
}

void Aura::OnDamageCalculate(Unit* victim, Unit* attacker, int32& advertisedBenefit, float& totalMod)
{
    if (AuraScript* script = GetAuraScript())
        return script->OnDamageCalculate(this, attacker, victim, advertisedBenefit, totalMod);
}

void Aura::OnCritChanceCalculate(Unit const* victim, float& chance, SpellEntry const* spellInfo)
{
    if (AuraScript* script = GetAuraScript())
        return script->OnCritChanceCalculate(this, victim, chance, spellInfo);
}

void Aura::OnApply(bool apply)
{
    if (AuraScript* script = GetAuraScript())
        script->OnApply(this, apply);
}

void Aura::OnAfterApply(bool apply)
{
    if (AuraScript* script = GetAuraScript())
        script->OnAfterApply(this, apply);
}

bool Aura::OnCheckProc(ProcExecutionData& data)
{
    if (AuraScript* script = GetAuraScript())
        return script->OnCheckProc(this, data);
    return true;
}

SpellAuraProcResult Aura::OnProc(ProcExecutionData& data)
{
    if (AuraScript* script = GetAuraScript())
        return script->OnProc(this, data);
    return SPELL_AURA_PROC_OK;
}

void Aura::OnAbsorb(int32& currentAbsorb, int32& remainingDamage, uint32& reflectedSpellId, int32& reflectDamage, bool& preventedDeath, bool& dropCharge, DamageEffectType damageType)
{
    if (AuraScript* script = GetAuraScript())
        script->OnAbsorb(this, currentAbsorb, remainingDamage, reflectedSpellId, reflectDamage, preventedDeath, dropCharge, damageType);
}

void Aura::OnManaAbsorb(int32& currentAbsorb)
{
    if (AuraScript* script = GetAuraScript())
        script->OnManaAbsorb(this, currentAbsorb);
}

void Aura::OnAuraDeathPrevention(int32& remainingDamage)
{
    if (AuraScript* script = GetAuraScript())
        script->OnAuraDeathPrevention(this, remainingDamage);
}

void Aura::OnPeriodicCalculateAmount(uint32& amount)
{
    if (AuraScript* script = GetAuraScript())
        script->OnPeriodicCalculateAmount(this, amount);
}

void Aura::OnHeartbeat()
{
    // TODO: move HB resist here
    if (AuraScript* script = GetAuraScript())
        script->OnHeartbeat(this);
}

bool Aura::OnAffectCheck(SpellEntry const* spellInfo) const
{
    if (AuraScript* script = GetAuraScript())
        return script->OnAffectCheck(this, spellInfo);
    return false;
}

uint32 Aura::GetAuraScriptCustomizationValue()
{
    if (AuraScript* script = GetAuraScript())
       return script->GetAuraScriptCustomizationValue(this);
    return 0;
}

void Aura::ForcePeriodicity(uint32 periodicTime)
{
    if (periodicTime == 0)
        m_isPeriodic = false;
    else
        m_isPeriodic = true;
    m_modifier.periodictime = periodicTime;
    m_periodicTimer = periodicTime;
}

void Aura::OnPeriodicTrigger(PeriodicTriggerData& data)
{
    if (AuraScript* script = GetAuraScript())
        script->OnPeriodicTrigger(this, data);
}

void Aura::OnPeriodicDummy()
{
    if (AuraScript* script = GetAuraScript())
        script->OnPeriodicDummy(this);
}

void Aura::OnPeriodicTickEnd()
{
    if (AuraScript* script = GetAuraScript())
        script->OnPeriodicTickEnd(this);
}
