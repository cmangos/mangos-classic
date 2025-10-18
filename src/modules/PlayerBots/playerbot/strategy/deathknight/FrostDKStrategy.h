#pragma once
#include "GenericDKStrategy.h"

namespace ai
{
    class FrostDKStrategy : public GenericDKStrategy
    {
    public:
        FrostDKStrategy(PlayerbotAI* ai);
        std::string getName() override { return "frost"; }
        int GetType() override { return STRATEGY_TYPE_COMBAT | STRATEGY_TYPE_DPS | STRATEGY_TYPE_MELEE; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*> &triggers) override;
        NextAction** GetDefaultCombatActions() override;
    };

    class FrostDKAoeStrategy : public CombatStrategy
    {
    public:
        FrostDKAoeStrategy(PlayerbotAI* ai) : CombatStrategy(ai) {}
        std::string getName() override { return "frost aoe"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*> &triggers) override;
    };
}
