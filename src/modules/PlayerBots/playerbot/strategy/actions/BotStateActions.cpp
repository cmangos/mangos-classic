
#include "playerbot/playerbot.h"
#include "BotStateActions.h"
#include "playerbot/strategy/values/PositionValue.h"

using namespace ai;

bool SetCombatStateAction::Execute(Event& event)
{
    SetDuration(sPlayerbotAIConfig.reactDelay);
    ai->OnCombatStarted();
    return true;
}

bool SetNonCombatStateAction::Execute(Event& event)
{
    SetDuration(sPlayerbotAIConfig.reactDelay);
    if (ai->IsStateActive(BotState::BOT_STATE_DEAD))
    {
        ai->OnResurrected();
    }
    else
    {
        ai->OnCombatEnded();
    }

    return true;
}

bool SetDeadStateAction::Execute(Event& event)
{
    SetDuration(sPlayerbotAIConfig.reactDelay);
    ai->OnDeath();
    return true;
}
