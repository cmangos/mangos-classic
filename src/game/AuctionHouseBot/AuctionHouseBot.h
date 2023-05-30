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

#ifndef AUCTION_HOUSE_BOT_H
#define AUCTION_HOUSE_BOT_H

#include "AuctionHouse/AuctionHouseMgr.h"
#include "Config/Config.h"
#include "Entities/ItemPrototype.h"
#include "Globals/SharedDefines.h"
#include "Loot/LootMgr.h"
#include "Util/Util.h"

struct AuctionHouseBotItemData
{
    uint32 Value = 0;
    uint32 AddChance = 0;
    uint32 MinAmount = 0;
    uint32 MaxAmount = 0;
};

struct AuctionHouseBotStatusInfoPerType
{
    uint32 ItemsCount;
    uint32 QualityInfo[MAX_ITEM_QUALITY];
};

typedef AuctionHouseBotStatusInfoPerType AuctionHouseBotStatusInfo[MAX_AUCTION_HOUSE_TYPE];

class AuctionHouseBot
{
    public:
        AuctionHouseBot();
        ~AuctionHouseBot();

        void Initialize();
        void SetConfigFileName(const std::string& filename) { m_configFileName = filename; }
        void Update();

        // Following methods are mainly used by level3.cpp for ingame/console commands
        bool ReloadAllConfig();
        void Rebuild(bool all);
        void PrepareStatusInfos(AuctionHouseBotStatusInfo& statusInfo) const;
        void SetItemData(uint32 item, AuctionHouseBotItemData& itemData, bool reset = false);
        AuctionHouseBotItemData GetItemData(uint32 item);

    private:
        uint32 GetMinMaxConfig(const char* config, uint32 minValue, uint32 maxValue, uint32 defaultValue);
        void ParseLootConfig(char const* fieldname, std::vector<int32>& lootConfig);
        void FillUintVectorFromQuery(char const* query, std::vector<uint32>& lootTemplates);
        void ParseItemValueConfig(char const* fieldname, std::vector<uint32>& itemValues);
        void AddLootToItemMap(LootStore* store, std::vector<int32>& lootConfig, std::vector<uint32>& lootTemplates, std::unordered_map<uint32, uint32>& itemMap);
        uint32 CalculateBuyoutPrice(ItemPrototype const* prototype);
        uint32 ValueWithVariance(uint32 itemValue) { return (uint32) (itemValue + ((int32) urand(0, m_valueVariance * 2 + 1) - (int32) m_valueVariance) * (int32) (itemValue / 100)); };

        std::string m_configFileName;
        Config m_ahBotCfg;

        uint32 m_houseAction;

        uint32 m_chanceSell;
        uint32 m_chanceBuy;

        std::vector<int32> m_creatureLootNormalConfig;
        std::vector<int32> m_creatureLootRareConfig;
        std::vector<int32> m_creatureLootEliteConfig;
        std::vector<int32> m_creatureLootRareEliteConfig;
        std::vector<int32> m_creatureLootWorldBossConfig;
        std::vector<int32> m_disenchantLootConfig;
        std::vector<int32> m_fishingLootConfig;
        std::vector<int32> m_gameobjectLootConfig;
        std::vector<int32> m_skinningLootConfig;
        std::vector<int32> m_professionItemsConfig;

        std::vector<std::vector<uint32>> m_itemValue = std::vector<std::vector<uint32>>(MAX_ITEM_QUALITY, std::vector<uint32>(MAX_ITEM_CLASS));
        bool m_vendorValue;
        uint32 m_valueVariance;
        uint32 m_auctionBidMin;
        uint32 m_auctionBidMax;
        uint32 m_auctionTimeMin;
        uint32 m_auctionTimeMax;
        uint32 m_buyValue;

        std::vector<uint32> m_creatureLootNormalTemplates;
        std::vector<uint32> m_creatureLootRareTemplates;
        std::vector<uint32> m_creatureLootEliteTemplates;
        std::vector<uint32> m_creatureLootRareEliteTemplates;
        std::vector<uint32> m_creatureLootWorldBossTemplates;
        std::vector<uint32> m_disenchantLootTemplates;
        std::vector<uint32> m_fishingLootTemplates;
        std::vector<uint32> m_gameobjectLootTemplates;
        std::vector<uint32> m_skinningLootTemplates;
        std::vector<uint32> m_professionItems;

        std::unordered_set<uint32> m_vendorItems;

        std::unordered_map<uint32, AuctionHouseBotItemData> m_itemData;
};

#define sAuctionHouseBot MaNGOS::Singleton<AuctionHouseBot>::Instance()

#endif
