#pragma once
#include "GenericActions.h"

namespace ai
{
    class TellReputationAction : public ChatCommandAction
    {
    public:
        TellReputationAction(PlayerbotAI* ai) : ChatCommandAction(ai, "reputation") {}
        virtual bool isUsefulWhenStunned() override { return true; }

    private:
        bool Execute(Event& event) override;
    };
}