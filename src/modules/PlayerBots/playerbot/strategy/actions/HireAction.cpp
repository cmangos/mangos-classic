
#include "playerbot/playerbot.h"
#include "HireAction.h"

using namespace ai;

bool HireAction::Execute(Event& event)
{
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();
    if (!requester)
        return false;

    if (!sRandomPlayerbotMgr.IsRandomBot(bot))
        return false;

    uint32 account = sObjectMgr.GetPlayerAccountIdByGUID(requester->GetObjectGuid());
    auto results = CharacterDatabase.PQuery("SELECT count(*) FROM characters where account = '%u'", account);

    uint32 charCount = 10;
    if (results)
    {
        Field* fields = results->Fetch();
        charCount = fields[0].GetUInt32();
    }

    if (charCount >= 10)
    {
        ai->TellPlayer(requester, "You already have the maximum number of characters");
        return false;
    }

    if ((int)bot->GetLevel() > (int)requester->GetLevel())
    {
        ai->TellPlayer(requester, "You cannot hire higher level characters than you");
        return false;
    }

    uint32 discount = sRandomPlayerbotMgr.GetTradeDiscount(bot, requester);
    uint32 m = 1 + (bot->GetLevel() / 10);
    uint32 moneyReq = m * 5000 * bot->GetLevel();
    if ((int)discount < (int)moneyReq)
    {
        std::ostringstream out;
        out << "You cannot hire me - I barely know you. Make sure you have at least " << chat->formatMoney(moneyReq) << " as a trade discount";
        ai->TellPlayer(requester, out.str());
        return false;
    }

    ai->TellPlayer(requester, "I will join you at your next relogin");

    bot->SetMoney(moneyReq);
    sRandomPlayerbotMgr.Remove(bot);
    CharacterDatabase.PExecute("update characters set account = '%u' where guid = '%u'",
            account, bot->GetGUIDLow());

    return true;
}
