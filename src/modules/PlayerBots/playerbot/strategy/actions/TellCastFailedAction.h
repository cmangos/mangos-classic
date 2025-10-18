#pragma once
#include "GenericActions.h"

namespace ai
{
    class TellSpellAction : public ChatCommandAction
    {
    public:
        TellSpellAction(PlayerbotAI* ai) : ChatCommandAction(ai, "spell") {}
        virtual bool Execute(Event& event) override;
        virtual bool isUsefulWhenStunned() override { return true; }
    };

    class TellCastFailedAction : public Action 
    {
    public:
        TellCastFailedAction(PlayerbotAI* ai) : Action(ai, "tell cast failed") {}

        virtual bool Execute(Event& event);
        virtual bool isUsefulWhenStunned() override { return true; }
    };
}
