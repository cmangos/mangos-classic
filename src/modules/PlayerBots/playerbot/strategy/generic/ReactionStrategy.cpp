
#include "playerbot/playerbot.h"
#include "playerbot/ServerFacade.h"
#include "ReactionStrategy.h"

using namespace ai;

void ReactionStrategy::InitReactionTriggers(std::list<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode(
        "combat start",
        NextAction::array(0, new NextAction("set combat state", ACTION_PASSTROUGH + 10), NULL)));

    triggers.push_back(new TriggerNode(
        "combat end",
        NextAction::array(0, new NextAction("set non combat state", ACTION_PASSTROUGH + 10), NULL)));

    triggers.push_back(new TriggerNode(
        "death",
        NextAction::array(0, new NextAction("set dead state", ACTION_PASSTROUGH + 10), NULL)));

    triggers.push_back(new TriggerNode(
        "resurrect",
        NextAction::array(0, new NextAction("set non combat state", ACTION_PASSTROUGH + 10), NULL)));
}