#pragma once
#include "NonCombatStrategy.h"

namespace ai
{
    class StayStrategy : public Strategy
    {
    public:
        StayStrategy(PlayerbotAI* ai) : Strategy(ai) {}
        std::string getName() override { return "stay"; }

    private:
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        NextAction** GetDefaultNonCombatActions() override;
        NextAction** GetDefaultCombatActions() override;
        virtual void OnStrategyAdded(BotState state) override;
    };

    class SitStrategy : public NonCombatStrategy
    {
    public:
        SitStrategy(PlayerbotAI* ai) : NonCombatStrategy(ai) {}
        std::string getName() override { return "sit"; }

    private:
        void InitNonCombatTriggers(std::list<TriggerNode*> &triggers) override;
    };
}
