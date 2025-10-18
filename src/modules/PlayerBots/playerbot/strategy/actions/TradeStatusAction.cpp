
#include "playerbot/playerbot.h"
#include "TradeStatusAction.h"

#include "playerbot/strategy/ItemVisitors.h"
#include "playerbot/PlayerbotAIConfig.h"
#include "playerbot/RandomPlayerbotMgr.h"
#include "playerbot/ServerFacade.h"
#include "playerbot/strategy/values/CraftValues.h"
#include "playerbot/strategy/values/ItemUsageValue.h"
#include "SetCraftAction.h"

using namespace ai;

bool TradeStatusAction::Execute(Event& event)
{
    Player* trader = bot->GetTrader();
    Player* master = GetMaster();
    if (!trader)
        return false;

    bool shouldTrade = true;
    if (!trader->GetPlayerbotAI())
    {
        shouldTrade = false;
        if (trader == master || bot->IsInGroup(trader))
        {
            shouldTrade = ai->GetSecurity()->CheckLevelFor(PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false, trader);
        }
    }

    if (!shouldTrade)
    {
        WorldPacket p;
        uint32 status = 0;
        p << status;
        bot->GetSession()->HandleCancelTradeOpcode(p);
        return false;
    }

    WorldPacket p(event.getPacket());
    p.rpos(0);
    uint32 status;
    p >> status;

    if (status == TRADE_STATUS_TRADE_ACCEPT || (status == TRADE_STATUS_BACK_TO_TRADE && trader->GetTradeData() && trader->GetTradeData()->IsAccepted()))
    {
        WorldPacket p;
        uint32 status = 0;
        p << status;

        uint32 discount = sRandomPlayerbotMgr.GetTradeDiscount(bot, trader);
        if (CheckTrade())
        {
            int32 botMoney = CalculateCost(bot, true);

            std::map<uint32, uint32> givenItemIds, takenItemIds;
            for (uint32 slot = 0; slot < TRADE_SLOT_TRADED_COUNT; ++slot)
            {
                Item* item = trader->GetTradeData()->GetItem((TradeSlots)slot);
                if (item)
                    givenItemIds[item->GetProto()->ItemId] += item->GetCount();

                item = bot->GetTradeData()->GetItem((TradeSlots)slot);
                if (item)
                    takenItemIds[item->GetProto()->ItemId] += item->GetCount();
            }

            bot->GetSession()->HandleAcceptTradeOpcode(p);

            if (bot->GetTradeData())
            {
                sRandomPlayerbotMgr.SetTradeDiscount(bot, trader, discount);
                return false;
            }

            for (std::map<uint32, uint32>::iterator i = givenItemIds.begin(); i != givenItemIds.end(); ++i)
            {
                uint32 itemId = i->first;
                uint32 count = i->second;

                CraftData &craftData = AI_VALUE(CraftData&, "craft");
                if (!craftData.IsEmpty() && craftData.IsRequired(itemId))
                {
                    craftData.AddObtained(itemId, count);
                }
            }

            for (std::map<uint32, uint32>::iterator i = takenItemIds.begin(); i != takenItemIds.end(); ++i)
            {
                uint32 itemId = i->first;
                uint32 count = i->second;

                CraftData &craftData = AI_VALUE(CraftData&, "craft");
                if (!craftData.IsEmpty() && craftData.itemId == itemId)
                {
                    craftData.Crafted(count);
                }
            }

            return true;
        }
    }
    else if (status == TRADE_STATUS_BEGIN_TRADE)
    {
        if (!sServerFacade.IsInFront(bot, trader, sPlayerbotAIConfig.sightDistance, CAST_ANGLE_IN_FRONT))
            sServerFacade.SetFacingTo(bot, trader);

        BeginTrade();

        return true;
    }

    return false;
}

void TradeStatusAction::BeginTrade()
{
    Player* trader = bot->GetTrader();
    if (!trader || trader->GetPlayerbotAI())
        return;

    WorldPacket p;
    bot->GetSession()->HandleBeginTradeOpcode(p);

    ListItemsVisitor visitor;
    ai->InventoryIterateItems(&visitor, IterateItemsMask::ITERATE_ITEMS_IN_BAGS);

    ai->TellPlayer(trader, "=== Inventory ===");
    ai->InventoryTellItems(trader, visitor.items, visitor.soulbound);

    if (sRandomPlayerbotMgr.IsRandomBot(bot))
    {
        uint32 discount = sRandomPlayerbotMgr.GetTradeDiscount(bot, ai->GetMaster());
        if (discount)
        {
            std::ostringstream out; out << "Discount up to: " << chat->formatMoney(discount);
            ai->TellPlayer(trader, out);
        }
    }
}

bool TradeStatusAction::CheckTrade()
{
    Player* trader = bot->GetTrader();
    if (!bot->GetTradeData() || !trader || !trader->GetTradeData())
        return false;

    if (!ai->HasActivePlayerMaster() && bot->GetTrader()->GetPlayerbotAI())
    {
        bool isGivingItem = false;
        for (uint32 slot = 0; slot < TRADE_SLOT_TRADED_COUNT; ++slot)
        {
            Item* item = bot->GetTradeData()->GetItem((TradeSlots)slot);
            if (item)
            {
                isGivingItem = true;
                break;
            }
        }

        bool isGettingItem = false;
        for (uint32 slot = 0; slot < TRADE_SLOT_TRADED_COUNT; ++slot)
        {
            Item* item = trader->GetTradeData()->GetItem((TradeSlots)slot);
            if (item)
            {
                isGettingItem = true;
                break;
            }
        }
        
        if (!isGettingItem) //Enchanting an item.
            isGettingItem = (bot->GetTradeData()->GetItem(TRADE_SLOT_NONTRADED) && trader->GetTradeData()->GetSpell());

        if (isGettingItem)
        {
            std::string name = trader->GetName();
            if (bot->GetGroup() && bot->GetGroup()->IsMember(bot->GetTrader()->GetObjectGuid()) && ai->HasRealPlayerMaster())
            {
                ai->TellPlayerNoFacing(trader, "Thank you " + name + ".", PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false);
            }
            else
            {
                bot->Say("Thank you " + name + ".", (bot->GetTeam() == ALLIANCE ? LANG_COMMON : LANG_ORCISH));
            }
        }
        return isGettingItem;
    }

    if (!sRandomPlayerbotMgr.IsRandomBot(bot))
    {
        int32 botItemsMoney = CalculateCost(bot, true);
        int32 botMoney = bot->GetTradeData()->GetMoney() + botItemsMoney;
        int32 playerItemsMoney = CalculateCost(trader, false);
        int32 playerMoney = trader->GetTradeData()->GetMoney() + playerItemsMoney;
        if (playerMoney || botMoney)
        {
            ai->PlaySound(playerMoney < botMoney ? TEXTEMOTE_SIGH : TEXTEMOTE_THANK);
        }

        return true;
    }

    int32 botItemsMoney = CalculateCost(bot, true);
    int32 botMoney = bot->GetTradeData()->GetMoney() + botItemsMoney;
    int32 playerItemsMoney = CalculateCost(trader, false);
    int32 playerMoney = trader->GetTradeData()->GetMoney() + playerItemsMoney;

    for (uint32 slot = 0; slot < TRADE_SLOT_TRADED_COUNT; ++slot)
    {
        Item* item = bot->GetTradeData()->GetItem((TradeSlots)slot);
        if (item && !ItemUsageValue::GetBotSellPrice(item->GetProto(), bot))
        {
            std::ostringstream out;
            out << chat->formatItem(item) << " - This is not for sale";
            ai->TellPlayer(trader, out);
            ai->PlaySound(TEXTEMOTE_NO);
            return false;
        }

        item = trader->GetTradeData()->GetItem((TradeSlots)slot);
        if (item)
        {
            ItemUsage usage = AI_VALUE2(ItemUsage, "item usage", ItemQualifier(item).GetQualifier());
            if ((botMoney && !ItemUsageValue::GetBotBuyPrice(item->GetProto(), bot)) || usage == ItemUsage::ITEM_USAGE_NONE)
            {
                std::ostringstream out;
                out << chat->formatItem(item) << " - I don't need this";
                ai->TellPlayer(trader, out);
                ai->PlaySound(TEXTEMOTE_NO);
                return false;
            }
        }
    }

    if (!botMoney && !playerMoney)
        return true;

    if (!botItemsMoney && !playerItemsMoney)
    {
        ai->TellError(trader, "There are no items to trade");
        return false;
    }

    int32 discount = (int32)sRandomPlayerbotMgr.GetTradeDiscount(bot, trader);
    int32 delta = playerMoney - botMoney;
    int32 moneyDelta = (int32)trader->GetTradeData()->GetMoney() - (int32)bot->GetTradeData()->GetMoney();
    bool success = false;
    if (delta < 0)
    {
        if (delta + discount >= 0)
        {
            if (moneyDelta < 0)
            {
                ai->TellPlayer(trader, "You can use discount to buy items only");
                ai->PlaySound(TEXTEMOTE_NO);
                return false;
            }
            success = true;
        }
    }
    else
    {
        success = true;
    }

    if (success)
    {
        sRandomPlayerbotMgr.AddTradeDiscount(bot, trader, delta);
        switch (urand(0, 4)) {
        case 0:
            ai->TellPlayer(trader, "A pleasure doing business with you");
            break;
        case 1:
            ai->TellPlayer(trader, "Fair trade");
            break;
        case 2:
            ai->TellPlayer(trader, "Thanks");
            break;
        case 3:
            ai->TellPlayer(trader, "Off with you");
            break;
        }
        ai->PlaySound(TEXTEMOTE_THANK);
        return true;
    }

    std::ostringstream out;
    out << "I want " << chat->formatMoney(-(delta + discount)) << " for this";
    ai->TellPlayer(trader, out);
    ai->PlaySound(TEXTEMOTE_NO);
    return false;
}

int32 TradeStatusAction::CalculateCost(Player* player, bool sell)
{
    Player* trader = bot->GetTrader();
    TradeData* data = player->GetTradeData();
    if (!data)
        return 0;

    uint32 sum = 0;
    for (uint32 slot = 0; slot < TRADE_SLOT_TRADED_COUNT; ++slot)
    {
        Item* item = data->GetItem((TradeSlots)slot);
        if (!item)
            continue;

        ItemPrototype const* proto = item->GetProto();
        if (!proto)
            continue;

        if (proto->Quality < ITEM_QUALITY_NORMAL)
            return 0;

        CraftData &craftData = AI_VALUE(CraftData&, "craft");
        if (!craftData.IsEmpty())
        {
            if (player == trader && !sell && craftData.IsRequired(proto->ItemId))
            {
                continue;
            }

            if (player == bot && sell && craftData.itemId == proto->ItemId && craftData.IsFulfilled())
            {
                sum += item->GetCount() * SetCraftAction::GetCraftFee(craftData);
                continue;
            }
        }

        if (sell)
        {
            sum += item->GetCount() * ItemUsageValue::GetBotSellPrice(proto, bot);
        }
        else
        {
            sum += item->GetCount() * ItemUsageValue::GetBotBuyPrice(proto, bot);
        }
    }

    return sum;
}