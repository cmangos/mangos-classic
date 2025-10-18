#pragma once

#include "playerbot/strategy/Action.h"

namespace ai
{
    class SecurityCheckAction : public Action
    {
    public:
        SecurityCheckAction(PlayerbotAI* ai) : Action(ai, "security check") {}
        virtual bool isUseful();
        virtual bool Execute(Event& event) override;
        virtual bool isUsefulWhenStunned() override { return true; }
    };
}
