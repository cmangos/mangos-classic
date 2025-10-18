
#include "playerbot/playerbot.h"
#include "DpsAssistStrategy.h"

using namespace ai;

void DpsAssistStrategy::InitCombatTriggers(std::list<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode(
        "not dps target active",
        NextAction::array(0, new NextAction("dps assist", 60.0f), NULL)));
}

void DpsAoeStrategy::InitCombatTriggers(std::list<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode(
        "not dps aoe target active",
        NextAction::array(0, new NextAction("dps aoe", 60.0f), NULL)));
}