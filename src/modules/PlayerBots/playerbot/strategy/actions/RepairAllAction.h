#pragma once
#include "GenericActions.h"

namespace ai
{
    class RepairAllAction : public ChatCommandAction
    {
    public:
        RepairAllAction(PlayerbotAI* ai) : ChatCommandAction(ai, "repair") {}
        virtual bool Execute(Event& event) override;
    };
}