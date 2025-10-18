
#include "playerbot/playerbot.h"
#include "BuyAction.h"
#include "playerbot/strategy/ItemVisitors.h"
#include "playerbot/strategy/values/ItemCountValue.h"
#include "playerbot/strategy/values/BudgetValues.h"
#include "playerbot/strategy/values/MountValues.h"

using namespace ai;

bool BuyAction::Execute(Event& event)
{
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();
    bool buyUseful = false;
    ItemIds itemIds;
    std::string link = event.getParam();

    if (link == "vendor")
        buyUseful = true;
    else
    {
        itemIds = chat->parseItems(link);
    }

    std::list<ObjectGuid> vendors = ai->GetAiObjectContext()->GetValue<std::list<ObjectGuid> >("nearest npcs")->Get();
    bool vendored = false, result = false;

    UsageBoughtList bought;

    for (std::list<ObjectGuid>::iterator i = vendors.begin(); i != vendors.end(); ++i)
    {
        ObjectGuid vendorguid = *i;
        Creature *pCreature = bot->GetNPCIfCanInteractWith(vendorguid, UNIT_NPC_FLAG_VENDOR);
        if (!pCreature)
            continue;

        vendored = true;

        if (buyUseful)
        {
            //Items are evaluated from high-level to low level.
            //For each item the bot checks again if an item is useful.
            //Bot will buy until no useful items are left.

            VendorItemData const* tItems = pCreature->GetVendorItems();
            VendorItemData const* vItems = {};
#ifndef MANGOSBOT_ZERO                
            vItems = pCreature->GetVendorTemplateItems();
#endif
            if (!tItems && !vItems)
                continue;
            
            VendorItemList m_items_sorted;
            
            if (tItems)
                m_items_sorted.insert(m_items_sorted.begin(), tItems->m_items.begin(), tItems->m_items.end());
            if (vItems)
                m_items_sorted.insert(m_items_sorted.begin(), vItems->m_items.begin(), vItems->m_items.end());
            

            m_items_sorted.erase(std::remove_if(m_items_sorted.begin(), m_items_sorted.end(), [](VendorItem* i) {ItemPrototype const* proto = sObjectMgr.GetItemPrototype(i->item); return !proto; }), m_items_sorted.end());

            if (m_items_sorted.empty())
                continue;

            std::sort(m_items_sorted.begin(), m_items_sorted.end(), [](VendorItem* i, VendorItem* j) {return sObjectMgr.GetItemPrototype(i->item)->ItemLevel > sObjectMgr.GetItemPrototype(j->item)->ItemLevel; });

            for (auto& tItem : m_items_sorted)
            {
                ItemPrototype const* proto = sObjectMgr.GetItemPrototype(tItem->item);
                if (!proto)
                    continue;

                // reputation discount 
                uint32 price = uint32(floor(proto->BuyPrice * bot->GetReputationPriceDiscount(pCreature)));

                auto pmo = sPerformanceMonitor.start(PERF_MON_VALUE, "IsWorthBuyingFromVendorToResellAtAH", &context->performanceStack);

                // if item is worth selling to AH? 
                bool canFlipAH = ItemUsageValue::IsWorthBuyingFromVendorToResellAtAH(proto, tItem->maxcount > 0);

                pmo.reset();

#ifndef MANGOSBOT_ZERO
                const ItemExtendedCostEntry* iece = nullptr;
                if (tItem->ExtendedCost)
                {
                    iece = sItemExtendedCostStore.LookupEntry(tItem->ExtendedCost);
                    if (!iece)
                        continue;
                }
#endif

                for (uint32 n = 0; n < 10; ++n) //Buy 10 times or until no longer usefull/possible 
                {
                    ItemUsage usage = AI_VALUE2(ItemUsage, "item usage", tItem->item);

                    uint32 moneyKey = 0;
                    bool usageAllowed = true;
                    switch (usage)
                    {
                        case ItemUsage::ITEM_USAGE_EQUIP: moneyKey = (uint32)NeedMoneyFor::gear; break;
                        case ItemUsage::ITEM_USAGE_USE: moneyKey = (uint32)NeedMoneyFor::consumables; break;
                        case ItemUsage::ITEM_USAGE_SKILL: moneyKey = (uint32)NeedMoneyFor::tradeskill; break;
                        case ItemUsage::ITEM_USAGE_AMMO: moneyKey = (uint32)NeedMoneyFor::ammo; break;
                        case ItemUsage::ITEM_USAGE_QUEST:
                        case ItemUsage::ITEM_USAGE_FORCE_NEED:
                        case ItemUsage::ITEM_USAGE_FORCE_GREED: moneyKey = (uint32)NeedMoneyFor::anything; break;
                        case ItemUsage::ITEM_USAGE_AH:
                            usageAllowed = canFlipAH;
                            moneyKey = (uint32)NeedMoneyFor::anything;
                            break;
                        default: usageAllowed = false; break;
                    }
                    if (!usageAllowed)
                        break;

                    // Gold affordability 
                    RESET_AI_VALUE2(uint32, "free money for", moneyKey);
                    uint32 money = AI_VALUE2(uint32, "free money for", moneyKey);
                    if (price > money)
                        break;

#ifndef MANGOSBOT_ZERO
                    // ExtendedCost check
                    if (iece)
                    {
                        if (iece->reqhonorpoints && bot->GetHonorPoints() < iece->reqhonorpoints)
                            break;
                        if (iece->reqarenapoints && bot->GetArenaPoints() < iece->reqarenapoints)
                            break;

                        bool itemsOk = true;
                        for (uint8 k = 0; k < MAX_EXTENDED_COST_ITEMS; ++k)
                        {
                            if (iece->reqitem[k] && !bot->HasItemCount(iece->reqitem[k], iece->reqitemcount[k]))
                            {
                                itemsOk = false;
                                break;
                            }
                        }
                        if (!itemsOk)
                            break;
#ifdef MANGOSBOT_TWO
                        if (iece->reqpersonalarenarating && bot->GetMaxPersonalArenaRatingRequirement(iece->reqarenaslot) < iece->reqpersonalarenarating)
                            break;
#endif
                    }
#endif

                    if (usage == ItemUsage::ITEM_USAGE_USE && ItemUsageValue::CurrentStacks(ai, proto) >= 1)
                        break;

                    bool didBuy = false;
                    didBuy = BuyItem(requester, tItems, vendorguid, proto, bought, usage);
                    if (!didBuy)
                        didBuy = BuyItem(requester, vItems, vendorguid, proto, bought, usage);

                    result |= didBuy;
                    if (!didBuy)
                        break;

                    RESET_AI_VALUE2(ItemUsage, "item usage", tItem->item);
                    RESET_AI_VALUE(std::vector<MountValue>, "mount list");

                    if (usage == ItemUsage::ITEM_USAGE_EQUIP || usage == ItemUsage::ITEM_USAGE_BAD_EQUIP) //Equip upgrades and stop buying this time. 
                    {
                        RESET_AI_VALUE2(ItemUsage, "item usage", tItem->item);
                        ai->DoSpecificAction("equip upgrades", event, true);
                        break;
                    }
                }
            }
        }
        else
        {
            if (itemIds.empty())
                return false;

            for (ItemIds::iterator i = itemIds.begin(); i != itemIds.end(); i++)
            {
                uint32 itemId = *i;
                const ItemPrototype* proto = sObjectMgr.GetItemPrototype(itemId);
                if (!proto)
                    continue;

                result |= BuyItem(requester, pCreature->GetVendorItems(), vendorguid, proto, bought);
                if (!result)
                    result |= BuyItem(requester, pCreature->GetVendorTemplateItems(), vendorguid, proto, bought);

                if (!result)
                {
                    std::ostringstream out; out << "Nobody sells " << ChatHelper::formatItem(proto) << " nearby";
                    ai->TellPlayer(requester, out.str(), PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false);
                }
            }
        }
    }

    if (!vendored)
    {
        ai->TellError(requester, "There are no vendors nearby");
        return false;
    }
    else
    {
        for (auto& [usage, boughtList] : bought)
        {
            for (auto& [id, count] : boughtList)
            {                
                ItemQualifier qualifier(id);

                std::ostringstream out; 
                
                out << "Buying " << ChatHelper::formatItem(qualifier, count) << " ";
                out << ItemUsageValue::ReasonForNeed(usage, qualifier, count, bot);
                ai->TellPlayer(requester, out.str(), PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false);
            }
        }
    }

    return result;
}

bool BuyAction::BuyItem(Player* requester, VendorItemData const* tItems, ObjectGuid vendorguid, const ItemPrototype* proto, UsageBoughtList& bought, ItemUsage usage)
{
    uint32 oldCount = AI_VALUE2(uint32, "item count", proto->Name1);

    if (!tItems)
        return false;

    uint32 itemId = proto->ItemId;
    for (uint32 slot = 0; slot < tItems->GetItemCount(); slot++)
    {
        if (tItems->GetItem(slot)->item == itemId)
        {       
            uint32 botMoney = bot->GetMoney();
            if (ai->HasCheat(BotCheatMask::gold))
            {
                bot->SetMoney(10000000);
            }

#ifdef MANGOSBOT_TWO
            bot->BuyItemFromVendorSlot(vendorguid, slot, itemId, 1, NULL_BAG, NULL_SLOT);
#else
            bot->BuyItemFromVendor(vendorguid, itemId, 1, NULL_BAG, NULL_SLOT);
#endif
            if (ai->HasCheat(BotCheatMask::gold))
            {
                bot->SetMoney(botMoney);
            }

            if (oldCount < AI_VALUE2(uint32, "item count", proto->Name1)) //BuyItem Always returns false (unless unique) so we have to check the item counts.
            {
                sPlayerbotAIConfig.logEvent(ai, "BuyAction", proto->Name1, std::to_string(proto->ItemId));

                if (usage == ItemUsage::ITEM_USAGE_NONE)
                {

                    std::ostringstream out; out << "Buying " << ChatHelper::formatItem(proto);
                    ai->TellPlayer(requester, out.str(), PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false);
                }
                else if (usage == ItemUsage::ITEM_USAGE_EQUIP) //We need to put these here since we are only buying 1 (hopefully) and need to report ReasonForNeed with old item still equiped.
                {
                    ItemQualifier qualifier(proto->ItemId);

                    std::ostringstream out;

                    out << "Buying " << ChatHelper::formatItem(qualifier) << " ";
                    out << ItemUsageValue::ReasonForNeed(usage, qualifier, 1, bot);
                    ai->TellPlayer(requester, out.str(), PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false);
                }
                else
                {
                    bought[usage][proto->ItemId]++;
                }
                return true;
            }
 
            return false;
        }
    }

    return false;
}

bool BuyBackAction::Execute(Event& event)
{
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();
    std::string text = event.getParam();

    std::list<Item*> found = ai->InventoryParseItems(text, IterateItemsMask::ITERATE_ITEMS_IN_BUYBACK);

    //Sort items on itemLevel descending.
    found.sort([](Item* i, Item* j) {return i->GetProto()->ItemLevel > j->GetProto()->ItemLevel; });

    if (found.empty())
    {
        ai->TellError(requester, "No buyback items found");
        return false;
    }

    bool hasVendor = false;
    //Find vendor to interact with.
    ObjectGuid vendorguid;

    std::list<ObjectGuid> vendors = ai->GetAiObjectContext()->GetValue<std::list<ObjectGuid> >("nearest npcs")->Get();
    for (std::list<ObjectGuid>::iterator i = vendors.begin(); i != vendors.end(); ++i)
    {
        vendorguid = *i;
        Creature* pCreature = bot->GetNPCIfCanInteractWith(vendorguid, UNIT_NPC_FLAG_VENDOR);
        if (!pCreature)
            continue;

        hasVendor = true;
        sServerFacade.SetFacingTo(bot, pCreature);
        break;
    }

    if (!hasVendor)
    {
        ai->TellError(requester, "There are no vendors nearby");
        return false;
    }

    bool result = false;

    for (auto& item : found)
    {
        uint32 slot = BUYBACK_SLOT_START;

        while (slot < BUYBACK_SLOT_END && bot->GetItemFromBuyBackSlot(slot) != item)
            slot++;

        if (slot == BUYBACK_SLOT_END)
            continue;

        uint32 price = bot->GetUInt32Value(PLAYER_FIELD_BUYBACK_PRICE_1 + slot - BUYBACK_SLOT_START);
        if (bot->GetMoney() < price)
            continue;

        WorldPacket p1(CMSG_BUYBACK_ITEM);
        p1 << vendorguid;
        p1 << slot;
        bot->GetSession()->HandleBuybackItem(p1);
        if (bot->GetItemFromBuyBackSlot(slot) == nullptr)
            result = true;
    }

    return result;
}
