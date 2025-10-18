
#include "playerbot/playerbot.h"
#include "TellItemCountAction.h"
#include "playerbot/strategy/values/ItemCountValue.h"

using namespace ai;

bool TellItemCountAction::Execute(Event& event)
{
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();
    if(requester)
    {
        std::string text = event.getParam();

        if (text.find("@") == 0)
            return false;

        if (text.empty())
            text = "inventory";

        IterateItemsMask mask = IterateItemsMask((uint8)IterateItemsMask::ITERATE_ITEMS_IN_EQUIP | (uint8)IterateItemsMask::ITERATE_ITEMS_IN_BAGS);
        if (text == "inventory")
            mask = IterateItemsMask::ITERATE_ITEMS_IN_BAGS;
        else if (text == "all")
            mask = IterateItemsMask::ITERATE_ALL_ITEMS;
        else if (text == "bank")
            mask = IterateItemsMask::ITERATE_ITEMS_IN_BANK;
        else if (text == "buyback")
            mask = IterateItemsMask::ITERATE_ITEMS_IN_BUYBACK;

        std::list<Item*> found = ai->InventoryParseItems(text, mask);
        std::map<uint32, uint32> itemMap;
        std::map<uint32, bool> soulbound;
        std::map<uint32, bool> equiped;
        bool hasEquip = false;

        for (std::list<Item*>::iterator i = found.begin(); i != found.end(); i++)
        {
            ItemPrototype const* proto = (*i)->GetProto();
            itemMap[proto->ItemId] += (*i)->GetCount();
            soulbound[proto->ItemId] = (*i)->IsSoulBound();
            equiped[proto->ItemId] = (*i)->GetBagSlot() == INVENTORY_SLOT_BAG_0 && (*i)->GetSlot() < EQUIPMENT_SLOT_END;
            if (hasEquip == false && equiped[proto->ItemId])
                ai->TellPlayer(requester, "=== Equipment ===");

            hasEquip = hasEquip || equiped[proto->ItemId];

            if ((*i)->GetBagSlot() == INVENTORY_SLOT_BAG_0 && (*i)->GetSlot() < EQUIPMENT_SLOT_END)
            {
                ItemQualifier itemQ = ItemQualifier((*i));
                std::ostringstream out;
                out << chat->formatItem(itemQ);
                if ((*i)->IsSoulBound())
                    out << " (soulbound)";

                ai->TellPlayer(requester, out.str());
            }
        }

        if (text == "equip")
            return true;

        if(!itemMap.empty())
            ai->TellPlayer(requester, "=== Inventory ===");
        for (std::map<uint32, uint32>::iterator i = itemMap.begin(); i != itemMap.end(); ++i)
        {
            if (equiped[i->first] && i->second == 1)
                continue;

            ItemPrototype const* proto = sItemStorage.LookupEntry<ItemPrototype>(i->first);
            ai->InventoryTellItem(requester, proto, i->second - equiped[i->first], soulbound[i->first]);
        }

        return true;
    }

    return false;
}
