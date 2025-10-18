#pragma once
#include "GenericActions.h"

namespace ai
{
    class FactionAction : public ChatCommandAction
    {
    public:
        FactionAction(PlayerbotAI* ai) : ChatCommandAction(ai, "faction") {}
        virtual bool Execute(Event& event) override;
        virtual bool isUsefulWhenStunned() override { return true; }

#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "faction"; } //Must equal iternal name
        virtual std::string GetHelpDescription()
        {
            return "This chat command gives information about a bot\'s reputation with factions.\n"
                "Examples:\n"
                "faction : List all factions and their reputation level and at war flag.\n"
                "faction [name] : List the factions matching a certain name or faction link.\n"
                "faction +atwar <name> : Flags the selected factions (name optional) as at war.\n";
                "faction -atwar <name> : Flags the selected factions (name optional) as not at war.\n";
        }
        virtual std::vector<std::string> GetUsedActions() { return {}; }
        virtual std::vector<std::string> GetUsedValues() { return {}; }
#endif 
    };
}
