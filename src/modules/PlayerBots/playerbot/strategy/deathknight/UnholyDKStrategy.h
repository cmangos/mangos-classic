#pragma once
#include "GenericDKStrategy.h"

namespace ai
{
    class UnholyDKStrategy : public GenericDKStrategy
    {
    public:
        UnholyDKStrategy(PlayerbotAI* ai) : GenericDKStrategy(ai) {}
        std::string getName() override { return "unholy"; }
        int GetType() override { return STRATEGY_TYPE_COMBAT | STRATEGY_TYPE_DPS | STRATEGY_TYPE_MELEE; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*> &triggers) override;
        NextAction** GetDefaultCombatActions() override;
        
    };

    class UnholyDKAoeStrategy : public CombatStrategy
    {
    public:
        UnholyDKAoeStrategy(PlayerbotAI* ai) : CombatStrategy(ai) {}
        std::string getName() override { return "unholy aoe"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*> &triggers) override;
    };
}
