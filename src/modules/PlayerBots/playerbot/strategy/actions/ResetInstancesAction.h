#pragma once

#include "playerbot/strategy/Action.h"

namespace ai
{
    class ResetInstancesAction : public Action 
    {
    public:
        ResetInstancesAction(PlayerbotAI* ai) : Action(ai, "reset instances") {}
        virtual bool Execute(Event& event);
        virtual bool isUseful() { return ai->GetGroupMaster() == bot; };
        virtual bool isUsefulWhenStunned() override { return true; }
    };

    class ResetRaidsAction : public Action
    {
    public:
        ResetRaidsAction(PlayerbotAI* ai) : Action(ai, "reset raids") {}
        virtual bool Execute(Event& event);
        virtual bool isUseful() { return true; };
        virtual bool isUsefulWhenStunned() override { return true; }
    };
}
