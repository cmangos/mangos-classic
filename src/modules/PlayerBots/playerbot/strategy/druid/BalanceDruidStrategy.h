#pragma once
#include "DruidStrategy.h"

namespace ai
{
    class BalanceDruidPlaceholderStrategy : public SpecPlaceholderStrategy
    {
    public:
        BalanceDruidPlaceholderStrategy(PlayerbotAI* ai) : SpecPlaceholderStrategy(ai) {}
        int GetType() override { return STRATEGY_TYPE_DPS | STRATEGY_TYPE_RANGED; }
        std::string getName() override { return "balance"; }
    };

    class BalanceDruidStrategy : public DruidStrategy
    {
    public:
        BalanceDruidStrategy(PlayerbotAI* ai);

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
    
        virtual NextAction** GetDefaultCombatActions() override;
    };

    class BalanceDruidPveStrategy : public BalanceDruidStrategy
    {
    public:
        BalanceDruidPveStrategy(PlayerbotAI* ai) : BalanceDruidStrategy(ai) {}
        std::string getName() override { return "balance pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
        void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class BalanceDruidPvpStrategy : public BalanceDruidStrategy
    {
    public:
        BalanceDruidPvpStrategy(PlayerbotAI* ai) : BalanceDruidStrategy(ai) {}
        std::string getName() override { return "balance pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
        void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class BalanceDruidRaidStrategy : public BalanceDruidStrategy
    {
    public:
        BalanceDruidRaidStrategy(PlayerbotAI* ai) : BalanceDruidStrategy(ai) {}
        std::string getName() override { return "balance raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
        void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;

        NextAction** GetDefaultCombatActions() override;
    };

    class BalanceDruidAoeStrategy : public DruidAoeStrategy
    {
    public:
        BalanceDruidAoeStrategy(PlayerbotAI* ai) : DruidAoeStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class BalanceDruidAoePveStrategy : public BalanceDruidAoeStrategy
    {
    public:
        BalanceDruidAoePveStrategy(PlayerbotAI* ai) : BalanceDruidAoeStrategy(ai) {}
        std::string getName() override { return "aoe balance pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class BalanceDruidAoePvpStrategy : public BalanceDruidAoeStrategy
    {
    public:
        BalanceDruidAoePvpStrategy(PlayerbotAI* ai) : BalanceDruidAoeStrategy(ai) {}
        std::string getName() override { return "aoe balance pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class BalanceDruidAoeRaidStrategy : public BalanceDruidAoeStrategy
    {
    public:
        BalanceDruidAoeRaidStrategy(PlayerbotAI* ai) : BalanceDruidAoeStrategy(ai) {}
        std::string getName() override { return "aoe balance raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class BalanceDruidBuffStrategy : public DruidBuffStrategy
    {
    public:
        BalanceDruidBuffStrategy(PlayerbotAI* ai) : DruidBuffStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class BalanceDruidBuffPveStrategy : public BalanceDruidBuffStrategy
    {
    public:
        BalanceDruidBuffPveStrategy(PlayerbotAI* ai) : BalanceDruidBuffStrategy(ai) {}
        std::string getName() override { return "buff balance pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class BalanceDruidBuffPvpStrategy : public BalanceDruidBuffStrategy
    {
    public:
        BalanceDruidBuffPvpStrategy(PlayerbotAI* ai) : BalanceDruidBuffStrategy(ai) {}
        std::string getName() override { return "buff balance pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class BalanceDruidBuffRaidStrategy : public BalanceDruidBuffStrategy
    {
    public:
        BalanceDruidBuffRaidStrategy(PlayerbotAI* ai) : BalanceDruidBuffStrategy(ai) {}
        std::string getName() override { return "buff balance raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class BalanceDruidBoostStrategy : public DruidBoostStrategy
    {
    public:
        BalanceDruidBoostStrategy(PlayerbotAI* ai) : DruidBoostStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class BalanceDruidBoostPveStrategy : public BalanceDruidBoostStrategy
    {
    public:
        BalanceDruidBoostPveStrategy(PlayerbotAI* ai) : BalanceDruidBoostStrategy(ai) {}
        std::string getName() override { return "boost balance pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class BalanceDruidBoostPvpStrategy : public BalanceDruidBoostStrategy
    {
    public:
        BalanceDruidBoostPvpStrategy(PlayerbotAI* ai) : BalanceDruidBoostStrategy(ai) {}
        std::string getName() override { return "boost balance pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class BalanceDruidBoostRaidStrategy : public BalanceDruidBoostStrategy
    {
    public:
        BalanceDruidBoostRaidStrategy(PlayerbotAI* ai) : BalanceDruidBoostStrategy(ai) {}
        std::string getName() override { return "boost balance raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class BalanceDruidCcStrategy : public DruidCcStrategy
    {
    public:
        BalanceDruidCcStrategy(PlayerbotAI* ai) : DruidCcStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class BalanceDruidCcPveStrategy : public BalanceDruidCcStrategy
    {
    public:
        BalanceDruidCcPveStrategy(PlayerbotAI* ai) : BalanceDruidCcStrategy(ai) {}
        std::string getName() override { return "cc balance pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class BalanceDruidCcPvpStrategy : public BalanceDruidCcStrategy
    {
    public:
        BalanceDruidCcPvpStrategy(PlayerbotAI* ai) : BalanceDruidCcStrategy(ai) {}
        std::string getName() override { return "cc balance pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class BalanceDruidCcRaidStrategy : public BalanceDruidCcStrategy
    {
    public:
        BalanceDruidCcRaidStrategy(PlayerbotAI* ai) : BalanceDruidCcStrategy(ai) {}
        std::string getName() override { return "cc balance raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class BalanceDruidCureStrategy : public DruidCureStrategy
    {
    public:
        BalanceDruidCureStrategy(PlayerbotAI* ai) : DruidCureStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class BalanceDruidCurePveStrategy : public BalanceDruidCureStrategy
    {
    public:
        BalanceDruidCurePveStrategy(PlayerbotAI* ai) : BalanceDruidCureStrategy(ai) {}
        std::string getName() override { return "cure balance pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class BalanceDruidCurePvpStrategy : public BalanceDruidCureStrategy
    {
    public:
        BalanceDruidCurePvpStrategy(PlayerbotAI* ai) : BalanceDruidCureStrategy(ai) {}
        std::string getName() override { return "cure balance pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class BalanceDruidCureRaidStrategy : public BalanceDruidCureStrategy
    {
    public:
        BalanceDruidCureRaidStrategy(PlayerbotAI* ai) : BalanceDruidCureStrategy(ai) {}
        std::string getName() override { return "cure balance raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };
}