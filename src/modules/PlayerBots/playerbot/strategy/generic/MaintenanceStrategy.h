#pragma once
#include "NonCombatStrategy.h"

namespace ai
{
    class MaintenanceStrategy : public NonCombatStrategy
    {
    public:
        MaintenanceStrategy(PlayerbotAI* ai) : NonCombatStrategy(ai) {}
        std::string getName() override { return "maintenance"; }
        int GetType() override { return STRATEGY_TYPE_NONCOMBAT; }

    private:
        void InitNonCombatTriggers(std::list<TriggerNode*> &triggers) override;
    };
}
