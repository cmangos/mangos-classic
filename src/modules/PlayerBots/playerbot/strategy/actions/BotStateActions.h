#pragma once

#include "playerbot/strategy/Action.h"

namespace ai
{
    class SetCombatStateAction : public Action 
    {
    public:
        SetCombatStateAction(PlayerbotAI* ai, std::string name = "set combat state") : Action(ai, name) {}
        bool Execute(Event& event) override;
    };

    class SetNonCombatStateAction : public Action
    {
    public:
        SetNonCombatStateAction(PlayerbotAI* ai, std::string name = "set non combat state") : Action(ai, name) {}
        bool Execute(Event& event) override;
    };

    class SetDeadStateAction : public Action
    {
    public:
        SetDeadStateAction(PlayerbotAI* ai, std::string name = "set dead state") : Action(ai, name) {}
        bool Execute(Event& event) override;
    };
}
