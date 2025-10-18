#pragma once
#include "PaladinStrategy.h"

namespace ai
{
    class ProtectionPaladinPlaceholderStrategy : public SpecPlaceholderStrategy
    {
    public:
        ProtectionPaladinPlaceholderStrategy(PlayerbotAI* ai) : SpecPlaceholderStrategy(ai) {}
        int GetType() override { return STRATEGY_TYPE_TANK | STRATEGY_TYPE_MELEE; }
        std::string getName() override { return "protection"; }
    };

    class ProtectionPaladinStrategy : public PaladinStrategy
    {
    public:
        ProtectionPaladinStrategy(PlayerbotAI* ai);

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;

        virtual NextAction** GetDefaultCombatActions() override;
    };

    class ProtectionPaladinPveStrategy : public ProtectionPaladinStrategy
    {
    public:
        ProtectionPaladinPveStrategy(PlayerbotAI* ai) : ProtectionPaladinStrategy(ai) {}
        std::string getName() override { return "protection pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
        void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class ProtectionPaladinPvpStrategy : public ProtectionPaladinStrategy
    {
    public:
        ProtectionPaladinPvpStrategy(PlayerbotAI* ai) : ProtectionPaladinStrategy(ai) {}
        std::string getName() override { return "protection pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
        void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class ProtectionPaladinRaidStrategy : public ProtectionPaladinStrategy
    {
    public:
        ProtectionPaladinRaidStrategy(PlayerbotAI* ai) : ProtectionPaladinStrategy(ai) {}
        std::string getName() override { return "protection raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
        void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class ProtectionPaladinAoeStrategy : public PaladinAoeStrategy
    {
    public:
        ProtectionPaladinAoeStrategy(PlayerbotAI* ai) : PaladinAoeStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class ProtectionPaladinAoePveStrategy : public ProtectionPaladinAoeStrategy
    {
    public:
        ProtectionPaladinAoePveStrategy(PlayerbotAI* ai) : ProtectionPaladinAoeStrategy(ai) {}
        std::string getName() override { return "aoe protection pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class ProtectionPaladinAoePvpStrategy : public ProtectionPaladinAoeStrategy
    {
    public:
        ProtectionPaladinAoePvpStrategy(PlayerbotAI* ai) : ProtectionPaladinAoeStrategy(ai) {}
        std::string getName() override { return "aoe protection pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class ProtectionPaladinAoeRaidStrategy : public ProtectionPaladinAoeStrategy
    {
    public:
        ProtectionPaladinAoeRaidStrategy(PlayerbotAI* ai) : ProtectionPaladinAoeStrategy(ai) {}
        std::string getName() override { return "aoe protection raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class ProtectionPaladinBuffStrategy : public PaladinBuffStrategy
    {
    public:
        ProtectionPaladinBuffStrategy(PlayerbotAI* ai) : PaladinBuffStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class ProtectionPaladinBuffPveStrategy : public ProtectionPaladinBuffStrategy
    {
    public:
        ProtectionPaladinBuffPveStrategy(PlayerbotAI* ai) : ProtectionPaladinBuffStrategy(ai) {}
        std::string getName() override { return "buff protection pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class ProtectionPaladinBuffPvpStrategy : public ProtectionPaladinBuffStrategy
    {
    public:
        ProtectionPaladinBuffPvpStrategy(PlayerbotAI* ai) : ProtectionPaladinBuffStrategy(ai) {}
        std::string getName() override { return "buff protection pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class ProtectionPaladinBuffRaidStrategy : public ProtectionPaladinBuffStrategy
    {
    public:
        ProtectionPaladinBuffRaidStrategy(PlayerbotAI* ai) : ProtectionPaladinBuffStrategy(ai) {}
        std::string getName() override { return "buff protection raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class ProtectionPaladinBoostStrategy : public PaladinBoostStrategy
    {
    public:
        ProtectionPaladinBoostStrategy(PlayerbotAI* ai) : PaladinBoostStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class ProtectionPaladinBoostPveStrategy : public ProtectionPaladinBoostStrategy
    {
    public:
        ProtectionPaladinBoostPveStrategy(PlayerbotAI* ai) : ProtectionPaladinBoostStrategy(ai) {}
        std::string getName() override { return "boost protection pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class ProtectionPaladinBoostPvpStrategy : public ProtectionPaladinBoostStrategy
    {
    public:
        ProtectionPaladinBoostPvpStrategy(PlayerbotAI* ai) : ProtectionPaladinBoostStrategy(ai) {}
        std::string getName() override { return "boost protection pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class ProtectionPaladinBoostRaidStrategy : public ProtectionPaladinBoostStrategy
    {
    public:
        ProtectionPaladinBoostRaidStrategy(PlayerbotAI* ai) : ProtectionPaladinBoostStrategy(ai) {}
        std::string getName() override { return "boost protection raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class ProtectionPaladinCcStrategy : public PaladinCcStrategy
    {
    public:
        ProtectionPaladinCcStrategy(PlayerbotAI* ai) : PaladinCcStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class ProtectionPaladinCcPveStrategy : public ProtectionPaladinCcStrategy
    {
    public:
        ProtectionPaladinCcPveStrategy(PlayerbotAI* ai) : ProtectionPaladinCcStrategy(ai) {}
        std::string getName() override { return "cc protection pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class ProtectionPaladinCcPvpStrategy : public ProtectionPaladinCcStrategy
    {
    public:
        ProtectionPaladinCcPvpStrategy(PlayerbotAI* ai) : ProtectionPaladinCcStrategy(ai) {}
        std::string getName() override { return "cc protection pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class ProtectionPaladinCcRaidStrategy : public ProtectionPaladinCcStrategy
    {
    public:
        ProtectionPaladinCcRaidStrategy(PlayerbotAI* ai) : ProtectionPaladinCcStrategy(ai) {}
        std::string getName() override { return "cc protection raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class ProtectionPaladinCureStrategy : public PaladinCureStrategy
    {
    public:
        ProtectionPaladinCureStrategy(PlayerbotAI* ai) : PaladinCureStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class ProtectionPaladinCurePveStrategy : public ProtectionPaladinCureStrategy
    {
    public:
        ProtectionPaladinCurePveStrategy(PlayerbotAI* ai) : ProtectionPaladinCureStrategy(ai) {}
        std::string getName() override { return "cure protection pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class ProtectionPaladinCurePvpStrategy : public ProtectionPaladinCureStrategy
    {
    public:
        ProtectionPaladinCurePvpStrategy(PlayerbotAI* ai) : ProtectionPaladinCureStrategy(ai) {}
        std::string getName() override { return "cure protection pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class ProtectionPaladinCureRaidStrategy : public ProtectionPaladinCureStrategy
    {
    public:
        ProtectionPaladinCureRaidStrategy(PlayerbotAI* ai) : ProtectionPaladinCureStrategy(ai) {}
        std::string getName() override { return "cure protection raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class ProtectionPaladinAuraStrategy : public PaladinAuraStrategy
    {
    public:
        ProtectionPaladinAuraStrategy(PlayerbotAI* ai) : PaladinAuraStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class ProtectionPaladinAuraPveStrategy : public ProtectionPaladinAuraStrategy
    {
    public:
        ProtectionPaladinAuraPveStrategy(PlayerbotAI* ai) : ProtectionPaladinAuraStrategy(ai) {}
        std::string getName() override { return "aura protection pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class ProtectionPaladinAuraPvpStrategy : public ProtectionPaladinAuraStrategy
    {
    public:
        ProtectionPaladinAuraPvpStrategy(PlayerbotAI* ai) : ProtectionPaladinAuraStrategy(ai) {}
        std::string getName() override { return "aura protection pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class ProtectionPaladinAuraRaidStrategy : public ProtectionPaladinAuraStrategy
    {
    public:
        ProtectionPaladinAuraRaidStrategy(PlayerbotAI* ai) : ProtectionPaladinAuraStrategy(ai) {}
        std::string getName() override { return "aura protection raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class ProtectionPaladinBlessingStrategy : public PaladinBlessingStrategy
    {
    public:
        ProtectionPaladinBlessingStrategy(PlayerbotAI* ai) : PaladinBlessingStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class ProtectionPaladinBlessingPveStrategy : public ProtectionPaladinBlessingStrategy
    {
    public:
        ProtectionPaladinBlessingPveStrategy(PlayerbotAI* ai) : ProtectionPaladinBlessingStrategy(ai) {}
        std::string getName() override { return "blessing protection pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class ProtectionPaladinBlessingPvpStrategy : public ProtectionPaladinBlessingStrategy
    {
    public:
        ProtectionPaladinBlessingPvpStrategy(PlayerbotAI* ai) : ProtectionPaladinBlessingStrategy(ai) {}
        std::string getName() override { return "blessing protection pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class ProtectionPaladinBlessingRaidStrategy : public ProtectionPaladinBlessingStrategy
    {
    public:
        ProtectionPaladinBlessingRaidStrategy(PlayerbotAI* ai) : ProtectionPaladinBlessingStrategy(ai) {}
        std::string getName() override { return "blessing protection raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };
}