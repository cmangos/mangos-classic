#pragma once
#include "GenericActions.h"

namespace ai
{
    class CustomStrategyEditAction : public ChatCommandAction
    {
    public:
        CustomStrategyEditAction(PlayerbotAI* ai) : ChatCommandAction(ai, "cs") {}
        virtual bool Execute(Event& event) override;
        virtual bool isUsefulWhenStunned() override { return true; }

    private:
        bool PrintHelp(Player* requester);
        bool PrintActionLine(uint32 idx, std::string command, Player* requester);
        bool Print(std::string name, Player* requester);
        bool Edit(std::string name, uint32 idx, std::string command, Player* requester);
    };
}
