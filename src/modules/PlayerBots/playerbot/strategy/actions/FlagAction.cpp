
#include "playerbot/playerbot.h"
#include "FlagAction.h"
#include "playerbot/strategy/values/LootStrategyValue.h"
#include "LootAction.h"

using namespace ai;

bool FlagAction::Execute(Event& event)
{
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();
    std::string cmd = event.getParam();
    if (!qualifier.empty())
        cmd = qualifier;
    std::vector<std::string> ss = split(cmd, ' ');
    if (ss.size() != 2)
    {
        ai->TellError(requester, "Usage: flag cloak/helm/pvp/sheathe on/set/off/clear/toggle/?");
        return false;
    }

    bool setFlag = (ss[1] == "set" || ss[1] == "on");
    bool clearFlag = (ss[1] == "clear" || ss[1] == "off");
    bool toggleFlag = (ss[1] == "toggle");
    if (ss[0] == "pvp")
    {
        if (setFlag) bot->SetPvP(true);
        else if (clearFlag) bot->SetPvP(false);
        else if (toggleFlag) bot->SetPvP(!bot->IsPvP());
        std::ostringstream out; out << ss[0] << " flag is " << chat->formatBoolean(bot->IsPvP());
        ai->TellPlayer(requester, out.str());
        return true;
    }

    if (ss[0] == "sheathe")
    {
        SheathState currentSheathe = bot->GetSheath();
        if (setFlag || (toggleFlag && currentSheathe == SHEATH_STATE_UNARMED))
        {
            if (bot->getClass() == CLASS_HUNTER)
                bot->SetSheath(SHEATH_STATE_RANGED);
            else
                bot->SetSheath(SHEATH_STATE_MELEE);
        }
        else if (clearFlag || (toggleFlag && currentSheathe != SHEATH_STATE_UNARMED))
            bot->SetSheath(SHEATH_STATE_UNARMED);
        else if (ss[1] == "ranged" && bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_RANGED))
            bot->SetSheath(SHEATH_STATE_RANGED);

        return true;
    }

    uint32 playerFlags;
    if (ss[0] == "cloak") playerFlags = PLAYER_FLAGS_HIDE_CLOAK;
    if (ss[0] == "helm") playerFlags = PLAYER_FLAGS_HIDE_HELM;

    if (clearFlag) bot->SetFlag(PLAYER_FLAGS, playerFlags);
    else if (setFlag) bot->RemoveFlag(PLAYER_FLAGS, playerFlags);
    else if (toggleFlag && bot->HasFlag(PLAYER_FLAGS, playerFlags)) bot->RemoveFlag(PLAYER_FLAGS, playerFlags);
    else if (toggleFlag && !bot->HasFlag(PLAYER_FLAGS, playerFlags)) bot->SetFlag(PLAYER_FLAGS, playerFlags);
    std::ostringstream out; out << ss[0] << " flag is " << chat->formatBoolean(!bot->HasFlag(PLAYER_FLAGS, playerFlags));
    ai->TellPlayer(requester, out.str());
    return true;
}

