#pragma once
#include "playerbot/strategy/Strategy.h"

namespace ai
{
    class KarazhanDungeonStrategy : public Strategy
    {
    public:
        KarazhanDungeonStrategy(PlayerbotAI* ai) : Strategy(ai) {}
        std::string getName() override { return "karazhan"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class NetherspiteFightStrategy : public Strategy
    {
    public:
        NetherspiteFightStrategy(PlayerbotAI* ai) : Strategy(ai) {}
        std::string getName() override { return "netherspite"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class PrinceMalchezaarFightStrategy : public Strategy
    {
    public:
        PrinceMalchezaarFightStrategy(PlayerbotAI* ai) : Strategy(ai) {}
        std::string getName() override { return "prince malchezaar"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
    };
}