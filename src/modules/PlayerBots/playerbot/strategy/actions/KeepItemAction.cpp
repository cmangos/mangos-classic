
#include "playerbot/playerbot.h"
#include "KeepItemAction.h"

#include "playerbot/strategy/values/ItemCountValue.h"
#include "playerbot/strategy/values/ItemUsageValue.h"

using namespace ai;

bool KeepItemAction::Execute(Event& event)
{
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();
    std::string text = event.getParam();

    std::string type = text.substr(0, text.find(" "));

    if (text.empty()) //No param = help feedback
    {
        std::ostringstream out;
        out << "Please specify the items that should be kept. See " << ChatHelper::formatValue("help", "action:keep", "keep help") << " for more information.";
        ai->TellPlayer(requester, out.str());
        return false;
    }
    else if (std::string("none,keep,equip,greed,need,?,").find(type + ",") == std::string::npos) //Non type = using keep.
        type = "keep";
    else if(text.find(" ") != std::string::npos) //Remove type from param.
        text = text.substr(text.find(" ") + 1);

    ItemIds ids = chat->parseItems(text);

    if (ids.empty())
    {
        IterateItemsMask mask = IterateItemsMask((uint8)IterateItemsMask::ITERATE_ITEMS_IN_EQUIP | (uint8)IterateItemsMask::ITERATE_ITEMS_IN_BAGS | (uint8)IterateItemsMask::ITERATE_ITEMS_IN_BANK);

        std::list<Item*> found = ai->InventoryParseItems(text, mask);

        for (auto& item : found)
            ids.insert(item->GetProto()->ItemId);
    }
   
    if (ids.empty())
    {
        std::ostringstream out;

        if (type == "keep")
            out << "Please specify the items that should be kept. See " << ChatHelper::formatValue("help", "action:keep", "keep help") << " for more information.";
        else
            out << "No items found.";

        ai->TellPlayer(requester, out.str());
        return true;
    }

    if (type == "?")
    {
        for (auto& id : ids)
        {
            std::ostringstream out;
            ItemQualifier qualifier(id);
            out << chat->formatItem(qualifier);
            out << ": " << keepName[AI_VALUE2_EXISTS(ForceItemUsage, "force item usage", id, ForceItemUsage::FORCE_USAGE_NONE)] << " the item.";
            ai->TellPlayer(requester, out.str());

        }
        return true;
    }

    uint32 changed = 0;

    ForceItemUsage usage = ForceItemUsage::FORCE_USAGE_NONE;
    if (type == "keep")
        usage = ForceItemUsage::FORCE_USAGE_KEEP;
    else if (type == "equip")
        usage = ForceItemUsage::FORCE_USAGE_EQUIP;
    else if (type == "greed")
        usage = ForceItemUsage::FORCE_USAGE_GREED;
    else if (type == "need")
        usage = ForceItemUsage::FORCE_USAGE_NEED;

    for (auto& id : ids)
    {
        if (AI_VALUE2_EXISTS(ForceItemUsage, "force item usage", id, ForceItemUsage::FORCE_USAGE_NONE) != usage)
        {
            changed++;
            SET_AI_VALUE2(ForceItemUsage, "force item usage", id, usage);
        }
    }

    std::ostringstream out;
    out << changed;
    out << " items changed to: " << keepName[usage] << " the item.";
    ai->TellPlayer(requester, out.str());

    if (!sPlayerbotAIConfig.bExplicitDbStoreSave)
    {
       sPlayerbotDbStore.Save(ai);
    }

    return true;
}