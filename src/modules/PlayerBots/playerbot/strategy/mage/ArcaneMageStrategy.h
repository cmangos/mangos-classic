#pragma once
#include "MageStrategy.h"

namespace ai
{
    class ArcaneMagePlaceholderStrategy : public SpecPlaceholderStrategy
    {
    public:
        ArcaneMagePlaceholderStrategy(PlayerbotAI* ai) : SpecPlaceholderStrategy(ai) {}
        int GetType() override { return STRATEGY_TYPE_DPS | STRATEGY_TYPE_RANGED; }
        std::string getName() override { return "arcane"; }
    };

    class ArcaneMageStrategy : public MageStrategy
    {
    public:
        ArcaneMageStrategy(PlayerbotAI* ai);

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;

        virtual NextAction** GetDefaultCombatActions() override;
    };

    class ArcaneMagePveStrategy : public ArcaneMageStrategy
    {
    public:
        ArcaneMagePveStrategy(PlayerbotAI* ai) : ArcaneMageStrategy(ai) {}
        std::string getName() override { return "arcane pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
        void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class ArcaneMagePvpStrategy : public ArcaneMageStrategy
    {
    public:
        ArcaneMagePvpStrategy(PlayerbotAI* ai) : ArcaneMageStrategy(ai) {}
        std::string getName() override { return "arcane pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
        void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class ArcaneMageRaidStrategy : public ArcaneMageStrategy
    {
    public:
        ArcaneMageRaidStrategy(PlayerbotAI* ai) : ArcaneMageStrategy(ai) {}
        std::string getName() override { return "arcane raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
        void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class ArcaneMageAoeStrategy : public MageAoeStrategy
    {
    public:
        ArcaneMageAoeStrategy(PlayerbotAI* ai) : MageAoeStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class ArcaneMageAoePveStrategy : public ArcaneMageAoeStrategy
    {
    public:
        ArcaneMageAoePveStrategy(PlayerbotAI* ai) : ArcaneMageAoeStrategy(ai) {}
        std::string getName() override { return "aoe arcane pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class ArcaneMageAoePvpStrategy : public ArcaneMageAoeStrategy
    {
    public:
        ArcaneMageAoePvpStrategy(PlayerbotAI* ai) : ArcaneMageAoeStrategy(ai) {}
        std::string getName() override { return "aoe arcane pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class ArcaneMageAoeRaidStrategy : public ArcaneMageAoeStrategy
    {
    public:
        ArcaneMageAoeRaidStrategy(PlayerbotAI* ai) : ArcaneMageAoeStrategy(ai) {}
        std::string getName() override { return "aoe arcane raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class ArcaneMageBuffStrategy : public MageBuffStrategy
    {
    public:
        ArcaneMageBuffStrategy(PlayerbotAI* ai) : MageBuffStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class ArcaneMageBuffPveStrategy : public ArcaneMageBuffStrategy
    {
    public:
        ArcaneMageBuffPveStrategy(PlayerbotAI* ai) : ArcaneMageBuffStrategy(ai) {}
        std::string getName() override { return "buff arcane pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class ArcaneMageBuffPvpStrategy : public ArcaneMageBuffStrategy
    {
    public:
        ArcaneMageBuffPvpStrategy(PlayerbotAI* ai) : ArcaneMageBuffStrategy(ai) {}
        std::string getName() override { return "buff arcane pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class ArcaneMageBuffRaidStrategy : public ArcaneMageBuffStrategy
    {
    public:
        ArcaneMageBuffRaidStrategy(PlayerbotAI* ai) : ArcaneMageBuffStrategy(ai) {}
        std::string getName() override { return "buff arcane raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class ArcaneMageBoostStrategy : public MageBoostStrategy
    {
    public:
        ArcaneMageBoostStrategy(PlayerbotAI* ai) : MageBoostStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class ArcaneMageBoostPveStrategy : public ArcaneMageBoostStrategy
    {
    public:
        ArcaneMageBoostPveStrategy(PlayerbotAI* ai) : ArcaneMageBoostStrategy(ai) {}
        std::string getName() override { return "boost arcane pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class ArcaneMageBoostPvpStrategy : public ArcaneMageBoostStrategy
    {
    public:
        ArcaneMageBoostPvpStrategy(PlayerbotAI* ai) : ArcaneMageBoostStrategy(ai) {}
        std::string getName() override { return "boost arcane pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class ArcaneMageBoostRaidStrategy : public ArcaneMageBoostStrategy
    {
    public:
        ArcaneMageBoostRaidStrategy(PlayerbotAI* ai) : ArcaneMageBoostStrategy(ai) {}
        std::string getName() override { return "boost arcane raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class ArcaneMageCcStrategy : public MageCcStrategy
    {
    public:
        ArcaneMageCcStrategy(PlayerbotAI* ai) : MageCcStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class ArcaneMageCcPveStrategy : public ArcaneMageCcStrategy
    {
    public:
        ArcaneMageCcPveStrategy(PlayerbotAI* ai) : ArcaneMageCcStrategy(ai) {}
        std::string getName() override { return "cc arcane pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class ArcaneMageCcPvpStrategy : public ArcaneMageCcStrategy
    {
    public:
        ArcaneMageCcPvpStrategy(PlayerbotAI* ai) : ArcaneMageCcStrategy(ai) {}
        std::string getName() override { return "cc arcane pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class ArcaneMageCcRaidStrategy : public ArcaneMageCcStrategy
    {
    public:
        ArcaneMageCcRaidStrategy(PlayerbotAI* ai) : ArcaneMageCcStrategy(ai) {}
        std::string getName() override { return "cc arcane raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class ArcaneMageCureStrategy : public MageCureStrategy
    {
    public:
        ArcaneMageCureStrategy(PlayerbotAI* ai) : MageCureStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class ArcaneMageCurePveStrategy : public ArcaneMageCureStrategy
    {
    public:
        ArcaneMageCurePveStrategy(PlayerbotAI* ai) : ArcaneMageCureStrategy(ai) {}
        std::string getName() override { return "cure arcane pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class ArcaneMageCurePvpStrategy : public ArcaneMageCureStrategy
    {
    public:
        ArcaneMageCurePvpStrategy(PlayerbotAI* ai) : ArcaneMageCureStrategy(ai) {}
        std::string getName() override { return "cure arcane pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class ArcaneMageCureRaidStrategy : public ArcaneMageCureStrategy
    {
    public:
        ArcaneMageCureRaidStrategy(PlayerbotAI* ai) : ArcaneMageCureStrategy(ai) {}
        std::string getName() override { return "cure arcane raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };
}