#pragma once
#include "PaladinStrategy.h"

namespace ai
{
    class RetributionPaladinPlaceholderStrategy : public SpecPlaceholderStrategy
    {
    public:
        RetributionPaladinPlaceholderStrategy(PlayerbotAI* ai) : SpecPlaceholderStrategy(ai) {}
        int GetType() override { return STRATEGY_TYPE_DPS | STRATEGY_TYPE_MELEE; }
        std::string getName() override { return "retribution"; }
    };

    class RetributionPaladinStrategy : public PaladinStrategy
    {
    public:
        RetributionPaladinStrategy(PlayerbotAI* ai);

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;

        virtual NextAction** GetDefaultCombatActions() override;
    };

    class RetributionPaladinPveStrategy : public RetributionPaladinStrategy
    {
    public:
        RetributionPaladinPveStrategy(PlayerbotAI* ai) : RetributionPaladinStrategy(ai) {}
        std::string getName() override { return "retribution pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
        void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class RetributionPaladinPvpStrategy : public RetributionPaladinStrategy
    {
    public:
        RetributionPaladinPvpStrategy(PlayerbotAI* ai) : RetributionPaladinStrategy(ai) {}
        std::string getName() override { return "retribution pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
        void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class RetributionPaladinRaidStrategy : public RetributionPaladinStrategy
    {
    public:
        RetributionPaladinRaidStrategy(PlayerbotAI* ai) : RetributionPaladinStrategy(ai) {}
        std::string getName() override { return "retribution raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
        void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class RetributionPaladinAoeStrategy : public PaladinAoeStrategy
    {
    public:
        RetributionPaladinAoeStrategy(PlayerbotAI* ai) : PaladinAoeStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class RetributionPaladinAoePveStrategy : public RetributionPaladinAoeStrategy
    {
    public:
        RetributionPaladinAoePveStrategy(PlayerbotAI* ai) : RetributionPaladinAoeStrategy(ai) {}
        std::string getName() override { return "aoe retribution pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class RetributionPaladinAoePvpStrategy : public RetributionPaladinAoeStrategy
    {
    public:
        RetributionPaladinAoePvpStrategy(PlayerbotAI* ai) : RetributionPaladinAoeStrategy(ai) {}
        std::string getName() override { return "aoe retribution pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class RetributionPaladinAoeRaidStrategy : public RetributionPaladinAoeStrategy
    {
    public:
        RetributionPaladinAoeRaidStrategy(PlayerbotAI* ai) : RetributionPaladinAoeStrategy(ai) {}
        std::string getName() override { return "aoe retribution raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class RetributionPaladinBuffStrategy : public PaladinBuffStrategy
    {
    public:
        RetributionPaladinBuffStrategy(PlayerbotAI* ai) : PaladinBuffStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class RetributionPaladinBuffPveStrategy : public RetributionPaladinBuffStrategy
    {
    public:
        RetributionPaladinBuffPveStrategy(PlayerbotAI* ai) : RetributionPaladinBuffStrategy(ai) {}
        std::string getName() override { return "buff retribution pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class RetributionPaladinBuffPvpStrategy : public RetributionPaladinBuffStrategy
    {
    public:
        RetributionPaladinBuffPvpStrategy(PlayerbotAI* ai) : RetributionPaladinBuffStrategy(ai) {}
        std::string getName() override { return "buff retribution pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class RetributionPaladinBuffRaidStrategy : public RetributionPaladinBuffStrategy
    {
    public:
        RetributionPaladinBuffRaidStrategy(PlayerbotAI* ai) : RetributionPaladinBuffStrategy(ai) {}
        std::string getName() override { return "buff retribution raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class RetributionPaladinBoostStrategy : public PaladinBoostStrategy
    {
    public:
        RetributionPaladinBoostStrategy(PlayerbotAI* ai) : PaladinBoostStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class RetributionPaladinBoostPveStrategy : public RetributionPaladinBoostStrategy
    {
    public:
        RetributionPaladinBoostPveStrategy(PlayerbotAI* ai) : RetributionPaladinBoostStrategy(ai) {}
        std::string getName() override { return "boost retribution pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class RetributionPaladinBoostPvpStrategy : public RetributionPaladinBoostStrategy
    {
    public:
        RetributionPaladinBoostPvpStrategy(PlayerbotAI* ai) : RetributionPaladinBoostStrategy(ai) {}
        std::string getName() override { return "boost retribution pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class RetributionPaladinBoostRaidStrategy : public RetributionPaladinBoostStrategy
    {
    public:
        RetributionPaladinBoostRaidStrategy(PlayerbotAI* ai) : RetributionPaladinBoostStrategy(ai) {}
        std::string getName() override { return "boost retribution raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class RetributionPaladinCcStrategy : public PaladinCcStrategy
    {
    public:
        RetributionPaladinCcStrategy(PlayerbotAI* ai) : PaladinCcStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class RetributionPaladinCcPveStrategy : public RetributionPaladinCcStrategy
    {
    public:
        RetributionPaladinCcPveStrategy(PlayerbotAI* ai) : RetributionPaladinCcStrategy(ai) {}
        std::string getName() override { return "cc retribution pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class RetributionPaladinCcPvpStrategy : public RetributionPaladinCcStrategy
    {
    public:
        RetributionPaladinCcPvpStrategy(PlayerbotAI* ai) : RetributionPaladinCcStrategy(ai) {}
        std::string getName() override { return "cc retribution pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class RetributionPaladinCcRaidStrategy : public RetributionPaladinCcStrategy
    {
    public:
        RetributionPaladinCcRaidStrategy(PlayerbotAI* ai) : RetributionPaladinCcStrategy(ai) {}
        std::string getName() override { return "cc retribution raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class RetributionPaladinCureStrategy : public PaladinCureStrategy
    {
    public:
        RetributionPaladinCureStrategy(PlayerbotAI* ai) : PaladinCureStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class RetributionPaladinCurePveStrategy : public RetributionPaladinCureStrategy
    {
    public:
        RetributionPaladinCurePveStrategy(PlayerbotAI* ai) : RetributionPaladinCureStrategy(ai) {}
        std::string getName() override { return "cure retribution pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class RetributionPaladinCurePvpStrategy : public RetributionPaladinCureStrategy
    {
    public:
        RetributionPaladinCurePvpStrategy(PlayerbotAI* ai) : RetributionPaladinCureStrategy(ai) {}
        std::string getName() override { return "cure retribution pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class RetributionPaladinCureRaidStrategy : public RetributionPaladinCureStrategy
    {
    public:
        RetributionPaladinCureRaidStrategy(PlayerbotAI* ai) : RetributionPaladinCureStrategy(ai) {}
        std::string getName() override { return "cure retribution raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class RetributionPaladinAuraStrategy : public PaladinAuraStrategy
    {
    public:
        RetributionPaladinAuraStrategy(PlayerbotAI* ai) : PaladinAuraStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class RetributionPaladinAuraPveStrategy : public RetributionPaladinAuraStrategy
    {
    public:
        RetributionPaladinAuraPveStrategy(PlayerbotAI* ai) : RetributionPaladinAuraStrategy(ai) {}
        std::string getName() override { return "aura retribution pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class RetributionPaladinAuraPvpStrategy : public RetributionPaladinAuraStrategy
    {
    public:
        RetributionPaladinAuraPvpStrategy(PlayerbotAI* ai) : RetributionPaladinAuraStrategy(ai) {}
        std::string getName() override { return "aura retribution pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class RetributionPaladinAuraRaidStrategy : public RetributionPaladinAuraStrategy
    {
    public:
        RetributionPaladinAuraRaidStrategy(PlayerbotAI* ai) : RetributionPaladinAuraStrategy(ai) {}
        std::string getName() override { return "aura retribution raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class RetributionPaladinBlessingStrategy : public PaladinBlessingStrategy
    {
    public:
        RetributionPaladinBlessingStrategy(PlayerbotAI* ai) : PaladinBlessingStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class RetributionPaladinBlessingPveStrategy : public RetributionPaladinBlessingStrategy
    {
    public:
        RetributionPaladinBlessingPveStrategy(PlayerbotAI* ai) : RetributionPaladinBlessingStrategy(ai) {}
        std::string getName() override { return "blessing retribution pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class RetributionPaladinBlessingPvpStrategy : public RetributionPaladinBlessingStrategy
    {
    public:
        RetributionPaladinBlessingPvpStrategy(PlayerbotAI* ai) : RetributionPaladinBlessingStrategy(ai) {}
        std::string getName() override { return "blessing retribution pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class RetributionPaladinBlessingRaidStrategy : public RetributionPaladinBlessingStrategy
    {
    public:
        RetributionPaladinBlessingRaidStrategy(PlayerbotAI* ai) : RetributionPaladinBlessingStrategy(ai) {}
        std::string getName() override { return "blessing retribution raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };
}
