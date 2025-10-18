#pragma once

#include "playerbot/strategy/Action.h"
#include "QuestAction.h"

namespace ai
{
    class CheckMailAction : public Action
    {
    public:
        CheckMailAction(PlayerbotAI* ai) : Action(ai, "check mail") {}

#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "check mail"; }
        virtual std::string GetHelpDescription()
        {
            return "This action makes the bot automatically check its mailbox when not in combat or battlegrounds.\n"
                   "Any items received will be returned to sender unless they were specifically requested.\n"
                   "The bot only checks mail when it has no active master controlling it.";
        }
        virtual std::vector<std::string> GetUsedActions() { return {}; }
        virtual std::vector<std::string> GetUsedValues() { return {}; }
#endif 
        virtual bool Execute(Event& event);
        virtual bool isUseful();

    private:
        void ProcessMail(Mail* mail, Player* owner);
    };
}
