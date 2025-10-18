#pragma once
#include "PassTroughStrategy.h"

namespace ai
{
    class LfgStrategy : public PassTroughStrategy
    {
    public:
        LfgStrategy(PlayerbotAI* ai) : PassTroughStrategy(ai) {}
		int GetType() override { return STRATEGY_TYPE_NONCOMBAT; }
        std::string getName() override { return "lfg"; }

    private:
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override {}
        void InitDeadTriggers(std::list<TriggerNode*>& triggers) override {}
    };
}
