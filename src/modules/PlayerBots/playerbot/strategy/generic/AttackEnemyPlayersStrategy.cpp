
#include "playerbot/playerbot.h"
#include "AttackEnemyPlayersStrategy.h"

using namespace ai;

void AttackEnemyPlayersStrategy::InitCombatTriggers(std::list<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode(
        "enemy player near",
        NextAction::array(0, new NextAction("attack enemy player", ACTION_EMERGENCY), NULL)));
}