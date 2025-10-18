#pragma once

#include "playerbot/strategy/Multiplier.h"

class Action;

namespace ai
{
    class PreventMoveAwayFromCreatureOnReachToCastMultiplier : public Multiplier
    {
    public:
        PreventMoveAwayFromCreatureOnReachToCastMultiplier(PlayerbotAI* ai) : Multiplier(ai, "cast spell after reach") {}

    public:
        virtual float GetValue(Action* action);
    };
}
