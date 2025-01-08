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

#include "Maps/SpawnGroup.h"

#include "Entities/Creature.h"
#include "Entities/GameObject.h"
#include "Entities/Player.h"
#include "Maps/Map.h"
#include "Maps/SpawnGroupDefines.h"
#include "Maps/MapPersistentStateMgr.h"
#include "Globals/ObjectMgr.h"
#include "MotionGenerators/TargetedMovementGenerator.h"
#include "World/World.h"
#include "Maps/InstanceData.h"

namespace
{
    const char* GetMoveTypeStr(MovementGeneratorType mType)
    {
        switch (mType)
        {
            case IDLE_MOTION_TYPE:                return "IDLE_MOTION_TYPE";
            case RANDOM_MOTION_TYPE:              return "RANDOM_MOTION_TYPE";
            case WAYPOINT_MOTION_TYPE:            return "WAYPOINT_MOTION_TYPE";
            case LINEAR_WP_MOTION_TYPE:           return "LINEAR_WP_MOTION_TYPE";
            case PATH_MOTION_TYPE:                return "PATH_MOTION_TYPE";
            case DISTRACT_MOTION_TYPE:            return "DISTRACT_MOTION_TYPE";
            case STAY_MOTION_TYPE:                return "STAY_MOTION_TYPE";
            case FOLLOW_MOTION_TYPE:              return "FOLLOW_MOTION_TYPE";
            case CHASE_MOTION_TYPE:               return "CHASE_MOTION_TYPE";
            case RETREAT_MOTION_TYPE:             return "RETREAT_MOTION_TYPE";
            case TIMED_FLEEING_MOTION_TYPE:       return "TIMED_FLEEING_MOTION_TYPE";
            case POINT_MOTION_TYPE:               return "POINT_MOTION_TYPE";
            case HOME_MOTION_TYPE:                return "HOME_MOTION_TYPE";
            case FLEEING_MOTION_TYPE:             return "FLEEING_MOTION_TYPE";
            case CONFUSED_MOTION_TYPE:            return "CONFUSED_MOTION_TYPE";
            case EFFECT_MOTION_TYPE:              return "EFFECT_MOTION_TYPE";
            case TAXI_MOTION_TYPE:                return "TAXI_MOTION_TYPE";
            case TIMED_RANDOM_MOTION_TYPE:        return "TIMED_RANDOM_MOTION_TYPE";
            case EXTERNAL_WAYPOINT_MOVE:          return "EXTERNAL_WAYPOINT_MOVE";
            case EXTERNAL_WAYPOINT_MOVE_START:    return "EXTERNAL_WAYPOINT_MOVE_START";
            case EXTERNAL_WAYPOINT_FINISHED_LAST: return "EXTERNAL_WAYPOINT_FINISHED_LAST";
            case FORMATION_MOTION_TYPE:           return "FORMATION_MOTION_TYPE";
            default:                              return "UKNOWN_MOTION_TYPE";
        }
    }
}

SpawnGroup::SpawnGroup(SpawnGroupEntry const& entry, Map& map, uint32 typeId) : m_entry(entry), m_map(map), m_chosenSquad(-1), m_objectTypeId(typeId), m_enabled(m_entry.EnabledByDefault)
{
}

void SpawnGroup::AddObject(uint32 dbGuid, uint32 entry)
{
    m_objects[dbGuid] = entry;
}

void SpawnGroup::RemoveObject(WorldObject* wo)
{
    m_objects.erase(wo->GetDbGuid());

    if (!m_map.IsDungeon() && m_objects.empty())
    {
        if (m_entry.RespawnOverrideMin)
            m_cooldown = m_map.GetCurrentClockTime() + std::chrono::seconds(urand(*m_entry.RespawnOverrideMin, m_entry.RespawnOverrideMax ? *m_entry.RespawnOverrideMax : *m_entry.RespawnOverrideMin));
        else if (m_chosenSquad != -1) // avoid instant respawn artifacts - schedule respawn so full group is around after full wipe
            m_cooldown = m_map.GetCurrentClockTime() + std::chrono::seconds(wo->GetRespawnDelay());

        m_chosenSquad = -1;

        if (m_entry.HasChancedSpawns)
        {
            m_chosenSpawns.clear();
            // save same respawn time for all guids so they all respawn at the same time if chanced
            time_t lastRespawnTime = m_map.GetPersistentState()->GetObjectRespawnTime(GetObjectTypeId(), wo->GetDbGuid());
            for (auto& dbGuid : m_entry.DbGuids)
            {
                if (dbGuid.DbGuid == wo->GetDbGuid())
                    continue;

                m_map.GetPersistentState()->SaveObjectRespawnTime(GetObjectTypeId(), dbGuid.DbGuid, lastRespawnTime);
            }
        }
    }
}

uint32 SpawnGroup::GetGuidEntry(uint32 dbGuid) const
{
    auto itr = m_objects.find(dbGuid);
    if (itr == m_objects.end())
        return 0; // should never happen
    return (*itr).second;
}

void SpawnGroup::Update()
{
    Spawn(false);
}

uint32 SpawnGroup::GetEligibleEntry(std::map<uint32, uint32>& existingEntries, std::map<uint32, uint32>& minEntries)
{
    if (m_entry.RandomEntries.empty())
        return 0;

    if (minEntries.size() > 0)
    {
        auto itr = minEntries.begin();
        std::advance(itr, urand(0, minEntries.size() - 1));
        uint32 entry = (*itr).first;
        return entry;
    }

    if (m_entry.ExplicitlyChanced.size())
    {
        int32 roll = urand(1, 100);
        for (auto explicitly : m_entry.ExplicitlyChanced)
        {
            if (existingEntries[explicitly->Entry] > 0)
            {
                if (roll <= static_cast<int>(explicitly->Chance))
                    return explicitly->Entry;

                roll -= int32(explicitly->Chance);
            }
        }
    }

    if (m_entry.EquallyChanced.empty())
        return 0;

    auto equallyCopy = m_entry.EquallyChanced;
    std::shuffle(equallyCopy.begin(), equallyCopy.end(), *GetRandomGenerator());

    for (auto equally : equallyCopy)
        if (existingEntries[equally->Entry] > 0)
            return equally->Entry;

    return 0;
}

void SpawnGroup::Spawn(bool force)
{
    if (!m_enabled && !force)
        return;

    if (m_cooldown > m_map.GetCurrentClockTime())
        return;

    // duplicated code for optimization - way fewer cond fails
    if ((m_entry.Flags & SPAWN_GROUP_DESPAWN_ON_COND_FAIL) != 0) // must be before count check
    {
        if (!m_objects.empty() && !IsWorldstateConditionSatisfied())
        {
            Despawn(0, 1); // respawn delay 1 to immediately be available for spawn on next condition success
            return;
        }
    }

    if (m_objects.size() >= m_entry.MaxCount)
        return;

    if (!IsWorldstateConditionSatisfied())
        return;

    if (m_entry.HasChancedSpawns && m_chosenSpawns.size() >= m_entry.MaxCount)
        return;

    if (!m_entry.Squads.empty() && m_chosenSquad != -1 && m_entry.Squads[m_chosenSquad].GuidToEntry.size() == m_objects.size())
        return;

    std::vector<SpawnGroupDbGuids const*> eligibleGuids;
    std::map<uint32, uint32> validEntries;
    std::map<uint32, uint32> minEntries;

    auto pickCreatureEntry = [&](const SpawnGroupDbGuids* guids) -> uint32
    {
        uint32 entry;
        // some group members can have static entry, or selfcontained random entry
        if (guids->RandomEntry)
            entry = sObjectMgr.GetRandomCreatureEntry(guids->DbGuid);
        else if (guids->OwnEntry)
            entry = guids->OwnEntry;
        else
            entry = GetEligibleEntry(validEntries, minEntries);

        return entry;
    };

    auto pickGoEntry = [&](const SpawnGroupDbGuids* guids) -> uint32
    {
        uint32 entry;
        // some group members can have static entry, or selfcontained random entry
        if (guids->RandomEntry)
            entry = sObjectMgr.GetRandomGameObjectEntry(guids->DbGuid);
        else if (guids->OwnEntry)
            entry = guids->OwnEntry;
        else
            entry = GetEligibleEntry(validEntries, minEntries);

        return entry;
    };

    auto eraseEntry = [&](uint32 entry)
    {
        if (entry)
        {
            if (validEntries[entry])
                --validEntries[entry];

            auto itr = minEntries.find(entry);
            if (itr != minEntries.end())
            {
                (*itr).second -= 1;
                if ((*itr).second == 0)
                    minEntries.erase(itr);
            }
        }
    };

    if (m_entry.Squads.empty())
    {
        for (auto& guid : m_entry.DbGuids)
            eligibleGuids.push_back(&guid);

        for (auto& randomEntry : m_entry.RandomEntries)
        {
            validEntries[randomEntry.Entry] = randomEntry.MaxCount > 0 ? randomEntry.MaxCount : std::numeric_limits<uint32>::max();
            if (randomEntry.MinCount > 0)
                minEntries.emplace(randomEntry.Entry, randomEntry.MinCount);
        }

        for (auto& data : m_objects)
        {
            eligibleGuids.erase(std::remove_if(eligibleGuids.begin(), eligibleGuids.end(), [dbGuid = data.first](SpawnGroupDbGuids const* entry) { return entry->DbGuid == dbGuid; }), eligibleGuids.end());
            if (validEntries.size() > 0)
            {
                uint32 curCount = validEntries[data.second];
                validEntries[data.second] = curCount > 0 ? curCount - 1 : 0;
            }
            if (minEntries.size() > 0)
            {
                auto itr = minEntries.find(data.second);
                if (itr != minEntries.end())
                {
                    --(*itr).second;
                    if ((*itr).second == 0)
                        minEntries.erase(itr);
                }
            }
        }
    }
    else // squad code
    {
        if (!m_map.IsDungeon() || m_chosenSquad == -1) // choose only on first spawn in dungeon
        {
            auto randIdx = urand(0, m_entry.Squads.size() - 1);
            m_chosenSquad = randIdx;
        }
        auto& chosenSquad = m_entry.Squads[m_chosenSquad];
        for (auto& guid : m_entry.DbGuids)
            if (chosenSquad.GuidToEntry.find(guid.DbGuid) != chosenSquad.GuidToEntry.end())
                eligibleGuids.push_back(&guid);
    }

    time_t now = time(nullptr);
    for (auto itr = eligibleGuids.begin(); itr != eligibleGuids.end();)
    {
        if (m_map.GetPersistentState()->GetObjectRespawnTime(GetObjectTypeId(), (*itr)->DbGuid) > now)
        {
            if (!force)
            {
                if (m_entry.MaxCount == 1) // rare mob case - prevent respawn until all are off CD
                    return;
                itr = eligibleGuids.erase(itr);
                continue;
            }
            else
                m_map.GetPersistentState()->SaveObjectRespawnTime(GetObjectTypeId(), (*itr)->DbGuid, now);
        }
        ++itr;
    }

    // TBC has difficulty block here

    if (m_entry.Squads.empty())
    {
        std::shuffle(eligibleGuids.begin(), eligibleGuids.end(), *GetRandomGenerator());

        for (auto itr = eligibleGuids.begin(); itr != eligibleGuids.end() && !eligibleGuids.empty() && m_objects.size() < m_entry.MaxCount;)
        {
            uint32 dbGuid = (*itr)->DbGuid;
            if (m_entry.HasChancedSpawns)
            {
                if ((*itr)->Chance)
                {
                    auto spawnItr = m_chosenSpawns.find(dbGuid);
                    bool spawn = true;
                    if (spawnItr == m_chosenSpawns.end())
                    {
                        spawn = roll_chance_i((*itr)->Chance);
                        m_chosenSpawns[dbGuid] = spawn;
                    }
                    else
                        spawn = spawnItr->second;

                    if (!spawn)
                    {
                        itr = eligibleGuids.erase(itr);
                        continue;
                    }
                }
                else // filling redundant entries when a group has chanced spawns for optimization so we can stop at start
                    m_chosenSpawns[dbGuid] = true;
            }
            ++itr;
        }

        eligibleGuids.resize(std::min(eligibleGuids.size(), m_entry.MaxCount - m_objects.size())); // now we have final count for processing

        // pick static and random entry first in dungeons so spawn group logic can decide after
        if (m_map.IsDungeon() && GetObjectTypeId() == TYPEID_UNIT)
        {
            for (auto data : eligibleGuids)
            {
                if (data->RandomEntry || data->OwnEntry)
                {
                    uint32 entry = pickCreatureEntry(data);
                    m_chosenEntries[data->DbGuid] = entry;
                    eraseEntry(entry);
                }
            }
            for (auto data : eligibleGuids)
            {
                if (!data->RandomEntry && !data->OwnEntry)
                {
                    uint32 entry = pickCreatureEntry(data);
                    m_chosenEntries[data->DbGuid] = entry;
                    eraseEntry(entry);
                }
            }
        }
    }
    else // squad code
    {
        auto& chosenSquad = m_entry.Squads[m_chosenSquad];
        if (m_map.IsDungeon() && GetObjectTypeId() == TYPEID_UNIT)
            for (auto& squadSpawn : chosenSquad.GuidToEntry)
                m_chosenEntries[squadSpawn.first] = squadSpawn.second;
    }

    for (auto itr = eligibleGuids.begin(); itr != eligibleGuids.end(); ++itr)
    {
        uint32 dbGuid = (*itr)->DbGuid;
        uint32 entry = 0;
        // creatures pick random entry on first spawn in dungeons - else always pick random entry
        if (m_map.IsDungeon() && GetObjectTypeId() == TYPEID_UNIT)
            entry = m_chosenEntries[dbGuid]; // only held in memory - implement saving to db if it becomes a major issue
        else
        {
            if (m_chosenSquad == -1)
            {
                if (GetObjectTypeId() == TYPEID_UNIT)
                    entry = pickCreatureEntry(*itr);
                else // GOs always pick random entry
                    entry = pickGoEntry(*itr);
                eraseEntry(entry);
            }
            else
            {
                auto& chosenSquad = m_entry.Squads[m_chosenSquad];
                entry = chosenSquad.GuidToEntry.find(dbGuid)->second; // always exists
            }
        }   

        float x, y;
        if (GetObjectTypeId() == TYPEID_UNIT)
        {
            auto data = sObjectMgr.GetCreatureData(dbGuid);
            x = data->posX; y = data->posY;
            m_map.GetPersistentState()->AddCreatureToGrid(dbGuid, data);
        }
        else
        {
            auto data = sObjectMgr.GetGOData(dbGuid);
            x = data->posX; y = data->posY;
            m_map.GetPersistentState()->AddGameobjectToGrid(dbGuid, data);
        }
        AddObject(dbGuid, entry);
        if (force || m_entry.Active || m_map.IsLoaded(x, y))
        {
            if (GetObjectTypeId() == TYPEID_UNIT)
                WorldObject::SpawnCreature(dbGuid, &m_map, entry);
            else
                WorldObject::SpawnGameObject(dbGuid, &m_map, entry);
        }
    }
}

bool SpawnGroup::IsWorldstateConditionSatisfied() const
{
    if (m_entry.WorldStateCondition)
    {
        return IsConditionSatisfied(m_entry.WorldStateCondition, nullptr, &m_map, nullptr, CONDITION_FROM_WORLDSTATE);
    }
    else if (m_entry.WorldStateExpression)
    {
        return sObjectMgr.IsWorldStateExpressionSatisfied(m_entry.WorldStateExpression, &m_map);
    }

    return true;
}

void SpawnGroup::RespawnIfInVicinity(Position pos, float range)
{
    if (!IsWorldstateConditionSatisfied())
        return;

    time_t now = time(nullptr);
    bool eligible = false; // if one is eligible, reset whole group
    for (auto& dbGuid : m_entry.DbGuids)
    {
        float x, y;
        if (GetObjectTypeId() == TYPEID_UNIT)
        {
            auto data = sObjectMgr.GetCreatureData(dbGuid.DbGuid);
            x = data->posX; y = data->posY;
        }
        else
        {
            auto data = sObjectMgr.GetGOData(dbGuid.DbGuid);
            x = data->posX; y = data->posY;
        }

        if (pos.GetDistance(Position(x, y, pos.z)) < range * range)
        {
            eligible = true;
            break;
        }
    }

    if (!eligible)
        return;

    for (auto& dbGuid : m_entry.DbGuids)
        m_map.GetPersistentState()->SaveObjectRespawnTime(GetObjectTypeId(), dbGuid.DbGuid, now);
}

bool SpawnGroup::IsRespawnOverriden() const
{
    return m_entry.RespawnOverrideMin.has_value();
}

uint32 SpawnGroup::GetRandomRespawnTime() const
{
    if (!m_entry.RespawnOverrideMax.has_value())
        return *m_entry.RespawnOverrideMin;

    return urand(*m_entry.RespawnOverrideMin, *m_entry.RespawnOverrideMax);
}

std::string SpawnGroup::to_string() const
{
    std::stringstream result;
    for (auto obj : m_objects)
    {
        std::string cData = "";
        auto creature = m_map.GetCreature(obj.first);
        std::string guidStr = "Not found!";
        if (creature)
            guidStr = creature->GetGuidStr();

        result << "[" << obj.first << ", " << obj.second << "] " << guidStr << "\n";
    }
    return result.str();
}


CreatureGroup::CreatureGroup(SpawnGroupEntry const& entry, Map& map) : SpawnGroup(entry, map, uint32(TYPEID_UNIT))
{
    if (entry.formationEntry)
        m_formationData = std::make_shared<FormationData>(this, entry.formationEntry);
    else
        m_formationData = nullptr;
}

void CreatureGroup::RemoveObject(WorldObject* wo)
{
    SpawnGroup::RemoveObject(wo);
    CreatureData const* data = sObjectMgr.GetCreatureData(wo->GetDbGuid());
    m_map.GetPersistentState()->RemoveCreatureFromGrid(wo->GetDbGuid(), data);
    if (m_objects.empty() && wo->GetInstanceData()) // on last being removed
    {
        wo->GetInstanceData()->OnCreatureGroupDespawn(this, static_cast<Creature*>(wo));
        if (GetFormationData())
            GetFormationData()->ResetLastWP();
    }
}

void CreatureGroup::TriggerLinkingEvent(uint32 event, Unit* target)
{
    switch (event)
    {
        case CREATURE_GROUP_EVENT_AGGRO:
            if ((m_entry.Flags & CREATURE_GROUP_AGGRO_TOGETHER) == 0 || !target)
                return;

            // disallow aggro of a group member
            if (!target->HasCharmer() && m_objects.find(target->GetDbGuid()) != m_objects.end())
                return;

            if (m_linkageTargets.find(target) != m_linkageTargets.end()) // already triggering for target
                return;

            m_linkageTargets.insert(target);

            for (auto& data : m_objects)
            {
                uint32 dbGuid = data.first;
                if (Creature* creature = m_map.GetCreature(dbGuid))
                {
                    creature->AddThreat(target);
                    target->AddThreat(creature);
                    target->SetInCombatWith(creature);
                    target->GetCombatManager().TriggerCombatTimer(creature);
                }
            }

            for (uint32 linkedGroup : m_entry.LinkedGroups)
            {
                // ensured on db load that it will be valid fetch
                CreatureGroup* group = static_cast<CreatureGroup*>(m_map.GetSpawnManager().GetSpawnGroup(linkedGroup));
                group->TriggerLinkingEvent(event, target);
            }

            m_linkageTargets.erase(target);
            break;
        case CREATURE_GROUP_EVENT_EVADE:
            if ((m_entry.Flags & CREATURE_GROUP_EVADE_TOGETHER) != 0)
            {
                for (auto& data : m_objects)
                {
                    uint32 dbGuid = data.first;
                    if (Creature* creature = m_map.GetCreature(dbGuid))
                        if (!creature->GetCombatManager().IsEvadingHome())
                            creature->AI()->EnterEvadeMode();
                }
            }
            break;
        case CREATURE_GROUP_EVENT_HOME:
            if (FormationData* formation = GetFormationData())
                if (!IsEvading()) // on last evade complete
                    formation->OnHome();
            [[fallthrough]];
        case CREATURE_GROUP_EVENT_RESPAWN:
            if ((m_entry.Flags & CREATURE_GROUP_RESPAWN_TOGETHER) == 0)
                return;

            ClearRespawnTimes();
            break;
        case CREATURE_GROUP_EVENT_MEMBER_DIED:
            for (auto const& data : m_objects)
            {
                uint32 dbGuid = data.first;
                if (Creature* creature = m_map.GetCreature(dbGuid))
                    creature->AI()->CreatureGroupMemberDied(target);
            }
            break;
    }
}

void CreatureGroup::SetFormationData(FormationEntrySPtr fEntry)
{
    if (fEntry)
    {
        m_formationData = std::make_shared<FormationData>(this, fEntry);
        m_formationData->Reset();
    }
    else
    {
        m_formationData->Disband();
        m_formationData = nullptr;
    }
}

void CreatureGroup::Update()
{
    SpawnGroup::Update();
    if (m_formationData)
        m_formationData->Update();
}

void CreatureGroup::MoveHome()
{
    for (auto objItr : m_objects)
    {
        auto creature = m_map.GetCreature(objItr.first);
        if (creature)
        {
            auto respPos = creature->GetRespawnPosition();
            creature->GetMotionMaster()->MovePoint(0, respPos, FORCED_MOVEMENT_RUN, 0.f, true);
        }
    }
}

void CreatureGroup::Despawn(uint32 timeMSToDespawn, bool onlyAlive, uint32 forcedDespawnTime)
{
    time_t when = time(nullptr) + forcedDespawnTime;
    auto objects = m_objects;
    
    for (auto objItr : objects)
    {
        uint32 dbGuid = objItr.first;
        if (Creature* creature = m_map.GetCreature(dbGuid))
        {
            if (forcedDespawnTime)
                creature->SetRespawnDelay(forcedDespawnTime, true);
            creature->ForcedDespawn(timeMSToDespawn, onlyAlive);
        }
        else if (timeMSToDespawn == 0)
        {
            CreatureData const* data = sObjectMgr.GetCreatureData(dbGuid);
            m_map.GetPersistentState()->RemoveCreatureFromGrid(dbGuid, data);
            m_map.GetPersistentState()->SaveObjectRespawnTime(GetObjectTypeId(), dbGuid, when);
        }
    }
    if (timeMSToDespawn == 0) // only when instant - clears grid unloaded cases
        m_objects.clear();
}

bool CreatureGroup::IsOutOfCombat()
{
    for (auto objItr : m_objects)
    {
        if (Creature* creature = m_map.GetCreature(objItr.first))
        {
            if (!creature->IsAlive())
                continue;

            if (creature->IsInCombat())
                return false;
        }
    }
    return true;
}

bool CreatureGroup::IsEvading()
{
    for (auto objItr : m_objects)
    {
        if (Creature* creature = m_map.GetCreature(objItr.first))
        {
            if (!creature->IsAlive())
                continue;

            if (creature->GetCombatManager().IsEvadingHome())
                return true;
        }
    }
    return false;
}

bool CreatureGroup::HasGroupMember(WorldObject* wo) const
{
    return m_objects.find(wo->GetDbGuid()) != m_objects.end();
}

void CreatureGroup::ClearRespawnTimes()
{
    time_t now = time(nullptr);
    for (auto& data : m_entry.DbGuids)
        m_map.GetPersistentState()->SaveObjectRespawnTime(GetObjectTypeId(), data.DbGuid, now);
}

GameObjectGroup::GameObjectGroup(SpawnGroupEntry const& entry, Map& map) : SpawnGroup(entry, map, uint32(TYPEID_GAMEOBJECT))
{
}

void GameObjectGroup::RemoveObject(WorldObject* wo)
{
    SpawnGroup::RemoveObject(wo);
    GameObjectData const* data = sObjectMgr.GetGOData(wo->GetDbGuid());
    m_map.GetPersistentState()->RemoveGameobjectFromGrid(wo->GetDbGuid(), data);
}

void GameObjectGroup::Despawn(uint32 timeMSToDespawn /*= 0*/, uint32 forcedDespawnTime /*= 0*/)
{
    time_t when = time(nullptr) + forcedDespawnTime;
    auto objects = m_objects;

    for (auto objItr : objects)
    {
        uint32 dbGuid = objItr.first;
        if (GameObject* go = m_map.GetGameObject(dbGuid))
        {
            if (forcedDespawnTime)
                go->SetRespawnDelay(forcedDespawnTime, true);
            go->ForcedDespawn(timeMSToDespawn);
        }
        else if (timeMSToDespawn == 0)
        {
            GameObjectData const* data = sObjectMgr.GetGOData(dbGuid);
            m_map.GetPersistentState()->RemoveGameobjectFromGrid(dbGuid, data);
            m_map.GetPersistentState()->SaveObjectRespawnTime(GetObjectTypeId(), dbGuid, when);
        }
    }
    if (timeMSToDespawn == 0) // only when instant - clears grid unloaded cases
        m_objects.clear();
}

////////////////////
// Formation code //
////////////////////

FormationData::FormationData(CreatureGroup* gData, FormationEntrySPtr fEntry) :
    m_groupData(gData), m_fEntry(fEntry), m_mirrorState(false), m_followerStopped(false), m_masterDied(false), m_lastWP(0), m_wpPathId(0)
{
    for (auto const& sData : m_groupData->GetGroupEntry().DbGuids)
    {
        m_slotsMap.emplace(sData.SlotId, new FormationSlotData(sData.SlotId, sData.DbGuid, m_groupData));

        // save real master guid
        if (sData.SlotId == 0)
            m_realMasterDBGuid = sData.DbGuid;
    }

    m_masterMotionType = static_cast<MovementGeneratorType>(fEntry->MovementType);

    // provided slot id should be ordered with no gap!
    m_slotGuid = m_slotsMap.size();

    // set current value from their defaults
    m_currentFormationShape = m_fEntry->Type;
    m_currentSpread = m_fEntry->Spread;
    m_currentOptions = m_fEntry->Options;
}

FormationData::~FormationData()
{
    sLog.outDebug("Deleting formation (%u)!!!!!", m_groupData->GetGroupEntry().Id);
}

bool FormationData::SetFollowersMaster()
{
    Unit* master = GetMaster();
    if (!master)
    {
        return false;
    }

    for (auto slotItr : m_slotsMap)
    {
        auto& currentSlot = slotItr.second;

        if (currentSlot->IsFormationMaster())
            continue;

        auto follower = currentSlot->GetOwner();

        if (follower && follower->IsAlive())
        {
            bool setMgen = false;
            if (follower->GetMotionMaster()->GetCurrentMovementGeneratorType() != FORMATION_MOTION_TYPE)
                setMgen = true;
            else
            {
                auto mgen = static_cast<FormationMovementGenerator const*>(follower->GetMotionMaster()->GetCurrent());
                if (mgen->GetCurrentTarget() != master)
                    setMgen = true;
            }

            if (setMgen)
            {
                follower->GetMotionMaster()->Clear(false, true);
                follower->GetMotionMaster()->MoveInFormation(currentSlot, true);
                currentSlot->GetRecomputePosition() = true;
            }
        }
    }

    //sLog.outDebug("FormationData::SetFollowersMaste> called for groupId(%u)", m_groupData->GetGroupEntry().Id);

    return false;
}

bool FormationData::SwitchFormation(SpawnGroupFormationType newShape)
{
    if (m_currentFormationShape == newShape)
        return false;

    m_currentFormationShape = newShape;

    FixSlotsPositions();
    return true;
}

void FormationData::SetOptions(uint32 options)
{
    if (HaveOption(SPAWN_GROUP_FORMATION_OPTION_KEEP_CONPACT, m_currentOptions) != HaveOption(SPAWN_GROUP_FORMATION_OPTION_KEEP_CONPACT, options))
    {
        FixSlotsPositions();
    }

    m_currentOptions = options;
}

// remove all creatures from formation data
void FormationData::Disband()
{
    ClearMoveGen();
    for (auto& slotItr : m_slotsMap)
    {
        auto& slot = slotItr.second;

        Unit* slotUnit = slot->GetOwner();
        if (slotUnit)
            slotUnit->SetFormationSlot(nullptr);
    }

    m_slotsMap.clear();
}

// remove all movegen (maybe we should remove only move in formation one)
void FormationData::ClearMoveGen()
{
    for (auto& slotItr : m_slotsMap)
    {
        auto& slot = slotItr.second;

        Unit* slotUnit = slot->GetOwner();
        if (slotUnit && slotUnit->IsAlive())
        {
            if (m_fEntry->IsDynamic && slot->IsFormationMaster())
                continue;
            if (slot->IsFormationMaster())
            {
                // do not change leader movement in dynamic state, script have to handle that
                if (m_fEntry->IsDynamic)
                    continue;

                m_lastWP = slotUnit->GetMotionMaster()->getLastReachedWaypoint();
                m_wpPathId = slotUnit->GetMotionMaster()->GetPathId();
            }
            slotUnit->GetMotionMaster()->Initialize();
        }
    }
}

Unit* FormationData::GetMaster()
{
    if (m_slotsMap.begin() != m_slotsMap.end() && m_slotsMap.begin()->first == 0)
        return m_slotsMap.begin()->second->GetOwner();
    return nullptr;
}

void FormationData::SetMasterMovement()
{
    auto newMaster = m_slotsMap.at(0)->GetOwner();

    if (!newMaster)
        return;

    newMaster->GetMotionMaster()->Clear(true, true);
    if (m_masterMotionType == WAYPOINT_MOTION_TYPE)
    {
        newMaster->GetMotionMaster()->MoveWaypoint(m_fEntry->MovementIdOrWander, PATH_FROM_WAYPOINT_PATH);
        newMaster->GetMotionMaster()->SetNextWaypoint(m_lastWP + 1);
        m_wpPathId = 0;
        m_lastWP = 0;
    }
    else if (m_masterMotionType == LINEAR_WP_MOTION_TYPE)
    {
        newMaster->GetMotionMaster()->MoveLinearWP(m_fEntry->MovementIdOrWander, PATH_FROM_WAYPOINT_PATH);
        newMaster->GetMotionMaster()->SetNextWaypoint(m_lastWP + 1);
        m_wpPathId = 0;
        m_lastWP = 0;
    }
    else if (m_masterMotionType == RANDOM_MOTION_TYPE)
    {
        newMaster->GetMotionMaster()->MoveRandomAroundPoint(m_spawnPos.x, m_spawnPos.y, m_spawnPos.z, m_spawnPos.radius);
    }
    else
    {
        newMaster->GetMotionMaster()->MoveIdle();
    }
}

FormationSlotDataSPtr FormationData::GetFirstEmptySlot()
{
    for (auto slot : m_slotsMap)
    {
        if (slot.second->IsRemovable())
            continue;

        if (!slot.second->GetOwner())
            return slot.second;
    }
    return nullptr;
}

FormationSlotDataSPtr FormationData::GetFirstAliveSlot()
{
    for (auto slot : m_slotsMap)
    {
        if (slot.second->IsRemovable())
            continue;

        if (slot.second->GetOwner() && slot.second->GetOwner()->IsAlive())
            return slot.second;
    }
    return nullptr;
}

bool FormationData::TrySetNewMaster(Unit* masterCandidat /*= nullptr*/)
{
    auto masterSlotItr = m_slotsMap.find(0);
    if (masterSlotItr == m_slotsMap.end())
        return false;

    auto& masterSlot = masterSlotItr->second;

    FormationSlotDataSPtr aliveSlot = nullptr;

    if (masterCandidat && masterCandidat->IsAlive())
    {
        auto candidateSlot = masterCandidat->GetFormationSlot();

        // candidate have to be in this group
        if (candidateSlot && candidateSlot->GetCreatureGroup()->GetGroupId() == m_groupData->GetGroupId())
            aliveSlot = candidateSlot;
    }
    else
    {
        // Get first alive slot
        aliveSlot = GetFirstAliveSlot();
    }

    if (aliveSlot)
    {
        StopFollower();
        SwitchSlotOwner(masterSlot, aliveSlot);
        FixSlotsPositions();

        // will start master movement only if its not dynamic formation
        if (!m_fEntry->IsDynamic)
            SetMasterMovement();

        SetFollowersMaster();
        return true;
    }

    return false;
}

void FormationData::Reset()
{
    m_mirrorState = false;
    m_masterDied = false;

    SwitchFormation(m_fEntry->Type);

    FormationSlotMap::iterator slotItr = m_slotsMap.end();
    for (FormationSlotMap::iterator itr = m_slotsMap.begin(); itr != m_slotsMap.end();)
    {
        auto slot = itr->second;
        if (slot->IsRemovable())
        {
            if (slot->GetOwner())
            {
                slot->GetOwner()->GetMotionMaster()->Initialize();
                slot->GetOwner()->SetFormationSlot(nullptr);
            }
            itr = m_slotsMap.erase(itr);
        }
        else
        {
            if (!slot->GetOwner() || slot->GetOwner()->GetDbGuid() != slot->GetRealOwnerGuid())
            {
                auto creature = m_groupData->GetMap().GetCreature(slot->GetRealOwnerGuid());
                if (creature)
                {
                    if (creature->GetFormationSlot())
                    {
                        if (creature->GetFormationSlot() != slot)
                        {
                            SwitchSlotOwner(slot, creature->GetFormationSlot());
                        }
                    }

                    creature->SetFormationSlot(slot);
                    slot->SetOwner(creature);
                }
                else
                {
                    slot->SetOwner(nullptr);
                }
            }
            ++itr;
        }
    }

    auto masterSlotItr = m_slotsMap.find(0);
    if (masterSlotItr != m_slotsMap.end())
    {
        TrySetNewMaster(masterSlotItr->second->GetOwner());
    }
    StartFollower();
}

void FormationData::OnDeath(Creature* creature)
{
    auto slot = creature->GetFormationSlot();
    if(!slot)
        return;
    //sLog.outString("Deleting %s from formation(%u)", creature->GetGuidStr().c_str(), m_groupData->GetGroupEntry().Id);

    bool formationMaster = false;
    if (slot->IsFormationMaster())
    {
        m_lastWP = creature->GetMotionMaster()->getLastReachedWaypoint();
        m_wpPathId = creature->GetMotionMaster()->GetPathId();

        formationMaster = true;
    }
    slot->SetOwner(nullptr);
    creature->SetFormationSlot(nullptr);

    if (formationMaster)
    {
        if (!m_groupData->IsOutOfCombat()) // must be deferred to arrival home
            m_masterDied = true;
        else
            TrySetNewMaster();
    }
    else if (HaveOption(SPAWN_GROUP_FORMATION_OPTION_KEEP_CONPACT))
        FixSlotsPositions();
}

void FormationData::OnHome()
{
    if (m_masterDied)
    {
        m_masterDied = false;
        TrySetNewMaster();
    }
}

void FormationData::OnDelete(Creature* creature)
{
    // we can handle it like a death event
    OnDeath(creature);
}

// get formation slot id for provided dbGuid, return -1 if not found
int32 FormationData::GetDefaultSlotId(uint32 dbGuid)
{
    for (auto const& entry : m_groupData->GetGroupEntry().DbGuids)
        if (entry.DbGuid == dbGuid)
            return entry.SlotId;
    return -1;
}

FormationSlotDataSPtr FormationData::GetDefaultSlot(uint32 dbGuid, SpawnGroupFormationSlotType slotType /*= SPAWN_GROUP_FORMATION_SLOT_TYPE_STATIC*/)
{
    FormationSlotDataSPtr result = nullptr;
    switch (slotType)
    {
        case SPAWN_GROUP_FORMATION_SLOT_TYPE_STATIC:
        {
            auto slotId = GetDefaultSlotId(dbGuid);
            if (slotId < 0)
                result = nullptr;
            else
                result = m_slotsMap[slotId];
            break;
        }
        case SPAWN_GROUP_FORMATION_SLOT_TYPE_SCRIPT:
        case SPAWN_GROUP_FORMATION_SLOT_TYPE_PLAYER:
            for (auto slotItr : m_slotsMap)
            {
                if (slotItr.second->GetSlotType() != slotType)
                    continue;
    
                if (slotItr.second->GetRealOwnerGuid() == dbGuid)
                {
                    result = slotItr.second;
                    break;
                }
            }
            break;
        default:
            break;
    }

    return result;
}

void FormationData::SwitchSlotOwner(FormationSlotDataSPtr slotA, FormationSlotDataSPtr slotB)
{
    Unit* aUnit = slotA->GetOwner();
    Unit* bUnit = slotB->GetOwner();

    slotA->SetOwner(bUnit);
    if (aUnit)
        aUnit->SetFormationSlot(slotB);

    slotB->SetOwner(aUnit);
    if (bUnit)
        bUnit->SetFormationSlot(slotA);
}

bool FormationData::FreeSlot(FormationSlotDataSPtr slot)
{
    if (!slot->GetOwner())
        return true;

    auto newSlot = GetDefaultSlot(slot->GetOwner()->GetDbGuid());

    if (!newSlot || newSlot == slot)
    {
        newSlot = GetFirstEmptySlot();
        if (!newSlot)
        {
            sLog.outError("FormationData::MoveSlotOwner> Unable to find free place in formation groupID: %u for %s",
                m_groupData->GetGroupId(), slot->GetOwner()->GetGuidStr().c_str());
            return false;
        }
    }

    SwitchSlotOwner(slot, newSlot);
    return true;
}

bool FormationData::AddInFormationSlot(Unit* newUnit, SpawnGroupFormationSlotType slotType /*= SPAWN_GROUP_FORMATION_SLOT_TYPE_STATIC*/)
{
    if (!newUnit || !newUnit->IsAlive())
    {
        sLog.outError("FormationData::AddInFormationSlot> Invalid call detected! (unit is nullptr or not alive)");
        return false;
    }

    Unit* oldUnit = nullptr;

    // TODO:: its normal to not have default slot for dynamically added creature/player to the formation we should add one
    auto slot = GetDefaultSlot(newUnit->GetDbGuid(), slotType);
    if (!slot)
    {
        sLog.outError("FormationData::AddInFormationSlot> Unable to find default slot for %s , is it part of the formation? Aborting...", newUnit->GetGuidStr().c_str());
        return false;
    }

    if (!FreeSlot(slot))
    {
        sLog.outError("FormationData::AddInFormationSlot> Unable to free occupied slot by %s for %s", slot->GetOwner()->GetGuidStr().c_str(), newUnit->GetGuidStr().c_str());
        return false;
    }

    slot->SetOwner(newUnit);
    newUnit->SetFormationSlot(slot);

    //sLog.outString("Slot(%u) filled by %s in formation(%u)", slot->GetSlotId(), newUnit->GetGuidStr().c_str(), m_groupData->GetGroupEntry().Id);
    return true;
}

void FormationData::StartFollower()
{
    for (auto& slot : m_slotsMap)
        slot.second->SetCanFollow(true);
    m_followerStopped = false;
}

void FormationData::StopFollower()
{
    for (auto& slot : m_slotsMap)
        slot.second->SetCanFollow(false);
    m_followerStopped = true;
    m_followerStartTime = World::GetCurrentClockTime() + std::chrono::seconds(5);
}

void FormationData::Compact(bool set /*= true*/)
{
    if (set)
        m_currentOptions = m_currentOptions | static_cast<uint32>(SPAWN_GROUP_FORMATION_OPTION_KEEP_CONPACT);
    else
        m_currentOptions = m_currentOptions & ~(static_cast<uint32>(SPAWN_GROUP_FORMATION_OPTION_KEEP_CONPACT));

    FixSlotsPositions();
}

void FormationData::Add(Creature* creature)
{
    if (creature->GetFormationSlot())
        return;

    if (creature->IsTemporarySummon())
    {
        sLog.outError("FormationData::Add> unable to add temporary summon creature %s to formation %u", creature->GetGuidStr().c_str(), m_groupData->GetGroupId());
        return;
    }

    if (!creature->IsAlive())
    {
        sLog.outError("FormationData::Add> Cannot add dead creature %s to formation %u", creature->GetGuidStr().c_str(), m_groupData->GetGroupId());
        return;
    }

    SpawnGroupFormationSlotType slotType = SPAWN_GROUP_FORMATION_SLOT_TYPE_SCRIPT;

    // be sure that no slot already exist before creating a new one
    auto defaultSlot = GetDefaultSlot(creature->GetDbGuid(), slotType);
    if (!defaultSlot)
    {
        uint32 slotId = m_slotGuid++;
        auto result = m_slotsMap.emplace(slotId, new FormationSlotData(slotId, creature->GetDbGuid(), m_groupData, slotType));
        if (!result.second)
        {
            sLog.outError("FormationData::Add> Failled to add creature %s to formation %u", creature->GetGuidStr().c_str(), m_groupData->GetGroupId());
            return;
        }
    }

    SetFormationSlot(creature, slotType);
}

void FormationData::Add(Player* player)
{
    if (player->GetFormationSlot())
        return;

    uint32 slotId = m_slotGuid++;
    if (slotId == 0)
    {
        // this should have at least one master
        sLog.outError("FormationData::Add(Player* player)> Unable to add %s to formation %u as master", player->GetGuidStr().c_str(), m_groupData->GetGroupId());
        m_slotGuid = 0;
        return;
    }

    // be sure that no slot already exist before creating a new one
    auto defaultSlot = GetDefaultSlot(player->GetDbGuid(), SPAWN_GROUP_FORMATION_SLOT_TYPE_PLAYER);
    if (!defaultSlot)
    {
        auto result = m_slotsMap.emplace(slotId, new FormationSlotData(slotId, player->GetDbGuid(), m_groupData, SPAWN_GROUP_FORMATION_SLOT_TYPE_PLAYER));
        if (!result.second)
        {
            sLog.outError("FormationData::Add(Player* player)> Failled to add %s to formation %u", player->GetGuidStr().c_str(), m_groupData->GetGroupId());
            return;
        }

        defaultSlot = result.first->second;
    }
    player->SetFormationSlot(defaultSlot);
    player->GetFormationSlot()->SetOwner(player);
    FixSlotsPositions();
    SetFollowersMaster();
}

void FormationData::Remove(Unit* unit)
{
    if (unit->IsPlayer())
    {
        Remove(static_cast<Player*>(unit));
    }
    else if (unit->IsCreature())
        Remove(static_cast<Creature*>(unit));
}

void FormationData::Remove(Creature* creature)
{
    auto slot = creature->GetFormationSlot();
    if (!slot)
        return;

    // check if slot is still owner one
    if (slot->GetRealOwnerGuid() != creature->GetDbGuid())
    {
        FormationSlotMap::iterator slotItr = m_slotsMap.end();
        for (FormationSlotMap::iterator itr = m_slotsMap.begin(); itr != m_slotsMap.end(); ++itr)
        {
            if (itr->second->GetRealOwnerGuid() == creature->GetDbGuid())
            {
                slotItr = itr;
                break;
            }
        }

        if (slotItr == m_slotsMap.end())
        {
            // slot can be just freed like the creature was dead
            OnDeath(creature);
        }
        else
        {
            SwitchSlotOwner(slot, slotItr->second);
            slot = slotItr->second;
        }
    }

    // free the slot
    OnDeath(creature);

    creature->GetMotionMaster()->Initialize();

    if (slot->IsRemovable())
        m_slotsMap.erase(slot->GetSlotId());
    if (HaveOption(SPAWN_GROUP_FORMATION_OPTION_KEEP_CONPACT))
        FixSlotsPositions();
}

void FormationData::Remove(Player* player)
{
    auto slot = player->GetFormationSlot();
    if (!slot)
        return;

    if (slot->GetRealOwnerGuid() != player->GetDbGuid())
    {
        FormationSlotMap::iterator slotItr = m_slotsMap.end();
        for (FormationSlotMap::iterator itr = m_slotsMap.begin(); itr != m_slotsMap.end(); ++itr)
        {
            if (itr->second->GetSlotType() == SPAWN_GROUP_FORMATION_SLOT_TYPE_PLAYER)
                continue;

            if (itr->second->GetRealOwnerGuid() == player->GetDbGuid())
            {
                slotItr = itr;
                break;
            }
        }

        if (slotItr == m_slotsMap.end())
        {
            // slot can be just freed
            slot = nullptr;
        }
        else
        {
            SwitchSlotOwner(slot, slotItr->second);
            slot = slotItr->second;
        }
    }

    if (!slot)
    {
        sLog.outError("FormationData::Remove(Player* player)> Failled to remove player %s from formation %u", player->GetGuidStr().c_str(), m_groupData->GetGroupId());
        return;
    }

    if (slot->IsRemovable())
    {
        // remove the slot
        m_slotsMap.erase(slot->GetSlotId());
    }
    else
        slot->SetOwner(nullptr);

    player->SetFormationSlot(nullptr);
    player->GetMotionMaster()->Initialize();

    if (HaveOption(SPAWN_GROUP_FORMATION_OPTION_KEEP_CONPACT))
        FixSlotsPositions();
}

FormationSlotDataSPtr FormationData::SetFormationSlot(Creature* creature, SpawnGroupFormationSlotType slotType /*= SPAWN_GROUP_FORMATION_SLOT_TYPE_STATIC*/)
{
    if (!creature->IsAlive())
        return nullptr;

    auto const& gEntry = m_groupData->GetGroupEntry();

    if (m_fEntry == nullptr)
        return nullptr;

    uint32 dbGuid = creature->GetDbGuid();

    // check if existing slot
    if (auto currentSlot = creature->GetFormationSlot())
    {
        // no more work to do
        return currentSlot;
    }

    // add it in the corresponding slot
    if (!AddInFormationSlot(creature, slotType))
        return nullptr;

    // set the creature as active to avoid some problem
    //creature->SetActiveObjectState(true); // maybe not needed?

    auto slot = creature->GetFormationSlot();
    if (slot->GetSlotId() == 0)
    {
        m_realMasterDBGuid = dbGuid;
        creature->GetRespawnCoord(m_spawnPos.x, m_spawnPos.y, m_spawnPos.z, nullptr, &m_spawnPos.radius);

        // spawned/respawned master
        SetMasterMovement();

        // reset formation shape as it will restart from node 0 in respawn case
        SwitchFormation(m_fEntry->Type);
    }

    if (GetMaster())
    {
        FixSlotsPositions();
        SetFollowersMaster();
    }
    return slot;
}

std::string FormationData::to_string() const
{
    std::stringstream result;

    static const std::string FormationType[] = {
        "[0]Random",
        "[1]Single file",
        "[2]Side by side",
        "[3]Like geese",
        "[4]Fanned out behind",
        "[5]Fanned out in front",
        "[6]Circle the leader"
    };

    std::string fType = FormationType[static_cast<uint32>(m_currentFormationShape)];
    std::string fMoveType = GetMoveTypeStr(m_masterMotionType);
    std::string fOptions = (HaveOption(SPAWN_GROUP_FORMATION_OPTION_KEEP_CONPACT)) ? ", keepCompact" : "no options";
    result << "Formation group id: " << m_fEntry->GroupId    << "\n";
    result << "Shape: "              << fType                << "\n";
    result << "Spread: "             << m_currentSpread      << "\n";
    result << "MovementType: "       << fMoveType            << "\n";
    result << "MovementId: "         << m_fEntry->MovementIdOrWander << "\n";
    result << "Options: "            << fOptions             << "\n";
    result << "Comment: "            << m_fEntry->Comment    << "\n";

    for (auto slot : m_slotsMap)
    {
        std::string guidStr = "";
        if (slot.second->GetOwner())
            guidStr = slot.second->GetOwner()->GetGuidStr();
        else
            guidStr = "empty slot";

        result << "[" << slot.first << "] " << guidStr << "\n";
    }

    return result.str();
}

// Change movement data so it can resume it if leader change
void FormationData::SetMovementInfo(MovementGeneratorType moveType, uint32 wanderOrPahtId)
{
    m_fEntry->MovementIdOrWander = wanderOrPahtId;
    m_fEntry->MovementType = moveType;
    m_masterMotionType = moveType;
    m_lastWP = 0;
    auto master = GetMaster();
    if (master)
    {
        static_cast<Creature*>(master)->GetRespawnCoord(m_spawnPos.x, m_spawnPos.y, m_spawnPos.z, nullptr, &m_spawnPos.radius);

        if (wanderOrPahtId)
            m_spawnPos.radius = static_cast<float>(wanderOrPahtId);
    }
}

void FormationData::Update()
{
    // check if stop timer for follower is hit
    if (m_followerStopped)
    {
        if (m_followerStartTime < World::GetCurrentClockTime())
        {
            StartFollower();
        }
    }

    if (m_currentFormationShape != SPAWN_GROUP_FORMATION_TYPE_RANDOM)
        return;

    // Change position using variation of real position
    if (GetMaster() && GetMaster()->IsMoving())
    {
        if (m_nextVariation < World::GetCurrentClockTime())
        {
            m_nextVariation = World::GetCurrentClockTime() + std::chrono::milliseconds(urand(20000, 50000));

            for (auto& slotItr : m_slotsMap)
            {
                auto& slot = slotItr.second;

                if (slot->GetOwner() && slot->GetOwner() == GetMaster())
                    continue;

                slot->AddPositionVariation();
            }
            //sLog.outString("Adding variation to formation %u", m_groupData->GetGroupId());
        }
    }

    // Update position slowly to computed variation of the real position
    if (m_nextVariationUpdate < World::GetCurrentClockTime())
    {
        m_nextVariationUpdate = World::GetCurrentClockTime() + std::chrono::milliseconds(1000);
        for (auto& slotItr : m_slotsMap)
        {
            auto& slot = slotItr.second;

            if (slot->GetOwner() && slot->GetOwner() == GetMaster())
                continue;

            slot->Update();
        }
    }
}

void FormationData::FixSlotsPositions()
{
    float defaultDist = m_currentSpread;
    auto& slots = m_slotsMap;
    float totalMembers = 0;
    bool onlyAlive = HaveOption(SPAWN_GROUP_FORMATION_OPTION_KEEP_CONPACT);

    float modelWidth = 0;
    for (auto& slotItr : slots)
    {
        auto& slot = slotItr.second;

        if (onlyAlive && (!slot->GetOwner() || slot->GetOwner()->IsAlive()))
            continue;

        if (slot->GetOwner() && modelWidth < slot->GetOwner()->GetCollisionWidth())
            modelWidth = slot->GetOwner()->GetCollisionWidth();

        ++totalMembers;
    }

    if (totalMembers <= 1)
        return;

    // only take account of the followers
    --totalMembers;

    switch (m_currentFormationShape)
    {
        // random formation
        case SPAWN_GROUP_FORMATION_TYPE_RANDOM:
        {
            uint32 membCount = 1;
            for (auto& slotItr : m_slotsMap)
            {
                auto& slot = slotItr.second;
                if (slot->GetOwner() && slot->GetOwner() == GetMaster())
                {
                    slot->SetAngle(0);
                    slot->SetDistance(0);
                    continue;
                }

                if (HaveOption(SPAWN_GROUP_FORMATION_OPTION_KEEP_CONPACT) && (!slot->GetOwner() || !slot->GetOwner()->IsAlive()))
                    continue;

                // guessed value, will be the space between each group members
                float testInterSpread = modelWidth * 3.0f;
                float finalAngle = 0;
                float finalDist = 0;
                switch (membCount)
                {
                    case 1:
                    {
                        float distBack = m_currentSpread + testInterSpread;
                        float hyp = std::sqrt(distBack * distBack + testInterSpread * testInterSpread);

                        finalAngle = M_PI_F - std::asin(testInterSpread / hyp);
                        finalDist = hyp;
                        break;
                    }
                    case 2:
                    {
                        float distBack = m_currentSpread + testInterSpread;
                        float hyp = std::sqrt(distBack * distBack + testInterSpread * testInterSpread);

                        finalAngle = M_PI_F + std::asin(testInterSpread / hyp);
                        finalDist = hyp;
                        break;
                    }
                    case 3:
                    {
                        finalAngle = M_PI_F;
                        finalDist = m_currentSpread + (testInterSpread * 2.0f);
                        break;

                    }
                    case 4:
                    {
                        finalAngle = M_PI_F;
                        finalDist = m_currentSpread + (testInterSpread * 3.0f);
                        break;
                    }
                    case 5:
                    {
                        float distBack = m_currentSpread + (testInterSpread * 2.0f);
                        float hyp = std::sqrt(distBack * distBack + testInterSpread * testInterSpread);

                        finalAngle = M_PI_F - std::asin(testInterSpread / hyp);
                        finalDist = hyp;
                        break;
                    }
                    case 6:
                    {
                        float distBack = m_currentSpread + (testInterSpread * 2.0f);
                        float hyp = std::sqrt(distBack * distBack + testInterSpread * testInterSpread);

                        finalAngle = M_PI_F + std::asin(testInterSpread / hyp);
                        finalDist = hyp;
                        break;
                    }
                    case 7:
                    {
                        float distBack = m_currentSpread + (testInterSpread * 3.0f);
                        float hyp = std::sqrt(distBack * distBack + testInterSpread * testInterSpread);

                        finalAngle = M_PI_F - std::asin(testInterSpread / hyp);
                        finalDist = hyp;
                        break;
                    }
                    case 8:
                    {
                        float distBack = m_currentSpread + (testInterSpread * 3.0f);
                        float hyp = std::sqrt(distBack * distBack + testInterSpread * testInterSpread);

                        finalAngle = M_PI_F + std::asin(testInterSpread / hyp);
                        finalDist = hyp;
                        break;
                    }
                    case 9:
                    {
                        finalAngle = M_PI_F;
                        finalDist = m_currentSpread + testInterSpread;
                        break;
                    }

                    default:
                    {
                        // put extra member beside the leader
                        if ((membCount & 1) == 0)
                            finalAngle = (M_PI_F / 2.0f) + M_PI_F;
                        else
                            finalAngle = M_PI_F / 2.0f;
                        finalDist = testInterSpread * (((membCount - 10) / 2) + 1);
                        break;
                    }
                }
                slot->SetAngle(finalAngle);
                slot->SetDistance(finalDist);
                slot->SetMaxVariation((M_PI_F / 8.0f) * modelWidth, testInterSpread);
                slot->AddPositionVariation(true);
                slot->GetRecomputePosition() = true;
                ++membCount;
            }
            break;
        }
    
        // single file formation
        case SPAWN_GROUP_FORMATION_TYPE_SINGLE_FILE:
        {
            uint32 membCount = 1;
            for (auto& slotItr : slots)
            {
                auto& slot = slotItr.second;
    
                if (slot->GetOwner() && slot->GetOwner() == GetMaster())
                {
                    slot->SetAngle(0);
                    slot->SetDistance(0);
                    continue;
                }
    
                if (onlyAlive && (!slot->GetOwner() || !slot->GetOwner()->IsAlive()))
                    continue;
    
                slot->SetAngle(M_PI_F);
                slot->SetDistance(defaultDist * membCount);
                slot->GetRecomputePosition() = true;
                ++membCount;
            }
            break;
        }
    
        // side by side formation
        case SPAWN_GROUP_FORMATION_TYPE_SIDE_BY_SIDE:
        {
            uint32 membCount = 1;
            for (auto& slotItr : slots)
            {
                auto& slot = slotItr.second;
    
                if (slot->GetOwner() && slot->GetOwner() == GetMaster())
                {
                    slot->SetAngle(0);
                    slot->SetDistance(0);
                    continue;
                }
    
                if (onlyAlive && (!slot->GetOwner() || !slot->GetOwner()->IsAlive()))
                    continue;
    
                if ((membCount & 1) == 0)
                    slot->SetAngle((M_PI_F / 2.0f) + M_PI_F);
                else
                    slot->SetAngle(M_PI_F / 2.0f);
                slot->SetDistance(defaultDist * (((membCount - 1) / 2) + 1));
                slot->GetRecomputePosition() = true;
                ++membCount;
            }
            break;
        }
    
        // like a geese formation
        case SPAWN_GROUP_FORMATION_TYPE_LIKE_GEESE:
        {
            uint32 membCount = 1;
            for (auto& slotItr : slots)
            {
                auto& slot = slotItr.second;
    
                if (slot->GetOwner() && slot->GetOwner() == GetMaster())
                {
                    slot->SetAngle(0);
                    slot->SetDistance(0);
                    continue;
                }
    
                if (onlyAlive && (!slot->GetOwner() || !slot->GetOwner()->IsAlive()))
                    continue;
    
                if ((membCount & 1) == 0)
                    slot->SetAngle(M_PI_F + (M_PI_F / 4.0f));
                else
                    slot->SetAngle(M_PI_F - (M_PI_F / 4.0f));
                slot->SetDistance(defaultDist* (((membCount - 1) / 2) + 1));
                slot->GetRecomputePosition() = true;
                ++membCount;
            }
            break;
        }
    
        // fanned behind formation
        case SPAWN_GROUP_FORMATION_TYPE_FANNED_OUT_BEHIND:
        {
            uint32 membCount = 1;
            for (auto& slotItr : slots)
            {
                auto& slot = slotItr.second;
    
                if (slot->GetOwner() && slot->GetOwner() == GetMaster())
                {
                    slot->SetAngle(0);
                    slot->SetDistance(0);
                    continue;
                }
    
                if (onlyAlive && (!slot->GetOwner() || !slot->GetOwner()->IsAlive()))
                    continue;
    
                slot->SetAngle((M_PI_F / 2.0f) + (M_PI_F / totalMembers) * (membCount - 1));
                slot->SetDistance(defaultDist);
                slot->GetRecomputePosition() = true;
                ++membCount;
            }
            break;
        }
    
        // fanned in front formation
        case SPAWN_GROUP_FORMATION_TYPE_FANNED_OUT_IN_FRONT:
        {
            uint32 membCount = 1;
            for (auto& slotItr : slots)
            {
                auto& slot = slotItr.second;
    
                if (slot->GetOwner() && slot->GetOwner() == GetMaster())
                {
                    slot->SetAngle(0);
                    slot->SetDistance(0);
                    continue;
                }
    
                if (onlyAlive && (!slot->GetOwner() || !slot->GetOwner()->IsAlive()))
                    continue;
    
                slot->SetAngle(M_PI_F + (M_PI_F / 2.0f) + (M_PI_F / totalMembers) * (membCount - 1));
                if (slot->GetRealAngle() > M_PI_F * 2.0f)
                    slot->SetAngle(slot->GetRealAngle() - M_PI_F * 2.0f);
                slot->SetDistance(defaultDist);
                slot->GetRecomputePosition() = true;
                ++membCount;
            }
            break;
        }
    
        // circle formation
        case SPAWN_GROUP_FORMATION_TYPE_CIRCLE_THE_LEADER:
        {
            uint32 membCount = 1;
            for (auto& slotItr : slots)
            {
                auto& slot = slotItr.second;
    
                if (slot->GetOwner() && slot->GetOwner() == GetMaster())
                {
                    slot->SetAngle(0);
                    slot->SetDistance(0);
                    continue;
                }
    
                if (onlyAlive && (!slot->GetOwner() || !slot->GetOwner()->IsAlive()))
                    continue;
    
                slot->SetAngle(((M_PI_F * 2.0f) / totalMembers) * (membCount - 1));
                slot->SetDistance(defaultDist);
                slot->GetRecomputePosition() = true;
                ++membCount;
            }
            break;
        }
        default:
            break;
    }
}

Unit* FormationSlotData::GetMaster()
{
    return GetFormationData()->GetMaster();
}

bool FormationSlotData::IsFormationMaster()
{
    return GetMaster() == m_owner;
}

float FormationSlotData::GetAngle()
{
    if (GetCreatureGroup()->GetGroupEntry().IsMirroring())
    {
        if (!GetFormationData()->GetMirrorState())
            return m_angleVariation;
        return (2 * M_PI_F) - m_angleVariation;
    }

    return m_angleVariation;
}

float FormationSlotData::GetDistance() const
{
    return m_distanceVariation;
}

void FormationSlotData::AddPositionVariation(bool now /*= false*/)
{
    m_angleVariationDest = m_realAngle - (m_maxAngleVariation / 2.0f) + frand(m_maxAngleVariation / 100.0f, m_maxAngleVariation);
    m_distanceVariationDest = m_realDistance - (m_maxDistanceVariation / 2.0f) + frand(m_maxDistanceVariation / 100.0f, m_maxDistanceVariation);
    if (now)
    {
        m_angleVariation = m_angleVariationDest;
        m_distanceVariation = m_distanceVariationDest;
    }
}

void FormationSlotData::Update()
{
    float diff = m_angleVariationDest - m_angleVariation;
    if (diff > 0.01f)
    {
        m_angleVariation = m_angleVariation + (diff / 20.0f);
    }
    else
        m_angleVariation = m_angleVariationDest;

    diff = m_distanceVariationDest - m_distanceVariation;
    if (diff > 0.1f)
    {
        m_distanceVariation = m_distanceVariation + (diff / 20.0f);
    }
    else
        m_distanceVariation = m_distanceVariationDest;
}
