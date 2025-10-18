#include "CombatStrategy.h"
#pragma once

namespace ai
{
    class RangedCombatStrategy : public CombatStrategy
    {
    public:
        RangedCombatStrategy(PlayerbotAI* ai) : CombatStrategy(ai) {}
        int GetType() override { return STRATEGY_TYPE_COMBAT | STRATEGY_TYPE_RANGED; }
        std::string getName() override { return "ranged"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };
}
