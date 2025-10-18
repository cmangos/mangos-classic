#pragma once

#include "playerbot/strategy/Action.h"

namespace ai
{
    class InventoryChangeFailureAction : public Action {
    public:
        InventoryChangeFailureAction(PlayerbotAI* ai) : Action(ai, "inventory change failure") {}
        virtual bool Execute(Event& event);

    private:
        static std::map<InventoryResult, std::string> messages;
    };
}