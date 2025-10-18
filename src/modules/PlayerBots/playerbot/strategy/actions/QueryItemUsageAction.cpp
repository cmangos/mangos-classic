
#include "playerbot/playerbot.h"
#include "QueryItemUsageAction.h"
#include "playerbot/strategy/values/ItemUsageValue.h"
#include "playerbot/RandomPlayerbotMgr.h"
#include "playerbot/RandomItemMgr.h"

using namespace ai;

bool QueryItemUsageAction::Execute(Event& event)
{
    Player* requester = event.getOwner();
    if (!GetMaster() && !sPlayerbotAIConfig.randomBotSayWithoutMaster && !requester)
        return false;

    WorldPacket& data = event.getPacket();
    if (!data.empty())
    {
        data.rpos(0);

        ObjectGuid guid;
        data >> guid;
        if (guid != bot->GetObjectGuid())
            return false;

        uint32 received, created, isShowChatMessage, slotId, itemId,suffixFactor, count;
        uint32 itemRandomPropertyId;
        //uint32 invCount;
        uint8 bagSlot;

        data >> received;                               // 0=looted, 1=from npc
        data >> created;                                // 0=received, 1=created
        data >> isShowChatMessage;                                      // IsShowChatMessage
        data >> bagSlot;
                                                                // item slot, but when added to stack: 0xFFFFFFFF
        data >> slotId;
        data >> itemId;
        data >> suffixFactor;
        data >> itemRandomPropertyId;
        data >> count;
        // data >> invCount; // [-ZERO] count of items in inventory

        ItemQualifier itemQualifier(itemId, (int32)itemRandomPropertyId);

        if (!itemQualifier.GetProto())
            return false;

        ai->TellPlayer(requester, QueryItem(itemQualifier, count, GetCount(itemQualifier)), PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false);

        if (sPlayerbotAIConfig.hasLog("bot_events.csv"))
        {
            if (event.getSource() == "item push result")
            {

                QuestStatusMap& questMap = bot->getQuestStatusMap();
                for (QuestStatusMap::const_iterator i = questMap.begin(); i != questMap.end(); i++)
                {
                    const Quest* questTemplate = sObjectMgr.GetQuestTemplate(i->first);
                    if (!questTemplate)
                        continue;

                    uint32 questId = questTemplate->GetQuestId();
                    QuestStatus status = bot->GetQuestStatus(questId);
                    if (status == QUEST_STATUS_INCOMPLETE || (status == QUEST_STATUS_COMPLETE && !bot->GetQuestRewardStatus(questId)))
                    {
                        QuestStatusData const& questStatus = i->second;
                        for (int i = 0; i < QUEST_OBJECTIVES_COUNT; i++)
                        {
                            if (questTemplate->ReqItemId[i] != itemId)
                                continue;

                            int required = questTemplate->ReqItemCount[i];
                            int available = questStatus.m_itemcount[i];

                            if (!required)
                                continue;

                            sPlayerbotAIConfig.logEvent(ai, "QueryItemUsageAction", questTemplate->GetTitle(), std::to_string((float)available / (float)required));
                        }
                    }
                }
            }
        }

        return true;
    }

    std::string text = event.getParam();
    std::vector<ItemQualifier> qualifiers;
    
    for (auto& stringQualifier : chat->parseItemQualifiers(text))
        qualifiers.push_back(ItemQualifier(stringQualifier));

    if (qualifiers.empty())
    {
        IterateItemsMask mask = IterateItemsMask((uint8)IterateItemsMask::ITERATE_ITEMS_IN_EQUIP | (uint8)IterateItemsMask::ITERATE_ITEMS_IN_BAGS | (uint8)IterateItemsMask::ITERATE_ITEMS_IN_BANK);

        std::list<Item*> found = ai->InventoryParseItems(text, mask);

        for (auto& item : found)
            qualifiers.push_back(item);
    }

    for (auto itemQualifier : qualifiers)
    {
        if (!itemQualifier.GetProto()) continue;

        ai->TellPlayer(requester, QueryItem(itemQualifier, 0, GetCount(itemQualifier)), PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false);
    }
    return true;
}

uint32 QueryItemUsageAction::GetCount(ItemQualifier& qualifier)
{
    IterateItemsMask mask = IterateItemsMask((uint8)IterateItemsMask::ITERATE_ITEMS_IN_EQUIP | (uint8)IterateItemsMask::ITERATE_ITEMS_IN_BAGS);

    uint32 total = 0;
    std::list<Item*> items = ai->InventoryParseItems(qualifier.GetProto()->Name1, mask);
    if (!items.empty())
    {
        for (std::list<Item*>::iterator i = items.begin(); i != items.end(); ++i)
        {
            total += (*i)->GetCount();
        }
    }
    return total;
}

std::string QueryItemUsageAction::QueryItem(ItemQualifier& qualifier, uint32 count, uint32 total)
{
    std::ostringstream out;
#ifdef CMANGOS
    std::string usage = QueryItemUsage(qualifier);
#endif
#ifdef MANGOS
    bool usage = QueryItemUsage(item);
#endif
    std::string quest = QueryQuestItem(qualifier.GetId());
    std::string price = QueryItemPrice(qualifier);
    std::string power = QueryItemPower(qualifier);
#ifdef CMANGOS
    if (usage.empty())
#endif
#ifdef MANGOS
    if (!usage)
#endif
        usage = (quest.empty() ? "Useless" : "Quest");

    out << chat->formatItem(qualifier, count, total) << ": " << usage;
    if (!quest.empty())
        out << ", " << quest;
    if (!price.empty())
        out << ", " << price;
    if (!power.empty())
        out << ", " << power;
    return out.str();
}

std::string QueryItemUsageAction::QueryItemUsage(ItemQualifier& qualifier)
{
    ItemUsage usage = AI_VALUE2(ItemUsage, "item usage", qualifier.GetQualifier());
    switch (usage)
    {
    case ItemUsage::ITEM_USAGE_EQUIP:
        return "Equip";
    case ItemUsage::ITEM_USAGE_BAD_EQUIP:
        return "Equip (temporary)";
    case ItemUsage::ITEM_USAGE_BROKEN_EQUIP:
        return "Broken Equip";
    case ItemUsage::ITEM_USAGE_QUEST:
        return "Quest (other)";
    case ItemUsage::ITEM_USAGE_SKILL:
        return "Tradeskill";
    case ItemUsage::ITEM_USAGE_USE:
        return "Use";
    case ItemUsage::ITEM_USAGE_GUILD_TASK:
        return "Guild task";
    case ItemUsage::ITEM_USAGE_DISENCHANT:
        return "Disenchant";
    case ItemUsage::ITEM_USAGE_VENDOR:
        return "Vendor";
    case ItemUsage::ITEM_USAGE_KEEP:
        return "Useful but have enough";
    case ItemUsage::ITEM_USAGE_BROKEN_AH:
        return "Auctionhouse but damaged";
    case ItemUsage::ITEM_USAGE_AH:
        return "Auctionhouse";
    case ItemUsage::ITEM_USAGE_AMMO:
        return "Ammunition";
    case ItemUsage::ITEM_USAGE_FORCE_NEED:
        return "(forced) need";
    case ItemUsage::ITEM_USAGE_FORCE_GREED:
        return "(forced) greed";
    }

    return "";
}

std::string QueryItemUsageAction::QueryItemPrice(ItemQualifier& qualifier)
{
    if (!sRandomPlayerbotMgr.IsRandomBot(bot))
        return "";

    if (qualifier.GetProto()->Bonding == BIND_WHEN_PICKED_UP)
        return "";

    std::ostringstream msg;

    IterateItemsMask mask = IterateItemsMask((uint8)IterateItemsMask::ITERATE_ITEMS_IN_EQUIP | (uint8)IterateItemsMask::ITERATE_ITEMS_IN_BAGS);

    std::list<Item*> items = ai->InventoryParseItems(qualifier.GetProto()->Name1, mask);
    int32 sellPrice = 0;
    if (!items.empty())
    {
        for (std::list<Item*>::iterator i = items.begin(); i != items.end(); ++i)
        {
            Item* sell = *i;
            sellPrice += sell->GetCount() * ItemUsageValue::GetBotSellPrice(sell->GetProto(), bot);
        }

        if(sellPrice)
            msg << "Sell: " << chat->formatMoney(sellPrice);
    }

    ItemUsage usage = AI_VALUE2(ItemUsage, "item usage", qualifier.GetQualifier());
    if (usage == ItemUsage::ITEM_USAGE_NONE)
        return msg.str();

    int32 buyPrice = ItemUsageValue::GetBotBuyPrice(qualifier.GetProto(), bot);
    if (buyPrice)
    {
        if (sellPrice) msg << " ";
        msg << "Buy: " << chat->formatMoney(buyPrice);
    }

    return msg.str();
}

std::string QueryItemUsageAction::QueryQuestItem(uint32 itemId)
{
    Player *bot = ai->GetBot();
    QuestStatusMap& questMap = bot->getQuestStatusMap();
    for (QuestStatusMap::const_iterator i = questMap.begin(); i != questMap.end(); i++)
    {
        const Quest *questTemplate = sObjectMgr.GetQuestTemplate( i->first );
        if( !questTemplate )
            continue;

        uint32 questId = questTemplate->GetQuestId();
        QuestStatus status = bot->GetQuestStatus(questId);
        if (status == QUEST_STATUS_INCOMPLETE || (status == QUEST_STATUS_COMPLETE && !bot->GetQuestRewardStatus(questId)))
        {
            QuestStatusData const& questStatus = i->second;
            std::string usage = QueryQuestItem(itemId, questTemplate, &questStatus);
            if (!usage.empty()) return usage;
        }
    }

    return "";
}


std::string QueryItemUsageAction::QueryQuestItem(uint32 itemId, const Quest *questTemplate, const QuestStatusData *questStatus)
{
    for (int i = 0; i < QUEST_OBJECTIVES_COUNT; i++)
    {
        if (questTemplate->ReqItemId[i] != itemId)
            continue;

        int required = questTemplate->ReqItemCount[i];
        int available = questStatus->m_itemcount[i];

        if (!required)
            continue;

        return chat->formatQuestObjective(chat->formatQuest(questTemplate), available, required);
    }

    return "";
}

std::string QueryItemUsageAction::QueryItemPower(ItemQualifier& qualifier)
{
    uint32 power = sRandomItemMgr.ItemStatWeight(bot, qualifier);

    ItemPrototype const* proto = ObjectMgr::GetItemPrototype(qualifier.GetId());

    if (power)
    {
        std::ostringstream out;
        char color[32];
        sprintf(color, "%x", ItemQualityColors[qualifier.GetProto()->Quality]);
        out << "power: |h|c" << color << "|h" << std::to_string(power) << "|h|cffffffff";
        return out.str().c_str();
    }

    return "";
}