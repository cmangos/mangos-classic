#pragma once
#include "playerbot/strategy/Strategy.h"

namespace ai
{
    // Generic strategy to be used for all class rotations
    class ClassStrategy : public Strategy
    {
    public:
        ClassStrategy(PlayerbotAI* ai) : Strategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*> &triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitReactionTriggers(std::list<TriggerNode *>& triggers) override;
        virtual void InitDeadTriggers(std::list<TriggerNode *>& triggers) override;
    };

    class ClassPvpStrategy
    {
    protected:
        static void InitCombatTriggers(std::list<TriggerNode*>& triggers);
        static void InitNonCombatTriggers(std::list<TriggerNode*>& triggers);
        static void InitReactionTriggers(std::list<TriggerNode*>& triggers);
        static void InitDeadTriggers(std::list<TriggerNode*>& triggers);
    };

    class ClassPveStrategy
    {
    protected:
        static void InitCombatTriggers(std::list<TriggerNode*>& triggers);
        static void InitNonCombatTriggers(std::list<TriggerNode*>& triggers);
        static void InitReactionTriggers(std::list<TriggerNode*>& triggers);
        static void InitDeadTriggers(std::list<TriggerNode*>& triggers);
    };

    class ClassRaidStrategy
    {
    protected:
        static void InitCombatTriggers(std::list<TriggerNode*>& triggers);
        static void InitNonCombatTriggers(std::list<TriggerNode*>& triggers);
        static void InitReactionTriggers(std::list<TriggerNode*>& triggers);
        static void InitDeadTriggers(std::list<TriggerNode*>& triggers);
    };

    // Generic strategy to be used for all class aoe rotations
    class AoeStrategy : public Strategy
    {
    public:
        AoeStrategy(PlayerbotAI* ai) : Strategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class AoePvpStrategy
    {
    protected:
        static void InitCombatTriggers(std::list<TriggerNode*>& triggers);
        static void InitNonCombatTriggers(std::list<TriggerNode*>& triggers);
    };

    class AoePveStrategy
    {
    protected:
        static void InitCombatTriggers(std::list<TriggerNode*>& triggers);
        static void InitNonCombatTriggers(std::list<TriggerNode*>& triggers);
    };

    class AoeRaidStrategy
    {
    protected:
        static void InitCombatTriggers(std::list<TriggerNode*>& triggers);
        static void InitNonCombatTriggers(std::list<TriggerNode*>& triggers);
    };

    // Generic strategy to be used for all class cure rotations
    class CureStrategy : public Strategy
    {
    public:
        CureStrategy(PlayerbotAI* ai) : Strategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class CurePvpStrategy
    {
    protected:
        static void InitCombatTriggers(std::list<TriggerNode*>& triggers);
        static void InitNonCombatTriggers(std::list<TriggerNode*>& triggers);
    };

    class CurePveStrategy
    {
    protected:
        static void InitCombatTriggers(std::list<TriggerNode*>& triggers);
        static void InitNonCombatTriggers(std::list<TriggerNode*>& triggers);
    };

    class CureRaidStrategy
    {
    protected:
        static void InitCombatTriggers(std::list<TriggerNode*>& triggers);
        static void InitNonCombatTriggers(std::list<TriggerNode*>& triggers);
    };

    // Generic strategy to be used for all class cc rotations
    class CcStrategy : public Strategy
    {
    public:
        CcStrategy(PlayerbotAI* ai) : Strategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class CcPvpStrategy
    {
    protected:
        static void InitCombatTriggers(std::list<TriggerNode*>& triggers);
        static void InitNonCombatTriggers(std::list<TriggerNode*>& triggers);
    };

    class CcPveStrategy
    {
    protected:
        static void InitCombatTriggers(std::list<TriggerNode*>& triggers);
        static void InitNonCombatTriggers(std::list<TriggerNode*>& triggers);
    };

    class CcRaidStrategy
    {
    protected:
        static void InitCombatTriggers(std::list<TriggerNode*>& triggers);
        static void InitNonCombatTriggers(std::list<TriggerNode*>& triggers);
    };

    // Generic strategy to be used for all class buff rotations
    class BuffStrategy : public Strategy
    {
    public:
        BuffStrategy(PlayerbotAI* ai) : Strategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class BuffPvpStrategy
    {
    protected:
        static void InitCombatTriggers(std::list<TriggerNode*>& triggers);
        static void InitNonCombatTriggers(std::list<TriggerNode*>& triggers);
    };

    class BuffPveStrategy
    {
    protected:
        static void InitCombatTriggers(std::list<TriggerNode*>& triggers);
        static void InitNonCombatTriggers(std::list<TriggerNode*>& triggers);
    };

    class BuffRaidStrategy
    {
    protected:
        static void InitCombatTriggers(std::list<TriggerNode*>& triggers);
        static void InitNonCombatTriggers(std::list<TriggerNode*>& triggers);
    };

    // Generic strategy to be used for all class boost CDs rotations 
    class BoostStrategy : public Strategy
    {
    public:
        BoostStrategy(PlayerbotAI* ai) : Strategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class BoostPvpStrategy
    {
    protected:
        static void InitCombatTriggers(std::list<TriggerNode*>& triggers);
        static void InitNonCombatTriggers(std::list<TriggerNode*>& triggers);
    };

    class BoostPveStrategy
    {
    protected:
        static void InitCombatTriggers(std::list<TriggerNode*>& triggers);
        static void InitNonCombatTriggers(std::list<TriggerNode*>& triggers);
    };

    class BoostRaidStrategy
    {
    protected:
        static void InitCombatTriggers(std::list<TriggerNode*>& triggers);
        static void InitNonCombatTriggers(std::list<TriggerNode*>& triggers);
    };

    // Generic strategy to be used for stealth classes (druid and rogue) rotations
    class StealthStrategy : public Strategy
    {
    public:
        StealthStrategy(PlayerbotAI* ai) : Strategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class StealthPvpStrategy
    {
    public:
        static void InitCombatTriggers(std::list<TriggerNode*>& triggers);
        static void InitNonCombatTriggers(std::list<TriggerNode*>& triggers);
    };

    class StealthPveStrategy
    {
    public:
        static void InitCombatTriggers(std::list<TriggerNode*>& triggers);
        static void InitNonCombatTriggers(std::list<TriggerNode*>& triggers);
    };

    class StealthRaidStrategy
    {
    public:
        static void InitCombatTriggers(std::list<TriggerNode*>& triggers);
        static void InitNonCombatTriggers(std::list<TriggerNode*>& triggers);
    };

    // Generic strategy to be used for offheal spell rotations
    class OffhealStrategy : public Strategy
    {
    public:
        OffhealStrategy(PlayerbotAI* ai) : Strategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class OffhealPvpStrategy
    {
    public:
        static void InitCombatTriggers(std::list<TriggerNode*>& triggers);
        static void InitNonCombatTriggers(std::list<TriggerNode*>& triggers);
    };

    class OffhealPveStrategy
    {
    public:
        static void InitCombatTriggers(std::list<TriggerNode*>& triggers);
        static void InitNonCombatTriggers(std::list<TriggerNode*>& triggers);
    };

    class OffhealRaidStrategy
    {
    public:
        static void InitCombatTriggers(std::list<TriggerNode*>& triggers);
        static void InitNonCombatTriggers(std::list<TriggerNode*>& triggers);
    };

    // Generic strategy to be used for offdps spell rotations
    class OffdpsStrategy : public Strategy
    {
    public:
        OffdpsStrategy(PlayerbotAI* ai) : Strategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class OffdpsPvpStrategy
    {
    public:
        static void InitCombatTriggers(std::list<TriggerNode*>& triggers);
        static void InitNonCombatTriggers(std::list<TriggerNode*>& triggers);
    };

    class OffdpsPveStrategy
    {
    public:
        static void InitCombatTriggers(std::list<TriggerNode*>& triggers);
        static void InitNonCombatTriggers(std::list<TriggerNode*>& triggers);
    };

    class OffdpsRaidStrategy
    {
    public:
        static void InitCombatTriggers(std::list<TriggerNode*>& triggers);
        static void InitNonCombatTriggers(std::list<TriggerNode*>& triggers);
    };

    // Generic strategy to be used for white-listing what can be done during stealth (druid and rogue)
    class StealthedStrategy : public Strategy
    {
    public:
        StealthedStrategy(PlayerbotAI* ai) : Strategy(ai) {}
        std::string getName() override { return "stealthed"; }
    };

    // This is a strategy to be used only as a placeholder
    class PlaceholderStrategy : public Strategy
    {
    public:
        PlaceholderStrategy(PlayerbotAI* ai) : Strategy(ai) {}
    };

    // This strategy is used to hold the bot spec strategy
    class SpecPlaceholderStrategy : public PlaceholderStrategy
    {
    public:
        SpecPlaceholderStrategy(PlayerbotAI* ai) : PlaceholderStrategy(ai) {}
        int GetType() override = 0;

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    // This strategy is used to hold the aoe strategy
    class AoePlaceholderStrategy : public PlaceholderStrategy
    {
    public:
        AoePlaceholderStrategy(PlayerbotAI* ai) : PlaceholderStrategy(ai) {}
        std::string getName() override { return "aoe"; }
    };

    // This strategy is used to hold the cure strategy
    class CurePlaceholderStrategy : public PlaceholderStrategy
    {
    public:
        CurePlaceholderStrategy(PlayerbotAI* ai) : PlaceholderStrategy(ai) {}
        std::string getName() override { return "cure"; }
    };

    // This strategy is used to hold the cc strategy
    class CcPlaceholderStrategy : public PlaceholderStrategy
    {
    public:
        CcPlaceholderStrategy(PlayerbotAI* ai) : PlaceholderStrategy(ai) {}
        std::string getName() override { return "cc"; }
    };

    // This strategy is used to hold the buff strategy
    class BuffPlaceholderStrategy : public PlaceholderStrategy
    {
    public:
        BuffPlaceholderStrategy(PlayerbotAI* ai) : PlaceholderStrategy(ai) {}
        std::string getName() override { return "buff"; }
    };

    // This strategy is used to hold the buff strategy
    class BoostPlaceholderStrategy : public PlaceholderStrategy
    {
    public:
        BoostPlaceholderStrategy(PlayerbotAI* ai) : PlaceholderStrategy(ai) {}
        std::string getName() override { return "boost"; }
    };

    // This strategy is used to hold the stealth strategy
    class StealthPlaceholderStrategy : public PlaceholderStrategy
    {
    public:
        StealthPlaceholderStrategy(PlayerbotAI* ai) : PlaceholderStrategy(ai) {}
        std::string getName() override { return "stealth"; }
    };

    // This strategy is used to hold the offheal strategy
    class OffhealPlaceholderStrategy : public PlaceholderStrategy
    {
    public:
        OffhealPlaceholderStrategy(PlayerbotAI* ai) : PlaceholderStrategy(ai) {}
        std::string getName() override { return "offheal"; }
    };

    // This strategy is used to hold the offdps strategy
    class OffdpsPlaceholderStrategy : public PlaceholderStrategy
    {
    public:
        OffdpsPlaceholderStrategy(PlayerbotAI* ai) : PlaceholderStrategy(ai) {}
        std::string getName() override { return "offdps"; }
    };
}