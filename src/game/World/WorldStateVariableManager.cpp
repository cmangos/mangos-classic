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

#include "World/WorldStateVariableManager.h"
#include "Util/ByteBuffer.h"
#include "Maps/Map.h"
#include "Maps/MapReference.h"
#include "Entities/Player.h"
#include "Globals/ObjectMgr.h"

WorldStateVariableManager::WorldStateVariableManager(Map* map) : m_owner(map)
{
}

void WorldStateVariableManager::Initialize(uint32 completedEncounterMask)
{
    auto bounds = sObjectMgr.GetDungeonEncounterBoundsByMap(m_owner->GetId());
    for (auto itr = bounds.first; itr != bounds.second; ++itr)
    {
        DungeonEncounterEntry const* encounter = (*itr).second.dbcEntry;

        if (encounter->CompleteWorldStateID) // use official data whenever available
            SetVariable(encounter->CompleteWorldStateID, ((1 << encounter->encounterIndex) & completedEncounterMask) != 0);
        else // phase this out eventually
            SetEncounterVariable(encounter->Id, ((1 << encounter->encounterIndex) & completedEncounterMask) != 0);
    }
}

int32 WorldStateVariableManager::GetVariable(int32 Id) const
{
    auto itr = m_variables.find(Id);
    if (itr == m_variables.end())
        return 0;
    return (*itr).second.value;
}

WorldStateVariable const* WorldStateVariableManager::GetVariableData(int32 Id) const
{
    auto itr = m_variables.find(Id);
    if (itr == m_variables.end())
        return nullptr;
    return &((*itr).second);
}

void WorldStateVariableManager::SetVariable(int32 Id, int32 value)
{
    auto& variable = m_variables[Id];
    if (variable.value == value)
        return;

    variable.value = value;
    if (m_variables[Id].send)
        BroadcastVariable(Id);
}

void WorldStateVariableManager::SetVariableData(int32 Id, bool send, uint32 zoneId, uint32 areaId)
{
    auto& variable = m_variables[Id];
    variable.send = send;
    variable.zoneId = zoneId;
    variable.areaId = areaId;
}

void WorldStateVariableManager::AddVariableExecutor(int32 Id, std::function<void()>& executor)
{
    m_variables[Id].executors.push_back(executor);
}

void WorldStateVariableManager::FillInitialWorldStates(ByteBuffer& data, uint32& count, uint32 zoneId, uint32 areaId)
{
    for (auto& variable : m_variables)
    {
        if (variable.second.send &&
            (!variable.second.zoneId || variable.second.zoneId == zoneId) &&
            (!variable.second.areaId || variable.second.areaId == areaId))
        {
            data << int32(variable.first);
            data << int32(variable.second.value);
            ++count;
        }
    }
}

void WorldStateVariableManager::BroadcastVariable(int32 Id) const
{
    auto const& lPlayers = m_owner->GetPlayers();
    if (!lPlayers.isEmpty())
    {
        WorldStateVariable const* variable = GetVariableData(Id);
        MANGOS_ASSERT(variable); // if we are broadcasting a variable it must be initialized
        int32 value = variable->value;
        bool queryIds = variable->zoneId || variable->areaId;
        for (const auto& lPlayer : lPlayers)
        {
            if (Player* player = lPlayer.getSource())
            {
                uint32 zoneId, areaId;
                if (queryIds) // optimization
                    player->GetZoneAndAreaId(zoneId, areaId);
                if ((!variable->zoneId || variable->zoneId == zoneId) &&
                    (!variable->areaId || variable->areaId == areaId))
                    player->SendUpdateWorldState(Id, value);
            }
        }
    }
}

std::string WorldStateVariableManager::GetVariableList() const
{
    std::string output;
    for (auto& data : m_variables)
    {
        WorldStateName* name = sObjectMgr.GetWorldStateName(data.first);
        output += "Id: " + std::to_string(data.first) + " Val: " + std::to_string(data.second.value) + "\n" + "Name:" + (name ? name->Name : std::string()) + "\n";
    }
    return output;
}

void WorldStateVariableManager::SetEncounterVariable(uint32 encounterId, bool state)
{
    SetVariable(encounterId * 100, state);
    SetVariable(encounterId * 100 + 1, !state);
}

