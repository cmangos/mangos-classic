
#include "playerbot/playerbot.h"
#include "GuildStrategy.h"

using namespace ai;

void GuildStrategy::InitNonCombatTriggers(std::list<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode(
        "random",
        NextAction::array(0, new NextAction("offer petition nearby", 4.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "random",
        NextAction::array(0, new NextAction("guild manage nearby", 4.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "petition signed",
        NextAction::array(0, new NextAction("turn in petition", 10.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "buy tabard",
        NextAction::array(0, new NextAction("buy tabard", 10.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "leave large guild",
        NextAction::array(0, new NextAction("guild leave", 4.0f), NULL)));
}
