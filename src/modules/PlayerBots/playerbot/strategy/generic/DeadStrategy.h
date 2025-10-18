#pragma once
#include "PassTroughStrategy.h"

namespace ai
{
    class DeadStrategy : public PassTroughStrategy
    {
    public:
        DeadStrategy(PlayerbotAI* ai) : PassTroughStrategy(ai) {}
        std::string getName() override { return "dead"; }

#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "dead"; } //Must equal iternal name
        virtual std::string GetHelpDescription() {
            return "This strategy will includes various behavior when the bot is dead.\n The main goal is to revive in a safe location.";
        }
        virtual std::vector<std::string> GetRelatedStrategies() { return { }; }
#endif
    private:
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override {}
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override {}
        void InitDeadTriggers(std::list<TriggerNode*> &triggers) override;
    };
}
