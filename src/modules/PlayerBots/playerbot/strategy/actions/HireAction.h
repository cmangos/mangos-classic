#pragma once
#include "GenericActions.h"

namespace ai
{
    class HireAction : public ChatCommandAction
    {
    public:
        HireAction(PlayerbotAI* ai) : ChatCommandAction(ai, "hire") {}
        virtual bool Execute(Event& event) override;
    };
}
