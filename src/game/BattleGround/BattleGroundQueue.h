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

#ifndef __BATTLEGROUNDQUEUE_H
#define __BATTLEGROUNDQUEUE_H

#include "Common.h"
#include "BattleGround/BattleGround.h"

struct GroupQueueInfo;                                      // type predefinition
struct PlayerQueueInfo                                      // stores information for players in queue
{
    uint32  lastOnlineTime;                                 // for tracking and removing offline players from queue after 5 minutes
    GroupQueueInfo* groupInfo;                              // pointer to the associated groupqueueinfo
};

typedef std::map<ObjectGuid, PlayerQueueInfo*> GroupQueueInfoPlayers;

struct AddGroupToQueueInfo
{
    Team team;
    std::vector<ObjectGuid> members; // empty when not in group
    uint32 clientInstanceId;
    uint32 mapId;
};

struct GroupQueueInfo                                       // stores information about the group in queue (also used when joined as solo!)
{
    GroupQueueInfoPlayers players;                          // player queue info map
    Team  groupTeam;                                        // Player team (ALLIANCE/HORDE)
    BattleGroundTypeId bgTypeId;                            // battleground type id
    BattleGroundBracketId bgBracketId;                      // battleground bracked id
    uint32  mapId;                                          // invited to mapId
    uint32  clientInstanceId;                               // invited to client instance id
    uint32  joinTime;                                       // time when group was added
    uint32  removeInviteTime;                               // time when we will remove invite for players in group
    uint32  isInvitedToBgInstanceGuid;                      // was invited to certain BG
    uint32  desiredInstanceId;                              // queued for this instance specifically
};

struct BattleGroundInQueueInfo
{
    BattleGroundTypeId bgTypeId;
    uint32 instanceId;
    bool isBattleGround;
    BattleGroundBracketId bracketId;
    BattleGroundStatus status; // only altered in bg and synched here
    uint32 m_clientInstanceId;
    uint32 mapId;

    uint32 playersInside;
    uint32 maxPlayers;
    uint32 m_maxPlayersPerTeam;
    uint32 m_minPlayersPerTeam;

    uint32 GetInstanceId() const { return instanceId; }
    bool IsBattleGround() const { return isBattleGround; }
    BattleGroundTypeId GetTypeId() const { return bgTypeId; }
    BattleGroundBracketId GetBracketId() const { return bracketId; }
    BattleGroundStatus GetStatus() const { return status; }
    uint32 GetClientInstanceId() const { return m_clientInstanceId; }
    uint32 GetMapId() const { return mapId; }

    uint32 GetPlayersSize() const { return playersInside; }
    uint32 GetMaxPlayers() const { return maxPlayers; }
    bool HasFreeSlots() const { return GetPlayersSize() < GetMaxPlayers(); }
    uint32 GetMaxPlayersPerTeam() const { return m_maxPlayersPerTeam; }
    uint32 GetMinPlayersPerTeam() const { return m_minPlayersPerTeam; }

    void DecreaseInvitedCount(Team team);
    void IncreaseInvitedCount(Team team);
    uint32 GetInvitedCount(Team team) const
    {
        if (team == ALLIANCE)
            return m_invitedAlliance;
        return m_invitedHorde;
    }

    uint32 GetFreeSlotsForTeam(Team team) const
    {
        // return free slot count to MaxPlayerPerTeam
        if (GetStatus() == STATUS_WAIT_JOIN || GetStatus() == STATUS_IN_PROGRESS)
            return (GetInvitedCount(team) < GetMaxPlayersPerTeam()) ? GetMaxPlayersPerTeam() - GetInvitedCount(team) : 0;

        return 0;
    }

    void Fill(BattleGround* bg);

    private:
        uint32 m_invitedAlliance;
        uint32 m_invitedHorde;
};

// this container can't be deque, because deque doesn't like removing the last element - if you remove it, it invalidates next iterator and crash appears
typedef std::list<BattleGroundInQueueInfo> BgFreeSlotQueueType;

class BattleGroundQueue;

class BattleGroundQueueItem
{
    public:
        BattleGroundQueueItem();
        ~BattleGroundQueueItem();

        void Update(BattleGroundQueue& queue, BattleGroundTypeId /*bgTypeId*/, BattleGroundBracketId /*bracketId*/);

        void FillPlayersToBg(BattleGroundInQueueInfo& queueInfo, BattleGroundBracketId /*bracketId*/);
        bool CheckPremadeMatch(BattleGroundBracketId /*bracketId*/, uint32 /*minPlayersPerTeam*/, uint32 /*maxPlayersPerTeam*/);
        bool CheckNormalMatch(BattleGroundQueue& queue, BattleGround* /*bgTemplate*/, BattleGroundBracketId /*bracketId*/, uint32 /*minPlayers*/, uint32 /*maxPlayers*/);
        bool CheckSkirmishForSameFaction(BattleGroundBracketId /*bracketId*/, uint32 /*minPlayersPerTeam*/);
        GroupQueueInfo* AddGroup(ObjectGuid leader, AddGroupToQueueInfo const& /*groupInfo*/, BattleGroundTypeId /*bgTypeId*/, BattleGroundBracketId /*bracketEntry*/, bool /*isPremade*/, uint32 /*instanceId*/);
        void RemovePlayer(BattleGroundQueue& queue, ObjectGuid guid, bool decreaseInvitedCount);
        bool IsPlayerInvited(ObjectGuid /*playerGuid*/, const uint32 /*bgInstanceGuid*/, const uint32 /*removeTime*/);
        bool GetPlayerGroupInfoData(ObjectGuid /*guid*/, GroupQueueInfo* /*groupInfo*/);
        void PlayerInvitedToBgUpdateAverageWaitTime(GroupQueueInfo* /*groupInfo*/, BattleGroundBracketId /*bracketId*/);
        uint32 GetAverageQueueWaitTime(GroupQueueInfo* /*groupInfo*/, BattleGroundBracketId /*bracketId*/);

    private:
        typedef std::map<ObjectGuid, PlayerQueueInfo> QueuedPlayersMap;
        QueuedPlayersMap m_queuedPlayers;

        // we need constant add to begin and constant remove / add from the end, therefore deque suits our problem well
        typedef std::list<GroupQueueInfo*> GroupsQueueType;

        /*
        This two dimensional array is used to store All queued groups
        First dimension specifies the bgTypeId
        Second dimension specifies the player's group types -
             BG_QUEUE_PREMADE_ALLIANCE  is used for premade alliance groups and alliance rated arena teams
             BG_QUEUE_PREMADE_HORDE     is used for premade horde groups and horde rated arena teams
             BG_QUEUE_NORMAL_ALLIANCE   is used for normal (or small) alliance groups or non-rated arena matches
             BG_QUEUE_NORMAL_HORDE      is used for normal (or small) horde groups or non-rated arena matches
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
            uint32 GetPlayerCount() const { return playerCount; }
            GroupsQueueType selectedGroups;
        private:
            uint32 playerCount;
        };

        // one selection pool for horde, other one for alliance
        SelectionPool m_selectionPools[PVP_TEAM_COUNT];

        bool InviteGroupToBg(GroupQueueInfo* groupInfo, BattleGroundInQueueInfo& queueInfo, Team side);

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

class BattleGroundQueue
{
    friend class BgQueueInviteEvent;
    friend class BgQueueRemoveEvent;
    public:
        BattleGroundQueue();

        void Update();

        Messager<BattleGroundQueue>& GetMessager() { return m_messager; }

        void ScheduleQueueUpdate(BattleGroundQueueTypeId /*bgQueueTypeId*/, BattleGroundTypeId /*bgTypeId*/, BattleGroundBracketId /*bracketId*/);

        void AddBgToFreeSlots(BattleGroundInQueueInfo const& info);
        void RemoveBgFromFreeSlots(BattleGroundTypeId typeId, uint32 instanceId);

        void RemovePlayer(BattleGroundQueueTypeId bgQueueTypeId, ObjectGuid player, bool decreaseInvitedCount);

        BgFreeSlotQueueType& GetFreeSlotQueueItem(BattleGroundTypeId bgTypeId);
        BattleGroundInQueueInfo* GetFreeSlotInstance(BattleGroundTypeId bgTypeId, uint32 instanceId);
        BattleGroundQueueItem& GetBattleGroundQueue(BattleGroundQueueTypeId bgQueueTypeId);

        void SetNextRatingDiscardUpdate(std::chrono::milliseconds timePoint);

        bool IsTesting() const { return m_testing; }
        void SetTesting(bool state) { m_testing = state; }

        uint32 CreateClientVisibleInstanceId(BattleGroundTypeId /*bgTypeId*/, BattleGroundBracketId /*bracketId*/);
        void DeleteClientVisibleInstanceId(BattleGroundTypeId bgTypeId, BattleGroundBracketId bracketId, uint32 clientInstanceId)
        {
            m_clientBattleGroundIds[bgTypeId][bracketId].erase(clientInstanceId);
        }

        void BuildBattleGroundListPacket(WorldPacket& data, ObjectGuid guid, uint32 playerLevel, BattleGroundTypeId bgTypeId) const;
    private:
        BattleGroundQueueItem m_battleGroundQueues[MAX_BATTLEGROUND_QUEUE_TYPES];

        BgFreeSlotQueueType m_bgFreeSlotQueue[MAX_BATTLEGROUND_TYPE_ID];

        std::vector<uint64> m_queueUpdateScheduler;

        Messager<BattleGroundQueue> m_messager;

        bool m_testing;

        typedef std::set<uint32> ClientBattleGroundIdSet;
        ClientBattleGroundIdSet m_clientBattleGroundIds[MAX_BATTLEGROUND_TYPE_ID][MAX_BATTLEGROUND_BRACKETS]; // the instanceids just visible for the client
};

#endif