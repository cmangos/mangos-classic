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
#include "WorldPacket.h"
#include "Server/Opcodes.h"
#include "Log.h"
#include "World/World.h"
#include "Globals/ObjectMgr.h"
#include "Spells/SpellMgr.h"
#include "Entities/Player.h"
#include "Entities/Unit.h"
#include "Spells/Spell.h"
#include "Entities/DynamicObject.h"
#include "Spells/SpellAuras.h"
#include "Entities/UpdateData.h"
#include "Globals/SharedDefines.h"
#include "Entities/Pet.h"
#include "Entities/GameObject.h"
#include "Entities/GossipDef.h"
#include "Entities/Creature.h"
#include "Entities/Totem.h"
#include "AI/BaseAI/CreatureAI.h"
#include "BattleGround/BattleGroundMgr.h"
#include "BattleGround/BattleGround.h"
#include "BattleGround/BattleGroundWS.h"
#include "Tools/Language.h"
#include "Social/SocialMgr.h"
#include "Util.h"
#include "Entities/TemporarySpawn.h"
#include "AI/ScriptDevAI/ScriptDevAIMgr.h"
#include "Tools/Formulas.h"
#include "Grids/GridNotifiers.h"
#include "Grids/GridNotifiersImpl.h"
#include "Grids/CellImpl.h"
#include "G3D/Vector3.h"
#include "Loot/LootMgr.h"
#include "Movement/MoveSpline.h"

pEffect SpellEffects[MAX_SPELL_EFFECTS] =
{
    &Spell::EffectNULL,                                     //  0
    &Spell::EffectInstaKill,                                //  1 SPELL_EFFECT_INSTAKILL
    &Spell::EffectSchoolDMG,                                //  2 SPELL_EFFECT_SCHOOL_DAMAGE
    &Spell::EffectDummy,                                    //  3 SPELL_EFFECT_DUMMY
    &Spell::EffectUnused,                                   //  4 SPELL_EFFECT_PORTAL_TELEPORT          unused from pre-1.2.1
    &Spell::EffectTeleportUnits,                            //  5 SPELL_EFFECT_TELEPORT_UNITS
    &Spell::EffectApplyAura,                                //  6 SPELL_EFFECT_APPLY_AURA
    &Spell::EffectEnvironmentalDMG,                         //  7 SPELL_EFFECT_ENVIRONMENTAL_DAMAGE
    &Spell::EffectPowerDrain,                               //  8 SPELL_EFFECT_POWER_DRAIN
    &Spell::EffectHealthLeech,                              //  9 SPELL_EFFECT_HEALTH_LEECH
    &Spell::EffectHeal,                                     // 10 SPELL_EFFECT_HEAL
    &Spell::EffectBind,                                     // 11 SPELL_EFFECT_BIND
    &Spell::EffectUnused,                                   // 12 SPELL_EFFECT_PORTAL                   unused from pre-1.2.1, exist 2 spell, but not exist any data about its real usage
    &Spell::EffectUnused,                                   // 13 SPELL_EFFECT_RITUAL_BASE              unused from pre-1.2.1
    &Spell::EffectUnused,                                   // 14 SPELL_EFFECT_RITUAL_SPECIALIZE        unused from pre-1.2.1
    &Spell::EffectUnused,                                   // 15 SPELL_EFFECT_RITUAL_ACTIVATE_PORTAL   unused from pre-1.2.1
    &Spell::EffectQuestComplete,                            // 16 SPELL_EFFECT_QUEST_COMPLETE
    &Spell::EffectWeaponDmg,                                // 17 SPELL_EFFECT_WEAPON_DAMAGE_NOSCHOOL
    &Spell::EffectResurrect,                                // 18 SPELL_EFFECT_RESURRECT
    &Spell::EffectAddExtraAttacks,                          // 19 SPELL_EFFECT_ADD_EXTRA_ATTACKS
    &Spell::EffectEmpty,                                    // 20 SPELL_EFFECT_DODGE                    one spell: Dodge
    &Spell::EffectEmpty,                                    // 21 SPELL_EFFECT_EVADE                    one spell: Evade (DND)
    &Spell::EffectParry,                                    // 22 SPELL_EFFECT_PARRY
    &Spell::EffectBlock,                                    // 23 SPELL_EFFECT_BLOCK                    one spell: Block
    &Spell::EffectCreateItem,                               // 24 SPELL_EFFECT_CREATE_ITEM
    &Spell::EffectEmpty,                                    // 25 SPELL_EFFECT_WEAPON                   spell per weapon type, in ItemSubclassmask store mask that can be used for usability check at equip, but current way using skill also work.
    &Spell::EffectEmpty,                                    // 26 SPELL_EFFECT_DEFENSE                  one spell: Defense
    &Spell::EffectPersistentAA,                             // 27 SPELL_EFFECT_PERSISTENT_AREA_AURA
    &Spell::EffectSummon,                                   // 28 SPELL_EFFECT_SUMMON
    &Spell::EffectLeapForward,                              // 29 SPELL_EFFECT_LEAP
    &Spell::EffectEnergize,                                 // 30 SPELL_EFFECT_ENERGIZE
    &Spell::EffectWeaponDmg,                                // 31 SPELL_EFFECT_WEAPON_PERCENT_DAMAGE
    &Spell::EffectTriggerMissileSpell,                      // 32 SPELL_EFFECT_TRIGGER_MISSILE
    &Spell::EffectOpenLock,                                 // 33 SPELL_EFFECT_OPEN_LOCK
    &Spell::EffectSummonChangeItem,                         // 34 SPELL_EFFECT_SUMMON_CHANGE_ITEM
    &Spell::EffectApplyAreaAura,                            // 35 SPELL_EFFECT_APPLY_AREA_AURA_PARTY
    &Spell::EffectLearnSpell,                               // 36 SPELL_EFFECT_LEARN_SPELL
    &Spell::EffectEmpty,                                    // 37 SPELL_EFFECT_SPELL_DEFENSE            one spell: SPELLDEFENSE (DND)
    &Spell::EffectDispel,                                   // 38 SPELL_EFFECT_DISPEL
    &Spell::EffectEmpty,                                    // 39 SPELL_EFFECT_LANGUAGE                 misc store lang id
    &Spell::EffectDualWield,                                // 40 SPELL_EFFECT_DUAL_WIELD
    &Spell::EffectSummonWild,                               // 41 SPELL_EFFECT_SUMMON_WILD
    &Spell::EffectSummonGuardian,                           // 42 SPELL_EFFECT_SUMMON_GUARDIAN
    &Spell::EffectTeleUnitsFaceCaster,                      // 43 SPELL_EFFECT_TELEPORT_UNITS_FACE_CASTER
    &Spell::EffectLearnSkill,                               // 44 SPELL_EFFECT_SKILL_STEP
    &Spell::EffectAddHonor,                                 // 45 SPELL_EFFECT_ADD_HONOR                honor/pvp related
    &Spell::EffectSpawn,                                    // 46 SPELL_EFFECT_SPAWN                    spawn/login animation, expected by spawn unit cast, also base points store some dynflags
    &Spell::EffectTradeSkill,                               // 47 SPELL_EFFECT_TRADE_SKILL
    &Spell::EffectUnused,                                   // 48 SPELL_EFFECT_STEALTH                  one spell: Base Stealth
    &Spell::EffectUnused,                                   // 49 SPELL_EFFECT_DETECT                   one spell: Detect
    &Spell::EffectTransmitted,                              // 50 SPELL_EFFECT_TRANS_DOOR
    &Spell::EffectUnused,                                   // 51 SPELL_EFFECT_FORCE_CRITICAL_HIT       unused from pre-1.2.1
    &Spell::EffectUnused,                                   // 52 SPELL_EFFECT_GUARANTEE_HIT            unused from pre-1.2.1
    &Spell::EffectEnchantItemPerm,                          // 53 SPELL_EFFECT_ENCHANT_ITEM
    &Spell::EffectEnchantItemTmp,                           // 54 SPELL_EFFECT_ENCHANT_ITEM_TEMPORARY
    &Spell::EffectTameCreature,                             // 55 SPELL_EFFECT_TAMECREATURE
    &Spell::EffectSummonPet,                                // 56 SPELL_EFFECT_SUMMON_PET
    &Spell::EffectLearnPetSpell,                            // 57 SPELL_EFFECT_LEARN_PET_SPELL
    &Spell::EffectWeaponDmg,                                // 58 SPELL_EFFECT_WEAPON_DAMAGE
    &Spell::EffectOpenLock,                                 // 59 SPELL_EFFECT_OPEN_LOCK_ITEM
    &Spell::EffectProficiency,                              // 60 SPELL_EFFECT_PROFICIENCY
    &Spell::EffectSendEvent,                                // 61 SPELL_EFFECT_SEND_EVENT
    &Spell::EffectPowerBurn,                                // 62 SPELL_EFFECT_POWER_BURN
    &Spell::EffectThreat,                                   // 63 SPELL_EFFECT_THREAT
    &Spell::EffectTriggerSpell,                             // 64 SPELL_EFFECT_TRIGGER_SPELL
    &Spell::EffectUnused,                                   // 65 SPELL_EFFECT_HEALTH_FUNNEL            unused
    &Spell::EffectUnused,                                   // 66 SPELL_EFFECT_POWER_FUNNEL             unused from pre-1.2.1
    &Spell::EffectHealMaxHealth,                            // 67 SPELL_EFFECT_HEAL_MAX_HEALTH
    &Spell::EffectInterruptCast,                            // 68 SPELL_EFFECT_INTERRUPT_CAST
    &Spell::EffectDistract,                                 // 69 SPELL_EFFECT_DISTRACT
    &Spell::EffectPull,                                     // 70 SPELL_EFFECT_PULL                     one spell: Distract Move
    &Spell::EffectPickPocket,                               // 71 SPELL_EFFECT_PICKPOCKET
    &Spell::EffectAddFarsight,                              // 72 SPELL_EFFECT_ADD_FARSIGHT
    &Spell::EffectSummonPossessed,                          // 73 SPELL_EFFECT_SUMMON_POSSESSED
    &Spell::EffectSummonTotem,                              // 74 SPELL_EFFECT_SUMMON_TOTEM
    &Spell::EffectHealMechanical,                           // 75 SPELL_EFFECT_HEAL_MECHANICAL          one spell: Mechanical Patch Kit
    &Spell::EffectSummonObjectWild,                         // 76 SPELL_EFFECT_SUMMON_OBJECT_WILD
    &Spell::EffectScriptEffect,                             // 77 SPELL_EFFECT_SCRIPT_EFFECT
    &Spell::EffectUnused,                                   // 78 SPELL_EFFECT_ATTACK
    &Spell::EffectSanctuary,                                // 79 SPELL_EFFECT_SANCTUARY
    &Spell::EffectAddComboPoints,                           // 80 SPELL_EFFECT_ADD_COMBO_POINTS
    &Spell::EffectUnused,                                   // 81 SPELL_EFFECT_CREATE_HOUSE             one spell: Create House (TEST)
    &Spell::EffectNULL,                                     // 82 SPELL_EFFECT_BIND_SIGHT
    &Spell::EffectDuel,                                     // 83 SPELL_EFFECT_DUEL
    &Spell::EffectStuck,                                    // 84 SPELL_EFFECT_STUCK
    &Spell::EffectSummonPlayer,                             // 85 SPELL_EFFECT_SUMMON_PLAYER
    &Spell::EffectActivateObject,                           // 86 SPELL_EFFECT_ACTIVATE_OBJECT
    &Spell::EffectSummonTotem,                              // 87 SPELL_EFFECT_SUMMON_TOTEM_SLOT1
    &Spell::EffectSummonTotem,                              // 88 SPELL_EFFECT_SUMMON_TOTEM_SLOT2
    &Spell::EffectSummonTotem,                              // 89 SPELL_EFFECT_SUMMON_TOTEM_SLOT3
    &Spell::EffectSummonTotem,                              // 90 SPELL_EFFECT_SUMMON_TOTEM_SLOT4
    &Spell::EffectUnused,                                   // 91 SPELL_EFFECT_THREAT_ALL               one spell: zzOLDBrainwash
    &Spell::EffectEnchantHeldItem,                          // 92 SPELL_EFFECT_ENCHANT_HELD_ITEM
    &Spell::EffectUnused,                                   // 93 SPELL_EFFECT_SUMMON_PHANTASM
    &Spell::EffectSelfResurrect,                            // 94 SPELL_EFFECT_SELF_RESURRECT
    &Spell::EffectSkinning,                                 // 95 SPELL_EFFECT_SKINNING
    &Spell::EffectCharge,                                   // 96 SPELL_EFFECT_CHARGE
    &Spell::EffectSummonCritter,                            // 97 SPELL_EFFECT_SUMMON_CRITTER
    &Spell::EffectKnockBack,                                // 98 SPELL_EFFECT_KNOCK_BACK
    &Spell::EffectDisEnchant,                               // 99 SPELL_EFFECT_DISENCHANT
    &Spell::EffectInebriate,                                // 100 SPELL_EFFECT_INEBRIATE
    &Spell::EffectFeedPet,                                  // 101 SPELL_EFFECT_FEED_PET
    &Spell::EffectDismissPet,                               // 102 SPELL_EFFECT_DISMISS_PET
    &Spell::EffectReputation,                               // 103 SPELL_EFFECT_REPUTATION
    &Spell::EffectSummonObject,                             // 104 SPELL_EFFECT_SUMMON_OBJECT_SLOT1
    &Spell::EffectSummonObject,                             // 105 SPELL_EFFECT_SUMMON_OBJECT_SLOT2
    &Spell::EffectSummonObject,                             // 106 SPELL_EFFECT_SUMMON_OBJECT_SLOT3
    &Spell::EffectSummonObject,                             // 107 SPELL_EFFECT_SUMMON_OBJECT_SLOT4
    &Spell::EffectDispelMechanic,                           // 108 SPELL_EFFECT_DISPEL_MECHANIC
    &Spell::EffectSummonDeadPet,                            // 109 SPELL_EFFECT_SUMMON_DEAD_PET
    &Spell::EffectDestroyAllTotems,                         // 110 SPELL_EFFECT_DESTROY_ALL_TOTEMS
    &Spell::EffectDurabilityDamage,                         // 111 SPELL_EFFECT_DURABILITY_DAMAGE
    &Spell::EffectSummonDemon,                              // 112 SPELL_EFFECT_SUMMON_DEMON
    &Spell::EffectResurrectNew,                             // 113 SPELL_EFFECT_RESURRECT_NEW
    &Spell::EffectTaunt,                                    // 114 SPELL_EFFECT_ATTACK_ME
    &Spell::EffectDurabilityDamagePCT,                      // 115 SPELL_EFFECT_DURABILITY_DAMAGE_PCT
    &Spell::EffectSkinPlayerCorpse,                         // 116 SPELL_EFFECT_SKIN_PLAYER_CORPSE       one spell: Remove Insignia, bg usage, required special corpse flags...
    &Spell::EffectSpiritHeal,                               // 117 SPELL_EFFECT_SPIRIT_HEAL              one spell: Spirit Heal
    &Spell::EffectSkill,                                    // 118 SPELL_EFFECT_SKILL                    professions and more
    &Spell::EffectApplyAreaAura,                            // 119 SPELL_EFFECT_APPLY_AREA_AURA_PET
    &Spell::EffectUnused,                                   // 120 SPELL_EFFECT_TELEPORT_GRAVEYARD       one spell: Graveyard Teleport Test
    &Spell::EffectWeaponDmg,                                // 121 SPELL_EFFECT_NORMALIZED_WEAPON_DMG
    &Spell::EffectUnused,                                   // 122 SPELL_EFFECT_122                      unused
    &Spell::EffectSendTaxi,                                 // 123 SPELL_EFFECT_SEND_TAXI                taxi/flight related (misc value is taxi path id)
    &Spell::EffectPlayerPull,                               // 124 SPELL_EFFECT_PLAYER_PULL              opposite of knockback effect (pulls player twoard caster)
    &Spell::EffectModifyThreatPercent,                      // 125 SPELL_EFFECT_MODIFY_THREAT_PERCENT
    &Spell::EffectUnused,                                   // 126 SPELL_EFFECT_126                      future spell steal effect? now only used one test spell
    &Spell::EffectUnused,                                   // 127 SPELL_EFFECT_127                      future Prospecting spell, not have spells
    &Spell::EffectUnused,                                   // 128 SPELL_EFFECT_128                      future SPELL_EFFECT_APPLY_AREA_AURA_FRIEND, not have spells
    &Spell::EffectUnused,                                   // 129 SPELL_EFFECT_129                      future SPELL_EFFECT_APPLY_AREA_AURA_ENEMY, now only one test spell
};

void Spell::EffectEmpty(SpellEffectIndex /*eff_idx*/)
{
    // NOT NEED ANY IMPLEMENTATION CODE, EFFECT POSISBLE USED AS MARKER OR CLIENT INFORM
}

void Spell::EffectNULL(SpellEffectIndex /*eff_idx*/)
{
    DEBUG_LOG("WORLD: Spell Effect DUMMY");
}

void Spell::EffectUnused(SpellEffectIndex /*eff_idx*/)
{
    // NOT USED BY ANY SPELL OR USELESS OR IMPLEMENTED IN DIFFERENT WAY IN MANGOS
}

void Spell::EffectResurrectNew(SpellEffectIndex eff_idx)
{
    if (!unitTarget || unitTarget->isAlive())
        return;

    if (unitTarget->GetTypeId() != TYPEID_PLAYER)
        return;

    if (!unitTarget->IsInWorld())
        return;

    Player* pTarget = ((Player*)unitTarget);

    if (pTarget->isRessurectRequested())      // already have one active request
        return;

    uint32 health = damage;
    uint32 mana = m_spellInfo->EffectMiscValue[eff_idx];
    pTarget->setResurrectRequestData(m_caster->GetObjectGuid(), m_caster->GetMapId(), m_caster->GetPositionX(), m_caster->GetPositionY(), m_caster->GetPositionZ(), health, mana);
    SendResurrectRequest(pTarget);
    m_spellLog.AddLog(uint32(SPELL_EFFECT_RESURRECT_NEW), unitTarget->GetObjectGuid());
}

void Spell::EffectInstaKill(SpellEffectIndex /*eff_idx*/)
{
    if (!unitTarget || !unitTarget->isAlive())
        return;

    // Demonic Sacrifice
    if (m_spellInfo->Id == 18788 && unitTarget->GetTypeId() == TYPEID_UNIT)
    {
        uint32 entry = unitTarget->GetEntry();
        uint32 spellID;
        switch (entry)
        {
            case   416: spellID = 18789; break;             // imp
            case   417: spellID = 18792; break;             // fellhunter
            case  1860: spellID = 18790; break;             // void
            case  1863: spellID = 18791; break;             // succubus
            default:
                sLog.outError("EffectInstaKill: Unhandled creature entry (%u) case.", entry);
                return;
        }

        m_caster->CastSpell(m_caster, spellID, TRIGGERED_OLD_TRIGGERED);
    }

    if (m_caster == unitTarget)                             // prevent interrupt message
        finish();

    WorldObject* caster = GetCastingObject();               // we need the original casting object

    WorldPacket data(SMSG_SPELLINSTAKILLLOG, (8 + 4));
    data << unitTarget->GetObjectGuid();                    // Victim GUID
    data << uint32(m_spellInfo->Id);
    m_caster->SendMessageToSet(data, true);

    m_spellLog.AddLog(uint32(SPELL_EFFECT_INSTAKILL), unitTarget->GetObjectGuid());

    m_caster->DealDamage(unitTarget, unitTarget->GetHealth(), nullptr, INSTAKILL, SPELL_SCHOOL_MASK_NORMAL, m_spellInfo, false);
}

void Spell::EffectEnvironmentalDMG(SpellEffectIndex eff_idx)
{
    uint32 absorb = 0;
    int32 resist = 0;

    // Note: this hack with damage replace required until GO casting not implemented
    // environment damage spells already have around enemies targeting but this not help in case nonexistent GO casting support
    // currently each enemy selected explicitly and self cast damage, we prevent apply self casted spell bonuses/etc
    damage = m_spellInfo->CalculateSimpleValue(eff_idx);

    m_caster->CalculateDamageAbsorbAndResist(m_caster, GetSpellSchoolMask(m_spellInfo), SPELL_DIRECT_DAMAGE, damage, &absorb, &resist);

    m_caster->SendSpellNonMeleeDamageLog(m_caster, m_spellInfo->Id, damage, GetSpellSchoolMask(m_spellInfo), absorb, resist, false, 0, false);
    if (m_caster->GetTypeId() == TYPEID_PLAYER)
        ((Player*)m_caster)->EnvironmentalDamage(DAMAGE_FIRE, damage);
}

void Spell::EffectSchoolDMG(SpellEffectIndex eff_idx)
{
    if (unitTarget && unitTarget->isAlive())
    {
        switch (m_spellInfo->SpellFamilyName)
        {
            case SPELLFAMILY_GENERIC:
            {
                switch (m_spellInfo->Id)                    // better way to check unknown
                {
                    // Meteor like spells (divided damage to targets)
                    case 24340: case 26558: case 28884:     // Meteor
                    case 26789:                             // Shard of the Fallen Star
                    {
                        uint32 count = 0;
                        for (TargetList::const_iterator ihit = m_UniqueTargetInfo.begin(); ihit != m_UniqueTargetInfo.end(); ++ihit)
                            if (ihit->effectHitMask & (1 << eff_idx))
                                ++count;

                        damage /= count;                    // divide to all targets
                        break;
                    }
                    // percent from health with min
                    case 25599:                             // Thundercrash
                    {
                        damage = unitTarget->GetHealth() / 2;
                        if (damage < 200)
                            damage = 200;
                        break;
                    }
                }
                break;
            }

            case SPELLFAMILY_MAGE:
                break;
            case SPELLFAMILY_WARRIOR:
            {
                // Bloodthirst
                if (m_spellInfo->SpellIconID == 38 && m_spellInfo->SpellFamilyFlags & uint64(0x2000000))
                {
                    damage = uint32(damage * (m_caster->GetTotalAttackPowerValue(BASE_ATTACK)) / 100);
                }
                // Shield Slam
                else if (m_spellInfo->SpellIconID == 413 && m_spellInfo->SpellFamilyFlags & uint64(0x2000000))
                    damage += int32(m_caster->GetShieldBlockValue());
                if (m_spellInfo->Id == 20647)
                    m_caster->SetPower(POWER_RAGE, 0);
                break;
            }
            case SPELLFAMILY_WARLOCK:
            {
                // Conflagrate - consumes Immolate
                if (m_spellInfo->SpellFamilyFlags & uint64(0x0000000000000200))
                {
                    // for caster applied auras only
                    Unit::AuraList const& mPeriodic = unitTarget->GetAurasByType(SPELL_AURA_PERIODIC_DAMAGE);
                    for (auto i : mPeriodic)
                    {
                        if (i->GetCasterGuid() == m_caster->GetObjectGuid() &&
                                // Immolate
                                i->GetSpellProto()->IsFitToFamily(SPELLFAMILY_WARLOCK, uint64(0x0000000000000004)))
                        {
                            unitTarget->RemoveAurasByCasterSpell(i->GetId(), m_caster->GetObjectGuid());
                            break;
                        }
                    }
                }
                break;
            }
            case SPELLFAMILY_DRUID:
            {
                // Ferocious Bite
                if ((m_spellInfo->SpellFamilyFlags & uint64(0x000800000)) && m_spellInfo->SpellVisual == 6587)
                {
                    // converts each extra point of energy into additional damage
                    damage += int32(m_caster->GetPower(POWER_ENERGY) * m_spellInfo->DmgMultiplier[eff_idx]);
                    m_caster->SetPower(POWER_ENERGY, 0);
                }
                break;
            }
            case SPELLFAMILY_ROGUE:
            {
                // Eviscerate
                if ((m_spellInfo->SpellFamilyFlags & uint64(0x00020000)) && m_caster->GetTypeId() == TYPEID_PLAYER)
                {
                    if (uint32 combo = ((Player*)m_caster)->GetComboPoints())
                    {
                        damage += int32(m_caster->GetTotalAttackPowerValue(BASE_ATTACK) * combo * 0.03f);
                    }
                }
                break;
            }
            case SPELLFAMILY_HUNTER:
                break;
            case SPELLFAMILY_PALADIN:
                break;
        }

        if (damage >= 0)
            m_damagePerEffect[eff_idx] = damage;
    }
}

void Spell::EffectDummy(SpellEffectIndex eff_idx)
{
    // selection by spell family
    switch (m_spellInfo->SpellFamilyName)
    {
        case SPELLFAMILY_GENERIC:
        {
            switch (m_spellInfo->Id)
            {
                case 3360:                                  // Curse of the Eye
                {
                    if (unitTarget)
                        m_caster->CastSpell(unitTarget, ((unitTarget->getGender() == GENDER_MALE) ? 10651 : 10653), TRIGGERED_OLD_TRIGGERED);

                    return;
                }
                case 7671:                                  // Transformation (human<->worgen)
                {
                    if (unitTarget) // Transform Visual
                        unitTarget->CastSpell(unitTarget, 24085, TRIGGERED_OLD_TRIGGERED);

                    return;
                }
                case 8063:                                  // Deviate Fish
                {
                    if (m_caster->GetTypeId() == TYPEID_PLAYER)
                    {
                        // 8064 = Sleepy | 8065 = Invigorate | 8066 = Shrink | 8067 = Party Time! | 8068 = Healthy Spirit
                        const uint32 spell_list[5] = {8064, 8065, 8066, 8067, 8068};
                        m_caster->CastSpell(m_caster, spell_list[urand(0, 4)], TRIGGERED_OLD_TRIGGERED, nullptr);
                    }

                    return;
                }
                case 8213:                                  // Savory Deviate Delight
                {
                    if (m_caster->GetTypeId() == TYPEID_PLAYER)
                        m_caster->CastSpell(m_caster, (urand(0, 1)
                                                       ? (m_caster->getGender() == GENDER_MALE ? 8219 : 8220)                      // Flip Out - ninja
                                                       : (m_caster->getGender() == GENDER_MALE ? 8221 : 8222)), TRIGGERED_OLD_TRIGGERED, nullptr);    // Yaaarrrr - pirate

                    return;
                }
                case 8344:                                  // Gnomish Universal Remote (ItemID: 7506)
                {
                    if (m_CastItem && unitTarget)
                    {
                        // 8345 - Control the machine | 8346 = Malfunction the machine (root) | 8347 = Taunt/enrage the machine
                        const uint32 spell_list[3] = { 8345, 8346, 8347 };
                        m_caster->CastSpell(unitTarget, spell_list[urand(0, 2)], TRIGGERED_OLD_TRIGGERED, m_CastItem);
                    }

                    return;
                }
                case 8606:                                  // Summon Cyclonian
                {
                    if (!(m_targets.m_targetMask & TARGET_FLAG_DEST_LOCATION))
                        return;

                    float x, y, z;
                    m_targets.getDestination(x, y, z); // database loaded coordinates due to target type
                    m_caster->SummonCreature(6239, x, y, z, 0.0f, TEMPSPAWN_TIMED_OOC_DESPAWN, 30 * IN_MILLISECONDS);

                    return;
                }
                case 9012:                                  // Attract Rift Spawn
                {
                    if (!unitTarget || !unitTarget->HasAura(9032))
                        return;

                    unitTarget->CastSpell(unitTarget, 9010, TRIGGERED_OLD_TRIGGERED);
                    return;
                }
                case 9204:                                  // Hate to Zero
                case 20538:
                case 26569:
                case 26637:
                {
                    m_caster->getThreatManager().modifyThreatPercent(unitTarget, -100);
                    return;
                }
                case 9976:                                  // Polly Eats the E.C.A.C.
                {
                    if (unitTarget && unitTarget->GetTypeId() == TYPEID_UNIT)
                    {
                        // Summon Polly Jr.
                        unitTarget->CastSpell(unitTarget, 9998, TRIGGERED_OLD_TRIGGERED);
                        ((Creature*)unitTarget)->ForcedDespawn(100);
                    }

                    return;
                }
                case 10254:                                 // Stone Dwarf Awaken Visual
                {
                    if (m_caster->GetTypeId() == TYPEID_UNIT) // see spell 10255 (aura dummy)
                    {
                        m_caster->clearUnitState(UNIT_STAT_ROOT);
                        m_caster->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                    }

                    return;
                }
                case 12975:                                 // Last Stand
                {
                    int32 healthModSpellBasePoints0 = int32(m_caster->GetMaxHealth() * 0.3);
                    m_caster->CastCustomSpell(m_caster, 12976, &healthModSpellBasePoints0, nullptr, nullptr, TRIGGERED_OLD_TRIGGERED, nullptr);

                    return;
                }
                case 13006:                                 // Gnomish Shrink Ray (ItemID: 10716)
                {
                    if (unitTarget && m_CastItem)
                    {
                        uint32 roll = urand(0, 99);
                        // These rates are hella random; someone feel free to correct them
                        if (roll < 3)                                         // Whole party will grow
                            m_caster->CastSpell(m_caster, 13004, TRIGGERED_OLD_TRIGGERED);
                        else if (roll < 6)                                    // Whole party will shrink
                            m_caster->CastSpell(m_caster, 13010, TRIGGERED_OLD_TRIGGERED);
                        else if (roll < 9)                                    // Whole enemy 'team' will grow
                            m_caster->CastSpell(unitTarget, 13004, TRIGGERED_OLD_TRIGGERED);
                        else if (roll < 12)                                    // Whole enemy 'team' will shrink
                            m_caster->CastSpell(unitTarget, 13010, TRIGGERED_OLD_TRIGGERED);
                        else if (roll < 24)                                   // Caster will shrink
                            m_caster->CastSpell(m_caster, 13003, TRIGGERED_OLD_TRIGGERED);
                        else                                                  // Enemy target will shrink
                            m_caster->CastSpell(unitTarget, 13003, TRIGGERED_OLD_TRIGGERED);
                    }

                    return;
                }
                case 13120:                                 // Net-O-Matic
                {
                    if (unitTarget && m_CastItem)
                    {
                        uint32 roll = urand(0, 99);
                        if (roll < 2)                           // 2% for 30 sec self root (off-like chance unknown)
                            m_caster->CastSpell(unitTarget, 16566, TRIGGERED_OLD_TRIGGERED, m_CastItem);
                        else if (roll < 4)                      // 2% for 20 sec root, charge to target (off-like chance unknown)
                            m_caster->CastSpell(unitTarget, 13119, TRIGGERED_OLD_TRIGGERED, m_CastItem);
                        else                                    // normal root
                            m_caster->CastSpell(unitTarget, 13099, TRIGGERED_OLD_TRIGGERED, m_CastItem);
                    }

                    return;
                }
                case 13180:                                 // Gnomish Mind Control Cap (ItemID: 10726)
                {
                    if (unitTarget && m_CastItem)
                    {
                        uint32 roll = urand(0, 9);
                        if (roll == 1 && unitTarget->GetTypeId() == TYPEID_PLAYER)
                            unitTarget->CastSpell(m_caster, 13181, TRIGGERED_OLD_TRIGGERED, m_CastItem);
                        else if (roll)
                            m_caster->CastSpell(unitTarget, 13181, TRIGGERED_OLD_TRIGGERED, m_CastItem);
                    }

                    return;
                }
                case 13489:
                {
                    if (unitTarget)
                        unitTarget->CastSpell(unitTarget, 14744, TRIGGERED_OLD_TRIGGERED);

                    return;
                }
                case 13535:                                 // Tame Beast
                {
                    if (!m_originalCaster || m_originalCaster->GetTypeId() != TYPEID_PLAYER)
                        return;

                    if (Unit* channelTarget = m_originalCaster->GetChannelObject())
                    {
                        if (channelTarget->GetTypeId() == TYPEID_UNIT)
                            m_originalCaster->CastSpell(channelTarget, 13481, TRIGGERED_OLD_TRIGGERED, nullptr, nullptr, m_originalCasterGUID, m_spellInfo);
                    }

                    return;
                }
                case 13567:                                 // Dummy Trigger
                {
                    // can be used for different aura triggering, so select by aura
                    if (m_triggeredByAuraSpell && unitTarget)
                    {
                        switch (m_triggeredByAuraSpell->Id)
                        {
                            case 26467:                         // Persistent Shield
                                m_caster->CastCustomSpell(unitTarget, 26470, &damage, nullptr, nullptr, TRIGGERED_OLD_TRIGGERED);
                                break;
                            default:
                                sLog.outError("EffectDummy: Non-handled case for spell 13567 for triggered aura %u", m_triggeredByAuraSpell->Id);
                                break;
                        }
                    }

                    return;
                }
                case 14185:                                 // Preparation Rogue
                {
                    if (m_caster->GetTypeId() == TYPEID_PLAYER)
                    {
                        // immediately finishes the cooldown on certain Rogue abilities
                        auto cdCheck = [](SpellEntry const & spellEntry) -> bool { return (spellEntry.SpellFamilyName == SPELLFAMILY_ROGUE && (spellEntry.SpellFamilyFlags & uint64(0x0000026000000860))); };
                        static_cast<Player*>(m_caster)->RemoveSomeCooldown(cdCheck);
                    }

                    return;
                }
                case 14537:                                 // Six Demon Bag
                {
                    if (unitTarget)
                    {
                        uint32 roll = urand(0, 99);
                        if (roll < 25)                          // Fireball (25% chance)
                            m_caster->CastSpell(unitTarget, 15662, TRIGGERED_OLD_TRIGGERED, m_CastItem);
                        else if (roll < 50)                     // Frostbolt (25% chance)
                            m_caster->CastSpell(unitTarget, 11538, TRIGGERED_OLD_TRIGGERED, m_CastItem);
                        else if (roll < 70)                     // Chain Lighting (20% chance)
                            m_caster->CastSpell(unitTarget, 21179, TRIGGERED_OLD_TRIGGERED, m_CastItem);
                        else if (roll < 85)                     // Enveloping Winds (15% chance)
                            m_caster->CastSpell(unitTarget, 25189, TRIGGERED_OLD_TRIGGERED, m_CastItem);
                        else if (roll < 90)                     // Summon Felhund minion (5% chance)
                            m_caster->CastSpell(m_caster, 14642, TRIGGERED_OLD_TRIGGERED, m_CastItem);
                        else if (roll < 97)                     // Polymorph (7% to target, 10% chance total)
                            m_caster->CastSpell(unitTarget, 14621, TRIGGERED_OLD_TRIGGERED, m_CastItem);
                        else                                    // Polymorph (3% to self, backfire, 10% chance total)
                            m_caster->CastSpell(m_caster, 14621, TRIGGERED_OLD_TRIGGERED, m_CastItem);
                    }

                    return;
                }
                case 15998:                                 // Capture Worg Pup
                {
                    if (unitTarget && unitTarget->GetTypeId() == TYPEID_UNIT)
                        ((Creature*)unitTarget)->ForcedDespawn();

                    return;
                }
                case 16049:                                 // Emberseer Growing
                {
                    if (UnitAI* ai = m_caster->AI())
                        ai->SendAIEvent(AI_EVENT_CUSTOM_A, m_caster, m_caster);
                    return;
                }
                case 16589:                                 // Noggenfogger Elixir
                {
                    if (m_caster->GetTypeId() == TYPEID_PLAYER)
                    {
                        const uint32 spell_list[3] = {16595, 16593, 16591};
                        m_caster->CastSpell(m_caster, spell_list[urand(0, 2)], TRIGGERED_OLD_TRIGGERED, nullptr);
                    }

                    return;
                }
                case 17009:                                 // Voodoo
                {
                    if (unitTarget && unitTarget->GetTypeId() == TYPEID_PLAYER)
                    {
                        // 16707 = Hex | 16708 = Hex | 16709 = Hex | 16711 = Grow | 16712 = Special Brew | 16713 = Ghostly | 16716 = Launch
                        const uint32 spell_list[7] = {16707, 16708, 16709, 16711, 16712, 16713, 16716};
                        m_caster->CastSpell(unitTarget, spell_list[urand(0, 6)], TRIGGERED_OLD_TRIGGERED, nullptr, nullptr, m_originalCasterGUID, m_spellInfo);
                    }

                    return;
                }
                case 17251:                                 // Spirit Healer Res
                {
                    if (unitTarget)
                    {
                        Unit* caster = GetAffectiveCaster();
                        if (caster && caster->GetTypeId() == TYPEID_PLAYER)
                        {
                            WorldPacket data(SMSG_SPIRIT_HEALER_CONFIRM, 8);
                            data << unitTarget->GetObjectGuid();
                            ((Player*)caster)->GetSession()->SendPacket(data);
                        }
                    }

                    return;
                }
                case 17271:                                 // Test Fetid Skull
                {
                    // 17269 = Create Resonating Skull | 17270 = Create Bone Dust
                    if (m_caster->GetTypeId() == TYPEID_PLAYER)
                        m_caster->CastSpell(m_caster, (urand(0, 1) ? 17269 : 17270), TRIGGERED_OLD_TRIGGERED);

                    return;
                }
                case 17678:                                 // Despawn Spectral Combatants (sic)
                {
                    if (unitTarget && unitTarget->GetTypeId() == TYPEID_UNIT)
                    {
                        Creature* creatureTarget = (Creature*)unitTarget;
                        if (creatureTarget)
                            creatureTarget->ForcedDespawn(3000);
                    }

                    return;
                }
                case 17770:                                 // Wolfshead Helm Energy
                {
                    m_caster->CastSpell(m_caster, 29940, TRIGGERED_OLD_TRIGGERED, nullptr);

                    return;
                }
                case 17950:                                 // Shadow Portal
                {
                    if (unitTarget)
                    {
                        const uint32 spell_list[6] = {17863, 17939, 17943, 17944, 17946, 17948};
                        m_caster->CastSpell(unitTarget, spell_list[urand(0, 5)], TRIGGERED_OLD_TRIGGERED);
                    }

                    return;
                }
                case 18350:                                 // Dummy Trigger
                {
                    if (m_triggeredByAuraSpell && unitTarget)
                    {
                        switch (m_triggeredByAuraSpell->Id)
                        {
                            case 13810: // Frost Trap Aura
                            {
                                // Need to check casting of entrapment on every pulse
                                // Chose this route so that whole proc system doesnt have to run
                                // IDs are in reverse order because its more likely someone will have max rank
                                float chance = 0.f;
                                uint32 entrapmentIDs[] = { 19388 , 19387, 19184 };
                                for (uint32 spellId : entrapmentIDs)
                                    if (SpellAuraHolder* holder = m_caster->GetSpellAuraHolder(spellId))
                                    {
                                        chance = holder->GetSpellProto()->procChance;
                                        break;
                                    }

                                if (roll_chance_f(chance))
                                    m_caster->CastSpell(unitTarget, 19185, TRIGGERED_OLD_TRIGGERED);
                                break;
                            }
                            case 28821: // Lightning Shield
                            {
                                // Need remove self if Lightning Shield not active
                                Unit::SpellAuraHolderMap const& auras = unitTarget->GetSpellAuraHolderMap();
                                for (const auto& aura : auras)
                                {
                                    SpellEntry const* spell = aura.second->GetSpellProto();
                                    if (spell->SpellFamilyName == SPELLFAMILY_SHAMAN
                                        && spell->SpellFamilyFlags & uint64(0x0000000000000400))
                                        return;
                                }

                                unitTarget->RemoveAurasDueToSpell(28820);
                                break;
                            }
                        }
                    }

                    return;
                }
                case 19395:                                 // Gordunni Trap
                {
                    if (unitTarget && unitTarget->GetTypeId() == TYPEID_PLAYER)
                        unitTarget->CastSpell(unitTarget, urand(0, 1) ? 19394 : 11756, TRIGGERED_OLD_TRIGGERED);

                    return;
                }
                case 19411:                                 // Lava Bomb
                {
                    if (unitTarget)
                        unitTarget->CastSpell(unitTarget, 20494, TRIGGERED_OLD_TRIGGERED);

                    return;
                }
                case 20474:                                 // Lava Bomb
                {
                    if (unitTarget)
                        unitTarget->CastSpell(unitTarget, 20495, TRIGGERED_OLD_TRIGGERED);

                    return;
                }
                case 20572:                                 // Blood Fury
                {
                    if (m_caster->GetTypeId() == TYPEID_PLAYER)
                    {
                        m_caster->CastSpell(m_caster, 23230, TRIGGERED_OLD_TRIGGERED);

                        damage = uint32(damage * (m_caster->GetTotalAttackPowerValue(BASE_ATTACK)) / 100);
                        m_caster->CastCustomSpell(m_caster, 23234, &damage, nullptr, nullptr, TRIGGERED_OLD_TRIGGERED, nullptr);
                    }

                    return;
                }
                case 19869:                                 // Dragon Orb
                {
                    if (unitTarget && unitTarget->GetTypeId() == TYPEID_PLAYER && !unitTarget->HasAura(23958))
                        unitTarget->CastSpell(nullptr, 19832, TRIGGERED_OLD_TRIGGERED);

                    return;
                }
                case 20037:                                 // Explode Orb Effect
                {
                    if (unitTarget)
                        unitTarget->CastSpell(unitTarget, 20038, TRIGGERED_OLD_TRIGGERED);

                    return;
                }
                case 20577:                                 // Cannibalize
                {
                    m_caster->CastSpell(nullptr, 20578, TRIGGERED_NONE);
                    return;
                }
                case 21108:                                 // Summon Sons of Flame
                {
                    // Trigger the eight summoning spells for the adds in Ragnaros encounter
                    for (const uint32 spell : {21110, 21111, 21112, 21113, 21114, 21115, 21116, 21117})
                    m_caster->CastSpell(m_caster, spell, TRIGGERED_OLD_TRIGGERED, nullptr);
                    return;
                }
                case 21147:                                 // Arcane Vacuum
                {
                    if (unitTarget)
                    {
                        // Spell used by Azuregos to teleport all the players to him
                        // This also resets the target threat
                        if (m_caster->getThreatManager().getThreat(unitTarget))
                            m_caster->getThreatManager().modifyThreatPercent(unitTarget, -100);

                        // cast summon player
                        m_caster->CastSpell(unitTarget, 21150, TRIGGERED_OLD_TRIGGERED);
                    }

                    return;
                }
                case 21908:                                 // Lava Burst Randomizer
                {
                    // randomly cast one of the nine Lava Burst spell A to I in Ragnaros encounter
                    const uint32 spell_list[9] = {21886, 21900, 21901, 21902, 21903, 21904, 21905, 21906, 21907};
                    m_caster->CastSpell(m_caster, spell_list[urand(0, 8)], TRIGGERED_OLD_TRIGGERED);
                    return;
                }
                case 22276:                                 // Elemental Shield
                {
                    uint32 elemshields[] = { 22277, 22278, 22279, 22280, 22281 };

                    for (uint32 spell : elemshields)
                        if (m_caster->HasAura(spell))
                            m_caster->RemoveAurasDueToSpell(spell);
                    m_caster->CastSpell(m_caster, elemshields[urand(0, 4)], TRIGGERED_OLD_TRIGGERED);

                    return;
                }
                case 22282:                                 // Brood Power
                {
                    uint32 broodpowers[] = { 22283, 22285, 22286, 22287, 22288 };

                    for (uint32 spell : broodpowers)
                        if (m_caster->HasAura(spell))
                            m_caster->RemoveAurasDueToSpell(spell);
                    m_caster->CastSpell(m_caster, broodpowers[urand(0, 4)], TRIGGERED_OLD_TRIGGERED);

                    return;
                }
                case 22659:                                 // Spawn Drakonid
                {
                    // Spawn drakonids depending on caster color (spawner type)
                    uint32 spellSummonDrakonid = 0;
                    switch (m_caster->GetEntry())
                    {
                        case 14312: spellSummonDrakonid = 22658; break; // Blue
                        case 14310: spellSummonDrakonid = 22656; break; // Green
                        case 14311: spellSummonDrakonid = 22657; break; // Bronze
                        case 14309: spellSummonDrakonid = 22655; break; // Red
                        case 14307: spellSummonDrakonid = 22654; break; // Black
                        default: return;
                    }
                    m_caster->CastSpell(m_caster, spellSummonDrakonid, TRIGGERED_OLD_TRIGGERED);
                    return;
                }
                case 22664:                                 // Shadowblink
                {
                    if (eff_idx == EFFECT_INDEX_1)
                    {
                        // randomly cast one of the nine Shadowblink spell in Nefarian encounter (phase 1)
                        const uint32 spell_list[9] = {22668, 22669, 22670, 22671, 22672, 22673, 22674, 22675, 22676};
                        m_caster->CastSpell(m_caster, spell_list[urand(0, 8)], TRIGGERED_OLD_TRIGGERED);
                    }
                    return;
                }
                case 23019:                                 // Crystal Prison Dummy DND
                {
                    if (unitTarget && unitTarget->GetTypeId() == TYPEID_UNIT && unitTarget->isAlive())
                    {
                        Creature* creatureTarget = (Creature*)unitTarget;
                        if (!creatureTarget->IsPet())
                        {
                            creatureTarget->CastSpell(creatureTarget, 23022, TRIGGERED_OLD_TRIGGERED);
                            creatureTarget->ForcedDespawn();
                        }
                    }

                    return;
                }
                case 23074:                                 // Arcanite Dragonling
                {
                    if (m_CastItem)
                        m_caster->CastSpell(m_caster, 19804, TRIGGERED_OLD_TRIGGERED, m_CastItem);

                    return;
                }
                case 23075:                                 // Mithril Mechanical Dragonling
                {
                    if (m_CastItem)
                        m_caster->CastSpell(m_caster, 12749, TRIGGERED_OLD_TRIGGERED, m_CastItem);

                    return;
                }
                case 23076:                                 // Mechanical Dragonling
                {
                    if (m_CastItem)
                        m_caster->CastSpell(m_caster, 4073, TRIGGERED_OLD_TRIGGERED, m_CastItem);

                    return;
                }
                case 23133:                                 // Gnomish Battle Chicken
                {
                    if (m_CastItem)
                        m_caster->CastSpell(m_caster, 13166, TRIGGERED_OLD_TRIGGERED, m_CastItem);

                    return;
                }
                case 23134:                                 // Goblin Bomb Dispenser
                {
                    if (m_CastItem)
                        m_caster->CastSpell(m_caster, 13258, TRIGGERED_OLD_TRIGGERED, m_CastItem);

                    return;
                }
                case 23138:                                 // Gate of Shazzrah
                {
                    // Effect probably include a threat change, but it is unclear if fully
                    // reset or just forced upon target for teleport (SMSG_HIGHEST_THREAT_UPDATE)
                    if (unitTarget)
                        m_caster->CastSpell(unitTarget, 23139, TRIGGERED_OLD_TRIGGERED);

                    return;
                }
                case 23173:                                 // Brood Affliction
                {
                    // This spell selects one brood affliction amongst five and apply it onto 15 targets
                    // If there are less than 15 targets, then the spell loops over the targets again
                    // until there are 15 appliance of the chosen brood affliction

                    // Brood Affliction selection
                    uint32 afflictions[] = { 23153, 23154, 23155, 23170, 23169 };   // Blue / Black / Red / Bronze / Green
                    uint32 spellAfflict = afflictions[urand(0, 4)];

                    // Get the fifteen (potentially duplicate) targets from threat list
                    GuidVector vGuids;
                    ((Creature*)m_caster)->FillGuidsListFromThreatList(vGuids);

                    std::vector<Unit*> fifteenTargets;
                    uint8 targetsCount = 0;
                    while (targetsCount < 15)
                    {
                        for (GuidVector::const_iterator i = vGuids.begin(); i != vGuids.end(); ++i)
                        {
                            Unit* unit = m_caster->GetMap()->GetUnit(*i);
                            if (unit && targetsCount < 15 && unit->GetTypeId() == TYPEID_PLAYER && unit->isAlive())
                            {
                                fifteenTargets.push_back(unit);
                                targetsCount++;
                            }
                            else break;
                        }
                        // Prevent infinite loop: if fifteenTargets is still empty after first iteration: return
                        if (targetsCount == 0)
                        {
                            return;
                        }
                    }

                    for (auto unit : fifteenTargets)
                    {
                        // Cast Brood Affliction if not mutated
                        if (!(unit->HasAura(23174, EFFECT_INDEX_0)))
                            m_caster->CastSpell(unit, spellAfflict, TRIGGERED_OLD_TRIGGERED);

                        // Cast Chromatic Mutation (23174) if target is now affected by all five brood afflictions
                        if (unit->HasAura(23153, EFFECT_INDEX_0)
                                && unit->HasAura(23154, EFFECT_INDEX_0)
                                && unit->HasAura(23155, EFFECT_INDEX_0)
                                && unit->HasAura(23170, EFFECT_INDEX_0)
                                && unit->HasAura(23169, EFFECT_INDEX_0))
                        {
                            unit->RemoveAllAuras();
                            m_caster->CastSpell(unit, 23174, TRIGGERED_OLD_TRIGGERED);
                            unit->CastSpell(unit, 23175, TRIGGERED_OLD_TRIGGERED);
                            unit->CastSpell(unit, 23177, TRIGGERED_OLD_TRIGGERED);
                        }
                    }
                    return;
                }
                case 23195:                                 // Dragondog Breath Selection
                {
                    // This spell selects two breaths for Chromaggus amongst five and send them to BWL instance script through events

                    // Breaths selection
                    // 5 possiblities for the first breath, 4 for the second: 20 total possiblites
                    uint8 rightBreath = urand(0, 4);
                    uint8 leftBreath = (rightBreath + urand(1, 4)) % 5;

                    // Sending left breath to instance script
                    uint32 leftBreaths[] = { 23317, 23318, 23319, 23320, 23321 };
                    m_caster->CastSpell(m_caster, leftBreaths[leftBreath], TRIGGERED_OLD_TRIGGERED);

                    // Sending right breath to instance script
                    uint32 rightBreaths[] = { 23322, 23323, 23324, 23325, 23326 };
                    m_caster->CastSpell(m_caster, rightBreaths[rightBreath], TRIGGERED_OLD_TRIGGERED);

                    return;
                }
                case 23343:                                 // Nefarian Tunnel Selection
                {
                    // This spell selects two kinds of drakonids amongst five and send them to BWL instance script through events

                    // Drakonids selection
                    // 5 possiblities for the first kind, 4 for the second: 20 total possiblites
                    uint8 rightDrakonid = urand(0, 4);
                    uint8 leftDrakonid = (rightDrakonid + urand(1, 4)) % 5;

                    // Sending left tunnel selection to instance script
                    uint32 leftDrakonids[] = { 23344, 23345, 23346, 23347, 23348 };
                    m_caster->CastSpell(m_caster, leftDrakonids[leftDrakonid], TRIGGERED_OLD_TRIGGERED);

                    // Sending right tunnel selection to instance script
                    uint32 rightDrakonids[] = { 23349, 23350, 23351, 23352, 23353 };
                    m_caster->CastSpell(m_caster, rightDrakonids[rightDrakonid], TRIGGERED_OLD_TRIGGERED);

                    return;
                }
                case 23450:                                 // Transporter Arrival - (Gadgetzan + Everlook)
                {
                    uint32 rand = urand(0, 5);              // Roll for minor malfunctions:
                    if (!rand)                              // (1/6) - Polymorph (23456 & 23457 are both carbon copies of this one)
                        m_caster->CastSpell(m_caster, 23444, TRIGGERED_OLD_TRIGGERED);
                    else if (rand <= 2)                     // (2/6) - Evil Twin
                        m_caster->CastSpell(m_caster, 23445, TRIGGERED_OLD_TRIGGERED);

                    return;
                }
                case 23453:                                 // Gnomish Transporter - Ultrasafe Transporter: Gadgetzan
                {
                    // Roll chance for major malfunction (1/6); 23441 = success | 23446 = malfunction (missed destination)
                    m_caster->CastSpell(m_caster, (urand(0, 5) ? 23441 : 23446), TRIGGERED_OLD_TRIGGERED);

                    return;
                }
                case 23645:                                 // Hourglass Sand
                {
                    m_caster->RemoveAurasDueToSpell(23170); // Brood Affliction: Bronze

                    return;
                }
                case 23725:                                 // Gift of Life (warrior bwl trinket)
                {
                    int32 basepoints = m_caster->GetMaxHealth() * 0.15;
                    m_caster->CastCustomSpell(m_caster, 23782, &basepoints, nullptr, nullptr, TRIGGERED_OLD_TRIGGERED, nullptr);
                    m_caster->CastCustomSpell(m_caster, 23783, &basepoints, nullptr, nullptr, TRIGGERED_OLD_TRIGGERED, nullptr);

                    return;
                }
                case 23484:                                 // Dispel Drakonids
                {
                    // Despawn all drakonids from Nefarian phase 1
                    if (unitTarget->GetTypeId() == TYPEID_UNIT)
                    {
                        switch (unitTarget->GetEntry())
                        {
                            case 14261:
                            case 14262:
                            case 14263:
                            case 14264:
                            case 14265:
                            case 14302:
                                ((Creature*)unitTarget)->ForcedDespawn();
                        }
                    }
                    return;
                }
                case 24019:                                 // Gurubashi Axe Thrower; Axe Flurry.
                {
                    if (unitTarget && m_caster->IsWithinLOSInMap(unitTarget))
                        m_caster->CastSpell(unitTarget, 24020, TRIGGERED_OLD_TRIGGERED);

                    return;
                }
                case 24150:                                 // Stinger Charge Primer
                {
                    if (unitTarget->HasAura(25187))
                        m_caster->CastSpell(unitTarget, 25191, TRIGGERED_OLD_TRIGGERED);
                    else
                        m_caster->CastSpell(unitTarget, 25190, TRIGGERED_OLD_TRIGGERED);

                    return;
                }
                case 24796:                                 // Summon Demented Druid Spirit
                {
                    if (unitTarget && unitTarget->GetTypeId() == TYPEID_PLAYER)
                        m_caster->CastSpell(unitTarget, 24795, TRIGGERED_OLD_TRIGGERED);

                    return;
                }
                case 25145:                                 // Merithra's Wake
                {
                    unitTarget->CastSpell(unitTarget, 25172, TRIGGERED_OLD_TRIGGERED);
                    return;
                }
                case 25149:                                 // Arygos's Vengeance
                {
                    unitTarget->CastSpell(unitTarget, 25168, TRIGGERED_OLD_TRIGGERED);
                    return;
                }
                case 25150:                                 // Molten Rain
                {
                    unitTarget->CastSpell(unitTarget, 25169, TRIGGERED_OLD_TRIGGERED);
                    unitTarget->CastSpell(unitTarget, 25170, TRIGGERED_OLD_TRIGGERED);
                    return;
                }
                case 25158:                                 // Time Stop
                {
                    unitTarget->SetImmuneToNPC(true);
                    unitTarget->RemoveAllAuras();
                    unitTarget->CastSpell(unitTarget, 25171, TRIGGERED_OLD_TRIGGERED);
                    return;
                }
                case 26080:                                 // Stinger Charge Primer
                {
                    if (unitTarget->HasAura(26078))
                        m_caster->CastSpell(unitTarget, 26082, TRIGGERED_OLD_TRIGGERED);
                    else
                        m_caster->CastSpell(unitTarget, 26081, TRIGGERED_OLD_TRIGGERED);

                    return;
                }
                case 24721:                                 // Buru Transform
                {
                    // remaining Buru Eggs summon Hive'Zara Hatchlings and despawn
                    if (unitTarget->GetEntry() == 15514)
                    {
                        unitTarget->CastSpell(unitTarget, 1881, TRIGGERED_OLD_TRIGGERED);
                        ((Creature*)unitTarget)->ForcedDespawn();
                    }

                    return;
                }
                case 24781:                                 // Dream Fog
                {
                    // Let the current target be and move to the new one
                    if (m_caster->getVictim())
                    {
                        m_caster->DeleteThreatList();
                        m_caster->AttackStop(true, false, false);
                    }
                    if (unitTarget && m_caster->AI())
                        m_caster->AI()->AttackStart(unitTarget);

                    return;
                }
                case 24886:                                 // Despawn Taerar Shades
                {
                    if (unitTarget && unitTarget->GetEntry() == 15302)
                    {
                        ((Creature*)unitTarget)->ForcedDespawn();
                        ((Creature*)unitTarget)->RemoveFromWorld();
                    }

                    return;
                }
                case 24930:                                 // Hallow's End Treat
                {
                    // 24924 = Larger and Orange | 24925 = Skeleton | 24926 = Pirate | 24927 = Ghost
                    const uint32 spell_list[4] = {24924, 24925, 24926, 24927};
                    m_caster->CastSpell(m_caster, spell_list[urand(0, 3)], TRIGGERED_OLD_TRIGGERED);

                    return;
                }
                case 25860:                                 // Reindeer Transformation
                {
                    if (m_caster->HasAuraType(SPELL_AURA_MOUNTED))
                    {
                        m_caster->RemoveSpellsCausingAura(SPELL_AURA_MOUNTED);

                        // 25859 = 100% ground reindeer | 25858 = 60% ground reindeer
                        m_caster->CastSpell(m_caster, ((m_caster->GetSpeedRate(MOVE_RUN) >= 2.0f) ? 25859 : 25858), TRIGGERED_OLD_TRIGGERED);
                    }

                    return;
                }
                case 26074:                                 // Holiday Cheer
                {
                    // implemented at client side
                    return;
                }
                case 26374:                                 // Elune's Candle
                {
                    if (unitTarget->GetTypeId() == TYPEID_UNIT && unitTarget->GetEntry() == 15467)  // Omen
                    {
                        uint32 eluneCandle[5] = { 26622, 26623, 26624, 26625, 26649 };
                        m_caster->CastSpell(unitTarget, eluneCandle[urand(0, 4)], TRIGGERED_OLD_TRIGGERED); // Damage (random visual)
                        return;
                    }
                    // Default harmless spell
                    m_caster->CastSpell(unitTarget, 26636, TRIGGERED_OLD_TRIGGERED);

                    return;
                }
                case 26626:                                 // Mana Burn Area
                {
                    if (unitTarget->GetTypeId() == TYPEID_UNIT && unitTarget->GetPowerType() == POWER_MANA)
                        m_caster->CastSpell(unitTarget, 25779, TRIGGERED_OLD_TRIGGERED);

                    return;
                }
                case 26899:                                 // Give Friendship Bracers
                {
                    if ((unitTarget && unitTarget->GetTypeId() == TYPEID_UNIT) || !unitTarget->HasAura(26898)) // Target is not a player or is not heartbroken
                        return;

                    unitTarget->RemoveAurasDueToSpell(26898);                               // Remove Heartbroken
                    unitTarget->CastSpell(unitTarget, 26921, TRIGGERED_OLD_TRIGGERED);      // cast Cancel Heartbroken, Create Bracelet
                    return;
                }
                case 28006:                                 // Arcane Cloaking
                {
                    // Naxxramas Entry Flag Effect DND
                    if (unitTarget && unitTarget->GetTypeId() == TYPEID_PLAYER)
                        m_caster->CastSpell(unitTarget, 29294, TRIGGERED_OLD_TRIGGERED);

                    return;
                }
                case 28089:                                 // Polarity Shift
                {
                    if (!unitTarget)
                        return;

                    unitTarget->RemoveAurasDueToSpell(28059);
                    unitTarget->RemoveAurasDueToSpell(28084);

                    // 28059 28084
                    switch (m_scriptValue)
                    {
                        case 0: // first target random
                            m_scriptValue = urand(0, 1) ? 28059 : 28084;
                            unitTarget->CastSpell(unitTarget, m_scriptValue, TRIGGERED_OLD_TRIGGERED);
                            break;
                        case 28059: // second target the other
                            m_scriptValue = 1;
                            unitTarget->CastSpell(unitTarget, 28084, TRIGGERED_OLD_TRIGGERED);
                            break;
                        case 28084:
                            m_scriptValue = 1;
                            unitTarget->CastSpell(unitTarget, 28059, TRIGGERED_OLD_TRIGGERED);
                            break;
                        default: // third and later random
                            unitTarget->CastSpell(unitTarget, urand(0, 1) ? 28059 : 28084, TRIGGERED_OLD_TRIGGERED);
                            break;
                    }
                    return;
                }
                case 28098:                                 // Stalagg Tesla Effect
                case 28110:                                 // Feugen Tesla Effect
                {
                    if (unitTarget && unitTarget->GetTypeId() == TYPEID_UNIT)
                    {
                        if (m_caster->getVictim() && !m_caster->IsWithinDistInMap(unitTarget, 60.0f))
                        {
                            // Cast Shock on nearby targets
                            if (Unit* pTarget = ((Creature*)m_caster)->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
                                unitTarget->CastSpell(pTarget, 28099, TRIGGERED_NONE);
                        }
                        else
                        {
                            // "Evade"
                            unitTarget->RemoveAurasDueToSpell(m_spellInfo->Id == 28098 ? 28097 : 28109);
                            unitTarget->CombatStop(true);
                            // Recast chain (Stalagg Chain or Feugen Chain
                            unitTarget->CastSpell(m_caster, m_spellInfo->Id == 28098 ? 28096 : 28111, TRIGGERED_NONE);
                        }
                    }
                    return;
                }
                case 28414:                                 // Call of the Ashbringer
                {
                    if (!m_caster || m_caster->GetTypeId() != TYPEID_PLAYER)
                        return;
                    static constexpr uint32 AshbringerSounds[12] = { 8906,8907,8908,8920,8921,8922,8923,8924,8925,8926,8927,8928 };
                    m_caster->PlayDirectSound(AshbringerSounds[urand(0, 11)]);
                    return;
                }
                case 28697:                                 // Forgiveness
                {
                    m_caster->DealDamage(unitTarget, unitTarget->GetHealth(), nullptr, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, nullptr, false);
                    return;
                }
            }

            // All IconID Check in there
            switch (m_spellInfo->SpellIconID)
            {
                // Berserking (troll racial traits)
                case 1661:
                {
                    uint32 healthPerc = uint32((float(m_caster->GetHealth()) / m_caster->GetMaxHealth()) * 100);
                    int32 haste_mod = 10;
                    if (healthPerc <= 40)
                        haste_mod = 30;
                    if (healthPerc < 100 && healthPerc > 40)
                        haste_mod = 10 + (100 - healthPerc) / 3;

                    // haste_mod is the same for all effects
                    int32 hasteModBasePoints0 = haste_mod;    // Haste Melee
                    int32 hasteModBasePoints1 = haste_mod;    // Haste Range
                    int32 hasteModBasePoints2 = haste_mod;    // Haste Cast

                    // FIXME: custom spell required this aura state by some unknown reason, we not need remove it anyway
                    m_caster->ModifyAuraState(AURA_STATE_BERSERKING, true);
                    m_caster->CastCustomSpell(m_caster, 26635, &hasteModBasePoints0, &hasteModBasePoints1, &hasteModBasePoints2, TRIGGERED_OLD_TRIGGERED, nullptr);
                    return;
                }
            }
            break;
        }
        case SPELLFAMILY_MAGE:
        {
            switch (m_spellInfo->Id)
            {
                case 11189:                                 // Frost Warding
                case 28332:
                {
                    // increase reflection chance (effect 1) of Frost Ward, removed in aura boosts
                    if (unitTarget && unitTarget->GetTypeId() == TYPEID_PLAYER)
                    {
                        SpellModifier* mod = new SpellModifier(SPELLMOD_RESIST_MISS_CHANCE, SPELLMOD_FLAT, damage, m_spellInfo->Id, uint64(0x0000000000000100));
                        ((Player*)unitTarget)->AddSpellMod(mod, true);
                    }

                    break;
                }
                case 12472:                                 // Cold Snap
                {
                    if (m_caster->GetTypeId() == TYPEID_PLAYER)
                    {
                        // immediately finishes the cooldown on Frost spells
                        auto cdCheck = [](SpellEntry const & spellEntry) -> bool
                        {
                            if (spellEntry.SpellFamilyName != SPELLFAMILY_MAGE)
                                return false;
                            if ((GetSpellSchoolMask(&spellEntry) & SPELL_SCHOOL_MASK_FROST) && GetSpellRecoveryTime(&spellEntry) > 0)
                                return true;
                            return false;
                        };
                        static_cast<Player*>(m_caster)->RemoveSomeCooldown(cdCheck);
                    }

                    return;
                }
            }
            break;
        }
        case SPELLFAMILY_WARRIOR:
        {
            switch (m_spellInfo->Id)
            {
                case 21977:                  // Warrior's Wrath
                {
                    if (unitTarget)
                        m_caster->CastSpell(unitTarget, 21887, TRIGGERED_OLD_TRIGGERED); // spell mod

                    return;
                }
                case 23424:                  // Corrupted Totems (Shaman Nefarian class call)
                {
                    uint32 totemsSpellIds[] = { 23419, 23420, 23422, 23423 };   // Corrupted Fire Nova Totem / Corrupted Stoneskin Totem / Corrupted Healing Stream Totem (Rank 5) / Corrupted Windfury Totem
                    m_caster->CastSpell(m_caster, totemsSpellIds[urand(0, 3)], TRIGGERED_OLD_TRIGGERED);
                    return;
                }
            }

            break;
        }
        case SPELLFAMILY_WARLOCK:
        {
            // Life Tap
            if (m_spellInfo->SpellFamilyFlags & uint64(0x0000000000040000))
            {
                float cost = m_currentBasePoints[EFFECT_INDEX_0];

                if (Player* modOwner = m_caster->GetSpellModOwner())
                    modOwner->ApplySpellMod(m_spellInfo->Id, SPELLMOD_COST, cost, this);

                int32 dmg = m_caster->SpellDamageBonusTaken(m_caster, m_spellInfo, (m_caster->SpellDamageBonusDone(m_caster, m_spellInfo, uint32(cost > 0 ? cost : 0), SPELL_DIRECT_DAMAGE)), SPELL_DIRECT_DAMAGE);

                if (int32(m_caster->GetHealth()) > dmg)
                {
                    int32 mana = dmg;

                    Unit::AuraList const& auraDummy = m_caster->GetAurasByType(SPELL_AURA_DUMMY);
                    for (Unit::AuraList::const_iterator itr = auraDummy.begin(); itr != auraDummy.end(); ++itr)
                    {
                        if ((*itr)->isAffectedOnSpell(m_spellInfo))
                        {
                            switch ((*itr)->GetSpellProto()->Id)
                            {
                                case 28830: // Plagueheart Rainment - reduce hp cost
                                    dmg += dmg * (*itr)->GetModifier()->m_amount / 100; break;
                                    // Improved Life Tap
                                default: mana = ((*itr)->GetModifier()->m_amount + 100) * mana / 100; break;
                            }
                        }
                    }

                    // Shouldn't Appear in Combat Log
                    m_caster->ModifyHealth(-dmg);

                    m_caster->CastCustomSpell(m_caster, 31818, &mana, nullptr, nullptr, TRIGGERED_OLD_TRIGGERED);
                }
                else
                    SendCastResult(SPELL_FAILED_FIZZLE);

                return;
            }
            break;
        }
        case SPELLFAMILY_PRIEST:
        {
            switch (m_spellInfo->Id)
            {
                case 28598:                                 // Touch of Weakness triggered spell
                {
                    if (!unitTarget || !m_triggeredByAuraSpell)
                        return;

                    uint32 spellid;
                    switch (m_triggeredByAuraSpell->Id)
                    {
                        case 2652:  spellid =  2943; break; // Rank 1
                        case 19261: spellid = 19249; break; // Rank 2
                        case 19262: spellid = 19251; break; // Rank 3
                        case 19264: spellid = 19252; break; // Rank 4
                        case 19265: spellid = 19253; break; // Rank 5
                        case 19266: spellid = 19254; break; // Rank 6
                        case 25461: spellid = 25460; break; // Rank 7
                        default:
                            sLog.outError("Spell::EffectDummy: Spell 28598 triggered by unhandeled spell %u", m_triggeredByAuraSpell->Id);
                            return;
                    }
                    m_caster->CastSpell(unitTarget, spellid, TRIGGERED_OLD_TRIGGERED, nullptr);
                    return;
                }
            }

            break;
        }
        case SPELLFAMILY_DRUID:
        {
            switch (m_spellInfo->Id)
            {
                case 5229:                                  // Enrage
                {
                    int32 reductionMod = -27;
                    if (m_caster->HasAura(9634)) // If in Dire Bear form only 16%
                        reductionMod = -16;
                    m_caster->CastCustomSpell(nullptr, 25503, &reductionMod, nullptr, nullptr, TRIGGERED_OLD_TRIGGERED);
                    return;
                }
                case 29201:                                 // Loatheb Corrupted Mind triggered sub spells
                {
                    uint32 spellid = 0;
                    switch (unitTarget->getClass())
                    {
                        case CLASS_PALADIN: spellid = 29196; break;
                        case CLASS_PRIEST: spellid = 29185; break;
                        case CLASS_SHAMAN: spellid = 29198; break;
                        case CLASS_DRUID: spellid = 29194; break;
                        default: break;
                    }
                    if (spellid != 0)
                        m_caster->CastSpell(unitTarget, spellid, TRIGGERED_OLD_TRIGGERED, nullptr);
                    return;
                }
            }
            break;
        }
        case SPELLFAMILY_ROGUE:
            break;
        case SPELLFAMILY_HUNTER:
        {
            // Steady Shot
            if (m_spellInfo->SpellFamilyFlags & uint64(0x100000000))
            {
                if (unitTarget && unitTarget->isAlive())
                {
                    bool found = false;

                    // check dazed affect
                    Unit::AuraList const& decSpeedList = unitTarget->GetAurasByType(SPELL_AURA_MOD_DECREASE_SPEED);
                    for (Unit::AuraList::const_iterator iter = decSpeedList.begin(); iter != decSpeedList.end(); ++iter)
                    {
                        if ((*iter)->GetSpellProto()->SpellIconID == 15 && (*iter)->GetSpellProto()->Dispel == 0)
                        {
                            found = true;
                            break;
                        }
                    }

                    if (found)
                        m_damage += damage;
                }

                return;
            }

            switch (m_spellInfo->Id)
            {
                case 23989:                                 // Readiness talent
                {
                    if (m_caster->GetTypeId() == TYPEID_PLAYER)
                    {
                        // immediately finishes the cooldown for hunter abilities
                        auto cdCheck = [](SpellEntry const & spellEntry) -> bool { return (spellEntry.SpellFamilyName == SPELLFAMILY_HUNTER && spellEntry.Id != 23989 && GetSpellRecoveryTime(&spellEntry) > 0); };
                        static_cast<Player*>(m_caster)->RemoveSomeCooldown(cdCheck);
                    }

                    return;
                }
            }

            break;
        }
        case SPELLFAMILY_PALADIN:
        {
            switch (m_spellInfo->SpellIconID)
            {
                case 156:                                   // Holy Shock
                {
                    if (!unitTarget)
                        return;

                    int hurt = 0;
                    int heal = 0;

                    switch (m_spellInfo->Id)
                    {
                        case 20473: hurt = 25912; heal = 25914; break;
                        case 20929: hurt = 25911; heal = 25913; break;
                        case 20930: hurt = 25902; heal = 25903; break;
                        default:
                            sLog.outError("Spell::EffectDummy: Spell %u not handled in HS", m_spellInfo->Id);
                            return;
                    }

                    if (m_caster->CanAssistSpell(unitTarget, m_spellInfo))
                        m_caster->CastSpell(unitTarget, heal, TRIGGERED_OLD_TRIGGERED);
                    else
                        m_caster->CastSpell(unitTarget, hurt, TRIGGERED_OLD_TRIGGERED);

                    return;
                }
                case 561:                                   // Judgement of command
                {
                    if (unitTarget)
                        if (SpellEntry const* spell_proto = sSpellTemplate.LookupEntry<SpellEntry>(m_currentBasePoints[eff_idx]))
                        {
                            if (!unitTarget->IsStunned() && m_caster->GetTypeId() == TYPEID_PLAYER)
                            {
                                // decreased damage (/2) for non-stunned target.
                                SpellModifier* mod = new SpellModifier(SPELLMOD_DAMAGE, SPELLMOD_PCT, -50, m_spellInfo->Id, uint64(0x0000020000000000));

                                ((Player*)m_caster)->AddSpellMod(mod, true);
                                m_caster->CastSpell(unitTarget, spell_proto, TRIGGERED_OLD_TRIGGERED, nullptr);
                                // mod deleted
                                ((Player*)m_caster)->AddSpellMod(mod, false);
                            }
                            else
                                m_caster->CastSpell(unitTarget, spell_proto, TRIGGERED_OLD_TRIGGERED, nullptr);
                        }

                    return;
                }
            }

            break;
        }
        case SPELLFAMILY_SHAMAN:
        {
            if (m_spellInfo->SpellFamilyFlags & uint64(0x0000000000200000)) // Flametongue Weapon Proc, Ranks
            {
                if (m_CastItem)
                {
                    // found spelldamage coefficients of 0.381% per 0.1 speed and 15.244 per 4.0 speed
                    // but own calculation say 0.385 gives at most one point difference to published values
                    int32 bonusDamage = m_caster->SpellBaseDamageBonusDone(GetSpellSchoolMask(m_spellInfo))
                                        + unitTarget->SpellBaseDamageBonusTaken(GetSpellSchoolMask(m_spellInfo));
                    // Does Amplify Magic/Dampen Magic influence flametongue? If not, the above addition must be removed.
                    float weaponSpeed = float(m_CastItem->GetProto()->Delay) / IN_MILLISECONDS;
                    bonusDamage = m_caster->SpellBonusWithCoeffs(m_spellInfo, 0, bonusDamage, 0, SPELL_DIRECT_DAMAGE, false); // apply spell coeff
                    int32 totalDamage = (damage * 0.01 * weaponSpeed) + bonusDamage;

                    m_caster->CastCustomSpell(unitTarget, 10444, &totalDamage, nullptr, nullptr, TRIGGERED_OLD_TRIGGERED, m_CastItem);
                }
                else
                    sLog.outError("Spell::EffectDummy: spell %i requires cast Item", m_spellInfo->Id);

                return;
            }

            if (m_spellInfo->SpellFamilyFlags & uint64(0x0000000400000000)) // Flametongue Totem Proc, Ranks
            {
                if (m_CastItem) // Does not scale with gear
                {
                    float weaponSpeed = float(m_CastItem->GetProto()->Delay) / IN_MILLISECONDS;
                    int32 totalDamage = (damage * 0.01 * weaponSpeed);
                    m_caster->CastCustomSpell(unitTarget, 16368, &totalDamage, nullptr, nullptr, true, m_CastItem);
                }
                else
                    sLog.outError("Spell::EffectDummy: spell %i requires cast Item", m_spellInfo->Id);

                return;
            }
            break;
        }
    }

    // pet auras
    if (PetAura const* petSpell = sSpellMgr.GetPetAura(m_spellInfo->Id))
    {
        m_caster->AddPetAura(petSpell);
        return;
    }

    // Script based implementation. Must be used only for not good for implementation in core spell effects
    // So called only for not processed cases
    bool libraryResult = false;
    if (gameObjTarget)
        libraryResult = sScriptDevAIMgr.OnEffectDummy(m_caster, m_spellInfo->Id, eff_idx, gameObjTarget, m_originalCasterGUID);
    else if (unitTarget && unitTarget->GetTypeId() == TYPEID_UNIT)
        libraryResult = sScriptDevAIMgr.OnEffectDummy(m_caster, m_spellInfo->Id, eff_idx, (Creature*)unitTarget, m_originalCasterGUID);
    else if (itemTarget)
        libraryResult = sScriptDevAIMgr.OnEffectDummy(m_caster, m_spellInfo->Id, eff_idx, itemTarget, m_originalCasterGUID);

    if (unitTarget && !libraryResult)
    {
        // Previous effect might have started script
        if (ScriptMgr::CanSpellEffectStartDBScript(m_spellInfo, eff_idx))
        {
            DEBUG_FILTER_LOG(LOG_FILTER_SPELL_CAST, "Spell ScriptStart spellid %u in EffectDummy", m_spellInfo->Id);
            m_caster->GetMap()->ScriptsStart(sSpellScripts, m_spellInfo->Id, m_caster, unitTarget);
        }
    }
}

void Spell::EffectTriggerSpell(SpellEffectIndex eff_idx)
{
    uint32 triggered_spell_id = m_spellInfo->EffectTriggerSpell[eff_idx];

    // special cases
    switch (triggered_spell_id)
    {
        // Temporal Parasite Summon #2, special case because chance is set to 101% in DBC while description is 67%
        case 16630:
            if (urand(0, 100) < 67)
                m_caster->CastSpell(unitTarget, triggered_spell_id, TRIGGERED_OLD_TRIGGERED);
            return;
        // Temporal Parasite Summon #3, special case because chance is set to 101% in DBC while description is 34%
        case 16631:
            if (urand(0, 100) < 34)
                m_caster->CastSpell(unitTarget, triggered_spell_id, TRIGGERED_OLD_TRIGGERED);
            return;
        // Vanish (not exist)
        case 18461:
        {
            unitTarget->RemoveSpellsCausingAura(SPELL_AURA_MOD_ROOT);
            unitTarget->RemoveSpellsCausingAura(SPELL_AURA_MOD_DECREASE_SPEED);
            unitTarget->RemoveSpellsCausingAura(SPELL_AURA_MOD_STALKED);

            // if this spell is given to NPC it must handle rest by it's own AI
            if (unitTarget->GetTypeId() != TYPEID_PLAYER)
                return;

            // get highest rank of the Stealth spell
            SpellEntry const* stealthSpellEntry = nullptr;
            const PlayerSpellMap& sp_list = ((Player*)unitTarget)->GetSpellMap();
            for (PlayerSpellMap::const_iterator itr = sp_list.begin(); itr != sp_list.end(); ++itr)
            {
                // only highest rank is shown in spell book, so simply check if shown in spell book
                if (!itr->second.active || itr->second.disabled || itr->second.state == PLAYERSPELL_REMOVED)
                    continue;

                SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(itr->first);
                if (!spellInfo)
                    continue;

                if (spellInfo->IsFitToFamily(SPELLFAMILY_ROGUE, uint64(0x0000000000400000)))
                {
                    stealthSpellEntry = spellInfo;
                    break;
                }
            }

            // no Stealth spell found
            if (!stealthSpellEntry)
                return;

            // reset cooldown on it if needed
            if (!unitTarget->IsSpellReady(*stealthSpellEntry))
                unitTarget->RemoveSpellCooldown(*stealthSpellEntry);

            m_caster->CastSpell(unitTarget, stealthSpellEntry, TRIGGERED_OLD_TRIGGERED);
            return;
        }
        // Terrordale Haunting Spirit #2, special case because chance is set to 101% in DBC while description is 55%
        case 23209:
            if (urand(0, 100) < 55)
                m_caster->CastSpell(unitTarget, triggered_spell_id, TRIGGERED_OLD_TRIGGERED);
            return;
        // Terrordale Haunting Spirit #3, special case because chance is set to 101% in DBC while description is 35%
        case 23253:
            if (urand(0, 100) < 35)
                m_caster->CastSpell(unitTarget, triggered_spell_id, TRIGGERED_OLD_TRIGGERED);
            return;
        // just skip
        case 23770:                                         // Sayge's Dark Fortune of *
            // not exist, common cooldown can be implemented in scripts if need.
            return;
        // Brittle Armor - (need add max stack of 24575 Brittle Armor)
        case 29284:
            m_caster->CastSpell(unitTarget, 24575, TRIGGERED_OLD_TRIGGERED, m_CastItem, nullptr, m_originalCasterGUID);
            return;
        // Mercurial Shield - (need add max stack of 26464 Mercurial Shield)
        case 29286:
            m_caster->CastSpell(unitTarget, 26464, TRIGGERED_OLD_TRIGGERED, m_CastItem, nullptr, m_originalCasterGUID);
            return;
    }

    // normal case
    SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(triggered_spell_id);
    if (!spellInfo)
    {
        sLog.outError("EffectTriggerSpell of spell %u: triggering unknown spell id %i", m_spellInfo->Id, triggered_spell_id);
        return;
    }

    // select formal caster for triggered spell
    Unit* caster = m_caster;

    // some triggered spells require specific equipment
    if (spellInfo->EquippedItemClass >= 0 && m_caster->GetTypeId() == TYPEID_PLAYER)
    {
        // main hand weapon required
        if (spellInfo->AttributesEx3 & SPELL_ATTR_EX3_MAIN_HAND)
        {
            Item* item = ((Player*)m_caster)->GetWeaponForAttack(BASE_ATTACK, true, false);

            // skip spell if no weapon in slot or broken
            if (!item)
                return;

            // skip spell if weapon not fit to triggered spell
            if (!item->IsFitToSpellRequirements(spellInfo))
                return;
        }

        // offhand hand weapon required
        if (spellInfo->AttributesEx3 & SPELL_ATTR_EX3_REQ_OFFHAND)
        {
            Item* item = ((Player*)m_caster)->GetWeaponForAttack(OFF_ATTACK, true, false);

            // skip spell if no weapon in slot or broken
            if (!item)
                return;

            // skip spell if weapon not fit to triggered spell
            if (!item->IsFitToSpellRequirements(spellInfo))
                return;
        }
    }

    switch (triggered_spell_id) // custom targeting cases - for now classic only
    {
        case 6608: // Lash
            caster = unitTarget;
            break;
    }

    SpellCastTargets targets;

    switch (m_spellInfo->EffectImplicitTargetA[eff_idx])
    {
        case TARGET_LOCATION_UNIT_MINION_POSITION: break; // confirmed by 31348 nothing is forwarded
        default: targets.setUnitTarget(unitTarget);
    }

    if (spellInfo->Targets & TARGET_FLAG_DEST_LOCATION)
    {
        if (m_targets.m_targetMask & TARGET_FLAG_DEST_LOCATION)
        {
            float x, y, z;
            m_targets.getDestination(x, y, z);
            targets.setDestination(x, y, z);
        }
        else if (unitTarget)
        {
            float x, y, z;
            unitTarget->GetPosition(x, y, z);
            targets.setDestination(x, y, z);
        }
    }

    caster->CastSpell(targets, spellInfo, TRIGGERED_OLD_TRIGGERED, m_CastItem, nullptr, m_originalCasterGUID, m_spellInfo);
}

void Spell::EffectTriggerMissileSpell(SpellEffectIndex effect_idx)
{
    uint32 triggered_spell_id = m_spellInfo->EffectTriggerSpell[effect_idx];

    // normal case
    SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(triggered_spell_id);

    if (!spellInfo)
    {
        if (unitTarget)
        {
            DEBUG_FILTER_LOG(LOG_FILTER_SPELL_CAST, "Spell ScriptStart spellid %u in EffectTriggerMissileSpell", m_spellInfo->Id);
            m_caster->GetMap()->ScriptsStart(sSpellScripts, m_spellInfo->Id, m_caster, unitTarget);
        }
        else
            sLog.outError("EffectTriggerMissileSpell of spell %u (eff: %u): triggering unknown spell id %u",
                          m_spellInfo->Id, effect_idx, triggered_spell_id);
        return;
    }

    if (m_CastItem)
        DEBUG_FILTER_LOG(LOG_FILTER_SPELL_CAST, "WORLD: cast Item spellId - %i", spellInfo->Id);

    SpellCastTargets targets;
    if (unitTarget)
        m_targets.setUnitTarget(unitTarget);
    else if (gameObjTarget)
        m_targets.setGOTarget(gameObjTarget);
    else
        m_targets.setDestination(m_targets.m_destX, m_targets.m_destY, m_targets.m_destZ);
    m_caster->CastSpell(m_targets, spellInfo, TRIGGERED_OLD_TRIGGERED, m_CastItem, nullptr, m_originalCasterGUID, m_spellInfo);
}

void Spell::EffectTeleportUnits(SpellEffectIndex eff_idx)   // TODO - Use target settings for this effect!
{
    if (!unitTarget || unitTarget->IsTaxiFlying())
        return;

    // Target dependend on TargetB, if there is none provided, decide dependend on A
    uint32 targetType = m_spellInfo->EffectImplicitTargetB[eff_idx];
    if (!targetType)
        targetType = m_spellInfo->EffectImplicitTargetA[eff_idx];

    switch (targetType)
    {
        case TARGET_LOCATION_CASTER_HOME_BIND:
        {
            // Only players can teleport to innkeeper
            if (unitTarget->GetTypeId() != TYPEID_PLAYER)
                return;

            ((Player*)unitTarget)->TeleportToHomebind(unitTarget == m_caster ? TELE_TO_SPELL : 0);
            return;
        }
        case TARGET_ENUM_UNITS_SCRIPT_AOE_AT_SRC_LOC:                     // in all cases first TARGET_LOCATION_DATABASE
        case TARGET_LOCATION_DATABASE:
        {
            SpellTargetPosition const* st = sSpellMgr.GetSpellTargetPosition(m_spellInfo->Id);
            if (!st)
            {
                sLog.outError("Spell::EffectTeleportUnits - unknown Teleport coordinates for spell ID %u", m_spellInfo->Id);
                return;
            }

            if (st->target_mapId == unitTarget->GetMapId())
                unitTarget->NearTeleportTo(st->target_X, st->target_Y, st->target_Z, st->target_Orientation, unitTarget == m_caster);
            else if (unitTarget->GetTypeId() == TYPEID_PLAYER)
                ((Player*)unitTarget)->TeleportTo(st->target_mapId, st->target_X, st->target_Y, st->target_Z, st->target_Orientation, unitTarget == m_caster ? TELE_TO_SPELL : 0);
            break;
        }
        case TARGET_LOCATION_CASTER_DEST:
        {
            // m_destN filled, but sometimes for wrong dest and does not have TARGET_FLAG_DEST_LOCATION

            float x = unitTarget->GetPositionX();
            float y = unitTarget->GetPositionY();
            float z = unitTarget->GetPositionZ();
            float orientation = m_caster->GetOrientation();

            m_caster->NearTeleportTo(x, y, z, orientation, unitTarget == m_caster);
            return;
        }
        default:
        {
            // If not exist data for dest location - return
            if (!(m_targets.m_targetMask & TARGET_FLAG_DEST_LOCATION))
            {
                sLog.outError("Spell::EffectTeleportUnits - unknown EffectImplicitTargetB[%u] = %u for spell ID %u", eff_idx, m_spellInfo->EffectImplicitTargetB[eff_idx], m_spellInfo->Id);
                return;
            }
            // Init dest coordinates
            float x = m_targets.m_destX;
            float y = m_targets.m_destY;
            float z = m_targets.m_destZ;
            float orientation = unitTarget->GetOrientation();
            // Teleport
            unitTarget->NearTeleportTo(x, y, z, orientation, unitTarget == m_caster);
            return;
        }
    }

    // post effects for TARGET_LOCATION_DATABASE
    switch (m_spellInfo->Id)
    {
        case 23441:                                 // Ultrasafe Transporter: Gadgetzan
        {
            // Wrong destination already rolled for, only handle minor malfunction on sucess
            m_caster->CastSpell(m_caster, 23450, TRIGGERED_OLD_TRIGGERED); // Transporter Arrival

            return;
        }
        case 23442:                                 // Dimensional Ripper - Everlook
        {
            // Roll for major malfunction (1/6); 23450 = success | 23449 = malfunction (being set afire)
            m_caster->CastSpell(m_caster, (urand(0, 5) ? 23450 : 23449), TRIGGERED_OLD_TRIGGERED);

            return;
        }
    }
}

void Spell::EffectApplyAura(SpellEffectIndex eff_idx)
{
    if (!unitTarget)
        return;

    // ghost spell check, allow apply any auras at player loading in ghost mode (will be cleanup after load)
    if ((!unitTarget->isAlive() && !(IsDeathOnlySpell(m_spellInfo) || IsDeathPersistentSpell(m_spellInfo))) &&
            (unitTarget->GetTypeId() != TYPEID_PLAYER || !((Player*)unitTarget)->GetSession()->PlayerLoading()))
        return;

    Unit* caster = GetAffectiveCaster();
    if (!caster)
    {
        // FIXME: currently we can't have auras applied explicitly by gameobjects
        // so for auras from wild gameobjects (no owner) target used
        if (m_originalCasterGUID.IsGameObject())
            caster = unitTarget;
        else
            return;
    }

    DEBUG_FILTER_LOG(LOG_FILTER_SPELL_CAST, "Spell: Aura is: %u", m_spellInfo->EffectApplyAuraName[eff_idx]);

    Aura* aur = CreateAura(m_spellInfo, eff_idx, &damage, &m_currentBasePoints[eff_idx], m_spellAuraHolder, unitTarget, caster, m_CastItem);
    m_spellAuraHolder->AddAura(aur, eff_idx);
}

void Spell::EffectPowerDrain(SpellEffectIndex eff_idx)
{
    if (m_spellInfo->EffectMiscValue[eff_idx] < 0 || m_spellInfo->EffectMiscValue[eff_idx] >= MAX_POWERS)
        return;

    Powers powerType = Powers(m_spellInfo->EffectMiscValue[eff_idx]);

    if (!unitTarget)
        return;
    if (!unitTarget->isAlive())
        return;
    if (unitTarget->GetPowerType() != powerType)
        return;
    if (damage < 0)
        return;

    uint32 curPower = unitTarget->GetPower(powerType);

    // add spell damage bonus
    damage = m_caster->SpellDamageBonusDone(unitTarget, m_spellInfo, uint32(damage), SPELL_DIRECT_DAMAGE);
    damage = unitTarget->SpellDamageBonusTaken(m_caster, m_spellInfo, uint32(damage), SPELL_DIRECT_DAMAGE);

    int32 new_damage;
    if (curPower < uint32(damage)) // damage should not be under zero at this point (checked above)
        new_damage = curPower;
    else
        new_damage = damage;

    unitTarget->ModifyPower(powerType, -new_damage);

    float gainMultiplier = 1.0f;

    // Do not gain power from self drain or when power types don't match
    if (m_caster->GetPowerType() == powerType && m_caster != unitTarget)
    {
        gainMultiplier = m_spellInfo->EffectMultipleValue[eff_idx];

        if (Player* modOwner = m_caster->GetSpellModOwner())
            modOwner->ApplySpellMod(m_spellInfo->Id, SPELLMOD_MULTIPLE_VALUE, gainMultiplier);
    }

    m_spellLog.AddLog(uint32(SPELL_EFFECT_POWER_DRAIN), unitTarget->GetObjectGuid(), new_damage, uint32(powerType), gainMultiplier);

    if (int32 gain = int32(new_damage * gainMultiplier))
        m_caster->EnergizeBySpell(m_caster, m_spellInfo->Id, gain, powerType);
}

void Spell::EffectSendEvent(SpellEffectIndex effectIndex)
{
    /*
    we do not handle a flag dropping or clicking on flag in battleground by sendevent system
    TODO: Actually, why not...
    */
    DEBUG_FILTER_LOG(LOG_FILTER_SPELL_CAST, "Spell ScriptStart %u for spellid %u in EffectSendEvent ", m_spellInfo->EffectMiscValue[effectIndex], m_spellInfo->Id);

    StartEvents_Event(m_caster->GetMap(), m_spellInfo->EffectMiscValue[effectIndex], m_caster, focusObject, true, m_caster);
}

void Spell::EffectPowerBurn(SpellEffectIndex eff_idx)
{
    if (m_spellInfo->EffectMiscValue[eff_idx] < 0 || m_spellInfo->EffectMiscValue[eff_idx] >= MAX_POWERS)
        return;

    Powers powertype = Powers(m_spellInfo->EffectMiscValue[eff_idx]);

    if (!unitTarget)
        return;
    if (!unitTarget->isAlive())
        return;
    if (unitTarget->GetPowerType() != powertype)
        return;
    if (damage < 0)
        return;

    int32 curPower = int32(unitTarget->GetPower(powertype));

    int32 new_damage = (curPower < damage) ? curPower : damage;

    unitTarget->ModifyPower(powertype, -new_damage);
    float multiplier = m_spellInfo->EffectMultipleValue[eff_idx];

    if (Player* modOwner = m_caster->GetSpellModOwner())
        modOwner->ApplySpellMod(m_spellInfo->Id, SPELLMOD_MULTIPLE_VALUE, multiplier);

    new_damage = int32(new_damage * multiplier);
    m_damage += new_damage;

    // should use here effect POWER_DRAIN because POWER_BURN is not implemented on client
    m_spellLog.AddLog(uint32(SPELL_EFFECT_POWER_DRAIN), unitTarget->GetObjectGuid(), new_damage, uint32(powertype), multiplier);
}

void Spell::EffectHeal(SpellEffectIndex eff_idx)
{
    if (unitTarget && unitTarget->isAlive() && damage >= 0)
    {
        // Try to get original caster
        Unit* caster = GetAffectiveCaster();
        if (!caster)
            return;

        int32 addhealth = damage;

        // Swiftmend - consumes Regrowth or Rejuvenation
        if (m_spellInfo->Id == 18562)
        {
            Unit::AuraList const& RejorRegr = unitTarget->GetAurasByType(SPELL_AURA_PERIODIC_HEAL);
            // find most short by duration
            Aura* targetAura = nullptr;
            for (Unit::AuraList::const_iterator i = RejorRegr.begin(); i != RejorRegr.end(); ++i)
            {
                if ((*i)->GetSpellProto()->SpellFamilyName == SPELLFAMILY_DRUID &&
                        // Regrowth or Rejuvenation 0x40 | 0x10
                        ((*i)->GetSpellProto()->SpellFamilyFlags & uint64(0x0000000000000050)))
                {
                    if (!targetAura || (*i)->GetAuraDuration() < targetAura->GetAuraDuration())
                        targetAura = *i;
                }
            }

            if (!targetAura)
            {
                sLog.outError("Target (GUID: %u TypeId: %u) has aurastate AURA_STATE_SWIFTMEND but no matching aura.", unitTarget->GetGUIDLow(), unitTarget->GetTypeId());
                return;
            }
            int idx = 0;
            while (idx < 3)
            {
                if (targetAura->GetSpellProto()->EffectApplyAuraName[idx] == SPELL_AURA_PERIODIC_HEAL)
                    break;
                idx++;
            }

            int32 tickheal = targetAura->GetModifier()->m_amount;
            int32 tickcount = GetSpellDuration(targetAura->GetSpellProto()) / targetAura->GetSpellProto()->EffectAmplitude[idx];

            unitTarget->RemoveAurasByCasterSpell(targetAura->GetId(), targetAura->GetCasterGuid());

            addhealth += tickheal * tickcount;
        }

        addhealth = caster->SpellHealingBonusDone(unitTarget, m_spellInfo, addhealth, HEAL);
        addhealth = unitTarget->SpellHealingBonusTaken(caster, m_spellInfo, addhealth, HEAL);

        m_healingPerEffect[eff_idx] = addhealth;
    }
}

void Spell::EffectHealMechanical(SpellEffectIndex /*eff_idx*/)
{
    // Mechanic creature type should be correctly checked by targetCreatureType field
    if (unitTarget && unitTarget->isAlive() && damage >= 0)
    {
        // Try to get original caster
        Unit* caster = GetAffectiveCaster();
        if (!caster)
            return;

        uint32 addhealth = caster->SpellHealingBonusDone(unitTarget, m_spellInfo, damage, HEAL);
        addhealth = unitTarget->SpellHealingBonusTaken(caster, m_spellInfo, addhealth, HEAL);

        caster->DealHeal(unitTarget, addhealth, m_spellInfo);
    }
}

void Spell::EffectHealthLeech(SpellEffectIndex eff_idx)
{
    if (!unitTarget)
        return;
    if (!unitTarget->isAlive())
        return;

    if (damage < 0)
        return;

    DEBUG_FILTER_LOG(LOG_FILTER_SPELL_CAST, "HealthLeech :%i", damage);

    uint32 curHealth = unitTarget->GetHealth();
    damage = m_caster->SpellNonMeleeDamageLog(unitTarget, m_spellInfo->Id, damage);
    if ((int32)curHealth < damage)
        damage = curHealth;

    float multiplier = m_spellInfo->EffectMultipleValue[eff_idx];

    if (Player* modOwner = m_caster->GetSpellModOwner())
        modOwner->ApplySpellMod(m_spellInfo->Id, SPELLMOD_MULTIPLE_VALUE, multiplier);

    uint32 heal = uint32(damage * multiplier);
    if (m_caster->isAlive())
    {
        heal = m_caster->SpellHealingBonusTaken(m_caster, m_spellInfo, heal, HEAL);

        m_caster->DealHeal(m_caster, heal, m_spellInfo);
    }
}

bool Spell::DoCreateItem(SpellEffectIndex /*eff_idx*/, uint32 itemtype)
{
    if (!unitTarget || unitTarget->GetTypeId() != TYPEID_PLAYER)
        return false;

    Player* player = (Player*)unitTarget;

    uint32 newitemid = itemtype;
    ItemPrototype const* pProto = ObjectMgr::GetItemPrototype(newitemid);
    if (!pProto)
    {
        player->SendEquipError(EQUIP_ERR_ITEM_NOT_FOUND, nullptr, nullptr);
        return false;
    }

    // bg reward have some special in code work
    uint32 bgType = 0;
    switch (m_spellInfo->Id)
    {
        case SPELL_AV_MARK_WINNER:
        case SPELL_AV_MARK_LOSER:
            bgType = BATTLEGROUND_AV;
            break;
        case SPELL_WS_MARK_WINNER:
        case SPELL_WS_MARK_LOSER:
            bgType = BATTLEGROUND_WS;
            break;
        case SPELL_AB_MARK_WINNER:
        case SPELL_AB_MARK_LOSER:
            bgType = BATTLEGROUND_AB;
            break;
        default:
            break;
    }

    uint32 num_to_add = damage;

    if (num_to_add < 1)
        num_to_add = 1;
    if (num_to_add > pProto->Stackable)
        num_to_add = pProto->Stackable;

    // init items_count to 1, since 1 item will be created regardless of specialization
    int items_count = 1;

    // really will be created more items
    num_to_add *= items_count;

    // can the player store the new item?
    ItemPosCountVec dest;
    uint32 no_space = 0;
    InventoryResult msg = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, newitemid, num_to_add, &no_space);
    if (msg != EQUIP_ERR_OK)
    {
        // convert to possible store amount
        if (msg == EQUIP_ERR_INVENTORY_FULL || msg == EQUIP_ERR_CANT_CARRY_MORE_OF_THIS)
            num_to_add -= no_space;

        // for battleground marks send by mail if not add all expected
        if (no_space > 0 && bgType)
        {
            if (BattleGround * bg = sBattleGroundMgr.GetBattleGroundTemplate(BattleGroundTypeId(bgType)))
            {
                bg->SendRewardMarkByMail(player, newitemid, no_space);
                return true;
            }
        }

        // if not created by another reason from full inventory or unique items amount limitation
        player->SendEquipError(msg, nullptr, nullptr, newitemid);
        return false;
    }

    if (num_to_add)
    {
        // create the new item and store it
        Item* pItem = player->StoreNewItem(dest, newitemid, true, Item::GenerateItemRandomPropertyId(newitemid));

        // was it successful? return error if not
        if (!pItem)
        {
            player->SendEquipError(EQUIP_ERR_ITEM_NOT_FOUND, nullptr, nullptr);
            return false;
        }

        // set the "Crafted by ..." property of the item
        if (pItem->GetProto()->Class != ITEM_CLASS_CONSUMABLE && pItem->GetProto()->Class != ITEM_CLASS_QUEST)
            pItem->SetGuidValue(ITEM_FIELD_CREATOR, player->GetObjectGuid());

        // send info to the client
        player->SendNewItem(pItem, num_to_add, true, !bgType);

        // we succeeded in creating at least one item, so a levelup is possible
        if (!bgType)
            player->UpdateCraftSkill(m_spellInfo->Id);
    }

    return true;
}

void Spell::EffectCreateItem(SpellEffectIndex eff_idx)
{
    if (DoCreateItem(eff_idx, m_spellInfo->EffectItemType[eff_idx]))
        m_spellLog.AddLog(uint32(SPELL_EFFECT_CREATE_ITEM), m_spellInfo->EffectItemType[eff_idx]);
}

void Spell::EffectPersistentAA(SpellEffectIndex eff_idx)
{
    Unit* pCaster = GetAffectiveCaster();
    // FIXME: in case wild GO will used wrong affective caster (target in fact) as dynobject owner
    if (!pCaster)
        pCaster = m_caster;

    SpellTarget target = SpellTarget(m_spellInfo->EffectImplicitTargetB[eff_idx] ? m_spellInfo->EffectImplicitTargetB[eff_idx] : m_spellInfo->EffectImplicitTargetA[eff_idx]);

    float radius = GetSpellRadius(sSpellRadiusStore.LookupEntry(m_spellInfo->EffectRadiusIndex[eff_idx]));

    if (Player* modOwner = pCaster->GetSpellModOwner())
        modOwner->ApplySpellMod(m_spellInfo->Id, SPELLMOD_RADIUS, radius);

    DynamicObject* dynObj = new DynamicObject;
    if (!dynObj->Create(pCaster->GetMap()->GenerateLocalLowGuid(HIGHGUID_DYNAMICOBJECT), pCaster, m_spellInfo->Id,
                        eff_idx, m_targets.m_destX, m_targets.m_destY, m_targets.m_destZ, m_duration, radius, DYNAMIC_OBJECT_AREA_SPELL, target, damage, m_currentBasePoints[eff_idx]))
    {
        delete dynObj;
        return;
    }

    pCaster->AddDynObject(dynObj);
    pCaster->GetMap()->Add(dynObj);

    // Potential Hack - at the time of channel start Dynamic Object is not created yet, so have to do it here
    // Sent in next Object Update so for client its the same
    if (eff_idx == EFFECT_INDEX_0 && IsChanneledSpell(m_spellInfo))
    {
        m_caster->SetChannelObject(dynObj);
        m_caster->SendForcedObjectUpdate();
    }
}

void Spell::EffectEnergize(SpellEffectIndex eff_idx)
{
    if (!unitTarget)
        return;
    if (!unitTarget->isAlive())
        return;

    if (m_spellInfo->EffectMiscValue[eff_idx] < 0 || m_spellInfo->EffectMiscValue[eff_idx] >= MAX_POWERS)
        return;

    Powers power = Powers(m_spellInfo->EffectMiscValue[eff_idx]);

    // Some level depends spells
    int level_multiplier = 0;
    int level_diff = 0;
    switch (m_spellInfo->Id)
    {
        case 5530:
            if (m_caster->getClass() == CLASS_ROGUE) // Warrior and rogue use same spell, on rogue not supposed to give resource, WTF blizzard
                return;
            break;
        case 9512:                                          // Restore Energy
            level_diff = m_caster->getLevel() - 40;
            level_multiplier = 2;
            break;
        case 24571:                                         // Blood Fury
            level_diff = m_caster->getLevel() - 60;
            level_multiplier = 10;
            break;
        case 24532:                                         // Burst of Energy
            level_diff = m_caster->getLevel() - 60;
            level_multiplier = 4;
            break;
        default:
            break;
    }

    if (level_diff > 0)
        damage -= level_multiplier * level_diff;

    if (damage < 0)
        return;

    if (unitTarget->GetMaxPower(power) == 0)
        return;

    m_caster->EnergizeBySpell(unitTarget, m_spellInfo->Id, damage, power);
}

void Spell::SendLoot(ObjectGuid guid, LootType loottype, LockType lockType)
{
    switch (guid.GetHigh())
    {
        case HIGHGUID_GAMEOBJECT:
        {
            if (gameObjTarget)
            {
                switch (gameObjTarget->GetGoType())
                {
                    case GAMEOBJECT_TYPE_DOOR:
                    case GAMEOBJECT_TYPE_BUTTON:
                    case GAMEOBJECT_TYPE_QUESTGIVER:
                    case GAMEOBJECT_TYPE_SPELL_FOCUS:
                    case GAMEOBJECT_TYPE_GOOBER:
                        gameObjTarget->Use(m_caster);
                        return;

                    case GAMEOBJECT_TYPE_CHEST:
                        gameObjTarget->Use(m_caster);
                        // Don't return, let loots been taken
                        break;

                    case GAMEOBJECT_TYPE_TRAP:
                        switch (lockType)
                        {
                            case LOCKTYPE_NONE:
                            case LOCKTYPE_DISARM_TRAP:
                            case LOCKTYPE_OPEN_ATTACKING:
                                gameObjTarget->SetLootState(GO_ACTIVATED);
                                return;
                            default:
                                sLog.outError("Spell::SendLoot unhandled locktype %u for GameObject trap (entry %u) for spell %u.", lockType, gameObjTarget->GetEntry(), m_spellInfo->Id);
                                return;
                        }
                    default:
                        sLog.outError("Spell::SendLoot unhandled GameObject type %u (entry %u) for spell %u.", gameObjTarget->GetGoType(), gameObjTarget->GetEntry(), m_spellInfo->Id);
                        return;
                }

                if (m_caster->GetTypeId() != TYPEID_PLAYER)
                    return;

                Loot*& loot = gameObjTarget->loot;
                if (!loot)
                {
                    loot = new Loot((Player*)m_caster, gameObjTarget, loottype);
                    TakeCastItem();
                }
                loot->ShowContentTo((Player*)m_caster);
                return;
            }
            break;
        }

        case HIGHGUID_ITEM:
        {
            if (itemTarget)
            {
                Loot*& loot = itemTarget->loot;
                if (!loot)
                {
                    loot = new Loot((Player*)m_caster, itemTarget, loottype);
                    TakeCastItem();
                }
                loot->ShowContentTo((Player*)m_caster);
                return;
            }

            break;
        }
        default:
            sLog.outError("Spell::SendLoot unhandled Object type %s for spell %u.", guid.GetString().c_str(), m_spellInfo->Id);
            break;
    }
}

void Spell::EffectOpenLock(SpellEffectIndex eff_idx)
{
    if (!m_caster || m_caster->GetTypeId() != TYPEID_PLAYER)
    {
        DEBUG_LOG("WORLD: Open Lock - No Player Caster!");
        return;
    }

    Player* player = (Player*)m_caster;

    uint32 lockId = 0;

    // Get lockId
    if (gameObjTarget)
    {
        GameObjectInfo const* goInfo = gameObjTarget->GetGOInfo();
        // Arathi Basin banner opening !
        if ((goInfo->type == GAMEOBJECT_TYPE_BUTTON && goInfo->button.noDamageImmune) ||
                (goInfo->type == GAMEOBJECT_TYPE_GOOBER && goInfo->goober.losOK))
        {
            // CanUseBattleGroundObject() already called in CheckCast()
            // in battleground check
            if (BattleGround* bg = player->GetBattleGround())
            {
                // check if it's correct bg
                if (bg->GetTypeID() == BATTLEGROUND_AB || bg->GetTypeID() == BATTLEGROUND_AV)
                    bg->EventPlayerClickedOnFlag(player, gameObjTarget);
                return;
            }
        }
        else if (goInfo->type == GAMEOBJECT_TYPE_FLAGSTAND)
        {
            // CanUseBattleGroundObject() already called in CheckCast()
            // in battleground check
            if (BattleGround* bg = player->GetBattleGround())
                return;
        }
        lockId = goInfo->GetLockId();
    }
    else if (itemTarget)
    {
        lockId = itemTarget->GetProto()->LockID;
    }
    else
    {
        DEBUG_LOG("WORLD: Open Lock - No GameObject/Item Target!");
        return;
    }

    SkillType skillId = SKILL_NONE;
    int32 reqSkillValue = 0;
    int32 skillValue;

    SpellCastResult res = CanOpenLock(eff_idx, lockId, skillId, reqSkillValue, skillValue);
    if (res != SPELL_CAST_OK)
    {
        SendCastResult(res);
        return;
    }

    // not allow use skill grow at item base open
    if (!m_CastItem && skillId != SKILL_NONE)
    {
        // update skill if really known
        if (uint32 pureSkillValue = player->GetSkillValuePure(skillId))
        {
            if (gameObjTarget && !gameObjTarget->loot)
            {
                // Allow one skill-up until respawned
                if (!gameObjTarget->IsInSkillupList(player) &&
                        player->UpdateGatherSkill(skillId, pureSkillValue, reqSkillValue))
                    gameObjTarget->AddToSkillupList(player);
            }
            else if (itemTarget && !itemTarget->loot)
            {
                // Do one skill-up
                player->UpdateGatherSkill(skillId, pureSkillValue, reqSkillValue);
            }
        }
    }

    // mark item as unlocked
    if (itemTarget)
    {
        itemTarget->SetFlag(ITEM_FIELD_FLAGS, ITEM_DYNFLAG_UNLOCKED);

        // only send loot if owner is player, else client sends release anyway
        if (itemTarget->GetOwnerGuid() == m_caster->GetObjectGuid())
        {
            SendLoot(itemTarget->GetObjectGuid(), LOOT_SKINNING, LockType(m_spellInfo->EffectMiscValue[eff_idx]));
            m_spellLog.AddLog(uint32(SPELL_EFFECT_OPEN_LOCK), itemTarget->GetObjectGuid());
        }
    }
    else
    {
        SendLoot(gameObjTarget->GetObjectGuid(), LOOT_SKINNING, LockType(m_spellInfo->EffectMiscValue[eff_idx]));
        m_spellLog.AddLog(uint32(SPELL_EFFECT_OPEN_LOCK), gameObjTarget->GetObjectGuid());
    }
}

void Spell::EffectSummonChangeItem(SpellEffectIndex eff_idx)
{
    if (m_caster->GetTypeId() != TYPEID_PLAYER)
        return;

    Player* player = (Player*)m_caster;

    // applied only to using item
    if (!m_CastItem)
        return;

    // ... only to item in own inventory/bank/equip_slot
    if (m_CastItem->GetOwnerGuid() != player->GetObjectGuid())
        return;

    uint32 newitemid = m_spellInfo->EffectItemType[eff_idx];
    if (!newitemid)
        return;

    Item* oldItem = m_CastItem;

    // prevent crash at access and unexpected charges counting with item update queue corrupt
    ClearCastItem();

    uint32 curItemCount = player->GetItemCount(newitemid, true);

    if (Item* newItem = player->ConvertItem(oldItem, newitemid))
    {
        player->ItemAddedQuestCheck(newItem->GetEntry(), newItem->GetCount());

        // Push packet to client so it knows we've created an item and quest can show objective update
        player->SendNewItem(newItem, newItem->GetCount() - curItemCount, true, false);
    }
}

void Spell::EffectProficiency(SpellEffectIndex /*eff_idx*/)
{
    if (m_caster->GetTypeId() != TYPEID_PLAYER)
        return;
    Player* player = static_cast<Player*>(m_caster);

    uint32 subClassMask = m_spellInfo->EquippedItemSubClassMask;
    if (m_spellInfo->EquippedItemClass == ITEM_CLASS_WEAPON && !(player->GetWeaponProficiency() & subClassMask))
    {
        player->AddWeaponProficiency(subClassMask);
        player->SendProficiency(ITEM_CLASS_WEAPON, player->GetWeaponProficiency());
    }
    if (m_spellInfo->EquippedItemClass == ITEM_CLASS_ARMOR && !(player->GetArmorProficiency() & subClassMask))
    {
        player->AddArmorProficiency(subClassMask);
        player->SendProficiency(ITEM_CLASS_ARMOR, player->GetArmorProficiency());
    }
}

void Spell::EffectApplyAreaAura(SpellEffectIndex eff_idx)
{
    if (!unitTarget)
        return;
    if (!unitTarget->isAlive())
        return;

    AreaAura* Aur = new AreaAura(m_spellInfo, eff_idx, &damage, &m_currentBasePoints[eff_idx], m_spellAuraHolder, unitTarget, m_caster, m_CastItem);
    m_spellAuraHolder->AddAura(Aur, eff_idx);
}

void Spell::EffectSummon(SpellEffectIndex eff_idx)
{
    if (m_caster->GetPetGuid())
        return;

    uint32 pet_entry = m_spellInfo->EffectMiscValue[eff_idx];
    if (!pet_entry)
        return;

    CreatureInfo const* cInfo = ObjectMgr::GetCreatureTemplate(pet_entry);
    if (!cInfo)
    {
        sLog.outErrorDb("Spell::DoSummon: creature entry %u not found for spell %u.", pet_entry, m_spellInfo->Id);
        return;
    }

    Pet* spawnCreature = new Pet(GUARDIAN_PET);

    // Summon in dest location
    CreatureCreatePos pos(m_caster->GetMap(), m_targets.m_destX, m_targets.m_destY, m_targets.m_destZ, -m_caster->GetOrientation());

    if (!(m_targets.m_targetMask & TARGET_FLAG_DEST_LOCATION))
    {
        float px, py, pz;
        m_caster->GetClosePoint(px, py, pz, 2.0f);
        pos = CreatureCreatePos(m_caster->GetMap(), px, py, pz, -m_caster->GetOrientation());
    }

    Map* map = m_caster->GetMap();
    uint32 pet_number = sObjectMgr.GeneratePetNumber();
    if (!spawnCreature->Create(map->GenerateLocalLowGuid(HIGHGUID_PET), pos, cInfo, pet_number))
    {
        sLog.outErrorDb("Spell::EffectSummon: can't create creature with entry %u for spell %u", cInfo->Entry, m_spellInfo->Id);
        delete spawnCreature;
        return;
    }

    // Level of pet summoned
    uint32 level = m_caster->getLevel();
    if (m_caster->GetTypeId() != TYPEID_PLAYER)
    {
        // pet players do not need this
        // TODO :: Totem, Pet and Critter may not use this. This is probably wrongly used and need more research.
        uint32 resultLevel = level + std::max(m_spellInfo->EffectMultipleValue[eff_idx], 1.0f);

        // result level should be a possible level for creatures
        if (resultLevel > 0 && resultLevel <= DEFAULT_MAX_CREATURE_LEVEL)
            level = resultLevel;
    }

    spawnCreature->SetRespawnCoord(pos);

    // set timer for unsummon
    if (m_duration > 0)
        spawnCreature->SetDuration(m_duration);

    spawnCreature->SetOwnerGuid(m_caster->GetObjectGuid());
    spawnCreature->SetUInt32Value(UNIT_FIELD_FLAGS, cInfo->UnitFlags);
    spawnCreature->SetUInt32Value(UNIT_NPC_FLAGS, cInfo->NpcFlags);
    spawnCreature->setFaction(m_caster->getFaction());
    spawnCreature->SetUInt32Value(UNIT_FIELD_PET_NAME_TIMESTAMP, 0);
    spawnCreature->SetUInt32Value(UNIT_CREATED_BY_SPELL, m_spellInfo->Id);

    spawnCreature->InitStatsForLevel(level);

    if (CharmInfo* charmInfo = spawnCreature->GetCharmInfo())
        charmInfo->SetPetNumber(pet_number, (m_caster->GetTypeId() == TYPEID_PLAYER && spawnCreature->isControlled()));

    // spawnCreature->SetName("");                          // generated by client

    if (spawnCreature->getPetType() == GUARDIAN_PET)
        m_caster->AddGuardian(spawnCreature);
    else
        m_caster->SetPet(spawnCreature);

    if (m_caster->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED))
        spawnCreature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED);

    if (m_caster->IsImmuneToNPC())
        spawnCreature->SetImmuneToNPC(true);

    if (m_caster->IsImmuneToPlayer())
        spawnCreature->SetImmuneToPlayer(true);

    map->Add((Creature*)spawnCreature);
    spawnCreature->AIM_Initialize();
    spawnCreature->AI()->SetReactState(REACT_DEFENSIVE);
    spawnCreature->InitPetCreateSpells();

    // Notify Summoner
    if (m_originalCaster && (m_originalCaster != m_caster) && (m_originalCaster->AI()))
        m_originalCaster->AI()->JustSummoned(spawnCreature);
    else if (m_caster->AI())
        m_caster->AI()->JustSummoned(spawnCreature);
}

void Spell::EffectLearnSpell(SpellEffectIndex eff_idx)
{
    if (!unitTarget)
        return;

    if (unitTarget->GetTypeId() != TYPEID_PLAYER)
    {
        if (m_caster->GetTypeId() == TYPEID_PLAYER)
            EffectLearnPetSpell(eff_idx);

        return;
    }

    Player* player = (Player*)unitTarget;

    uint32 spellToLearn = m_spellInfo->EffectTriggerSpell[eff_idx];
    player->learnSpell(spellToLearn, false);

    if (WorldObject const* caster = GetCastingObject())
        DEBUG_LOG("Spell: %s has learned spell %u from %s", player->GetGuidStr().c_str(), spellToLearn, caster->GetGuidStr().c_str());
}

void Spell::EffectDispel(SpellEffectIndex eff_idx)
{
    if (!unitTarget)
        return;

    // Shield Slam 50% chance dispel
    if (m_spellInfo->SpellFamilyName == SPELLFAMILY_WARRIOR && (m_spellInfo->SpellFamilyFlags & uint64(0x0000000100000000)) &&
            !roll_chance_i(50))
        return;

    // Fill possible dispel list
    std::list <std::pair<SpellAuraHolder*, uint32> > dispel_list;

    // Create dispel mask by dispel type
    uint32 dispelMask  = GetDispellMask(DispelType(m_spellInfo->EffectMiscValue[eff_idx]));
    Unit::SpellAuraHolderMap const& auras = unitTarget->GetSpellAuraHolderMap();
    for (Unit::SpellAuraHolderMap::const_iterator itr = auras.begin(); itr != auras.end(); ++itr)
    {
        SpellAuraHolder* holder = itr->second;
        if ((1 << holder->GetSpellProto()->Dispel) & dispelMask)
        {
            if (holder->GetSpellProto()->Dispel == DISPEL_MAGIC)
            {
                // do not remove positive auras if friendly target
                //               negative auras if non-friendly target
                if (holder->IsPositive() == m_caster->CanAssistSpell(unitTarget, m_spellInfo))
                    continue;
            }
            dispel_list.push_back(std::pair<SpellAuraHolder*, uint32>(holder, holder->GetStackAmount()));
        }
    }
    // Ok if exist some buffs for dispel try dispel it
    if (!dispel_list.empty())
    {
        std::list<std::pair<SpellAuraHolder*, uint32> > success_list;  // (spell_id,casterGuid)
        std::list < uint32 > fail_list;                     // spell_id

        // some spells have effect value = 0 and all from its by meaning expect 1
        if (!damage)
            damage = 1;

        // Dispel N = damage buffs (or while exist buffs for dispel)
        for (int32 count = 0; count < damage && !dispel_list.empty(); ++count)
        {
            // Random select buff for dispel
            std::list<std::pair<SpellAuraHolder*, uint32> >::iterator dispel_itr = dispel_list.begin();
            std::advance(dispel_itr, urand(0, dispel_list.size() - 1));

            SpellAuraHolder* holder = dispel_itr->first;

            dispel_itr->second -= 1;

            // remove entry from dispel_list if nothing left in stack
            if (dispel_itr->second == 0)
                dispel_list.erase(dispel_itr);

            SpellEntry const* spellInfo = holder->GetSpellProto();
            // Base dispel chance
            // TODO: possible chance depend from spell level??
            int32 miss_chance = 0;
            // Apply dispel mod from aura caster
            if (Unit* caster = holder->GetCaster())
            {
                if (Player* modOwner = caster->GetSpellModOwner())
                    modOwner->ApplySpellMod(spellInfo->Id, SPELLMOD_RESIST_DISPEL_CHANCE, miss_chance, this);
            }
            // Try dispel
            if (roll_chance_i(miss_chance))
                fail_list.push_back(spellInfo->Id);
            else
            {
                bool foundDispelled = false;
                for (std::list<std::pair<SpellAuraHolder*, uint32> >::iterator success_iter = success_list.begin(); success_iter != success_list.end(); ++success_iter)
                {
                    if (success_iter->first->GetId() == holder->GetId() && success_iter->first->GetCasterGuid() == holder->GetCasterGuid())
                    {
                        success_iter->second += 1;
                        foundDispelled = true;
                        break;
                    }
                }
                if (!foundDispelled)
                    success_list.push_back(std::pair<SpellAuraHolder*, uint32>(holder, 1));
            }
        }
        // Send success log and really remove auras
        if (!success_list.empty())
        {
            int32 count = success_list.size();
            WorldPacket data(SMSG_SPELLDISPELLOG, 8 + 8 + 4 + count * 4);
            data << unitTarget->GetPackGUID();              // Victim GUID
            data << m_caster->GetPackGUID();                // Caster GUID
            data << uint32(count);                          // count
            for (std::list<std::pair<SpellAuraHolder*, uint32> >::iterator j = success_list.begin(); j != success_list.end(); ++j)
            {
                SpellAuraHolder* dispelledHolder = j->first;
                data << uint32(dispelledHolder->GetId());   // Spell Id
                unitTarget->RemoveAuraHolderDueToSpellByDispel(dispelledHolder->GetId(), j->second, dispelledHolder->GetCasterGuid(), m_caster);
            }
            m_caster->SendMessageToSet(data, true);
            m_spellLog.AddLog(uint32(SPELL_EFFECT_DISPEL), unitTarget->GetObjectGuid());

            // On success dispel
            // Devour Magic
            if (m_spellInfo->SpellFamilyName == SPELLFAMILY_WARLOCK && m_spellInfo->Category == SPELLCATEGORY_DEVOUR_MAGIC)
            {
                uint32 heal_spell = 0;
                switch (m_spellInfo->Id)
                {
                    case 19505: heal_spell = 19658; break;
                    case 19731: heal_spell = 19732; break;
                    case 19734: heal_spell = 19733; break;
                    case 19736: heal_spell = 19735; break;
                    default:
                        DEBUG_LOG("Spell for Devour Magic %d not handled in Spell::EffectDispel", m_spellInfo->Id);
                        break;
                }
                if (heal_spell)
                    m_caster->CastSpell(m_caster, heal_spell, TRIGGERED_OLD_TRIGGERED);
            }
        }
        // Send fail log to client
        if (!fail_list.empty())
        {
            // Failed to dispel
            WorldPacket data(SMSG_DISPEL_FAILED, 8 + 8 + 4 * fail_list.size());
            data << m_caster->GetObjectGuid();              // Caster GUID
            data << unitTarget->GetObjectGuid();            // Victim GUID
            for (std::list< uint32 >::iterator j = fail_list.begin(); j != fail_list.end(); ++j)
                data << uint32(*j);                         // Spell Id
            m_caster->SendMessageToSet(data, true);
        }
    }
}

void Spell::EffectDualWield(SpellEffectIndex /*eff_idx*/)
{
    if (unitTarget)
        unitTarget->SetCanDualWield(true);
}

void Spell::EffectPull(SpellEffectIndex /*eff_idx*/)
{
    // TODO: create a proper pull towards distract spell center for distract
    DEBUG_LOG("WORLD: Spell Effect DUMMY");
}

void Spell::EffectDistract(SpellEffectIndex /*eff_idx*/)
{
    // Check for possible target
    if (!unitTarget || unitTarget->isInCombat())
        return;

    // target must be OK to do this
    if (unitTarget->hasUnitState(UNIT_STAT_CAN_NOT_REACT))
        return;

    unitTarget->clearUnitState(UNIT_STAT_MOVING);

    if (unitTarget->GetTypeId() == TYPEID_UNIT)
        unitTarget->GetMotionMaster()->MoveDistract(damage * IN_MILLISECONDS);

    float orientation = unitTarget->GetAngle(m_targets.m_destX, m_targets.m_destY);
    unitTarget->SetFacingTo(orientation);
    // This is needed to change the facing server side as well (and it must be after the MoveDistract call)
    unitTarget->SetOrientation(orientation);
    m_spellLog.AddLog(uint32(SPELL_EFFECT_DISTRACT), unitTarget->GetObjectGuid());
}

void Spell::EffectPickPocket(SpellEffectIndex /*eff_idx*/)
{
    if (m_caster->GetTypeId() != TYPEID_PLAYER)
        return;

    // victim must be creature and attackable
    if (!unitTarget || unitTarget->GetTypeId() != TYPEID_UNIT || !m_caster->CanAttackNow(unitTarget))
        return;

    Creature* creatureTarget = static_cast<Creature*>(unitTarget);
    Player* playerCaster = static_cast<Player*>(m_caster);

    // victim have to be alive and humanoid or undead
    if (unitTarget->isAlive() && (unitTarget->GetCreatureTypeMask() & CREATURE_TYPEMASK_HUMANOID_OR_UNDEAD) != 0)
    {
        int32 chance = 10 + int32(m_caster->getLevel()) - int32(unitTarget->getLevel());

        if (chance > irand(0, 19))
        {
            // Stealing successful
            // DEBUG_LOG("Sending loot from pickpocket");
            Loot*& loot = unitTarget->loot;
            if (!loot)
                loot = new Loot(playerCaster, creatureTarget, LOOT_PICKPOCKETING);
            else
            {
                if (loot->GetLootType() == LOOT_PICKPOCKETING)
                {
                    auto msec = (World::GetCurrentClockTime() - loot->GetCreateTime()).count();

                    if (msec > creatureTarget->GetRespawnDelay() * 1000)
                    {
                        // refill pickpocket
                        delete loot;
                        loot = new Loot(playerCaster, creatureTarget, LOOT_PICKPOCKETING);
                    }
                }
                else
                {
                    delete loot;
                    loot = new Loot(playerCaster, creatureTarget, LOOT_PICKPOCKETING);
                }
            }
            loot->ShowContentTo(playerCaster);
        }
        else
        {
            // Reveal action + get attack
            m_caster->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_TALK);
            unitTarget->AttackedBy(m_caster);
        }
    }
}

void Spell::EffectAddFarsight(SpellEffectIndex eff_idx)
{
    if (m_caster->GetTypeId() != TYPEID_PLAYER)
        return;

    DynamicObject* dynObj = new DynamicObject;

    // set radius to 0: spell not expected to work as persistent aura
    if (!dynObj->Create(m_caster->GetMap()->GenerateLocalLowGuid(HIGHGUID_DYNAMICOBJECT), m_caster,
                        m_spellInfo->Id, eff_idx, m_targets.m_destX, m_targets.m_destY, m_targets.m_destZ, m_duration, 0, DYNAMIC_OBJECT_FARSIGHT_FOCUS, TARGET_NONE, damage, m_currentBasePoints[eff_idx]))
    {
        delete dynObj;
        return;
    }

    m_caster->AddDynObject(dynObj);
    m_caster->GetMap()->Add(dynObj);

    ((Player*)m_caster)->GetCamera().SetView(dynObj);
}

void Spell::EffectSummonWild(SpellEffectIndex eff_idx)
{
    uint32 creature_entry = m_spellInfo->EffectMiscValue[eff_idx];
    if (!creature_entry)
        return;

    // Level of pet summoned
    uint32 level = m_caster->getLevel();
    if (m_caster->GetTypeId() != TYPEID_PLAYER)
    {
        // pet players do not need this
        // TODO :: Totem, Pet and Critter may not use this. This is probably wrongly used and need more research.
        uint32 resultLevel = level + std::max(m_spellInfo->EffectMultipleValue[eff_idx], 1.0f);

        // result level should be a possible level for creatures
        if (resultLevel > 0 && resultLevel <= DEFAULT_MAX_CREATURE_LEVEL)
            level = resultLevel;
    }

    CreatureInfo const* cInfo = ObjectMgr::GetCreatureTemplate(creature_entry);

    if (!cInfo)
        return;

    if (m_caster->getLevel() >= cInfo->MaxLevel)
        level = cInfo->MaxLevel;

    else if (m_caster->getLevel() <= cInfo->MinLevel)
        level = cInfo->MinLevel;

    // select center of summon position
    float center_x = m_targets.m_destX;
    float center_y = m_targets.m_destY;
    float center_z = m_targets.m_destZ;

    float radius = GetSpellRadius(sSpellRadiusStore.LookupEntry(m_spellInfo->EffectRadiusIndex[eff_idx]));
    TempSpawnType summonType = (m_duration == 0) ? TEMPSPAWN_DEAD_DESPAWN : TEMPSPAWN_TIMED_OR_DEAD_DESPAWN;

    int32 amount = damage > 0 ? damage : 1;

    for (int32 count = 0; count < amount; ++count)
    {
        float px, py, pz;
        // If dest location if present
        if (m_targets.m_targetMask & TARGET_FLAG_DEST_LOCATION)
        {
            // Summon 1 unit in dest location
            if (count == 0)
            {
                px = m_targets.m_destX;
                py = m_targets.m_destY;
                pz = m_targets.m_destZ;
            }
            // Summon in random point all other units if location present
            else
                m_caster->GetRandomPoint(center_x, center_y, center_z, radius, px, py, pz);
        }
        // Summon if dest location not present near caster
        else
        {
            if (radius > 0.0f)
            {
                // not using bounding radius of caster here
                m_caster->GetClosePoint(px, py, pz, 0.0f, radius);
            }
            else
            {
                // EffectRadiusIndex 0 or 36
                px = m_caster->GetPositionX();
                py = m_caster->GetPositionY();
                pz = m_caster->GetPositionZ();
            }
        }

        if (Creature* summon = m_caster->SummonCreature(creature_entry, px, py, pz, m_caster->GetOrientation(), summonType, m_duration, false, IsSpellSetRun(m_spellInfo)))
        {
            summon->SetUInt32Value(UNIT_CREATED_BY_SPELL, m_spellInfo->Id);

            // UNIT_FIELD_CREATEDBY are not set for these kind of spells.
            // Does exceptions exist? If so, what are they?
            // summon->SetCreatorGuid(m_caster->GetObjectGuid());

            // Notify original caster if not done already
            if (m_originalCaster && (m_originalCaster != m_caster) && (m_originalCaster->AI()))
                m_originalCaster->AI()->JustSummoned(summon);
            else if (m_caster->AI())
                m_caster->AI()->JustSummoned(summon);
        }
    }
}

void Spell::EffectSummonGuardian(SpellEffectIndex eff_idx)
{
    uint32 pet_entry = m_spellInfo->EffectMiscValue[eff_idx];
    if (!pet_entry)
        return;

    CreatureInfo const* cInfo = ObjectMgr::GetCreatureTemplate(pet_entry);
    if (!cInfo)
    {
        sLog.outErrorDb("Spell::DoSummonGuardian: creature entry %u not found for spell %u.", pet_entry, m_spellInfo->Id);
        return;
    }

    // set timer for unsummon
    int32 duration = CalculateSpellDuration(m_spellInfo, m_caster);

    // second direct cast unsummon guardian(s) (guardians without like functionality have cooldown > spawn time)
    if (!m_IsTriggeredSpell && m_caster->GetTypeId() == TYPEID_PLAYER)
    {
        bool found = false;
        // including protector
        while (Pet* old_summon = m_caster->FindGuardianWithEntry(pet_entry))
        {
            old_summon->Unsummon(PET_SAVE_AS_DELETED, m_caster);
            found = true;
        }

        if (found && !(m_spellInfo->DurationIndex && m_spellInfo->Category))
            return;
    }

    // Get casting object
    WorldObject* realCaster = GetCastingObject();
    if (!realCaster)
    {
        sLog.outError("EffectSummonGuardian: No Casting Object found for spell %u, (caster = %s)", m_spellInfo->Id, m_caster->GetGuidStr().c_str());
        return;
    }

    Unit* responsibleCaster = m_originalCaster;
    if (realCaster->GetTypeId() == TYPEID_GAMEOBJECT)
        responsibleCaster = ((GameObject*)realCaster)->GetOwner();

    Unit* petInvoker = responsibleCaster ? responsibleCaster : m_caster;
    uint32 level = petInvoker->getLevel();
    if (petInvoker->GetTypeId() != TYPEID_PLAYER)
    {
        // pet players do not need this
        // TODO :: Totem, Pet and Critter may not use this. This is probably wrongly used and need more research.
        uint32 resultLevel = level + std::max(m_spellInfo->EffectMultipleValue[eff_idx], .0f);

        // result level should be a possible level for creatures
        if (resultLevel > 0 && resultLevel <= DEFAULT_MAX_CREATURE_LEVEL)
            level = resultLevel;
    }
    // level of pet summoned using engineering item based at engineering skill level
    else if (m_CastItem)
    {
        ItemPrototype const* proto = m_CastItem->GetProto();
        if (proto && proto->RequiredSkill == SKILL_ENGINEERING && proto->InventoryType == INVTYPE_TRINKET)
        {
            uint16 skill202 = ((Player*)m_caster)->GetSkillValue(SKILL_ENGINEERING);
            if (skill202)
            {
                level = skill202 / 5;
            }
        }
        else if (cInfo)
        {
            if (level >= cInfo->MaxLevel)
                level = cInfo->MaxLevel;
            else if (level <= cInfo->MinLevel)
                level = cInfo->MinLevel;
        }
    }

    // select center of summon position
    float center_x = m_targets.m_destX;
    float center_y = m_targets.m_destY;
    float center_z = m_targets.m_destZ;

    float radius = GetSpellRadius(sSpellRadiusStore.LookupEntry(m_spellInfo->EffectRadiusIndex[eff_idx]));

    int32 amount = damage > 0 ? damage : 1;

    for (int32 count = 0; count < amount; ++count)
    {
        Pet* spawnCreature = new Pet(GUARDIAN_PET);

        // If dest location if present
        // Summon 1 unit in dest location
        CreatureCreatePos pos(m_caster->GetMap(), m_targets.m_destX, m_targets.m_destY, m_targets.m_destZ, -m_caster->GetOrientation());

        if (m_targets.m_targetMask & TARGET_FLAG_DEST_LOCATION)
        {
            // Summon in random point all other units if location present
            if (count > 0)
            {
                float x, y, z;
                m_caster->GetRandomPoint(center_x, center_y, center_z, radius, x, y, z);
                pos = CreatureCreatePos(m_caster->GetMap(), x, y, z, m_caster->GetOrientation());
            }
        }
        // Summon if dest location not present near caster
        else
        {
            float px, py, pz;
            m_caster->GetClosePoint(px, py, pz, 2.0f);
            pos = CreatureCreatePos(m_caster->GetMap(), px, py, pz, m_caster->GetOrientation());
        }

        Map* map = m_caster->GetMap();
        uint32 pet_number = sObjectMgr.GeneratePetNumber();
        if (!spawnCreature->Create(map->GenerateLocalLowGuid(HIGHGUID_PET), pos, cInfo, pet_number))
        {
            sLog.outError("Spell::DoSummonGuardian: can't create creature entry %u for spell %u.", pet_entry, m_spellInfo->Id);
            delete spawnCreature;
            return;
        }

        spawnCreature->SetLoading(true);

        spawnCreature->SetRespawnCoord(pos);

        if (m_duration > 0)
            spawnCreature->SetDuration(m_duration);

        // spawnCreature->SetName("");                      // generated by client
        spawnCreature->SetOwnerGuid(m_caster->GetObjectGuid());
        spawnCreature->SetUInt32Value(UNIT_FIELD_FLAGS, cInfo->UnitFlags);
        spawnCreature->SetUInt32Value(UNIT_NPC_FLAGS, cInfo->NpcFlags);
        spawnCreature->setFaction(m_caster->getFaction());
        spawnCreature->SetUInt32Value(UNIT_FIELD_PET_NAME_TIMESTAMP, 0);
        spawnCreature->SetUInt32Value(UNIT_CREATED_BY_SPELL, m_spellInfo->Id);

        spawnCreature->InitStatsForLevel(level);

        if (m_caster->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED))
            spawnCreature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED);

        if (m_caster->IsImmuneToNPC())
            spawnCreature->SetImmuneToNPC(true);

        if (m_caster->IsImmuneToPlayer())
            spawnCreature->SetImmuneToPlayer(true);

        if (m_caster->IsPvP())
            spawnCreature->SetPvP(true);

        spawnCreature->SetByteValue(UNIT_FIELD_BYTES_2, 1, UNIT_BYTE2_FLAG_SUPPORTABLE | UNIT_BYTE2_FLAG_UNK5);

        if (CharmInfo* charmInfo = spawnCreature->GetCharmInfo())
            charmInfo->SetPetNumber(pet_number, false);

        spawnCreature->SetLoading(false);
        m_caster->AddGuardian(spawnCreature);

        map->Add((Creature*)spawnCreature);
        spawnCreature->AIM_Initialize();

        // Notify Summoner
        if (m_originalCaster && (m_originalCaster != m_caster) && (m_originalCaster->AI()))
            m_originalCaster->AI()->JustSummoned(spawnCreature);
        else if (m_caster->AI())
            m_caster->AI()->JustSummoned(spawnCreature);
    }
}

void Spell::EffectTeleUnitsFaceCaster(SpellEffectIndex /*eff_idx*/)
{
    if (!unitTarget)
        return;

    if (unitTarget->IsTaxiFlying())
        return;

    float x, y, z;
    if (m_targets.m_targetMask & TARGET_FLAG_DEST_LOCATION)
        m_targets.getDestination(x, y, z);
    else
        return;

    unitTarget->NearTeleportTo(x, y, z, -m_caster->GetOrientation(), unitTarget == m_caster);
}

void Spell::EffectLearnSkill(SpellEffectIndex eff_idx)
{
    if (unitTarget->GetTypeId() != TYPEID_PLAYER)
        return;

    if (damage < 0)
        return;

    Player* target = static_cast<Player*>(unitTarget);

    uint16 skillid =  uint16(m_spellInfo->EffectMiscValue[eff_idx]);
    uint16 step = uint16(damage);
    target->SetSkillStep(skillid, step);

    if (WorldObject const* caster = GetCastingObject())
        DEBUG_LOG("Spell: %s teaches %s skill %u (to step %u)", caster->GetGuidStr().c_str(), target->GetGuidStr().c_str(), skillid, step);
}

void Spell::EffectAddHonor(SpellEffectIndex /*eff_idx*/)
{
    if (unitTarget->GetTypeId() != TYPEID_PLAYER)
        return;

    // honor-spells don't scale with level and won't be casted by an item
    // also we must use damage (spelldescription says +25 honor but damage is only 24)
    ((Player*)unitTarget)->AddHonorCP(float(damage), HONORABLE);
    DEBUG_FILTER_LOG(LOG_FILTER_SPELL_CAST, "SpellEffect::AddHonor (spell_id %u) rewards %u honor points (non scale) for player: %u", m_spellInfo->Id, damage, ((Player*)unitTarget)->GetGUIDLow());
}

void Spell::EffectSpawn(SpellEffectIndex /*eff_idx*/)
{
    switch (m_spellInfo->Id)
    {
        case 15750: // Rookery Whelp Spawn-in Spell
        case 26262: // Birth
            m_caster->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            break;
    }
}

void Spell::EffectTradeSkill(SpellEffectIndex /*eff_idx*/)
{
    if (unitTarget->GetTypeId() != TYPEID_PLAYER)
        return;
    // uint32 skillid =  m_spellInfo->EffectMiscValue[i];
    // uint16 skillmax = ((Player*)unitTarget)->(skillid);
    // ((Player*)unitTarget)->SetSkill(skillid,skillval?skillval:1,skillmax+75);
}

void Spell::EffectEnchantItemPerm(SpellEffectIndex eff_idx)
{
    if (m_caster->GetTypeId() != TYPEID_PLAYER)
        return;
    if (!itemTarget)
        return;

    Player* p_caster = (Player*)m_caster;

    // not grow at item use at item case
    p_caster->UpdateCraftSkill(m_spellInfo->Id);

    uint32 enchant_id = m_spellInfo->EffectMiscValue[eff_idx];
    if (!enchant_id)
        return;

    SpellItemEnchantmentEntry const* pEnchant = sSpellItemEnchantmentStore.LookupEntry(enchant_id);
    if (!pEnchant)
        return;

    // item can be in trade slot and have owner diff. from caster
    Player* item_owner = itemTarget->GetOwner();
    if (!item_owner)
        return;

    if (item_owner != p_caster && p_caster->GetSession()->GetSecurity() > SEC_PLAYER && sWorld.getConfig(CONFIG_BOOL_GM_LOG_TRADE))
    {
        sLog.outCommand(p_caster->GetSession()->GetAccountId(), "GM %s (Account: %u) enchanting(perm): %s (Entry: %d) for player: %s (Account: %u)",
                        p_caster->GetName(), p_caster->GetSession()->GetAccountId(),
                        itemTarget->GetProto()->Name1, itemTarget->GetEntry(),
                        item_owner->GetName(), item_owner->GetSession()->GetAccountId());
    }

    // remove old enchanting before applying new if equipped
    item_owner->ApplyEnchantment(itemTarget, PERM_ENCHANTMENT_SLOT, false);

    itemTarget->SetEnchantment(PERM_ENCHANTMENT_SLOT, enchant_id, 0, 0, m_caster->GetObjectGuid());

    // add new enchanting if equipped
    item_owner->ApplyEnchantment(itemTarget, PERM_ENCHANTMENT_SLOT, true);
}

void Spell::EffectEnchantItemTmp(SpellEffectIndex eff_idx)
{
    if (m_caster->GetTypeId() != TYPEID_PLAYER)
        return;
    if (!itemTarget)
        return;

    Player* p_caster = (Player*)m_caster;

    uint32 enchant_id = m_spellInfo->EffectMiscValue[eff_idx];
    if (!enchant_id)
    {
        sLog.outError("Spell %u Effect %u (SPELL_EFFECT_ENCHANT_ITEM_TEMPORARY) have 0 as enchanting id", m_spellInfo->Id, eff_idx);
        return;
    }

    SpellItemEnchantmentEntry const* pEnchant = sSpellItemEnchantmentStore.LookupEntry(enchant_id);
    if (!pEnchant)
    {
        sLog.outError("Spell %u Effect %u (SPELL_EFFECT_ENCHANT_ITEM_TEMPORARY) have nonexistent enchanting id %u ", m_spellInfo->Id, eff_idx, enchant_id);
        return;
    }

    // select enchantment duration
    uint32 duration;

    // TODO: Strange stuff in following code
    // shaman family enchantments
    if (m_spellInfo->Attributes == (SPELL_ATTR_TARGET_MAINHAND_ITEM | SPELL_ATTR_NOT_SHAPESHIFT | SPELL_ATTR_DONT_AFFECT_SHEATH_STATE))
        duration = 300;                                     // 5 mins
    // imbue enchantments except Imbue Weapon - Beastslayer
    else if (m_spellInfo->SpellIconID == 241 && m_spellInfo->Id != 7434)
        duration = 3600;                                    // 1 hour
    // Consecrated Weapon and Blessed Wizard Oil
    else if (m_spellInfo->Id == 28891 || m_spellInfo->Id == 28898)
        duration = 3600;                                    // 1 hour
    // some fishing pole bonuses
    else if (m_spellInfo->HasAttribute(SPELL_ATTR_HIDDEN_CLIENTSIDE))
        duration = 600;                                     // 10 mins
    // default case
    else
        duration = 1800;                                    // 30 mins

    // item can be in trade slot and have owner diff. from caster
    Player* item_owner = itemTarget->GetOwner();
    if (!item_owner)
        return;

    if (item_owner != p_caster && p_caster->GetSession()->GetSecurity() > SEC_PLAYER && sWorld.getConfig(CONFIG_BOOL_GM_LOG_TRADE))
    {
        sLog.outCommand(p_caster->GetSession()->GetAccountId(), "GM %s (Account: %u) enchanting(temp): %s (Entry: %d) for player: %s (Account: %u)",
                        p_caster->GetName(), p_caster->GetSession()->GetAccountId(),
                        itemTarget->GetProto()->Name1, itemTarget->GetEntry(),
                        item_owner->GetName(), item_owner->GetSession()->GetAccountId());
    }

    // remove old enchanting before applying new if equipped
    item_owner->ApplyEnchantment(itemTarget, TEMP_ENCHANTMENT_SLOT, false);

    itemTarget->SetEnchantment(TEMP_ENCHANTMENT_SLOT, enchant_id, duration * 1000, 0, m_caster->GetObjectGuid());

    // add new enchanting if equipped
    item_owner->ApplyEnchantment(itemTarget, TEMP_ENCHANTMENT_SLOT, true);
}

void Spell::EffectTameCreature(SpellEffectIndex /*eff_idx*/)
{
    // Caster must be player, checked in Spell::CheckCast
    // Spell can be triggered, we need to check original caster prior to caster
    Player* plr = (Player*)GetAffectiveCaster();

    Creature* creatureTarget = (Creature*)unitTarget;

    // cast finish successfully
    // SendChannelUpdate(0);
    finish();

    Pet* pet = new Pet(HUNTER_PET);

    if (!pet->CreateBaseAtCreature(creatureTarget))
    {
        delete pet;
        return;
    }

    pet->SetLoading(true);
    pet->SetOwnerGuid(plr->GetObjectGuid());
    pet->setFaction(plr->getFaction());
    pet->SetUInt32Value(UNIT_CREATED_BY_SPELL, m_spellInfo->Id);

    if (plr->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED))
        pet->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED);

    if (plr->IsImmuneToNPC())
        pet->SetImmuneToNPC(true);

    if (plr->IsImmuneToPlayer())
        pet->SetImmuneToPlayer(true);

    if (plr->IsPvP())
        pet->SetPvP(true);

    pet->GetCharmInfo()->SetPetNumber(sObjectMgr.GeneratePetNumber(), (m_caster->GetTypeId() == TYPEID_PLAYER));

    uint32 level = creatureTarget->getLevel();
    pet->InitStatsForLevel(level);

    pet->SetHealthPercent(creatureTarget->GetHealthPercent());

    // destroy creature object
    creatureTarget->ForcedDespawn();

    // prepare visual effect for levelup
    pet->SetUInt32Value(UNIT_FIELD_LEVEL, level - 1);

    // add pet object to the world
    pet->GetMap()->Add((Creature*)pet);
    pet->AIM_Initialize();
    pet->AI()->SetReactState(REACT_DEFENSIVE);

    // visual effect for levelup
    pet->SetUInt32Value(UNIT_FIELD_LEVEL, level);

    // this enables pet details window (Shift+P)
    pet->InitPetCreateSpells();

    pet->LearnPetPassives();
    pet->CastPetAuras(true);
    pet->CastOwnerTalentAuras();
    pet->UpdateAllStats();

    pet->SetRequiredXpForNextLoyaltyLevel();

    // caster have pet now
    plr->SetPet(pet);

    plr->PetSpellInitialize();
    pet->SetLoading(false);

    pet->SavePetToDB(PET_SAVE_AS_CURRENT, plr);
}

void Spell::EffectSummonPet(SpellEffectIndex eff_idx)
{
    uint32 petentry = m_spellInfo->EffectMiscValue[eff_idx];

    Pet* NewSummon = new Pet;

    Player* _player = nullptr;

    if (m_caster->GetTypeId() == TYPEID_PLAYER)
    {
        _player = static_cast<Player*>(m_caster);

        switch (m_caster->getClass())
        {
            case CLASS_HUNTER:
            {
                NewSummon->LoadPetFromDB(_player);
                return;
            }
            case CLASS_WARLOCK:
            {
                // Remove Demonic Sacrifice auras (known pet)
                Unit::AuraList const& auraClassScripts = m_caster->GetAurasByType(SPELL_AURA_OVERRIDE_CLASS_SCRIPTS);
                for (Unit::AuraList::const_iterator itr = auraClassScripts.begin(); itr != auraClassScripts.end();)
                {
                    if ((*itr)->GetModifier()->m_miscvalue == 2228)
                    {
                        m_caster->RemoveAurasDueToSpell((*itr)->GetId());
                        itr = auraClassScripts.begin();
                    }
                    else
                        ++itr;
                }
            }
            default:
            {
                if (Pet* OldSummon = m_caster->GetPet())
                    OldSummon->Unsummon(PET_SAVE_NOT_IN_SLOT, m_caster);

                // Load pet from db; if any to load
                if (NewSummon->LoadPetFromDB(_player, petentry))
                    return;

                NewSummon->setPetType(SUMMON_PET);
            }
        }
    }
    else
        NewSummon->setPetType(GUARDIAN_PET);

    CreatureInfo const* cInfo = petentry ? ObjectMgr::GetCreatureTemplate(petentry) : nullptr;
    if (!cInfo)
    {
        sLog.outErrorDb("EffectSummonPet: creature entry %u not found for spell %u.", petentry, m_spellInfo->Id);
        delete NewSummon;
        return;
    }

    float px, py, pz;
    m_caster->GetClosePoint(px, py, pz, 2.0f);
    CreatureCreatePos pos(m_caster->GetMap(), px, py, pz, -m_caster->GetOrientation());

    Map* map = m_caster->GetMap();
    uint32 pet_number = sObjectMgr.GeneratePetNumber();
    if (!NewSummon->Create(map->GenerateLocalLowGuid(HIGHGUID_PET), pos, cInfo, pet_number))
    {
        delete NewSummon;
        return;
    }

    NewSummon->SetLoading(true);
    NewSummon->SetRespawnCoord(pos);

    // Level of pet summoned
    uint32 level = m_caster->getLevel();
    if (!_player)
    {
        // pet players do not need this
        // TODO :: Totem, Pet and Critter may not use this. This is probably wrongly used and need more research.
        uint32 resultLevel = level + std::max(m_spellInfo->EffectMultipleValue[eff_idx], 1.0f);

        // result level should be a possible level for creatures
        if (resultLevel > 0 && resultLevel <= DEFAULT_MAX_CREATURE_LEVEL)
            level = resultLevel;
    }

    NewSummon->SetOwnerGuid(m_caster->GetObjectGuid());
    NewSummon->setFaction(m_caster->getFaction());
    NewSummon->SetUInt32Value(UNIT_FIELD_PET_NAME_TIMESTAMP, uint32(time(nullptr)));
    NewSummon->SetUInt32Value(UNIT_CREATED_BY_SPELL, m_spellInfo->Id);

    NewSummon->InitStatsForLevel(level);
    NewSummon->InitPetCreateSpells();

    if (NewSummon->getPetType() == GUARDIAN_PET)
        m_caster->AddGuardian(NewSummon);
    else
        m_caster->SetPet(NewSummon);
    DEBUG_LOG("New Pet has guid %u", NewSummon->GetGUIDLow());

    if (m_caster->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED))
    {
        NewSummon->SetUInt32Value(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED);
        NewSummon->SetUInt32Value(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_NONE);
        NewSummon->SetByteValue(UNIT_FIELD_BYTES_2, 1, UNIT_BYTE2_FLAG_SUPPORTABLE | UNIT_BYTE2_FLAG_AURAS);
    }
    else
    {
        NewSummon->SetFlag(UNIT_FIELD_FLAGS, cInfo->UnitFlags);
        NewSummon->SetUInt32Value(UNIT_NPC_FLAGS, cInfo->NpcFlags);
    }

    if (m_caster->IsImmuneToNPC())
        NewSummon->SetImmuneToNPC(true);

    if (m_caster->IsImmuneToPlayer())
        NewSummon->SetImmuneToPlayer(true);

    if (m_caster->IsPvP())
        NewSummon->SetPvP(true);

    map->Add((Creature*)NewSummon);
    NewSummon->AIM_Initialize();
    NewSummon->AI()->SetReactState(REACT_DEFENSIVE);

    if (_player)
    {
        NewSummon->GetCharmInfo()->SetPetNumber(pet_number, true);

        // generate new name for summon pet
        NewSummon->SetName(sObjectMgr.GeneratePetName(petentry));

        NewSummon->LearnPetPassives();
        NewSummon->CastPetAuras(true);
        NewSummon->CastOwnerTalentAuras();
        NewSummon->UpdateAllStats();

        NewSummon->SavePetToDB(PET_SAVE_AS_CURRENT, _player);
        ((Player*)m_caster)->PetSpellInitialize();
        NewSummon->SetLoading(false);
    }
    else
    {
        NewSummon->SetLoading(false);
        // Notify Summoner
        if (m_originalCaster && (m_originalCaster != m_caster) && (m_originalCaster->AI()))
            m_originalCaster->AI()->JustSummoned(NewSummon);
        else if (m_caster->AI())
            m_caster->AI()->JustSummoned(NewSummon);
    }
}

void Spell::EffectLearnPetSpell(SpellEffectIndex eff_idx)
{
    if (m_caster->GetTypeId() != TYPEID_PLAYER)
        return;

    Player* _player = (Player*)m_caster;

    Pet* pet = _player->GetPet();
    if (!pet)
        return;
    if (!pet->isAlive())
        return;

    SpellEntry const* learn_spellproto = sSpellTemplate.LookupEntry<SpellEntry>(m_spellInfo->EffectTriggerSpell[eff_idx]);
    if (!learn_spellproto)
        return;

    pet->SetTP(pet->m_TrainingPoints - pet->GetTPForSpell(learn_spellproto->Id));
    pet->learnSpell(learn_spellproto->Id);

    pet->SavePetToDB(PET_SAVE_AS_CURRENT, _player);
    _player->PetSpellInitialize();

    if (WorldObject const* caster = GetCastingObject())
        DEBUG_LOG("Spell: %s has learned spell %u from %s", pet->GetGuidStr().c_str(), learn_spellproto->Id, caster->GetGuidStr().c_str());
}

void Spell::EffectTaunt(SpellEffectIndex /*eff_idx*/)
{
    if (!unitTarget)
        return;

    // this effect use before aura Taunt apply for prevent taunt already attacking target
    // for spell as marked "non effective at already attacking target"
    if (unitTarget->GetTypeId() != TYPEID_PLAYER)
    {
        if (unitTarget->getVictim() == m_caster)
        {
            SendCastResult(SPELL_FAILED_DONT_REPORT);
            return;
        }
    }

    // Also use this effect to set the taunter's threat to the taunted creature's highest value
    if (unitTarget->CanHaveThreatList() && unitTarget->getThreatManager().getCurrentVictim())
    {
        float addedThreat = unitTarget->getThreatManager().getCurrentVictim()->getThreat() - unitTarget->getThreatManager().getThreat(m_caster);
        unitTarget->getThreatManager().addThreatDirectly(m_caster, addedThreat);
        unitTarget->getThreatManager().setCurrentVictimByTarget(m_caster); // force changes the target to caster of taunt
    }
    m_spellLog.AddLog(uint32(SPELL_EFFECT_ATTACK_ME), unitTarget->GetObjectGuid());
}

void Spell::EffectWeaponDmg(SpellEffectIndex eff_idx)
{
    if (!unitTarget)
        return;
    if (!unitTarget->isAlive())
        return;

    // multiple weapon dmg effect workaround
    // execute only the last weapon damage
    // and handle all effects at once
    for (int j = 0; j < MAX_EFFECT_INDEX; ++j)
    {
        switch (m_spellInfo->Effect[j])
        {
            case SPELL_EFFECT_WEAPON_DAMAGE:
            case SPELL_EFFECT_WEAPON_DAMAGE_NOSCHOOL:
            case SPELL_EFFECT_NORMALIZED_WEAPON_DMG:
            case SPELL_EFFECT_WEAPON_PERCENT_DAMAGE:
                if (j < int(eff_idx))                       // we must calculate only at last weapon effect
                    return;
                break;
        }
    }

    // some spell specific modifiers
    bool customBonusDamagePercentMod = false;
    float bonusDamagePercentMod  = 1.0f;                    // applied to fixed effect damage bonus if set customBonusDamagePercentMod
    float weaponDamagePercentMod = 1.0f;                    // applied to weapon damage (and to fixed effect damage bonus if customBonusDamagePercentMod not set
    float totalDamagePercentMod  = 1.0f;                    // applied to final bonus+weapon damage
    bool normalized = false;

    int32 spell_bonus = 0;                                  // bonus specific for spell

    switch (m_spellInfo->SpellFamilyName)
    {
        case SPELLFAMILY_ROGUE:
        {
            // Ambush
            if (m_spellInfo->SpellFamilyFlags & uint64(0x00000200))
            {
                customBonusDamagePercentMod = true;
                bonusDamagePercentMod = 2.5f;               // 250%
            }
            break;
        }
        case SPELLFAMILY_PALADIN:
        {
            break;
        }
    }

    int32 fixed_bonus = 0;
    for (int j = 0; j < MAX_EFFECT_INDEX; ++j)
    {
        switch (m_spellInfo->Effect[j])
        {
            case SPELL_EFFECT_WEAPON_DAMAGE:
            case SPELL_EFFECT_WEAPON_DAMAGE_NOSCHOOL:
                fixed_bonus += CalculateDamage(SpellEffectIndex(j), unitTarget);
                break;
            case SPELL_EFFECT_NORMALIZED_WEAPON_DMG:
                fixed_bonus += CalculateDamage(SpellEffectIndex(j), unitTarget);
                normalized = true;
                break;
            case SPELL_EFFECT_WEAPON_PERCENT_DAMAGE:
                weaponDamagePercentMod *= float(CalculateDamage(SpellEffectIndex(j), unitTarget)) / 100.0f;

                // applied only to prev.effects fixed damage
                if (customBonusDamagePercentMod)
                    fixed_bonus = int32(fixed_bonus * bonusDamagePercentMod);
                else
                    fixed_bonus = int32(fixed_bonus * weaponDamagePercentMod);
                break;
            default:
                break;                                      // not weapon damage effect, just skip
        }
    }

    // non-weapon damage
    int32 bonus = spell_bonus + fixed_bonus;

    bool addDamage = false; // hack in place until effect handler is split up
    switch (m_spellInfo->DmgClass)
    {
        case SPELL_DAMAGE_CLASS_MELEE:
        case SPELL_DAMAGE_CLASS_RANGED: addDamage = true; break;
        case SPELL_DAMAGE_CLASS_MAGIC: break; // TODO: Add behaviour for spell behaviour
        case SPELL_DAMAGE_CLASS_NONE: break; // do nothing
    }

    if (m_spellInfo->Effect[eff_idx] == SPELL_EFFECT_WEAPON_DAMAGE_NOSCHOOL)
        addDamage = true;

    if (addDamage)
    {
        // apply to non-weapon bonus weapon total pct effect, weapon total flat effect included in weapon damage
        if (bonus)
        {
            UnitMods unitMod;
            switch (m_attackType)
            {
                default:
                case BASE_ATTACK:   unitMod = UNIT_MOD_DAMAGE_MAINHAND; break;
                case OFF_ATTACK:    unitMod = UNIT_MOD_DAMAGE_OFFHAND;  break;
                case RANGED_ATTACK: unitMod = UNIT_MOD_DAMAGE_RANGED;   break;
            }

            float weapon_total_pct = m_caster->GetModifierValue(unitMod, TOTAL_PCT);
            bonus = int32(bonus * weapon_total_pct);
        }

        // + weapon damage with applied weapon% dmg to base weapon damage in call
        bonus += int32(m_caster->CalculateDamage(m_attackType, normalized) * weaponDamagePercentMod);
    }

    // total damage
    bonus = int32(bonus * totalDamagePercentMod);

    // prevent negative damage
    m_damage += uint32(bonus > 0 ? bonus : 0);

    // Mangle (Cat): CP
    if (m_spellInfo->IsFitToFamily(SPELLFAMILY_DRUID, uint64(0x0000040000000000)))
    {
        if (m_caster->GetTypeId() == TYPEID_PLAYER)
            ((Player*)m_caster)->AddComboPoints(unitTarget, 1);
    }
}

void Spell::EffectThreat(SpellEffectIndex /*eff_idx*/)
{
    if (!unitTarget || !unitTarget->isAlive() || !m_caster->isAlive())
        return;

    if (!unitTarget->CanHaveThreatList())
        return;

    unitTarget->AddThreat(m_caster, float(damage), false, GetSpellSchoolMask(m_spellInfo), m_spellInfo);
    m_spellLog.AddLog(uint32(SPELL_EFFECT_THREAT), unitTarget->GetObjectGuid());
}

void Spell::EffectHealMaxHealth(SpellEffectIndex /*eff_idx*/)
{
    if (!unitTarget)
        return;
    if (!unitTarget->isAlive())
        return;

    uint32 heal = m_caster->GetMaxHealth();

    m_healing += heal;
}

void Spell::EffectInterruptCast(SpellEffectIndex /*eff_idx*/)
{
    if (!unitTarget)
        return;
    if (!unitTarget->isAlive())
        return;

    uint32 interruptedSpellId = 0;
    // TODO: not all spells that used this effect apply cooldown at school spells
    // also exist case: apply cooldown to interrupted cast only and to all spells
    for (uint32 i = CURRENT_FIRST_NON_MELEE_SPELL; i < CURRENT_MAX_SPELL; ++i)
    {
        if (Spell* spell = unitTarget->GetCurrentSpell(CurrentSpellTypes(i)))
        {
            SpellEntry const* curSpellInfo = spell->m_spellInfo;
            // check if we can interrupt spell
            if ((curSpellInfo->InterruptFlags & SPELL_INTERRUPT_FLAG_INTERRUPT) && curSpellInfo->PreventionType == SPELL_PREVENTION_TYPE_SILENCE)
            {
                unitTarget->LockOutSpells(GetSpellSchoolMask(curSpellInfo), GetSpellDuration(m_spellInfo));
                unitTarget->InterruptSpell(CurrentSpellTypes(i), false);
                interruptedSpellId = curSpellInfo->Id;
            }
        }
    }

    if (interruptedSpellId)
        m_spellLog.AddLog(uint32(SPELL_EFFECT_INTERRUPT_CAST), unitTarget->GetObjectGuid(), interruptedSpellId);
}

void Spell::EffectSummonObjectWild(SpellEffectIndex eff_idx)
{
    uint32 gameobject_id = m_spellInfo->EffectMiscValue[eff_idx];

    GameObject* pGameObj = new GameObject;

    WorldObject* target = focusObject;
    if (!target)
        target = m_caster;

    float x, y, z;
    if (m_targets.m_targetMask & TARGET_FLAG_DEST_LOCATION)
        m_targets.getDestination(x, y, z);
    else
        m_caster->GetClosePoint(x, y, z, DEFAULT_WORLD_OBJECT_SIZE);

    Map* map = target->GetMap();

    if (!pGameObj->Create(map->GenerateLocalLowGuid(HIGHGUID_GAMEOBJECT), gameobject_id, map,
                          x, y, z, target->GetOrientation()))
    {
        delete pGameObj;
        return;
    }

    pGameObj->SetRespawnTime(m_duration > 0 ? m_duration / IN_MILLISECONDS : 0);
    pGameObj->SetSpellId(m_spellInfo->Id);

    // Wild object not have owner and check clickable by players
    map->Add(pGameObj);
    pGameObj->AIM_Initialize();

    if (pGameObj->GetGoType() == GAMEOBJECT_TYPE_FLAGDROP && m_caster->GetTypeId() == TYPEID_PLAYER)
    {
        Player* pl = (Player*)m_caster;
        BattleGround* bg = ((Player*)m_caster)->GetBattleGround();

        switch (pGameObj->GetMapId())
        {
            case 489:                                       // WS
            {
                if (bg && bg->GetTypeID() == BATTLEGROUND_WS && bg->GetStatus() == STATUS_IN_PROGRESS)
                {
                    Team team = pl->GetTeam() == ALLIANCE ? HORDE : ALLIANCE;

                    ((BattleGroundWS*)bg)->SetDroppedFlagGuid(pGameObj->GetObjectGuid(), team);
                }
                break;
            }
        }
    }

    // Notify Summoner
    if (m_originalCaster && (m_originalCaster != m_caster) && (m_originalCaster->AI()))
        m_originalCaster->AI()->JustSummoned(pGameObj);
    else if (m_caster->AI())
        m_caster->AI()->JustSummoned(pGameObj);
}

void Spell::EffectScriptEffect(SpellEffectIndex eff_idx)
{
    // TODO: we must implement hunter pet summon at login there (spell 6962)

    switch (m_spellInfo->SpellFamilyName)
    {
        case SPELLFAMILY_GENERIC:
        {
            switch (m_spellInfo->Id)
            {
                case 5249:                                  // Ice Lock
                {
                    if (unitTarget)
                        unitTarget->CastSpell(unitTarget, 22856, TRIGGERED_OLD_TRIGGERED, nullptr);
                    return;
                }
                case 8856:                                  // Bending Shinbone
                {
                    if (!itemTarget && m_caster->GetTypeId() != TYPEID_PLAYER)
                        return;

                    uint32 spell_id = 0;
                    switch (urand(1, 5))
                    {
                        case 1:  spell_id = 8854; break;
                        default: spell_id = 8855; break;
                    }

                    m_caster->CastSpell(m_caster, spell_id, TRIGGERED_OLD_TRIGGERED, nullptr);
                    return;
                }
                case 10101:                                 // Knock Away variants
                case 18670:
                case 18813:
                case 18945:
                case 19633:
                case 20686:
                case 23382:
                case 25778:
                case 30121:                                 // Forceful Howl - Plagued Deathhound
                {
                    // Knock Away variants and derrivatives with scripted threat reduction component
                    if (!unitTarget)
                        return;
                    // Default (most of the time) reduction is 50%. TODO: Verify when possible per spell
                    int32 pct = -50;
                    // A subset of spells has different values
                    switch (m_spellInfo->Id)
                    {
                        case 19633:
                        case 25778:
                            pct = -25;
                    }
                    m_caster->getThreatManager().modifyThreatPercent(unitTarget, pct);
                    return;
                }
                case 17512:                                 // Piccolo of the Flaming Fire
                {
                    if (!unitTarget || unitTarget->GetTypeId() != TYPEID_PLAYER)
                        return;

                    unitTarget->HandleEmoteCommand(EMOTE_STATE_DANCE);

                    return;
                }
                case 22539:                                 // Shadow Flame (All script effects, not just end ones to
                case 22972:                                 // prevent player from dodging the last triggered spell)
                case 22975:
                case 22976:
                case 22977:
                case 22978:
                case 22979:
                case 22980:
                case 22981:
                case 22982:
                case 22983:
                case 22984:
                case 22985:
                {
                    if (!unitTarget || !unitTarget->isAlive())
                        return;

                    // Onyxia Scale Cloak
                    if (unitTarget->GetDummyAura(22683))
                        return;

                    // Shadow Flame
                    m_caster->CastSpell(unitTarget, 22682, TRIGGERED_OLD_TRIGGERED);
                    return;
                }
                case 23853:                                 // Jubling Cooldown
                {
                    unitTarget->CastSpell(nullptr, 23852, TRIGGERED_NONE);
                    return;
                }
                case 24194:                                 // Uther's Tribute
                case 24195:                                 // Grom's Tribute
                {
                    if (m_caster->GetTypeId() != TYPEID_PLAYER)
                        return;

                    uint8 race = m_caster->getRace();
                    uint32 spellId = 0;

                    switch (m_spellInfo->Id)
                    {
                        case 24194:
                            switch (race)
                            {
                                case RACE_HUMAN:            spellId = 24105; break;
                                case RACE_DWARF:            spellId = 24107; break;
                                case RACE_NIGHTELF:         spellId = 24108; break;
                                case RACE_GNOME:            spellId = 24106; break;
                            }
                            break;
                        case 24195:
                            switch (race)
                            {
                                case RACE_ORC:              spellId = 24104; break;
                                case RACE_UNDEAD:           spellId = 24103; break;
                                case RACE_TAUREN:           spellId = 24102; break;
                                case RACE_TROLL:            spellId = 24101; break;
                            }
                            break;
                    }

                    if (spellId)
                        m_caster->CastSpell(m_caster, spellId, TRIGGERED_OLD_TRIGGERED);

                    return;
                }
                case 24320:                                 // Poisonous Blood
                {
                    unitTarget->CastSpell(unitTarget, 24321, TRIGGERED_OLD_TRIGGERED, nullptr, nullptr, m_caster->GetObjectGuid());
                    return;
                }
                case 24324:                                 // Blood Siphon
                {
                    if (!unitTarget || unitTarget->GetTypeId() != TYPEID_PLAYER)
                        return;

                    unitTarget->CastSpell(m_caster, unitTarget->HasAura(24321) ? 24323 : 24322, TRIGGERED_OLD_TRIGGERED);
                    return;
                }
                case 24590:                                 // Brittle Armor - need remove one 24575 Brittle Armor aura
                    unitTarget->RemoveAuraHolderFromStack(24575);
                    return;
                case 24714:                                 // Trick
                {
                    if (m_caster->GetTypeId() != TYPEID_PLAYER)
                        return;

                    if (roll_chance_i(14))                  // Trick (can be different critter models). 14% since below can have 1 of 6
                        m_caster->CastSpell(m_caster, 24753, TRIGGERED_OLD_TRIGGERED);
                    else                                    // Random Costume, 6 different (plus add. for gender)
                        m_caster->CastSpell(m_caster, 24720, TRIGGERED_OLD_TRIGGERED);

                    return;
                }
                case 24717:                                 // Pirate Costume
                {
                    if (!unitTarget || unitTarget->GetTypeId() != TYPEID_PLAYER)
                        return;

                    // Pirate Costume (male or female)
                    m_caster->CastSpell(unitTarget, unitTarget->getGender() == GENDER_MALE ? 24708 : 24709, TRIGGERED_OLD_TRIGGERED);
                    return;
                }
                case 24718:                                 // Ninja Costume
                {
                    if (!unitTarget || unitTarget->GetTypeId() != TYPEID_PLAYER)
                        return;

                    // Ninja Costume (male or female)
                    m_caster->CastSpell(unitTarget, unitTarget->getGender() == GENDER_MALE ? 24711 : 24710, TRIGGERED_OLD_TRIGGERED);
                    return;
                }
                case 24719:                                 // Leper Gnome Costume
                {
                    if (!unitTarget || unitTarget->GetTypeId() != TYPEID_PLAYER)
                        return;

                    // Leper Gnome Costume (male or female)
                    m_caster->CastSpell(unitTarget, unitTarget->getGender() == GENDER_MALE ? 24712 : 24713, TRIGGERED_OLD_TRIGGERED);
                    return;
                }
                case 24720:                                 // Random Costume
                {
                    if (!unitTarget || unitTarget->GetTypeId() != TYPEID_PLAYER)
                        return;

                    uint32 spellId = 0;

                    switch (urand(0, 6))
                    {
                        case 0:
                            spellId = unitTarget->getGender() == GENDER_MALE ? 24708 : 24709;
                            break;
                        case 1:
                            spellId = unitTarget->getGender() == GENDER_MALE ? 24711 : 24710;
                            break;
                        case 2:
                            spellId = unitTarget->getGender() == GENDER_MALE ? 24712 : 24713;
                            break;
                        case 3:
                            spellId = 24723;
                            break;
                        case 4:
                            spellId = 24732;
                            break;
                        case 5:
                            spellId = unitTarget->getGender() == GENDER_MALE ? 24735 : 24736;
                            break;
                        case 6:
                            spellId = 24740;
                            break;
                    }

                    m_caster->CastSpell(unitTarget, spellId, TRIGGERED_OLD_TRIGGERED);
                    return;
                }
                case 24731:                                    // Cannon Fire
                {
                    if (!unitTarget || m_caster->GetTypeId() != TYPEID_PLAYER)
                        return;

                    unitTarget->CastSpell(m_caster, 24742, TRIGGERED_OLD_TRIGGERED);
                    return;
                }
                case 24737:                                 // Ghost Costume
                {
                    if (!unitTarget || unitTarget->GetTypeId() != TYPEID_PLAYER)
                        return;

                    // Ghost Costume (male or female)
                    m_caster->CastSpell(unitTarget, unitTarget->getGender() == GENDER_MALE ? 24735 : 24736, TRIGGERED_OLD_TRIGGERED);
                    return;
                }
                case 24742:                                 // Magic Wings
                {
                    if(!unitTarget)
                        return;

                    unitTarget->RemoveAurasDueToSpell(24754);   // Darkmoon Faire Cannon root aura
                    return;
                }
                case 24751:                                 // Trick or Treat
                {
                    if (!unitTarget || unitTarget->GetTypeId() != TYPEID_PLAYER)
                        return;

                    // Tricked or Treated
                    unitTarget->CastSpell(unitTarget, 24755, TRIGGERED_OLD_TRIGGERED);

                    // Treat / Trick
                    unitTarget->CastSpell(unitTarget, roll_chance_i(50) ? 24714 : 24715, TRIGGERED_OLD_TRIGGERED);
                    return;
                }
                case 25671:                                 // Drain Mana
                case 25755:
                {
                    unitTarget->CastSpell(m_caster, 26639, TRIGGERED_OLD_TRIGGERED);
                    return;
                }
                case 25676: // Moam                         // Drain Mana
                case 26559: // Obsidian Nullifier
                {
                    m_caster->CastSpell(unitTarget, 25671, TRIGGERED_OLD_TRIGGERED);
                    return;
                }
                case 25684:                                 // Summon Mana Fiends
                {
                    m_caster->CastSpell(m_caster, 25681, TRIGGERED_OLD_TRIGGERED);
                    m_caster->CastSpell(m_caster, 25682, TRIGGERED_OLD_TRIGGERED);
                    m_caster->CastSpell(m_caster, 25683, TRIGGERED_OLD_TRIGGERED);

                    return;
                }
                case 25754: // Obsidian Destroyer           // Drain Mana
                case 26457: // Obsidian Eradicator
                {
                    m_caster->CastSpell(unitTarget, 25755, TRIGGERED_OLD_TRIGGERED);
                    return;
                }
                case 26004:                                 // Mistletoe
                {
                    if (!unitTarget)
                        return;

                    unitTarget->HandleEmote(EMOTE_ONESHOT_CHEER);
                    return;
                }
                case 26137:                                 // Rotate Trigger
                {
                    if (!unitTarget)
                        return;

                    unitTarget->CastSpell(unitTarget, urand(0, 1) ? 26009 : 26136, TRIGGERED_OLD_TRIGGERED);
                    return;
                }
                case 26218:                                 // Mistletoe
                {
                    if (!unitTarget || unitTarget->GetTypeId() != TYPEID_PLAYER)
                        return;

                    uint32 spells[2] = {26206, 26207};

                    m_caster->CastSpell(unitTarget, spells[urand(0, 1)], TRIGGERED_OLD_TRIGGERED);
                    return;
                }
                case 26275:                                 // PX-238 Winter Wondervolt TRAP
                {
                    uint32 spells[4] = {26272, 26157, 26273, 26274};

                    // check presence
                    for (int j = 0; j < 4; ++j)
                        if (unitTarget->HasAura(spells[j], EFFECT_INDEX_0))
                            return;

                    unitTarget->CastSpell(unitTarget, spells[urand(0, 3)], TRIGGERED_OLD_TRIGGERED);
                    return;
                }
                case 26465:                                 // Mercurial Shield - need remove one 26464 Mercurial Shield aura
                    unitTarget->RemoveAuraHolderFromStack(26464);
                    return;
                case 26524:                                 // Sand Trap (AQ20 - Kurinnaxx, parent spell selecting one target)
                {
                    if (unitTarget)
                        unitTarget->CastSpell(unitTarget, 25648, TRIGGERED_OLD_TRIGGERED, nullptr, nullptr, m_caster->GetObjectGuid());
                    return;
                }
                case 26528:                                 // Summon Reindeer - trigger actual summoning
                {
                    if (unitTarget)
                        unitTarget->CastSpell(nullptr, 26529, TRIGGERED_NONE);
                    return;
                }
                case 26656:                                 // Summon Black Qiraji Battle Tank
                {
                    if (!unitTarget)
                        return;

                    // Prevent stacking of mounts
                    unitTarget->RemoveSpellsCausingAura(SPELL_AURA_MOUNTED);

                    // Two separate mounts depending on area id (allows use both in and out of specific instance)
                    if (unitTarget->GetAreaId() == 3428)
                        unitTarget->CastSpell(unitTarget, 25863, TRIGGERED_NONE);
                    else
                        unitTarget->CastSpell(unitTarget, 26655, TRIGGERED_NONE);

                    return;
                }
                case 26663:                                 // Valentine - Orgrimmar Grunt
                case 26923:                                 // Valentine - Thunderbluff Watcher
                case 26924:                                 // Valentine - Undercity Guardian
                case 27541:                                 // Valentine - Darnassus Sentinel
                case 27547:                                 // Valentine - Ironforge Guard
                case 27548:                                 // Valentine - Stormwind City Guard
                {
                    if (!unitTarget || unitTarget->GetTypeId() != TYPEID_PLAYER)
                        return;

                    uint32 PledgeGiftOrHeartbroken;
                    // Guard spellIds map [Pledge of Friendship , Pledge of Adoration]
                    std::map<uint32, std::vector<uint32>> loveAirSpellsMapForFaction = {
                            {11, {27242, 27510}},   // Stormwind
                            {85, {27247, 27507}},   // Orgrimmar
                            {57, {27244, 27506}},   // Ironforge
                            {68, {27246, 27515}},   // Undercity Guardian
                            {71, {27246, 27515}},   // Undercity Seeker
                            {79, {27245, 27504}},   // Darnassus
                            {105, {27248, 27513}}   // Thunderbluff
                        };

                    if (loveAirSpellsMapForFaction.count(m_caster->getFaction()))
                    {
                        if (!urand(0, 5))                       // Sets 1 in 6 chance to cast Heartbroken
                            PledgeGiftOrHeartbroken = 26898;    // Heartbroken
                        else if (!unitTarget->HasAura(26680))
                        {
                            PledgeGiftOrHeartbroken = loveAirSpellsMapForFaction[m_caster->getFaction()][1];    // Pledge of Adoration for related faction
                            unitTarget->CastSpell(unitTarget, 26680, TRIGGERED_OLD_TRIGGERED);                  // Cast Adored
                        }
                        else
                            PledgeGiftOrHeartbroken = loveAirSpellsMapForFaction[m_caster->getFaction()][0];    // Pledge of Friendship for related faction

                        unitTarget->CastSpell(unitTarget, PledgeGiftOrHeartbroken, TRIGGERED_OLD_TRIGGERED);
                        m_caster->RemoveAurasDueToSpell(27741);                             // Remove Love is in the Air from guard
                        ((Player*)unitTarget)->DestroyItemCount(21815, 1, true, false);     // Remove 1 love token on cast from inventory
                    }
                    return;
                }
                case 26678:                                 // Create Heart Candy
                {
                    uint32 spellId = 0;

                    switch (urand(0, 7))
                    {
                        case 0: spellId = 26668; break;
                        case 1: spellId = 26670; break;
                        case 2: spellId = 26671; break;
                        case 3: spellId = 26672; break;
                        case 4: spellId = 26673; break;
                        case 5: spellId = 26674; break;
                        case 6: spellId = 26675; break;
                        case 7: spellId = 26676; break;
                    }

                    m_caster->CastSpell(m_caster, spellId, TRIGGERED_OLD_TRIGGERED);
                    return;
                }
                case 27549:                                 // Valentine - Horde Civilian
                case 27550:                                 // Valentine - Alliance Civilian
                {
                    if (!unitTarget || unitTarget->GetTypeId() != TYPEID_PLAYER)
                        return;

                    uint32 PledgeGiftOrHeartbroken;
                    // Civilian spellIds map [Gift of Friendship , Gift of Adoration]
                    std::map<uint32, std::vector<uint32>> loveAirSpellsMapForFaction = {
                            {12, {27525, 27509}},   // Stormwind
                            {29, {27523, 27505}},   // Orgrimmar orcs
                            {55, {27520, 27503}},   // Ironforge dwarves
                            {68, {27529, 27512}},   // Undercity
                            {80, {27519, 26901}},   // Darnassus
                            {104, {27524, 27511}},  // Thunderbluff
                            {126, {27523, 27505}},  // Orgrimmar trolls
                            {875, {27520, 27503}}   // Ironforge gnomes
                        };

                    if (loveAirSpellsMapForFaction.count(m_caster->getFaction()))
                    {
                        if (!urand(0, 5))                       // Sets 1 in 6 chance to cast Heartbroken
                            PledgeGiftOrHeartbroken = 26898;    // Heartbroken
                        else if (!unitTarget->HasAura(26680))
                        {
                            PledgeGiftOrHeartbroken = loveAirSpellsMapForFaction[m_caster->getFaction()][1];    // Gift of Adoration for related faction
                            unitTarget->CastSpell(unitTarget, 26680, TRIGGERED_OLD_TRIGGERED);                  // Cast Adored
                        }
                        else
                            PledgeGiftOrHeartbroken = loveAirSpellsMapForFaction[m_caster->getFaction()][0];    // Gift of Friendship for related faction

                        unitTarget->CastSpell(unitTarget, PledgeGiftOrHeartbroken, TRIGGERED_OLD_TRIGGERED);
                        m_caster->RemoveAurasDueToSpell(27741);                             // Remove Love is in the Air from civilian
                        ((Player*)unitTarget)->DestroyItemCount(21815, 1, true, false);     // remove 1 love token on cast from inventory
                    }
                    return;
                }
                case 27654:                                 // Love is in the Air Test
                {
                    if (unitTarget->GetTypeId() != TYPEID_PLAYER)
                    {
                        unitTarget->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);          // Add gossip flag for NPC missing it
                        if (!unitTarget->HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_INNKEEPER))  // Cast Amorous if caster is not an innkeeper
                            unitTarget->CastSpell(m_caster, 26869, TRIGGERED_OLD_TRIGGERED);
                    }
                    return;
                }
                case 27687:                                 // Summon Bone Minions
                {
                    if (!unitTarget)
                        return;

                    unitTarget->CastSpell(unitTarget, 27690, TRIGGERED_OLD_TRIGGERED);
                    unitTarget->CastSpell(unitTarget, 27691, TRIGGERED_OLD_TRIGGERED);
                    unitTarget->CastSpell(unitTarget, 27692, TRIGGERED_OLD_TRIGGERED);
                    unitTarget->CastSpell(unitTarget, 27693, TRIGGERED_OLD_TRIGGERED);
                    return;
                }
                case 27695:                                 // Summon Bone Mages
                {
                    if (!unitTarget)
                        return;

                    unitTarget->CastSpell(unitTarget, 27696, TRIGGERED_OLD_TRIGGERED);
                    unitTarget->CastSpell(unitTarget, 27697, TRIGGERED_OLD_TRIGGERED);
                    unitTarget->CastSpell(unitTarget, 27698, TRIGGERED_OLD_TRIGGERED);
                    unitTarget->CastSpell(unitTarget, 27699, TRIGGERED_OLD_TRIGGERED);
                    return;
                }
                case 28352:                                 // Breath of Sargeras
                {
                    if (!unitTarget || unitTarget->GetTypeId() != TYPEID_PLAYER)
                        return;

                    unitTarget->CastSpell(unitTarget, 28342, TRIGGERED_OLD_TRIGGERED);
                    return;
                }
                case 28374:                                 // Decimate (Naxxramas: Gluth)
                {
                    if (!unitTarget)
                        return;

                    int32 damage = unitTarget->GetHealth() - unitTarget->GetMaxHealth() * 0.05f;
                    if (damage > 0)
                        m_caster->CastCustomSpell(unitTarget, 28375, &damage, nullptr, nullptr, TRIGGERED_OLD_TRIGGERED);
                    return;
                }
                case 28560:                                 // Summon Blizzard
                {
                    if (!unitTarget)
                        return;

                    unitTarget->CastSpell(unitTarget, 28561, TRIGGERED_OLD_TRIGGERED, nullptr, nullptr, m_caster->GetObjectGuid());
                    return;
                }
                case 30132:                                 // Despawn Ice Block
                {
                    if (unitTarget && unitTarget->GetTypeId() == TYPEID_PLAYER)
                    {
                        unitTarget->RemoveAurasDueToSpell(31800); // Icebolt immunity spell
                        unitTarget->RemoveAurasDueToSpell(28522); // Icebolt stun/damage spell
                    }
                    return;
                }
                case 30918:                                 // Improved Sprint
                {
                    if (!unitTarget)
                        return;

                    // Removes snares and roots.
                    unitTarget->RemoveAurasAtMechanicImmunity(IMMUNE_TO_ROOT_AND_SNARE_MASK, 30918, true);
                    break;
                }
            }
            break;
        }
        case SPELLFAMILY_WARLOCK:
        {
            switch (m_spellInfo->Id)
            {
                case  6201:                                 // Healthstone creating spells
                case  6202:
                case  5699:
                case 11729:
                case 11730:
                {
                    if (!unitTarget)
                        return;

                    uint32 itemType = GetUsableHealthStoneItemType(unitTarget);
                    if (itemType)
                        DoCreateItem(eff_idx, itemType);
                    return;
                }
            }
            break;
        }
        case SPELLFAMILY_PALADIN:
        {
            // Holy Light
            if (m_spellInfo->SpellIconID == 70)
            {
                if (!unitTarget || !unitTarget->isAlive())
                    return;
                int32 heal = damage;
                int32 spellid = m_spellInfo->Id;            // send main spell id as basepoints for not used effect
                m_caster->CastCustomSpell(unitTarget, 19968, &heal, &spellid, nullptr, TRIGGERED_OLD_TRIGGERED);
            }
            // Flash of Light
            else if (m_spellInfo->SpellIconID  == 242)
            {
                if (!unitTarget || !unitTarget->isAlive())
                    return;
                int32 heal = damage;
                int32 spellid = m_spellInfo->Id;            // send main spell id as basepoints for not used effect
                m_caster->CastCustomSpell(unitTarget, 19993, &heal, &spellid, nullptr, TRIGGERED_OLD_TRIGGERED);
            }
            else if (m_spellInfo->SpellFamilyFlags & uint64(0x0000000000800000))
            {
                if (!unitTarget || !unitTarget->isAlive())
                    return;

                uint32 spellId2 = 0;

                // all seals have aura dummy
                Unit::AuraList const& m_dummyAuras = m_caster->GetAurasByType(SPELL_AURA_DUMMY);
                for (Unit::AuraList::const_iterator itr = m_dummyAuras.begin(); itr != m_dummyAuras.end(); ++itr)
                {
                    SpellEntry const* spellInfo = (*itr)->GetSpellProto();

                    // search seal (all seals have judgement's aura dummy spell id in 2 effect
                    if (!spellInfo || !IsSealSpell((*itr)->GetSpellProto()) || (*itr)->GetEffIndex() != 2)
                        continue;

                    // must be calculated base at raw base points in spell proto, GetModifier()->m_value for S.Righteousness modified by SPELLMOD_DAMAGE
                    spellId2 = (*itr)->GetSpellProto()->CalculateSimpleValue(EFFECT_INDEX_2);

                    if (spellId2 <= 1)
                        continue;

                    // found, remove seal
                    m_caster->RemoveAurasDueToSpell((*itr)->GetId());

                    break;
                }

                m_caster->CastSpell(unitTarget, spellId2, TRIGGERED_OLD_TRIGGERED | TRIGGERED_IGNORE_HIT_CALCULATION);

                return;
            }
            break;
        }
    }

    // normal DB scripted effect
    if (!unitTarget)
        return;

    // Script based implementation. Must be used only for not good for implementation in core spell effects
    // So called only for not processed cases
    if (unitTarget->GetTypeId() == TYPEID_UNIT)
    {
        if (sScriptDevAIMgr.OnEffectScriptEffect(m_caster, m_spellInfo->Id, eff_idx, (Creature*)unitTarget, m_originalCasterGUID))
            return;
    }

    // Previous effect might have started script
    if (!ScriptMgr::CanSpellEffectStartDBScript(m_spellInfo, eff_idx))
        return;

    DEBUG_FILTER_LOG(LOG_FILTER_SPELL_CAST, "Spell ScriptStart spellid %u in EffectScriptEffect", m_spellInfo->Id);
    m_caster->GetMap()->ScriptsStart(sSpellScripts, m_spellInfo->Id, m_caster, unitTarget);
}

void Spell::EffectSanctuary(SpellEffectIndex /*eff_idx*/)
{
    if (!unitTarget)
        return;

    unitTarget->InterruptSpellsCastedOnMe(true);
    unitTarget->CombatStop(false, false);

    // Vanish allows to remove all threat and cast regular stealth so other spells can be used
    if (m_spellInfo->IsFitToFamily(SPELLFAMILY_ROGUE, uint64(0x0000000000000800)))
    {
        MANGOS_ASSERT(m_caster->GetTypeId() == TYPEID_PLAYER);
        Player* casterPlayer = static_cast<Player*>(m_caster);
        casterPlayer->RemoveSpellsCausingAura(SPELL_AURA_MOD_ROOT);
        casterPlayer->SetCannotBeDetectedTimer(1000);
    }
    m_spellLog.AddLog(uint32(SPELL_EFFECT_SANCTUARY), unitTarget->GetObjectGuid());
}

void Spell::EffectAddComboPoints(SpellEffectIndex /*eff_idx*/)
{
    if (!unitTarget)
        return;

    if (m_caster->GetTypeId() != TYPEID_PLAYER)
        return;

    if (damage <= 0)
        return;

    ((Player*)m_caster)->AddComboPoints(unitTarget, damage);
}

void Spell::EffectDuel(SpellEffectIndex eff_idx)
{
    if (!m_caster || !unitTarget || m_caster->GetTypeId() != TYPEID_PLAYER || unitTarget->GetTypeId() != TYPEID_PLAYER)
        return;

    Player* caster = (Player*)m_caster;
    Player* target = (Player*)unitTarget;

    // caster or target already have requested duel
    if (caster->duel || target->duel || !target->GetSocial() || target->GetSocial()->HasIgnore(caster->GetObjectGuid()))
        return;

    // Players can only fight a duel with each other outside (=not inside dungeons and not in capital cities)
    AreaTableEntry const* casterAreaEntry = GetAreaEntryByAreaID(caster->GetAreaId());
    if (casterAreaEntry && !(casterAreaEntry->flags & AREA_FLAG_DUEL))
    {
        SendCastResult(SPELL_FAILED_NO_DUELING);            // Dueling isn't allowed here
        return;
    }

    AreaTableEntry const* targetAreaEntry = GetAreaEntryByAreaID(target->GetAreaId());
    if (targetAreaEntry && !(targetAreaEntry->flags & AREA_FLAG_DUEL))
    {
        SendCastResult(SPELL_FAILED_NO_DUELING);            // Dueling isn't allowed here
        return;
    }

    // CREATE DUEL FLAG OBJECT
    GameObject* pGameObj = new GameObject;

    uint32 gameobject_id = m_spellInfo->EffectMiscValue[eff_idx];

    Map* map = m_caster->GetMap();
    float x = (m_caster->GetPositionX() + unitTarget->GetPositionX()) * 0.5f;
    float y = (m_caster->GetPositionY() + unitTarget->GetPositionY()) * 0.5f;
    float z = m_caster->GetPositionZ();
    m_caster->UpdateAllowedPositionZ(x, y, z);
    if (!pGameObj->Create(map->GenerateLocalLowGuid(HIGHGUID_GAMEOBJECT), gameobject_id, map, x, y, z, m_caster->GetOrientation()))
    {
        delete pGameObj;
        return;
    }

    pGameObj->SetUInt32Value(GAMEOBJECT_FACTION, m_caster->getFaction());
    pGameObj->SetUInt32Value(GAMEOBJECT_LEVEL, m_caster->getLevel() + 1);

    pGameObj->SetRespawnTime(m_duration > 0 ? m_duration / IN_MILLISECONDS : 0);
    pGameObj->SetSpellId(m_spellInfo->Id);

    m_caster->AddGameObject(pGameObj);
    map->Add(pGameObj);
    pGameObj->AIM_Initialize();

    // END

    // Send request
    WorldPacket data(SMSG_DUEL_REQUESTED, 8 + 8);
    data << pGameObj->GetObjectGuid();
    data << caster->GetObjectGuid();
    caster->GetSession()->SendPacket(data);
    target->GetSession()->SendPacket(data);

    // create duel-info
    DuelInfo* duel   = new DuelInfo;
    duel->initiator  = caster;
    duel->opponent   = target;
    duel->startTime  = 0;
    duel->startTimer = 0;
    caster->duel     = duel;

    DuelInfo* duel2   = new DuelInfo;
    duel2->initiator  = caster;
    duel2->opponent   = caster;
    duel2->startTime  = 0;
    duel2->startTimer = 0;
    target->duel      = duel2;

    caster->SetGuidValue(PLAYER_DUEL_ARBITER, pGameObj->GetObjectGuid());
    target->SetGuidValue(PLAYER_DUEL_ARBITER, pGameObj->GetObjectGuid());
}

void Spell::EffectStuck(SpellEffectIndex /*eff_idx*/)
{
    if (m_caster->GetTypeId() != TYPEID_PLAYER)
        return;

    if (!sWorld.getConfig(CONFIG_BOOL_CAST_UNSTUCK))
        return;

    Player* player = (Player*)m_caster;

    DEBUG_LOG("Spell Effect: Stuck");
    DETAIL_LOG("Player %s (guid %u) used auto-unstuck feature at map %u (%f, %f, %f)", player->GetName(), player->GetGUIDLow(), player->GetMapId(), player->GetPositionX(), player->GetPositionY(), player->GetPositionZ());

    if (player->IsTaxiFlying())
        return;

    // If the player is dead, it will return them to the graveyard closest to their corpse.
    if (player->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_GHOST))
    {
        player->RepopAtGraveyard();
        return;
    }

    // If the player is alive, and their hearthstone is in their inventory, and their hearthstone
    // is cooled down, it will activate their hearthstone. The 30 minute hearthstone cooldown is activated as usual.
    if (player->IsSpellReady(8690) && player->HasItemCount(6948, 1, false))
    {
        player->TeleportToHomebind(TELE_TO_SPELL);
        // Trigger cooldown
        SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(8690);
        if (!spellInfo)
            return;
        Spell spell(player, spellInfo, TRIGGERED_OLD_TRIGGERED);
        spell.SendSpellCooldown();
    }
    else
    {
        // If the player is alive, but their hearthstone is either not in their inventory (e.g. in the bank) or
        // their hearthstone is on cooldown, then the game will try to "nudge" the player in a seemingly random direction.
        // @todo This check could possibly more accurately find a safe position to port to, has the potential for porting underground.
        float x, y, z;
        player->GetNearPoint(player, x, y, z, DEFAULT_WORLD_OBJECT_SIZE, 10.0f, player->GetOrientation());
        player->NearTeleportTo(x, y, z, player->GetOrientation());
    }
}

void Spell::EffectSummonPlayer(SpellEffectIndex /*eff_idx*/)
{
    if (!unitTarget || unitTarget->GetTypeId() != TYPEID_PLAYER)
        return;

    // Evil Twin (ignore player summon, but hide this for summoner)
    if (unitTarget->GetDummyAura(23445))
        return;

    float x, y, z;
    m_caster->GetPosition(x, y, z);

    ((Player*)unitTarget)->SetSummonPoint(m_caster->GetMapId(), x, y, z);

    WorldPacket data(SMSG_SUMMON_REQUEST, 8 + 4 + 4);
    data << m_caster->GetObjectGuid();                      // summoner guid
    data << uint32(m_caster->GetZoneId());                  // summoner zone
    data << uint32(MAX_PLAYER_SUMMON_DELAY * IN_MILLISECONDS); // auto decline after msecs
    ((Player*)unitTarget)->GetSession()->SendPacket(data);
}

static ScriptInfo generateActivateCommand()
{
    ScriptInfo si;
    si.command = SCRIPT_COMMAND_ACTIVATE_OBJECT;
    si.id = 0;
    si.buddyEntry = 0;
    si.searchRadiusOrGuid = 0;
    si.data_flags = 0x00;
    return si;
}

void Spell::EffectActivateObject(SpellEffectIndex eff_idx)
{
    if (!gameObjTarget)
        return;

    uint32 misc_value = m_spellInfo->EffectMiscValue[eff_idx];

    switch (misc_value)
    {
        case 3:                     // GO custom anim - found mostly in Lunar Fireworks spells
            gameObjTarget->SendGameObjectCustomAnim(gameObjTarget->GetObjectGuid());
            // no break: the GO is then used;
        case 1:                     // GO simple use
        case 2:                     // unk - 2 spells
        case 4:                     // unk - 1 spell
        case 5:                     // GO trap usage
        case 7:                     // unk - 2 spells
        case 8:                     // GO usage with TargetB = none or random
        case 10:                    // unk - 2 spells
        {
            // Specific case for Darkmoon Faire Cannon (this is probably a hint that our logic about GO use / activation is not accurate)
            switch (m_spellInfo->Id)
            {
            case 24731:
                gameObjTarget->SendGameObjectCustomAnim(gameObjTarget->GetObjectGuid());
                return;
            }

            static ScriptInfo activateCommand = generateActivateCommand();

            int32 delay_secs = m_spellInfo->CalculateSimpleValue(eff_idx);

            gameObjTarget->GetMap()->ScriptCommandStart(activateCommand, delay_secs, m_caster, gameObjTarget);
            break;
        }
        case 12:                    // GO state active alternative - found mostly in Simon Game spells
            gameObjTarget->UseDoorOrButton(0, true);
            break;
        case 15:                    // GO destroy
            gameObjTarget->SetLootState(GO_JUST_DEACTIVATED);
            break;
        case 16:                    // GO custom use - found mostly in Wind Stones spells, Simon Game spells and other GO target summoning spells
        {
            switch (m_spellInfo->Id)
            {
                case 24734:         // Summon Templar Random
                case 24744:         // Summon Templar (fire)
                case 24756:         // Summon Templar (air)
                case 24758:         // Summon Templar (earth)
                case 24760:         // Summon Templar (water)
                case 24763:         // Summon Duke Random
                case 24765:         // Summon Duke (fire)
                case 24768:         // Summon Duke (air)
                case 24770:         // Summon Duke (earth)
                case 24772:         // Summon Duke (water)
                case 24784:         // Summon Royal Random
                case 24786:         // Summon Royal (fire)
                case 24788:         // Summon Royal (air)
                case 24789:         // Summon Royal (earth)
                case 24790:         // Summon Royal (water)
                {
                    uint32 npcEntry = 0;
                    uint32 templars[] = {15209, 15211, 15212, 15307};
                    uint32 dukes[] = {15206, 15207, 15208, 15220};
                    uint32 royals[] = {15203, 15204, 15205, 15305};

                    switch (m_spellInfo->Id)
                    {
                        case 24734: npcEntry = templars[urand(0, 3)]; break;
                        case 24763: npcEntry = dukes[urand(0, 3)];    break;
                        case 24784: npcEntry = royals[urand(0, 3)];   break;
                        case 24744: npcEntry = 15209;                 break;
                        case 24756: npcEntry = 15212;                 break;
                        case 24758: npcEntry = 15307;                 break;
                        case 24760: npcEntry = 15211;                 break;
                        case 24765: npcEntry = 15206;                 break;
                        case 24768: npcEntry = 15220;                 break;
                        case 24770: npcEntry = 15208;                 break;
                        case 24772: npcEntry = 15207;                 break;
                        case 24786: npcEntry = 15203;                 break;
                        case 24788: npcEntry = 15204;                 break;
                        case 24789: npcEntry = 15205;                 break;
                        case 24790: npcEntry = 15305;                 break;
                    }

                    gameObjTarget->SummonCreature(npcEntry, gameObjTarget->GetPositionX(), gameObjTarget->GetPositionY(), gameObjTarget->GetPositionZ(), gameObjTarget->GetAngle(m_caster), TEMPSPAWN_TIMED_OOC_OR_DEAD_DESPAWN, MINUTE * IN_MILLISECONDS);
                    gameObjTarget->SetLootState(GO_JUST_DEACTIVATED);
                    break;
                }
            }
            break;
        }
        default:
            sLog.outError("Spell::EffectActivateObject called with unknown misc value. Spell Id %u", m_spellInfo->Id);
            break;
    }
}

void Spell::EffectSummonTotem(SpellEffectIndex eff_idx)
{
    int slot = 0;
    switch (m_spellInfo->Effect[eff_idx])
    {
        case SPELL_EFFECT_SUMMON_TOTEM:       slot = TOTEM_SLOT_NONE;  break;
        case SPELL_EFFECT_SUMMON_TOTEM_SLOT1: slot = TOTEM_SLOT_FIRE;  break;
        case SPELL_EFFECT_SUMMON_TOTEM_SLOT2: slot = TOTEM_SLOT_EARTH; break;
        case SPELL_EFFECT_SUMMON_TOTEM_SLOT3: slot = TOTEM_SLOT_WATER; break;
        case SPELL_EFFECT_SUMMON_TOTEM_SLOT4: slot = TOTEM_SLOT_AIR;   break;
        default: return;
    }

    // unsummon old totem
    if (slot < MAX_TOTEM_SLOT)
        if (Totem* OldTotem = m_caster->GetTotem(TotemSlot(slot)))
            OldTotem->UnSummon();

    // FIXME: Setup near to finish point because GetObjectBoundingRadius set in Create but some Create calls can be dependent from proper position
    // if totem have creature_template_addon.auras with persistent point for example or script call
    float angle = slot < MAX_TOTEM_SLOT ? M_PI_F / MAX_TOTEM_SLOT - (slot * 2 * M_PI_F / MAX_TOTEM_SLOT) : 0;
    float x, y, z;
    m_targets.getDestination(x, y, z);
    CreatureCreatePos pos(m_caster->GetMap(), x, y, z, m_caster->GetOrientation());

    CreatureInfo const* cinfo = ObjectMgr::GetCreatureTemplate(m_spellInfo->EffectMiscValue[eff_idx]);
    if (!cinfo)
    {
        sLog.outErrorDb("Creature entry %u does not exist but used in spell %u totem summon.", m_spellInfo->Id, m_spellInfo->EffectMiscValue[eff_idx]);
        return;
    }

    Totem* pTotem = new Totem;

    if (!pTotem->Create(m_caster->GetMap()->GenerateLocalLowGuid(HIGHGUID_UNIT), pos, cinfo, m_caster))
    {
        delete pTotem;
        return;
    }

    pTotem->SetRespawnCoord(pos);

    if (slot < MAX_TOTEM_SLOT)
        m_caster->_AddTotem(TotemSlot(slot), pTotem);

    // pTotem->SetName("");                                 // generated by client
    pTotem->SetOwnerGuid(m_caster->GetObjectGuid());
    pTotem->setFaction(m_caster->getFaction());
    pTotem->SetLevel(m_caster->getLevel());
    pTotem->SetTypeBySummonSpell(m_spellInfo);              // must be after Create call where m_spells initialized

    pTotem->SetDuration(m_duration);

    if (damage)                                             // if not spell info, DB values used
    {
        pTotem->SetMaxHealth(damage);
        pTotem->SetHealth(damage);
    }

    pTotem->SetUInt32Value(UNIT_CREATED_BY_SPELL, m_spellInfo->Id);

    if (m_caster->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED))
        pTotem->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED);

    if (m_caster->IsImmuneToNPC())
        pTotem->SetImmuneToNPC(true);

    if (m_caster->IsImmuneToPlayer())
        pTotem->SetImmuneToPlayer(true);

    if (m_caster->IsPvP())
        pTotem->SetPvP(true);

    pTotem->Summon(m_caster);
    m_spellLog.AddLog(uint32(SPELL_EFFECT_SUMMON), pTotem->GetPackGUID());
}

void Spell::EffectSummonPossessed(SpellEffectIndex eff_idx)
{
    uint32 creatureEntry = m_spellInfo->EffectMiscValue[eff_idx];
    CreatureInfo const* cInfo = ObjectMgr::GetCreatureTemplate(creatureEntry);
    if (!cInfo)
    {
        sLog.outErrorDb("Spell::DoSummonPossessed: creature entry %u not found for spell %u.", creatureEntry, m_spellInfo->Id);
        return;
    }

    Unit* newUnit = m_caster->TakePossessOf(m_spellInfo, eff_idx, m_targets.m_destX, m_targets.m_destY, m_targets.m_destZ, m_caster->GetOrientation());
    if (!newUnit)
    {
        sLog.outError("Spell::DoSummonPossessed: creature entry %u for spell %u could not be summoned.", creatureEntry, m_spellInfo->Id);
        return;
    }

    // Notify Summoner
    if (m_originalCaster && m_originalCaster != m_caster && m_originalCaster->AI())
        m_originalCaster->AI()->JustSummoned((Creature*)newUnit);
}

void Spell::EffectEnchantHeldItem(SpellEffectIndex eff_idx)
{
    // this is only item spell effect applied to main-hand weapon of target player (players in area)
    if (!unitTarget || unitTarget->GetTypeId() != TYPEID_PLAYER)
        return;

    Player* item_owner = (Player*)unitTarget;
    Item* item = item_owner->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);

    if (!item)
        return;

    // must be equipped
    if (!item ->IsEquipped())
        return;

    if (m_spellInfo->EffectMiscValue[eff_idx])
    {
        uint32 enchant_id = m_spellInfo->EffectMiscValue[eff_idx];
        int32 duration = m_duration;                        // Try duration index first...
        if (!duration)
            duration = m_currentBasePoints[eff_idx];        // Base points after...
        if (!duration)
            duration = 10;                                  // 10 seconds for enchants which don't have listed duration

        SpellItemEnchantmentEntry const* pEnchant = sSpellItemEnchantmentStore.LookupEntry(enchant_id);
        if (!pEnchant)
            return;

        // Always go to temp enchantment slot
        EnchantmentSlot slot = TEMP_ENCHANTMENT_SLOT;

        // Enchantment will not be applied if a different one already exists
        if (item->GetEnchantmentId(slot) && item->GetEnchantmentId(slot) != enchant_id)
            return;

        // Apply the temporary enchantment
        item->SetEnchantment(slot, enchant_id, duration * IN_MILLISECONDS, 0);

        // Improved Weapon Totems
        if (m_spellInfo->IsFitToFamilyMask(0x0000000004000000)) // Flametongue totem
        {
            SpellAuraHolder* holder = m_caster->GetOwner()->GetSpellAuraHolder(29192);
            if (!holder)
                holder = m_caster->GetOwner()->GetSpellAuraHolder(29193);
            if (holder && holder->m_auras[0] && holder->GetSpellProto())
                item->SetEnchantmentModifier(new SpellModifier(SPELLMOD_ATTACK_POWER, SPELLMOD_PCT, holder->m_auras[1]->GetModifier()->m_amount, holder->GetId(), uint64(0x00400000000)));
        }
        if (m_spellInfo->IsFitToFamilyMask(0x0000000200000000)) // Windfury totem
        {
            SpellAuraHolder* holder = m_caster->GetOwner()->GetSpellAuraHolder(29192);
            if (!holder)
                holder = m_caster->GetOwner()->GetSpellAuraHolder(29193);
            if (holder && holder->m_auras[0] && holder->GetSpellProto())
                item->SetEnchantmentModifier(new SpellModifier(SPELLMOD_ATTACK_POWER, SPELLMOD_PCT, holder->m_auras[0]->GetModifier()->m_amount, holder->GetId(), uint64(0x00200000000)));
        }
        item_owner->ApplyEnchantment(item, slot, true);
    }
}

void Spell::EffectDisEnchant(SpellEffectIndex /*eff_idx*/)
{
    if (m_caster->GetTypeId() != TYPEID_PLAYER)
        return;

    Player* p_caster = (Player*)m_caster;
    if (!itemTarget || !itemTarget->GetProto()->DisenchantID)
        return;

    p_caster->UpdateCraftSkill(m_spellInfo->Id);

    Loot*& loot = itemTarget->loot;
    if (!loot)
        loot = new Loot(p_caster, itemTarget, LOOT_DISENCHANTING);
    else
    {
        if (loot->GetLootType() != LOOT_DISENCHANTING)
        {
            delete loot;
            loot = new Loot(p_caster, itemTarget, LOOT_DISENCHANTING);
        }
    }

    loot->ShowContentTo(p_caster);

    // item will be removed at disenchanting end
}

void Spell::EffectInebriate(SpellEffectIndex /*eff_idx*/)
{
    if (!unitTarget || unitTarget->GetTypeId() != TYPEID_PLAYER)
        return;

    Player* player = (Player*)unitTarget;
    uint16 currentDrunk = player->GetDrunkValue();
    uint16 drunkMod = damage * 256;
    if (currentDrunk + drunkMod > 0xFFFF)
        currentDrunk = 0xFFFF;
    else
        currentDrunk += drunkMod;
    player->SetDrunkValue(currentDrunk, m_CastItem ? m_CastItem->GetEntry() : 0);
}

void Spell::EffectFeedPet(SpellEffectIndex eff_idx)
{
    if (m_caster->GetTypeId() != TYPEID_PLAYER)
        return;

    Player* _player = (Player*)m_caster;

    Item* foodItem = itemTarget;
    if (!foodItem)
        return;

    Pet* pet = _player->GetPet();
    if (!pet)
        return;

    if (!pet->isAlive())
        return;

    int32 benefit = pet->GetCurrentFoodBenefitLevel(foodItem->GetProto()->ItemLevel);
    if (benefit <= 0)
        return;

    m_spellLog.AddLog(uint32(SPELL_EFFECT_FEED_PET), foodItem->GetEntry());
    // send log now before remove it from player inventory
    m_spellLog.SendToSet();

    uint32 count = 1;
    _player->DestroyItemCount(foodItem, count, true);
    // TODO: fix crash when a spell has two effects, both pointed at the same item target

    m_caster->CastCustomSpell(nullptr, m_spellInfo->EffectTriggerSpell[eff_idx], &benefit, nullptr, nullptr, TRIGGERED_OLD_TRIGGERED);
}

void Spell::EffectDismissPet(SpellEffectIndex /*eff_idx*/)
{
    if (m_caster->GetTypeId() != TYPEID_PLAYER)
        return;

    Pet* pet = m_caster->GetPet();

    // not let dismiss dead pet
    if (!pet || !pet->isAlive())
        return;

    m_spellLog.AddLog(uint32(SPELL_EFFECT_DISMISS_PET), pet->GetObjectGuid());
    // send log now before remove it from map to avoid "unknown" name
    m_spellLog.SendToSet();

    pet->Unsummon(PET_SAVE_NOT_IN_SLOT, m_caster);
}

void Spell::EffectSummonObject(SpellEffectIndex eff_idx)
{
    uint32 go_id = m_spellInfo->EffectMiscValue[eff_idx];

    uint8 slot = 0;
    switch (m_spellInfo->Effect[eff_idx])
    {
        case SPELL_EFFECT_SUMMON_OBJECT_SLOT1: slot = 0; break;
        case SPELL_EFFECT_SUMMON_OBJECT_SLOT2: slot = 1; break;
        case SPELL_EFFECT_SUMMON_OBJECT_SLOT3: slot = 2; break;
        case SPELL_EFFECT_SUMMON_OBJECT_SLOT4: slot = 3; break;
        default: return;
    }

    if (ObjectGuid guid = m_caster->m_ObjectSlotGuid[slot])
    {
        if (GameObject* obj = m_caster ? m_caster->GetMap()->GetGameObject(guid) : nullptr)
            obj->SetLootState(GO_JUST_DEACTIVATED);
        m_caster->m_ObjectSlotGuid[slot].Clear();
    }

    GameObject* pGameObj = new GameObject;

    float x, y, z;
    // If dest location if present
    if (m_targets.m_targetMask & TARGET_FLAG_DEST_LOCATION)
        m_targets.getDestination(x, y, z);
    // Summon in random point all other units if location present
    else
        m_caster->GetClosePoint(x, y, z, DEFAULT_WORLD_OBJECT_SIZE);

    Map* map = m_caster->GetMap();
    if (!pGameObj->Create(map->GenerateLocalLowGuid(HIGHGUID_GAMEOBJECT), go_id, map,
                          x, y, z, m_caster->GetOrientation(), 0.0f, 0.0f, 0.0f, 0.0f, GO_ANIMPROGRESS_DEFAULT, GO_STATE_READY))
    {
        delete pGameObj;
        return;
    }

    pGameObj->SetUInt32Value(GAMEOBJECT_LEVEL, m_caster->getLevel());
    pGameObj->SetRespawnTime(m_duration > 0 ? m_duration / IN_MILLISECONDS : 0);
    pGameObj->SetSpellId(m_spellInfo->Id);
    m_caster->AddGameObject(pGameObj);

    map->Add(pGameObj);
    pGameObj->AIM_Initialize();

    WorldPacket data(SMSG_GAMEOBJECT_SPAWN_ANIM_OBSOLETE, 8);
    data << ObjectGuid(pGameObj->GetObjectGuid());
    m_caster->SendMessageToSet(data, true);

    m_caster->m_ObjectSlotGuid[slot] = pGameObj->GetObjectGuid();

    // Notify Summoner
    if (m_originalCaster && (m_originalCaster != m_caster) && (m_originalCaster->AI()))
        m_originalCaster->AI()->JustSummoned(pGameObj);
    else if (m_caster->AI())
        m_caster->AI()->JustSummoned(pGameObj);
}

void Spell::EffectResurrect(SpellEffectIndex /*eff_idx*/)
{
    if (!unitTarget || unitTarget->GetTypeId() != TYPEID_PLAYER)
        return;

    if (unitTarget->isAlive() || !unitTarget->IsInWorld())
        return;

    switch (m_spellInfo->Id)
    {
        case 8342:                                          // Defibrillate (Goblin Jumper Cables) has 33% chance on success
        case 22999:                                         // Defibrillate (Goblin Jumper Cables XL) has 50% chance on success
        {
            uint32 failChance = 0;
            uint32 failSpellId = 0;
            switch (m_spellInfo->Id)
            {
                case 8342:  failChance = 67; failSpellId = 8338;  break;
                case 22999: failChance = 50; failSpellId = 23055; break;
            }

            if (roll_chance_i(failChance))
            {
                if (failSpellId)
                    m_caster->CastSpell(m_caster, failSpellId, TRIGGERED_OLD_TRIGGERED, m_CastItem);
                return;
            }
            break;
        }
        default:
            break;
    }

    Player* pTarget = ((Player*)unitTarget);

    if (pTarget->isRessurectRequested())      // already have one active request
        return;

    uint32 health = pTarget->GetMaxHealth() * damage / 100;
    uint32 mana   = pTarget->GetMaxPower(POWER_MANA) * damage / 100;

    m_spellLog.AddLog(uint32(SPELL_EFFECT_RESURRECT), unitTarget->GetObjectGuid());
    pTarget->setResurrectRequestData(m_caster->GetObjectGuid(), m_caster->GetMapId(), m_caster->GetPositionX(), m_caster->GetPositionY(), m_caster->GetPositionZ(), health, mana);
    SendResurrectRequest(pTarget);
}

void Spell::EffectAddExtraAttacks(SpellEffectIndex /*eff_idx*/)
{
    if (!unitTarget || !unitTarget->isAlive())
        return;

    if (unitTarget->m_extraAttacks)
        return;

    unitTarget->m_extraAttacks = damage;
    m_spellLog.AddLog(uint32(SPELL_EFFECT_ADD_EXTRA_ATTACKS), unitTarget->GetObjectGuid(), damage);
}

void Spell::EffectParry(SpellEffectIndex /*eff_idx*/)
{
    m_caster->SetCanParry(true);
}

void Spell::EffectBlock(SpellEffectIndex /*eff_idx*/)
{
    m_caster->SetCanBlock(true);
}

void Spell::EffectLeapForward(SpellEffectIndex /*eff_idx*/)
{
    if (!unitTarget)
        return;

    float x, y, z;
    m_targets.getDestination(x, y, z);

    float orientation = unitTarget->GetOrientation();

    unitTarget->NearTeleportTo(x, y, z, orientation, unitTarget == m_caster);
}

void Spell::EffectReputation(SpellEffectIndex eff_idx)
{
    if (!unitTarget || unitTarget->GetTypeId() != TYPEID_PLAYER)
        return;

    Player* _player = (Player*)unitTarget;

    int32  rep_change = m_currentBasePoints[eff_idx];
    uint32 faction_id = m_spellInfo->EffectMiscValue[eff_idx];

    FactionEntry const* factionEntry = sFactionStore.LookupEntry(faction_id);

    if (!factionEntry)
        return;

    rep_change = _player->CalculateReputationGain(REPUTATION_SOURCE_SPELL, rep_change, faction_id);

    _player->GetReputationMgr().ModifyReputation(factionEntry, rep_change);
}

void Spell::EffectQuestComplete(SpellEffectIndex eff_idx)
{
    if (!unitTarget || unitTarget->GetTypeId() != TYPEID_PLAYER)
        return;

    uint32 quest_id = m_spellInfo->EffectMiscValue[eff_idx];
    ((Player*)unitTarget)->AreaExploredOrEventHappens(quest_id);
}

void Spell::EffectSelfResurrect(SpellEffectIndex eff_idx)
{
    if (m_caster->GetTypeId() != TYPEID_PLAYER)
        return;
    Player* player = static_cast<Player*>(m_caster);

    uint32 health;
    uint32 mana = 0;

    // flat case
    if (damage < 0)
    {
        health = uint32(-damage);
        mana = m_spellInfo->EffectMiscValue[eff_idx];
    }
    // percent case
    else
    {
        health = uint32(damage / 100.0f * player->GetMaxHealth());
        if (player->GetMaxPower(POWER_MANA) > 0)
            mana = uint32(damage / 100.0f * player->GetMaxPower(POWER_MANA));
    }

    player->ResurrectPlayer(0.0f);

    player->SetHealth(health);
    player->SetPower(POWER_MANA, mana);
    player->SetPower(POWER_RAGE, 0);
    player->SetPower(POWER_ENERGY, player->GetMaxPower(POWER_ENERGY));

    player->SpawnCorpseBones();
}

void Spell::EffectSkinning(SpellEffectIndex /*eff_idx*/)
{
    if (unitTarget->GetTypeId() != TYPEID_UNIT)
        return;
    if (!m_caster || m_caster->GetTypeId() != TYPEID_PLAYER)
        return;

    Creature* creature = (Creature*)unitTarget;
    int32 targetLevel = creature->getLevel();

    uint32 skill = creature->GetCreatureInfo()->GetRequiredLootSkill();

    Loot*& loot = unitTarget->loot;

    if (loot)
    {
        if (loot->GetLootType() != LOOT_SKINNING)
        {
            delete loot;
            loot = nullptr;
        }
    }

    if (!loot)
    {
        loot = new Loot((Player*)m_caster, creature, LOOT_SKINNING);

        int32 reqValue = targetLevel < 10 ? 0 : targetLevel < 20 ? (targetLevel - 10) * 10 : targetLevel * 5;

        int32 skillValue = ((Player*)m_caster)->GetSkillValuePure(skill);

        // Double chances for elites
        ((Player*)m_caster)->UpdateGatherSkill(skill, skillValue, reqValue, creature->IsElite() ? 2 : 1);
    }

    loot->ShowContentTo((Player*)m_caster);
    creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SKINNABLE);
}

void Spell::EffectCharge(SpellEffectIndex /*eff_idx*/)
{
    if (!unitTarget || !m_caster)
        return;

    WorldLocation pos;
    unitTarget->GetFirstCollisionPosition(pos, unitTarget->GetCombatReach(), unitTarget->GetAngle(m_caster));

    if (unitTarget->GetTypeId() != TYPEID_PLAYER)
        ((Creature*)unitTarget)->StopMoving();

    // charge changes fall time
    if (m_caster->GetTypeId() == TYPEID_PLAYER)
        static_cast<Player*>(m_caster)->SetFallInformation(0, m_caster->GetPositionZ());

    float speed = m_spellInfo->speed ? m_spellInfo->speed : BASE_CHARGE_SPEED;

    // Only send MOVEMENTFLAG_WALK_MODE, client has strange issues with other move flags
    if (m_caster->m_movementInfo.HasMovementFlag(MovementFlags(MOVEFLAG_FALLING | MOVEFLAG_FALLINGFAR)) && (pos.coord_z < m_caster->GetPositionZ()) && (fabs(pos.coord_z - m_caster->GetPositionZ()) > 3.0f))
        m_caster->MonsterMoveWithSpeed(pos.coord_x, pos.coord_y, (pos.coord_z + unitTarget->GetObjectScale()), speed, false, false);
    else
        m_caster->MonsterMoveWithSpeed(pos.coord_x, pos.coord_y, (pos.coord_z + unitTarget->GetObjectScale()), speed, true, true);
}

void Spell::EffectSummonCritter(SpellEffectIndex eff_idx)
{
    uint32 pet_entry = m_spellInfo->EffectMiscValue[eff_idx];
    CreatureInfo const* cInfo = ObjectMgr::GetCreatureTemplate(pet_entry);
    if (!cInfo)
    {
        sLog.outErrorDb("Spell::EffectSummonCritter: creature entry %u not found for spell %u.", pet_entry, m_spellInfo->Id);
        return;
    }

    Pet* old_critter = m_caster->GetMiniPet();

    // for same pet just despawn (player unsummon command)
    if (m_caster->GetTypeId() == TYPEID_PLAYER && old_critter && old_critter->GetEntry() == pet_entry)
    {
        m_caster->RemoveMiniPet();
        return;
    }

    // despawn old pet before summon new
    if (old_critter)
        m_caster->RemoveMiniPet();

    CreatureCreatePos pos(m_caster->GetMap(), m_targets.m_destX, m_targets.m_destY, m_targets.m_destZ, m_caster->GetOrientation());
    if (!(m_targets.m_targetMask & TARGET_FLAG_DEST_LOCATION))
        pos = CreatureCreatePos(m_caster, m_caster->GetOrientation());

    // summon new pet
    Pet* critter = new Pet(MINI_PET);

    Map* map = m_caster->GetMap();
    if (!critter->Create(map->GenerateLocalLowGuid(HIGHGUID_PET), pos, cInfo, pet_entry))
    {
        sLog.outError("Spell::EffectSummonCritter, spellid %u: no such creature entry %u", m_spellInfo->Id, pet_entry);
        delete critter;
        return;
    }

    critter->SetLoading(true);
    critter->SetRespawnCoord(pos);

    // critter->SetName("");                                // generated by client
    critter->SetOwnerGuid(m_caster->GetObjectGuid());
    critter->setFaction(m_caster->getFaction());
    critter->SetUInt32Value(UNIT_CREATED_BY_SPELL, m_spellInfo->Id);
    critter->SelectLevel();                                 // some summoned critters have different from 1 DB data for level/hp
    const CreatureInfo* info = critter->GetCreatureInfo();
    // Some companions have additional UNIT_FLAG_NON_ATTACKABLE (0x2), perphaps coming from template, so add template flags
    critter->SetUInt32Value(UNIT_FIELD_FLAGS, info->UnitFlags);
    critter->SetUInt32Value(UNIT_NPC_FLAGS, info->NpcFlags);// some companions may have quests, so they need npc flags
    critter->InitPetCreateSpells();                         // some companions may have spells (e.g. disgusting oozeling)
    if (m_duration > 0)                                     // set timer for unsummon
        critter->SetDuration(m_duration);

    m_caster->SetMiniPet(critter);

    if (m_caster->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED))
        critter->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED);

    // NOTE: All companions should have these (creatureinfo needs to be tuned accordingly before we can remove these two lines):
    critter->SetImmuneToNPC(true);
    critter->SetImmuneToPlayer(true);
    critter->SetLoading(false);

    // NOTE: Do not set PvP flags (confirmed) for companions.

    critter->SetByteValue(UNIT_FIELD_BYTES_2, 1, UNIT_BYTE2_FLAG_SUPPORTABLE | UNIT_BYTE2_FLAG_UNK5);

    map->Add((Creature*)critter);
    critter->AIM_Initialize();

    //for MINI_PET we ensure to set a passive react
    if (UnitAI* critterAI = critter->AI())
        critterAI->SetReactState(REACT_PASSIVE);

    // Notify Summoner
    if (m_originalCaster && (m_originalCaster != m_caster) && (m_originalCaster->AI()))
        m_originalCaster->AI()->JustSummoned(critter);
    else if (m_caster->AI())
        m_caster->AI()->JustSummoned(critter);
}

void Spell::EffectKnockBack(SpellEffectIndex eff_idx)
{
    if (!unitTarget || unitTarget->GetTypeId() != TYPEID_PLAYER)
        return;

    ((Player*)unitTarget)->KnockBackFrom(m_caster, float(m_spellInfo->EffectMiscValue[eff_idx]) / 10, float(damage) / 10);
}

void Spell::EffectSendTaxi(SpellEffectIndex eff_idx)
{
    if (!unitTarget || unitTarget->GetTypeId() != TYPEID_PLAYER)
        return;

    ((Player*)unitTarget)->ActivateTaxiPathTo(m_spellInfo->EffectMiscValue[eff_idx], m_spellInfo->Id);
}

void Spell::EffectPlayerPull(SpellEffectIndex eff_idx)
{
    if (!unitTarget || unitTarget->GetTypeId() != TYPEID_PLAYER)
        return;

    float dist = unitTarget->GetDistance(m_caster, false);
    if (damage && dist > damage)
        dist = float(damage);

    if (dist < 0.1f)
        return;

    // Projectile motion
    float speedXY = float(m_spellInfo->EffectMiscValue[eff_idx]) * 0.1f;
    float time = dist / speedXY;
    float speedZ = ((m_caster->GetPositionZ() - unitTarget->GetPositionZ()) + 0.5f * time * time * Movement::gravity) / time;

    ((Player*)unitTarget)->KnockBackFrom(m_caster, -speedXY, speedZ);
}

void Spell::EffectDispelMechanic(SpellEffectIndex eff_idx)
{
    if (!unitTarget)
        return;

    uint32 mechanic = m_spellInfo->EffectMiscValue[eff_idx];

    Unit::SpellAuraHolderMap& Auras = unitTarget->GetSpellAuraHolderMap();
    for (Unit::SpellAuraHolderMap::iterator iter = Auras.begin(), next; iter != Auras.end(); iter = next)
    {
        next = iter;
        ++next;
        SpellEntry const* spell = iter->second->GetSpellProto();
        if (iter->second->HasMechanic(mechanic))
        {
            unitTarget->RemoveAurasDueToSpell(spell->Id);
            if (Auras.empty())
                break;
            next = Auras.begin();
        }
    }

    m_spellLog.AddLog(uint32(SPELL_EFFECT_DISPEL_MECHANIC), unitTarget->GetObjectGuid());
}

void Spell::EffectSummonDeadPet(SpellEffectIndex /*eff_idx*/)
{
    if (m_caster->GetTypeId() != TYPEID_PLAYER)
        return;

    Player* _player = static_cast<Player*>(m_caster);
    Pet* pet = _player->GetPet();
    if (!pet)
    {
        pet = new Pet();
        if (!pet->LoadPetFromDB(_player, 0, 0, false, damage))
            delete pet;
        // if above successfully loaded the pet all is done
        return;
    }
    if (pet->isAlive())
        return;

    if (_player->GetDistance(pet) >= 2.0f)
    {
        float x, y, z; // this effect is destination targeted
        m_targets.getDestination(x, y, z);
        pet->NearTeleportTo(x, y, z, -m_caster->GetOrientation());
    }

    pet->SetUInt32Value(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_NONE);
    pet->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SKINNABLE);
    pet->SetDeathState(ALIVE);
    pet->clearUnitState(static_cast<uint32>(UNIT_STAT_ALL_STATE));
    pet->SetHealth(uint32(pet->GetMaxHealth() * (float(damage) / 100)));

    // _player->PetSpellInitialize(); // action bar not removed at death and not required send at revive
    pet->SavePetToDB(PET_SAVE_AS_CURRENT, _player);
}

void Spell::EffectDestroyAllTotems(SpellEffectIndex /*eff_idx*/)
{
    for (int slot = 0;  slot < MAX_TOTEM_SLOT; ++slot)
        if (Totem* totem = m_caster->GetTotem(TotemSlot(slot)))
            totem->UnSummon();
}

void Spell::EffectDurabilityDamage(SpellEffectIndex eff_idx)
{
    if (!unitTarget || unitTarget->GetTypeId() != TYPEID_PLAYER)
        return;

    int32 slot = m_spellInfo->EffectMiscValue[eff_idx];

    // FIXME: some spells effects have value -1/-2
    // Possibly its mean -1 all player equipped items and -2 all items
    if (slot < 0)
    {
        ((Player*)unitTarget)->DurabilityPointsLossAll(damage, (slot < -1));
        m_spellLog.AddLog(uint32(SPELL_EFFECT_DURABILITY_DAMAGE), unitTarget->GetObjectGuid(), -1, damage);
        return;
    }

    // invalid slot value
    if (slot >= INVENTORY_SLOT_BAG_END)
        return;

    if (Item* item = ((Player*)unitTarget)->GetItemByPos(INVENTORY_SLOT_BAG_0, slot))
    {
        m_spellLog.AddLog(uint32(SPELL_EFFECT_DURABILITY_DAMAGE), unitTarget->GetObjectGuid(), item->GetEntry(), damage);
        ((Player*)unitTarget)->DurabilityPointsLoss(item, damage);
    }
}

void Spell::EffectDurabilityDamagePCT(SpellEffectIndex eff_idx)
{
    if (!unitTarget || unitTarget->GetTypeId() != TYPEID_PLAYER)
        return;

    int32 slot = m_spellInfo->EffectMiscValue[eff_idx];

    // FIXME: some spells effects have value -1/-2
    // Possibly its mean -1 all player equipped items and -2 all items
    if (slot < 0)
    {
        ((Player*)unitTarget)->DurabilityLossAll(double(damage) / 100.0f, (slot < -1));
        return;
    }

    // invalid slot value
    if (slot >= INVENTORY_SLOT_BAG_END)
        return;

    if (damage <= 0)
        return;

    if (Item* item = ((Player*)unitTarget)->GetItemByPos(INVENTORY_SLOT_BAG_0, slot))
        ((Player*)unitTarget)->DurabilityLoss(item, double(damage) / 100.0f);
}

void Spell::EffectModifyThreatPercent(SpellEffectIndex /*eff_idx*/)
{
    if (!unitTarget)
        return;

    unitTarget->getThreatManager().modifyThreatPercent(m_caster, damage);
    m_spellLog.AddLog(uint32(SPELL_EFFECT_MODIFY_THREAT_PERCENT), unitTarget->GetObjectGuid());
}

void Spell::EffectTransmitted(SpellEffectIndex eff_idx)
{
    uint32 name_id = m_spellInfo->EffectMiscValue[eff_idx];

    GameObjectInfo const* goinfo = ObjectMgr::GetGameObjectInfo(name_id);

    if (!goinfo)
    {
        sLog.outErrorDb("Gameobject (Entry: %u) not exist and not created at spell (ID: %u) cast", name_id, m_spellInfo->Id);
        return;
    }

    float fx, fy, fz;

    if (m_targets.m_targetMask & TARGET_FLAG_DEST_LOCATION)
        m_targets.getDestination(fx, fy, fz);
    // FIXME: this can be better check for most objects but still hack
    else if (m_spellInfo->EffectRadiusIndex[eff_idx] && m_spellInfo->speed == 0)
    {
        float dis = GetSpellRadius(sSpellRadiusStore.LookupEntry(m_spellInfo->EffectRadiusIndex[eff_idx]));
        m_caster->GetClosePoint(fx, fy, fz, DEFAULT_WORLD_OBJECT_SIZE, dis);
    }
    else
    {
        float min_dis = GetSpellMinRange(sSpellRangeStore.LookupEntry(m_spellInfo->rangeIndex));
        float max_dis = GetSpellMaxRange(sSpellRangeStore.LookupEntry(m_spellInfo->rangeIndex));
        float dis = rand_norm_f() * (max_dis - min_dis) + min_dis;

        // special code for fishing bobber (TARGET_LOCATION_CASTER_FISHING_SPOT), should not try to avoid objects
        // nor try to find ground level, but randomly vary in angle
        if (goinfo->type == GAMEOBJECT_TYPE_FISHINGNODE)
        {
            // calculate angle variation for roughly equal dimensions of target area
            float max_angle = (max_dis - min_dis) / (max_dis + m_caster->GetObjectBoundingRadius());
            float angle_offset = max_angle * (rand_norm_f() - 0.5f);
            m_caster->GetNearPoint2D(fx, fy, dis + m_caster->GetObjectBoundingRadius(), m_caster->GetOrientation() + angle_offset);

            GridMapLiquidData liqData;
            if (!m_caster->GetTerrain()->IsInWater(fx, fy, m_caster->GetPositionZ() + 1.f, &liqData))
            {
                SendCastResult(SPELL_FAILED_NOT_FISHABLE);
                SendChannelUpdate(0);
                return;
            }

            fz = liqData.level;
            // finally, check LoS
            if (!m_caster->IsWithinLOS(fx, fy, fz))
            {
                SendCastResult(SPELL_FAILED_LINE_OF_SIGHT);
                SendChannelUpdate(0);
                return;
            }
        }
        else
            m_caster->GetClosePoint(fx, fy, fz, DEFAULT_WORLD_OBJECT_SIZE, dis);
    }

    Map* cMap = m_caster->GetMap();


    // if gameobject is summoning object, it should be spawned right on caster's position
    if (goinfo->type == GAMEOBJECT_TYPE_SUMMONING_RITUAL)
    {
        m_caster->GetPosition(fx, fy, fz);
    }

    GameObject* pGameObj = new GameObject;

    if (!pGameObj->Create(cMap->GenerateLocalLowGuid(HIGHGUID_GAMEOBJECT), name_id, cMap,
                          fx, fy, fz, m_caster->GetOrientation()))
    {
        delete pGameObj;
        return;
    }

    int32 duration = m_duration;

    switch (goinfo->type)
    {
        case GAMEOBJECT_TYPE_FISHINGNODE:
        {
            m_caster->SetChannelObject(pGameObj);
            m_caster->AddGameObject(pGameObj);              // will removed at spell cancel

            // end time of range when possible catch fish (FISHING_BOBBER_READY_TIME..GetDuration(m_spellInfo))
            // start time == fish-FISHING_BOBBER_READY_TIME (0..GetDuration(m_spellInfo)-FISHING_BOBBER_READY_TIME)
            int32 lastSec = 0;
            switch (urand(0, 3))
            {
                case 0: lastSec =  3; break;
                case 1: lastSec =  7; break;
                case 2: lastSec = 13; break;
                case 3: lastSec = 17; break;
            }

            duration = duration - lastSec * IN_MILLISECONDS + FISHING_BOBBER_READY_TIME * IN_MILLISECONDS;
            break;
        }
        case GAMEOBJECT_TYPE_SUMMONING_RITUAL:
        {
            if (m_caster->GetTypeId() == TYPEID_PLAYER)
            {
                Player* player = (Player*)m_caster;
                pGameObj->AddUniqueUse(player);
                pGameObj->SetActionTarget(player->GetSelectionGuid());
                m_caster->AddGameObject(pGameObj);          // will removed at spell cancel
            }
            break;
        }
        case GAMEOBJECT_TYPE_SPELLCASTER:
        {
            m_caster->AddGameObject(pGameObj);
            break;
        }
        // Other GO types have startOpen attribute but only GAMEOBJECT_TYPE_BUTTON is used by spells
        case GAMEOBJECT_TYPE_BUTTON:
        {
            // Change GO state if it is supposed to be already activated at summoning
            pGameObj->SetGoState((goinfo->button.startOpen ? GO_STATE_ACTIVE : GO_STATE_READY));
            break;
        }
        case GAMEOBJECT_TYPE_FISHINGHOLE:
        case GAMEOBJECT_TYPE_CHEST:
        default:
            break;
    }

    pGameObj->SetRespawnTime(duration > 0 ? duration / IN_MILLISECONDS : 0);

    pGameObj->SetOwnerGuid(m_caster->GetObjectGuid());

    pGameObj->SetUInt32Value(GAMEOBJECT_LEVEL, m_caster->getLevel());
    pGameObj->SetSpellId(m_spellInfo->Id);

    DEBUG_LOG("AddObject at SpellEfects.cpp EffectTransmitted");
    // m_caster->AddGameObject(pGameObj);
    // m_ObjToDel.push_back(pGameObj);

    cMap->Add(pGameObj);
    pGameObj->AIM_Initialize();

    // Notify Summoner
    if (m_originalCaster && (m_originalCaster != m_caster) && (m_originalCaster->AI()))
        m_originalCaster->AI()->JustSummoned(pGameObj);
    else if (m_caster->AI())
        m_caster->AI()->JustSummoned(pGameObj);
}

void Spell::EffectSkill(SpellEffectIndex /*eff_idx*/)
{
    DEBUG_LOG("WORLD: SkillEFFECT");
}

void Spell::EffectSummonDemon(SpellEffectIndex eff_idx)
{
    // ToDo: research what effect Multiple value really does
    //       it is only used by two spells that are triggered by parent spells
    //       which pick every enemy around the original caster and force each target
    //       to cast the summon demon spell

    // Effect base points control how many demons are summoned
    if (!m_currentBasePoints[eff_idx])
        return;

    float x = m_targets.m_destX;
    float y = m_targets.m_destY;
    float z = m_targets.m_destZ;

    for (int i = 0; i < m_currentBasePoints[eff_idx]; i++)
    {
        Creature* Charmed = m_caster->SummonCreature(m_spellInfo->EffectMiscValue[eff_idx], x, y, z, m_caster->GetOrientation(), TEMPSPAWN_TIMED_OR_DEAD_DESPAWN, 3600000);

        if (!Charmed)
            return;

        // might not always work correctly, maybe the creature that dies from CoD casts the effect on itself and is therefore the caster?
        Charmed->SetLevel(m_caster->getLevel());

        // TODO: Add damage/mana/hp according to level

        switch (m_spellInfo->Id)
        {
            case 1122: // Warlock Infernal - requires custom code - generalized in WOTLK
            {
                Charmed->SelectLevel(m_caster->getLevel()); // needs to have casters level
                // Enslave demon effect, without mana cost and cooldown
                Charmed->CastSpell(Charmed, 22707, TRIGGERED_OLD_TRIGGERED);  // short root spell on infernal from sniffs
                m_caster->CastSpell(Charmed, 20882, TRIGGERED_OLD_TRIGGERED);
                Charmed->CastSpell(nullptr, 22699, TRIGGERED_NONE);  // Inferno effect
                Charmed->CastSpell(x, y, z, 20310, TRIGGERED_NONE);  // Stun
                Charmed->CastSpell(nullptr, 19483, TRIGGERED_NONE);  // Immolation - needs to be cast after level update
                break;
            }
        }
    }
}

void Spell::EffectSpiritHeal(SpellEffectIndex /*eff_idx*/)
{
    // TODO player can't see the heal-animation - he should respawn some ticks later
    if (!unitTarget || unitTarget->isAlive())
        return;
    if (unitTarget->GetTypeId() != TYPEID_PLAYER)
        return;
    if (!unitTarget->IsInWorld())
        return;
    if (m_spellInfo->Id == 22012 && !unitTarget->HasAura(2584))
        return;

    ((Player*)unitTarget)->ResurrectPlayer(1.0f);
    ((Player*)unitTarget)->SpawnCorpseBones();
}

// remove insignia spell effect
void Spell::EffectSkinPlayerCorpse(SpellEffectIndex /*eff_idx*/)
{
    DEBUG_LOG("Effect: SkinPlayerCorpse");
    if ((m_caster->GetTypeId() != TYPEID_PLAYER) || (unitTarget->GetTypeId() != TYPEID_PLAYER) || (unitTarget->isAlive()))
        return;

    ((Player*)unitTarget)->RemovedInsignia((Player*)m_caster);
    m_spellLog.AddLog(uint32(SPELL_EFFECT_SKIN_PLAYER_CORPSE), unitTarget->GetObjectGuid());
}

void Spell::EffectBind(SpellEffectIndex /*eff_idx*/)
{
    if (!unitTarget || unitTarget->GetTypeId() != TYPEID_PLAYER)
        return;

    Player* player = (Player*)unitTarget;

    uint32 area_id;
    WorldLocation loc;
    player->GetPosition(loc);
    area_id = player->GetAreaId();

    player->SetHomebindToLocation(loc, area_id);

    // binding
    WorldPacket data(SMSG_BINDPOINTUPDATE, (4 + 4 + 4 + 4 + 4));
    data << float(loc.coord_x);
    data << float(loc.coord_y);
    data << float(loc.coord_z);
    data << uint32(loc.mapid);
    data << uint32(area_id);
    player->SendDirectMessage(data);

    DEBUG_LOG("New Home Position X is %f", loc.coord_x);
    DEBUG_LOG("New Home Position Y is %f", loc.coord_y);
    DEBUG_LOG("New Home Position Z is %f", loc.coord_z);
    DEBUG_LOG("New Home MapId is %u", loc.mapid);
    DEBUG_LOG("New Home AreaId is %u", area_id);

    // zone update
    data.Initialize(SMSG_PLAYERBOUND, 8 + 4);
    data << m_caster->GetObjectGuid();
    data << uint32(area_id);
    player->SendDirectMessage(data);
}

uint32 Spell::GetUsableHealthStoneItemType(Unit* target)
{
    if (!target || target->GetTypeId() != TYPEID_PLAYER)
        return 0;

    uint32 itemtype;
    uint32 rank = 0;
    Unit::AuraList const& mDummyAuras = target->GetAurasByType(SPELL_AURA_DUMMY);
    for (auto mDummyAura : mDummyAuras)
    {
        if (mDummyAura->GetId() == 18692)
        {
            rank = 1;
            break;
        }
        if (mDummyAura->GetId() == 18693)
        {
            rank = 2;
            break;
        }
    }

    static uint32 const itypes[6][3] =
    {
        { 5512, 19004, 19005},              // Minor Healthstone
        { 5511, 19006, 19007},              // Lesser Healthstone
        { 5509, 19008, 19009},              // Healthstone
        { 5510, 19010, 19011},              // Greater Healthstone
        { 9421, 19012, 19013},              // Major Healthstone
    };

    switch (m_spellInfo->Id)
    {
        case  6201:
            itemtype = itypes[0][rank]; break; // Minor Healthstone
        case  6202:
            itemtype = itypes[1][rank]; break; // Lesser Healthstone
        case  5699:
            itemtype = itypes[2][rank]; break; // Healthstone
        case 11729:
            itemtype = itypes[3][rank]; break; // Greater Healthstone
        case 11730:
            itemtype = itypes[4][rank]; break; // Major Healthstone
    }
    return itemtype;
}
