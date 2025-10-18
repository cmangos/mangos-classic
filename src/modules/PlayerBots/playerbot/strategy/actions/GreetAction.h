#pragma once

#include "playerbot/strategy/Action.h"
#include "QuestAction.h"
#include "UseItemAction.h"

namespace ai
{
    class GreetAction : public Action
    {
    public:
        GreetAction(PlayerbotAI* ai);
        virtual bool Execute(Event& event);
    };
}
