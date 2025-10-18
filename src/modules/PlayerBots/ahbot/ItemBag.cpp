
#include "Category.h"
#include "ItemBag.h"
#include "ConsumableCategory.h"
#include "TradeCategory.h"
#include "AhBotConfig.h"
#include "Server/DBCStructure.h"
#include "Log/Log.h"
#include "Database/QueryResult.h"
#include "Database/DatabaseEnv.h"
#include "Database/SQLStorage.h"
#include "Database/DBCStore.h"
#include "Server/SQLStorages.h"
#include "AuctionHouse/AuctionHouseMgr.h"
#include "Globals/ObjectMgr.h"

using namespace ahbot;
char * strstri (const char* str1, const char* str2);

CategoryList CategoryList::instance;

CategoryList::CategoryList()
{
    Add(new Equip());
    Add(new Quiver());
    Add(new Container());
    Add(new Projectile());
    Add(new Recipe());
    Add(new Alchemy());
    Add(new Scroll());
    Add(new Food());
    Add(new Bandage());
    Add(new ItemEnchant());
    Add(new Reagent());
    Add(new ahbot::Quest());
    Add(new DevicesAndParts());

    Add(new TradeSkill(SKILL_TAILORING, true));
    Add(new TradeSkill(SKILL_LEATHERWORKING, true));
    Add(new TradeSkill(SKILL_ENGINEERING, true));
    Add(new TradeSkill(SKILL_BLACKSMITHING, true));
    Add(new TradeSkill(SKILL_ALCHEMY, true));
    Add(new TradeSkill(SKILL_ENCHANTING, true));
    Add(new TradeSkill(SKILL_FISHING, true));
    Add(new TradeSkill(SKILL_FIRST_AID, true));
    Add(new TradeSkill(SKILL_COOKING, true));

    Add(new TradeSkill(SKILL_MINING, true));
    Add(new TradeSkill(SKILL_HERBALISM, true));
    Add(new TradeSkill(SKILL_SKINNING, true));

    Add(new TradeSkill(SKILL_TAILORING, false));
    Add(new TradeSkill(SKILL_LEATHERWORKING, false));
    Add(new TradeSkill(SKILL_ENGINEERING, false));
    Add(new TradeSkill(SKILL_BLACKSMITHING, false));
    Add(new TradeSkill(SKILL_ALCHEMY, false));
    Add(new TradeSkill(SKILL_ENCHANTING, false));
    Add(new TradeSkill(SKILL_FISHING, false));
    Add(new TradeSkill(SKILL_FIRST_AID, false));
    Add(new TradeSkill(SKILL_COOKING, false));

#ifndef MANGOSBOT_ZERO
    Add(new TradeSkill(SKILL_JEWELCRAFTING, true));
    Add(new TradeSkill(SKILL_JEWELCRAFTING, false));
#endif
}

void CategoryList::Add(Category* category)
{
    for (uint32 quality = ITEM_QUALITY_NORMAL; quality <= ITEM_QUALITY_LEGENDARY; ++quality)
        categories.push_back(new QualityCategoryWrapper(category, quality));
}

CategoryList::~CategoryList()
{
    for (std::vector<Category*>::const_iterator i = categories.begin(); i != categories.end(); ++i)
        delete *i;
}

ItemBag::ItemBag()
{
    for (int i = 0; i < CategoryList::instance.size(); i++)
    {
        content[CategoryList::instance[i]] = std::vector<uint32>();
    }
}

void ItemBag::Init(bool silent)
{
    // test skip ahbot loading at start
    // return;

    if (silent)
    {
        Load();
        return;
    }

    sLog.outString("Loading/Scanning %s...", GetName().c_str());

    Load();
	uint32 ahcounter = 0;
    for (int i = 0; i < CategoryList::instance.size(); i++)
    {
        Category* category = CategoryList::instance[i];
        Shuffle(content[category]);
        sLog.outDetail("loaded %zu %s items", content[category].size(), category->GetDisplayName().c_str());
		ahcounter += content[category].size();
    }
	sLog.outString("Loaded %u items", ahcounter);
}

int32 ItemBag::GetCount(Category* category, uint32 item)
{
    uint32 count = 0;

    std::vector<uint32>& items = content[category];
    for (std::vector<uint32>::iterator i = items.begin(); i != items.end(); ++i)
    {
        if (*i == item)
            count++;
    }

    return count;
}

bool ItemBag::Add(ItemPrototype const* proto)
{
    if (!proto ||
        proto->Bonding == BIND_WHEN_PICKED_UP ||
        proto->Bonding == BIND_QUEST_ITEM)
        return false;

    if (proto->RequiredLevel > sAhBotConfig.maxRequiredLevel || proto->ItemLevel > sAhBotConfig.maxItemLevel)
        return false;

    if (proto->Duration > 0)
        return false;

    if (proto->Area)
        return false;

    if (proto->Duration & 0x80000000)
        return false;

    if (sAhBotConfig.ignoreItemIds.find(proto->ItemId) != sAhBotConfig.ignoreItemIds.end())
        return false;

    if (strstri(proto->Name1, "qa") || strstri(proto->Name1, "test") || strstri(proto->Name1, "deprecated"))
        return false;

    bool contains = false;
    for (int i = 0; i < CategoryList::instance.size(); i++)
    {
        if (CategoryList::instance[i]->Contains(proto))
        {
            content[CategoryList::instance[i]].push_back(proto->ItemId);
            contains = true;
        }
    }

    if (!contains)
        sLog.outDetail("Item %s does not included in any category", proto->Name1);

    return contains;
}

void AvailableItemsBag::Load()
{
    std::set<uint32> vendorItems;

      auto results = WorldDatabase.PQuery("SELECT item FROM npc_vendor where maxcount = 0");
      if (results != NULL)
      {
          do
          {
              Field* fields = results->Fetch();
              vendorItems.insert(fields[0].GetUInt32());
          } while (results->NextRow());
      }

      BarGoLink bar(sItemStorage.GetMaxEntry());
      for (uint32 itemId = 0; itemId < sItemStorage.GetMaxEntry(); ++itemId)
      {
          if (vendorItems.find(itemId) == vendorItems.end() || sAhBotConfig.ignoreVendorItemIds.find(itemId) != sAhBotConfig.ignoreVendorItemIds.end())
              Add(sObjectMgr.GetItemPrototype(itemId));

          bar.step();
    }

}

void InAuctionItemsBag::Load()
{
    AuctionHouseEntry const* ahEntry = sAuctionHouseStore.LookupEntry(auctionId);
    if(!ahEntry)
        return;

    AuctionHouseObject* auctionHouse = sAuctionMgr.GetAuctionsMap(ahEntry);
    AuctionHouseObject::AuctionEntryMap const& auctionEntryMap = auctionHouse->GetAuctions();
    for (AuctionHouseObject::AuctionEntryMap::const_iterator itr = auctionEntryMap.begin(); itr != auctionEntryMap.end(); ++itr)
    {
        ItemPrototype const* proto = sObjectMgr.GetItemPrototype(itr->second->itemTemplate);
        if (!proto)
            continue;

        Add(proto);
    }
}

std::string InAuctionItemsBag::GetName()
{
    std::ostringstream out; out << "auction house " << auctionId;
    return out.str();
}
