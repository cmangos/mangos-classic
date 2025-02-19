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
#include "AI/ScriptDevAI/scripts/world/scourge_invasion.h"
#include "Globals/ObjectAccessor.h"

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
                        uint32 state = 0xF;
                        loadStream >> state >> respawnTime;
                        m_emeraldDragonsState = (uint8)state; // done so that it reads an int, not a char
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
                            loadStream >> m_aqData.m_phase2Tier >> m_aqData.m_killedBosses;
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
                case SAVE_ID_SCOURGE_INVASION:
                    if (data.size())
                    {
                        try
                        {
                            uint32 state;
                            loadStream >> state;
                            m_siData.m_state = SIState(state);
                            for (uint32 i = 0; i < SI_TIMER_MAX; ++i)
                            {
                                uint64 time;
                                loadStream >> time;
                                m_siData.m_timers[i] = std::chrono::time_point_cast<std::chrono::milliseconds>(Clock::from_time_t(time));
                            }
                            loadStream >> m_siData.m_battlesWon >> m_siData.m_lastAttackZone;
                            for (uint32 i = 0; i < SI_REMAINING_MAX; ++i)
                                loadStream >> m_siData.m_remaining[i];
                        }
                        catch (std::exception& e)
                        {
                            sLog.outError("%s", e.what());
                            m_siData.Reset();
                        }
                    }
                    break;
            }
        }
        while (result->NextRow());
    }
    StartWarEffortEvent();
    SpawnWarEffortGos();
    if (m_siData.m_state == STATE_1_ENABLED)
    {
        StartScourgeInvasion(false);
        HandleDefendedZones();
    }
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
        case SAVE_ID_SCOURGE_INVASION:
        {
            std::string siData = m_siData.GetData();
            SaveHelper(siData, SAVE_ID_SCOURGE_INVASION);
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

void WorldState::HandleGameObjectUse(GameObject* go, Unit* /*user*/)
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

void WorldState::HandleGameObjectRevertState(GameObject* /*go*/)
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

    // Scourge Invasion
    switch (conditionId)
    {
        case WORLD_STATE_SCOURGE_WINTERSPRING:
            return GetSIRemaining(SI_REMAINING_WINTERSPRING) > 0;
        case WORLD_STATE_SCOURGE_AZSHARA:
            return GetSIRemaining(SI_REMAINING_AZSHARA) > 0;
        case WORLD_STATE_SCOURGE_BLASTED_LANDS:
            return GetSIRemaining(SI_REMAINING_BLASTED_LANDS) > 0;
        case WORLD_STATE_SCOURGE_BURNING_STEPPES:
            return GetSIRemaining(SI_REMAINING_BURNING_STEPPES) > 0;
        case WORLD_STATE_SCOURGE_TANARIS:
            return GetSIRemaining(SI_REMAINING_TANARIS) > 0;
        case WORLD_STATE_SCOURGE_EASTERN_PLAGUELANDS:
            return GetSIRemaining(SI_REMAINING_EASTERN_PLAGUELANDS) > 0;
    }

    return m_transportStates.at(conditionId) == state;
}

void WorldState::HandleConditionStateChange(uint32 conditionId, uint32 state)
{
    m_transportStates[conditionId] = state;
}

Map* WorldState::GetMap(uint32 mapId, Position const& invZone)
{
    Map* map = sMapMgr.FindMap(mapId);
    if (!map)
        sLog.outError("ScourgeInvasionEvent::GetMap found no map with mapId %u, x: %f, y: %f.", mapId, invZone.x, invZone.y);
    return map;
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

    if (m_siData.m_state != STATE_0_DISABLED)
    {
        if (m_siData.m_broadcastTimer <= diff)
        {
            m_siData.m_broadcastTimer = 10000;
            BroadcastSIWorldstates();

            if (m_siData.m_state == STATE_1_ENABLED)
            {
                for (auto& zone : m_siData.m_attackPoints)
                {
                    if (zone.second.zoneId == ZONEID_UNDERCITY)
                        StartNewCityAttackIfTime(SI_TIMER_UNDERCITY, zone.second.zoneId);
                    else if (zone.second.zoneId == ZONEID_STORMWIND_CITY)
                        StartNewCityAttackIfTime(SI_TIMER_STORMWIND, zone.second.zoneId);
                }

                auto now = sWorld.GetCurrentClockTime();
                for (auto& zone : m_siData.m_invasionPoints)
                    HandleActiveZone(GetTimerIdForZone(zone.second.zoneId), zone.second.zoneId, zone.second.remainingVar, now);
            }
        }
        else m_siData.m_broadcastTimer -= diff;
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

// War effort section

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
    Save(SAVE_ID_AHN_QIRAJ);
}

void WorldState::StartWarEffortEvent()
{
    switch (m_aqData.m_phase)
    {
        case PHASE_1_GATHERING_RESOURCES: sGameEventMgr.StartEvent(GAME_EVENT_AHN_QIRAJ_EFFORT_PHASE_1); break;
        case PHASE_2_TRANSPORTING_RESOURCES: sGameEventMgr.StartEvent(GAME_EVENT_AHN_QIRAJ_EFFORT_PHASE_2); break;
        case PHASE_3_GONG_TIME: sGameEventMgr.StartEvent(GAME_EVENT_AHN_QIRAJ_EFFORT_PHASE_3); break;
        case PHASE_4_10_HOUR_WAR:
            sGameEventMgr.StartEvent(GAME_EVENT_AHN_QIRAJ_EFFORT_PHASE_4);
            if (m_aqData.m_killedBosses & (1 << AQ_SILITHUS_BOSS_ASHI))
                sGameEventMgr.StartEvent(GAME_EVENT_ASHI_DEAD);
            if (m_aqData.m_killedBosses & (1 << AQ_SILITHUS_BOSS_REGAL))
                sGameEventMgr.StartEvent(GAME_EVENT_REGAL_DEAD);
            if (m_aqData.m_killedBosses & (1 << AQ_SILITHUS_BOSS_ZORA))
                sGameEventMgr.StartEvent(GAME_EVENT_ZORA_DEAD);
            break;
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
        case PHASE_4_10_HOUR_WAR:
            sGameEventMgr.StopEvent(GAME_EVENT_AHN_QIRAJ_EFFORT_PHASE_4);
            sGameEventMgr.StopEvent(GAME_EVENT_ASHI_DEAD);
            sGameEventMgr.StopEvent(GAME_EVENT_REGAL_DEAD);
            sGameEventMgr.StopEvent(GAME_EVENT_ZORA_DEAD);
            break;
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

enum AQResourceTier : uint32
{
    RESOURCE_TIER_0 = 0,
    RESOURCE_TIER_1 = 1,
    RESOURCE_TIER_2 = 2,
    RESOURCE_TIER_3 = 3,
    RESOURCE_TIER_4 = 4,
    RESOURCE_TIER_5 = 5,
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

void WorldState::SetSilithusBossKilled(AQSilithusBoss boss)
{
    std::lock_guard<std::mutex> guard(m_aqData.m_warEffortMutex);
    m_aqData.m_killedBosses |= (1 << boss);
    if (boss == AQ_SILITHUS_BOSS_ASHI)
        sGameEventMgr.StartEvent(GAME_EVENT_ASHI_DEAD);
    if (boss == AQ_SILITHUS_BOSS_REGAL)
        sGameEventMgr.StartEvent(GAME_EVENT_REGAL_DEAD);
    if (boss == AQ_SILITHUS_BOSS_ZORA)
        sGameEventMgr.StartEvent(GAME_EVENT_ZORA_DEAD);
    Save(SAVE_ID_AHN_QIRAJ);
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
    auto curTime = World::GetCurrentClockTime();
    auto respawnTime = std::chrono::milliseconds(m_timer) + curTime;
    std::string output = std::to_string(m_phase) + " " + std::to_string(uint64(Clock::to_time_t(respawnTime)));
    for (uint32 value : m_WarEffortCounters)
        output += " " + std::to_string(value);
    output += " " + std::to_string(m_phase2Tier) + " " + std::to_string(m_killedBosses);
    return output;
}

uint32 AhnQirajData::GetDaysRemaining() const
{
    return uint32(m_timer / (DAY * IN_MILLISECONDS)) + 1;
}

// Scourge invasion section

void WorldState::SetScourgeInvasionState(SIState state)
{
    SIState oldState = m_siData.m_state;
    if (oldState == state)
        return;
    
    m_siData.m_state = state;
    if (oldState == STATE_0_DISABLED)
        StartScourgeInvasion(true);
    else if (state == STATE_0_DISABLED)
        StopScourgeInvasion();
    Save(SAVE_ID_SCOURGE_INVASION);
}

void WorldState::StartScourgeInvasion(bool sendMail)
{
    sGameEventMgr.StartEvent(GAME_EVENT_SCOURGE_INVASION, false, !sendMail);
    BroadcastSIWorldstates();
    if (m_siData.m_state == STATE_1_ENABLED)
    {
        for (auto& zone : m_siData.m_attackPoints)
        {
            if (zone.second.zoneId == ZONEID_UNDERCITY)
                StartNewCityAttackIfTime(SI_TIMER_UNDERCITY, zone.second.zoneId);
            else if (zone.second.zoneId == ZONEID_STORMWIND_CITY)
                StartNewCityAttackIfTime(SI_TIMER_STORMWIND, zone.second.zoneId);
        }

        // randomization of init so that not every invasion starts the same way
        std::vector<uint32> randomIds(m_siData.m_invasionPoints.size());
        uint32 i = 0;
        for (auto& zone : m_siData.m_invasionPoints)
        {
            randomIds[i] = zone.first;
            ++i;
        }
        std::shuffle(randomIds.begin(), randomIds.end(), *GetRandomGenerator());
        for (auto id : randomIds)
            OnEnable(m_siData.m_invasionPoints[id]);
    }
}

ScourgeInvasionData::ScourgeInvasionData() : m_state(STATE_0_DISABLED), m_battlesWon(0), m_broadcastTimer(10000)
{
    memset(m_remaining, 0, sizeof(m_remaining));

    InvasionZone winterspring;
    {
        winterspring.map = 1;
        winterspring.zoneId = ZONEID_WINTERSPRING;
        winterspring.remainingVar = SI_REMAINING_WINTERSPRING;
        winterspring.necroAmount = 3;
        winterspring.mouth.push_back(Position(7736.56f, -4033.75f, 696.327f, 5.51524f));
    }

    InvasionZone tanaris;
    {
        tanaris.map = 1;
        tanaris.zoneId = ZONEID_TANARIS;
        tanaris.remainingVar = SI_REMAINING_TANARIS;
        tanaris.necroAmount = 3;
        tanaris.mouth.push_back(Position(-8352.68f, -3972.68f, 10.0753f, 2.14675f));
    }

    InvasionZone azshara;
    {
        azshara.map = 1;
        azshara.zoneId = ZONEID_AZSHARA;
        azshara.remainingVar = SI_REMAINING_AZSHARA;
        azshara.necroAmount = 2;
        azshara.mouth.push_back(Position(3273.75f, -4276.98f, 125.509f, 5.44543f));
    }

    InvasionZone blasted_lands;
    {
        blasted_lands.map = 0;
        blasted_lands.zoneId = ZONEID_BLASTED_LANDS;
        blasted_lands.remainingVar = SI_REMAINING_BLASTED_LANDS;
        blasted_lands.necroAmount = 2;
        blasted_lands.mouth.push_back(Position(-11429.3f, -3327.82f, 7.73628f, 1.0821f));
    }

    InvasionZone eastern_plaguelands;
    {
        eastern_plaguelands.map = 0;
        eastern_plaguelands.zoneId = ZONEID_EASTERN_PLAGUELANDS;
        eastern_plaguelands.remainingVar = SI_REMAINING_EASTERN_PLAGUELANDS;
        eastern_plaguelands.necroAmount = 2;
        eastern_plaguelands.mouth.push_back(Position(2014.55f, -4934.52f, 73.9846f, 0.0698132f));
    }

    InvasionZone burning_steppes;
    {
        burning_steppes.map = 0;
        burning_steppes.zoneId = ZONEID_BURNING_STEPPES;
        burning_steppes.remainingVar = SI_REMAINING_BURNING_STEPPES;
        burning_steppes.necroAmount = 2;
        burning_steppes.mouth.push_back(Position(-8229.53f, -1118.11f, 144.012f, 6.17846f));
    }

    m_invasionPoints.emplace(ZONEID_WINTERSPRING, winterspring);
    m_invasionPoints.emplace(ZONEID_TANARIS, tanaris);
    m_invasionPoints.emplace(ZONEID_AZSHARA, azshara);
    m_invasionPoints.emplace(ZONEID_BLASTED_LANDS, blasted_lands);
    m_invasionPoints.emplace(ZONEID_EASTERN_PLAGUELANDS, eastern_plaguelands);
    m_invasionPoints.emplace(ZONEID_BURNING_STEPPES, burning_steppes);

    CityAttack undercity;
    {
        undercity.map = 0;
        undercity.zoneId = ZONEID_UNDERCITY;
        undercity.pallid.push_back(Position(1595.87f, 440.539f, -46.3349f, 2.28207f)); // Royal Quarter
        undercity.pallid.push_back(Position(1659.2f, 265.988f, -62.1788f, 3.64283f)); // Trade Quarter
    }

    CityAttack stormwind;
    {
        stormwind.map = 0;
        stormwind.zoneId = ZONEID_STORMWIND_CITY;
        stormwind.pallid.push_back(Position(-8578.15f, 886.382f, 87.3148f, 0.586275f)); // Stormwind Keep
        stormwind.pallid.push_back(Position(-8578.15f, 886.382f, 87.3148f, 0.586275f)); // Trade District
    }

    m_attackPoints.emplace(ZONEID_UNDERCITY, undercity);
    m_attackPoints.emplace(ZONEID_STORMWIND_CITY, stormwind);
}

void ScourgeInvasionData::Reset()
{
    std::lock_guard<std::mutex> guard(m_siMutex);
    for (auto& timepoint : m_timers)
        timepoint = TimePoint();

    m_battlesWon = 0;
    m_lastAttackZone = 0;
    m_broadcastTimer = 10000;
    memset(m_remaining, 0, sizeof(m_remaining));
}

std::string ScourgeInvasionData::GetData()
{
    std::string output = std::to_string(m_state) + " ";
    for (auto& timer : m_timers)
        output += std::to_string(timer.time_since_epoch().count()) + " ";
    output += std::to_string(m_battlesWon) + " " + std::to_string(m_lastAttackZone) + " ";
    for (auto& remaining : m_remaining)
        output += std::to_string(remaining) + " ";
    return output;
}

void WorldState::StopScourgeInvasion()
{
    sGameEventMgr.StopEvent(GAME_EVENT_SCOURGE_INVASION);
    sGameEventMgr.StopEvent(GAME_EVENT_SCOURGE_INVASION_WINTERSPRING);
    sGameEventMgr.StopEvent(GAME_EVENT_SCOURGE_INVASION_TANARIS);
    sGameEventMgr.StopEvent(GAME_EVENT_SCOURGE_INVASION_AZSHARA);
    sGameEventMgr.StopEvent(GAME_EVENT_SCOURGE_INVASION_BLASTED_LANDS);
    sGameEventMgr.StopEvent(GAME_EVENT_SCOURGE_INVASION_EASTERN_PLAGUELANDS);
    sGameEventMgr.StopEvent(GAME_EVENT_SCOURGE_INVASION_BURNING_STEPPES);
    BroadcastSIWorldstates();
    m_siData.Reset();

    for (auto& zone : m_siData.m_attackPoints)
        OnDisable(zone.second);

    for (auto& zone : m_siData.m_invasionPoints)
        OnDisable(zone.second);
}

uint32 WorldState::GetSIRemaining(SIRemaining remaining) const
{
    return m_siData.m_remaining[remaining];
}

uint32 WorldState::GetSIRemainingByZone(uint32 zoneId) const
{
    SIRemaining remainingId;
    switch (zoneId)
    {
        default:
        case ZONEID_WINTERSPRING: remainingId = SI_REMAINING_WINTERSPRING; break;
        case ZONEID_AZSHARA: remainingId = SI_REMAINING_AZSHARA; break;
        case ZONEID_EASTERN_PLAGUELANDS: remainingId = SI_REMAINING_EASTERN_PLAGUELANDS; break;
        case ZONEID_BLASTED_LANDS: remainingId = SI_REMAINING_BLASTED_LANDS; break;
        case ZONEID_BURNING_STEPPES: remainingId = SI_REMAINING_BURNING_STEPPES; break;
        case ZONEID_TANARIS: remainingId = SI_REMAINING_TANARIS; break;
    }
    return GetSIRemaining(remainingId);
}

void WorldState::SetSIRemaining(SIRemaining remaining, uint32 value)
{
    std::lock_guard<std::mutex> guard(m_siData.m_siMutex);
    m_siData.m_remaining[remaining] = value;
    Save(SAVE_ID_SCOURGE_INVASION);
}

TimePoint WorldState::GetSITimer(SITimers timer)
{
    return m_siData.m_timers[timer];
}

void WorldState::SetSITimer(SITimers timer, TimePoint timePoint)
{
    m_siData.m_timers[timer] = timePoint;
}

uint32 WorldState::GetBattlesWon()
{
    std::lock_guard<std::mutex> guard(m_siData.m_siMutex);
    return m_siData.m_battlesWon;
}

void WorldState::AddBattlesWon(int32 count)
{
    std::lock_guard<std::mutex> guard(m_siData.m_siMutex);
    m_siData.m_battlesWon += count;
    HandleDefendedZones();
    Save(SAVE_ID_SCOURGE_INVASION);
}

uint32 WorldState::GetLastAttackZone()
{
    std::lock_guard<std::mutex> guard(m_siData.m_siMutex);
    return m_siData.m_lastAttackZone;
}

void WorldState::SetLastAttackZone(uint32 zoneId)
{
    std::lock_guard<std::mutex> guard(m_siData.m_siMutex);
    m_siData.m_lastAttackZone = zoneId;
}

void WorldState::BroadcastSIWorldstates()
{
    uint32 victories = GetBattlesWon();

    uint32 remainingAzshara = GetSIRemaining(SI_REMAINING_AZSHARA);
    uint32 remainingBlastedLands = GetSIRemaining(SI_REMAINING_BLASTED_LANDS);
    uint32 remainingBurningSteppes = GetSIRemaining(SI_REMAINING_BURNING_STEPPES);
    uint32 remainingEasternPlaguelands = GetSIRemaining(SI_REMAINING_EASTERN_PLAGUELANDS);
    uint32 remainingTanaris = GetSIRemaining(SI_REMAINING_TANARIS);
    uint32 remainingWinterspring = GetSIRemaining(SI_REMAINING_WINTERSPRING);

    HashMapHolder<Player>::MapType& m = sObjectAccessor.GetPlayers();
    for (const auto& itr : m)
    {
        Player* pl = itr.second;
        // do not process players which are not in world
        if (!pl->IsInWorld())
            continue;

        pl->SendUpdateWorldState(WORLD_STATE_SCOURGE_AZSHARA, remainingAzshara > 0 ? 1 : 0);
        pl->SendUpdateWorldState(WORLD_STATE_SCOURGE_BLASTED_LANDS, remainingBlastedLands > 0 ? 1 : 0);
        pl->SendUpdateWorldState(WORLD_STATE_SCOURGE_BURNING_STEPPES, remainingBurningSteppes > 0 ? 1 : 0);
        pl->SendUpdateWorldState(WORLD_STATE_SCOURGE_EASTERN_PLAGUELANDS, remainingEasternPlaguelands > 0 ? 1 : 0);
        pl->SendUpdateWorldState(WORLD_STATE_SCOURGE_TANARIS, remainingTanaris > 0 ? 1 : 0);
        pl->SendUpdateWorldState(WORLD_STATE_SCOURGE_WINTERSPRING, remainingWinterspring > 0 ? 1 : 0);

        pl->SendUpdateWorldState(WORLD_STATE_SCOURGE_BATTLES_WON, victories);
        pl->SendUpdateWorldState(WORLD_STATE_SCOURGE_NECROPOLIS_AZSHARA, remainingAzshara);
        pl->SendUpdateWorldState(WORLD_STATE_SCOURGE_NECROPOLIS_BLASTED_LANDS, remainingBlastedLands);
        pl->SendUpdateWorldState(WORLD_STATE_SCOURGE_NECROPOLIS_BURNING_STEPPES, remainingBurningSteppes);
        pl->SendUpdateWorldState(WORLD_STATE_SCOURGE_NECROPOLIS_EASTERN_PLAGUELANDS, remainingEasternPlaguelands);
        pl->SendUpdateWorldState(WORLD_STATE_SCOURGE_NECROPOLIS_TANARIS, remainingTanaris);
        pl->SendUpdateWorldState(WORLD_STATE_SCOURGE_NECROPOLIS_WINTERSPRING, remainingWinterspring);
    }
}

void WorldState::HandleDefendedZones()
{
    if (m_siData.m_battlesWon < 50) {
        sGameEventMgr.StopEvent(GAME_EVENT_SCOURGE_INVASION_50_INVASIONS);
        sGameEventMgr.StopEvent(GAME_EVENT_SCOURGE_INVASION_100_INVASIONS);
        sGameEventMgr.StopEvent(GAME_EVENT_SCOURGE_INVASION_150_INVASIONS);
    }
    else if (m_siData.m_battlesWon >= 50 && m_siData.m_battlesWon < 100)
        sGameEventMgr.StartEvent(GAME_EVENT_SCOURGE_INVASION_50_INVASIONS);
    else if (m_siData.m_battlesWon >= 100 && m_siData.m_battlesWon < 150) {
        sGameEventMgr.StopEvent(GAME_EVENT_SCOURGE_INVASION_50_INVASIONS);
        sGameEventMgr.StartEvent(GAME_EVENT_SCOURGE_INVASION_100_INVASIONS);
    }
    else if (m_siData.m_battlesWon >= 150) {
        sGameEventMgr.StopEvent(GAME_EVENT_SCOURGE_INVASION);
        sGameEventMgr.StopEvent(GAME_EVENT_SCOURGE_INVASION_50_INVASIONS);
        sGameEventMgr.StopEvent(GAME_EVENT_SCOURGE_INVASION_100_INVASIONS);
        sGameEventMgr.StartEvent(GAME_EVENT_SCOURGE_INVASION_INVASIONS_DONE);
        sGameEventMgr.StartEvent(GAME_EVENT_SCOURGE_INVASION_150_INVASIONS);
    }
}

void WorldState::StartZoneEvent(SIZoneIds eventId)
{
    switch (eventId)
    {
        default:
        case SI_ZONE_AZSHARA: StartNewInvasion(ZONEID_AZSHARA); break;
        case SI_ZONE_BLASTED_LANDS: StartNewInvasion(ZONEID_BLASTED_LANDS); break;
        case SI_ZONE_BURNING_STEPPES: StartNewInvasion(ZONEID_BURNING_STEPPES); break;
        case SI_ZONE_EASTERN_PLAGUELANDS: StartNewInvasion(ZONEID_EASTERN_PLAGUELANDS); break;
        case SI_ZONE_TANARIS: StartNewInvasion(ZONEID_TANARIS); break;
        case SI_ZONE_WINTERSPRING: StartNewInvasion(ZONEID_WINTERSPRING); break;
        case SI_ZONE_STORMWIND: StartNewCityAttack(ZONEID_STORMWIND_CITY); break;
        case SI_ZONE_UNDERCITY: StartNewCityAttack(ZONEID_UNDERCITY); break;
    }
}

void WorldState::StartNewInvasionIfTime(uint32 attackTimeVar, uint32 zoneId)
{
    TimePoint now = sWorld.GetCurrentClockTime();

    // Not yet time
    if (now < sWorldState.GetSITimer(SITimers(attackTimeVar)))
        return;

    StartNewInvasion(zoneId);
}

void WorldState::StartNewCityAttackIfTime(uint32 attackTimeVar, uint32 zoneId)
{
    TimePoint now = sWorld.GetCurrentClockTime();

    // Not yet time
    if (now < sWorldState.GetSITimer(SITimers(attackTimeVar)))
        return;

    StartNewCityAttack(zoneId);
    uint32 cityAttackTimer = urand(CITY_ATTACK_TIMER_MIN, CITY_ATTACK_TIMER_MAX);
    TimePoint next_attack = now + std::chrono::seconds(cityAttackTimer);
    sWorldState.SetSITimer(SITimers(attackTimeVar), next_attack);
}

void WorldState::StartNewInvasion(uint32 zoneId)
{
    if (IsActiveZone(zoneId))
        return;

    // Don't attack same zone as before.
    if (zoneId == sWorldState.GetLastAttackZone())
        return;

    // If we have at least one victory and more than 1 active zones stop here.
    if (GetActiveZones() > 1 && sWorldState.GetBattlesWon() > 0)
        return;

    sLog.outBasic("[Scourge Invasion Event] Starting new invasion in %d.", zoneId);

    ScourgeInvasionData::InvasionZone& zone = m_siData.m_invasionPoints[zoneId];

    Map* mapPtr = GetMap(zone.map, zone.mouth[0]);

    if (!mapPtr)
    {
        sLog.outError("ScourgeInvasionEvent::StartNewInvasionIfTime unable to access required map (%d). Retrying next update.", zone.map);
        return;
    }

    switch (zoneId)
    {
        case ZONEID_AZSHARA: sGameEventMgr.StartEvent(GAME_EVENT_SCOURGE_INVASION_AZSHARA); break;
        case ZONEID_BLASTED_LANDS: sGameEventMgr.StartEvent(GAME_EVENT_SCOURGE_INVASION_BLASTED_LANDS); break;
        case ZONEID_BURNING_STEPPES: sGameEventMgr.StartEvent(GAME_EVENT_SCOURGE_INVASION_BURNING_STEPPES); break;
        case ZONEID_EASTERN_PLAGUELANDS: sGameEventMgr.StartEvent(GAME_EVENT_SCOURGE_INVASION_EASTERN_PLAGUELANDS); break;
        case ZONEID_TANARIS: sGameEventMgr.StartEvent(GAME_EVENT_SCOURGE_INVASION_TANARIS); break;
        case ZONEID_WINTERSPRING: sGameEventMgr.StartEvent(GAME_EVENT_SCOURGE_INVASION_WINTERSPRING); break;
    }

    if (mapPtr)
        SummonMouth(mapPtr, zone, zone.mouth[0], true);
}

void WorldState::StartNewCityAttack(uint32 zoneId)
{
    sLog.outBasic("[Scourge Invasion Event] Starting new City attack in zone %d.", zoneId);

    ScourgeInvasionData::CityAttack& zone = m_siData.m_attackPoints[zoneId];

    uint32 SpawnLocationID = (urand(0, zone.pallid.size() - 1));

    Map* mapPtr = GetMap(zone.map, zone.pallid[SpawnLocationID]);

    // If any of the required maps are not available we return. Will cause the invasion to be started
    // on next update instead
    if (!mapPtr)
    {
        sLog.outError("ScourgeInvasionEvent::StartNewCityAttackIfTime unable to access required map (%d). Retrying next update.", zone.map);
        return;
    }

    if (m_siData.m_pendingPallids.find(zoneId) != m_siData.m_pendingPallids.end())
        return;

    /*if (!m_siData.m_attackPoints[zoneId].pallidGuid.IsEmpty())
        return;*/

    if (mapPtr && SummonPallid(mapPtr, zone, zone.pallid[SpawnLocationID], SpawnLocationID))
        sLog.outBasic("[Scourge Invasion Event] Pallid Horror summoned in zone %d.", zoneId);
    else
        sLog.outError("ScourgeInvasionEvent::StartNewCityAttackIfTime unable to spawn pallid in %d.", zone.map);
}

bool WorldState::ResumeInvasion(ScourgeInvasionData::InvasionZone& zone)
{
    // Dont have a save variable to know which necropolises had already been destroyed, so we
    // just summon the same amount, but not necessarily the same necropolises
    sLog.outBasic("[Scourge Invasion Event] Resuming Scourge invasion in zone %d", zone.zoneId);

    uint32 num_necropolises_remaining = sWorldState.GetSIRemaining(SIRemaining(zone.remainingVar));

    // Just making sure we can access all maps before starting the invasion
    for (uint32 i = 0; i < num_necropolises_remaining; i++)
    {
        if (!GetMap(zone.map, zone.mouth[0]))
        {
            sLog.outError("ScourgeInvasionEvent::ResumeInvasion map %d not accessible. Retry next update.", zone.map);
            return false;
        }
    }

    Map* mapPtr = GetMap(zone.map, zone.mouth[0]);
    if (!mapPtr)
    {
        sLog.outError("ScourgeInvasionEvent::ResumeInvasion failed getting map, even after making sure they were loaded....");
        return false;
    }

    SummonMouth(mapPtr, zone, zone.mouth[0], false);

    return true;
}

bool WorldState::SummonMouth(Map* map, ScourgeInvasionData::InvasionZone& zone, Position position, bool newInvasion)
{
    AddPendingInvasion(zone.zoneId);
    map->GetMessager().AddMessage([=](Map* map)
    {
        // Remove old mouth if required.
        Creature* existingMouth = map->GetCreature(zone.mouthGuid);

        if (existingMouth)
            existingMouth->RemoveFromWorld();

        if (Creature* mouth = WorldObject::SummonCreature(TempSpawnSettings(nullptr, NPC_MOUTH_OF_KELTHUZAD, position.x, position.y, position.z, position.o, TEMPSPAWN_DEAD_DESPAWN, 0, true), map))
        {
            mouth->AI()->SendAIEvent(AI_EVENT_CUSTOM_A, mouth, mouth, EVENT_MOUTH_OF_KELTHUZAD_ZONE_START);
            sWorldState.SetMouthGuid(zone.zoneId, mouth->GetObjectGuid());
            if (newInvasion)
                sWorldState.SetSIRemaining(SIRemaining(zone.remainingVar), zone.necroAmount);
        }
        sWorldState.RemovePendingInvasion(zone.zoneId);
    });

    return true;
}

bool WorldState::SummonPallid(Map* map, ScourgeInvasionData::CityAttack& zone, Position position, uint32 spawnLoc)
{
    AddPendingPallid(zone.zoneId);
    map->GetMessager().AddMessage([=](Map* map)
    {
        // Remove old pallid if required.
        Creature* existingPallid = map->GetCreature(zone.pallidGuid);
        uint32 pathID = 0;

        if (existingPallid)
            existingPallid->RemoveFromWorld();

        if (Creature* pallid = WorldObject::SummonCreature(TempSpawnSettings(nullptr, PickRandomValue(NPC_PALLID_HORROR, NPC_PATCHWORK_TERROR), position.x, position.y, position.z, position.o, TEMPSPAWN_DEAD_DESPAWN, 0, true), map))
        {
            pallid->GetMotionMaster()->Clear(false, true);
            if (pallid->GetZoneId() == ZONEID_UNDERCITY)
                pathID = spawnLoc == 0 ? 1 : 0;
            else
                pathID = spawnLoc == 0 ? 2 : 3;

            pallid->GetMotionMaster()->MoveWaypoint(pathID, PATH_FROM_ENTRY);

            sWorldState.SetPallidGuid(zone.zoneId, pallid->GetObjectGuid());
        }
        sWorldState.RemovePendingPallid(zone.zoneId);
    });

    return true;
}

void WorldState::HandleActiveZone(uint32 attackTimeVar, uint32 zoneId, uint32 remainingVar, TimePoint now)
{
    TimePoint timePoint = sWorldState.GetSITimer(SITimers(attackTimeVar));

    ScourgeInvasionData::InvasionZone& zone = m_siData.m_invasionPoints[zoneId];

    Map* map = sMapMgr.FindMap(zone.map);

    if (zone.zoneId != zoneId)
        return;

    uint32 remaining = GetSIRemaining(SIRemaining(remainingVar));

    // Calculate the next possible attack between ZONE_ATTACK_TIMER_MIN and ZONE_ATTACK_TIMER_MAX.
    uint32 zoneAttackTimer = urand(ZONE_ATTACK_TIMER_MIN, ZONE_ATTACK_TIMER_MAX);
    TimePoint next_attack = now + std::chrono::seconds(zoneAttackTimer);
    uint64 timeToNextAttack = std::chrono::duration_cast<std::chrono::minutes>(next_attack-now).count();

    if (zone.mouthGuid)
    {
        map->GetMessager().AddMessage([=](Map* map)
        {
            // Handles the inactive zone, without a Mouth of Kel'Thuzad summoned (which spawns the whole zone event).
            Creature* mouth = map->GetCreature(zone.mouthGuid);
            if (!mouth)
                sWorldState.SetMouthGuid(zone.zoneId, ObjectGuid()); // delays spawning until next tick
            // Handles the active zone that has no necropolis left.
            else if (timePoint < now && remaining == 0)
            {
                sWorldState.SetSITimer(SITimers(attackTimeVar), next_attack);
                sWorldState.AddBattlesWon(1);
                sWorldState.SetLastAttackZone(zoneId);

                sLog.outBasic("[Scourge Invasion Event] The Scourge has been defeated in %d, next attack starting in %ld minutes.", zoneId, timeToNextAttack);
                sLog.outBasic("[Scourge Invasion Event] %d victories", sWorldState.GetBattlesWon());

                if (mouth)
                    mouth->AI()->SendAIEvent(AI_EVENT_CUSTOM_A, mouth, mouth, EVENT_MOUTH_OF_KELTHUZAD_ZONE_STOP);
                else
                    sLog.outError("ScourgeInvasionEvent::HandleActiveZone ObjectGuid %llu not found.", (long long unsigned)zone.mouthGuid.GetRawValue());
            }
        });
    }
    else
    {
        // If more than one zones are alreay being attacked, set the timer again to ZONE_ATTACK_TIMER.
        if (GetActiveZones() > 1)
            sWorldState.SetSITimer(SITimers(attackTimeVar), next_attack);

        // Try to start the zone if attackTimeVar is 0.
        StartNewInvasionIfTime(attackTimeVar, zoneId);
    }
}

void WorldState::SetPallidGuid(uint32 zoneId, ObjectGuid guid)
{
    m_siData.m_attackPoints[zoneId].pallidGuid = guid;
}

void WorldState::SetMouthGuid(uint32 zoneId, ObjectGuid guid)
{
    m_siData.m_invasionPoints[zoneId].mouthGuid = guid;
}

void WorldState::AddPendingInvasion(uint32 zoneId)
{
    std::lock_guard<std::mutex> guard(m_siData.m_siMutex);
    m_siData.m_pendingInvasions.insert(zoneId);
}

void WorldState::RemovePendingInvasion(uint32 zoneId)
{
    std::lock_guard<std::mutex> guard(m_siData.m_siMutex);
    m_siData.m_pendingInvasions.erase(zoneId);
}

void WorldState::AddPendingPallid(uint32 zoneId)
{
    std::lock_guard<std::mutex> guard(m_siData.m_siMutex);
    m_siData.m_pendingPallids.insert(zoneId);
}

void WorldState::RemovePendingPallid(uint32 zoneId)
{
    std::lock_guard<std::mutex> guard(m_siData.m_siMutex);
    m_siData.m_pendingPallids.erase(zoneId);
}

void WorldState::OnEnable(ScourgeInvasionData::InvasionZone& zone)
{
    // If there were remaining necropolises in the old zone before shutdown, we
    // restore that zone
    if (sWorldState.GetSIRemaining(SIRemaining(zone.remainingVar)) > 0)
        ResumeInvasion(zone);
    // Otherwise we start a new Invasion
    else
        StartNewInvasionIfTime(GetTimerIdForZone(zone.zoneId), zone.zoneId);
}

void WorldState::OnDisable(ScourgeInvasionData::InvasionZone& zone)
{
    if (!zone.mouthGuid)
        return;

    Map* map = GetMap(zone.map, zone.mouth[0]);

    map->GetMessager().AddMessage([guid = zone.mouthGuid](Map* map)
    {
        if (Creature* mouth = map->GetCreature(guid))
            mouth->ForcedDespawn();
    });
}

void WorldState::OnDisable(ScourgeInvasionData::CityAttack& zone)
{
    if (!zone.pallidGuid)
        return;

    Map* map = GetMap(zone.map, zone.pallid[0]);

    map->GetMessager().AddMessage([guid = zone.pallidGuid](Map* map)
    {
        if (Creature* pallid = map->GetCreature(guid))
            pallid->ForcedDespawn();
    });
}

bool WorldState::IsActiveZone(uint32 zoneId)
{
    return false;
}

uint32 WorldState::GetActiveZones()
{
    int i = 0;
    i += m_siData.m_pendingInvasions.size();
    for (const auto& invasionPoint : m_siData.m_invasionPoints)
    {
        Map* mapPtr = GetMap(invasionPoint.second.map, invasionPoint.second.mouth[0]);
        if (!mapPtr)
        {
            sLog.outError("ScourgeInvasionEvent::GetActiveZones no map for zone %d.", invasionPoint.second.map);
            continue;
        }

        Creature* pMouth = mapPtr->GetCreature(invasionPoint.second.mouthGuid);
        if (pMouth)
            i++;
    }
    return i;
}

uint32 WorldState::GetTimerIdForZone(uint32 zoneId)
{
    uint32 attackTime = 0;
    switch (zoneId)
    {
        case ZONEID_TANARIS: attackTime = SI_TIMER_TANARIS; break;
        case ZONEID_BLASTED_LANDS: attackTime = SI_TIMER_BLASTED_LANDS; break;
        case ZONEID_EASTERN_PLAGUELANDS: attackTime = SI_TIMER_EASTERN_PLAGUELANDS; break;
        case ZONEID_BURNING_STEPPES: attackTime = SI_TIMER_BURNING_STEPPES; break;
        case ZONEID_WINTERSPRING: attackTime = SI_TIMER_WINTERSPRING; break;
        case ZONEID_AZSHARA: attackTime = SI_TIMER_AZSHARA; break;
    }
    return attackTime;
}

void WorldState::FillInitialWorldStates(ByteBuffer& data, uint32& count, uint32 zoneId)
{
    if (m_siData.m_state != STATE_0_DISABLED) // scourge invasion active - need to send all worldstates
    {
        uint32 victories = GetBattlesWon();

        uint32 remainingAzshara = GetSIRemaining(SI_REMAINING_AZSHARA);
        uint32 remainingBlastedLands = GetSIRemaining(SI_REMAINING_BLASTED_LANDS);
        uint32 remainingBurningSteppes = GetSIRemaining(SI_REMAINING_BURNING_STEPPES);
        uint32 remainingEasternPlaguelands = GetSIRemaining(SI_REMAINING_EASTERN_PLAGUELANDS);
        uint32 remainingTanaris = GetSIRemaining(SI_REMAINING_TANARIS);
        uint32 remainingWinterspring = GetSIRemaining(SI_REMAINING_WINTERSPRING);

        FillInitialWorldStateData(data, count, WORLD_STATE_SCOURGE_AZSHARA, remainingAzshara > 0 ? 1 : 0);
        FillInitialWorldStateData(data, count, WORLD_STATE_SCOURGE_BLASTED_LANDS, remainingBlastedLands > 0 ? 1 : 0);
        FillInitialWorldStateData(data, count, WORLD_STATE_SCOURGE_BURNING_STEPPES, remainingBurningSteppes > 0 ? 1 : 0);
        FillInitialWorldStateData(data, count, WORLD_STATE_SCOURGE_EASTERN_PLAGUELANDS, remainingEasternPlaguelands > 0 ? 1 : 0);
        FillInitialWorldStateData(data, count, WORLD_STATE_SCOURGE_TANARIS, remainingTanaris > 0 ? 1 : 0);
        FillInitialWorldStateData(data, count, WORLD_STATE_SCOURGE_WINTERSPRING, remainingWinterspring > 0 ? 1 : 0);

        FillInitialWorldStateData(data, count, WORLD_STATE_SCOURGE_BATTLES_WON, victories);
        FillInitialWorldStateData(data, count, WORLD_STATE_SCOURGE_NECROPOLIS_AZSHARA, remainingAzshara);
        FillInitialWorldStateData(data, count, WORLD_STATE_SCOURGE_NECROPOLIS_BLASTED_LANDS, remainingBlastedLands);
        FillInitialWorldStateData(data, count, WORLD_STATE_SCOURGE_NECROPOLIS_BURNING_STEPPES, remainingBurningSteppes);
        FillInitialWorldStateData(data, count, WORLD_STATE_SCOURGE_NECROPOLIS_EASTERN_PLAGUELANDS, remainingEasternPlaguelands);
        FillInitialWorldStateData(data, count, WORLD_STATE_SCOURGE_NECROPOLIS_TANARIS, remainingTanaris);
        FillInitialWorldStateData(data, count, WORLD_STATE_SCOURGE_NECROPOLIS_WINTERSPRING, remainingWinterspring);
    }

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
