#pragma once
#include "GenericActions.h"

namespace ai
{
    class ChangeChatAction : public ChatCommandAction
    {
    public:
        ChangeChatAction(PlayerbotAI* ai) : ChatCommandAction(ai, "chat") {}

#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "chat"; }
        virtual std::string GetHelpDescription()
        {
            return "This command changes the bot's chat channel settings.\n"
                   "Usage: chat [channel]\n"
                   "Available channels: party, guild, whisper, general\n"
                   "Example: chat party (switches to party chat)\n"
                   "Example: chat whisper (enables private messaging)";
        }
        virtual std::vector<std::string> GetUsedActions() { return {}; }
        virtual std::vector<std::string> GetUsedValues() { return {}; }
#endif 
        virtual bool Execute(Event& event) override;
        virtual bool isUsefulWhenStunned() override { return true; }
    };
}
