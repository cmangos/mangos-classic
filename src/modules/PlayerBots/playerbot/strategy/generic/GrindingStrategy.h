#pragma once
#include "NonCombatStrategy.h"

namespace ai
{
    class GrindingStrategy : public NonCombatStrategy
    {
    public:
        GrindingStrategy(PlayerbotAI* ai) : NonCombatStrategy(ai) {}
        std::string getName() override { return "grind"; }
        int GetType() override { return STRATEGY_TYPE_DPS; }

    private:
        void InitNonCombatTriggers(std::list<TriggerNode*> &triggers) override;
    };
}
