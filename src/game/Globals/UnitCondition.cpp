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

#include "Globals/UnitCondition.h"
#include "Globals/Conditions.h"
#include "Entities/Unit.h"
#include "Database/DatabaseEnv.h"
#include "MotionGenerators/MotionMaster.h"
#include "MotionGenerators/MovementGenerator.h"
#include "Spells/SpellAuras.h"
#include "Util/ProgressBar.h"
#include "Log/Log.h"

std::shared_ptr<std::map<int32, UnitConditionEntry>> UnitConditionMgr::Load()
{
    auto unitConditions = std::make_shared<std::map<int32, UnitConditionEntry>>();

    std::unique_ptr<QueryResult> result(WorldDatabase.Query("SELECT * FROM unit_condition"));
    if (!result)
    {
        sLog.outString(">> Loaded 0 unit_condition definitions - table is empty.");
        sLog.outString();
        m_unitConditions = unitConditions;
        return unitConditions;
    }

    BarGoLink bar(result->GetRowCount());
    uint32 Count = 0;

    do
    {
        bar.step();
        Field* fields = result->Fetch();

        UnitConditionEntry conditionEntry;
        conditionEntry.Id = fields[0].GetUInt32();
        for (uint32 i = 0; i < 8; ++i)
        {
            conditionEntry.Variable[i] = fields[i + 2].GetUInt32();
            if (conditionEntry.Variable[i] >= uint32(UnitCondition::MAX))
            {
                sLog.outErrorDb("Table unit_condition entry %d has invalid variable condition type %u. Setting to None.", conditionEntry.Id, conditionEntry.Variable[i]);
                conditionEntry.Variable[i] = 0;
            }
            conditionEntry.Operation[i] = fields[i + 10].GetUInt32();
            if (conditionEntry.Operation[i] >= uint32(ConditionOperation::MAX))
            {
                sLog.outErrorDb("Table unit_condition entry %d has invalid operation %u. Setting to None.", conditionEntry.Id, conditionEntry.Operation[i]);
                conditionEntry.Operation[i] = 0;
            }
            conditionEntry.Value[i] = fields[i + 18].GetInt32();
        }

        unitConditions->emplace(conditionEntry.Id, conditionEntry);
    }
    while (result->NextRow());

    sLog.outString(">> Loaded %u unit_condition definitions", Count);
    sLog.outString();

    m_unitConditions = unitConditions;

    return unitConditions;
}

bool UnitConditionMgr::Meets(Unit const* source, Unit const* target, int32 Id)
{
    auto unitConditions = source->GetMap()->GetMapDataContainer().GetUnitConditions();
    auto itr = unitConditions->find(Id);
    if (unitConditions->end() == itr)
        return false;

    UnitConditionEntry& entry = itr->second;
    for (uint32 i = 0; i < 8; i++)
    {
        bool result = meets(source, target, UnitCondition(entry.Variable[i]), entry.Operation[i], entry.Value[i]);
        if (result && (entry.Flags & UNIT_CONDITION_FLAG_OR)) // return on first true or
            return true;
        else if (!result && (entry.Flags & UNIT_CONDITION_FLAG_OR) == 0) // return on first false and
            return false;
    }

    if ((entry.Flags & UNIT_CONDITION_FLAG_OR)) // no true reached - fail
        return false;
    else // and - success
        return true;
}

bool UnitConditionMgr::Exists(int32 Id)
{
    return m_unitConditions->find(Id) != m_unitConditions->end();
}

bool UnitConditionMgr::meets(Unit const* source, Unit const* target, UnitCondition conditionType, uint32 operation, int32 value)
{
    int32 condValue = getConditionValue(source, target, conditionType, value);
    return ConditionEntry::CheckOp(ConditionOperation(operation), condValue, value);
}

int32 UnitConditionMgr::getConditionValue(Unit const* source, Unit const* target, UnitCondition conditionType, int32 value)
{
    switch (conditionType)
    {
        case UnitCondition::NONE: return true;
        case UnitCondition::RACE: return source->getRace();
        case UnitCondition::CLASS: return source->getClass();
        case UnitCondition::LEVEL: return source->GetLevel();
        case UnitCondition::IS_SELF: return source == target;
        case UnitCondition::IS_MY_PET: return source->GetObjectGuid() == target->GetMasterGuid();
        case UnitCondition::IS_MASTER: return source->GetMasterGuid() == target->GetObjectGuid();
        case UnitCondition::IS_TARGET: return target->GetTarget() == source;
        case UnitCondition::CAN_ASSIST: return source->CanAssist(target);
        case UnitCondition::CAN_ATTACK: return source->CanAttack(target);
        case UnitCondition::HAS_PET: return source->HasAnyPet();
        case UnitCondition::HAS_WEAPON: return source->hasWeapon(BASE_ATTACK) || source->hasWeapon(OFF_ATTACK);
        case UnitCondition::HEALTH_PERCENT: return source->GetHealthPercent();
        case UnitCondition::MANA_PERCENT: return source->GetPowerPercent(POWER_MANA);
        case UnitCondition::RAGE_PERCENT: return source->GetPowerPercent(POWER_RAGE);
        case UnitCondition::ENERGY_PERCENT: return source->GetPowerPercent(POWER_ENERGY);
        case UnitCondition::COMBO_POINTS: return source->GetComboPoints();
        case UnitCondition::HAS_HELPFUL_AURA_SPELL:
            return source->HasAuraHolder([](SpellAuraHolder* holder)
            {
                return holder->IsPositive();
            }) ? value : 0;
        case UnitCondition::HAS_HELPFUL_AURA_DISPEL_TYPE:
            return source->HasAuraHolder([value](SpellAuraHolder* holder)
            {
                return holder->IsPositive() && holder->GetSpellProto()->Dispel == value;
            }) ? value : 0;
        case UnitCondition::HAS_HELPFUL_AURA_MECHANIC:
            return source->HasAuraHolder([value](SpellAuraHolder* holder)
            {
                return holder->IsPositive() && holder->HasMechanic(value);
            }) ? value : 0;
        case UnitCondition::HAS_HARMFUL_AURA_SPELL:
            return source->HasAuraHolder([](SpellAuraHolder* holder)
            {
                return !holder->IsPositive();
            }) ? value : 0;
        case UnitCondition::HAS_HARMFUL_AURA_DISPEL_TYPE: break;
            return source->HasAuraHolder([value](SpellAuraHolder* holder)
            {
                return !holder->IsPositive() && holder->GetSpellProto()->Dispel == value;
            }) ? value : 0;
        case UnitCondition::HAS_HARMFUL_AURA_MECHANIC:
            return source->HasAuraHolder([value](SpellAuraHolder* holder)
            {
                return !holder->IsPositive() && holder->HasMechanic(value);
            }) ? value : 0;
        case UnitCondition::HAS_HARMFUL_AURA_SCHOOL:
            return source->HasAuraHolder([value](SpellAuraHolder* holder)
            {
                return !holder->IsPositive() && holder->GetSpellProto()->GetSchoolMask() & (1 << value);
            }) ? value : 0;
        case UnitCondition::DAMAGE_PHYSICAL: break;
        case UnitCondition::DAMAGE_HOLY: break;
        case UnitCondition::DAMAGE_FIRE: break;
        case UnitCondition::DAMAGE_NATURE: break;
        case UnitCondition::DAMAGE_FROST: break;
        case UnitCondition::DAMAGE_SHADOW: break;
        case UnitCondition::DAMAGE_ARCANE: break;
        case UnitCondition::IN_COMBAT: return source->IsInCombat();
        case UnitCondition::IS_MOVING: return source->IsMoving();
        case UnitCondition::IS_CASTING: return source->GetCurrentSpell(CURRENT_GENERIC_SPELL) != nullptr;
        case UnitCondition::IS_CASTING_SPELL: return source->GetCurrentSpell(CURRENT_GENERIC_SPELL) != nullptr/* && !source->GetCurrentSpell(CURRENT_GENERIC_SPELL)->m_spellInfo->HasAttribute(SPELL_ATTR_IS_ABILITY)*/;
        case UnitCondition::IS_CHANNELING: return source->GetCurrentSpell(CURRENT_GENERIC_SPELL) != nullptr;
        case UnitCondition::IS_CHANNELING_SPELL: return source->GetCurrentSpell(CURRENT_GENERIC_SPELL) != nullptr/* && !source->GetCurrentSpell(CURRENT_GENERIC_SPELL)->m_spellInfo->HasAttribute(SPELL_ATTR_IS_ABILITY)*/;
        case UnitCondition::NUMBER_OF_MELEE_ATTACKERS:
            if (source->IsCreature())
            {
                if (!source->getThreatManager().isThreatListEmpty())
                {
                    return std::count_if(source->getThreatManager().getThreatList().begin(), source->getThreatManager().getThreatList().end(), [source](HostileReference* ref)
                    {
                        return source->CanReachWithMeleeAttack(ref->getTarget());
                    });
                }
                else
                {
                    return std::count_if(source->getHostileRefManager().begin(), source->getHostileRefManager().end(), [source](const Reference<Unit, ThreatManager>& ref)
                    {
                        return source->CanReachWithMeleeAttack(ref.getTarget());
                    }); 
                }
            }
            else // Experimental: This else block is the only official code from client, but client does not have the above
            {
                return std::count_if(source->getAttackers().begin(), source->getAttackers().end(), [source](Unit* attacker)
                {
                    return source->CanReachWithMeleeAttack(attacker);
                });
            }
        case UnitCondition::IS_ATTACKING_ME: return source->GetTarget() == target;
        case UnitCondition::RANGE: return sqrt(source->GetDistance(target, true, DIST_CALC_NONE));
        case UnitCondition::IN_MELEE_RANGE: return source->CanReachWithMeleeAttack(target);
        case UnitCondition::PURSUIT_TIME: break; // NYI - possible to implement from CombatManager timer
        case UnitCondition::HAS_HARMFUL_AURA_CANCELLED_BY_DAMAGE: return source->HasNegativeAuraWithInterruptFlag(AURA_INTERRUPT_FLAG_DAMAGE);
        case UnitCondition::HAS_HARMFUL_AURA_WITH_PERIODIC_DAMAGE: return source->HasAuraType(SPELL_AURA_PERIODIC_DAMAGE);
        case UnitCondition::NUMBER_OF_ENEMIES: return source->getThreatManager().getThreatList().size();
        case UnitCondition::NUMBER_OF_FRIENDS: break;
        case UnitCondition::THREAT_PHYSICAL: break;
        case UnitCondition::THREAT_HOLY: break;
        case UnitCondition::THREAT_FIRE: break;
        case UnitCondition::THREAT_NATURE: break;
        case UnitCondition::THREAT_FROST: break;
        case UnitCondition::THREAT_SHADOW: break;
        case UnitCondition::THREAT_ARCANE: break;
        case UnitCondition::IS_INTERRUPTIBLE: return source->IsInterruptible();
        case UnitCondition::NUMBER_OF_ATTACKERS: return source->getAttackers().size();
        case UnitCondition::NUMBER_OF_RANGED_ATTACKERS:
            return std::count_if(source->getAttackers().begin(), source->getAttackers().end(), [source](Unit* attacker)
            {
                return !source->CanReachWithMeleeAttack(attacker);
            });
        case UnitCondition::CREATURE_TYPE: return source->GetCreatureType();
        case UnitCondition::IS_MELEE_ATTACKING: return source->GetTarget() && source->CanReachWithMeleeAttack(source->GetTarget());
        case UnitCondition::IS_RANGED_ATTACKING: return source->GetTarget() && !source->CanReachWithMeleeAttack(source->GetTarget());
        case UnitCondition::HEALTH: return source->GetHealth();
        case UnitCondition::SPELL_KNOWN: return source->HasSpell(value) ? value : 0;
        case UnitCondition::HAS_HARMFUL_AURA_EFFECT: return value >= 0 && value < int32(TOTAL_AURAS) && source->HasAura([](Aura* aura)
        {
            return !aura->IsPositive();
        }, AuraType(value)) ? value : 0;
        case UnitCondition::IS_IMMUNE_TO_AREA_OF_EFFECT: return source->IsAOEImmune();
        case UnitCondition::IS_PLAYER: return source->IsPlayer();
        case UnitCondition::DAMAGE_MAGIC: break;
        case UnitCondition::DAMAGE_TOTAL: break;
        case UnitCondition::THREAT_MAGIC: break;
        case UnitCondition::THREAT_TOTAL: break;
        case UnitCondition::HAS_CRITTER: return source->GetMiniPet() != nullptr;
        case UnitCondition::HAS_TOTEM_IN_SLOT_1: return source->GetTotem(TOTEM_SLOT_FIRE) != nullptr;
        case UnitCondition::HAS_TOTEM_IN_SLOT_2: return source->GetTotem(TOTEM_SLOT_EARTH) != nullptr;
        case UnitCondition::HAS_TOTEM_IN_SLOT_3: return source->GetTotem(TOTEM_SLOT_WATER) != nullptr;
        case UnitCondition::HAS_TOTEM_IN_SLOT_4: return source->GetTotem(TOTEM_SLOT_AIR) != nullptr;
        case UnitCondition::HAS_TOTEM_IN_SLOT_5: break;
        case UnitCondition::CREATURE_ID: return source->GetEntry();
        case UnitCondition::STRING_ID: return source->HasStringId(value) ? value : 0;
        case UnitCondition::HAS_AURA: return source->HasAura(value) ? value : 0;
        case UnitCondition::IS_ENEMY: return source->IsEnemy(target);
        case UnitCondition::IS_SPEC_MELEE: break;
        case UnitCondition::IS_SPEC_TANK: break;
        case UnitCondition::IS_SPEC_RANGED: break;
        case UnitCondition::IS_SPEC_HEALER: break;
        case UnitCondition::IS_PLAYER_CONTROLLED_NPC: return source->IsCreature() && source->IsPlayerControlled();
        case UnitCondition::IS_DYING: return source->GetHealth() == 0;
        case UnitCondition::PATH_FAIL_COUNT: return source->GetMotionMaster()->GetCurrent()->IsReachable(); // TODO: Workaround
        case UnitCondition::UNUSED: break;
        case UnitCondition::LABEL: break;
        default: break;
    }
    return false;
}
