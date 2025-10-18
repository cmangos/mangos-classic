
#include "playerbot/playerbot.h"
#include "GroupStrategy.h"

using namespace ai;

void GroupStrategy::InitNonCombatTriggers(std::list<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode(
        "often",
        NextAction::array(0, new NextAction("invite nearby", 4.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "random",
        NextAction::array(0, new NextAction("invite guild", 4.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "seldom",
        NextAction::array(0, new NextAction("leave far away", 4.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "leader is afk",
        NextAction::array(0, new NextAction("leave far away", 4.0f), NULL)));

    /*triggers.push_back(new TriggerNode(
        "seldom",
        NextAction::array(0, new NextAction("reset instances", 1.0f), NULL)));*/
}

void GroupStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "seldom",
        NextAction::array(0, new NextAction("leave far away", 4.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "leader is afk",
        NextAction::array(0, new NextAction("leave far away", 4.0f), NULL)));
}