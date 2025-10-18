#pragma once
#include "playerbot/LootObjectStack.h"
#include "GenericActions.h"

namespace ai
{
    class FlagAction : public ChatCommandAction, public Qualified
    {
    public:
        FlagAction(PlayerbotAI* ai) : ChatCommandAction(ai, "flag") {}
        virtual bool Execute(Event& event) override;
        virtual bool isUsefulWhenStunned() override { return true; }

#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "flag"; }
        virtual std::string GetHelpDescription()
        {
            return "This command allows the bot to set or manage flags.\n"
                   "Usage: flag [flag_type] [action]\n"
                   "Flag types: pvp, sheathe, cloak, helm\n"
                   "Actions: on/set, off/clear, toggle\n"
                   "Examples:\n"
                   "  flag pvp on - Sets the PVP flag\n"
                   "  flag cloak toggle - Toggles the cloak visibility\n"
                   "  flag sheathe off - Unequips weapons (sheathes)\n"
                   "  flag helm set - Shows the helm\n"
                   "  flag pvp ? - Shows current PVP state";
        }
        virtual std::vector<std::string> GetUsedActions() { return {}; }
        virtual std::vector<std::string> GetUsedValues() { return {}; }
#endif 
    };
}
