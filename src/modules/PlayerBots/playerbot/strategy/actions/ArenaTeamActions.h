#pragma once
#include "playerbot/strategy/Action.h"

namespace ai
{
    class ArenaTeamAcceptAction : public Action 
    {
    public:
        ArenaTeamAcceptAction(PlayerbotAI* ai) : Action(ai, "arena team accept") {}
        virtual bool Execute(Event& event);
    };
}
