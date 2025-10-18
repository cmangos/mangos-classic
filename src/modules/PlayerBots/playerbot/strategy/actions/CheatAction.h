#pragma once
#include "GenericActions.h"

namespace ai
{
    class CheatAction : public ChatCommandAction
    {
    public:
        CheatAction(PlayerbotAI* ai) : ChatCommandAction(ai, "cheat") {}

#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "cheat"; }
        virtual std::string GetHelpDescription()
        {
            return "This command enables/disables various cheats for testing (GM only).\n"
                   "Usage: cheat [+/-][cheatname]\n"
                   "+ to enable, - to disable\n"
                   "Example: cheat +attackspeed (enables attack speed cheat)\n"
                   "Example: cheat ? (lists active cheats)";
        }
        virtual std::vector<std::string> GetUsedActions() { return {}; }
        virtual std::vector<std::string> GetUsedValues() { return {}; }
#endif 
        virtual bool Execute(Event& event) override;
        virtual bool isUsefulWhenStunned() override { return true; }
    private:
        static BotCheatMask GetCheatMask(std::string cheat);
        static std::string GetCheatName(BotCheatMask cheatMask);
        void ListCheats(Player* requester);
        void AddCheat(BotCheatMask cheatMask);
        void RemCheat(BotCheatMask cheatMask);
    };
}
