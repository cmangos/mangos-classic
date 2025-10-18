#pragma once

#include "GenericActions.h"

namespace ai
{
    class AutoCompleteQuestAction : public ChatCommandAction
    {
    public:
        AutoCompleteQuestAction(PlayerbotAI* ai) : ChatCommandAction(ai, "auto complete quest") {}
        virtual bool Execute(Event& event) override;
        virtual bool isUseful() override;
        virtual bool isUsefulWhenStunned() override { return true; }

#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "auto complete quest"; }
        virtual std::string GetHelpDescription()
        {
            return "This action allows the bot to automatically complete certain predefined quests.\n"
                   "These quests are currently impossible for bots to complete on their own. However they are still important for the bot's progression.\n";
        }
        virtual std::vector<std::string> GetUsedActions() { return {}; }
        virtual std::vector<std::string> GetUsedValues() { return {}; }
#endif 
    };
};
