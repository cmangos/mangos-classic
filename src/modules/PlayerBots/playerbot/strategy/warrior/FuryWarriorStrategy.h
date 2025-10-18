#pragma once
#include "WarriorStrategy.h"

namespace ai
{
    class FuryWarriorPlaceholderStrategy : public SpecPlaceholderStrategy
    {
    public:
        FuryWarriorPlaceholderStrategy(PlayerbotAI* ai) : SpecPlaceholderStrategy(ai) {}
        int GetType() override { return STRATEGY_TYPE_COMBAT | STRATEGY_TYPE_DPS | STRATEGY_TYPE_MELEE; }
        std::string getName() override { return "fury"; }
    };

    class FuryWarriorStrategy : public WarriorStrategy
    {
    public:
        FuryWarriorStrategy(PlayerbotAI* ai);

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;

        virtual NextAction** GetDefaultCombatActions() override;
    };

    class FuryWarriorPveStrategy : public FuryWarriorStrategy
    {
    public:
        FuryWarriorPveStrategy(PlayerbotAI* ai) : FuryWarriorStrategy(ai) {}
        std::string getName() override { return "fury pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
        void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class FuryWarriorPvpStrategy : public FuryWarriorStrategy
    {
    public:
        FuryWarriorPvpStrategy(PlayerbotAI* ai) : FuryWarriorStrategy(ai) {}
        std::string getName() override { return "fury pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
        void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class FuryWarriorRaidStrategy : public FuryWarriorStrategy
    {
    public:
        FuryWarriorRaidStrategy(PlayerbotAI* ai) : FuryWarriorStrategy(ai) {}
        std::string getName() override { return "fury raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
        void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class FuryWarriorAoeStrategy : public WarriorAoeStrategy
    {
    public:
        FuryWarriorAoeStrategy(PlayerbotAI* ai) : WarriorAoeStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class FuryWarriorAoePveStrategy : public FuryWarriorAoeStrategy
    {
    public:
        FuryWarriorAoePveStrategy(PlayerbotAI* ai) : FuryWarriorAoeStrategy(ai) {}
        std::string getName() override { return "aoe fury pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class FuryWarriorAoePvpStrategy : public FuryWarriorAoeStrategy
    {
    public:
        FuryWarriorAoePvpStrategy(PlayerbotAI* ai) : FuryWarriorAoeStrategy(ai) {}
        std::string getName() override { return "aoe fury pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class FuryWarriorAoeRaidStrategy : public FuryWarriorAoeStrategy
    {
    public:
        FuryWarriorAoeRaidStrategy(PlayerbotAI* ai) : FuryWarriorAoeStrategy(ai) {}
        std::string getName() override { return "aoe fury raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class FuryWarriorBuffStrategy : public WarriorBuffStrategy
    {
    public:
        FuryWarriorBuffStrategy(PlayerbotAI* ai) : WarriorBuffStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class FuryWarriorBuffPveStrategy : public FuryWarriorBuffStrategy
    {
    public:
        FuryWarriorBuffPveStrategy(PlayerbotAI* ai) : FuryWarriorBuffStrategy(ai) {}
        std::string getName() override { return "buff fury pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class FuryWarriorBuffPvpStrategy : public FuryWarriorBuffStrategy
    {
    public:
        FuryWarriorBuffPvpStrategy(PlayerbotAI* ai) : FuryWarriorBuffStrategy(ai) {}
        std::string getName() override { return "buff fury pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class FuryWarriorBuffRaidStrategy : public FuryWarriorBuffStrategy
    {
    public:
        FuryWarriorBuffRaidStrategy(PlayerbotAI* ai) : FuryWarriorBuffStrategy(ai) {}
        std::string getName() override { return "buff fury raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class FuryWarriorBoostStrategy : public WarriorBoostStrategy
    {
    public:
        FuryWarriorBoostStrategy(PlayerbotAI* ai) : WarriorBoostStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class FuryWarriorBoostPveStrategy : public FuryWarriorBoostStrategy
    {
    public:
        FuryWarriorBoostPveStrategy(PlayerbotAI* ai) : FuryWarriorBoostStrategy(ai) {}
        std::string getName() override { return "boost fury pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class FuryWarriorBoostPvpStrategy : public FuryWarriorBoostStrategy
    {
    public:
        FuryWarriorBoostPvpStrategy(PlayerbotAI* ai) : FuryWarriorBoostStrategy(ai) {}
        std::string getName() override { return "boost fury pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class FuryWarriorBoostRaidStrategy : public FuryWarriorBoostStrategy
    {
    public:
        FuryWarriorBoostRaidStrategy(PlayerbotAI* ai) : FuryWarriorBoostStrategy(ai) {}
        std::string getName() override { return "boost fury raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class FuryWarriorCcStrategy : public WarriorCcStrategy
    {
    public:
        FuryWarriorCcStrategy(PlayerbotAI* ai) : WarriorCcStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class FuryWarriorCcPveStrategy : public FuryWarriorCcStrategy
    {
    public:
        FuryWarriorCcPveStrategy(PlayerbotAI* ai) : FuryWarriorCcStrategy(ai) {}
        std::string getName() override { return "cc fury pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class FuryWarriorCcPvpStrategy : public FuryWarriorCcStrategy
    {
    public:
        FuryWarriorCcPvpStrategy(PlayerbotAI* ai) : FuryWarriorCcStrategy(ai) {}
        std::string getName() override { return "cc fury pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class FuryWarriorCcRaidStrategy : public FuryWarriorCcStrategy
    {
    public:
        FuryWarriorCcRaidStrategy(PlayerbotAI* ai) : FuryWarriorCcStrategy(ai) {}
        std::string getName() override { return "cc fury raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };
}