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
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#ifndef WORLD_STATE_H
#define WORLD_STATE_H

#include "Policies/Singleton.h"
#include "Globals/ObjectMgr.h"
#include "Entities/GameObject.h"
#include "Entities/Player.h"
#include "Globals/SharedDefines.h"
#include <atomic>
#include <string>

enum ZoneIds
{
    ZONEID_STORMWIND_CITY       = 1519,
};

//enum AreaIds
//{
//};

//enum SpellId
//{
//};

//enum GoId
//{
//};

//enum Conditions
//{
//};

enum Events
{
    // vanilla
    CUSTOM_EVENT_YSONDRE_DIED,
    CUSTOM_EVENT_LETHON_DIED,
    CUSTOM_EVENT_EMERISS_DIED,
    CUSTOM_EVENT_TAERAR_DIED,
    CUSTOM_EVENT_LOVE_IS_IN_THE_AIR_LEADER,
};

enum SaveIds
{
    SAVE_ID_EMERALD_DRAGONS = 0,
    SAVE_ID_AHN_QIRAJ = 1,
    SAVE_ID_LOVE_IS_IN_THE_AIR = 2,

    // SAVE_ID_QUEL_DANAS = 20,
    // SAVE_ID_EXPANSION_RELEASE = 21,
};

enum GameEvents
{
};

// To be used
struct AhnQirajData
{
    std::string GetData() { return ""; }
};

enum LoveIsInTheAirLeaders
{
    LOVE_LEADER_THRALL,
    LOVE_LEADER_CAIRNE,
    LOVE_LEADER_SYLVANAS,
    LOVE_LEADER_BOLVAR,
    LOVE_LEADER_MAGNI,
    LOVE_LEADER_TYRANDE,
    LOVE_LEADER_MAX,
};

struct LoveIsInTheAir
{
    uint32 counters[LOVE_LEADER_MAX]; // potential race condition which wont cause anything critical
};

// Intended for implementing server wide scripts, note: all behaviour must be safeguarded towards multithreading
class WorldState
{
    public:
        WorldState();
        virtual ~WorldState();

        void Load();
        void Save(SaveIds saveId);
        void SaveHelper(std::string& stringToSave, SaveIds saveId);

        // Called when a gameobject is created or removed
        void HandleGameObjectUse(GameObject* go, Unit* user);
        void HandleGameObjectRevertState(GameObject* go);

        void HandlePlayerEnterZone(Player* player, uint32 zoneId);
        void HandlePlayerLeaveZone(Player* player, uint32 zoneId);

        //void HandlePlayerEnterArea(Player* player, uint32 areaId);
        //void HandlePlayerLeaveArea(Player* player, uint32 areaId);

        bool IsConditionFulfilled(uint32 conditionId, uint32 state) const;
        void HandleConditionStateChange(uint32 conditionId, uint32 state);

        void HandleExternalEvent(uint32 eventId, uint32 param);
        void ExecuteOnAreaPlayers(uint32 areaId, std::function<void(Player*)> executor);

        void Update(const uint32 diff);

        // vanilla section
        void SendLoveIsInTheAirWorldstateUpdate(uint32 value, uint32 worldStateId);
        uint32 GetLoveIsInTheAirCounter(LoveIsInTheAirLeaders leader) { return m_loveIsInTheAirData.counters[leader]; }

        void FillInitialWorldStates(ByteBuffer& data, uint32& count, uint32 zoneId);

        // helper functions for world state list fill
        inline void FillInitialWorldStateData(ByteBuffer& data, uint32& count, uint32 state, uint32 value)
        {
            data << uint32(state);
            data << uint32(value);
            ++count;
        }

        inline void FillInitialWorldStateData(ByteBuffer& data, uint32& count, uint32 state, int32 value)
        {
            data << uint32(state);
            data << int32(value);
            ++count;
        }
    private:
        std::map<uint32, GuidVector> m_areaPlayers;
        std::map<uint32, std::atomic<uint32>> m_transportStates; // atomic to avoid having to lock

        std::mutex m_mutex; // all World State operations are thread unsafe
        uint32 m_saveTimer;

        // vanilla section
        bool IsDragonSpawned(uint32 entry);
        void RespawnEmeraldDragons();

        uint8 m_emeraldDragonsState;
        uint32 m_emeraldDragonsTimer;
        std::vector<uint32> m_emeraldDragonsChosenPositions;
        AhnQirajData m_aqData;

        LoveIsInTheAir m_loveIsInTheAirData;
        GuidVector m_loveIsInTheAirCapitalsPlayers;

        std::mutex m_loveIsInTheAirMutex; // capital cities optimization
};

#define sWorldState MaNGOS::Singleton<WorldState>::Instance()

#endif

