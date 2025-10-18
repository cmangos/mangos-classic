
#include "playerbot/playerbot.h"
#include "LootNonCombatStrategy.h"

using namespace ai;

void LootNonCombatStrategy::InitNonCombatTriggers(std::list<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode(
        "loot available",
        NextAction::array(0, new NextAction("loot", 6.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "far from loot target",
        NextAction::array(0, new NextAction("move to loot", 7.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "can loot",
        NextAction::array(0, new NextAction("open loot", 8.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "often",
        NextAction::array(0, new NextAction("add all loot", 1.0f), NULL)));
}

void GatherStrategy::InitNonCombatTriggers(std::list<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode(
        "timer",
        NextAction::array(0, new NextAction("add gathering loot", 2.0f), NULL)));
}

void RevealStrategy::InitNonCombatTriggers(std::list<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode(
        "often",
        NextAction::array(0, new NextAction("reveal gathering item", 50.0f), NULL)));
}

void RollStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "very often",
        NextAction::array(0, new NextAction("auto loot roll", 100.0f), NULL)));
}

void RollStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RollStrategy::InitNonCombatTriggers(triggers);
}

void DelayedRollStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "loot roll",
        NextAction::array(0, new NextAction("loot roll", 100.0f), NULL)));
}

void DelayedRollStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DelayedRollStrategy::InitNonCombatTriggers(triggers);
}