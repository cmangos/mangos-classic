#include "playerbot/playerbot.h"
#include "DungeonMultipliers.h"
#include "playerbot/strategy/actions/DungeonActions.h"
#include "playerbot/strategy/actions/ReachTargetActions.h"

using namespace ai;

float PreventMoveAwayFromCreatureOnReachToCastMultiplier::GetValue(Action* action)
{
    MoveAwayFromCreature* moveAwayAction = dynamic_cast<MoveAwayFromCreature*>(action);
    if (moveAwayAction)
    {
        const Action* lastExecutedAction = ai->GetLastExecutedAction(BotState::BOT_STATE_COMBAT);
        if (lastExecutedAction)
        {
            const ReachTargetAction* reachAction = dynamic_cast<const ReachTargetAction*>(lastExecutedAction);
            if (reachAction && !reachAction->GetSpellName().empty())
            {
                return 0.0f;
            }
        }
    }

    return 1.0f;
}
