
#include "playerbot/playerbot.h"
#include "TradeValues.h"
#include "ItemUsageValue.h"
#include "playerbot/TravelMgr.h"
#include "playerbot/RandomItemMgr.h"
#include "playerbot/ServerFacade.h"

using namespace ai;

bool ItemsUsefulToGiveValue::IsTradingItem(uint32 entry)
{
    TradeData* trade = bot->GetTradeData();

    if (!trade)
        return false;

    for (uint8 i = 0; i < TRADE_SLOT_TRADED_COUNT; i++)
    {
        Item* tradeItem = trade->GetItem(TradeSlots(i));

        if (tradeItem && tradeItem->GetEntry() == entry)
            return true;;
    }

    return false;
}

std::list<Item*> ItemsUsefulToGiveValue::Calculate()
{
    GuidPosition guidP = AI_VALUE(GuidPosition, "rpg target");
    
    Player* player = guidP.GetPlayer();

    std::list<Item*> giveItems;

    if (ai->HasActivePlayerMaster() || !player->GetPlayerbotAI())
        return giveItems;

    std::list<ItemUsage> myUsages = { ItemUsage::ITEM_USAGE_NONE , ItemUsage::ITEM_USAGE_VENDOR, ItemUsage::ITEM_USAGE_AH, ItemUsage::ITEM_USAGE_BROKEN_AH, ItemUsage::ITEM_USAGE_DISENCHANT };

    for (auto& myUsage : myUsages)
    {
        std::list<Item*> myItems = AI_VALUE2(std::list<Item*>, "inventory items", "usage " + std::to_string((uint8)myUsage));
        myItems.reverse();

        for (auto& item : myItems)
        {
            if (!item->CanBeTraded())
                continue;

            // do not trade equipped items
            if (item->IsEquipped())
                continue;

            TradeData* trade = bot->GetTradeData();

            if (trade)
            {

                if (trade->HasItem(item->GetObjectGuid())) //This specific item isn't being traded.
                    continue;

                if (IsTradingItem(item->GetEntry())) //A simular item isn't being traded.
                    continue;

                if (std::any_of(giveItems.begin(), giveItems.end(), [item](Item* i) {return i->GetEntry() == item->GetEntry(); })) //We didn't already add a simular item to this list.
                    continue;
            }

            ItemUsage otherUsage = PAI_VALUE2(ItemUsage, "item usage", ItemQualifier(item).GetQualifier());

            if (std::find(myUsages.begin(), myUsages.end(), otherUsage) == myUsages.end())
                giveItems.push_back(item);
        }
    }

    return giveItems;
}

std::list<Item*> ItemsUsefulToEnchantValue::Calculate()
{
    GuidPosition guidP = AI_VALUE(GuidPosition, "rpg target");

    Player* player = guidP.GetPlayer();

    std::list<Item*> enchantItems;

    if (ai->HasActivePlayerMaster() || !player->GetPlayerbotAI())
        return enchantItems;

    std::vector<uint32> enchantSpells = AI_VALUE(std::vector<uint32>, "enchant spells");

    for (auto& spellId : enchantSpells)
    {
        const SpellEntry* pSpellInfo = sServerFacade.LookupSpellInfo(spellId);

        if (!pSpellInfo)
            continue;

        uint32 castCount = AI_VALUE2(uint32, "has reagents for", spellId);

        if (!castCount)
            continue;

        Item* item = PAI_VALUE2(Item*, "item for spell", spellId);

        if (!item)
            continue;

        if (PHAS_AI_VALUE2("force item usage", item->GetProto()->ItemId))
            continue;

        // Only trade equipped items
        if (!item->IsEquipped())
            continue;

        TradeData* trade = bot->GetTradeData();

        if (trade)
        {

            if (trade->HasItem(item->GetObjectGuid())) //This specific item isn't being traded.
                continue;

            if (std::any_of(enchantItems.begin(), enchantItems.end(), [item](Item* i) {return i->GetEntry() == item->GetEntry(); })) //We didn't already add a simular item to this list.
                continue;
        }

        uint32 enchant_id = pSpellInfo->EffectMiscValue[0];

        uint32 currentEnchnatWeight = 0;
        if (item->GetEnchantmentId(PERM_ENCHANTMENT_SLOT))
            currentEnchnatWeight = sRandomItemMgr.CalculateEnchantWeight(bot->getClass(), sRandomItemMgr.GetPlayerSpecId(bot), item->GetEnchantmentId(PERM_ENCHANTMENT_SLOT));

        uint32 newEnchantWeight = sRandomItemMgr.CalculateEnchantWeight(bot->getClass(), sRandomItemMgr.GetPlayerSpecId(bot), enchant_id);

        if (!item->GetEnchantmentId(PERM_ENCHANTMENT_SLOT) || currentEnchnatWeight >= newEnchantWeight)
            continue;

        enchantItems.push_back(item);
    }

    return enchantItems;
}