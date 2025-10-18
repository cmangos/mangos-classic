
#include "playerbot/playerbot.h"
#include "WtsAction.h"
#include "playerbot/AiFactory.h"
#include "playerbot/strategy/ItemVisitors.h"
#include "playerbot/RandomPlayerbotMgr.h"
#include "playerbot/strategy/values/ItemUsageValue.h"

using namespace ai;

bool WtsAction::Execute(Event& event)
{
    Player* owner = event.getOwner();
    if (!owner)
        return false;

    std::ostringstream out;
    std::string text = event.getParam();

    if (!sRandomPlayerbotMgr.IsRandomBot(bot))
        return false;

    std::string link = event.getParam();

    ItemIds itemIds = chat->parseItems(link);
    if (itemIds.empty())
        return false;

    for (ItemIds::iterator i = itemIds.begin(); i != itemIds.end(); i++)
    {
        uint32 itemId = *i;
        const ItemPrototype* proto = sObjectMgr.GetItemPrototype(itemId);
        if (!proto)
            continue;

        std::ostringstream out; out << itemId;
        ItemUsage usage = AI_VALUE2(ItemUsage, "item usage", out.str());
        if (usage == ItemUsage::ITEM_USAGE_NONE)
            continue;

        int32 buyPrice = ItemUsageValue::GetBotBuyPrice(proto, bot);
        if (!buyPrice)
            continue;

        if (urand(0, 15) > 2)
            continue;

        std::ostringstream tell;
        tell << "I'll buy " << chat->formatItem(proto) << " for " << chat->formatMoney(buyPrice);

        // ignore random bot chat filter
        bot->Whisper(tell.str(), LANG_UNIVERSAL, owner->GetObjectGuid());
    }

    return true;
}
