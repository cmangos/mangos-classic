
#include "playerbot/playerbot.h"
#include "DestroyItemAction.h"
#include "playerbot/strategy/values/ItemCountValue.h"

using namespace ai;

bool DestroyItemAction::Execute(Event& event)
{
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();
    std::string text = event.getParam();
    ItemIds ids = chat->parseItems(text);

    for (ItemIds::iterator i =ids.begin(); i != ids.end(); i++)
    {
        FindItemByIdVisitor visitor(*i);
        DestroyItem(&visitor, requester);
    }

    return true;
}

void DestroyItemAction::DestroyItem(FindItemVisitor* visitor, Player* requester)
{
    ai->InventoryIterateItems(visitor, IterateItemsMask::ITERATE_ITEMS_IN_BAGS);
    std::list<Item*> items = visitor->GetResult();
	for (std::list<Item*>::iterator i = items.begin(); i != items.end(); ++i)
    {
		Item* item = *i;
        std::ostringstream out; out << chat->formatItem(item) << " destroyed";
        bot->DestroyItem(item->GetBagSlot(),item->GetSlot(), true);
        ai->TellPlayer(requester, out, PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false);
    }
}

bool SmartDestroyItemAction::Execute(Event& event)
{
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();
    uint8 bagSpace = AI_VALUE(uint8, "bag space");

    if (bagSpace < 90)
        return false;

    bool onlyDestroyGray = false;

    if (ai->HasRealPlayerMaster() || ai->IsInRealGuild())
        onlyDestroyGray = true;

    if(sPlayerbotAIConfig.IsFreeAltBot(bot) && !ai->HasActivePlayerMaster())
        onlyDestroyGray = false;

    // only destroy grey items if with real player/guild
    if (onlyDestroyGray)
    {
        std::set<Item*> items;
        FindItemsToTradeByQualityVisitor visitor(ITEM_QUALITY_POOR, 5);
        ai->InventoryIterateItems(&visitor, IterateItemsMask::ITERATE_ITEMS_IN_BAGS);
        items.insert(visitor.GetResult().begin(), visitor.GetResult().end());

        for (auto& item : items)
        {
            if (HAS_AI_VALUE2("force item usage", item->GetProto()->ItemId))
                continue;

            FindItemByIdVisitor visitor(item->GetProto()->ItemId);
            DestroyItem(&visitor, requester);

            bagSpace = AI_VALUE(uint8, "bag space");

            if (bagSpace < 90)
                return true;
        }

        return true;
    }

    std::vector<ItemUsage> bestToDestroy = { ItemUsage::ITEM_USAGE_NONE }; //First destroy anything useless.

    if (!AI_VALUE(bool, "can sell") && AI_VALUE(bool, "should get money")) //We need money so quest items are less important since they can't directly be sold.
    {
        bestToDestroy.push_back(ItemUsage::ITEM_USAGE_QUEST);
    }
    else //We don't need money so destroy the cheapest stuff.
    {
        bestToDestroy.push_back(ItemUsage::ITEM_USAGE_VENDOR);
        bestToDestroy.push_back(ItemUsage::ITEM_USAGE_BROKEN_AH);
        bestToDestroy.push_back(ItemUsage::ITEM_USAGE_AH);
    }

    //If we still need room 
    bestToDestroy.push_back(ItemUsage::ITEM_USAGE_SKILL); //Items that might help tradeskill are more important than above but still expendable.
    bestToDestroy.push_back(ItemUsage::ITEM_USAGE_USE); //These are more likely to be useful 'soon' but still expendable.

    for (auto& usage : bestToDestroy)
    {
        std::list<uint32> items = AI_VALUE2(std::list<uint32>, "inventory item ids", "usage " + std::to_string((uint8)usage));

        items.reverse();

        for (auto& item : items)
        {
            if (HAS_AI_VALUE2("force item usage", item))
                continue;

            FindItemByIdVisitor visitor(item);
            DestroyItem(&visitor, requester);

            bagSpace = AI_VALUE(uint8, "bag space");

            if(bagSpace < 90)
                return true;
        }
    }

    return false;
}

bool DestroyAllGrayItemsAction::Execute(Event& event)
{
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();

    bool hasDestroyedAnyItems = false;

    for (int i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; ++i)
    {
        if (Bag* pBag = (Bag*)bot->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
        {
            for (uint32 j = 0; j < pBag->GetBagSize(); ++j)
            {
                if (Item* pItem = pBag->GetItemByPos(j))
                {
                    if (const ItemPrototype* proto = pItem->GetProto())
                    {
                        if (proto->Quality == ITEM_QUALITY_POOR)
                        {
                            std::ostringstream out; out << ai->GetChatHelper()->formatItem(pItem->GetProto()) << " destroyed";
                            sLog.outBasic("%s via DestroyAllGrayItemsAction", out.str().c_str());
                            bot->DestroyItem(pItem->GetBagSlot(), pItem->GetSlot(), true);
                            ai->TellPlayer(requester, out, PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false);

                            hasDestroyedAnyItems = true;
                        }
                    }
                }
            }
        }
    }

    for (int i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; ++i)
    {
        if (Item* pItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
        {
            if (const ItemPrototype* proto = pItem->GetProto())
            {
                if (proto->Quality == ITEM_QUALITY_POOR)
                {
                    std::ostringstream out; out << ai->GetChatHelper()->formatItem(pItem->GetProto()) << " destroyed";
                    sLog.outBasic("%s via DestroyAllGrayItemsAction", out.str().c_str());
                    bot->DestroyItem(pItem->GetBagSlot(), pItem->GetSlot(), true);
                    ai->TellPlayer(requester, out, PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false);

                    hasDestroyedAnyItems = true;
                }
            }
        }
    }

    return hasDestroyedAnyItems;
}