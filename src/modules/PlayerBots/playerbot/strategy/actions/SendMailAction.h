#pragma once
#include "GenericActions.h"

namespace ai
{
    class SendMailAction : public ChatCommandAction
    {
    public:
        SendMailAction(PlayerbotAI* ai) : ChatCommandAction(ai, "sendmail") {}
        virtual bool Execute(Event& event) override;
    };
}