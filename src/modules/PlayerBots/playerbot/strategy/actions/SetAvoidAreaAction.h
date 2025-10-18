#pragma once

#include "playerbot/strategy/Action.h"

namespace ai
{

    class SetAvoidAreaAction : public Action
    {
    public:
        SetAvoidAreaAction(PlayerbotAI* ai) : Action(ai, "set avoid area") {};
        virtual bool Execute(Event& event) override;
        virtual bool isUseful() override;
    };
}
