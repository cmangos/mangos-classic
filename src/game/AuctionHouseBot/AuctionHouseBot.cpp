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

#include "AuctionHouseBot.h"
#include "Globals/ObjectMgr.h"
#include "Log.h"
#include "Policies/Singleton.h"
#include "ProgressBar.h"
#include "SystemConfig.h"
#include "World/World.h"

// Format is YYYYMMDDRR where RR is the change in the conf file
// for that day.
#define AUCTIONHOUSEBOT_CONF_VERSION    2020010101

INSTANTIATE_SINGLETON_1(AuctionHouseBot);

AuctionHouseBot::AuctionHouseBot() : m_configFileName(_AUCTIONHOUSEBOT_CONFIG), m_houseAction(-1) {
}

AuctionHouseBot::~AuctionHouseBot() {
}

void AuctionHouseBot::Initialize() {
    if (!m_ahBotCfg.SetSource(m_configFileName)) {
        sLog.outString("AHBot is disabled. Unable to open configuration file(%s).", m_configFileName.c_str());
        return;
    }
    sLog.outString("AHBot using configuration file %s", m_configFileName.c_str());

    m_chanceSell = getMinMaxConfig("AuctionHouseBot.Chance.Sell", 0, 100, 10);
    m_chanceBuy = getMinMaxConfig("AuctionHouseBot.Chance.Buy", 0, 100, 10);

    sLog.outString("AHBot selling items: %s", m_chanceSell > 0 ? "Enabled" : "Disabled");
    sLog.outString("AHBot buying items: %s", m_chanceBuy > 0 ? "Enabled" : "Disabled");

    if (m_chanceSell > 0 || m_chanceBuy > 0) {
        // creature loot
        parseLootConfig("AuctionHouseBot.Loot.Creature.Normal", m_creatureLootNormalConfig);
        parseLootConfig("AuctionHouseBot.Loot.Creature.Elite", m_creatureLootEliteConfig);
        parseLootConfig("AuctionHouseBot.Loot.Creature.RareElite", m_creatureLootRareEliteConfig);
        parseLootConfig("AuctionHouseBot.Loot.Creature.WorldBoss", m_creatureLootWorldBossConfig);
        parseLootConfig("AuctionHouseBot.Loot.Creature.Rare", m_creatureLootRareConfig);
        fillUintVectorFromQuery("SELECT entry FROM creature_template WHERE rank = 0 AND entry IN (SELECT entry FROM creature_loot_template)", m_creatureLootNormalTemplates);
        fillUintVectorFromQuery("SELECT entry FROM creature_template WHERE rank = 1 AND entry IN (SELECT entry FROM creature_loot_template)", m_creatureLootEliteTemplates);
        fillUintVectorFromQuery("SELECT entry FROM creature_template WHERE rank = 2 AND entry IN (SELECT entry FROM creature_loot_template)", m_creatureLootRareEliteTemplates);
        fillUintVectorFromQuery("SELECT entry FROM creature_template WHERE rank = 3 AND entry IN (SELECT entry FROM creature_loot_template)", m_creatureLootWorldBossTemplates);
        fillUintVectorFromQuery("SELECT entry FROM creature_template WHERE rank = 4 AND entry IN (SELECT entry FROM creature_loot_template)", m_creatureLootRareTemplates);

        // disenchant loot
        parseLootConfig("AuctionHouseBot.Loot.Disenchant", m_disenchantLootConfig);
        fillUintVectorFromQuery("SELECT DISTINCT entry FROM disenchant_loot_template", m_disenchantLootTemplates);

        // fishing loot
        parseLootConfig("AuctionHouseBot.Loot.Fishing", m_fishingLootConfig);
        fillUintVectorFromQuery("SELECT DISTINCT entry FROM fishing_loot_template", m_fishingLootTemplates);

        // gameobject loot
        parseLootConfig("AuctionHouseBot.Loot.Gameobject", m_gameobjectLootConfig);
        fillUintVectorFromQuery("SELECT DISTINCT entry FROM gameobject_loot_template WHERE entry IN (SELECT data1 FROM gameobject_template WHERE entry IN (SELECT id FROM gameobject WHERE state = 1 AND spawntimesecsmax > 0))", m_gameobjectLootTemplates);

        // skinning loot
        parseLootConfig("AuctionHouseBot.Loot.Skinning", m_skinningLootConfig);
        fillUintVectorFromQuery("SELECT DISTINCT entry FROM skinning_loot_template", m_skinningLootTemplates);

        // profession items (different than the loot above, but use similar config)
        parseLootConfig("AuctionHouseBot.Items.Profession", m_professionItemsConfig);
        fillUintVectorFromQuery("SELECT entry FROM item_template WHERE entry IN (SELECT EffectItemType1 FROM spell_template WHERE attributes & 32 AND attributes & 65536)", m_professionItems);

        // vendor items (used to prevent items being bought from vendor and sold at ah for profit)
        std::vector<uint32> tmpVector;
        fillUintVectorFromQuery("SELECT item FROM npc_vendor", tmpVector);
        std::copy(tmpVector.begin(), tmpVector.end(), std::inserter(m_vendorItems, m_vendorItems.end()));

        // item value
        parseItemValueConfig("AuctionHouseBot.Value.Poor", m_itemValue[ITEM_QUALITY_POOR]);
        parseItemValueConfig("AuctionHouseBot.Value.Normal", m_itemValue[ITEM_QUALITY_NORMAL]);
        parseItemValueConfig("AuctionHouseBot.Value.Uncommon", m_itemValue[ITEM_QUALITY_UNCOMMON]);
        parseItemValueConfig("AuctionHouseBot.Value.Rare", m_itemValue[ITEM_QUALITY_RARE]);
        parseItemValueConfig("AuctionHouseBot.Value.Epic", m_itemValue[ITEM_QUALITY_EPIC]);
        parseItemValueConfig("AuctionHouseBot.Value.Legendary", m_itemValue[ITEM_QUALITY_LEGENDARY]);
        parseItemValueConfig("AuctionHouseBot.Value.Artifact", m_itemValue[ITEM_QUALITY_ARTIFACT]);

        // item value for items sold by vendors
        m_vendorValue = m_ahBotCfg.GetBoolDefault("AuctionHouseBot.Value.Vendor", true);

        // item value variance
        m_valueVariance = getMinMaxConfig("AuctionHouseBot.Value.Variance", 0, 100, 10);

        // auction min/max bid
        m_auctionBidMin = getMinMaxConfig("AuctionHouseBot.Bid.Min", 0, 100, 75);
        m_auctionBidMax = getMinMaxConfig("AuctionHouseBot.Bid.Max", 0, 100, 90);
        if (m_auctionBidMin > m_auctionBidMax) {
            sLog.outError("AHBot error: AuctionHouseBot.Bid.Min must be less or equal to AuctionHouseBot.Bid.Max. Setting Bid.Min equal to Bid.Max.");
            m_auctionBidMin = m_auctionBidMax;
        }

        // auction min/max time
        m_auctionTimeMin = getMinMaxConfig("AuctionHouseBot.Time.Min", 1, 72, 2);
        m_auctionTimeMax = getMinMaxConfig("AuctionHouseBot.Time.Max", 1, 72, 24);
        if (m_auctionTimeMin > m_auctionTimeMax) {
            sLog.outError("AHBot error: AuctionHouseBot.Time.Min must be less or equal to AuctionHouseBot.Time.Max. Setting Time.Min equal to Time.Max.");
            m_auctionTimeMin = m_auctionTimeMax;
        }

        // buy item value
        m_buyValue = getMinMaxConfig("AuctionHouseBot.Buy.Value", 0, 200, 90);
    }
}

uint32 AuctionHouseBot::getMinMaxConfig(const char* config, uint32 minValue, uint32 maxValue, uint32 defaultValue) {
    uint32 field = m_ahBotCfg.GetIntDefault(config, defaultValue);
    if (field < minValue) {
        sLog.outError("AHBot error: %s must be between %u and %u. Setting value to %u.", config, minValue, maxValue, defaultValue);
        field = defaultValue;
    } else if (field > maxValue) {
        sLog.outError("AHBot error: %s must be between %u and %u. Setting value to %u.", config, minValue, maxValue, defaultValue);
        field = defaultValue;
    }
    return field;
}

void AuctionHouseBot::parseLootConfig(char const* fieldname, std::vector<int32>& lootConfig) {
    std::stringstream includeStream(m_ahBotCfg.GetStringDefault(fieldname));
    std::string temp;
    lootConfig.clear();
    while (getline(includeStream, temp, ','))
        lootConfig.push_back(atoi(temp.c_str()));
    if (lootConfig.size() > 4)
        sLog.outError("AHBot error: Too many values specified for field %s (%lu), 4 values required. Additional values ignored.", fieldname, lootConfig.size());
    else if (lootConfig.size() < 4) {
        sLog.outError("AHBot error: Too few values specified for field %s (%lu), 4 values required. Setting 0 for remaining values.", fieldname, lootConfig.size());
        while (lootConfig.size() < 4)
            lootConfig.push_back(0);
    }
    for (uint32 index = 1; index < 4; ++index) {
        if (lootConfig[index] < 0) {
            sLog.outError("AHBot error: %s value (%d) for field %s should not be a negative number, setting value to 0.", (index == 1 ? "Second" : (index == 2 ? "Third" : "Fourth")), lootConfig[1], fieldname);
            lootConfig[index] = 0;
        }
    }
    if (lootConfig[0] > lootConfig[1]) {
        sLog.outError("AHBot error: First value (%d) must be less than or equal to second value (%d) for field %s. Setting first value to second value.", lootConfig[0], lootConfig[1], fieldname);
        lootConfig[0] = lootConfig[1];
    }
    if (lootConfig[2] > lootConfig[3]) {
        sLog.outError("AHBot error: Third value (%d) must be less than or equal to fourth value (%d) for field %s. Setting third value to fourth value.", lootConfig[2], lootConfig[3], fieldname);
        lootConfig[2] = lootConfig[3];
    }
}

void AuctionHouseBot::fillUintVectorFromQuery(char const* query, std::vector<uint32>& lootTemplates) {
    lootTemplates.clear();
    if (QueryResult* result = WorldDatabase.PQuery("%s", query)) {
        BarGoLink bar(result->GetRowCount());
        do {
            bar.step();
            Field* fields = result->Fetch();
            uint32 entry = fields[0].GetUInt32();
            if (!entry)
                continue;
            lootTemplates.push_back(fields[0].GetUInt32());
        } while (result->NextRow());
        delete result;
    }
}

void AuctionHouseBot::parseItemValueConfig(char const* fieldname, std::vector<uint32>& itemValues) {
    std::stringstream includeStream(m_ahBotCfg.GetStringDefault(fieldname));
    std::string temp;
    for (uint32 index = 0; getline(includeStream, temp, ','); ++index) {
        if (index < itemValues.size())
            itemValues[index] = atoi(temp.c_str());
    }
}

void AuctionHouseBot::addLootToItemMap(LootStore* store, std::vector<int32>& lootConfig, std::vector<uint32>& lootTemplates, std::unordered_map<uint32, uint32>& itemMap) {
    if (lootConfig[1] <= 0 || lootConfig[3] <= 0)
        return;
    int32 maxTemplates = lootConfig[0] < 0 ? urand(0, lootConfig[1] + 1 - lootConfig[0]) + lootConfig[0] : urand(lootConfig[0], lootConfig[1] + 1);
    if (maxTemplates <= 0)
        return;
    for (uint32 templateCounter = 0; templateCounter < maxTemplates; ++templateCounter) {
        uint32 lootTemplate = urand(0, lootTemplates.size());
        LootTemplate const* lootTable = store->GetLootFor(lootTemplates[lootTemplate]);
        if (!lootTable)
            continue;
        std::unique_ptr<Loot> loot = std::unique_ptr<Loot>(new Loot(LOOT_DEBUG));
        for (uint32 repeat = urand(lootConfig[2], lootConfig[3] + 1); repeat > 0; --repeat)
            lootTable->Process(*loot, nullptr, *store, store->IsRatesAllowed());

        LootItem* lootItem;
        for (uint32 slot = 0; (lootItem = loot->GetLootItemInSlot(slot)); ++slot)
            itemMap[lootItem->itemId] += lootItem->count;
    }
}

bool AuctionHouseBot::ReloadAllConfig() {
    Initialize();
    return true;
}

void AuctionHouseBot::PrepareStatusInfos(AuctionHouseBotStatusInfo& statusInfo) const {
    for (uint32 i = 0; i < MAX_AUCTION_HOUSE_TYPE; ++i) {
        statusInfo[i].ItemsCount = 0;

        for (unsigned int& j : statusInfo[i].QualityInfo)
            j = 0;

        AuctionHouseObject::AuctionEntryMapBounds bounds = sAuctionMgr.GetAuctionsMap(AuctionHouseType(i))->GetAuctionsBounds();
        for (AuctionHouseObject::AuctionEntryMap::const_iterator itr = bounds.first; itr != bounds.second; ++itr) {
            AuctionEntry* entry = itr->second;
            if (Item* item = sAuctionMgr.GetAItem(entry->itemGuidLow)) {
                ItemPrototype const* prototype = item->GetProto();
                if (!entry->owner) { // count only ahbot auctions
                    if (prototype->Quality < MAX_ITEM_QUALITY)
                        ++statusInfo[i].QualityInfo[prototype->Quality];

                    ++statusInfo[i].ItemsCount;
                }
            }
        }
    }
}

void AuctionHouseBot::Rebuild(bool all) {
    sLog.outError("AHBot: Rebuilding auction house items");
    for (uint32 i = 0; i < MAX_AUCTION_HOUSE_TYPE; ++i) {
        AuctionHouseObject::AuctionEntryMapBounds bounds = sAuctionMgr.GetAuctionsMap(AuctionHouseType(i))->GetAuctionsBounds();
        for (AuctionHouseObject::AuctionEntryMap::const_iterator itr = bounds.first; itr != bounds.second; ++itr) {
            AuctionEntry* entry = itr->second;
            if (!entry->owner) { // ahbot auction
                if (all || entry->bid == 0) // expire auction if no bid or forced
                    entry->expireTime = sWorld.GetGameTime();
            }
        }
    }
    // refill auction house with items, simulating typical max amount of items available after some time
    uint32 updateCounter = ((m_auctionTimeMax - m_auctionTimeMin) / 2 + m_auctionTimeMin) * 90;
    for (uint32 i = 0; i < updateCounter; ++i) {
        if (m_houseAction >= MAX_AUCTION_HOUSE_TYPE - 1)
            m_houseAction = -1; // this prevents AHBot from buying items when refilling
        Update();
    }
}

void AuctionHouseBot::Update() {
    if (++m_houseAction >= MAX_AUCTION_HOUSE_TYPE * 2)
        m_houseAction = 0;

    AuctionHouseType houseType = AuctionHouseType(m_houseAction % MAX_AUCTION_HOUSE_TYPE);
    AuctionHouseObject* auctionHouse = sAuctionMgr.GetAuctionsMap(houseType);
    if (m_houseAction < MAX_AUCTION_HOUSE_TYPE && urand(0, 100) < m_chanceSell) {
        // Sell items
        std::unordered_map<uint32, uint32> itemMap;

        addLootToItemMap(&LootTemplates_Creature, m_creatureLootNormalConfig, m_creatureLootNormalTemplates, itemMap);       // normal creature loot
        addLootToItemMap(&LootTemplates_Creature, m_creatureLootEliteConfig, m_creatureLootEliteTemplates, itemMap);         // elite creature loot
        addLootToItemMap(&LootTemplates_Creature, m_creatureLootRareEliteConfig, m_creatureLootRareEliteTemplates, itemMap); // rare elite creature loot
        addLootToItemMap(&LootTemplates_Creature, m_creatureLootWorldBossConfig, m_creatureLootWorldBossTemplates, itemMap); // world boss creature loot
        addLootToItemMap(&LootTemplates_Creature, m_creatureLootRareConfig, m_creatureLootRareTemplates, itemMap);           // rare creature loot

        addLootToItemMap(&LootTemplates_Disenchant, m_disenchantLootConfig, m_disenchantLootTemplates, itemMap);             // disenchant loot
        addLootToItemMap(&LootTemplates_Fishing, m_fishingLootConfig, m_fishingLootTemplates, itemMap);                      // fishing loot
        addLootToItemMap(&LootTemplates_Gameobject, m_gameobjectLootConfig, m_gameobjectLootTemplates, itemMap);             // gameobject loot
        addLootToItemMap(&LootTemplates_Skinning, m_skinningLootConfig, m_skinningLootTemplates, itemMap);                   // skinning loot

        // profession items are a bit different (not looted)
        if (m_professionItemsConfig[1] > 0 && m_professionItemsConfig[3] > 0) {
            int32 maxTemplates = m_professionItemsConfig[0] < 0 ? urand(0, m_professionItemsConfig[1] + 1 - m_professionItemsConfig[0]) + m_professionItemsConfig[0] : urand(m_professionItemsConfig[0], m_professionItemsConfig[1] + 1);
            if (maxTemplates > 0) {
                for (uint32 templateCounter = 0; templateCounter < maxTemplates; ++templateCounter) {
                    uint32 item = m_professionItems[urand(0, m_professionItems.size())];
                    ItemPrototype const* prototype = ObjectMgr::GetItemPrototype(item);
                    if (!prototype || prototype->Quality == 0 || urand(0, 1 << (prototype->Quality - 1)) > 0)
                        continue; // make it decreasingly likely that crafted items of higher quality is added to the auction house (white: 100%, green: 50%, blue: 25%, purple: 12.5%, ...)
                    uint32 count = (uint32) round(prototype->GetMaxStackSize() * urand(m_professionItemsConfig[2], m_professionItemsConfig[3] + 1) / 100.0);
                    itemMap[item] += count;
                }
            }
        }

        for (auto itemEntry : itemMap) {
            ItemPrototype const* prototype = sObjectMgr.GetItemPrototype(itemEntry.first);
            if (!prototype || prototype->GetMaxStackSize() == 0)
                continue; // really shouldn't happen, but better safe than sorry
            if (prototype->Bonding == BIND_WHEN_PICKED_UP || prototype->Bonding == BIND_QUEST_ITEM)
                continue; // nor BoP and quest items
            if (prototype->Flags & ITEM_FLAG_HAS_LOOT)
                continue; // no items containing loot
            if (m_itemValue[prototype->Quality][prototype->Class] == 0)
                continue; // item class is filtered out

            for (uint32 stackCounter = 0; stackCounter < itemEntry.second; stackCounter += prototype->GetMaxStackSize()) {
                uint32 count = itemEntry.second - stackCounter > prototype->GetMaxStackSize() ? prototype->GetMaxStackSize() : itemEntry.second - stackCounter;
                uint32 buyoutPrice = calculateBuyoutPrice(prototype, count);
                Item* item = Item::CreateItem(itemEntry.first, count);
                if (buyoutPrice == 0 || !item)
                    continue; // don't put up items we don't know the value of
                uint32 bidPrice = buyoutPrice * (urand(m_auctionBidMin, m_auctionBidMax + 1)) / 100;
                if (item)
                    auctionHouse->AddAuction(sAuctionHouseStore.LookupEntry(houseType == AUCTION_HOUSE_ALLIANCE ? 1 : (houseType == AUCTION_HOUSE_HORDE ? 6 : 7)), item, urand(m_auctionTimeMin, m_auctionTimeMax + 1) * HOUR, bidPrice, buyoutPrice);
            }
        }
    } else if (m_houseAction >= MAX_AUCTION_HOUSE_TYPE && urand(0, 100) < m_chanceBuy) {
        // Buy items
        AuctionHouseObject::AuctionEntryMapBounds bounds = auctionHouse->GetAuctionsBounds();
        std::vector<AuctionEntry*> buyoutAuctions;
        for (AuctionHouseObject::AuctionEntryMap::const_iterator itr = bounds.first; itr != bounds.second; ++itr) {
            AuctionEntry* auction = itr->second;
            if (auction->owner == 0 && auction->bid == 0)
                continue; // ignore bidding/buying auctions that were created by server and not bidded on by player
            Item* item = sAuctionMgr.GetAItem(auction->itemGuidLow);
            uint32 buyItemCheck = urand(0, calculateBuyoutPrice(item->GetProto(), item->GetCount()) * m_buyValue / 100);
            uint32 bidPrice = auction->bid + auction->GetAuctionOutBid();
            if (auction->startbid > bidPrice)
                bidPrice = auction->startbid;
            if (auction->buyout > 0 && buyItemCheck > auction->buyout)
                buyoutAuctions.push_back(auction); // can't buyout item here as that modifies the AuctionEntryMap, invalidating the iterator
            else if (buyItemCheck > bidPrice)
                auction->UpdateBid(bidPrice);
        }
        for (auto auction : buyoutAuctions)
            auction->UpdateBid(auction->buyout);
    }
}

uint32 AuctionHouseBot::calculateBuyoutPrice(ItemPrototype const* prototype, uint32 count) {
    if (!prototype)
        return 0;
    uint32 buyoutPrice = prototype->BuyPrice;
    if (buyoutPrice == 0) // if no buy price then use sell price multiplied by 4 if white or gray item, 5 if green or better
        buyoutPrice = prototype->SellPrice * (prototype->Quality <= ITEM_QUALITY_NORMAL ? 4 : 5);
    // multiply buyoutPrice with count and item quality price percentage
    // if item is sold by a vendor and vendor value is forced, then multiply by 100 (setting vendor price)
    buyoutPrice *= count * (m_vendorValue && m_vendorItems.find(prototype->ItemId) != m_vendorItems.end() ? 100 : m_itemValue[prototype->Quality][prototype->Class]);
    buyoutPrice += ((int32) urand(0, m_valueVariance * 2 + 1) - (int32) m_valueVariance) * (int32) (buyoutPrice / 100);
    buyoutPrice /= 100; // since we multiplied with m_itemValue
    return buyoutPrice;
}
