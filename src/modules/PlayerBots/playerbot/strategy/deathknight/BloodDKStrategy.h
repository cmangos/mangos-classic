#pragma once
#include "GenericDKStrategy.h"

namespace ai
{
    class BloodDKStrategy : public GenericDKStrategy
    {
    public:
        BloodDKStrategy(PlayerbotAI* ai);
        std::string getName() override { return "blood"; }
        int GetType() override { return STRATEGY_TYPE_TANK | STRATEGY_TYPE_MELEE; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*> &triggers) override;
        NextAction** GetDefaultCombatActions() override;
	};
}
