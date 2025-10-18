#pragma once
#include "NonCombatStrategy.h"

namespace ai
{
    class ReturnStrategy : public NonCombatStrategy
    {
    public:
        ReturnStrategy(PlayerbotAI* ai) : NonCombatStrategy(ai) {}
        std::string getName() override { return "return"; }

    private:
        virtual void InitNonCombatTriggers(std::list<TriggerNode*> &triggers) override;
    };
}
