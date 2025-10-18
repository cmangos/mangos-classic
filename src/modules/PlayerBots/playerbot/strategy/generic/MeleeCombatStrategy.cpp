
#include "playerbot/playerbot.h"
#include "MeleeCombatStrategy.h"

using namespace ai;

void MeleeCombatStrategy::InitCombatTriggers(std::list<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode(
        "enemy out of melee",
        NextAction::array(0, new NextAction("reach melee", ACTION_MOVE), NULL)));

    triggers.push_back(new TriggerNode(
        "enemy too close for melee",
        NextAction::array(0, new NextAction("move out of enemy contact", ACTION_NORMAL + 8.0f), NULL)));
}

void SetBehindCombatStrategy::InitCombatTriggers(std::list<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode(
        "not behind target",
        NextAction::array(0, new NextAction("set behind", ACTION_HIGH), NULL)));
}

void ChaseJumpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "very often",
        NextAction::array(0, new NextAction("jump::chase", ACTION_MOVE + 9.0f), NULL)));
}

void ChaseJumpStrategy::InitCombatTriggers(std::list<TriggerNode *> &triggers)
{
    InitNonCombatTriggers(triggers);
}

void ChaseJumpStrategy::InitReactionTriggers(std::list<TriggerNode *> &triggers)
{
    InitNonCombatTriggers(triggers);
}
