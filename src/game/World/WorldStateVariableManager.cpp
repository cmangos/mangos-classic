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
#include "ByteBuffer.h"
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
        DungeonEncounterEntry const* encounter = (*itr).second->dbcEntry;
        SetEncounterVariable(encounter->Id, ((1 << encounter->encounterIndex) & completedEncounterMask) == 1);
    }
}

int32 WorldStateVariableManager::GetVariable(uint32 Id) const
{
    auto itr = m_variables.find(Id);
    if (itr == m_variables.end())
        return 0;
    return (*itr).second.value;
}

void WorldStateVariableManager::SetVariable(uint32 Id, int32 value)
{
    m_variables[Id].value = value;
}

void WorldStateVariableManager::SetVariableData(uint32 Id, bool send, uint32 zoneId, uint32 areaId)
{
    auto& variable = m_variables[Id];
    variable.send = send;
    variable.zoneId = zoneId;
    variable.areaId = areaId;
}

void WorldStateVariableManager::AddVariableExecutor(uint32 Id, std::function<void()>& executor)
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
            data << uint32(variable.first);
            data << int32(variable.second.value);
            ++count;
        }
    }
}

void WorldStateVariableManager::BroadcastVariable(uint32 Id)
{
    auto const& lPlayers = m_owner->GetPlayers();
    if (!lPlayers.isEmpty())
    {
        int32 value = GetVariable(Id);
        for (const auto& lPlayer : lPlayers)
            if (Player* player = lPlayer.getSource())
                player->SendUpdateWorldState(Id, value);
    }
}

std::string WorldStateVariableManager::GetVariableList() const
{
    std::string output;
    for (auto& data : m_variables)
        output += "Id: " + std::to_string(data.first) + " Val: " + std::to_string(data.second.value) + "\n";
    return output;
}

void WorldStateVariableManager::SetEncounterVariable(uint32 encounterId, bool state)
{
    SetVariable(encounterId * 100, state);
    SetVariable(encounterId * 100 + 1, !state);
}

