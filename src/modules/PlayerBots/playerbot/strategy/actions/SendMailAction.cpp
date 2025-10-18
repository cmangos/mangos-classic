
#include "Mails/Mail.h"
#include "playerbot/playerbot.h"
#include "SendMailAction.h"

#include "playerbot/PlayerbotAIConfig.h"
#include "playerbot/strategy/ItemVisitors.h"

using namespace ai;

bool SendMailAction::Execute(Event& event)
{
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();
    uint32 account = sObjectMgr.GetPlayerAccountIdByGUID(bot->GetObjectGuid());
    bool randomBot = sPlayerbotAIConfig.IsInRandomAccountList(account);

    std::string text = event.getParam();
    Player* receiver = requester;
    Player* tellTo = requester;
    std::vector<std::string> ss = split(text, ' ');
    if (ss.size() > 1)
    {
        Player* p = sObjectMgr.GetPlayer(ss[ss.size() - 1].c_str());
        if (p) receiver = p;
    }

    if (!receiver) receiver = event.getOwner();

    if (!receiver || receiver == bot)
    {
        return false;
    }

    if (!tellTo) tellTo = receiver;

    ItemIds ids = chat->parseItems(text);
    if (ids.size() > 1)
    {
        bot->Whisper("You can not request more than one item", LANG_UNIVERSAL, tellTo->GetObjectGuid());
        return false;
    }

    if (ids.empty())
    {
        uint32 money = chat->parseMoney(text);
        if (!money)
            return false;

        if (randomBot)
        {
            bot->Whisper("I cannot send money", LANG_UNIVERSAL, tellTo->GetObjectGuid());
            return false;
        }

        if (bot->GetMoney() < money)
        {
            ai->TellError(requester, "I don't have enough money");
            return false;
        }

        std::ostringstream body;
        body << "Hello, " << receiver->GetName() << ",\n";
        body << "\n";
        body << "Here is the money you asked for";
        body << "\n";
        body << "Thanks,\n";
        body << bot->GetName() << "\n";


        MailDraft draft("Money you asked for", body.str());
        draft.SetMoney(money);
        bot->SetMoney(bot->GetMoney() - money);
        draft.SendMailTo(MailReceiver(receiver), MailSender(bot));

        std::ostringstream out; out << "Sending mail to " << receiver->GetName();
        ai->TellPlayer(requester, out.str());
        return true;
    }

    std::ostringstream body;
    body << "Hello, " << receiver->GetName() << ",\n";
    body << "\n";
    body << "Here are the item(s) you asked for";
    body << "\n";
    body << "Thanks,\n";
    body << bot->GetName() << "\n";

    MailDraft draft("Item(s) you asked for", body.str());
    for (ItemIds::iterator i =ids.begin(); i != ids.end(); i++)
    {
        FindItemByIdVisitor visitor(*i);
        IterateItemsMask mask = IterateItemsMask((uint8)IterateItemsMask::ITERATE_ITEMS_IN_BAGS | (uint8)IterateItemsMask::ITERATE_ITEMS_IN_EQUIP | (uint8)IterateItemsMask::ITERATE_ITEMS_IN_BANK);
        ai->InventoryIterateItems(&visitor, mask);
        std::list<Item*> items = visitor.GetResult();
        for (std::list<Item*>::iterator i = items.begin(); i != items.end(); ++i)
        {
            Item* item = *i;
            if (item->IsSoulBound() || item->IsConjuredConsumable())
            {
                std::ostringstream out;
                out << "Cannot send " << ChatHelper::formatItem(item);
                bot->Whisper(out.str(), LANG_UNIVERSAL, tellTo->GetObjectGuid());
                continue;
            }

            ItemPrototype const *proto = item->GetProto();
            bot->MoveItemFromInventory(item->GetBagSlot(), item->GetSlot(), true);
            item->DeleteFromInventoryDB();
            item->SetOwnerGuid(receiver->GetObjectGuid());
            item->SaveToDB();
            draft.AddItem(item);
            if (randomBot)
            {
                uint32 price = item->GetCount() * ItemUsageValue::GetBotSellPrice(proto, bot);
                if (!price)
                {
                    std::ostringstream out;
                    out << ChatHelper::formatItem(item) << ": it is not for sale";
                    bot->Whisper(out.str(), LANG_UNIVERSAL, tellTo->GetObjectGuid());
                    return false;
                }
                draft.SetCOD(price);
            }
            draft.SendMailTo(MailReceiver(receiver), MailSender(bot));

            std::ostringstream out; out << "Sent mail to " << receiver->GetName();
            bot->Whisper(out.str(), LANG_UNIVERSAL, tellTo->GetObjectGuid());
            return true;
        }
    }

    return false;
}
