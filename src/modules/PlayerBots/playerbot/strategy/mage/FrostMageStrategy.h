#pragma once
#include "MageStrategy.h"

namespace ai
{
    class FrostMagePlaceholderStrategy : public SpecPlaceholderStrategy
    {
    public:
        FrostMagePlaceholderStrategy(PlayerbotAI* ai) : SpecPlaceholderStrategy(ai) {}
        int GetType() override { return STRATEGY_TYPE_DPS | STRATEGY_TYPE_RANGED; }
        std::string getName() override { return "frost"; }
    };

    class FrostMageStrategy : public MageStrategy
    {
    public:
        FrostMageStrategy(PlayerbotAI* ai) : MageStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;

        virtual NextAction** GetDefaultCombatActions() override;
    };

    class FrostMagePveStrategy : public FrostMageStrategy
    {
    public:
        FrostMagePveStrategy(PlayerbotAI* ai) : FrostMageStrategy(ai) {}
        std::string getName() override { return "frost pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
        void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class FrostMagePvpStrategy : public FrostMageStrategy
    {
    public:
        FrostMagePvpStrategy(PlayerbotAI* ai) : FrostMageStrategy(ai) {}
        std::string getName() override { return "frost pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
        void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class FrostMageRaidStrategy : public FrostMageStrategy
    {
    public:
        FrostMageRaidStrategy(PlayerbotAI* ai) : FrostMageStrategy(ai) {}
        std::string getName() override { return "frost raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
        void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class FrostMageAoeStrategy : public MageAoeStrategy
    {
    public:
        FrostMageAoeStrategy(PlayerbotAI* ai) : MageAoeStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class FrostMageAoePveStrategy : public FrostMageAoeStrategy
    {
    public:
        FrostMageAoePveStrategy(PlayerbotAI* ai) : FrostMageAoeStrategy(ai) {}
        std::string getName() override { return "aoe frost pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class FrostMageAoePvpStrategy : public FrostMageAoeStrategy
    {
    public:
        FrostMageAoePvpStrategy(PlayerbotAI* ai) : FrostMageAoeStrategy(ai) {}
        std::string getName() override { return "aoe frost pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class FrostMageAoeRaidStrategy : public FrostMageAoeStrategy
    {
    public:
        FrostMageAoeRaidStrategy(PlayerbotAI* ai) : FrostMageAoeStrategy(ai) {}
        std::string getName() override { return "aoe frost raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class FrostMageBuffStrategy : public MageBuffStrategy
    {
    public:
        FrostMageBuffStrategy(PlayerbotAI* ai) : MageBuffStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class FrostMageBuffPveStrategy : public FrostMageBuffStrategy
    {
    public:
        FrostMageBuffPveStrategy(PlayerbotAI* ai) : FrostMageBuffStrategy(ai) {}
        std::string getName() override { return "buff frost pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class FrostMageBuffPvpStrategy : public FrostMageBuffStrategy
    {
    public:
        FrostMageBuffPvpStrategy(PlayerbotAI* ai) : FrostMageBuffStrategy(ai) {}
        std::string getName() override { return "buff frost pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class FrostMageBuffRaidStrategy : public FrostMageBuffStrategy
    {
    public:
        FrostMageBuffRaidStrategy(PlayerbotAI* ai) : FrostMageBuffStrategy(ai) {}
        std::string getName() override { return "buff frost raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class FrostMageBoostStrategy : public MageBoostStrategy
    {
    public:
        FrostMageBoostStrategy(PlayerbotAI* ai) : MageBoostStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class FrostMageBoostPveStrategy : public FrostMageBoostStrategy
    {
    public:
        FrostMageBoostPveStrategy(PlayerbotAI* ai) : FrostMageBoostStrategy(ai) {}
        std::string getName() override { return "boost frost pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class FrostMageBoostPvpStrategy : public FrostMageBoostStrategy
    {
    public:
        FrostMageBoostPvpStrategy(PlayerbotAI* ai) : FrostMageBoostStrategy(ai) {}
        std::string getName() override { return "boost frost pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class FrostMageBoostRaidStrategy : public FrostMageBoostStrategy
    {
    public:
        FrostMageBoostRaidStrategy(PlayerbotAI* ai) : FrostMageBoostStrategy(ai) {}
        std::string getName() override { return "boost frost raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class FrostMageCcStrategy : public MageCcStrategy
    {
    public:
        FrostMageCcStrategy(PlayerbotAI* ai) : MageCcStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class FrostMageCcPveStrategy : public FrostMageCcStrategy
    {
    public:
        FrostMageCcPveStrategy(PlayerbotAI* ai) : FrostMageCcStrategy(ai) {}
        std::string getName() override { return "cc frost pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class FrostMageCcPvpStrategy : public FrostMageCcStrategy
    {
    public:
        FrostMageCcPvpStrategy(PlayerbotAI* ai) : FrostMageCcStrategy(ai) {}
        std::string getName() override { return "cc frost pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class FrostMageCcRaidStrategy : public FrostMageCcStrategy
    {
    public:
        FrostMageCcRaidStrategy(PlayerbotAI* ai) : FrostMageCcStrategy(ai) {}
        std::string getName() override { return "cc frost raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class FrostMageCureStrategy : public MageCureStrategy
    {
    public:
        FrostMageCureStrategy(PlayerbotAI* ai) : MageCureStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class FrostMageCurePveStrategy : public FrostMageCureStrategy
    {
    public:
        FrostMageCurePveStrategy(PlayerbotAI* ai) : FrostMageCureStrategy(ai) {}
        std::string getName() override { return "cure frost pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class FrostMageCurePvpStrategy : public FrostMageCureStrategy
    {
    public:
        FrostMageCurePvpStrategy(PlayerbotAI* ai) : FrostMageCureStrategy(ai) {}
        std::string getName() override { return "cure frost pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class FrostMageCureRaidStrategy : public FrostMageCureStrategy
    {
    public:
        FrostMageCureRaidStrategy(PlayerbotAI* ai) : FrostMageCureStrategy(ai) {}
        std::string getName() override { return "cure frost raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };
}