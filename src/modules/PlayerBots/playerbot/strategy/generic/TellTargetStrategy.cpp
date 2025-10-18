
#include "playerbot/playerbot.h"
#include "TellTargetStrategy.h"

using namespace ai;

void TellTargetStrategy::InitCombatTriggers(std::list<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode(
        "target changed",
        NextAction::array(0, new NextAction("tell target", 51.0f), NULL)));
}
