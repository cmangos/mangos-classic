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

#include "Globals/Conditions.h"
#include "Entities/Unit.h"
#include "Entities/Player.h"
#include "GameEvents/GameEventMgr.h"
#include "Spells/SpellAuras.h"
#include "World/World.h"
#include "World/WorldState.h"
#include "Maps/InstanceData.h"
#include "Grids/GridNotifiers.h"
#include "Grids/GridNotifiersImpl.h"
#include "Grids/CellImpl.h"
#include "Spells/SpellMgr.h"

// Attention: make sure to keep this list in sync with ConditionSource to avoid array
//            out of bounds access! It is accessed with ConditionSource as index!
char const* conditionSourceToStr[] =
{
    "loot system",                   // CONDITION_FROM_LOOT
    "referencing loot",              // CONDITION_FROM_REFERING_LOOT
    "gossip menu",                   // CONDITION_FROM_GOSSIP_MENU
    "gossip menu option",            // CONDITION_FROM_GOSSIP_OPTION
    "event AI",                      // CONDITION_FROM_EVENTAI
    "hardcoded",                     // CONDITION_FROM_HARDCODED
    "vendor's item check",           // CONDITION_FROM_VENDOR
    "spell_area check",              // CONDITION_FROM_SPELL_AREA
    "npc_spellclick_spells check",   // Unused. For 3.x and later.
    "DBScript engine",               // CONDITION_FROM_DBSCRIPTS
    "trainer's spell check",         // CONDITION_FROM_TRAINER
    "areatrigger teleport check",    // CONDITION_FROM_AREATRIGGER_TELEPORT
    "quest template",                // CONDITION_FROM_QUEST
    "world state"                    // CONDITION_FROM_WORLDSTATE
};

// Stores what params need to be provided to each condition type.
// (source, target, map)
uint8 const ConditionTargetsInternal[] =
{
    CONDITION_REQ_NONE,               // -3
    CONDITION_REQ_NONE,               // -2
    CONDITION_REQ_NONE,               // -1
    CONDITION_REQ_NONE,               //  0
    CONDITION_REQ_TARGET_UNIT,        //  1
    CONDITION_REQ_TARGET_PLAYER,      //  2
    CONDITION_REQ_TARGET_PLAYER,      //  3
    CONDITION_REQ_ANY_WORLDOBJECT,    //  4
    CONDITION_REQ_TARGET_PLAYER_OR_CORPSE, //  5
    CONDITION_REQ_TARGET_PLAYER,      //  6
    CONDITION_REQ_TARGET_PLAYER,      //  7
    CONDITION_REQ_TARGET_PLAYER,      //  8
    CONDITION_REQ_TARGET_PLAYER,      //  9
    CONDITION_REQ_TARGET_PLAYER,      //  10
    CONDITION_REQ_TARGET_PLAYER_OR_CORPSE, //  11
    CONDITION_REQ_NONE,               //  12
    CONDITION_REQ_ANY_WORLDOBJECT,    //  13
    CONDITION_REQ_TARGET_UNIT_OR_CORPSE, //  14
    CONDITION_REQ_TARGET_UNIT,        //  15
    CONDITION_REQ_NONE,               //  16
    CONDITION_REQ_TARGET_PLAYER,      //  17
    CONDITION_REQ_MAP_OR_WORLDOBJECT, //  18
    CONDITION_REQ_TARGET_PLAYER,      //  19
    CONDITION_REQ_NONE,               //  20
    CONDITION_REQ_NONE,               //  21
    CONDITION_REQ_TARGET_PLAYER,      //  22
    CONDITION_REQ_TARGET_PLAYER,      //  23
    CONDITION_REQ_NONE,               //  24
    CONDITION_REQ_NONE,               //  25
    CONDITION_REQ_NONE,               //  26
    CONDITION_REQ_NONE,               //  27
    CONDITION_REQ_TARGET_PLAYER,      //  28
    CONDITION_REQ_TARGET_PLAYER,      //  29
    CONDITION_REQ_TARGET_PLAYER,      //  30
    CONDITION_REQ_MAP_OR_WORLDOBJECT, //  31
    CONDITION_REQ_NONE,               //  32
    CONDITION_REQ_SOURCE_CREATURE,    //  33
    CONDITION_REQ_NONE,               //  34
    CONDITION_REQ_TARGET_UNIT,        //  35
    CONDITION_REQ_NONE,               //  36
    CONDITION_REQ_TARGET_WORLDOBJECT, //  37
    CONDITION_REQ_NONE,               //  38
    CONDITION_REQ_MAP_OR_WORLDOBJECT, //  39
    CONDITION_REQ_NONE,               //  40
    CONDITION_REQ_NONE,               //  41
    CONDITION_REQ_NONE                //  42
};

// Starts from 4th element so that -3 will return first element.
uint8 const* ConditionTargets = &ConditionTargetsInternal[3];

// Checks if player meets the condition
bool ConditionEntry::Meets(WorldObject const* target, Map const* map, WorldObject const* source, ConditionSource conditionSourceType) const
{
    DEBUG_LOG("Condition-System: Check condition %u, type %i - called from %s with params target: %s, map %i, source %s",
              m_entry, m_condition, conditionSourceToStr[conditionSourceType], target ? target->GetGuidStr().c_str() : "<nullptr>", map ? map->GetId() : -1, source ? source->GetGuidStr().c_str() : "<nullptr>");

    if (m_flags & CONDITION_FLAG_SWAP_TARGETS)
        std::swap(source, target);

    if (!CheckParamRequirements(target, map, source))
    {
        sLog.outErrorDb("CONDITION %u type %u used with bad parameters, called from %s, used with target: %s, map %i, source %s",
            m_entry, m_condition, conditionSourceToStr[conditionSourceType], target ? target->GetGuidStr().c_str() : "<nullptr>", map ? map->GetId() : -1, source ? source->GetGuidStr().c_str() : "<nullptr>");
        return false;
    } 

    bool result = Evaluate(target, map, source, conditionSourceType);

    if (m_flags & CONDITION_FLAG_REVERSE_RESULT)
        result = !result;

    return result;
}

bool ConditionEntry::CheckOp(ConditionOperation op, int32 value, int32 operand)
{
    switch (op)
    {
        case ConditionOperation::EQUAL_TO: return value == operand;
        case ConditionOperation::NOT_EQUAL_TO: return value != operand;
        case ConditionOperation::LESS_THAN: return value < operand;
        case ConditionOperation::LESS_THAN_OR_EQUAL_TO: return value <= operand;
        case ConditionOperation::GREATER_THAN: return value > operand;
        case ConditionOperation::GREATER_THAN_OR_EQUAL_TO: return value >= operand;
        default: break;
    }
    return true;
}

// Actual evaluation of the condition done here.
bool inline ConditionEntry::Evaluate(WorldObject const* target, Map const* map, WorldObject const* source, ConditionSource conditionSourceType) const
{
    switch (m_condition)
    {
        case CONDITION_NOT:
        {
            // Checked on load
            return !sConditionStorage.LookupEntry<ConditionEntry>(m_value1)->Meets(target, map, source, conditionSourceType);
        }
        case CONDITION_OR:
        {
            // Third and fourth condition are optional
            if (m_value3 && sConditionStorage.LookupEntry<ConditionEntry>(m_value3)->Meets(target, map, source, conditionSourceType))
                return true;
            if (m_value4 && sConditionStorage.LookupEntry<ConditionEntry>(m_value4)->Meets(target, map, source, conditionSourceType))
                return true;
            
            return sConditionStorage.LookupEntry<ConditionEntry>(m_value1)->Meets(target, map, source, conditionSourceType) || sConditionStorage.LookupEntry<ConditionEntry>(m_value2)->Meets(target, map, source, conditionSourceType);
        }
        case CONDITION_AND:
        {
            // Third and fourth condition are optional
            bool extraConditionsSatisfied = true;
            if (m_value3)
                extraConditionsSatisfied = extraConditionsSatisfied && sConditionStorage.LookupEntry<ConditionEntry>(m_value3)->Meets(target, map, source, conditionSourceType);
            if (m_value4)
                extraConditionsSatisfied = extraConditionsSatisfied && sConditionStorage.LookupEntry<ConditionEntry>(m_value4)->Meets(target, map, source, conditionSourceType);

            return extraConditionsSatisfied && sConditionStorage.LookupEntry<ConditionEntry>(m_value1)->Meets(target, map, source, conditionSourceType) && sConditionStorage.LookupEntry<ConditionEntry>(m_value2)->Meets(target, map, source, conditionSourceType);
        }
        case CONDITION_NONE:
        {
            return true;                                    // empty condition, always met
        }
        case CONDITION_AURA:
        {
            return static_cast<Unit const*>(target)->HasAura(m_value1, SpellEffectIndex(m_value2));
        }
        case CONDITION_ITEM:
        {
            return static_cast<Player const*>(target)->HasItemCount(m_value1, m_value2);
        }
        case CONDITION_ITEM_EQUIPPED:
        {
            return static_cast<Player const*>(target)->HasItemWithIdEquipped(m_value1, 1);
        }
        case CONDITION_AREAID:
        {
            uint32 zone, area;
            WorldObject const* searcher = source ? source : target;
            searcher->GetZoneAndAreaId(zone, area);
            return (zone == m_value1 || area == m_value1) == (m_value2 == 0);
        }
        case CONDITION_REPUTATION_RANK_MIN:
        {
            FactionEntry const* faction = sFactionStore.LookupEntry(m_value1);
            return faction && static_cast<Player const*>(target)->GetReputationMgr().GetRank(faction) >= ReputationRank(m_value2);
        }
        case CONDITION_TEAM:
        {
            if (conditionSourceType == CONDITION_FROM_REFERING_LOOT && sWorld.getConfig(CONFIG_BOOL_ALLOW_TWO_SIDE_INTERACTION_AUCTION))
                return true;
            if (target->IsPlayer())
                return uint32(static_cast<Player const*>(target)->GetTeam()) == m_value1;
            else
                return uint32(static_cast<Corpse const*>(target)->GetTeam()) == m_value1;
        }
        case CONDITION_SKILL:
        {
            Player const* player = static_cast<Player const*>(target);
            return player->HasSkill(m_value1) && player->GetSkillValueBase(m_value1) >= m_value2;
        }
        case CONDITION_QUESTREWARDED:
        {
            return static_cast<Player const*>(target)->GetQuestRewardStatus(m_value1);
        }
        case CONDITION_QUESTTAKEN:
        {
            return static_cast<Player const*>(target)->IsCurrentQuest(m_value1, m_value2);
        }
        case CONDITION_AD_COMMISSION_AURA:
        {
            Unit::SpellAuraHolderMap const& auras = static_cast<Player const*>(target)->GetSpellAuraHolderMap();
            for (const auto& aura : auras)
                if ((aura.second->GetSpellProto()->HasAttribute(SPELL_ATTR_ALLOW_WHILE_MOUNTED) || aura.second->GetSpellProto()->HasAttribute(SPELL_ATTR_IS_ABILITY)) && aura.second->GetSpellProto()->SpellVisual == 3580)
                    return true;
            return false;
        }
        case CONDITION_PVP_RANK:
        {
            uint8 rank = 0;
            if (target->IsPlayer())
                rank = static_cast<Player const*>(target)->GetHonorRankInfo().rank;
            else
                rank = static_cast<Corpse const*>(target)->GetRankSnapshot();
            return rank >= m_value1 && rank <= m_value2;
        }
        case CONDITION_ACTIVE_GAME_EVENT:
        {
            return sGameEventMgr.IsActiveEvent(m_value1);
        }
        case CONDITION_AREA_FLAG:
        {
            WorldObject const* searcher = source ? source : target;
            if (AreaTableEntry const* pAreaEntry = GetAreaEntryByAreaID(searcher->GetAreaId()))
            {
                if ((!m_value1 || (pAreaEntry->flags & m_value1)) && (!m_value2 || !(pAreaEntry->flags & m_value2)))
                    return true;
            }
            return false;
        }
        case CONDITION_RACE_CLASS:
        {
            Unit const* unit = static_cast<Unit const*>(target);
            return ((!m_value1 || (unit->getRaceMask() & m_value1)) && (!m_value2 || (unit->getClassMask() & m_value2)));
        }
        case CONDITION_LEVEL:
        {
            Unit const* unit = static_cast<Unit const*>(target);
            switch (m_value2)
            {
                case 0: return unit->GetLevel() == m_value1;
                case 1: return unit->GetLevel() >= m_value1;
                case 2: return unit->GetLevel() <= m_value1;
            }
            return false;
        }
        case CONDITION_SPELL:
        {
            Player const* player = static_cast<Player const*>(target);
            switch (m_value2)
            {
                case 0: return player->HasSpell(m_value1);
                case 1: return !player->HasSpell(m_value1);
            }
            return false;
        }
        case CONDITION_INSTANCE_SCRIPT:
        {
            Player const* player = target && target->IsPlayer() ? static_cast<Player const*>(target) : nullptr;
            if (!map)
                map = target ? target->GetMap() : source->GetMap();

            if (InstanceData* data = map->GetInstanceData())
                return data->CheckConditionCriteriaMeet(player, m_value1, source, conditionSourceType);
            return false;
        }
        case CONDITION_QUESTAVAILABLE:
        {
            return static_cast<Player const*>(target)->CanTakeQuest(sObjectMgr.GetQuestTemplate(m_value1), false);
        }
        case CONDITION_CREATURE_IN_RANGE:
        {
            Creature* creature = nullptr;

            MaNGOS::NearestCreatureEntryWithLiveStateInObjectRangeCheck creature_check(*target, m_value1, true, false, m_value2, true);
            MaNGOS::CreatureLastSearcher<MaNGOS::NearestCreatureEntryWithLiveStateInObjectRangeCheck> searcher(creature, creature_check);
            Cell::VisitGridObjects(target, searcher, m_value2);

            return creature != nullptr;
        }
        case CONDITION_QUEST_NONE:
        {
            Player const* player = static_cast<Player const*>(target);
            return !player->IsCurrentQuest(m_value1) && !player->GetQuestRewardStatus(m_value1);
        }
        case CONDITION_ITEM_WITH_BANK:
        {
            return static_cast<Player const*>(target)->HasItemCount(m_value1, m_value2, true);
        }
        case CONDITION_ACTIVE_HOLIDAY:
        {
            return sGameEventMgr.IsActiveHoliday(HolidayIds(m_value1));
        }
        case CONDITION_LEARNABLE_ABILITY:
        {
            Player const* player = static_cast<Player const*>(target);

            // Already know the spell
            if (player->HasSpell(m_value1))
                return false;

            // If item defined, check if player has the item already.
            if (m_value2)
            {
                // Hard coded item count. This should be ok, since the intention with this condition is to have
                // a all-in-one check regarding items that learn some ability (primary/secondary tradeskills).
                // Commonly, items like this is unique and/or are not expected to be obtained more than once.
                if (player->HasItemCount(m_value2, 1, true))
                    return false;
            }

            bool isSkillOk = false;

            SkillLineAbilityMapBounds bounds = sSpellMgr.GetSkillLineAbilityMapBoundsBySpellId(m_value1);

            for (SkillLineAbilityMap::const_iterator itr = bounds.first; itr != bounds.second; ++itr)
            {
                const SkillLineAbilityEntry* skillInfo = itr->second;

                if (!skillInfo)
                    continue;

                // doesn't have skill
                if (!player->HasSkill(skillInfo->skillId))
                    return false;

                // doesn't match class
                if (skillInfo->classmask && (skillInfo->classmask & player->getClassMask()) == 0)
                    return false;

                // doesn't match race
                if (skillInfo->racemask && (skillInfo->racemask & player->getRaceMask()) == 0)
                    return false;

                // skill level too low
                if (skillInfo->min_value > player->GetSkillValue(skillInfo->skillId))
                    return false;

                isSkillOk = true;
                break;
            }

            return isSkillOk;
        }
        case CONDITION_GENDER:
        {
            return static_cast<Unit const*>(target)->getGender() == m_value1;
        }
        case CONDITION_SKILL_BELOW:
        {
            Player const* player = static_cast<Player const*>(target);

            if (m_value2 == 1)
                return !player->HasSkill(m_value1);

            return player->HasSkill(m_value1) && player->GetSkillValueBase(m_value1) < m_value2;
        }
        case CONDITION_REPUTATION_RANK_MAX:
        {
            FactionEntry const* faction = sFactionStore.LookupEntry(m_value1);
            Player const* player = static_cast<Player const*>(target);
            return faction && player->GetReputationMgr().GetRank(faction) <= ReputationRank(m_value2);
        }
        case CONDITION_COMPLETED_ENCOUNTER:
        {
            if (!map)
                map = target ? target->GetMap() : source->GetMap();
            if (!map->IsDungeon())
            {
                sLog.outErrorDb("CONDITION_COMPLETED_ENCOUNTER (entry %u) is used outside of a dungeon (on Map %u) by %s", m_entry, target->GetMapId(), target->GetGuidStr().c_str());
                return false;
            }

            uint32 completedEncounterMask = ((DungeonMap*)map)->GetPersistanceState()->GetCompletedEncountersMask();
            DungeonEncounterEntry const* dbcEntry1 = sDungeonEncounterStore.LookupEntry<DungeonEncounterEntry>(m_value1);
            DungeonEncounterEntry const* dbcEntry2 = sDungeonEncounterStore.LookupEntry<DungeonEncounterEntry>(m_value2);
            // Check that on proper map
            if (dbcEntry1->mapId != map->GetId())
            {
                sLog.outErrorDb("CONDITION_COMPLETED_ENCOUNTER (entry %u, DungeonEncounterEntry %u) is used on wrong map (used on Map %u) by %s", m_entry, m_value1, target->GetMapId(), target->GetGuidStr().c_str());
                return false;
            }
            return (completedEncounterMask & ((dbcEntry1 ? 1 << dbcEntry1->encounterIndex : 0) | (dbcEntry2 ? 1 << dbcEntry2->encounterIndex : 0))) != 0;
        }
        case CONDITION_LAST_WAYPOINT:
        {
            uint32 lastReachedWp = ((Creature*)source)->GetMotionMaster()->getLastReachedWaypoint();
            switch (m_value2)
            {
                case 0: return m_value1 == lastReachedWp;
                case 1: return m_value1 <= lastReachedWp;
                case 2: return m_value1 > lastReachedWp;
            }
            return false;
        }
        case CONDITION_DEAD_OR_AWAY:
        {
            // This condition is overcomplicated and doesn't fit neatly into the target checking.
            // Consider splitting it up into 4 separate conditions.
            Player const* player = (target && (target->GetTypeId() == TYPEID_PLAYER)) ? static_cast<Player const*>(target) : nullptr;
            switch (m_value1)
            {
                case 0:                                     // Player dead or out of range
                    return !player || !player->IsAlive() || (m_value2 && source && !source->IsWithinDistInMap(player, m_value2));
                case 1:                                     // All players in Group dead or out of range
                    if (!player)
                        return true;
                    if (Group* grp = ((Player*)player)->GetGroup())
                    {
                        for (GroupReference* itr = grp->GetFirstMember(); itr != nullptr; itr = itr->next())
                        {
                            Player* pl = itr->getSource();
                            if (pl && pl->IsAlive() && !pl->IsGameMaster() && (!m_value2 || !source || source->IsWithinDistInMap(pl, m_value2)))
                                return false;
                        }
                        return true;
                    }
                    return !player->IsAlive() || (m_value2 && source && !source->IsWithinDistInMap(player, m_value2));
                case 2:                                     // All players in instance dead or out of range
                    if (!map && (target || source))
                        map = source ? source->GetMap() : target->GetMap();
                    if (!map || !map->Instanceable())
                    {
                        sLog.outErrorDb("CONDITION_DEAD_OR_AWAY %u (Player in instance case) - called from %s without map param or from non-instanceable map %i", m_entry, conditionSourceToStr[conditionSourceType], map ? map->GetId() : -1);
                        return false;
                    }
                    for (const auto& itr : map->GetPlayers())
                    {
                        Player const* plr = itr.getSource();
                        if (plr && plr->IsAlive() && !plr->IsGameMaster() && (!m_value2 || !source || source->IsWithinDistInMap(plr, m_value2)))
                            return false;
                    }
                    return true;
                case 3:                                     // Creature source is dead
                    return !source || source->GetTypeId() != TYPEID_UNIT || !((Unit*)source)->IsAlive();
            }
            break;
        }
        case CONDITION_PVP_SCRIPT:
        {
            if (target->GetTypeId() != TYPEID_PLAYER)
            {
                sLog.outErrorDb("CONDITION_PVP_SCRIPT (entry %u) is used on non player target. Target is %s", m_entry, target->GetGuidStr().c_str());
                return false;
            }

            Player const* player = static_cast<Player const*>(target);
            if (player->InBattleGround() && player->GetZoneId() == m_value1)
            {
                if (BattleGround* bg = player->GetBattleGround())
                    return bg->IsConditionFulfilled(player, m_value2, source, conditionSourceType);
            }

            return false;
        }
        case CONDITION_SPAWN_COUNT:
        {
            if (!map)
                map = source ? source->GetMap() : target->GetMap();
            return const_cast<Map*>(map)->SpawnedCountForEntry(m_value1) >= m_value2;
        }
        case CONDITION_WORLD_SCRIPT:
        {
            return sWorldState.IsConditionFulfilled(m_value1, m_value2);
        }
        case CONDITION_WORLDSTATE:
        {
            int32 value = map->GetVariableManager().GetVariable(m_value1);
            return CheckOp(ConditionOperation(m_value2), value, m_value3);
        }
        default:
            break;
    }
    return false;
}

// Which params must be provided to a Condition
bool ConditionEntry::CheckParamRequirements(WorldObject const* target, Map const* map, WorldObject const* source) const
{
    switch (ConditionTargets[m_condition])
    {
        case CONDITION_REQ_NONE:
            return true;
        case CONDITION_REQ_TARGET_WORLDOBJECT:
            if (target)
                return true;
            return false;
        case CONDITION_REQ_TARGET_GAMEOBJECT:
            if (target && target->IsGameObject())
                return true;
            return false;
        case CONDITION_REQ_TARGET_UNIT:
            if (target && target->IsUnit())
                return true;
            return false;
        case CONDITION_REQ_TARGET_UNIT_OR_CORPSE:
            if (target && (target->IsUnit() || target->IsCorpse()))
                return true;
            return false;
        case CONDITION_REQ_TARGET_CREATURE:
            if (target && target->IsCreature())
                return true;
            return false;
        case CONDITION_REQ_TARGET_PLAYER:
            if (target && target->IsPlayer())
                return true;
            return false;
        case CONDITION_REQ_TARGET_PLAYER_OR_CORPSE:
            if (target && (target->IsPlayer() || target->IsCorpse()))
                return true;
            return false;
        case CONDITION_REQ_SOURCE_WORLDOBJECT:
            if (source)
                return true;
            return false;
        case CONDITION_REQ_SOURCE_GAMEOBJECT:
            if (source && source->IsGameObject())
                return true;
            return false;
        case CONDITION_REQ_SOURCE_UNIT:
            if (source && source->IsUnit())
                return true;
            return false;
        case CONDITION_REQ_SOURCE_CREATURE:
            if (source && source->IsCreature())
                return true;
            return false;
        case CONDITION_REQ_SOURCE_PLAYER:
            if (source && source->IsPlayer())
                return true;
            return false;
        case CONDITION_REQ_ANY_WORLDOBJECT:
            if (source || target)
                return true;
            return false;
        case CONDITION_REQ_MAP_OR_WORLDOBJECT:
            if (map || source || target)
                return true;
            return false;
        case CONDITION_REQ_BOTH_WORLDOBJECTS:
            if (source && target)
                return true;
            return false;
        case CONDITION_REQ_BOTH_GAMEOBJECTS:
            if (source && source->IsGameObject() && target && target->IsGameObject())
                return true;
            return false;
        case CONDITION_REQ_BOTH_UNITS:
            if (source && source->IsUnit() && target && target->IsUnit())
                return true;
            return false;
        case CONDITION_REQ_BOTH_PLAYERS:
            if (source && source->IsPlayer() && target && target->IsPlayer())
                return true;
            return false;
    }
    
    return true;
}

// Verification of condition values validity
bool ConditionEntry::IsValid() const
{
    switch (m_condition)
    {
        case CONDITION_NOT:
        {
            if (m_value1 >= m_entry)
            {
                sLog.outErrorDb("CONDITION_NOT (entry %u, type %d) has invalid value1 %u, must be lower than entry, skipped", m_entry, m_condition, m_value1);
                return false;
            }
            ConditionEntry const* condition1 = sConditionStorage.LookupEntry<ConditionEntry>(m_value1);
            if (!condition1)
            {
                sLog.outErrorDb("CONDITION_NOT (entry %u, type %d) has value1 %u without proper condition, skipped", m_entry, m_condition, m_value1);
                return false;
            }
            break;
        }
        case CONDITION_OR:
        case CONDITION_AND:
        {
            if (m_value1 >= m_entry)
            {
                sLog.outErrorDb("CONDITION _AND or _OR (entry %u, type %d) has invalid value1 %u, must be lower than entry, skipped", m_entry, m_condition, m_value1);
                return false;
            }
            if (m_value2 >= m_entry)
            {
                sLog.outErrorDb("CONDITION _AND or _OR (entry %u, type %d) has invalid value2 %u, must be lower than entry, skipped", m_entry, m_condition, m_value2);
                return false;
            }
            ConditionEntry const* condition1 = sConditionStorage.LookupEntry<ConditionEntry>(m_value1);
            if (!condition1)
            {
                sLog.outErrorDb("CONDITION _AND or _OR (entry %u, type %d) has value1 %u without proper condition, skipped", m_entry, m_condition, m_value1);
                return false;
            }
            ConditionEntry const* condition2 = sConditionStorage.LookupEntry<ConditionEntry>(m_value2);
            if (!condition2)
            {
                sLog.outErrorDb("CONDITION _AND or _OR (entry %u, type %d) has value2 %u without proper condition, skipped", m_entry, m_condition, m_value2);
                return false;
            }
            if (m_value3)
            {
                if (m_value3 >= m_entry)
                {
                    sLog.outErrorDb("CONDITION _AND or _OR (entry %u, type %d) has invalid value3 %u, must be lower than entry, skipped", m_entry, m_condition, m_value3);
                    return false;
                }
                ConditionEntry const* condition3 = sConditionStorage.LookupEntry<ConditionEntry>(m_value3);
                if (!condition3)
                {
                    sLog.outErrorDb("CONDITION _AND or _OR (entry %u, type %d) has value3 %u without proper condition, skipped", m_entry, m_condition, m_value3);
                    return false;
                }
            }
            if (m_value4)
            {
                if (m_value4 >= m_entry)
                {
                    sLog.outErrorDb("CONDITION _AND or _OR (entry %u, type %d) has invalid value4 %u, must be lower than entry, skipped", m_entry, m_condition, m_value4);
                    return false;
                }
                ConditionEntry const* condition4 = sConditionStorage.LookupEntry<ConditionEntry>(m_value4);
                if (!condition4)
                {
                    sLog.outErrorDb("CONDITION _AND or _OR (entry %u, type %d) has value4 %u without proper condition, skipped", m_entry, m_condition, m_value4);
                    return false;
                }
            }
            break;
        }
        case CONDITION_AURA:
        {
            if (!sSpellTemplate.LookupEntry<SpellEntry>(m_value1))
            {
                sLog.outErrorDb("Aura condition (entry %u, type %u) requires to have non existing spell (Id: %d), skipped", m_entry, m_condition, m_value1);
                return false;
            }
            if (m_value2 >= MAX_EFFECT_INDEX)
            {
                sLog.outErrorDb("Aura condition (entry %u, type %u) requires to have non existing effect index (%u) (must be 0..%u), skipped", m_entry, m_condition, m_value2, MAX_EFFECT_INDEX - 1);
                return false;
            }
            break;
        }
        case CONDITION_ITEM:
        case CONDITION_ITEM_WITH_BANK:
        {
            ItemPrototype const* proto = ObjectMgr::GetItemPrototype(m_value1);
            if (!proto)
            {
                sLog.outErrorDb("Item condition (entry %u, type %u) requires to have non existing item (%u), skipped", m_entry, m_condition, m_value1);
                return false;
            }

            if (m_value2 < 1)
            {
                sLog.outErrorDb("Item condition (entry %u, type %u) useless with count < 1, skipped", m_entry, m_condition);
                return false;
            }
            break;
        }
        case CONDITION_ITEM_EQUIPPED:
        {
            ItemPrototype const* proto = ObjectMgr::GetItemPrototype(m_value1);
            if (!proto)
            {
                sLog.outErrorDb("ItemEquipped condition (entry %u, type %u) requires to have non existing item (%u) equipped, skipped", m_entry, m_condition, m_value1);
                return false;
            }
            break;
        }
        case CONDITION_AREAID:
        {
            AreaTableEntry const* areaEntry = GetAreaEntryByAreaID(m_value1);
            if (!areaEntry)
            {
                sLog.outErrorDb("Zone condition (entry %u, type %u) requires to be in non existing area (%u), skipped", m_entry, m_condition, m_value1);
                return false;
            }

            if (m_value2 > 1)
            {
                sLog.outErrorDb("Zone condition (entry %u, type %u) has invalid argument %u (must be 0..1), skipped", m_entry, m_condition, m_value2);
                return false;
            }
            break;
        }
        case CONDITION_REPUTATION_RANK_MIN:
        case CONDITION_REPUTATION_RANK_MAX:
        {
            FactionEntry const* factionEntry = sFactionStore.LookupEntry(m_value1);
            if (!factionEntry)
            {
                sLog.outErrorDb("Reputation condition (entry %u, type %u) requires to have reputation non existing faction (%u), skipped", m_entry, m_condition, m_value1);
                return false;
            }

            if (m_value2 >= MAX_REPUTATION_RANK)
            {
                sLog.outErrorDb("Reputation condition (entry %u, type %u) has invalid rank requirement (value2 = %u) - must be between %u and %u, skipped", m_entry, m_condition, m_value2, MIN_REPUTATION_RANK, MAX_REPUTATION_RANK - 1);
                return false;
            }
            break;
        }
        case CONDITION_TEAM:
        {
            if (m_value1 != ALLIANCE && m_value1 != HORDE)
            {
                sLog.outErrorDb("Team condition (entry %u, type %u) specifies unknown team (%u), skipped", m_entry, m_condition, m_value1);
                return false;
            }
            break;
        }
        case CONDITION_SKILL:
        case CONDITION_SKILL_BELOW:
        {
            SkillLineEntry const* pSkill = sSkillLineStore.LookupEntry(m_value1);
            if (!pSkill)
            {
                sLog.outErrorDb("Skill condition (entry %u, type %u) specifies non-existing skill (%u), skipped", m_entry, m_condition, m_value1);
                return false;
            }
            if (m_value2 < 1 || m_value2 > sWorld.GetConfigMaxSkillValue())
            {
                sLog.outErrorDb("Skill condition (entry %u, type %u) specifies invalid skill value (%u), skipped", m_entry, m_condition, m_value2);
                return false;
            }
            break;
        }
        case CONDITION_QUESTREWARDED:
        case CONDITION_QUESTTAKEN:
        case CONDITION_QUESTAVAILABLE:
        case CONDITION_QUEST_NONE:
        {
            Quest const* Quest = sObjectMgr.GetQuestTemplate(m_value1);
            if (!Quest)
            {
                sLog.outErrorDb("Quest condition (entry %u, type %u) specifies non-existing quest (%u), skipped", m_entry, m_condition, m_value1);
                return false;
            }

            if (m_value2 && m_condition != CONDITION_QUESTTAKEN)
                sLog.outErrorDb("Quest condition (entry %u, type %u) has useless data in value2 (%u)!", m_entry, m_condition, m_value2);
            break;
        }
        case CONDITION_AD_COMMISSION_AURA:
        {
            if (m_value1)
                sLog.outErrorDb("Quest condition (entry %u, type %u) has useless data in value1 (%u)!", m_entry, m_condition, m_value1);
            if (m_value2)
                sLog.outErrorDb("Quest condition (entry %u, type %u) has useless data in value2 (%u)!", m_entry, m_condition, m_value2);
            break;
        }
        case CONDITION_PVP_RANK:
        {
            if (m_value1 > HONOR_RANK_COUNT)
                sLog.outErrorDb("Quest condition (entry %u, type %u) has too high rank idx in value1 (%u)!", m_entry, m_condition, m_value1);
            if (m_value2 > HONOR_RANK_COUNT)
                sLog.outErrorDb("Quest condition (entry %u, type %u) has too high rank idx in value2 (%u)!", m_entry, m_condition, m_value2);
            break;
        }
        case CONDITION_ACTIVE_GAME_EVENT:
        {
            if (!sGameEventMgr.IsValidEvent(m_value1))
            {
                sLog.outErrorDb("(Not)Active event condition (entry %u, type %u) requires existing event id (%u), skipped", m_entry, m_condition, m_value1);
                return false;
            }
            break;
        }
        case CONDITION_AREA_FLAG:
        {
            if (!m_value1 && !m_value2)
            {
                sLog.outErrorDb("Area flag condition (entry %u, type %u) has both values like 0, skipped", m_entry, m_condition);
                return false;
            }
            break;
        }
        case CONDITION_RACE_CLASS:
        {
            if (!m_value1 && !m_value2)
            {
                sLog.outErrorDb("Race_class condition (entry %u, type %u) has both values like 0, skipped", m_entry, m_condition);
                return false;
            }

            if (m_value1 && !(m_value1 & RACEMASK_ALL_PLAYABLE))
            {
                sLog.outErrorDb("Race_class condition (entry %u, type %u) has invalid player class %u, skipped", m_entry, m_condition, m_value1);
                return false;
            }

            if (m_value2 && !(m_value2 & CLASSMASK_ALL_PLAYABLE))
            {
                sLog.outErrorDb("Race_class condition (entry %u, type %u) has invalid race mask %u, skipped", m_entry, m_condition, m_value2);
                return false;
            }
            break;
        }
        case CONDITION_LEVEL:
        {
            if (!m_value1 || m_value1 > sWorld.getConfig(CONFIG_UINT32_MAX_PLAYER_LEVEL))
            {
                sLog.outErrorDb("Level condition (entry %u, type %u)has invalid level %u, skipped", m_entry, m_condition, m_value1);
                return false;
            }

            if (m_value2 > 2)
            {
                sLog.outErrorDb("Level condition (entry %u, type %u) has invalid argument %u (must be 0..2), skipped", m_entry, m_condition, m_value2);
                return false;
            }

            break;
        }
        case CONDITION_SPELL:
        {
            if (!sSpellTemplate.LookupEntry<SpellEntry>(m_value1))
            {
                sLog.outErrorDb("Spell condition (entry %u, type %u) requires to have non existing spell (Id: %d), skipped", m_entry, m_condition, m_value1);
                return false;
            }

            if (m_value2 > 1)
            {
                sLog.outErrorDb("Spell condition (entry %u, type %u) has invalid argument %u (must be 0..1), skipped", m_entry, m_condition, m_value2);
                return false;
            }

            break;
        }
        case CONDITION_LEARNABLE_ABILITY:
        {
            SkillLineAbilityMapBounds bounds = sSpellMgr.GetSkillLineAbilityMapBoundsBySpellId(m_value1);

            if (bounds.first == bounds.second)
            {
                sLog.outErrorDb("Learnable ability conditon (entry %u, type %u) has spell id %u defined, but this spell is not listed in SkillLineAbility and can not be used, skipping.", m_entry, m_condition, m_value1);
                return false;
            }

            if (m_value2)
            {
                ItemPrototype const* proto = ObjectMgr::GetItemPrototype(m_value2);
                if (!proto)
                {
                    sLog.outErrorDb("Learnable ability conditon (entry %u, type %u) has item entry %u defined but item does not exist, skipping.", m_entry, m_condition, m_value2);
                    return false;
                }
            }

            break;
        }
        case CONDITION_CREATURE_IN_RANGE:
        {
            if (!sCreatureStorage.LookupEntry<CreatureInfo>(m_value1))
            {
                sLog.outErrorDb("Creature in range condition (entry %u, type %u) has an invalid value in value1. (Creature %u does not exist in the database), skipping.", m_entry, m_condition, m_value1);
                return false;
            }
            if (m_value2 <= 0)
            {
                sLog.outErrorDb("Creature in range condition (entry %u, type %u) has an invalid value in value2. (Range %u must be greater than 0), skipping.", m_entry, m_condition, m_value2);
                return false;
            }
            break;
        }
        case CONDITION_COMPLETED_ENCOUNTER:
        {
            DungeonEncounterEntry const* dbcEntry1 = sDungeonEncounterStore.LookupEntry<DungeonEncounterEntry>(m_value1);
            DungeonEncounterEntry const* dbcEntry2 = sDungeonEncounterStore.LookupEntry<DungeonEncounterEntry>(m_value2);
            if (!dbcEntry1)
            {
                sLog.outErrorDb("Completed Encounter condition (entry %u, type %u) has an unknown DungeonEncounter entry %u defined (in value1), skipping.", m_entry, m_condition, m_value1);
                return false;
            }
            if (m_value2 && !dbcEntry2)
            {
                sLog.outErrorDb("Completed Encounter condition (entry %u, type %u) has an unknown DungeonEncounter entry %u defined (in value2), skipping.", m_entry, m_condition, m_value2);
                return false;
            }
            if (dbcEntry2 && dbcEntry1->mapId != dbcEntry2->mapId)
            {
                sLog.outErrorDb("Completed Encounter condition (entry %u, type %u) has different mapIds for both encounters, skipping.", m_entry, m_condition);
                return false;
            }
            break;
        }
        case CONDITION_LAST_WAYPOINT:
        {
            if (m_value2 > 2)
            {
                sLog.outErrorDb("Last Waypoint condition (entry %u, type %u) has an invalid value in value2. (Has %u, supported 0, 1, or 2), skipping.", m_entry, m_condition, m_value2);
                return false;
            }
            break;
        }
        case CONDITION_GENDER:
        {
            if (m_value1 >= MAX_GENDER)
            {
                sLog.outErrorDb("Gender condition (entry %u, type %u) has an invalid value in value1. (Has %u, must be smaller than %u), skipping.", m_entry, m_condition, m_value1, MAX_GENDER);
                return false;
            }
            break;
        }
        case CONDITION_DEAD_OR_AWAY:
        {
            if (m_value1 >= 4)
            {
                sLog.outErrorDb("Dead condition (entry %u, type %u) has an invalid value in value1. (Has %u, must be smaller than 4), skipping.", m_entry, m_condition, m_value1);
                return false;
            }
            break;
        }
        case CONDITION_SPAWN_COUNT:
        {
            if (!sCreatureStorage.LookupEntry<CreatureInfo>(m_value1))
            {
                sLog.outErrorDb("Spawn count condition (entry %u, type %u) has an invalid value in value1. (Creature %u does not exist in the database), skipping.", m_entry, m_condition, m_value1);
                return false;
            }
            break;
        }
        case CONDITION_NONE:
        case CONDITION_INSTANCE_SCRIPT:
        case CONDITION_ACTIVE_HOLIDAY:
        case CONDITION_WORLD_SCRIPT:
            break;
        case CONDITION_WORLDSTATE:
            if (m_value2 > uint32(ConditionOperation::MAX))
            {
                sLog.outErrorDb("Worldstate condition (entry %u, type %u) has invalid sign %u. Skipping.", m_entry, m_condition, m_value2);
                return false;
            }

            // only allow named to avoid not knowing what they are for
            if (!sObjectMgr.HasWorldStateName(m_value1))
            {
                sLog.outErrorDb("Worldstate condition (entry %u, type %u) has has no worldstate name assigned for worldstate %u. Skipping.", m_entry, m_condition, m_value1);
                return false;
            }
            break;
        default:
            sLog.outErrorDb("Condition entry %u has bad type of %d, skipped ", m_entry, m_condition);
            return false;
    }
    return true;
}

// Check if a condition can be used without providing a player param
bool ConditionEntry::CanBeUsedWithoutPlayer(uint32 entry)
{
    ConditionEntry const* condition = sConditionStorage.LookupEntry<ConditionEntry>(entry);
    if (!condition)
        return false;

    switch (condition->m_condition)
    {
        case CONDITION_NOT:
            return CanBeUsedWithoutPlayer(condition->m_value1);
        case CONDITION_AND:
        case CONDITION_OR:
            return CanBeUsedWithoutPlayer(condition->m_value1) && CanBeUsedWithoutPlayer(condition->m_value2);
        default:
            if (ConditionTargets[condition->m_condition] != CONDITION_REQ_TARGET_PLAYER)
                return true;
    }
    return false;
}

bool IsConditionSatisfied(uint32 conditionId, WorldObject const* target, Map const* map, WorldObject const* source, ConditionSource conditionSourceType)
{
    if (ConditionEntry const* condition = sConditionStorage.LookupEntry<ConditionEntry>(conditionId))
        return condition->Meets(target, map, source, conditionSourceType);

    return false;
}