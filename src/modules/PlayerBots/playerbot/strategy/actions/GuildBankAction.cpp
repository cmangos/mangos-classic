
#include "playerbot/playerbot.h"
#include "GuildBankAction.h"

#include "playerbot/strategy/values/ItemCountValue.h"
#include "Guilds/Guild.h"
#include "Guilds/GuildMgr.h"

using namespace ai;

bool GuildBankAction::Execute(Event& event)
{
#ifndef MANGOSBOT_ZERO
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();
    std::string text = event.getParam();
    if (text.empty())
        return false;

    if (!bot->GetGuildId() || (requester && requester->GetGuildId() != bot->GetGuildId()))
    {
        ai->TellPlayer(requester, "I'm not in your guild!");
            return false;
    }

    std::list<ObjectGuid> gos = *ai->GetAiObjectContext()->GetValue<std::list<ObjectGuid> >("nearest game objects no los");
    for (std::list<ObjectGuid>::iterator i = gos.begin(); i != gos.end(); ++i)
    {
        GameObject* go = ai->GetGameObject(*i);
        if (!go || !bot->GetGameObjectIfCanInteractWith(go->GetObjectGuid(), GAMEOBJECT_TYPE_GUILD_BANK))
            continue;

        return Execute(text, go, requester);
    }

    ai->TellPlayer(requester, BOT_TEXT("error_gbank_found"));
    return false;
#else
    return false;
#endif
}

bool GuildBankAction::Execute(std::string text, GameObject* bank, Player* requester)
{
    bool result = true;

    IterateItemsMask mask = IterateItemsMask((uint8)IterateItemsMask::ITERATE_ITEMS_IN_EQUIP | (uint8)IterateItemsMask::ITERATE_ITEMS_IN_BAGS);

    std::list<Item*> found = ai->InventoryParseItems(text, mask);
    if (found.empty())
        return false;

    for (std::list<Item*>::iterator i = found.begin(); i != found.end(); i++)
    {
        Item* item = *i;
        if (item)
            result &= MoveFromCharToBank(item, bank, requester);
    }

    return result;
}

bool GuildBankAction::MoveFromCharToBank(Item* item, GameObject* bank, Player* requester)
{
#ifndef MANGOSBOT_ZERO
    uint32 playerSlot = item->GetSlot();
    uint32 playerBag = item->GetBagSlot();
    std::ostringstream out;

    Guild* guild = sGuildMgr.GetGuildById(bot->GetGuildId());
    //guild->SwapItems(bot, 0, playerSlot, 0, INVENTORY_SLOT_BAG_0, 0);

    // check source pos rights (item moved to bank)
    if (!guild->IsMemberHaveRights(bot->GetGUIDLow(), 0, GUILD_BANK_RIGHT_DEPOSIT_ITEM))
        out << BOT_TEXT("error_cant_put") << chat->formatItem(item) << BOT_TEXT("error_gbank_rights");
    else
    {
        out << chat->formatItem(item) << BOT_TEXT("gbank_put");
        guild->MoveFromCharToBank(bot, playerBag, playerSlot, 0, 255, 0);
    }

    ai->TellPlayer(requester, out);
    return true;
#else
    return false;
#endif
}
