#pragma once
#include "PriestStrategy.h"

namespace ai
{
    class ShadowPriestPlaceholderStrategy : public SpecPlaceholderStrategy
    {
    public:
        ShadowPriestPlaceholderStrategy(PlayerbotAI* ai) : SpecPlaceholderStrategy(ai) {}
        int GetType() override { return STRATEGY_TYPE_DPS | STRATEGY_TYPE_RANGED; }
        std::string getName() override { return "shadow"; }
    };

    class ShadowPriestStrategy : public PriestStrategy
    {
    public:
        ShadowPriestStrategy(PlayerbotAI* ai);

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;

        virtual NextAction** GetDefaultCombatActions() override;
    };

    class ShadowPriestPveStrategy : public ShadowPriestStrategy
    {
    public:
        ShadowPriestPveStrategy(PlayerbotAI* ai) : ShadowPriestStrategy(ai) {}
        std::string getName() override { return "shadow pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
        void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class ShadowPriestPvpStrategy : public ShadowPriestStrategy
    {
    public:
        ShadowPriestPvpStrategy(PlayerbotAI* ai) : ShadowPriestStrategy(ai) {}
        std::string getName() override { return "shadow pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
        void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class ShadowPriestRaidStrategy : public ShadowPriestStrategy
    {
    public:
        ShadowPriestRaidStrategy(PlayerbotAI* ai) : ShadowPriestStrategy(ai) {}
        std::string getName() override { return "shadow raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
        void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class ShadowPriestAoeStrategy : public PriestAoeStrategy
    {
    public:
        ShadowPriestAoeStrategy(PlayerbotAI* ai) : PriestAoeStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class ShadowPriestAoePveStrategy : public ShadowPriestAoeStrategy
    {
    public:
        ShadowPriestAoePveStrategy(PlayerbotAI* ai) : ShadowPriestAoeStrategy(ai) {}
        std::string getName() override { return "aoe shadow pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class ShadowPriestAoePvpStrategy : public ShadowPriestAoeStrategy
    {
    public:
        ShadowPriestAoePvpStrategy(PlayerbotAI* ai) : ShadowPriestAoeStrategy(ai) {}
        std::string getName() override { return "aoe shadow pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class ShadowPriestAoeRaidStrategy : public ShadowPriestAoeStrategy
    {
    public:
        ShadowPriestAoeRaidStrategy(PlayerbotAI* ai) : ShadowPriestAoeStrategy(ai) {}
        std::string getName() override { return "aoe shadow raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class ShadowPriestBuffStrategy : public PriestBuffStrategy
    {
    public:
        ShadowPriestBuffStrategy(PlayerbotAI* ai) : PriestBuffStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class ShadowPriestBuffPveStrategy : public ShadowPriestBuffStrategy
    {
    public:
        ShadowPriestBuffPveStrategy(PlayerbotAI* ai) : ShadowPriestBuffStrategy(ai) {}
        std::string getName() override { return "buff shadow pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class ShadowPriestBuffPvpStrategy : public ShadowPriestBuffStrategy
    {
    public:
        ShadowPriestBuffPvpStrategy(PlayerbotAI* ai) : ShadowPriestBuffStrategy(ai) {}
        std::string getName() override { return "buff shadow pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class ShadowPriestBuffRaidStrategy : public ShadowPriestBuffStrategy
    {
    public:
        ShadowPriestBuffRaidStrategy(PlayerbotAI* ai) : ShadowPriestBuffStrategy(ai) {}
        std::string getName() override { return "buff shadow raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class ShadowPriestBoostStrategy : public PriestBoostStrategy
    {
    public:
        ShadowPriestBoostStrategy(PlayerbotAI* ai) : PriestBoostStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class ShadowPriestBoostPveStrategy : public ShadowPriestBoostStrategy
    {
    public:
        ShadowPriestBoostPveStrategy(PlayerbotAI* ai) : ShadowPriestBoostStrategy(ai) {}
        std::string getName() override { return "boost shadow pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class ShadowPriestBoostPvpStrategy : public ShadowPriestBoostStrategy
    {
    public:
        ShadowPriestBoostPvpStrategy(PlayerbotAI* ai) : ShadowPriestBoostStrategy(ai) {}
        std::string getName() override { return "boost shadow pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class ShadowPriestBoostRaidStrategy : public ShadowPriestBoostStrategy
    {
    public:
        ShadowPriestBoostRaidStrategy(PlayerbotAI* ai) : ShadowPriestBoostStrategy(ai) {}
        std::string getName() override { return "boost shadow raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class ShadowPriestCcStrategy : public PriestCcStrategy
    {
    public:
        ShadowPriestCcStrategy(PlayerbotAI* ai) : PriestCcStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class ShadowPriestCcPveStrategy : public ShadowPriestCcStrategy
    {
    public:
        ShadowPriestCcPveStrategy(PlayerbotAI* ai) : ShadowPriestCcStrategy(ai) {}
        std::string getName() override { return "cc shadow pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class ShadowPriestCcPvpStrategy : public ShadowPriestCcStrategy
    {
    public:
        ShadowPriestCcPvpStrategy(PlayerbotAI* ai) : ShadowPriestCcStrategy(ai) {}
        std::string getName() override { return "cc shadow pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class ShadowPriestCcRaidStrategy : public ShadowPriestCcStrategy
    {
    public:
        ShadowPriestCcRaidStrategy(PlayerbotAI* ai) : ShadowPriestCcStrategy(ai) {}
        std::string getName() override { return "cc shadow raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class ShadowPriestCureStrategy : public PriestCureStrategy
    {
    public:
        ShadowPriestCureStrategy(PlayerbotAI* ai) : PriestCureStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class ShadowPriestCurePveStrategy : public ShadowPriestCureStrategy
    {
    public:
        ShadowPriestCurePveStrategy(PlayerbotAI* ai) : ShadowPriestCureStrategy(ai) {}
        std::string getName() override { return "cure shadow pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class ShadowPriestCurePvpStrategy : public ShadowPriestCureStrategy
    {
    public:
        ShadowPriestCurePvpStrategy(PlayerbotAI* ai) : ShadowPriestCureStrategy(ai) {}
        std::string getName() override { return "cure shadow pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class ShadowPriestCureRaidStrategy : public ShadowPriestCureStrategy
    {
    public:
        ShadowPriestCureRaidStrategy(PlayerbotAI* ai) : ShadowPriestCureStrategy(ai) {}
        std::string getName() override { return "cure shadow raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };
}