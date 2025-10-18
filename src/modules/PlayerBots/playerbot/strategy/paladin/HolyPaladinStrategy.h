#pragma once
#include "PaladinStrategy.h"

namespace ai
{
    class HolyPaladinPlaceholderStrategy : public SpecPlaceholderStrategy
    {
    public:
        HolyPaladinPlaceholderStrategy(PlayerbotAI* ai) : SpecPlaceholderStrategy(ai) {}
        int GetType() override { return STRATEGY_TYPE_HEAL | STRATEGY_TYPE_RANGED; }
        std::string getName() override { return "holy"; }
    };

    class HolyPaladinStrategy : public PaladinStrategy
    {
    public:
        HolyPaladinStrategy(PlayerbotAI* ai) : PaladinStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class HolyPaladinPveStrategy : public HolyPaladinStrategy
    {
    public:
        HolyPaladinPveStrategy(PlayerbotAI* ai) : HolyPaladinStrategy(ai) {}
        std::string getName() override { return "holy pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
        void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class HolyPaladinPvpStrategy : public HolyPaladinStrategy
    {
    public:
        HolyPaladinPvpStrategy(PlayerbotAI* ai) : HolyPaladinStrategy(ai) {}
        std::string getName() override { return "holy pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
        void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class HolyPaladinRaidStrategy : public HolyPaladinStrategy
    {
    public:
        HolyPaladinRaidStrategy(PlayerbotAI* ai) : HolyPaladinStrategy(ai) {}
        std::string getName() override { return "holy raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
        void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class HolyPaladinAoeStrategy : public PaladinAoeStrategy
    {
    public:
        HolyPaladinAoeStrategy(PlayerbotAI* ai) : PaladinAoeStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class HolyPaladinAoePveStrategy : public HolyPaladinAoeStrategy
    {
    public:
        HolyPaladinAoePveStrategy(PlayerbotAI* ai) : HolyPaladinAoeStrategy(ai) {}
        std::string getName() override { return "aoe holy pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class HolyPaladinAoePvpStrategy : public HolyPaladinAoeStrategy
    {
    public:
        HolyPaladinAoePvpStrategy(PlayerbotAI* ai) : HolyPaladinAoeStrategy(ai) {}
        std::string getName() override { return "aoe holy pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class HolyPaladinAoeRaidStrategy : public HolyPaladinAoeStrategy
    {
    public:
        HolyPaladinAoeRaidStrategy(PlayerbotAI* ai) : HolyPaladinAoeStrategy(ai) {}
        std::string getName() override { return "aoe holy raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class HolyPaladinBuffStrategy : public PaladinBuffStrategy
    {
    public:
        HolyPaladinBuffStrategy(PlayerbotAI* ai) : PaladinBuffStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class HolyPaladinBuffPveStrategy : public HolyPaladinBuffStrategy
    {
    public:
        HolyPaladinBuffPveStrategy(PlayerbotAI* ai) : HolyPaladinBuffStrategy(ai) {}
        std::string getName() override { return "buff holy pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class HolyPaladinBuffPvpStrategy : public HolyPaladinBuffStrategy
    {
    public:
        HolyPaladinBuffPvpStrategy(PlayerbotAI* ai) : HolyPaladinBuffStrategy(ai) {}
        std::string getName() override { return "buff holy pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class HolyPaladinBuffRaidStrategy : public HolyPaladinBuffStrategy
    {
    public:
        HolyPaladinBuffRaidStrategy(PlayerbotAI* ai) : HolyPaladinBuffStrategy(ai) {}
        std::string getName() override { return "buff holy raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class HolyPaladinBoostStrategy : public PaladinBoostStrategy
    {
    public:
        HolyPaladinBoostStrategy(PlayerbotAI* ai) : PaladinBoostStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class HolyPaladinBoostPveStrategy : public HolyPaladinBoostStrategy
    {
    public:
        HolyPaladinBoostPveStrategy(PlayerbotAI* ai) : HolyPaladinBoostStrategy(ai) {}
        std::string getName() override { return "boost holy pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class HolyPaladinBoostPvpStrategy : public HolyPaladinBoostStrategy
    {
    public:
        HolyPaladinBoostPvpStrategy(PlayerbotAI* ai) : HolyPaladinBoostStrategy(ai) {}
        std::string getName() override { return "boost holy pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class HolyPaladinBoostRaidStrategy : public HolyPaladinBoostStrategy
    {
    public:
        HolyPaladinBoostRaidStrategy(PlayerbotAI* ai) : HolyPaladinBoostStrategy(ai) {}
        std::string getName() override { return "boost holy raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class HolyPaladinCcStrategy : public PaladinCcStrategy
    {
    public:
        HolyPaladinCcStrategy(PlayerbotAI* ai) : PaladinCcStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class HolyPaladinCcPveStrategy : public HolyPaladinCcStrategy
    {
    public:
        HolyPaladinCcPveStrategy(PlayerbotAI* ai) : HolyPaladinCcStrategy(ai) {}
        std::string getName() override { return "cc holy pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class HolyPaladinCcPvpStrategy : public HolyPaladinCcStrategy
    {
    public:
        HolyPaladinCcPvpStrategy(PlayerbotAI* ai) : HolyPaladinCcStrategy(ai) {}
        std::string getName() override { return "cc holy pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class HolyPaladinCcRaidStrategy : public HolyPaladinCcStrategy
    {
    public:
        HolyPaladinCcRaidStrategy(PlayerbotAI* ai) : HolyPaladinCcStrategy(ai) {}
        std::string getName() override { return "cc holy raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class HolyPaladinCureStrategy : public PaladinCureStrategy
    {
    public:
        HolyPaladinCureStrategy(PlayerbotAI* ai) : PaladinCureStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class HolyPaladinCurePveStrategy : public HolyPaladinCureStrategy
    {
    public:
        HolyPaladinCurePveStrategy(PlayerbotAI* ai) : HolyPaladinCureStrategy(ai) {}
        std::string getName() override { return "cure holy pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class HolyPaladinCurePvpStrategy : public HolyPaladinCureStrategy
    {
    public:
        HolyPaladinCurePvpStrategy(PlayerbotAI* ai) : HolyPaladinCureStrategy(ai) {}
        std::string getName() override { return "cure holy pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class HolyPaladinCureRaidStrategy : public HolyPaladinCureStrategy
    {
    public:
        HolyPaladinCureRaidStrategy(PlayerbotAI* ai) : HolyPaladinCureStrategy(ai) {}
        std::string getName() override { return "cure holy raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class HolyPaladinAuraStrategy : public PaladinAuraStrategy
    {
    public:
        HolyPaladinAuraStrategy(PlayerbotAI* ai) : PaladinAuraStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class HolyPaladinAuraPveStrategy : public HolyPaladinAuraStrategy
    {
    public:
        HolyPaladinAuraPveStrategy(PlayerbotAI* ai) : HolyPaladinAuraStrategy(ai) {}
        std::string getName() override { return "aura holy pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class HolyPaladinAuraPvpStrategy : public HolyPaladinAuraStrategy
    {
    public:
        HolyPaladinAuraPvpStrategy(PlayerbotAI* ai) : HolyPaladinAuraStrategy(ai) {}
        std::string getName() override { return "aura holy pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class HolyPaladinAuraRaidStrategy : public HolyPaladinAuraStrategy
    {
    public:
        HolyPaladinAuraRaidStrategy(PlayerbotAI* ai) : HolyPaladinAuraStrategy(ai) {}
        std::string getName() override { return "aura holy raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class HolyPaladinBlessingStrategy : public PaladinBlessingStrategy
    {
    public:
        HolyPaladinBlessingStrategy(PlayerbotAI* ai) : PaladinBlessingStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class HolyPaladinBlessingPveStrategy : public HolyPaladinBlessingStrategy
    {
    public:
        HolyPaladinBlessingPveStrategy(PlayerbotAI* ai) : HolyPaladinBlessingStrategy(ai) {}
        std::string getName() override { return "blessing holy pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class HolyPaladinBlessingPvpStrategy : public HolyPaladinBlessingStrategy
    {
    public:
        HolyPaladinBlessingPvpStrategy(PlayerbotAI* ai) : HolyPaladinBlessingStrategy(ai) {}
        std::string getName() override { return "blessing holy pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class HolyPaladinBlessingRaidStrategy : public HolyPaladinBlessingStrategy
    {
    public:
        HolyPaladinBlessingRaidStrategy(PlayerbotAI* ai) : HolyPaladinBlessingStrategy(ai) {}
        std::string getName() override { return "blessing holy raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };
}