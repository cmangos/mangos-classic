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
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
*/

#include "WorldState.h"
#include "Chat/ChannelMgr.h"
#include "Globals/SharedDefines.h"
#include "World/World.h"
#include "Maps/MapManager.h"
#include "Entities/Object.h"
#include "GameEvents/GameEventMgr.h"
#include <algorithm>
#include <map>
#include <World/WorldStateDefines.h>

std::map<AQResources, WorldStateID> aqWorldstateMap =
{
    {AQ_PEACEBLOOM, WORLD_STATE_AQ_PEACEBLOOM_NOW},
    {AQ_LEAN_WOLF_STEAK, WORLD_STATE_AQ_LEAN_WOLF_STEAK_NOW},
    {AQ_TIN_BAR, WORLD_STATE_AQ_TIN_BARS_NOW},
    {AQ_WOOL_BANDAGE, WORLD_STATE_AQ_WOOL_BANDAGE_NOW},
    {AQ_FIREBLOOM, WORLD_STATE_AQ_FIREBLOOM_NOW},
    {AQ_HEAVY_LEATHER, WORLD_STATE_AQ_HEAVY_LEATHER_NOW},
    {AQ_MITHRIL_BAR, WORLD_STATE_AQ_MITHRIL_BARS_NOW},
    {AQ_MAGEWEAVE_BANDAGE, WORLD_STATE_AQ_MAGEWEAVE_BANDAGE_NOW},
    {AQ_RUGGED_LEATHER, WORLD_STATE_AQ_RUGGED_LEATHER_NOW},
    {AQ_BAKED_SALMON, WORLD_STATE_AQ_BAKED_SALMON_NOW},
    {AQ_LIGHT_LEATHER, WORLD_STATE_AQ_LIGHT_LEATHER_NOW},
    {AQ_LINEN_BANDAGE, WORLD_STATE_AQ_LINEN_BANDAGE_NOW},
    {AQ_MEDIUM_LEATHER, WORLD_STATE_AQ_MEDIUM_LEATHER_NOW},
    {AQ_STRANGLEKELP, WORLD_STATE_AQ_STRANGLEKELP_NOW},
    {AQ_RAINBOW_FIN_ALBACORE, WORLD_STATE_AQ_RAINBOW_FIN_ALBACORE_NOW},
    {AQ_IRON_BAR, WORLD_STATE_AQ_IRON_BARS_NOW},
    {AQ_ROAST_RAPTOR, WORLD_STATE_AQ_ROAST_RAPTOR_NOW},
    {AQ_SILK_BANDAGE, WORLD_STATE_AQ_SILK_BANDAGE_NOW},
    {AQ_THORIUM_BAR, WORLD_STATE_AQ_THORIUM_BARS_NOW},
    {AQ_ARTHAS_TEARS, WORLD_STATE_AQ_ARTHAS_TEARS_NOW},
    {AQ_COPPER_BAR_ALLY, WORLD_STATE_AQ_COPPER_BARS_ALLY_NOW},
    {AQ_PURPLE_LOTUS_ALLY, WORLD_STATE_AQ_PURPLE_LOTUS_ALLY_NOW},
    {AQ_THICK_LEATHER_ALLY, WORLD_STATE_AQ_THICK_LEATHER_ALLY_NOW},
    {AQ_SPOTTED_YELLOWTAIL_ALLY, WORLD_STATE_AQ_SPOTTED_YELLOWTAIL_ALLY_NOW},
    {AQ_RUNECLOTH_BANDAGE_ALLY, WORLD_STATE_AQ_RUNECLOTH_BANDAGE_ALLY_NOW},
    {AQ_COPPER_BAR_HORDE, WORLD_STATE_AQ_COPPER_BARS_HORDE_NOW},
    {AQ_PURPLE_LOTUS_HORDE, WORLD_STATE_AQ_PURPLE_LOTUS_HORDE_NOW},
    {AQ_THICK_LEATHER_HORDE, WORLD_STATE_AQ_THICK_LEATHER_HORDE_NOW},
    {AQ_SPOTTED_YELLOWTAIL_HORDE, WORLD_STATE_AQ_SPOTTED_YELLOWTAIL_HORDE_NOW},
    {AQ_RUNECLOTH_BANDAGE_HORDE, WORLD_STATE_AQ_RUNECLOTH_BANDAGE_HORDE_NOW},
};

std::vector<std::pair<WorldStateID, uint32>> aqWorldStateTotalsMap =
{
    {WORLD_STATE_AQ_PEACEBLOOM_TOTAL, 96000},
    {WORLD_STATE_AQ_LEAN_WOLF_STEAK_TOTAL, 10000},
    {WORLD_STATE_AQ_TIN_BARS_TOTAL, 22000},
    {WORLD_STATE_AQ_WOOL_BANDAGE_TOTAL, 250000},
    {WORLD_STATE_AQ_FIREBLOOM_TOTAL, 19000},
    {WORLD_STATE_AQ_HEAVY_LEATHER_TOTAL, 60000},
    {WORLD_STATE_AQ_MITHRIL_BARS_TOTAL, 18000},
    {WORLD_STATE_AQ_MAGEWEAVE_BANDAGE_TOTAL, 250000},
    {WORLD_STATE_AQ_RUGGED_LEATHER_TOTAL, 60000},
    {WORLD_STATE_AQ_BAKED_SALMON_TOTAL, 10000},
    {WORLD_STATE_AQ_LIGHT_LEATHER_TOTAL, 180000},
    {WORLD_STATE_AQ_LINEN_BANDAGE_TOTAL, 800000},
    {WORLD_STATE_AQ_MEDIUM_LEATHER_TOTAL, 110000},
    {WORLD_STATE_AQ_STRANGLEKELP_TOTAL, 33000},
    {WORLD_STATE_AQ_RAINBOW_FIN_ALBACORE_TOTAL, 14000},
    {WORLD_STATE_AQ_IRON_BARS_TOTAL, 28000},
    {WORLD_STATE_AQ_ROAST_RAPTOR_TOTAL, 20000},
    {WORLD_STATE_AQ_SILK_BANDAGE_TOTAL, 600000},
    {WORLD_STATE_AQ_THORIUM_BARS_TOTAL, 24000},
    {WORLD_STATE_AQ_ARTHAS_TEARS_TOTAL, 20000},
    {WORLD_STATE_AQ_COPPER_BARS_TOTAL, 45000},
    {WORLD_STATE_AQ_PURPLE_LOTUS_TOTAL, 13000},
    {WORLD_STATE_AQ_THICK_LEATHER_TOTAL, 40000},
    {WORLD_STATE_AQ_SPOTTED_YELLOWTAIL_TOTAL, 8500},
    {WORLD_STATE_AQ_RUNECLOTH_BANDAGE_TOTAL, 200000},
};

WorldState::WorldState() : m_emeraldDragonsState(0xF), m_emeraldDragonsTimer(0), m_emeraldDragonsChosenPositions(4, 0)
{
    memset(m_loveIsInTheAirData.counters, 0, sizeof(LoveIsInTheAir));

    for (auto& data : aqWorldstateMap)
        m_aqWorldstateMapReverse.emplace(data.second, data.first);
}


WorldState::~WorldState()
{
}

void WorldState::Load()
{
    std::unique_ptr<QueryResult> result(CharacterDatabase.Query("SELECT Id, Data FROM world_state"));

    if (result)
    {
        // always one row
        do
        {
            Field* fields = result->Fetch();
            uint32 id = fields[0].GetUInt32();
            std::string data = fields[1].GetCppString();
            std::istringstream loadStream(data);
            switch (id)
            {
                case SAVE_ID_EMERALD_DRAGONS:
                {
                    auto curTime = World::GetCurrentClockTime();
                    uint64 respawnTime;
                    if (data.size())
                    {
                        loadStream >> m_emeraldDragonsState >> respawnTime;
                        for (uint32 i = 0; i < 4; ++i)
                            loadStream >> m_emeraldDragonsChosenPositions[i];
                        if (respawnTime)
                        {
                            TimePoint respawnTimePoint = std::chrono::time_point_cast<std::chrono::milliseconds>(Clock::from_time_t(respawnTime));
                            m_emeraldDragonsTimer = std::chrono::duration_cast<std::chrono::milliseconds>(respawnTimePoint - curTime).count();
                        }
                        else m_emeraldDragonsTimer = 0;
                    }
                    break;
                }
                case SAVE_ID_AHN_QIRAJ:
                {
                    if (data.size())
                    {
                        auto curTime = World::GetCurrentClockTime();
                        uint64 time;
                        try
                        {
                            loadStream >> m_aqData.m_phase >> time;
                            if (time)
                            {
                                TimePoint timePoint = std::chrono::time_point_cast<std::chrono::milliseconds>(Clock::from_time_t(time));
                                m_aqData.m_timer = std::chrono::duration_cast<std::chrono::milliseconds>(timePoint - curTime).count();
                            }
                            for (uint32 i = 0; i < RESOURCE_MAX; ++i)
                                loadStream >> m_aqData.m_WarEffortCounters[i];
                            loadStream >> m_aqData.m_phase2Tier;
                        }
                        catch (std::exception& e)
                        {
                            sLog.outError("%s", e.what());
                            memset(m_loveIsInTheAirData.counters, 0, sizeof(LoveIsInTheAir));
                        }
                    }
                    break;
                }
                case SAVE_ID_LOVE_IS_IN_THE_AIR:
                    if (data.size())
                    {
                        try
                        {
                            for (uint32 i = 0; i < LOVE_LEADER_MAX; ++i)
                                loadStream >> m_loveIsInTheAirData.counters[i];
                        }
                        catch (std::exception & e)
                        {
                            sLog.outError("%s", e.what());
                            memset(m_loveIsInTheAirData.counters, 0, sizeof(LoveIsInTheAir));
                        }
                    }
                    else
                        memset(m_loveIsInTheAirData.counters, 0, sizeof(LoveIsInTheAir));
                    break;
            }
        }
        while (result->NextRow());
    }
    StartWarEffortEvent();
    SpawnWarEffortGos();
    RespawnEmeraldDragons();
}

void WorldState::Save(SaveIds saveId)
{
    switch (saveId)
    {
        case SAVE_ID_EMERALD_DRAGONS:
        {
            uint64 time;
            if (m_emeraldDragonsTimer)
            {
                auto curTime = World::GetCurrentClockTime();
                auto respawnTime = std::chrono::milliseconds(m_emeraldDragonsTimer) + curTime;
                time = uint64(Clock::to_time_t(respawnTime));
            }
            else time = 0;
            std::string dragonsData = std::to_string(m_emeraldDragonsState) + " " + std::to_string(time);
            for (uint32 i = 0; i < 4; ++i)
                dragonsData += " " + std::to_string(m_emeraldDragonsChosenPositions[i]);
            CharacterDatabase.PExecute("DELETE FROM world_state WHERE Id='%u'", SAVE_ID_EMERALD_DRAGONS);
            CharacterDatabase.PExecute("INSERT INTO world_state(Id,Data) VALUES('%u','%s')", SAVE_ID_EMERALD_DRAGONS, dragonsData.data());
            break;
        }
        case SAVE_ID_AHN_QIRAJ:
        {
            std::string expansionData = m_aqData.GetData();
            SaveHelper(expansionData, SAVE_ID_AHN_QIRAJ);
            break;
        }
        case SAVE_ID_LOVE_IS_IN_THE_AIR:
        {
            std::string loveData;
            for (uint32 i = 0; i < LOVE_LEADER_MAX; ++i)
            {
                if (i != 0)
                    loveData += " ";
                loveData += std::to_string(m_loveIsInTheAirData.counters[i]);
            }
            SaveHelper(loveData, SAVE_ID_LOVE_IS_IN_THE_AIR);
            break;
        }
        default: break;
    }
}

void WorldState::SaveHelper(std::string& stringToSave, SaveIds saveId)
{
    CharacterDatabase.PExecute("DELETE FROM world_state WHERE Id='%u'", saveId);
    CharacterDatabase.PExecute("INSERT INTO world_state(Id,Data) VALUES('%u','%s')", saveId, stringToSave.data());
}

void WorldState::HandleGameObjectUse(GameObject* go, Unit* user)
{
    switch (go->GetEntry())
    {
        case OBJECT_EVENT_TRAP_THRALL:
        {
            HandleExternalEvent(CUSTOM_EVENT_LOVE_IS_IN_THE_AIR_LEADER, LOVE_LEADER_THRALL);
            SendWorldstateUpdate(m_loveIsInTheAirMutex, m_loveIsInTheAirCapitalsPlayers, GetLoveIsInTheAirCounter(LOVE_LEADER_THRALL), WORLD_STATE_LOVE_IS_IN_THE_AIR_THRALL);
            uint32 hordeSum = GetLoveIsInTheAirCounter(LOVE_LEADER_CAIRNE) + GetLoveIsInTheAirCounter(LOVE_LEADER_THRALL) + GetLoveIsInTheAirCounter(LOVE_LEADER_SYLVANAS);
            SendWorldstateUpdate(m_loveIsInTheAirMutex, m_loveIsInTheAirCapitalsPlayers, hordeSum, WORLD_STATE_LOVE_IS_IN_THE_AIR_TOTAL_HORDE);
            break;
        }
        case OBJECT_EVENT_TRAP_CAIRNE:
        {
            HandleExternalEvent(CUSTOM_EVENT_LOVE_IS_IN_THE_AIR_LEADER, LOVE_LEADER_CAIRNE);
            SendWorldstateUpdate(m_loveIsInTheAirMutex, m_loveIsInTheAirCapitalsPlayers, GetLoveIsInTheAirCounter(LOVE_LEADER_CAIRNE), WORLD_STATE_LOVE_IS_IN_THE_AIR_CAIRNE);
            uint32 hordeSum = GetLoveIsInTheAirCounter(LOVE_LEADER_CAIRNE) + GetLoveIsInTheAirCounter(LOVE_LEADER_THRALL) + GetLoveIsInTheAirCounter(LOVE_LEADER_SYLVANAS);
            SendWorldstateUpdate(m_loveIsInTheAirMutex, m_loveIsInTheAirCapitalsPlayers, hordeSum, WORLD_STATE_LOVE_IS_IN_THE_AIR_TOTAL_HORDE);
            break;
        }
        case OBJECT_EVENT_TRAP_SYLVANAS:
        {
            HandleExternalEvent(CUSTOM_EVENT_LOVE_IS_IN_THE_AIR_LEADER, LOVE_LEADER_SYLVANAS);
            SendWorldstateUpdate(m_loveIsInTheAirMutex, m_loveIsInTheAirCapitalsPlayers, GetLoveIsInTheAirCounter(LOVE_LEADER_SYLVANAS), WORLD_STATE_LOVE_IS_IN_THE_AIR_SYLVANAS);
            uint32 hordeSum = GetLoveIsInTheAirCounter(LOVE_LEADER_CAIRNE) + GetLoveIsInTheAirCounter(LOVE_LEADER_THRALL) + GetLoveIsInTheAirCounter(LOVE_LEADER_SYLVANAS);
            SendWorldstateUpdate(m_loveIsInTheAirMutex, m_loveIsInTheAirCapitalsPlayers, hordeSum, WORLD_STATE_LOVE_IS_IN_THE_AIR_TOTAL_HORDE);
            break;
        }
        case OBJECT_EVENT_TRAP_BOLVAR:
        {
            HandleExternalEvent(CUSTOM_EVENT_LOVE_IS_IN_THE_AIR_LEADER, LOVE_LEADER_BOLVAR);
            SendWorldstateUpdate(m_loveIsInTheAirMutex, m_loveIsInTheAirCapitalsPlayers, GetLoveIsInTheAirCounter(LOVE_LEADER_BOLVAR), WORLD_STATE_LOVE_IS_IN_THE_AIR_BOLVAR);
            uint32 allianceSum = GetLoveIsInTheAirCounter(LOVE_LEADER_BOLVAR) + GetLoveIsInTheAirCounter(LOVE_LEADER_TYRANDE) + GetLoveIsInTheAirCounter(LOVE_LEADER_MAGNI);
            SendWorldstateUpdate(m_loveIsInTheAirMutex, m_loveIsInTheAirCapitalsPlayers, allianceSum, WORLD_STATE_LOVE_IS_IN_THE_AIR_TOTAL_ALLIANCE);
            break;
        }
        case OBJECT_EVENT_TRAP_MAGNI:
        {
            HandleExternalEvent(CUSTOM_EVENT_LOVE_IS_IN_THE_AIR_LEADER, LOVE_LEADER_MAGNI);
            SendWorldstateUpdate(m_loveIsInTheAirMutex, m_loveIsInTheAirCapitalsPlayers, GetLoveIsInTheAirCounter(LOVE_LEADER_MAGNI), WORLD_STATE_LOVE_IS_IN_THE_AIR_MAGNI);
            uint32 allianceSum = GetLoveIsInTheAirCounter(LOVE_LEADER_BOLVAR) + GetLoveIsInTheAirCounter(LOVE_LEADER_TYRANDE) + GetLoveIsInTheAirCounter(LOVE_LEADER_MAGNI);
            SendWorldstateUpdate(m_loveIsInTheAirMutex, m_loveIsInTheAirCapitalsPlayers, allianceSum, WORLD_STATE_LOVE_IS_IN_THE_AIR_TOTAL_ALLIANCE);
            break;
        }
        case OBJECT_EVENT_TRAP_TYRANDE:
        {
            HandleExternalEvent(CUSTOM_EVENT_LOVE_IS_IN_THE_AIR_LEADER, LOVE_LEADER_TYRANDE);
            SendWorldstateUpdate(m_loveIsInTheAirMutex, m_loveIsInTheAirCapitalsPlayers, GetLoveIsInTheAirCounter(LOVE_LEADER_TYRANDE), WORLD_STATE_LOVE_IS_IN_THE_AIR_TYRANDE);
            uint32 allianceSum = GetLoveIsInTheAirCounter(LOVE_LEADER_BOLVAR) + GetLoveIsInTheAirCounter(LOVE_LEADER_TYRANDE) + GetLoveIsInTheAirCounter(LOVE_LEADER_MAGNI);
            SendWorldstateUpdate(m_loveIsInTheAirMutex, m_loveIsInTheAirCapitalsPlayers, allianceSum, WORLD_STATE_LOVE_IS_IN_THE_AIR_TOTAL_ALLIANCE);
            break;
        }
        default:
            break;
    }
}

void WorldState::HandleGameObjectRevertState(GameObject* go)
{

}

void WorldState::HandlePlayerEnterZone(Player* player, uint32 zoneId)
{
    switch (zoneId)
    {
        case ZONEID_STORMWIND_CITY:
        case ZONEID_DARNASSUS:
        case ZONEID_IRONFORGE:
        case ZONEID_ORGRIMMAR:
        case ZONEID_THUNDER_BLUFF:
        case ZONEID_UNDERCITY:
        {
            {
                std::lock_guard<std::mutex> guard(m_loveIsInTheAirMutex);
                m_loveIsInTheAirCapitalsPlayers.push_back(player->GetObjectGuid());
            }
            {
                std::lock_guard<std::mutex> guard(m_aqData.m_warEffortMutex);
                m_aqData.m_warEffortWorldstatesPlayers.push_back(player->GetObjectGuid());
            }
            break;
        }
        default:
            break;
    }
}

void WorldState::HandlePlayerLeaveZone(Player* player, uint32 zoneId)
{
    switch (zoneId)
    {
        case ZONEID_STORMWIND_CITY:
        case ZONEID_DARNASSUS:
        case ZONEID_IRONFORGE:
        case ZONEID_ORGRIMMAR:
        case ZONEID_THUNDER_BLUFF:
        case ZONEID_UNDERCITY:
        {
            {
                std::lock_guard<std::mutex> guard(m_loveIsInTheAirMutex);
                auto position = std::find(m_loveIsInTheAirCapitalsPlayers.begin(), m_loveIsInTheAirCapitalsPlayers.end(), player->GetObjectGuid());
                if (position != m_loveIsInTheAirCapitalsPlayers.end())
                    m_loveIsInTheAirCapitalsPlayers.erase(position);
            }
            {
                std::lock_guard<std::mutex> guard(m_aqData.m_warEffortMutex);
                auto position = std::find(m_aqData.m_warEffortWorldstatesPlayers.begin(), m_aqData.m_warEffortWorldstatesPlayers.end(), player->GetObjectGuid());
                if (position != m_aqData.m_warEffortWorldstatesPlayers.end())
                    m_aqData.m_warEffortWorldstatesPlayers.erase(position);
            }
            break;
        }
        default:
            break;
    }
}

//void WorldState::HandlePlayerEnterArea(Player* player, uint32 areaId)
//{
//
//}
//
//void WorldState::HandlePlayerLeaveArea(Player* player, uint32 areaId)
//{
//
//}

bool WorldState::IsConditionFulfilled(uint32 conditionId, uint32 state) const
{
    if (conditionId == WAR_EFFORT_DAYS_LEFT)
        return m_aqData.GetDaysRemaining() == state;

    auto itr = m_aqWorldstateMapReverse.find(conditionId);
    if (itr != m_aqWorldstateMapReverse.end())
    {
        uint32 id = (*itr).second;
        if (id >= aqWorldStateTotalsMap.size())
            id -= 5;
        return m_aqData.m_WarEffortCounters[id] == aqWorldStateTotalsMap[id].second;
    }

    return m_transportStates.at(conditionId) == state;
}

void WorldState::HandleConditionStateChange(uint32 conditionId, uint32 state)
{
    m_transportStates[conditionId] = state;
}

void WorldState::HandleExternalEvent(uint32 eventId, uint32 param)
{
    std::lock_guard<std::mutex> guard(m_mutex);

    switch (eventId)
    {
        case CUSTOM_EVENT_YSONDRE_DIED:
        case CUSTOM_EVENT_LETHON_DIED:
        case CUSTOM_EVENT_EMERISS_DIED:
        case CUSTOM_EVENT_TAERAR_DIED:
        {
            uint32 bossId;
            switch (eventId)
            {
                case CUSTOM_EVENT_YSONDRE_DIED: bossId = 0; break;
                case CUSTOM_EVENT_LETHON_DIED:  bossId = 1; break;
                case CUSTOM_EVENT_EMERISS_DIED: bossId = 2; break;
                case CUSTOM_EVENT_TAERAR_DIED:  bossId = 3; break;
            }
            m_emeraldDragonsState |= 1 << bossId;
            if (m_emeraldDragonsState == 0xF)
                m_emeraldDragonsTimer = 30 * HOUR * IN_MILLISECONDS;
            Save(SAVE_ID_EMERALD_DRAGONS); // save to DB right away
            break;
        }
        case CUSTOM_EVENT_LOVE_IS_IN_THE_AIR_LEADER:
        {
            MANGOS_ASSERT(param < LOVE_LEADER_MAX);
            ++m_loveIsInTheAirData.counters[param];
            Save(SAVE_ID_LOVE_IS_IN_THE_AIR);
            break;
        }
    }
}

void WorldState::Update(const uint32 diff)
{
    std::lock_guard<std::mutex> guard(m_mutex);

    if (m_emeraldDragonsTimer)
    {
        if (m_emeraldDragonsTimer <= diff)
        {
            m_emeraldDragonsTimer = 0;
            RespawnEmeraldDragons();
        }
        else m_emeraldDragonsTimer -= diff;
    }

    if (m_aqData.m_timer)
    {
        if (m_aqData.m_timer <= diff)
        {
            m_aqData.m_timer = 0;
            HandleWarEffortPhaseTransition(m_aqData.m_phase + 1);
        }
        else m_aqData.m_timer -= diff;

        if (m_aqData.m_phase == PHASE_2_TRANSPORTING_RESOURCES)
        {
            uint32 remainingDays = m_aqData.m_timer % (DAY * IN_MILLISECONDS);
            if (remainingDays < m_aqData.m_phase2Tier)
                ChangeWarEffortPhase2Tier(remainingDays);
        }
    }
}

void WorldState::SendWorldstateUpdate(std::mutex& mutex, GuidVector const& guids, uint32 value, uint32 worldStateId)
{
    std::lock_guard<std::mutex> guard(mutex);
    for (ObjectGuid const& guid : guids)
        if (Player* player = sObjectMgr.GetPlayer(guid))
            player->SendUpdateWorldState(worldStateId, value);
}

void WorldState::SendLoveIsInTheAirWorldstateUpdate(uint32 value, uint32 worldStateId)
{
    std::lock_guard<std::mutex> guard(m_loveIsInTheAirMutex);
    for (ObjectGuid& guid : m_loveIsInTheAirCapitalsPlayers)
        if (Player* player = sObjectMgr.GetPlayer(guid))
            player->SendUpdateWorldState(worldStateId, value);
}

void WorldState::ExecuteOnAreaPlayers(uint32 areaId, std::function<void(Player*)> executor)
{
    std::lock_guard<std::mutex> guard(m_mutex);
    for (ObjectGuid guid : m_areaPlayers[areaId])
        if (Player* player = sObjectMgr.GetPlayer(guid))
            executor(player);
}

// Emerald Dragons
enum EmeraldDragons : uint32
{
    NPC_YSONDRE = 14887,
    NPC_LETHON  = 14888,
    NPC_EMERISS = 14889,
    NPC_TAERAR  = 14890,
};

static float emeraldDragonSpawns[4][4] =
{
    {-10428.8f, -392.176f, 43.7411f, 0.932375f},
    {753.619f, -4012.f, 94.043f, 3.193f},
    {-2872.66f, 1884.25f, 52.7336f, 2.6529f},
    {3301.05f, -3732.57f, 173.544f, 2.9147f}
};

static uint32 pathIds[4] =
{
    1,0,0,0
};

bool WorldState::IsDragonSpawned(uint32 entry)
{
    return ((1 << (entry - NPC_YSONDRE)) & m_emeraldDragonsState) == 0;
}

void WorldState::RespawnEmeraldDragons()
{
    if (m_emeraldDragonsState == 0xF)
    {
        std::vector<uint32> ids = { NPC_YSONDRE, NPC_LETHON, NPC_EMERISS, NPC_TAERAR };
        for (uint32 i = 3; i > 0; --i)
        {
            uint32 random = urand(0, i);
            m_emeraldDragonsChosenPositions[3 - i] = ids[random];
            ids.erase(std::remove(ids.begin(), ids.end(), ids[random]), ids.end());
        }
        m_emeraldDragonsChosenPositions[3] = ids[0];
        m_emeraldDragonsState = 0;
        Save(SAVE_ID_EMERALD_DRAGONS);
    }
    sMapMgr.DoForAllMapsWithMapId(0, [&](Map* map)
    {
        if (IsDragonSpawned(m_emeraldDragonsChosenPositions[0]))
        {
            if (Creature* duskwoodDragon = WorldObject::SummonCreature(TempSpawnSettings(nullptr, m_emeraldDragonsChosenPositions[0], emeraldDragonSpawns[0][0], emeraldDragonSpawns[0][1], emeraldDragonSpawns[0][2], emeraldDragonSpawns[0][3], TEMPSPAWN_DEAD_DESPAWN, 0, false, false, pathIds[0]), map))
                duskwoodDragon->GetMotionMaster()->MoveWaypoint(pathIds[0]);
        }
        if (IsDragonSpawned(m_emeraldDragonsChosenPositions[1]))
            WorldObject::SummonCreature(TempSpawnSettings(nullptr, m_emeraldDragonsChosenPositions[1], emeraldDragonSpawns[1][0], emeraldDragonSpawns[1][1], emeraldDragonSpawns[1][2], emeraldDragonSpawns[1][3], TEMPSPAWN_DEAD_DESPAWN, 0, false, false, pathIds[1]), map);
    });
    sMapMgr.DoForAllMapsWithMapId(1, [&](Map* map)
    {
        if (IsDragonSpawned(m_emeraldDragonsChosenPositions[2]))
            WorldObject::SummonCreature(TempSpawnSettings(nullptr, m_emeraldDragonsChosenPositions[2], emeraldDragonSpawns[2][0], emeraldDragonSpawns[2][1], emeraldDragonSpawns[2][2], emeraldDragonSpawns[2][3], TEMPSPAWN_DEAD_DESPAWN, 0, false, false, pathIds[2]), map);
        if (IsDragonSpawned(m_emeraldDragonsChosenPositions[3]))
            WorldObject::SummonCreature(TempSpawnSettings(nullptr, m_emeraldDragonsChosenPositions[3], emeraldDragonSpawns[3][0], emeraldDragonSpawns[3][1], emeraldDragonSpawns[3][2], emeraldDragonSpawns[3][3], TEMPSPAWN_DEAD_DESPAWN, 0, false, false, pathIds[3]), map);
    });
}

void WorldState::AddWarEffortProgress(AQResources resource, uint32 count)
{
    std::lock_guard<std::mutex> guard(m_aqData.m_warEffortMutex);
    if (m_aqData.m_phase != PHASE_1_GATHERING_RESOURCES)
        return;
    m_aqData.m_WarEffortCounters[resource] += count;
    Save(SAVE_ID_AHN_QIRAJ);
    for (ObjectGuid& guid : m_aqData.m_warEffortWorldstatesPlayers)
        if (Player* player = sObjectMgr.GetPlayer(guid))
            player->SendUpdateWorldState(aqWorldstateMap[resource], m_aqData.m_WarEffortCounters[resource]);
    uint32 id = uint32(resource);
    if (id >= aqWorldStateTotalsMap.size())
        id -= 5;
    ChangeWarEffortGoSpawns(resource);
    if (m_aqData.m_WarEffortCounters[resource] >= aqWorldStateTotalsMap[id].second) // fulfilled this condition - check all
    {
        bool success = true;
        for (uint32 i = 0; i < RESOURCE_MAX; ++i)
        {
            uint32 id = i > RESOURCE_UNIQUE_MAX ? i - 5 : i;
            if (m_aqData.m_WarEffortCounters[i] < aqWorldStateTotalsMap[id].second)
            {
                success = false;
                break;
            }
        }
        if (success)
            HandleWarEffortPhaseTransition(PHASE_2_TRANSPORTING_RESOURCES);
    }
}

void WorldState::HandleWarEffortPhaseTransition(uint32 newPhase)
{
    StopWarEffortEvent();
    m_aqData.m_phase = newPhase;
    switch (m_aqData.m_phase)
    {
        case PHASE_2_TRANSPORTING_RESOURCES:
            m_aqData.m_phase = PHASE_2_TRANSPORTING_RESOURCES;
            m_aqData.m_timer = 5 * DAY * IN_MILLISECONDS;
            {
                std::lock_guard<std::mutex> guard(m_aqData.m_warEffortMutex);
                for (ObjectGuid& guid : m_aqData.m_warEffortWorldstatesPlayers)
                    if (Player* player = sObjectMgr.GetPlayer(guid))
                        player->SendUpdateWorldState(WORLD_STATE_AQ_DAYS_LEFT, m_aqData.GetDaysRemaining());
            }
            break;
        case PHASE_4_10_HOUR_WAR:
            m_aqData.m_phase = PHASE_4_10_HOUR_WAR;
            m_aqData.m_timer = 10 * HOUR * IN_MILLISECONDS;
            break;
        default: break;
    }
    StartWarEffortEvent();
}

void WorldState::StartWarEffortEvent()
{
    switch (m_aqData.m_phase)
    {
        case PHASE_1_GATHERING_RESOURCES: sGameEventMgr.StartEvent(GAME_EVENT_AHN_QIRAJ_EFFORT_PHASE_1); break;
        case PHASE_2_TRANSPORTING_RESOURCES: sGameEventMgr.StartEvent(GAME_EVENT_AHN_QIRAJ_EFFORT_PHASE_2); break;
        case PHASE_3_GONG_TIME: sGameEventMgr.StartEvent(GAME_EVENT_AHN_QIRAJ_EFFORT_PHASE_3); break;
        case PHASE_4_10_HOUR_WAR: sGameEventMgr.StartEvent(GAME_EVENT_AHN_QIRAJ_EFFORT_PHASE_4); break;
        case PHASE_5_DONE: sGameEventMgr.StartEvent(GAME_EVENT_AHN_QIRAJ_EFFORT_PHASE_5); break;
        default: break;
    }
}

void WorldState::StopWarEffortEvent()
{
    switch (m_aqData.m_phase)
    {
        case PHASE_1_GATHERING_RESOURCES: sGameEventMgr.StopEvent(GAME_EVENT_AHN_QIRAJ_EFFORT_PHASE_1); break;
        case PHASE_2_TRANSPORTING_RESOURCES: sGameEventMgr.StopEvent(GAME_EVENT_AHN_QIRAJ_EFFORT_PHASE_2); break;
        case PHASE_3_GONG_TIME: sGameEventMgr.StopEvent(GAME_EVENT_AHN_QIRAJ_EFFORT_PHASE_3); break;
        case PHASE_4_10_HOUR_WAR: sGameEventMgr.StopEvent(GAME_EVENT_AHN_QIRAJ_EFFORT_PHASE_4); break;
        case PHASE_5_DONE: sGameEventMgr.StopEvent(GAME_EVENT_AHN_QIRAJ_EFFORT_PHASE_5); break;
        default: break;
    }
}

void WorldState::SpawnWarEffortGos()
{
    if (m_aqData.m_phase > PHASE_2_TRANSPORTING_RESOURCES)
        return;

    for (uint32 i = 0; i < RESOURCE_MAX; ++i)
        ChangeWarEffortGoSpawns(AQResources(i), m_aqData.m_phase == PHASE_2_TRANSPORTING_RESOURCES ? m_aqData.m_phase2Tier : -1);
}

enum AQResourceTier
{
    RESOURCE_TIER_0,
    RESOURCE_TIER_1,
    RESOURCE_TIER_2,
    RESOURCE_TIER_3,
    RESOURCE_TIER_4,
    RESOURCE_TIER_5,
};

struct WarEffortSpawn
{
    AQResourceGroup resourceGroup;
    AQResourceTier resourceTier;
    uint32 dbGuid;
    Team team;
};

enum
{
    WAR_EFFORT_DB_GUID_PREFIX = 155000,

    MAP_AZEROTH = 0,
    MAP_KALIMDOR = 1,
};

static std::map<std::pair<AQResourceGroup, Team>, std::vector<AQResources>> teamResourcesMap
{
    { {RESOURCE_GROUP_SKINNING, ALLIANCE}, {AQ_THICK_LEATHER_ALLY, AQ_LIGHT_LEATHER, AQ_MEDIUM_LEATHER} },
    { {RESOURCE_GROUP_COOKING, ALLIANCE}, {AQ_RAINBOW_FIN_ALBACORE, AQ_ROAST_RAPTOR, AQ_SPOTTED_YELLOWTAIL_ALLY} },
    { {RESOURCE_GROUP_HERBS, ALLIANCE}, {AQ_STRANGLEKELP, AQ_ARTHAS_TEARS, AQ_PURPLE_LOTUS_ALLY} },
    { {RESOURCE_GROUP_BANDAGES, ALLIANCE}, {AQ_LINEN_BANDAGE, AQ_SILK_BANDAGE, AQ_RUNECLOTH_BANDAGE_ALLY} },
    { {RESOURCE_GROUP_BARS, ALLIANCE}, {AQ_IRON_BAR, AQ_THORIUM_BAR, AQ_COPPER_BAR_ALLY} },
    { {RESOURCE_GROUP_SKINNING, HORDE}, {AQ_HEAVY_LEATHER, AQ_RUGGED_LEATHER, AQ_THICK_LEATHER_HORDE} },
    { {RESOURCE_GROUP_COOKING, HORDE}, {AQ_LEAN_WOLF_STEAK, AQ_BAKED_SALMON, AQ_SPOTTED_YELLOWTAIL_HORDE} },
    { {RESOURCE_GROUP_HERBS, HORDE}, {AQ_PEACEBLOOM, AQ_FIREBLOOM, AQ_PURPLE_LOTUS_HORDE} },
    { {RESOURCE_GROUP_BANDAGES, HORDE}, {AQ_WOOL_BANDAGE, AQ_MAGEWEAVE_BANDAGE, AQ_RUNECLOTH_BANDAGE_HORDE} },
    { {RESOURCE_GROUP_BARS, HORDE}, {AQ_TIN_BAR, AQ_MITHRIL_BAR, AQ_COPPER_BAR_HORDE} }
};

static WarEffortSpawn warEffortSpawns[] =
{
    { RESOURCE_GROUP_SKINNING, RESOURCE_TIER_1, WAR_EFFORT_DB_GUID_PREFIX + 10, ALLIANCE},
    { RESOURCE_GROUP_BANDAGES, RESOURCE_TIER_1, WAR_EFFORT_DB_GUID_PREFIX + 11, ALLIANCE},
    { RESOURCE_GROUP_BARS,     RESOURCE_TIER_1, WAR_EFFORT_DB_GUID_PREFIX + 12, ALLIANCE},
    { RESOURCE_GROUP_COOKING,  RESOURCE_TIER_1, WAR_EFFORT_DB_GUID_PREFIX + 13, ALLIANCE},
    { RESOURCE_GROUP_HERBS,    RESOURCE_TIER_1, WAR_EFFORT_DB_GUID_PREFIX + 14, ALLIANCE},
    { RESOURCE_GROUP_SKINNING, RESOURCE_TIER_2, WAR_EFFORT_DB_GUID_PREFIX + 20, ALLIANCE},
    { RESOURCE_GROUP_BANDAGES, RESOURCE_TIER_2, WAR_EFFORT_DB_GUID_PREFIX + 21, ALLIANCE},
    { RESOURCE_GROUP_BARS,     RESOURCE_TIER_2, WAR_EFFORT_DB_GUID_PREFIX + 22, ALLIANCE},
    { RESOURCE_GROUP_COOKING,  RESOURCE_TIER_2, WAR_EFFORT_DB_GUID_PREFIX + 23, ALLIANCE},
    { RESOURCE_GROUP_HERBS,    RESOURCE_TIER_2, WAR_EFFORT_DB_GUID_PREFIX + 24, ALLIANCE},
    { RESOURCE_GROUP_SKINNING, RESOURCE_TIER_3, WAR_EFFORT_DB_GUID_PREFIX + 30, ALLIANCE},
    { RESOURCE_GROUP_BANDAGES, RESOURCE_TIER_3, WAR_EFFORT_DB_GUID_PREFIX + 31, ALLIANCE},
    { RESOURCE_GROUP_BARS,     RESOURCE_TIER_3, WAR_EFFORT_DB_GUID_PREFIX + 32, ALLIANCE},
    { RESOURCE_GROUP_COOKING,  RESOURCE_TIER_3, WAR_EFFORT_DB_GUID_PREFIX + 33, ALLIANCE},
    { RESOURCE_GROUP_HERBS,    RESOURCE_TIER_3, WAR_EFFORT_DB_GUID_PREFIX + 34, ALLIANCE},
    { RESOURCE_GROUP_SKINNING, RESOURCE_TIER_4, WAR_EFFORT_DB_GUID_PREFIX + 40, ALLIANCE},
    { RESOURCE_GROUP_BANDAGES, RESOURCE_TIER_4, WAR_EFFORT_DB_GUID_PREFIX + 41, ALLIANCE},
    { RESOURCE_GROUP_BARS,     RESOURCE_TIER_4, WAR_EFFORT_DB_GUID_PREFIX + 42, ALLIANCE},
    { RESOURCE_GROUP_COOKING,  RESOURCE_TIER_4, WAR_EFFORT_DB_GUID_PREFIX + 43, ALLIANCE},
    { RESOURCE_GROUP_HERBS,    RESOURCE_TIER_4, WAR_EFFORT_DB_GUID_PREFIX + 44, ALLIANCE},
    { RESOURCE_GROUP_SKINNING, RESOURCE_TIER_5, WAR_EFFORT_DB_GUID_PREFIX + 50, ALLIANCE},
    { RESOURCE_GROUP_BANDAGES, RESOURCE_TIER_5, WAR_EFFORT_DB_GUID_PREFIX + 51, ALLIANCE},
    { RESOURCE_GROUP_BARS,     RESOURCE_TIER_5, WAR_EFFORT_DB_GUID_PREFIX + 52, ALLIANCE},
    { RESOURCE_GROUP_COOKING,  RESOURCE_TIER_5, WAR_EFFORT_DB_GUID_PREFIX + 53, ALLIANCE},
    { RESOURCE_GROUP_HERBS,    RESOURCE_TIER_5, WAR_EFFORT_DB_GUID_PREFIX + 54, ALLIANCE},
    { RESOURCE_GROUP_SKINNING, RESOURCE_TIER_1, WAR_EFFORT_DB_GUID_PREFIX + 510, HORDE},
    { RESOURCE_GROUP_BANDAGES, RESOURCE_TIER_1, WAR_EFFORT_DB_GUID_PREFIX + 511, HORDE},
    { RESOURCE_GROUP_BARS,     RESOURCE_TIER_1, WAR_EFFORT_DB_GUID_PREFIX + 512, HORDE},
    { RESOURCE_GROUP_COOKING,  RESOURCE_TIER_1, WAR_EFFORT_DB_GUID_PREFIX + 513, HORDE},
    { RESOURCE_GROUP_HERBS,    RESOURCE_TIER_1, WAR_EFFORT_DB_GUID_PREFIX + 514, HORDE},
    { RESOURCE_GROUP_SKINNING, RESOURCE_TIER_2, WAR_EFFORT_DB_GUID_PREFIX + 520, HORDE},
    { RESOURCE_GROUP_BANDAGES, RESOURCE_TIER_2, WAR_EFFORT_DB_GUID_PREFIX + 521, HORDE},
    { RESOURCE_GROUP_BARS,     RESOURCE_TIER_2, WAR_EFFORT_DB_GUID_PREFIX + 522, HORDE},
    { RESOURCE_GROUP_COOKING,  RESOURCE_TIER_2, WAR_EFFORT_DB_GUID_PREFIX + 523, HORDE},
    { RESOURCE_GROUP_HERBS,    RESOURCE_TIER_2, WAR_EFFORT_DB_GUID_PREFIX + 524, HORDE},
    { RESOURCE_GROUP_SKINNING, RESOURCE_TIER_3, WAR_EFFORT_DB_GUID_PREFIX + 530, HORDE},
    { RESOURCE_GROUP_BANDAGES, RESOURCE_TIER_3, WAR_EFFORT_DB_GUID_PREFIX + 531, HORDE},
    { RESOURCE_GROUP_BARS,     RESOURCE_TIER_3, WAR_EFFORT_DB_GUID_PREFIX + 532, HORDE},
    { RESOURCE_GROUP_COOKING,  RESOURCE_TIER_3, WAR_EFFORT_DB_GUID_PREFIX + 533, HORDE},
    { RESOURCE_GROUP_HERBS,    RESOURCE_TIER_3, WAR_EFFORT_DB_GUID_PREFIX + 534, HORDE},
    { RESOURCE_GROUP_SKINNING, RESOURCE_TIER_4, WAR_EFFORT_DB_GUID_PREFIX + 540, HORDE},
    { RESOURCE_GROUP_BANDAGES, RESOURCE_TIER_4, WAR_EFFORT_DB_GUID_PREFIX + 541, HORDE},
    { RESOURCE_GROUP_BARS,     RESOURCE_TIER_4, WAR_EFFORT_DB_GUID_PREFIX + 542, HORDE},
    { RESOURCE_GROUP_COOKING,  RESOURCE_TIER_4, WAR_EFFORT_DB_GUID_PREFIX + 543, HORDE},
    { RESOURCE_GROUP_HERBS,    RESOURCE_TIER_4, WAR_EFFORT_DB_GUID_PREFIX + 544, HORDE},
    { RESOURCE_GROUP_SKINNING, RESOURCE_TIER_5, WAR_EFFORT_DB_GUID_PREFIX + 550, HORDE},
    { RESOURCE_GROUP_BANDAGES, RESOURCE_TIER_5, WAR_EFFORT_DB_GUID_PREFIX + 551, HORDE},
    { RESOURCE_GROUP_BARS,     RESOURCE_TIER_5, WAR_EFFORT_DB_GUID_PREFIX + 552, HORDE},
    { RESOURCE_GROUP_COOKING,  RESOURCE_TIER_5, WAR_EFFORT_DB_GUID_PREFIX + 553, HORDE},
    { RESOURCE_GROUP_HERBS,    RESOURCE_TIER_5, WAR_EFFORT_DB_GUID_PREFIX + 554, HORDE},
};

AQResourceTier GetResourceTier(uint32 counter, uint32 max)
{
    if (max * 90 / 100 < counter)
        return RESOURCE_TIER_5;
    if (max * 70 / 100 < counter)
        return RESOURCE_TIER_4;
    if (max * 50 / 100 < counter)
        return RESOURCE_TIER_3;
    if (max * 30 / 100 < counter)
        return RESOURCE_TIER_2;
    if (max * 10 / 100 < counter)
        return RESOURCE_TIER_1;
    return RESOURCE_TIER_0;
}

void WorldState::ChangeWarEffortGoSpawns(AQResources resource, int32 forcedTier)
{
    auto resourceInfo = GetResourceInfo(resource);
    auto counterInfo = GetResourceCounterAndMax(resourceInfo.first, resourceInfo.second);
    auto tier = GetResourceTier(counterInfo.first, counterInfo.second);
    if (forcedTier != -1)
        tier = AQResourceTier(uint32(forcedTier));
    std::vector<uint32> spawnsForSchedule;
    for (auto& spawn : warEffortSpawns)
        if (resourceInfo.first == spawn.resourceGroup && tier >= spawn.resourceTier)
            if (m_aqData.m_spawnedDbGuids.find(spawn.dbGuid) == m_aqData.m_spawnedDbGuids.end())
                spawnsForSchedule.push_back(spawn.dbGuid);
    for (uint32 dbGuid : spawnsForSchedule)
        m_aqData.m_spawnedDbGuids.insert(dbGuid);

    sMapMgr.DoForAllMapsWithMapId(resourceInfo.second == ALLIANCE ? MAP_AZEROTH : MAP_KALIMDOR, [=](Map* map)
    {
        for (uint32 dbGuid : spawnsForSchedule)
        {
            WorldObject::SpawnGameObject(dbGuid, map);
        }
    });
}

void WorldState::ChangeWarEffortPhase2Tier(uint32 remainingDays)
{
    m_aqData.m_phase2Tier = remainingDays;
    std::set<std::pair<uint32, Team>> guidsForDespawn;
    for (uint32 dbGuid : m_aqData.m_spawnedDbGuids)
        for (auto& spawn : warEffortSpawns)
            if (spawn.dbGuid == dbGuid && spawn.resourceTier > remainingDays)
                guidsForDespawn.insert({ dbGuid, spawn.team});

    DespawnWarEffortGuids(guidsForDespawn);
    Save(SAVE_ID_AHN_QIRAJ);
}

void WorldState::DespawnWarEffortGuids(std::set<std::pair<uint32, Team>>& guids)
{
    std::vector<uint32> kalimdor;
    std::vector<uint32> ek;
    for (auto& data : guids)
    {
        if (data.second == ALLIANCE)
            ek.push_back(data.first);
        else
            kalimdor.push_back(data.first);
    }
    if (!ek.empty())
    {
        for (uint32 dbGuid : ek)
            m_aqData.m_spawnedDbGuids.erase(dbGuid);

        sMapMgr.DoForAllMapsWithMapId(MAP_AZEROTH, [=](Map* map)
        {
            for (uint32 dbGuid : ek)
            {
                if (GameObject* go = map->GetGameObject(dbGuid))
                    go->AddObjectToRemoveList();
            }
        });
    }
    if (!kalimdor.empty())
    {
        for (uint32 dbGuid : kalimdor)
            m_aqData.m_spawnedDbGuids.erase(dbGuid);

        sMapMgr.DoForAllMapsWithMapId(MAP_KALIMDOR, [=](Map* map)
        {
            for (uint32 dbGuid : kalimdor)
            {
                if (GameObject* go = map->GetGameObject(dbGuid))
                    go->AddObjectToRemoveList();
            }
        });
    }
}

std::pair<AQResourceGroup, Team> WorldState::GetResourceInfo(AQResources resource)
{
    switch (resource)
    {
        case AQ_RAINBOW_FIN_ALBACORE: return { RESOURCE_GROUP_COOKING, ALLIANCE};
        case AQ_ROAST_RAPTOR: return { RESOURCE_GROUP_COOKING, ALLIANCE };
        case AQ_SPOTTED_YELLOWTAIL_ALLY: return { RESOURCE_GROUP_COOKING, ALLIANCE };
        case AQ_SPOTTED_YELLOWTAIL_HORDE: return { RESOURCE_GROUP_COOKING, HORDE };
        case AQ_LEAN_WOLF_STEAK: return { RESOURCE_GROUP_COOKING, HORDE };
        case AQ_BAKED_SALMON: return { RESOURCE_GROUP_COOKING, HORDE };
        case AQ_PEACEBLOOM: return { RESOURCE_GROUP_HERBS, HORDE };
        case AQ_FIREBLOOM: return { RESOURCE_GROUP_HERBS, HORDE };
        case AQ_PURPLE_LOTUS_HORDE: return { RESOURCE_GROUP_HERBS, HORDE };
        case AQ_PURPLE_LOTUS_ALLY: return { RESOURCE_GROUP_HERBS, ALLIANCE };
        case AQ_STRANGLEKELP: return { RESOURCE_GROUP_HERBS, ALLIANCE };
        case AQ_ARTHAS_TEARS: return { RESOURCE_GROUP_HERBS, ALLIANCE };
        case AQ_TIN_BAR: return { RESOURCE_GROUP_BARS, HORDE };
        case AQ_MITHRIL_BAR: return { RESOURCE_GROUP_BARS, HORDE };
        case AQ_IRON_BAR: return { RESOURCE_GROUP_BARS, ALLIANCE };
        case AQ_THORIUM_BAR: return { RESOURCE_GROUP_BARS, ALLIANCE };
        case AQ_COPPER_BAR_ALLY: return { RESOURCE_GROUP_BARS, ALLIANCE };
        case AQ_COPPER_BAR_HORDE: return { RESOURCE_GROUP_BARS, HORDE };
        case AQ_HEAVY_LEATHER: return { RESOURCE_GROUP_SKINNING, HORDE };
        case AQ_RUGGED_LEATHER: return { RESOURCE_GROUP_SKINNING, HORDE };
        case AQ_THICK_LEATHER_HORDE: return { RESOURCE_GROUP_SKINNING, HORDE };
        case AQ_THICK_LEATHER_ALLY: return { RESOURCE_GROUP_SKINNING, ALLIANCE };
        case AQ_LIGHT_LEATHER: return { RESOURCE_GROUP_SKINNING, ALLIANCE };
        case AQ_MEDIUM_LEATHER: return { RESOURCE_GROUP_SKINNING, ALLIANCE };
        case AQ_LINEN_BANDAGE: return { RESOURCE_GROUP_BANDAGES, ALLIANCE };
        case AQ_SILK_BANDAGE: return { RESOURCE_GROUP_BANDAGES, ALLIANCE };
        case AQ_RUNECLOTH_BANDAGE_ALLY: return { RESOURCE_GROUP_BANDAGES, ALLIANCE };
        case AQ_RUNECLOTH_BANDAGE_HORDE: return { RESOURCE_GROUP_BANDAGES, HORDE };
        case AQ_WOOL_BANDAGE: return { RESOURCE_GROUP_BANDAGES, HORDE };
        case AQ_MAGEWEAVE_BANDAGE: return { RESOURCE_GROUP_BANDAGES, HORDE };
        default: return { RESOURCE_GROUP_HERBS, HORDE };
    }
}

std::pair<uint32, uint32> WorldState::GetResourceCounterAndMax(AQResourceGroup group, Team team)
{
    auto& data = teamResourcesMap[{group, team}];
    uint32 counter = 0;
    uint32 max = 0;
    for (auto& resource : data)
    {
        counter += m_aqData.m_WarEffortCounters[resource];
        max += aqWorldStateTotalsMap[resource > RESOURCE_UNIQUE_MAX ? resource - 5 : resource].second;
    }
    return {counter, max};
}

std::string WorldState::GetAQPrintout()
{
    std::string output = "Phase: " + std::to_string(m_aqData.m_phase) + " Timer: " + std::to_string(m_aqData.m_timer) + "\nValues:";
    for (uint32 value : m_aqData.m_WarEffortCounters)
        output += " " + std::to_string(value);
    return output;
}

std::string AhnQirajData::GetData()
{
    std::string output = std::to_string(m_phase) + " " + std::to_string(m_timer);
    for (uint32 value : m_WarEffortCounters)
        output += " " + std::to_string(value);
    output += " " + std::to_string(m_phase2Tier);
    return output;
}

uint32 AhnQirajData::GetDaysRemaining() const
{
    return uint32(m_timer / (DAY * IN_MILLISECONDS));
}
void WorldState::FillInitialWorldStates(ByteBuffer& data, uint32& count, uint32 zoneId)
{
    switch (zoneId)
    {
        case ZONEID_STORMWIND_CITY:
        case ZONEID_DARNASSUS:
        case ZONEID_IRONFORGE:
        case ZONEID_ORGRIMMAR:
        case ZONEID_THUNDER_BLUFF:
        case ZONEID_UNDERCITY:
        {
            if (sGameEventMgr.IsActiveHoliday(HOLIDAY_LOVE_IS_IN_THE_AIR))
            {
                uint32 allianceSum = GetLoveIsInTheAirCounter(LOVE_LEADER_BOLVAR) + GetLoveIsInTheAirCounter(LOVE_LEADER_TYRANDE) + GetLoveIsInTheAirCounter(LOVE_LEADER_MAGNI);
                uint32 hordeSum = GetLoveIsInTheAirCounter(LOVE_LEADER_CAIRNE) + GetLoveIsInTheAirCounter(LOVE_LEADER_THRALL) + GetLoveIsInTheAirCounter(LOVE_LEADER_SYLVANAS);
                FillInitialWorldStateData(data, count, WORLD_STATE_LOVE_IS_IN_THE_AIR_BOLVAR, GetLoveIsInTheAirCounter(LOVE_LEADER_BOLVAR));
                FillInitialWorldStateData(data, count, WORLD_STATE_LOVE_IS_IN_THE_AIR_TYRANDE, GetLoveIsInTheAirCounter(LOVE_LEADER_TYRANDE));
                FillInitialWorldStateData(data, count, WORLD_STATE_LOVE_IS_IN_THE_AIR_MAGNI, GetLoveIsInTheAirCounter(LOVE_LEADER_MAGNI));
                FillInitialWorldStateData(data, count, WORLD_STATE_LOVE_IS_IN_THE_AIR_TOTAL_ALLIANCE, allianceSum);
                FillInitialWorldStateData(data, count, WORLD_STATE_LOVE_IS_IN_THE_AIR_CAIRNE, GetLoveIsInTheAirCounter(LOVE_LEADER_CAIRNE));
                FillInitialWorldStateData(data, count, WORLD_STATE_LOVE_IS_IN_THE_AIR_THRALL, GetLoveIsInTheAirCounter(LOVE_LEADER_THRALL));
                FillInitialWorldStateData(data, count, WORLD_STATE_LOVE_IS_IN_THE_AIR_SYLVANAS, GetLoveIsInTheAirCounter(LOVE_LEADER_SYLVANAS));
                FillInitialWorldStateData(data, count, WORLD_STATE_LOVE_IS_IN_THE_AIR_TOTAL_HORDE, hordeSum);
            }

            if (m_aqData.m_phase == PHASE_1_GATHERING_RESOURCES)
            {
                // totals first
                for (auto itr = aqWorldStateTotalsMap.begin(); itr != aqWorldStateTotalsMap.end(); ++itr)
                    FillInitialWorldStateData(data, count, (*itr).first, (*itr).second);
                for (auto itr = aqWorldstateMap.begin(); itr != aqWorldstateMap.end(); ++itr)
                    FillInitialWorldStateData(data, count, (*itr).second, m_aqData.m_WarEffortCounters[(*itr).first]);
            }
            else if (m_aqData.m_phase == PHASE_2_TRANSPORTING_RESOURCES)
                FillInitialWorldStateData(data, count, WORLD_STATE_AQ_DAYS_LEFT, m_aqData.GetDaysRemaining());
            break;
        }
    }
}
