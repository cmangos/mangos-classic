
#include "playerbot/playerbot.h"
#include "MarkRtiStrategy.h"

using namespace ai;

void MarkRtiStrategy::InitCombatTriggers(std::list<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode(
        "no rti target",
        NextAction::array(0, new NextAction("mark rti", ACTION_EMERGENCY), NULL)));
}