#pragma once
#include "PriestStrategy.h"

namespace ai
{
    class DisciplinePriestPlaceholderStrategy : public SpecPlaceholderStrategy
    {
    public:
        DisciplinePriestPlaceholderStrategy(PlayerbotAI* ai) : SpecPlaceholderStrategy(ai) {}
        int GetType() override { return STRATEGY_TYPE_HEAL | STRATEGY_TYPE_RANGED; }
        std::string getName() override { return "discipline"; }
    };

    class DisciplinePriestStrategy : public PriestStrategy
    {
    public:
        DisciplinePriestStrategy(PlayerbotAI* ai);

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class DisciplinePriestPveStrategy : public DisciplinePriestStrategy
    {
    public:
        DisciplinePriestPveStrategy(PlayerbotAI* ai) : DisciplinePriestStrategy(ai) {}
        std::string getName() override { return "discipline pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
        void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class DisciplinePriestPvpStrategy : public DisciplinePriestStrategy
    {
    public:
        DisciplinePriestPvpStrategy(PlayerbotAI* ai) : DisciplinePriestStrategy(ai) {}
        std::string getName() override { return "discipline pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
        void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class DisciplinePriestRaidStrategy : public DisciplinePriestStrategy
    {
    public:
        DisciplinePriestRaidStrategy(PlayerbotAI* ai) : DisciplinePriestStrategy(ai) {}
        std::string getName() override { return "discipline raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
        void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class DisciplinePriestAoeStrategy : public PriestAoeStrategy
    {
    public:
        DisciplinePriestAoeStrategy(PlayerbotAI* ai) : PriestAoeStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class DisciplinePriestAoePveStrategy : public DisciplinePriestAoeStrategy
    {
    public:
        DisciplinePriestAoePveStrategy(PlayerbotAI* ai) : DisciplinePriestAoeStrategy(ai) {}
        std::string getName() override { return "aoe discipline pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class DisciplinePriestAoePvpStrategy : public DisciplinePriestAoeStrategy
    {
    public:
        DisciplinePriestAoePvpStrategy(PlayerbotAI* ai) : DisciplinePriestAoeStrategy(ai) {}
        std::string getName() override { return "aoe discipline pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class DisciplinePriestAoeRaidStrategy : public DisciplinePriestAoeStrategy
    {
    public:
        DisciplinePriestAoeRaidStrategy(PlayerbotAI* ai) : DisciplinePriestAoeStrategy(ai) {}
        std::string getName() override { return "aoe discipline raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class DisciplinePriestBuffStrategy : public PriestBuffStrategy
    {
    public:
        DisciplinePriestBuffStrategy(PlayerbotAI* ai) : PriestBuffStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class DisciplinePriestBuffPveStrategy : public DisciplinePriestBuffStrategy
    {
    public:
        DisciplinePriestBuffPveStrategy(PlayerbotAI* ai) : DisciplinePriestBuffStrategy(ai) {}
        std::string getName() override { return "buff discipline pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class DisciplinePriestBuffPvpStrategy : public DisciplinePriestBuffStrategy
    {
    public:
        DisciplinePriestBuffPvpStrategy(PlayerbotAI* ai) : DisciplinePriestBuffStrategy(ai) {}
        std::string getName() override { return "buff discipline pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class DisciplinePriestBuffRaidStrategy : public DisciplinePriestBuffStrategy
    {
    public:
        DisciplinePriestBuffRaidStrategy(PlayerbotAI* ai) : DisciplinePriestBuffStrategy(ai) {}
        std::string getName() override { return "buff discipline raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class DisciplinePriestBoostStrategy : public PriestBoostStrategy
    {
    public:
        DisciplinePriestBoostStrategy(PlayerbotAI* ai) : PriestBoostStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class DisciplinePriestBoostPveStrategy : public DisciplinePriestBoostStrategy
    {
    public:
        DisciplinePriestBoostPveStrategy(PlayerbotAI* ai) : DisciplinePriestBoostStrategy(ai) {}
        std::string getName() override { return "boost discipline pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class DisciplinePriestBoostPvpStrategy : public DisciplinePriestBoostStrategy
    {
    public:
        DisciplinePriestBoostPvpStrategy(PlayerbotAI* ai) : DisciplinePriestBoostStrategy(ai) {}
        std::string getName() override { return "boost discipline pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class DisciplinePriestBoostRaidStrategy : public DisciplinePriestBoostStrategy
    {
    public:
        DisciplinePriestBoostRaidStrategy(PlayerbotAI* ai) : DisciplinePriestBoostStrategy(ai) {}
        std::string getName() override { return "boost discipline raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class DisciplinePriestCcStrategy : public PriestCcStrategy
    {
    public:
        DisciplinePriestCcStrategy(PlayerbotAI* ai) : PriestCcStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class DisciplinePriestCcPveStrategy : public DisciplinePriestCcStrategy
    {
    public:
        DisciplinePriestCcPveStrategy(PlayerbotAI* ai) : DisciplinePriestCcStrategy(ai) {}
        std::string getName() override { return "cc discipline pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class DisciplinePriestCcPvpStrategy : public DisciplinePriestCcStrategy
    {
    public:
        DisciplinePriestCcPvpStrategy(PlayerbotAI* ai) : DisciplinePriestCcStrategy(ai) {}
        std::string getName() override { return "cc discipline pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class DisciplinePriestCcRaidStrategy : public DisciplinePriestCcStrategy
    {
    public:
        DisciplinePriestCcRaidStrategy(PlayerbotAI* ai) : DisciplinePriestCcStrategy(ai) {}
        std::string getName() override { return "cc discipline raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class DisciplinePriestCureStrategy : public PriestCureStrategy
    {
    public:
        DisciplinePriestCureStrategy(PlayerbotAI* ai) : PriestCureStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class DisciplinePriestCurePveStrategy : public DisciplinePriestCureStrategy
    {
    public:
        DisciplinePriestCurePveStrategy(PlayerbotAI* ai) : DisciplinePriestCureStrategy(ai) {}
        std::string getName() override { return "cure discipline pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class DisciplinePriestCurePvpStrategy : public DisciplinePriestCureStrategy
    {
    public:
        DisciplinePriestCurePvpStrategy(PlayerbotAI* ai) : DisciplinePriestCureStrategy(ai) {}
        std::string getName() override { return "cure discipline pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class DisciplinePriestCureRaidStrategy : public DisciplinePriestCureStrategy
    {
    public:
        DisciplinePriestCureRaidStrategy(PlayerbotAI* ai) : DisciplinePriestCureStrategy(ai) {}
        std::string getName() override { return "cure discipline raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };
}