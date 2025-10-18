#include "VendorValues.h"
#include "ItemUsageValue.h"
#include "BudgetValues.h"
#include "playerbot/PlayerbotAI.h"
#include "SharedValueContext.h"

using namespace ai;

VendorMap* VendorMapValue::Calculate()
{
    VendorMap* vendorpMap = new VendorMap;


    for (uint32 entry = 0; entry < sCreatureStorage.GetMaxEntry(); entry++)
    {
        CreatureInfo const* cInfo = sObjectMgr.GetCreatureTemplate(entry);

        if (!cInfo)
            continue;

        VendorItemList vendorItems;
        VendorItemData const* vItems = sObjectMgr.GetNpcVendorItemList(entry);

        if (vItems)
            for (auto vItem : vItems->m_items)
                if (!vItem->maxcount) //Ignore limited amount items.
                    vendorpMap->insert(std::make_pair(vItem->item, entry));

        uint32 vendorId = cInfo->VendorTemplateId;
        if (vendorId)
        {
            vItems = sObjectMgr.GetNpcVendorTemplateItemList(vendorId);
            if (vItems)
                for (auto vItem : vItems->m_items)
                    if (!vItem->maxcount) //Ignore limited amount items.
                        vendorpMap->insert(std::make_pair(vItem->item, entry));
        }
    }

    return vendorpMap;
}

//What items does this entry have in its loot list?
std::list<int32> ItemVendorListValue::Calculate()
{
    uint32 itemId = stoi(getQualifier());

    VendorMap* vendorMap = GAI_VALUE(VendorMap*, "vendor map");

    std::list<int32> entries;

    auto range = vendorMap->equal_range(itemId);

    for (auto itr = range.first; itr != range.second; ++itr)
        entries.push_back(itr->second);

    return entries;
}

bool VendorHasUsefulItemValue::Calculate()
{
    uint32 entry = stoi(this->getQualifier());
    CreatureInfo const* cInfo = sObjectMgr.GetCreatureTemplate(entry);

    if (!cInfo)
        return false;

    VendorItemList vendorItems;
    VendorItemData const* vItems = sObjectMgr.GetNpcVendorItemList(entry);

    if (vItems)
        for (auto vItem : vItems->m_items)
                vendorItems.push_back(vItem);

    uint32 vendorId = cInfo->VendorTemplateId;

    if (vendorId)
    {
        vItems = sObjectMgr.GetNpcVendorTemplateItemList(vendorId);

        if (vItems)
            for (auto vItem : vItems->m_items)
                vendorItems.push_back(vItem);
    }

    std::unordered_map <ItemUsage, uint32> freeMoney;

    freeMoney[ItemUsage::ITEM_USAGE_EQUIP] = AI_VALUE2(uint32, "free money for", (uint32)NeedMoneyFor::gear);
    freeMoney[ItemUsage::ITEM_USAGE_USE] = AI_VALUE2(uint32, "free money for", (uint32)NeedMoneyFor::consumables);
    freeMoney[ItemUsage::ITEM_USAGE_SKILL] = freeMoney[ItemUsage::ITEM_USAGE_DISENCHANT] = AI_VALUE2(uint32, "free money for", (uint32)NeedMoneyFor::tradeskill);
    freeMoney[ItemUsage::ITEM_USAGE_AMMO] = AI_VALUE2(uint32, "free money for", (uint32)NeedMoneyFor::ammo);
    freeMoney[ItemUsage::ITEM_USAGE_QUEST] = AI_VALUE2(uint32, "free money for", (uint32)NeedMoneyFor::anything);

    for (auto vendorItem : vendorItems)
    {
        ItemPrototype const* proto = sObjectMgr.GetItemPrototype(vendorItem->item);
#ifndef MANGOSBOT_ZERO
        if (vendorItem->ExtendedCost) //Needs to be replaced with check if bot has (free) currency for this item.
            continue;
#endif

        ItemUsage usage = AI_VALUE2_LAZY(ItemUsage, "item usage", vendorItem->item);

        if (freeMoney.find(usage) == freeMoney.end() || proto->BuyPrice > freeMoney[usage])
            continue;

        return true;
    }

    return false;
}