
#include "playerbot/playerbot.h"
#include "RunawayStrategy.h"

using namespace ai;

void RunawayStrategy::InitCombatTriggers(std::list<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode(
        "enemy too close for spell",
        NextAction::array(0, new NextAction("runaway", 50.0f), NULL)));
}
