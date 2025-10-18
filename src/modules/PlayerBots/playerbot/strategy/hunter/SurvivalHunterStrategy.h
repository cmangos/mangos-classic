#pragma once
#include "HunterStrategy.h"

namespace ai
{
    class SurvivalHunterPlaceholderStrategy : public SpecPlaceholderStrategy
    {
    public:
        SurvivalHunterPlaceholderStrategy(PlayerbotAI* ai) : SpecPlaceholderStrategy(ai) {}
        int GetType() override { return STRATEGY_TYPE_DPS | STRATEGY_TYPE_RANGED; }
        std::string getName() override { return "survival"; }
    };

    class SurvivalHunterStrategy : public HunterStrategy
    {
    public:
        SurvivalHunterStrategy(PlayerbotAI* ai) : HunterStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class SurvivalHunterPveStrategy : public SurvivalHunterStrategy
    {
    public:
        SurvivalHunterPveStrategy(PlayerbotAI* ai) : SurvivalHunterStrategy(ai) {}
        std::string getName() override { return "survival pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
        void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class SurvivalHunterPvpStrategy : public SurvivalHunterStrategy
    {
    public:
        SurvivalHunterPvpStrategy(PlayerbotAI* ai) : SurvivalHunterStrategy(ai) {}
        std::string getName() override { return "survival pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
        void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class SurvivalHunterRaidStrategy : public SurvivalHunterStrategy
    {
    public:
        SurvivalHunterRaidStrategy(PlayerbotAI* ai) : SurvivalHunterStrategy(ai) {}
        std::string getName() override { return "survival raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
        void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class SurvivalHunterAoeStrategy : public HunterAoeStrategy
    {
    public:
        SurvivalHunterAoeStrategy(PlayerbotAI* ai) : HunterAoeStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class SurvivalHunterAoePveStrategy : public SurvivalHunterAoeStrategy
    {
    public:
        SurvivalHunterAoePveStrategy(PlayerbotAI* ai) : SurvivalHunterAoeStrategy(ai) {}
        std::string getName() override { return "aoe survival pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class SurvivalHunterAoePvpStrategy : public SurvivalHunterAoeStrategy
    {
    public:
        SurvivalHunterAoePvpStrategy(PlayerbotAI* ai) : SurvivalHunterAoeStrategy(ai) {}
        std::string getName() override { return "aoe survival pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class SurvivalHunterAoeRaidStrategy : public SurvivalHunterAoeStrategy
    {
    public:
        SurvivalHunterAoeRaidStrategy(PlayerbotAI* ai) : SurvivalHunterAoeStrategy(ai) {}
        std::string getName() override { return "aoe survival raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class SurvivalHunterBuffStrategy : public HunterBuffStrategy
    {
    public:
        SurvivalHunterBuffStrategy(PlayerbotAI* ai) : HunterBuffStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class SurvivalHunterBuffPveStrategy : public SurvivalHunterBuffStrategy
    {
    public:
        SurvivalHunterBuffPveStrategy(PlayerbotAI* ai) : SurvivalHunterBuffStrategy(ai) {}
        std::string getName() override { return "buff survival pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class SurvivalHunterBuffPvpStrategy : public SurvivalHunterBuffStrategy
    {
    public:
        SurvivalHunterBuffPvpStrategy(PlayerbotAI* ai) : SurvivalHunterBuffStrategy(ai) {}
        std::string getName() override { return "buff survival pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class SurvivalHunterBuffRaidStrategy : public SurvivalHunterBuffStrategy
    {
    public:
        SurvivalHunterBuffRaidStrategy(PlayerbotAI* ai) : SurvivalHunterBuffStrategy(ai) {}
        std::string getName() override { return "buff survival raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class SurvivalHunterBoostStrategy : public HunterBoostStrategy
    {
    public:
        SurvivalHunterBoostStrategy(PlayerbotAI* ai) : HunterBoostStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class SurvivalHunterBoostPveStrategy : public SurvivalHunterBoostStrategy
    {
    public:
        SurvivalHunterBoostPveStrategy(PlayerbotAI* ai) : SurvivalHunterBoostStrategy(ai) {}
        std::string getName() override { return "boost survival pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class SurvivalHunterBoostPvpStrategy : public SurvivalHunterBoostStrategy
    {
    public:
        SurvivalHunterBoostPvpStrategy(PlayerbotAI* ai) : SurvivalHunterBoostStrategy(ai) {}
        std::string getName() override { return "boost survival pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class SurvivalHunterBoostRaidStrategy : public SurvivalHunterBoostStrategy
    {
    public:
        SurvivalHunterBoostRaidStrategy(PlayerbotAI* ai) : SurvivalHunterBoostStrategy(ai) {}
        std::string getName() override { return "boost survival raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class SurvivalHunterCcStrategy : public HunterCcStrategy
    {
    public:
        SurvivalHunterCcStrategy(PlayerbotAI* ai) : HunterCcStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class SurvivalHunterCcPveStrategy : public SurvivalHunterCcStrategy
    {
    public:
        SurvivalHunterCcPveStrategy(PlayerbotAI* ai) : SurvivalHunterCcStrategy(ai) {}
        std::string getName() override { return "cc survival pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class SurvivalHunterCcPvpStrategy : public SurvivalHunterCcStrategy
    {
    public:
        SurvivalHunterCcPvpStrategy(PlayerbotAI* ai) : SurvivalHunterCcStrategy(ai) {}
        std::string getName() override { return "cc survival pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class SurvivalHunterCcRaidStrategy : public SurvivalHunterCcStrategy
    {
    public:
        SurvivalHunterCcRaidStrategy(PlayerbotAI* ai) : SurvivalHunterCcStrategy(ai) {}
        std::string getName() override { return "cc survival raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class SurvivalHunterStingStrategy : public HunterStingStrategy
    {
    public:
        SurvivalHunterStingStrategy(PlayerbotAI* ai) : HunterStingStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class SurvivalHunterStingPveStrategy : public SurvivalHunterStingStrategy
    {
    public:
        SurvivalHunterStingPveStrategy(PlayerbotAI* ai) : SurvivalHunterStingStrategy(ai) {}
        std::string getName() override { return "sting survival pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class SurvivalHunterStingPvpStrategy : public SurvivalHunterStingStrategy
    {
    public:
        SurvivalHunterStingPvpStrategy(PlayerbotAI* ai) : SurvivalHunterStingStrategy(ai) {}
        std::string getName() override { return "sting survival pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class SurvivalHunterStingRaidStrategy : public SurvivalHunterStingStrategy
    {
    public:
        SurvivalHunterStingRaidStrategy(PlayerbotAI* ai) : SurvivalHunterStingStrategy(ai) {}
        std::string getName() override { return "sting survival raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class SurvivalHunterAspectStrategy : public HunterAspectStrategy
    {
    public:
        SurvivalHunterAspectStrategy(PlayerbotAI* ai) : HunterAspectStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class SurvivalHunterAspectPveStrategy : public SurvivalHunterAspectStrategy
    {
    public:
        SurvivalHunterAspectPveStrategy(PlayerbotAI* ai) : SurvivalHunterAspectStrategy(ai) {}
        std::string getName() override { return "aspect survival pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class SurvivalHunterAspectPvpStrategy : public SurvivalHunterAspectStrategy
    {
    public:
        SurvivalHunterAspectPvpStrategy(PlayerbotAI* ai) : SurvivalHunterAspectStrategy(ai) {}
        std::string getName() override { return "aspect survival pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class SurvivalHunterAspectRaidStrategy : public SurvivalHunterAspectStrategy
    {
    public:
        SurvivalHunterAspectRaidStrategy(PlayerbotAI* ai) : SurvivalHunterAspectStrategy(ai) {}
        std::string getName() override { return "aspect survival raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };
}