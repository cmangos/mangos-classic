#pragma once
#include "playerbot/strategy/Action.h"

namespace ai
{
    class ReadyCheckAction : public Action
    {
    public:
        ReadyCheckAction(PlayerbotAI* ai, std::string name = "ready check") : Action(ai, name) {}
        virtual bool Execute(Event& event) override;

    protected:
        bool ReadyCheck(Player* requester);
    };

    class FinishReadyCheckAction : public ReadyCheckAction
    {
    public:
        FinishReadyCheckAction(PlayerbotAI* ai) : ReadyCheckAction(ai, "finish ready check") {}
        virtual bool Execute(Event& event) override;
    };
}
