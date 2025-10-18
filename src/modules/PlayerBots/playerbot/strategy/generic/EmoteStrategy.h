#pragma once
#include "playerbot/strategy/Strategy.h"

namespace ai
{
    class EmoteStrategy : public Strategy
    {
    public:
        EmoteStrategy(PlayerbotAI* ai) : Strategy(ai) {}
        std::string getName() override { return "emote"; }
#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "emote"; } //Must equal iternal name
        virtual std::string GetHelpDescription() {
            return "This strategy will make the bot react to or randomly send emotes.";
        }
        virtual std::vector<std::string> GetRelatedStrategies() { return { "rpg" }; }
#endif
    private:
        void InitNonCombatTriggers(std::list<TriggerNode*> &triggers) override;
    };
}
