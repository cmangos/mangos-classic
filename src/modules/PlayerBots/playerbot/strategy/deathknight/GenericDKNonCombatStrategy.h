#pragma once
#include "playerbot/strategy/generic/NonCombatStrategy.h"

namespace ai
{
    class GenericDKNonCombatStrategy : public NonCombatStrategy
    {
    public:
        GenericDKNonCombatStrategy(PlayerbotAI* ai);
        std::string getName() override { return "nc"; }

    private:
        void InitNonCombatTriggers(std::list<TriggerNode*> &triggers) override;
    };

    class DKBuffDpsStrategy : public Strategy
    {
    public:
        DKBuffDpsStrategy(PlayerbotAI* ai) : Strategy(ai) {}
        std::string getName() override { return "bdps"; }

    private:
        void InitNonCombatTriggers(std::list<TriggerNode*> &triggers) override;
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };
}
