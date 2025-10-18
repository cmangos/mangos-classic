
#include "playerbot/playerbot.h"
#include "RTSCStrategy.h"

using namespace ai;

void RTSCSJumptrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "rtsc jump active",
        NextAction::array(0, new NextAction("jump::rtsc", ACTION_MOVE), NULL)));
}
