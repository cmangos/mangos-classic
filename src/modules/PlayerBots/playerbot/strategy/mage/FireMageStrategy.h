#pragma once
#include "MageStrategy.h"

namespace ai
{
    class FireMagePlaceholderStrategy : public SpecPlaceholderStrategy
    {
    public:
        FireMagePlaceholderStrategy(PlayerbotAI* ai) : SpecPlaceholderStrategy(ai) {}
        int GetType() override { return STRATEGY_TYPE_DPS | STRATEGY_TYPE_RANGED; }
        std::string getName() override { return "fire"; }
    };

    class FireMageStrategy : public MageStrategy
    {
    public:
        FireMageStrategy(PlayerbotAI* ai);

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;

        virtual NextAction** GetDefaultCombatActions() override;
    };

    class FireMagePveStrategy : public FireMageStrategy
    {
    public:
        FireMagePveStrategy(PlayerbotAI* ai) : FireMageStrategy(ai) {}
        std::string getName() override { return "fire pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
        void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class FireMagePvpStrategy : public FireMageStrategy
    {
    public:
        FireMagePvpStrategy(PlayerbotAI* ai) : FireMageStrategy(ai) {}
        std::string getName() override { return "fire pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
        void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class FireMageRaidStrategy : public FireMageStrategy
    {
    public:
        FireMageRaidStrategy(PlayerbotAI* ai) : FireMageStrategy(ai) {}
        std::string getName() override { return "fire raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
        void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class FireMageAoeStrategy : public MageAoeStrategy
    {
    public:
        FireMageAoeStrategy(PlayerbotAI* ai) : MageAoeStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class FireMageAoePveStrategy : public FireMageAoeStrategy
    {
    public:
        FireMageAoePveStrategy(PlayerbotAI* ai) : FireMageAoeStrategy(ai) {}
        std::string getName() override { return "aoe fire pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class FireMageAoePvpStrategy : public FireMageAoeStrategy
    {
    public:
        FireMageAoePvpStrategy(PlayerbotAI* ai) : FireMageAoeStrategy(ai) {}
        std::string getName() override { return "aoe fire pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class FireMageAoeRaidStrategy : public FireMageAoeStrategy
    {
    public:
        FireMageAoeRaidStrategy(PlayerbotAI* ai) : FireMageAoeStrategy(ai) {}
        std::string getName() override { return "aoe fire raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class FireMageBuffStrategy : public MageBuffStrategy
    {
    public:
        FireMageBuffStrategy(PlayerbotAI* ai) : MageBuffStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class FireMageBuffPveStrategy : public FireMageBuffStrategy
    {
    public:
        FireMageBuffPveStrategy(PlayerbotAI* ai) : FireMageBuffStrategy(ai) {}
        std::string getName() override { return "buff fire pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class FireMageBuffPvpStrategy : public FireMageBuffStrategy
    {
    public:
        FireMageBuffPvpStrategy(PlayerbotAI* ai) : FireMageBuffStrategy(ai) {}
        std::string getName() override { return "buff fire pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class FireMageBuffRaidStrategy : public FireMageBuffStrategy
    {
    public:
        FireMageBuffRaidStrategy(PlayerbotAI* ai) : FireMageBuffStrategy(ai) {}
        std::string getName() override { return "buff fire raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class FireMageBoostStrategy : public MageBoostStrategy
    {
    public:
        FireMageBoostStrategy(PlayerbotAI* ai) : MageBoostStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class FireMageBoostPveStrategy : public FireMageBoostStrategy
    {
    public:
        FireMageBoostPveStrategy(PlayerbotAI* ai) : FireMageBoostStrategy(ai) {}
        std::string getName() override { return "boost fire pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class FireMageBoostPvpStrategy : public FireMageBoostStrategy
    {
    public:
        FireMageBoostPvpStrategy(PlayerbotAI* ai) : FireMageBoostStrategy(ai) {}
        std::string getName() override { return "boost fire pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class FireMageBoostRaidStrategy : public FireMageBoostStrategy
    {
    public:
        FireMageBoostRaidStrategy(PlayerbotAI* ai) : FireMageBoostStrategy(ai) {}
        std::string getName() override { return "boost fire raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class FireMageCcStrategy : public MageCcStrategy
    {
    public:
        FireMageCcStrategy(PlayerbotAI* ai) : MageCcStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class FireMageCcPveStrategy : public FireMageCcStrategy
    {
    public:
        FireMageCcPveStrategy(PlayerbotAI* ai) : FireMageCcStrategy(ai) {}
        std::string getName() override { return "cc fire pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class FireMageCcPvpStrategy : public FireMageCcStrategy
    {
    public:
        FireMageCcPvpStrategy(PlayerbotAI* ai) : FireMageCcStrategy(ai) {}
        std::string getName() override { return "cc fire pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class FireMageCcRaidStrategy : public FireMageCcStrategy
    {
    public:
        FireMageCcRaidStrategy(PlayerbotAI* ai) : FireMageCcStrategy(ai) {}
        std::string getName() override { return "cc fire raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class FireMageCureStrategy : public MageCureStrategy
    {
    public:
        FireMageCureStrategy(PlayerbotAI* ai) : MageCureStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class FireMageCurePveStrategy : public FireMageCureStrategy
    {
    public:
        FireMageCurePveStrategy(PlayerbotAI* ai) : FireMageCureStrategy(ai) {}
        std::string getName() override { return "cure fire pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class FireMageCurePvpStrategy : public FireMageCureStrategy
    {
    public:
        FireMageCurePvpStrategy(PlayerbotAI* ai) : FireMageCureStrategy(ai) {}
        std::string getName() override { return "cure fire pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class FireMageCureRaidStrategy : public FireMageCureStrategy
    {
    public:
        FireMageCureRaidStrategy(PlayerbotAI* ai) : FireMageCureStrategy(ai) {}
        std::string getName() override { return "cure fire raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };
}