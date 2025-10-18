
#include "playerbot/playerbot.h"
#include "KiteStrategy.h"

using namespace ai;

void KiteStrategy::InitCombatTriggers(std::list<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode(
        "has aggro",
        NextAction::array(0, new NextAction("runaway", 51.0f), NULL)));
}
