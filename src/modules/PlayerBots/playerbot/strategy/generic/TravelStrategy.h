#pragma once
#include "playerbot/strategy/Strategy.h"

namespace ai
{
    class TravelActionMultiplier : public Multiplier
    {
    public:
        TravelActionMultiplier(PlayerbotAI* ai) : Multiplier(ai, "travel action") {}

    public:
        virtual float GetValue(Action* action);
    };

    class TravelStrategy : public Strategy
    {
    public:
        TravelStrategy(PlayerbotAI* ai) : Strategy(ai) {}
        std::string getName() override { return "travel"; }

    public:
        void InitNonCombatMultipliers(std::list<Multiplier*>& multipliers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*> &triggers) override;
        NextAction** GetDefaultNonCombatActions() override;
    };

    class TravelOnceStrategy : public Strategy
    {
    public:
        TravelOnceStrategy(PlayerbotAI* ai) : Strategy(ai) {}
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        std::string getName() override { return "travel once"; }
    };

    class ExploreStrategy : public Strategy
    {
    public:
        ExploreStrategy(PlayerbotAI* ai) : Strategy(ai) {};
        std::string getName() override { return "explore"; }
    };

    class MapStrategy : public Strategy
    {
    public:
        MapStrategy(PlayerbotAI* ai) : Strategy(ai) {};
        std::string getName() override { return "map"; }
    };

    class MapFullStrategy : public Strategy
    {
    public:
        MapFullStrategy(PlayerbotAI* ai) : Strategy(ai) {};
        std::string getName() override { return "map full"; }
    };
}
