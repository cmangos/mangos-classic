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
    ZONEID_DARNASSUS            = 1657,
    ZONEID_IRONFORGE            = 1537,
    ZONEID_ORGRIMMAR            = 1637,
    ZONEID_THUNDER_BLUFF        = 1638,
    ZONEID_UNDERCITY            = 1497,
};

//enum AreaIds
//{
//};

//enum SpellId
//{
//};

enum GoId
{
    OBJECT_EVENT_TRAP_THRALL    = 181088,
    OBJECT_EVENT_TRAP_BOLVAR    = 181089,
    OBJECT_EVENT_TRAP_MAGNI     = 181090,
    OBJECT_EVENT_TRAP_TYRANDE   = 181091,
    OBJECT_EVENT_TRAP_CAIRNE    = 181092,
    OBJECT_EVENT_TRAP_SYLVANAS  = 181093,
};

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
    // Prepatch event
    //GAME_EVENT_BEFORE_THE_STORM = 100,

    // Isle phases
    //GAME_EVENT_QUEL_DANAS_PHASE_1               = 101,
    //GAME_EVENT_QUEL_DANAS_PHASE_2               = 102,
    //GAME_EVENT_QUEL_DANAS_PHASE_2_PORTAL        = 103,
    //GAME_EVENT_QUEL_DANAS_PHASE_3               = 104,
    //GAME_EVENT_QUEL_DANAS_PHASE_3_ANVIL         = 105,
    //GAME_EVENT_QUEL_DANAS_PHASE_4               = 106,
    //GAME_EVENT_QUEL_DANAS_PHASE_4_MONUMENT      = 107,
    //GAME_EVENT_QUEL_DANAS_PHASE_4_ALCHEMY_LAB   = 108,

    // AQ
    // giving items
    GAME_EVENT_AHN_QIRAJ_EFFORT_PHASE_1 = 120,
    // done with items - 5 days
    GAME_EVENT_AHN_QIRAJ_EFFORT_PHASE_2 = 121,
    // gong enabled
    GAME_EVENT_AHN_QIRAJ_EFFORT_PHASE_3 = 122,
    // 10 hour war
    GAME_EVENT_AHN_QIRAJ_EFFORT_PHASE_4 = 123,
    // base perpetual state
    GAME_EVENT_AHN_QIRAJ_EFFORT_PHASE_5 = 124,
};

enum AQResources
{
    // Horde
    AQ_PEACEBLOOM,
    AQ_LEAN_WOLF_STEAK,
    AQ_TIN_BAR,
    AQ_WOOL_BANDAGE,
    AQ_FIREBLOOM,
    AQ_HEAVY_LEATHER,
    AQ_MITHRIL_BAR,
    AQ_MAGEWEAVE_BANDAGE,
    AQ_RUGGED_LEATHER,
    AQ_BAKED_SALMON,
    // Alliance
    AQ_LIGHT_LEATHER,
    AQ_LINEN_BANDAGE,
    AQ_MEDIUM_LEATHER,
    AQ_STRANGLEKELP,
    AQ_RAINBOW_FIN_ALBACORE,
    AQ_IRON_BAR,
    AQ_ROAST_RAPTOR,
    AQ_SILK_BANDAGE,
    AQ_THORIUM_BAR,
    AQ_ARTHAS_TEARS,
    // Common
    AQ_COPPER_BAR_ALLY,
    AQ_PURPLE_LOTUS_ALLY,
    AQ_THICK_LEATHER_ALLY,
    AQ_SPOTTED_YELLOWTAIL_ALLY,
    AQ_RUNECLOTH_BANDAGE_ALLY,
    RESOURCE_UNIQUE_MAX = AQ_RUNECLOTH_BANDAGE_ALLY,
    AQ_COPPER_BAR_HORDE,
    AQ_PURPLE_LOTUS_HORDE,
    AQ_THICK_LEATHER_HORDE,
    AQ_SPOTTED_YELLOWTAIL_HORDE,
    AQ_RUNECLOTH_BANDAGE_HORDE,
    RESOURCE_MAX,
};

enum AQResourceGroup
{
    RESOURCE_GROUP_SKINNING,
    RESOURCE_GROUP_BANDAGES,
    RESOURCE_GROUP_BARS,
    RESOURCE_GROUP_COOKING,
    RESOURCE_GROUP_HERBS,
};

enum AQPhase
{
    PHASE_0_DISABLED,
    PHASE_1_GATHERING_RESOURCES,
    PHASE_2_TRANSPORTING_RESOURCES,
    PHASE_3_GONG_TIME,
    PHASE_4_10_HOUR_WAR,
    PHASE_5_DONE,
};

// To be used
struct AhnQirajData
{
    uint32 m_phase;
    uint64 m_timer;
    uint32 m_WarEffortCounters[RESOURCE_MAX];
    GuidVector m_warEffortWorldstatesPlayers;
    std::mutex m_warEffortMutex;
    std::set<uint32> m_spawnedDbGuids;
    uint32 m_phase2Tier;
    AhnQirajData() : m_phase(PHASE_0_DISABLED), m_timer(0), m_phase2Tier(0)
    {
        memset(m_WarEffortCounters, 0, sizeof(m_WarEffortCounters));
    }
    std::string GetData();
};

enum LoveIsInTheAirLeaders
{
    LOVE_LEADER_THRALL,
    LOVE_LEADER_CAIRNE,
    LOVE_LEADER_SYLVANAS,
    LOVE_LEADER_BOLVAR,
    LOVE_LEADER_MAGNI,
    LOVE_LEADER_TYRANDE,
    LOVE_LEADER_HORDE,
    LOVE_LEADER_ALLIANCE,
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

        void SendWorldstateUpdate(std::mutex& mutex, GuidVector const& guids, uint32 value, uint32 worldStateId);

        // vanilla section
        void SendLoveIsInTheAirWorldstateUpdate(uint32 value, uint32 worldStateId);
        uint32 GetLoveIsInTheAirCounter(LoveIsInTheAirLeaders leader) { return m_loveIsInTheAirData.counters[leader]; }

        void AddWarEffortProgress(AQResources resource, uint32 count);
        void HandleWarEffortPhaseTransition(uint32 newPhase);
        void StartWarEffortEvent();
        void StopWarEffortEvent();
        void SpawnWarEffortGos();
        void ChangeWarEffortGoSpawns(AQResources resource, int32 forcedTier = -1);
        void ChangeWarEffortPhase2Tier(uint32 remainingDays);
        void DespawnWarEffortGuids(std::set<std::pair<uint32, Team>>& guids);
        AQPhase GetAqPhase() { return (AQPhase)m_aqData.m_phase; }
        std::pair<AQResourceGroup, Team> GetResourceInfo(AQResources resource);
        std::pair<uint32, uint32> GetResourceCounterAndMax(AQResourceGroup group, Team team);
        std::string GetAQPrintout();

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

