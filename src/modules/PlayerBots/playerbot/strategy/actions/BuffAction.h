#pragma once
#include "GenericActions.h"

namespace ai
{
    class BuffAction : public ChatCommandAction
    {
    public:
        BuffAction(PlayerbotAI* ai) : ChatCommandAction(ai, "buff") {}

#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "buff"; }
        virtual std::string GetHelpDescription()
        {
            return "This command list the items that provide some kind of buff when used\n"
                   "Included are things like flags, scrolls and food.";
        }
        virtual std::vector<std::string> GetUsedActions() { return {}; }
        virtual std::vector<std::string> GetUsedValues() { return {}; }
#endif 
        virtual bool Execute(Event& event);
        virtual bool isUsefulWhenStunned() override { return true; }

    private:
        void TellHeader(uint32 subClass, Player* requester);
    };
}
