
#include "EngineValues.h"
#include "playerbot/strategy/AiObjectContext.h"
#include "playerbot/PlayerbotAI.h"

using namespace ai;

bool ActionPossibleValue::Calculate()
{
    Action* action = context->GetAction(getQualifier());

    if (!action)
        return false;

    return action->isPossible();
}

bool ActionUsefulValue::Calculate()
{
    Action* action = context->GetAction(getQualifier());

    if (!action)
        return false;

    return action->isUseful();
}

bool TriggerActiveValue::Calculate()
{
    Trigger* trigger = context->GetTrigger(getQualifier());

    if (!trigger)
        return false;

    if (!trigger->Check() || !trigger->IsActive())
        return false;

    return true;
}

bool HasStrategyValue::Calculate()
{
    std::vector<std::string> tokens = Qualified::getMultiQualifiers(getQualifier(), ",");

    BotState state = BotState::BOT_STATE_NON_COMBAT;

    if (tokens.size() == 2)
    {
        if (tokens[0] == "co")
            state = BotState::BOT_STATE_COMBAT;
        if (tokens[0] == "de")
            state = BotState::BOT_STATE_DEAD;
        if (tokens[0] == "react")
            state = BotState::BOT_STATE_REACTION;
    }

    return ai->HasStrategy(tokens.back(), state);
}

