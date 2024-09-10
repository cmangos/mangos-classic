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
#include "Util/UniqueTrackablePtr.h"
#include "BattleGround.h"
#include "BattleGround/BattleGroundDefines.h"

#include <mutex>

typedef std::map<uint32, MaNGOS::unique_trackable_ptr<BattleGround>> BattleGroundSet;

typedef std::unordered_map<uint32, BattleGroundTypeId> BattleMastersMap;
typedef std::unordered_map<uint32, BattleGroundEventIdx> CreatureBattleEventIndexesMap;
typedef std::unordered_map<uint32, BattleGroundEventIdx> GameObjectBattleEventIndexesMap;

class BattleGround;

class BattleGroundMgr
{
    public:
        /* Construction */
        BattleGroundMgr();
        ~BattleGroundMgr();
        void Update(uint32 /*diff*/);

        /* Packet Building */
        static void BuildPlayerJoinedBattleGroundPacket(WorldPacket& /*data*/, Player* /*player*/);
        static void BuildPlayerLeftBattleGroundPacket(WorldPacket& /*data*/, ObjectGuid /*guid*/);
        static void BuildGroupJoinedBattlegroundPacket(WorldPacket& /*data*/, BattleGroundTypeId /*bgTypeId*/, BattleGroundGroupJoinStatus /*status*/);
        static void BuildUpdateWorldStatePacket(WorldPacket& /*data*/, uint32 /*field*/, uint32 /*value*/);
        static void BuildPvpLogDataPacket(WorldPacket& /*data*/, BattleGround* /*bg*/);
        static void BuildBattleGroundStatusPacket(WorldPacket& data, bool bgExists, uint32 bgTypeId, uint32 bgClientInstanceId, uint32 mapId, uint8 queueSlot, uint8 statusId, uint32 time1, uint32 time2);
        static void BuildPlaySoundPacket(WorldPacket& /*data*/, uint32 /*soundId*/);

        /* Battlegrounds */
        BattleGround* GetBattleGroundThroughClientInstance(uint32 /*instanceId*/, BattleGroundTypeId /*bgTypeId*/);
        BattleGround* GetBattleGround(uint32 /*instanceId*/, BattleGroundTypeId /*bgTypeId*/); // there must be uint32 because MAX_BATTLEGROUND_TYPE_ID means unknown

        BattleGround* GetBattleGroundTemplate(BattleGroundTypeId /*bgTypeId*/) const;
        BattleGround* CreateNewBattleGround(BattleGroundTypeId /*bgTypeId*/, BattleGroundBracketId /*bracketId*/, uint32 instanceId, uint32 clientInstanceId);

        uint32 CreateBattleGround(BattleGroundTypeId /*bgTypeId*/, uint32 /*minPlayersPerTeam*/, uint32 /*maxPlayersPerTeam*/, uint32 /*levelMin*/, uint32 /*levelMax*/, char const* /*battleGroundName*/, uint32 /*mapId*/, float /*team1StartLocX*/, float /*team1StartLocY*/, float /*team1StartLocZ*/, float /*team1StartLocO*/, float /*team2StartLocX*/, float /*team2StartLocY*/, float /*team2StartLocZ*/, float /*team2StartLocO*/, float /*startMaxDist*/, uint32 /*playerSkinReflootId*/);

        void AddBattleGround(uint32 instanceId, BattleGroundTypeId bgTypeId, BattleGround* bg);
        void RemoveBattleGround(uint32 instanceId, BattleGroundTypeId bgTypeId) { m_battleGrounds[bgTypeId].erase(instanceId); }

        void CreateInitialBattleGrounds();
        void DeleteAllBattleGrounds();

        void SendToBattleGround(Player* /*player*/, uint32 /*instanceId*/, BattleGroundTypeId /*bgTypeId*/);

        uint32 GetPrematureFinishTime() const;

        void ToggleTesting();

        void LoadBattleMastersEntry(bool reload);
        BattleGroundTypeId GetBattleMasterBG(uint32 entry) const
        {
            BattleMastersMap::const_iterator itr = m_battleMastersMap->find(entry);
            if (itr != m_battleMastersMap->end())
                return itr->second;

            return BATTLEGROUND_TYPE_NONE;
        }
        std::shared_ptr<BattleMastersMap> GetBattleMastersMap() const { return m_battleMastersMap; }

        void LoadBattleEventIndexes(bool reload);
        const BattleGroundEventIdx GetCreatureEventIndex(uint32 dbGuid) const
        {
            CreatureBattleEventIndexesMap::const_iterator itr = m_creatureBattleEventIndexMap->find(dbGuid);
            if (itr != m_creatureBattleEventIndexMap->end())
                return itr->second;

            return m_creatureBattleEventIndexMap->find(static_cast<uint32>(-1))->second;
        }

        std::shared_ptr<CreatureBattleEventIndexesMap> GetCreatureEventIndexes() const { return m_creatureBattleEventIndexMap; }

        const BattleGroundEventIdx GetGameObjectEventIndex(uint32 dbGuid) const
        {
            GameObjectBattleEventIndexesMap::const_iterator itr = m_gameObjectBattleEventIndexMap->find(dbGuid);
            if (itr != m_gameObjectBattleEventIndexMap->end())
                return itr->second;

            return m_gameObjectBattleEventIndexMap->find(static_cast<uint32>(-1))->second;
        }

        std::shared_ptr<GameObjectBattleEventIndexesMap> GetGameObjectEventIndexes() const { return m_gameObjectBattleEventIndexMap; }

        bool IsTesting() const { return m_testing; }

        static BattleGroundQueueTypeId BgQueueTypeId(BattleGroundTypeId /*bgTypeId*/);
        static BattleGroundTypeId BgTemplateId(BattleGroundQueueTypeId /*bgQueueTypeId*/);

        static HolidayIds BgTypeToWeekendHolidayId(BattleGroundTypeId /*bgTypeId*/);
        static BattleGroundTypeId WeekendHolidayIdToBgType(HolidayIds /*holiday*/);
        static bool IsBgWeekend(BattleGroundTypeId /*bgTypeId*/);

        std::set<uint32> const& GetUsedRefLootIds() const { return m_usedRefloot; }

        uint32 GetMinLevelForBattleGroundBracketId(BattleGroundBracketId bracket_id, BattleGroundTypeId bgTypeId) const;
        uint32 GetMaxLevelForBattleGroundBracketId(BattleGroundBracketId bracket_id, BattleGroundTypeId bgTypeId) const;
        BattleGroundBracketId GetBattleGroundBracketIdFromLevel(BattleGroundTypeId bgTypeId, uint32 playerLevel) const;

        Messager<BattleGroundMgr>& GetMessager() { return m_messager; }
    private:
        std::mutex schedulerLock;
        std::shared_ptr<BattleMastersMap> m_battleMastersMap;
        std::shared_ptr<CreatureBattleEventIndexesMap> m_creatureBattleEventIndexMap;
        std::shared_ptr<GameObjectBattleEventIndexesMap> m_gameObjectBattleEventIndexMap;

        /* Battlegrounds */
        BattleGroundSet m_battleGrounds[MAX_BATTLEGROUND_TYPE_ID];
        bool m_testing;
        std::set<uint32> m_usedRefloot;

        Messager<BattleGroundMgr> m_messager;
};

#define sBattleGroundMgr MaNGOS::Singleton<BattleGroundMgr>::Instance()
#endif
