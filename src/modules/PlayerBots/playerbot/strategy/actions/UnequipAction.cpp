
#include "playerbot/playerbot.h"
#include "UnequipAction.h"

#include "playerbot/strategy/values/ItemCountValue.h"

using namespace ai;

std::vector<std::string> split(const std::string &s, char delim);

bool UnequipAction::Execute(Event& event)
{
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();
    std::string text = event.getParam();
    std::list<Item*> found = ai->InventoryParseItems(text, IterateItemsMask::ITERATE_ITEMS_IN_EQUIP);
    for (auto& item : found)
    {
        UnequipItem(requester, item);
    }

    return true;
}

void UnequipAction::UnequipItem(Player* requester, FindItemVisitor* visitor)
{
    IterateItemsMask mask = IterateItemsMask((uint8)IterateItemsMask::ITERATE_ITEMS_IN_BAGS | (uint8)IterateItemsMask::ITERATE_ITEMS_IN_EQUIP);
    ai->InventoryIterateItems(visitor, mask);
    std::list<Item*> items = visitor->GetResult();
	if (!items.empty()) UnequipItem(requester, *items.begin());
}

void UnequipAction::UnequipItem(Player* requester, Item* item)
{
    uint8 bagIndex = item->GetBagSlot();
    uint8 slot = item->GetSlot();
    uint8 dstBag = NULL_BAG;

    WorldPacket packet(CMSG_AUTOSTORE_BAG_ITEM, 3);
    packet << bagIndex << slot << dstBag;
    bot->GetSession()->HandleAutoStoreBagItemOpcode(packet);

    std::map<std::string, std::string> args;
    args["%item"] = chat->formatItem(item);
    ai->TellPlayer(requester, BOT_TEXT2("unequip_command", args), PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false);
}