#pragma once
#include "playerbot/strategy/Action.h"

namespace ai
{
    class RemoveAuraAction : public Action
    {
    public:
        RemoveAuraAction(PlayerbotAI* ai, std::string aura = "") : Action(ai, aura.empty() ? "ra" : "remove" + aura), aura(aura) {}
        virtual bool Execute(Event& event);

    private:
        std::string aura;
    };
}
