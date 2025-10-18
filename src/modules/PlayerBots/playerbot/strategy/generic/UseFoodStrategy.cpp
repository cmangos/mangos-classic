
#include "playerbot/playerbot.h"
#include "UseFoodStrategy.h"

using namespace ai;

void UseFoodStrategy::InitNonCombatTriggers(std::list<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode(
        "low health",
        NextAction::array(0, new NextAction("food", 3.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "high mana",
        NextAction::array(0, new NextAction("drink", 3.0f), NULL)));
}
