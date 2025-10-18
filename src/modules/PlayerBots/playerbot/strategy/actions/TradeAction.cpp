
#include "playerbot/playerbot.h"
#include "TradeAction.h"
#include "playerbot/strategy/ItemVisitors.h"
#include "playerbot/strategy/values/ItemCountValue.h"

using namespace ai;

bool TradeAction::Execute(Event& event)
{
    std::string text = event.getParam();

    if (!bot->GetTrader())
    {
        std::list<ObjectGuid> guids = chat->parseGameobjects(text);
        Player* player = nullptr;

        for(auto& guid: guids)
        {
            if (guid.IsPlayer())
            {
                player = sObjectMgr.GetPlayer(guid);
            }
        }

        if (!player)
        {
            player = event.getOwner() ? event.getOwner() : GetMaster();
        }

        if (!player) 
        {
            return false;
        }

        if (!player->GetTrader())
        {
            WorldPacket packet(CMSG_INITIATE_TRADE);
            packet << player->GetObjectGuid();
            bot->GetSession()->HandleInitiateTradeOpcode(packet);
            return true;
        }
        else if (player->GetTrader() != bot)
        {
            return false;
        }
    }
    
    uint32 copper = chat->parseMoney(text);
    if (copper > 0)
    {
        WorldPacket packet(CMSG_SET_TRADE_GOLD, 4);
        packet << copper;
        bot->GetSession()->HandleSetTradeGoldOpcode(packet);
    }

    size_t pos = text.rfind(" ");
    int count = pos!= std::string::npos ? atoi(text.substr(pos + 1).c_str()) : 1;

    IterateItemsMask mask = IterateItemsMask((uint8)IterateItemsMask::ITERATE_ITEMS_IN_EQUIP | (uint8)IterateItemsMask::ITERATE_ITEMS_IN_BAGS);
    std::list<Item*> found = ai->InventoryParseItems(text, mask);
    if (found.empty())
        return false;

    int traded = 0;
    for (std::list<Item*>::iterator i = found.begin(); i != found.end(); i++)
    {
        Item* item = *i;

        if (!bot->GetTrader() || item->IsInTrade())
            continue;

        int8 slot = item->CanBeTraded() ? -1 : TRADE_SLOT_NONTRADED;
        if (TradeItem(*item, slot) && slot != TRADE_SLOT_NONTRADED && ++traded >= count)
            break;
    }

    return true;
}

bool TradeAction::TradeItem(const Item& item, int8 slot)
{
    int8 tradeSlot = -1;
    Item* itemPtr = const_cast<Item*>(&item);

    TradeData* pTrade = bot->GetTradeData();
    if ((slot >= 0 && slot < TRADE_SLOT_COUNT) && pTrade->GetItem(TradeSlots(slot)) == NULL)
        tradeSlot = slot;

    if (slot == TRADE_SLOT_NONTRADED)
        pTrade->SetItem(TRADE_SLOT_NONTRADED, itemPtr);
    else
    {
        for (uint8 i = 0; i < TRADE_SLOT_TRADED_COUNT && tradeSlot == -1; i++)
        {
            if (pTrade->GetItem(TradeSlots(i)) == itemPtr)
            {
                tradeSlot = i;

                WorldPacket packet(CMSG_CLEAR_TRADE_ITEM, 1);
                packet << (uint8) tradeSlot;
                bot->GetSession()->HandleClearTradeItemOpcode(packet);
                pTrade->SetItem(TradeSlots(i), NULL);
                return true;
            }
        }

        for (uint8 i = 0; i < TRADE_SLOT_TRADED_COUNT && tradeSlot == -1; i++)
        {
            if (pTrade->GetItem(TradeSlots(i)) == NULL)
            {
                tradeSlot = i;
            }
        }
    }

    if (tradeSlot == -1) return false;

    WorldPacket packet(CMSG_SET_TRADE_ITEM, 3);
    packet << (uint8) tradeSlot << (uint8) item.GetBagSlot()
        << (uint8) item.GetSlot();
    bot->GetSession()->HandleSetTradeItemOpcode(packet);
    return true;
}

