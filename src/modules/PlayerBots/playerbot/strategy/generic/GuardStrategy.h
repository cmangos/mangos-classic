#pragma once
#include "NonCombatStrategy.h"

namespace ai
{
    class GuardStrategy : public NonCombatStrategy
    {
    public:
        GuardStrategy(PlayerbotAI* ai) : NonCombatStrategy(ai) {}
        std::string getName() override { return "guard"; }
#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "guard"; } //Must equal iternal name
        virtual std::string GetHelpDescription() {
            return "This a position strategy that will make the bot stay in a location until they have something to attack.";
        }
        virtual std::vector<std::string> GetRelatedStrategies() { return { "follow", "stay", "runaway", "flee from adds", "free" }; }
#endif
    private:
        NextAction** GetDefaultNonCombatActions() override;
        NextAction** GetDefaultCombatActions() override;
    };
}
