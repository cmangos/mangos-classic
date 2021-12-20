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

#ifndef WORLD_STATE_MANAGER_DEFINES_H
#define WORLD_STATE_MANAGER_DEFINES_H

#include "Platform/Define.h"
#include <map>
#include <functional>
#include <string>
#include <vector>

class ByteBuffer;
class Map;

struct WorldStateVariable
{
    int32 value;
    bool send;
    uint32 zoneId;
    uint32 areaId;
    std::vector<std::function<void()>> executors;

    WorldStateVariable() : value(0), send(false), zoneId(0), areaId(0) {}
};

class WorldStateVariableManager
{
    public:
        WorldStateVariableManager(Map* map);

        void Initialize();

        int32 GetVariable(uint32 Id) const;
        void SetVariable(uint32 Id, int32 value);
        void SetVariableData(uint32 Id, bool send, uint32 zoneId, uint32 areaId);
        void AddVariableExecutor(uint32 Id, std::function<void()>& executor);

        void FillInitialWorldStates(ByteBuffer& data, uint32& count, uint32 zoneId, uint32 areaId);
        void BroadcastVariable(uint32 Id);

        std::string GetVariableList() const;

        void SetEncounterVariable(uint32 encounterId, bool state);

    private:
        std::map<uint32, WorldStateVariable> m_variables;
        Map* m_owner;
};

#endif