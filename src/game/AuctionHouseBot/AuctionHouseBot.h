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

struct AuctionHouseBotStatusInfoPerType
{
    uint32 ItemsCount;
    uint32 QualityInfo[MAX_ITEM_QUALITY];
};

typedef AuctionHouseBotStatusInfoPerType AuctionHouseBotStatusInfo[MAX_AUCTION_HOUSE_TYPE];

class AuctionHouseBot {
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

    private:
        uint32 getMinMaxConfig(const char* config, uint32 minValue, uint32 maxValue, uint32 defaultValue);
        void parseLootConfig(char const* fieldname, std::vector<int32>& lootConfig);
        void fillUintVectorFromQuery(char const* query, std::vector<uint32>& lootTemplates);
        void parseItemPriceConfig(char const* fieldname, std::vector<uint32>& itemPrices);
        void addLootToItemMap(LootStore* store, std::vector<int32>& lootConfig, std::vector<uint32>& lootTemplates, std::unordered_map<uint32, uint32>& itemMap);
        uint32 calculateBuyoutPrice(Item* item);

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

        std::vector<std::vector<uint32>> m_itemPrice = std::vector<std::vector<uint32>>(MAX_ITEM_QUALITY, std::vector<uint32>(MAX_ITEM_CLASS));
        uint32 m_vendorPrice;
        uint32 m_itemPriceVariance;
        uint32 m_auctionBidMin;
        uint32 m_auctionBidMax;
        uint32 m_auctionTimeMin;
        uint32 m_auctionTimeMax;

        std::vector<uint32> m_creatureLootNormalTemplates;
        std::vector<uint32> m_creatureLootRareTemplates;
        std::vector<uint32> m_creatureLootEliteTemplates;
        std::vector<uint32> m_creatureLootRareEliteTemplates;
        std::vector<uint32> m_creatureLootWorldBossTemplates;
        std::vector<uint32> m_disenchantLootTemplates;
        std::vector<uint32> m_fishingLootTemplates;
        std::vector<uint32> m_gameobjectLootTemplates;
        std::vector<uint32> m_skinningLootTemplates;

        std::unordered_set<uint32> m_vendorItems;
};

#define sAuctionHouseBot MaNGOS::Singleton<AuctionHouseBot>::Instance()

#endif
