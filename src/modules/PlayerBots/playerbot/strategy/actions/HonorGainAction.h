#pragma once

#include "playerbot/strategy/Action.h"

namespace ai
{
    class HonorGainAction : public Action
    {
    public:
        HonorGainAction(PlayerbotAI* ai) : Action(ai, "honor gain") {}
        virtual bool Execute(Event& event);
    };
}