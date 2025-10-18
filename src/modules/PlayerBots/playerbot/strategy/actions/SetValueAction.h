#pragma once
#include "GenericActions.h"

namespace ai
{
    class SetValueAction: public ChatCommandAction
    {
    public:
        SetValueAction(PlayerbotAI* ai) : ChatCommandAction(ai, "set value") {}
        virtual bool Execute(Event& event) override;
        virtual bool isUsefulWhenStunned() override { return true; }
    };
}
