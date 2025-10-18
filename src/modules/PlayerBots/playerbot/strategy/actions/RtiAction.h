#pragma once
#include "GenericActions.h"

namespace ai
{
    class RtiAction : public ChatCommandAction
    {
    public:
        RtiAction(PlayerbotAI* ai) : ChatCommandAction(ai, "rti") {}
        virtual bool Execute(Event& event) override;
        virtual bool isUsefulWhenStunned() override { return true; }

    private:
        void AppendRti(std::ostringstream & out, std::string type);
    };

    class MarkRtiAction : public Action
    {
    public:
        MarkRtiAction(PlayerbotAI* ai) : Action(ai, "mark rti") {}
        virtual bool Execute(Event& event);
        virtual bool isUsefulWhenStunned() override { return true; }
    };
}
