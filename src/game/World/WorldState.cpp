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
#include "World/World.h"
#include "Maps/MapManager.h"
#include "Entities/Object.h"
#include <algorithm>
#include <map>
#include <World/WorldStateDefines.h>

WorldState::WorldState() : m_emeraldDragonsState(0xF), m_emeraldDragonsTimer(0), m_emeraldDragonsChosenPositions(4, 0)
{
    memset(m_loveIsInTheAirData.counters, 0, sizeof(LoveIsInTheAir));
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
                    else
                    {
                        m_emeraldDragonsState = 0xF;
                        m_emeraldDragonsTimer = 0;
                    }
                    break;
                }
                case SAVE_ID_AHN_QIRAJ: // TODO:
                    break;
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
        // TODO: Add saving for AQ and QD
        case SAVE_ID_AHN_QIRAJ:
        // case SAVE_ID_QUEL_DANAS:
            break;
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
            SendLoveIsInTheAirWorldstateUpdate(GetLoveIsInTheAirCounter(LOVE_LEADER_THRALL), WORLD_STATE_LOVE_IS_IN_THE_AIR_THRALL);
            uint32 hordeSum = GetLoveIsInTheAirCounter(LOVE_LEADER_CAIRNE) + GetLoveIsInTheAirCounter(LOVE_LEADER_THRALL) + GetLoveIsInTheAirCounter(LOVE_LEADER_SYLVANAS);
            SendLoveIsInTheAirWorldstateUpdate(hordeSum, WORLD_STATE_LOVE_IS_IN_THE_AIR_TOTAL_HORDE);
            break;
        }
        case OBJECT_EVENT_TRAP_CAIRNE:
        {
            HandleExternalEvent(CUSTOM_EVENT_LOVE_IS_IN_THE_AIR_LEADER, LOVE_LEADER_CAIRNE);
            SendLoveIsInTheAirWorldstateUpdate(GetLoveIsInTheAirCounter(LOVE_LEADER_CAIRNE), WORLD_STATE_LOVE_IS_IN_THE_AIR_CAIRNE);
            uint32 hordeSum = GetLoveIsInTheAirCounter(LOVE_LEADER_CAIRNE) + GetLoveIsInTheAirCounter(LOVE_LEADER_THRALL) + GetLoveIsInTheAirCounter(LOVE_LEADER_SYLVANAS);
            SendLoveIsInTheAirWorldstateUpdate(hordeSum, WORLD_STATE_LOVE_IS_IN_THE_AIR_TOTAL_HORDE);
            break;
        }
        case OBJECT_EVENT_TRAP_SYLVANAS:
        {
            HandleExternalEvent(CUSTOM_EVENT_LOVE_IS_IN_THE_AIR_LEADER, LOVE_LEADER_SYLVANAS);
            SendLoveIsInTheAirWorldstateUpdate(GetLoveIsInTheAirCounter(LOVE_LEADER_SYLVANAS), WORLD_STATE_LOVE_IS_IN_THE_AIR_SYLVANAS);
            uint32 hordeSum = GetLoveIsInTheAirCounter(LOVE_LEADER_CAIRNE) + GetLoveIsInTheAirCounter(LOVE_LEADER_THRALL) + GetLoveIsInTheAirCounter(LOVE_LEADER_SYLVANAS);
            SendLoveIsInTheAirWorldstateUpdate(hordeSum, WORLD_STATE_LOVE_IS_IN_THE_AIR_TOTAL_HORDE);
            break;
        }
        case OBJECT_EVENT_TRAP_BOLVAR:
        {
            HandleExternalEvent(CUSTOM_EVENT_LOVE_IS_IN_THE_AIR_LEADER, LOVE_LEADER_BOLVAR);
            SendLoveIsInTheAirWorldstateUpdate(GetLoveIsInTheAirCounter(LOVE_LEADER_BOLVAR), WORLD_STATE_LOVE_IS_IN_THE_AIR_BOLVAR);
            uint32 allianceSum = GetLoveIsInTheAirCounter(LOVE_LEADER_BOLVAR) + GetLoveIsInTheAirCounter(LOVE_LEADER_TYRANDE) + GetLoveIsInTheAirCounter(LOVE_LEADER_MAGNI);
            SendLoveIsInTheAirWorldstateUpdate(allianceSum, WORLD_STATE_LOVE_IS_IN_THE_AIR_TOTAL_ALLIANCE);
            break;
        }
        case OBJECT_EVENT_TRAP_MAGNI:
        {
            HandleExternalEvent(CUSTOM_EVENT_LOVE_IS_IN_THE_AIR_LEADER, LOVE_LEADER_MAGNI);
            SendLoveIsInTheAirWorldstateUpdate(GetLoveIsInTheAirCounter(LOVE_LEADER_MAGNI), WORLD_STATE_LOVE_IS_IN_THE_AIR_MAGNI);
            uint32 allianceSum = GetLoveIsInTheAirCounter(LOVE_LEADER_BOLVAR) + GetLoveIsInTheAirCounter(LOVE_LEADER_TYRANDE) + GetLoveIsInTheAirCounter(LOVE_LEADER_MAGNI);
            SendLoveIsInTheAirWorldstateUpdate(allianceSum, WORLD_STATE_LOVE_IS_IN_THE_AIR_TOTAL_ALLIANCE);
            break;
        }
        case OBJECT_EVENT_TRAP_TYRANDE:
        {
            HandleExternalEvent(CUSTOM_EVENT_LOVE_IS_IN_THE_AIR_LEADER, LOVE_LEADER_TYRANDE);
            SendLoveIsInTheAirWorldstateUpdate(GetLoveIsInTheAirCounter(LOVE_LEADER_TYRANDE), WORLD_STATE_LOVE_IS_IN_THE_AIR_TYRANDE);
            uint32 allianceSum = GetLoveIsInTheAirCounter(LOVE_LEADER_BOLVAR) + GetLoveIsInTheAirCounter(LOVE_LEADER_TYRANDE) + GetLoveIsInTheAirCounter(LOVE_LEADER_MAGNI);
            SendLoveIsInTheAirWorldstateUpdate(allianceSum, WORLD_STATE_LOVE_IS_IN_THE_AIR_TOTAL_ALLIANCE);
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
        {
            std::lock_guard<std::mutex> guard(m_loveIsInTheAirMutex);
            m_loveIsInTheAirCapitalsPlayers.push_back(player->GetObjectGuid());
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
        {
            std::lock_guard<std::mutex> guard(m_loveIsInTheAirMutex);
            auto position = std::find(m_loveIsInTheAirCapitalsPlayers.begin(), m_loveIsInTheAirCapitalsPlayers.end(), player->GetObjectGuid());
            if (position != m_loveIsInTheAirCapitalsPlayers.end())
                m_loveIsInTheAirCapitalsPlayers.erase(position);
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

void WorldState::FillInitialWorldStates(ByteBuffer& data, uint32& count, uint32 zoneId)
{
    if (sGameEventMgr.IsActiveHoliday(HOLIDAY_LOVE_IS_IN_THE_AIR))
    {
        switch (zoneId)
        {
            case ZONEID_STORMWIND_CITY: // TODO: Add rest
            {
                // TODO: add worldstate sending
                // FillInitialWorldStateData(data, count, WORLD_STATE_DEATHS_DOOR_NORTH_WARP_GATE_HEALTH, m_deathsDoorNorthHP);
                break;
            }
        }
    }
}
