#pragma once
#include "playerbot/strategy/Strategy.h"

namespace ai
{
    class RunawayStrategy : public Strategy
    {
    public:
        RunawayStrategy(PlayerbotAI* ai) : Strategy(ai) {}
        std::string getName() override { return "runaway"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*> &triggers) override;
    };
}
