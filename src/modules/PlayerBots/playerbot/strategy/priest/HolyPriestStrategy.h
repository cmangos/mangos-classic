#pragma once
#include "PriestStrategy.h"

namespace ai
{
    class HolyPriestPlaceholderStrategy : public SpecPlaceholderStrategy
    {
    public:
        HolyPriestPlaceholderStrategy(PlayerbotAI* ai) : SpecPlaceholderStrategy(ai) {}
        int GetType() override { return STRATEGY_TYPE_HEAL | STRATEGY_TYPE_RANGED; }
        std::string getName() override { return "holy"; }
    };

    class HolyPriestStrategy : public PriestStrategy
    {
    public:
        HolyPriestStrategy(PlayerbotAI* ai);

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class HolyPriestPveStrategy : public HolyPriestStrategy
    {
    public:
        HolyPriestPveStrategy(PlayerbotAI* ai) : HolyPriestStrategy(ai) {}
        std::string getName() override { return "holy pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
        void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class HolyPriestPvpStrategy : public HolyPriestStrategy
    {
    public:
        HolyPriestPvpStrategy(PlayerbotAI* ai) : HolyPriestStrategy(ai) {}
        std::string getName() override { return "holy pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
        void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class HolyPriestRaidStrategy : public HolyPriestStrategy
    {
    public:
        HolyPriestRaidStrategy(PlayerbotAI* ai) : HolyPriestStrategy(ai) {}
        std::string getName() override { return "holy raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
        void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class HolyPriestAoeStrategy : public PriestAoeStrategy
    {
    public:
        HolyPriestAoeStrategy(PlayerbotAI* ai) : PriestAoeStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class HolyPriestAoePveStrategy : public HolyPriestAoeStrategy
    {
    public:
        HolyPriestAoePveStrategy(PlayerbotAI* ai) : HolyPriestAoeStrategy(ai) {}
        std::string getName() override { return "aoe holy pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class HolyPriestAoePvpStrategy : public HolyPriestAoeStrategy
    {
    public:
        HolyPriestAoePvpStrategy(PlayerbotAI* ai) : HolyPriestAoeStrategy(ai) {}
        std::string getName() override { return "aoe holy pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class HolyPriestAoeRaidStrategy : public HolyPriestAoeStrategy
    {
    public:
        HolyPriestAoeRaidStrategy(PlayerbotAI* ai) : HolyPriestAoeStrategy(ai) {}
        std::string getName() override { return "aoe holy raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class HolyPriestBuffStrategy : public PriestBuffStrategy
    {
    public:
        HolyPriestBuffStrategy(PlayerbotAI* ai) : PriestBuffStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class HolyPriestBuffPveStrategy : public HolyPriestBuffStrategy
    {
    public:
        HolyPriestBuffPveStrategy(PlayerbotAI* ai) : HolyPriestBuffStrategy(ai) {}
        std::string getName() override { return "buff holy pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class HolyPriestBuffPvpStrategy : public HolyPriestBuffStrategy
    {
    public:
        HolyPriestBuffPvpStrategy(PlayerbotAI* ai) : HolyPriestBuffStrategy(ai) {}
        std::string getName() override { return "buff holy pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class HolyPriestBuffRaidStrategy : public HolyPriestBuffStrategy
    {
    public:
        HolyPriestBuffRaidStrategy(PlayerbotAI* ai) : HolyPriestBuffStrategy(ai) {}
        std::string getName() override { return "buff holy raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class HolyPriestBoostStrategy : public PriestBoostStrategy
    {
    public:
        HolyPriestBoostStrategy(PlayerbotAI* ai) : PriestBoostStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class HolyPriestBoostPveStrategy : public HolyPriestBoostStrategy
    {
    public:
        HolyPriestBoostPveStrategy(PlayerbotAI* ai) : HolyPriestBoostStrategy(ai) {}
        std::string getName() override { return "boost holy pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class HolyPriestBoostPvpStrategy : public HolyPriestBoostStrategy
    {
    public:
        HolyPriestBoostPvpStrategy(PlayerbotAI* ai) : HolyPriestBoostStrategy(ai) {}
        std::string getName() override { return "boost holy pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class HolyPriestBoostRaidStrategy : public HolyPriestBoostStrategy
    {
    public:
        HolyPriestBoostRaidStrategy(PlayerbotAI* ai) : HolyPriestBoostStrategy(ai) {}
        std::string getName() override { return "boost holy raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class HolyPriestCcStrategy : public PriestCcStrategy
    {
    public:
        HolyPriestCcStrategy(PlayerbotAI* ai) : PriestCcStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class HolyPriestCcPveStrategy : public HolyPriestCcStrategy
    {
    public:
        HolyPriestCcPveStrategy(PlayerbotAI* ai) : HolyPriestCcStrategy(ai) {}
        std::string getName() override { return "cc holy pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class HolyPriestCcPvpStrategy : public HolyPriestCcStrategy
    {
    public:
        HolyPriestCcPvpStrategy(PlayerbotAI* ai) : HolyPriestCcStrategy(ai) {}
        std::string getName() override { return "cc holy pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class HolyPriestCcRaidStrategy : public HolyPriestCcStrategy
    {
    public:
        HolyPriestCcRaidStrategy(PlayerbotAI* ai) : HolyPriestCcStrategy(ai) {}
        std::string getName() override { return "cc holy raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class HolyPriestCureStrategy : public PriestCureStrategy
    {
    public:
        HolyPriestCureStrategy(PlayerbotAI* ai) : PriestCureStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class HolyPriestCurePveStrategy : public HolyPriestCureStrategy
    {
    public:
        HolyPriestCurePveStrategy(PlayerbotAI* ai) : HolyPriestCureStrategy(ai) {}
        std::string getName() override { return "cure holy pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class HolyPriestCurePvpStrategy : public HolyPriestCureStrategy
    {
    public:
        HolyPriestCurePvpStrategy(PlayerbotAI* ai) : HolyPriestCureStrategy(ai) {}
        std::string getName() override { return "cure holy pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class HolyPriestCureRaidStrategy : public HolyPriestCureStrategy
    {
    public:
        HolyPriestCureRaidStrategy(PlayerbotAI* ai) : HolyPriestCureStrategy(ai) {}
        std::string getName() override { return "cure holy raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };
}