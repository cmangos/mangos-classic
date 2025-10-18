#pragma once
#include "GenericActions.h"

namespace ai
{
    class WtsAction : public ChatCommandAction 
    {
    public:
        WtsAction(PlayerbotAI* ai) : ChatCommandAction(ai, "wts") {}
        virtual bool Execute(Event& event) override;
        virtual bool isUsefulWhenStunned() override { return true; }
    };
}
