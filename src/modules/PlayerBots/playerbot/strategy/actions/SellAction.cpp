
#include "playerbot/playerbot.h"
#include "SellAction.h"
#include "playerbot/strategy/ItemVisitors.h"
#include "playerbot/strategy/values/ItemUsageValue.h"

using namespace ai;

class SellItemsVisitor : public IterateItemsVisitor
{
public:
    SellItemsVisitor(SellAction* action) : IterateItemsVisitor()
    {
        this->action = action;
    }

    virtual bool Visit(Item* item)
    {
        action->Sell(nullptr, item);
        return true;
    }

private:
    SellAction* action;
};

bool SellAction::Execute(Event& event)
{
    static uint32 minAutoSellItems = 5;
    static uint8 minAutoSellPercentageOfBag = 20;
    static uint8 maxAutoSellPercentageOfBag = 80;


    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();

    std::string text = event.getParam();

    if (text == "*" || text.empty())
        text = "gray";

    std::list<Item*> items = ai->InventoryParseItems(text, IterateItemsMask::ITERATE_ITEMS_IN_BAGS);

    if (event.getSource() == "rpg action")
    {
        items.sort([](Item* i, Item* j) {return i->GetProto()->SellPrice * i->GetCount() < j->GetProto()->SellPrice * j->GetCount(); }); //Sell cheapest items first.
    }

    uint32 soldItems = 0;
    uint32 shouldSell = std::max(minAutoSellItems, uint32(items.size() * urand(minAutoSellPercentageOfBag, maxAutoSellPercentageOfBag) / 100));
    for (std::list<Item*>::iterator i = items.begin(); i != items.end(); ++i)
    {
        if (Sell(requester, *i))
            soldItems++;

        if (event.getSource() == "rpg action" && soldItems >= shouldSell)
            break;
    }

    return soldItems;
}

bool SellAction::Sell(Player* requester, FindItemVisitor* visitor)
{
    bool didSell = false;
    ai->InventoryIterateItems(visitor, IterateItemsMask::ITERATE_ITEMS_IN_BAGS);
    std::list<Item*> items = visitor->GetResult();
    for (std::list<Item*>::iterator i = items.begin(); i != items.end(); ++i)
    {
        didSell |= Sell(requester, *i);
    }

    return didSell;
}

bool SellAction::Sell(Player* requester, Item* item)
{
    bool didSell = false;

    std::ostringstream out;
    std::list<ObjectGuid> vendors = ai->GetAiObjectContext()->GetValue<std::list<ObjectGuid> >("nearest npcs")->Get();

    for (std::list<ObjectGuid>::iterator i = vendors.begin(); i != vendors.end(); ++i)
    {
        ObjectGuid vendorguid = *i;
        Creature *pCreature = bot->GetNPCIfCanInteractWith(vendorguid,UNIT_NPC_FLAG_VENDOR);
        if (!pCreature)
            continue;     

        if (!item->GetProto()->SellPrice)
        {
            if(ai->HasActivePlayerMaster())
                out << "Unable to sell " << chat->formatItem(item);

            continue;
        }

        ObjectGuid itemguid = item->GetObjectGuid();
        uint32 count = item->GetCount();

        uint32 botMoney = bot->GetMoney();

        sPlayerbotAIConfig.logEvent(ai, "SellAction", item->GetProto()->Name1, std::to_string(item->GetProto()->ItemId));

        WorldPacket p;
        p << vendorguid << itemguid << count;
        bot->GetSession()->HandleSellItemOpcode(p);

        if (ai->HasCheat(BotCheatMask::gold))
        {
            bot->SetMoney(botMoney);
        }

        out << "Selling " << chat->formatItem(item);
        if (sPlayerbotAIConfig.globalSoundEffects)
            bot->PlayDistanceSound(120);

        didSell = true;

        ai->TellPlayer(requester, out, PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false);
        break;
    }

    return didSell;
}