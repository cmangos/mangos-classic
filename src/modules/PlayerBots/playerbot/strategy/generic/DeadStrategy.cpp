
#include "playerbot/playerbot.h"
#include "playerbot/strategy/Strategy.h"
#include "playerbot/strategy/values/DeadValues.h"
#include "DeadStrategy.h"

using namespace ai;

void DeadStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    PassTroughStrategy::InitDeadTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "very often",
        NextAction::array(0, new NextAction("auto release", relevance), NULL)));

    triggers.push_back(new TriggerNode(
        "val::gt32::{death count," + std::to_string(DeadValueConstants::DEATH_COUNT_BEFORE_EVAC) + "}",
        NextAction::array(0, new NextAction("repop", relevance + 4.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "very often",
        NextAction::array(0, new NextAction("self resurrect", relevance + 3.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "val::should spirit healer",
        NextAction::array(0, new NextAction("spirit healer", relevance + 2.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "dead",
       NextAction::array(0, new NextAction("find corpse", relevance), NULL)));

    triggers.push_back(new TriggerNode(
        "corpse near",
        NextAction::array(0, new NextAction("revive from corpse", relevance-1.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "resurrect request",
        NextAction::array(0, new NextAction("accept resurrect", relevance), NULL)));

    triggers.push_back(new TriggerNode(
        "falling far",
        NextAction::array(0, new NextAction("repop", relevance+1), NULL)));

    triggers.push_back(new TriggerNode(
        "move long stuck",
        NextAction::array(0, new NextAction("repop", relevance+1), NULL)));
}
