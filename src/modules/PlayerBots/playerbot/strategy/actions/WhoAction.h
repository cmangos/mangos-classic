#pragma once
#include "GenericActions.h"

namespace ai
{
    class WhoAction : public ChatCommandAction 
    {
    public:
        WhoAction(PlayerbotAI* ai) : ChatCommandAction(ai, "who") {}
        virtual bool Execute(Event& event) override;
        bool isUsefulWhenStunned() override { return true; }
    private:
        std::string QueryTrade(std::string text);
        std::string QuerySkill(std::string text);
        std::string QuerySpec(std::string text);
    };
}
