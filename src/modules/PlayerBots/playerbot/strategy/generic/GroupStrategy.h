#pragma once
#include "NonCombatStrategy.h"

namespace ai
{
    class GroupStrategy : public NonCombatStrategy
    {
    public:
        GroupStrategy(PlayerbotAI* ai) : NonCombatStrategy(ai) {}
        std::string getName() override { return "group"; }
        int GetType() override { return STRATEGY_TYPE_GENERIC; }

    private:
        void InitNonCombatTriggers(std::list<TriggerNode*> &triggers) override;
        void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
    };
}
