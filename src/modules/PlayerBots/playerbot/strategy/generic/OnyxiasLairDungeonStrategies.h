#pragma once
#include "playerbot/strategy/Strategy.h"

namespace ai
{
    class OnyxiasLairDungeonStrategy : public Strategy
    {
    public:
        OnyxiasLairDungeonStrategy(PlayerbotAI* ai) : Strategy(ai) {}
        std::string getName() override { return "onyxia's lair"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class OnyxiaFightStrategy : public Strategy
    {
    public:
        OnyxiaFightStrategy(PlayerbotAI* ai) : Strategy(ai) {}
        std::string getName() override { return "onyxia"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
        void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
    };
}