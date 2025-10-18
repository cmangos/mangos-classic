
#include "playerbot/playerbot.h"
#include "playerbot/ServerFacade.h"
#include "BotStateTriggers.h"

using namespace ai;

bool CombatStartTrigger::IsActive()
{
    if (!ai->IsStateActive(BotState::BOT_STATE_COMBAT) && !ai->IsStateActive(BotState::BOT_STATE_DEAD))
    {
        // Check if any member of the group (near this bot) is getting attacked
        return AI_VALUE(bool, "has attackers");
    }

    return false;
}

bool CombatEndTrigger::IsActive()
{
    // Check if the bot is currently in combat
    if (ai->IsStateActive(BotState::BOT_STATE_COMBAT))
    {
        // Check if any member of the group (near this bot) is getting attacked
        return !AI_VALUE(bool, "has attackers");
    }

    return false;
}

bool DeathTrigger::IsActive()
{
    return !ai->IsStateActive(BotState::BOT_STATE_DEAD) && !sServerFacade.IsAlive(bot);
}

bool ResurrectTrigger::IsActive()
{
    return ai->IsStateActive(BotState::BOT_STATE_DEAD) && sServerFacade.IsAlive(bot);
}