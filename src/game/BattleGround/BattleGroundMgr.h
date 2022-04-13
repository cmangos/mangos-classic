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

#ifndef __BATTLEGROUNDMGR_H
#define __BATTLEGROUNDMGR_H

#include "Common.h"
#include "Policies/Singleton.h"
#include "BattleGround.h"

#include <mutex>

typedef std::map<uint32, BattleGround*> BattleGroundSet;

// this container can't be deque, because deque doesn't like removing the last element - if you remove it, it invalidates next iterator and crash appears
typedef std::list<BattleGround*> BgFreeSlotQueueType;

typedef std::unordered_map<uint32, BattleGroundTypeId> BattleMastersMap;
typedef std::unordered_map<uint32, BattleGroundEventIdx> CreatureBattleEventIndexesMap;
typedef std::unordered_map<uint32, BattleGroundEventIdx> GameObjectBattleEventIndexesMap;

#define COUNT_OF_PLAYERS_TO_AVERAGE_WAIT_TIME 10

struct GroupQueueInfo;                                      // type predefinition
struct PlayerQueueInfo                                      // stores information for players in queue
{
    uint32  lastOnlineTime;                                 // for tracking and removing offline players from queue after 5 minutes
    GroupQueueInfo* groupInfo;                              // pointer to the associated groupqueueinfo
};

typedef std::map<ObjectGuid, PlayerQueueInfo*> GroupQueueInfoPlayers;

struct GroupQueueInfo                                       // stores information about the group in queue (also used when joined as solo!)
{
    GroupQueueInfoPlayers players;                          // player queue info map
    Team  groupTeam;                                        // Player team (ALLIANCE/HORDE)
    BattleGroundTypeId bgTypeId;                            // battleground type id
    uint32  joinTime;                                       // time when group was added
    uint32  removeInviteTime;                               // time when we will remove invite for players in group
    uint32  isInvitedToBgInstanceGuid;                      // was invited to certain BG
    uint32  desiredInstanceId;                              // queued for this instance specifically
};

enum BattleGroundQueueGroupTypes
{
    BG_QUEUE_PREMADE_ALLIANCE   = 0,
    BG_QUEUE_PREMADE_HORDE      = 1,
    BG_QUEUE_NORMAL_ALLIANCE    = 2,
    BG_QUEUE_NORMAL_HORDE       = 3
};

#define BG_QUEUE_GROUP_TYPES_COUNT 4

class BattleGround;
class BattleGroundQueue
{
    public:
        BattleGroundQueue();
        ~BattleGroundQueue();

        void Update(BattleGroundTypeId /*bgTypeId*/, BattleGroundBracketId /*bracketId*/);

        void FillPlayersToBg(BattleGround* /*bg*/, BattleGroundBracketId /*bracketId*/);
        bool CheckPremadeMatch(BattleGroundBracketId /*bracketId*/, uint32 /*minPlayersPerTeam*/, uint32 /*maxPlayersPerTeam*/);
        bool CheckNormalMatch(BattleGroundBracketId /*bracketId*/, uint32 /*minPlayers*/, uint32 /*maxPlayers*/);
        GroupQueueInfo* AddGroup(Player* /*leader*/, Group* /*group*/, BattleGroundTypeId /*bgTypeId*/, BattleGroundBracketId /*bracketEntry*/, bool /*isPremade*/, uint32 /*instanceId*/);
        void RemovePlayer(ObjectGuid /*guid*/, bool /*decreaseInvitedCount*/);
        bool IsPlayerInvited(ObjectGuid /*playerGuid*/, const uint32 /*bgInstanceGuid*/, const uint32 /*removeTime*/);
        bool GetPlayerGroupInfoData(ObjectGuid /*guid*/, GroupQueueInfo* /*groupInfo*/);
        void PlayerInvitedToBgUpdateAverageWaitTime(GroupQueueInfo* /*groupInfo*/, BattleGroundBracketId /*bracketId*/);
        uint32 GetAverageQueueWaitTime(GroupQueueInfo* /*groupInfo*/, BattleGroundBracketId /*bracketId*/);

    private:
        // mutex that should not allow changing private data, nor allowing to update Queue during private data change.
        std::recursive_mutex m_lock;


        typedef std::map<ObjectGuid, PlayerQueueInfo> QueuedPlayersMap;
        QueuedPlayersMap m_queuedPlayers;

        // we need constant add to begin and constant remove / add from the end, therefore deque suits our problem well
        typedef std::list<GroupQueueInfo*> GroupsQueueType;

        /*
        This two dimensional array is used to store All queued groups
        First dimension specifies the bgTypeId
        Second dimension specifies the player's group types -
             BG_QUEUE_PREMADE_ALLIANCE  is used for premade alliance groups
             BG_QUEUE_PREMADE_HORDE     is used for premade horde groups
             BG_QUEUE_NORMAL_ALLIANCE   is used for normal (or small) alliance groups
             BG_QUEUE_NORMAL_HORDE      is used for normal (or small) horde groups
        */
        GroupsQueueType m_queuedGroups[MAX_BATTLEGROUND_BRACKETS][BG_QUEUE_GROUP_TYPES_COUNT];

        // class to select and invite groups to bg
        class SelectionPool
        {
            public:
                SelectionPool() : playerCount(0) {}
                void Init();
                bool AddGroup(GroupQueueInfo* ginfo, uint32 desiredCount, uint32 bgInstanceId);
                bool KickGroup(uint32 size);
                uint32 GetPlayerCount() const {return playerCount;}
                GroupsQueueType selectedGroups;
            private:
                uint32 playerCount;
        };

        // one selection pool for horde, other one for alliance
        SelectionPool m_selectionPools[PVP_TEAM_COUNT];

        bool InviteGroupToBg(GroupQueueInfo* /*groupInfo*/, BattleGround* /*bg*/, Team /*side*/);

        uint32 m_waitTimes[PVP_TEAM_COUNT][MAX_BATTLEGROUND_BRACKETS][COUNT_OF_PLAYERS_TO_AVERAGE_WAIT_TIME];
        uint32 m_waitTimeLastPlayer[PVP_TEAM_COUNT][MAX_BATTLEGROUND_BRACKETS];
        uint32 m_sumOfWaitTimes[PVP_TEAM_COUNT][MAX_BATTLEGROUND_BRACKETS];
};

/*
    This class is used to invite player to BG again, when minute lasts from his first invitation
    it is capable to solve all possibilities
*/
class BgQueueInviteEvent : public BasicEvent
{
    public:
        BgQueueInviteEvent(ObjectGuid playerGuid, uint32 bgInstanceGuid, BattleGroundTypeId bgTypeId, uint32 removeTime) :
            m_playerGuid(playerGuid), m_bgInstanceGuid(bgInstanceGuid), m_bgTypeId(bgTypeId), m_removeTime(removeTime)
        {
        };
        virtual ~BgQueueInviteEvent() {};

        virtual bool Execute(uint64 e_time, uint32 p_time) override;
        virtual void Abort(uint64 e_time) override;

    private:
        ObjectGuid m_playerGuid;
        uint32 m_bgInstanceGuid;
        BattleGroundTypeId m_bgTypeId;
        uint32 m_removeTime;
};

/*
    This class is used to remove player from BG queue after 1 minute 20 seconds from first invitation
    We must store removeInvite time in case player left queue and joined and is invited again
    We must store bgQueueTypeId, because battleground can be deleted already, when player entered it
*/
class BgQueueRemoveEvent : public BasicEvent
{
    public:
        BgQueueRemoveEvent(ObjectGuid playerGuid, uint32 bgInstanceGuid, BattleGroundTypeId bgTypeId, BattleGroundQueueTypeId bgQueueTypeId, uint32 removeTime)
            : m_playerGuid(playerGuid), m_bgInstanceGuid(bgInstanceGuid), m_removeTime(removeTime), m_bgTypeId(bgTypeId), m_bgQueueTypeId(bgQueueTypeId)
        {}

        virtual ~BgQueueRemoveEvent() {}

        virtual bool Execute(uint64 e_time, uint32 p_time) override;
        virtual void Abort(uint64 e_time) override;

    private:
        ObjectGuid m_playerGuid;
        uint32 m_bgInstanceGuid;
        uint32 m_removeTime;
        BattleGroundTypeId m_bgTypeId;
        BattleGroundQueueTypeId m_bgQueueTypeId;
};

class BattleGroundMgr
{
    public:
        /* Construction */
        BattleGroundMgr();
        ~BattleGroundMgr();
        void Update(uint32 /*diff*/);

        /* Packet Building */
        void BuildPlayerJoinedBattleGroundPacket(WorldPacket& /*data*/, Player* /*player*/) const;
        void BuildPlayerLeftBattleGroundPacket(WorldPacket& /*data*/, ObjectGuid /*guid*/) const;
        void BuildBattleGroundListPacket(WorldPacket& /*data*/, ObjectGuid /*guid*/, Player* /*player*/, BattleGroundTypeId /*bgTypeId*/) const;
        void BuildGroupJoinedBattlegroundPacket(WorldPacket& /*data*/, BattleGroundTypeId /*bgTypeId*/) const;
        void BuildUpdateWorldStatePacket(WorldPacket& /*data*/, uint32 /*field*/, uint32 /*value*/) const;
        void BuildPvpLogDataPacket(WorldPacket& /*data*/, BattleGround* /*bg*/) const;
        void BuildBattleGroundStatusPacket(WorldPacket& /*data*/, BattleGround* /*bg*/, uint8 /*queueSlot*/, uint8 /*statusId*/, uint32 /*time1*/, uint32 /*time2*/) const;
        void BuildPlaySoundPacket(WorldPacket& /*data*/, uint32 /*soundId*/) const;

        /* Battlegrounds */
        BattleGround* GetBattleGroundThroughClientInstance(uint32 /*instanceId*/, BattleGroundTypeId /*bgTypeId*/);
        BattleGround* GetBattleGround(uint32 /*instanceId*/, BattleGroundTypeId /*bgTypeId*/); // there must be uint32 because MAX_BATTLEGROUND_TYPE_ID means unknown

        BattleGround* GetBattleGroundTemplate(BattleGroundTypeId /*bgTypeId*/);
        BattleGround* CreateNewBattleGround(BattleGroundTypeId /*bgTypeId*/, BattleGroundBracketId /*bracketEntry*/);

        uint32 CreateBattleGround(BattleGroundTypeId /*bgTypeId*/, uint32 /*minPlayersPerTeam*/, uint32 /*maxPlayersPerTeam*/, uint32 /*levelMin*/, uint32 /*levelMax*/, char const* /*battleGroundName*/, uint32 /*mapId*/, float /*team1StartLocX*/, float /*team1StartLocY*/, float /*team1StartLocZ*/, float /*team1StartLocO*/, float /*team2StartLocX*/, float /*team2StartLocY*/, float /*team2StartLocZ*/, float /*team2StartLocO*/, float /*startMaxDist*/);

        void AddBattleGround(uint32 instanceId, BattleGroundTypeId bgTypeId, BattleGround* bg) { m_battleGrounds[bgTypeId][instanceId] = bg; };
        void RemoveBattleGround(uint32 instanceId, BattleGroundTypeId bgTypeId) { m_battleGrounds[bgTypeId].erase(instanceId); }

        uint32 CreateClientVisibleInstanceId(BattleGroundTypeId /*bgTypeId*/, BattleGroundBracketId /*bracketId*/);
        void DeleteClientVisibleInstanceId(BattleGroundTypeId bgTypeId, BattleGroundBracketId bracketId, uint32 clientInstanceId)
        {
            m_clientBattleGroundIds[bgTypeId][bracketId].erase(clientInstanceId);
        }

        void CreateInitialBattleGrounds();
        void DeleteAllBattleGrounds();

        void SendToBattleGround(Player* /*player*/, uint32 /*instanceId*/, BattleGroundTypeId /*bgTypeId*/);

        /* Battleground queues */
        // these queues are instantiated when creating BattlegroundMrg
        BattleGroundQueue m_battleGroundQueues[MAX_BATTLEGROUND_QUEUE_TYPES]; // public, because we need to access them in BG handler code

        BgFreeSlotQueueType BgFreeSlotQueue[MAX_BATTLEGROUND_TYPE_ID];

        void ScheduleQueueUpdate(BattleGroundQueueTypeId /*bgQueueTypeId*/, BattleGroundTypeId /*bgTypeId*/, BattleGroundBracketId /*bracketId*/);
        uint32 GetPrematureFinishTime() const;

        void ToggleTesting();

        void LoadBattleMastersEntry();
        BattleGroundTypeId GetBattleMasterBG(uint32 entry) const
        {
            BattleMastersMap::const_iterator itr = m_battleMastersMap.find(entry);
            if (itr != m_battleMastersMap.end())
                return itr->second;

            return BATTLEGROUND_TYPE_NONE;
        }

        void LoadBattleEventIndexes();
        const BattleGroundEventIdx GetCreatureEventIndex(uint32 dbGuid) const
        {
            CreatureBattleEventIndexesMap::const_iterator itr = m_creatureBattleEventIndexMap.find(dbGuid);
            if (itr != m_creatureBattleEventIndexMap.end())
                return itr->second;

            return m_creatureBattleEventIndexMap.find(static_cast<uint32>(-1))->second;
        }

        const BattleGroundEventIdx GetGameObjectEventIndex(uint32 dbGuid) const
        {
            GameObjectBattleEventIndexesMap::const_iterator itr = m_gameObjectBattleEventIndexMap.find(dbGuid);
            if (itr != m_gameObjectBattleEventIndexMap.end())
                return itr->second;

            return m_gameObjectBattleEventIndexMap.find(static_cast<uint32>(-1))->second;
        }

        bool IsTesting() const { return m_testing; }

        static BattleGroundQueueTypeId BgQueueTypeId(BattleGroundTypeId /*bgTypeId*/);
        static BattleGroundTypeId BgTemplateId(BattleGroundQueueTypeId /*bgQueueTypeId*/);

        static HolidayIds BgTypeToWeekendHolidayId(BattleGroundTypeId /*bgTypeId*/);
        static BattleGroundTypeId WeekendHolidayIdToBgType(HolidayIds /*holiday*/);
        static bool IsBgWeekend(BattleGroundTypeId /*bgTypeId*/);

    private:
        std::mutex schedulerLock;
        BattleMastersMap m_battleMastersMap;
        CreatureBattleEventIndexesMap m_creatureBattleEventIndexMap;
        GameObjectBattleEventIndexesMap m_gameObjectBattleEventIndexMap;

        /* Battlegrounds */
        BattleGroundSet m_battleGrounds[MAX_BATTLEGROUND_TYPE_ID];
        std::vector<uint64> m_queueUpdateScheduler;
        typedef std::set<uint32> ClientBattleGroundIdSet;
        ClientBattleGroundIdSet m_clientBattleGroundIds[MAX_BATTLEGROUND_TYPE_ID][MAX_BATTLEGROUND_BRACKETS]; // the instanceids just visible for the client
        bool   m_testing;
};

#define sBattleGroundMgr MaNGOS::Singleton<BattleGroundMgr>::Instance()
#endif
