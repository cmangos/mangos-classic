
#include "playerbot/playerbot.h"
#include "playerbot/ServerFacade.h"
#include "playerbot/PlayerbotAIConfig.h"
#include "playerbot/strategy/values/LastMovementValue.h"
#include "CombatActions.h"

using namespace ai;

bool SwitchToMeleeAction::isUseful()
{
    return ai->HasStrategy("ranged", BotState::BOT_STATE_COMBAT);
}

bool SwitchToMeleeAction::Execute(Event &event)
{
    if (Unit* target = AI_VALUE(Unit*, "current target"))
    {
        bot->MeleeAttackStart(target);
        return ChangeCombatStrategyAction::Execute(event);
    }
    return false;
}

bool SwitchToRangedAction::isUseful()
{

    return ai->HasStrategy("close", BotState::BOT_STATE_COMBAT);
}

bool SwitchToRangedAction::Execute(Event &event)
{
    if (Unit* target = AI_VALUE(Unit*, "current target"))
    {
        bot->MeleeAttackStop(target);
        return ChangeCombatStrategyAction::Execute(event);
    }
    return false;
}
