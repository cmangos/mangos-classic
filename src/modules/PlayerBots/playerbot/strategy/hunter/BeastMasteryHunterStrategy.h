#pragma once
#include "HunterStrategy.h"

namespace ai
{
    class BeastMasteryHunterPlaceholderStrategy : public SpecPlaceholderStrategy
    {
    public:
        BeastMasteryHunterPlaceholderStrategy(PlayerbotAI* ai) : SpecPlaceholderStrategy(ai) {}
        int GetType() override { return STRATEGY_TYPE_DPS | STRATEGY_TYPE_RANGED; }
        std::string getName() override { return "beast mastery"; }
    };

    class BeastMasteryHunterStrategy : public HunterStrategy
    {
    public:
        BeastMasteryHunterStrategy(PlayerbotAI* ai) : HunterStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class BeastMasteryHunterPveStrategy : public BeastMasteryHunterStrategy
    {
    public:
        BeastMasteryHunterPveStrategy(PlayerbotAI* ai) : BeastMasteryHunterStrategy(ai) {}
        std::string getName() override { return "beast mastery pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
        void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class BeastMasteryHunterPvpStrategy : public BeastMasteryHunterStrategy
    {
    public:
        BeastMasteryHunterPvpStrategy(PlayerbotAI* ai) : BeastMasteryHunterStrategy(ai) {}
        std::string getName() override { return "beast mastery pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
        void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class BeastMasteryHunterRaidStrategy : public BeastMasteryHunterStrategy
    {
    public:
        BeastMasteryHunterRaidStrategy(PlayerbotAI* ai) : BeastMasteryHunterStrategy(ai) {}
        std::string getName() override { return "beast mastery raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
        void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class BeastMasteryHunterAoeStrategy : public HunterAoeStrategy
    {
    public:
        BeastMasteryHunterAoeStrategy(PlayerbotAI* ai) : HunterAoeStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class BeastMasteryHunterAoePveStrategy : public BeastMasteryHunterAoeStrategy
    {
    public:
        BeastMasteryHunterAoePveStrategy(PlayerbotAI* ai) : BeastMasteryHunterAoeStrategy(ai) {}
        std::string getName() override { return "aoe beast mastery pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class BeastMasteryHunterAoePvpStrategy : public BeastMasteryHunterAoeStrategy
    {
    public:
        BeastMasteryHunterAoePvpStrategy(PlayerbotAI* ai) : BeastMasteryHunterAoeStrategy(ai) {}
        std::string getName() override { return "aoe beast mastery pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class BeastMasteryHunterAoeRaidStrategy : public BeastMasteryHunterAoeStrategy
    {
    public:
        BeastMasteryHunterAoeRaidStrategy(PlayerbotAI* ai) : BeastMasteryHunterAoeStrategy(ai) {}
        std::string getName() override { return "aoe beast mastery raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class BeastMasteryHunterBuffStrategy : public HunterBuffStrategy
    {
    public:
        BeastMasteryHunterBuffStrategy(PlayerbotAI* ai) : HunterBuffStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class BeastMasteryHunterBuffPveStrategy : public BeastMasteryHunterBuffStrategy
    {
    public:
        BeastMasteryHunterBuffPveStrategy(PlayerbotAI* ai) : BeastMasteryHunterBuffStrategy(ai) {}
        std::string getName() override { return "buff beast mastery pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class BeastMasteryHunterBuffPvpStrategy : public BeastMasteryHunterBuffStrategy
    {
    public:
        BeastMasteryHunterBuffPvpStrategy(PlayerbotAI* ai) : BeastMasteryHunterBuffStrategy(ai) {}
        std::string getName() override { return "buff beast mastery pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class BeastMasteryHunterBuffRaidStrategy : public BeastMasteryHunterBuffStrategy
    {
    public:
        BeastMasteryHunterBuffRaidStrategy(PlayerbotAI* ai) : BeastMasteryHunterBuffStrategy(ai) {}
        std::string getName() override { return "buff beast mastery raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class BeastMasteryHunterBoostStrategy : public HunterBoostStrategy
    {
    public:
        BeastMasteryHunterBoostStrategy(PlayerbotAI* ai) : HunterBoostStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class BeastMasteryHunterBoostPveStrategy : public BeastMasteryHunterBoostStrategy
    {
    public:
        BeastMasteryHunterBoostPveStrategy(PlayerbotAI* ai) : BeastMasteryHunterBoostStrategy(ai) {}
        std::string getName() override { return "boost beast mastery pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class BeastMasteryHunterBoostPvpStrategy : public BeastMasteryHunterBoostStrategy
    {
    public:
        BeastMasteryHunterBoostPvpStrategy(PlayerbotAI* ai) : BeastMasteryHunterBoostStrategy(ai) {}
        std::string getName() override { return "boost beast mastery pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class BeastMasteryHunterBoostRaidStrategy : public BeastMasteryHunterBoostStrategy
    {
    public:
        BeastMasteryHunterBoostRaidStrategy(PlayerbotAI* ai) : BeastMasteryHunterBoostStrategy(ai) {}
        std::string getName() override { return "boost beast mastery raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class BeastMasteryHunterCcStrategy : public HunterCcStrategy
    {
    public:
        BeastMasteryHunterCcStrategy(PlayerbotAI* ai) : HunterCcStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class BeastMasteryHunterCcPveStrategy : public BeastMasteryHunterCcStrategy
    {
    public:
        BeastMasteryHunterCcPveStrategy(PlayerbotAI* ai) : BeastMasteryHunterCcStrategy(ai) {}
        std::string getName() override { return "cc beast mastery pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class BeastMasteryHunterCcPvpStrategy : public BeastMasteryHunterCcStrategy
    {
    public:
        BeastMasteryHunterCcPvpStrategy(PlayerbotAI* ai) : BeastMasteryHunterCcStrategy(ai) {}
        std::string getName() override { return "cc beast mastery pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class BeastMasteryHunterCcRaidStrategy : public BeastMasteryHunterCcStrategy
    {
    public:
        BeastMasteryHunterCcRaidStrategy(PlayerbotAI* ai) : BeastMasteryHunterCcStrategy(ai) {}
        std::string getName() override { return "cc beast mastery raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class BeastMasteryHunterStingStrategy : public HunterStingStrategy
    {
    public:
        BeastMasteryHunterStingStrategy(PlayerbotAI* ai) : HunterStingStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class BeastMasteryHunterStingPveStrategy : public BeastMasteryHunterStingStrategy
    {
    public:
        BeastMasteryHunterStingPveStrategy(PlayerbotAI* ai) : BeastMasteryHunterStingStrategy(ai) {}
        std::string getName() override { return "sting beast mastery pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class BeastMasteryHunterStingPvpStrategy : public BeastMasteryHunterStingStrategy
    {
    public:
        BeastMasteryHunterStingPvpStrategy(PlayerbotAI* ai) : BeastMasteryHunterStingStrategy(ai) {}
        std::string getName() override { return "sting beast mastery pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class BeastMasteryHunterStingRaidStrategy : public BeastMasteryHunterStingStrategy
    {
    public:
        BeastMasteryHunterStingRaidStrategy(PlayerbotAI* ai) : BeastMasteryHunterStingStrategy(ai) {}
        std::string getName() override { return "sting beast mastery raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class BeastMasteryHunterAspectStrategy : public HunterAspectStrategy
    {
    public:
        BeastMasteryHunterAspectStrategy(PlayerbotAI* ai) : HunterAspectStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class BeastMasteryHunterAspectPveStrategy : public BeastMasteryHunterAspectStrategy
    {
    public:
        BeastMasteryHunterAspectPveStrategy(PlayerbotAI* ai) : BeastMasteryHunterAspectStrategy(ai) {}
        std::string getName() override { return "aspect beast mastery pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class BeastMasteryHunterAspectPvpStrategy : public BeastMasteryHunterAspectStrategy
    {
    public:
        BeastMasteryHunterAspectPvpStrategy(PlayerbotAI* ai) : BeastMasteryHunterAspectStrategy(ai) {}
        std::string getName() override { return "aspect beast mastery pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class BeastMasteryHunterAspectRaidStrategy : public BeastMasteryHunterAspectStrategy
    {
    public:
        BeastMasteryHunterAspectRaidStrategy(PlayerbotAI* ai) : BeastMasteryHunterAspectStrategy(ai) {}
        std::string getName() override { return "aspect beast mastery raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };
}