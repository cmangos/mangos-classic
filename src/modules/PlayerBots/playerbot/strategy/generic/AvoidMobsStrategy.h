#pragma once
#include "playerbot/strategy/Strategy.h"

namespace ai
{   
    class AvoidMobsStrategy : public Strategy
    {
    public:
        AvoidMobsStrategy(PlayerbotAI* ai) : Strategy(ai) {}
        std::string getName() override { return "avoid mobs"; };
#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "avoid mobs"; } //Must equal iternal name
        virtual std::string GetHelpDescription() {
            return "This a strategy that will make bots mark mobs on the map so they try to walk around them.";
        }
        virtual std::vector<std::string> GetRelatedStrategies() { return { "flee", "flee from adds" }; }
#endif
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };
}
