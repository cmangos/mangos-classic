
#include "playerbot/playerbot.h"
#include "FleeStrategy.h"

using namespace ai;

void FleeStrategy::InitCombatTriggers(std::list<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode(
        "panic",
        NextAction::array(0, new NextAction("flee", ACTION_EMERGENCY + 9), NULL)));

    triggers.push_back(new TriggerNode(
        "outnumbered",
        NextAction::array(0, new NextAction("flee", ACTION_EMERGENCY + 9), NULL)));
}

void FleeFromAddsStrategy::InitCombatTriggers(std::list<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode(
        "has nearest adds",
        NextAction::array(0, new NextAction("runaway", 50.0f), NULL)));
}
