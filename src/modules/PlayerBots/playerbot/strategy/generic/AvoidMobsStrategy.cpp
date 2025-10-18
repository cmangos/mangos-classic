
#include "playerbot/playerbot.h"
#include "AvoidMobsStrategy.h"

using namespace ai;

void AvoidMobsStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "very often",
        NextAction::array(0, new NextAction("set avoid area", ACTION_NORMAL), NULL)));
}

void AvoidMobsStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AvoidMobsStrategy::InitNonCombatTriggers(triggers);
}
