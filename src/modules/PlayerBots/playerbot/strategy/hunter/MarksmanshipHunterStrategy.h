#pragma once
#include "HunterStrategy.h"

namespace ai
{
    class MarksmanshipHunterPlaceholderStrategy : public SpecPlaceholderStrategy
    {
    public:
        MarksmanshipHunterPlaceholderStrategy(PlayerbotAI* ai) : SpecPlaceholderStrategy(ai) {}
        int GetType() override { return STRATEGY_TYPE_DPS | STRATEGY_TYPE_RANGED; }
        std::string getName() override { return "marksmanship"; }
    };

    class MarksmanshipHunterStrategy : public HunterStrategy
    {
    public:
        MarksmanshipHunterStrategy(PlayerbotAI* ai) : HunterStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class MarksmanshipHunterPveStrategy : public MarksmanshipHunterStrategy
    {
    public:
        MarksmanshipHunterPveStrategy(PlayerbotAI* ai) : MarksmanshipHunterStrategy(ai) {}
        std::string getName() override { return "marksmanship pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
        void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class MarksmanshipHunterPvpStrategy : public MarksmanshipHunterStrategy
    {
    public:
        MarksmanshipHunterPvpStrategy(PlayerbotAI* ai) : MarksmanshipHunterStrategy(ai) {}
        std::string getName() override { return "marksmanship pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
        void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class MarksmanshipHunterRaidStrategy : public MarksmanshipHunterStrategy
    {
    public:
        MarksmanshipHunterRaidStrategy(PlayerbotAI* ai) : MarksmanshipHunterStrategy(ai) {}
        std::string getName() override { return "marksmanship raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
        void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class MarksmanshipHunterAoeStrategy : public HunterAoeStrategy
    {
    public:
        MarksmanshipHunterAoeStrategy(PlayerbotAI* ai) : HunterAoeStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class MarksmanshipHunterAoePveStrategy : public MarksmanshipHunterAoeStrategy
    {
    public:
        MarksmanshipHunterAoePveStrategy(PlayerbotAI* ai) : MarksmanshipHunterAoeStrategy(ai) {}
        std::string getName() override { return "aoe marksmanship pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class MarksmanshipHunterAoePvpStrategy : public MarksmanshipHunterAoeStrategy
    {
    public:
        MarksmanshipHunterAoePvpStrategy(PlayerbotAI* ai) : MarksmanshipHunterAoeStrategy(ai) {}
        std::string getName() override { return "aoe marksmanship pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class MarksmanshipHunterAoeRaidStrategy : public MarksmanshipHunterAoeStrategy
    {
    public:
        MarksmanshipHunterAoeRaidStrategy(PlayerbotAI* ai) : MarksmanshipHunterAoeStrategy(ai) {}
        std::string getName() override { return "aoe marksmanship raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class MarksmanshipHunterBuffStrategy : public HunterBuffStrategy
    {
    public:
        MarksmanshipHunterBuffStrategy(PlayerbotAI* ai) : HunterBuffStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class MarksmanshipHunterBuffPveStrategy : public MarksmanshipHunterBuffStrategy
    {
    public:
        MarksmanshipHunterBuffPveStrategy(PlayerbotAI* ai) : MarksmanshipHunterBuffStrategy(ai) {}
        std::string getName() override { return "buff marksmanship pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class MarksmanshipHunterBuffPvpStrategy : public MarksmanshipHunterBuffStrategy
    {
    public:
        MarksmanshipHunterBuffPvpStrategy(PlayerbotAI* ai) : MarksmanshipHunterBuffStrategy(ai) {}
        std::string getName() override { return "buff marksmanship pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class MarksmanshipHunterBuffRaidStrategy : public MarksmanshipHunterBuffStrategy
    {
    public:
        MarksmanshipHunterBuffRaidStrategy(PlayerbotAI* ai) : MarksmanshipHunterBuffStrategy(ai) {}
        std::string getName() override { return "buff marksmanship raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class MarksmanshipHunterBoostStrategy : public HunterBoostStrategy
    {
    public:
        MarksmanshipHunterBoostStrategy(PlayerbotAI* ai) : HunterBoostStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class MarksmanshipHunterBoostPveStrategy : public MarksmanshipHunterBoostStrategy
    {
    public:
        MarksmanshipHunterBoostPveStrategy(PlayerbotAI* ai) : MarksmanshipHunterBoostStrategy(ai) {}
        std::string getName() override { return "boost marksmanship pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class MarksmanshipHunterBoostPvpStrategy : public MarksmanshipHunterBoostStrategy
    {
    public:
        MarksmanshipHunterBoostPvpStrategy(PlayerbotAI* ai) : MarksmanshipHunterBoostStrategy(ai) {}
        std::string getName() override { return "boost marksmanship pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class MarksmanshipHunterBoostRaidStrategy : public MarksmanshipHunterBoostStrategy
    {
    public:
        MarksmanshipHunterBoostRaidStrategy(PlayerbotAI* ai) : MarksmanshipHunterBoostStrategy(ai) {}
        std::string getName() override { return "boost marksmanship raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class MarksmanshipHunterCcStrategy : public HunterCcStrategy
    {
    public:
        MarksmanshipHunterCcStrategy(PlayerbotAI* ai) : HunterCcStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class MarksmanshipHunterCcPveStrategy : public MarksmanshipHunterCcStrategy
    {
    public:
        MarksmanshipHunterCcPveStrategy(PlayerbotAI* ai) : MarksmanshipHunterCcStrategy(ai) {}
        std::string getName() override { return "cc marksmanship pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class MarksmanshipHunterCcPvpStrategy : public MarksmanshipHunterCcStrategy
    {
    public:
        MarksmanshipHunterCcPvpStrategy(PlayerbotAI* ai) : MarksmanshipHunterCcStrategy(ai) {}
        std::string getName() override { return "cc marksmanship pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class MarksmanshipHunterCcRaidStrategy : public MarksmanshipHunterCcStrategy
    {
    public:
        MarksmanshipHunterCcRaidStrategy(PlayerbotAI* ai) : MarksmanshipHunterCcStrategy(ai) {}
        std::string getName() override { return "cc marksmanship raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class MarksmanshipHunterStingStrategy : public HunterStingStrategy
    {
    public:
        MarksmanshipHunterStingStrategy(PlayerbotAI* ai) : HunterStingStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class MarksmanshipHunterStingPveStrategy : public MarksmanshipHunterStingStrategy
    {
    public:
        MarksmanshipHunterStingPveStrategy(PlayerbotAI* ai) : MarksmanshipHunterStingStrategy(ai) {}
        std::string getName() override { return "sting marksmanship pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class MarksmanshipHunterStingPvpStrategy : public MarksmanshipHunterStingStrategy
    {
    public:
        MarksmanshipHunterStingPvpStrategy(PlayerbotAI* ai) : MarksmanshipHunterStingStrategy(ai) {}
        std::string getName() override { return "sting marksmanship pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class MarksmanshipHunterStingRaidStrategy : public MarksmanshipHunterStingStrategy
    {
    public:
        MarksmanshipHunterStingRaidStrategy(PlayerbotAI* ai) : MarksmanshipHunterStingStrategy(ai) {}
        std::string getName() override { return "sting marksmanship raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class MarksmanshipHunterAspectStrategy : public HunterAspectStrategy
    {
    public:
        MarksmanshipHunterAspectStrategy(PlayerbotAI* ai) : HunterAspectStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class MarksmanshipHunterAspectPveStrategy : public MarksmanshipHunterAspectStrategy
    {
    public:
        MarksmanshipHunterAspectPveStrategy(PlayerbotAI* ai) : MarksmanshipHunterAspectStrategy(ai) {}
        std::string getName() override { return "aspect marksmanship pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class MarksmanshipHunterAspectPvpStrategy : public MarksmanshipHunterAspectStrategy
    {
    public:
        MarksmanshipHunterAspectPvpStrategy(PlayerbotAI* ai) : MarksmanshipHunterAspectStrategy(ai) {}
        std::string getName() override { return "aspect marksmanship pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class MarksmanshipHunterAspectRaidStrategy : public MarksmanshipHunterAspectStrategy
    {
    public:
        MarksmanshipHunterAspectRaidStrategy(PlayerbotAI* ai) : MarksmanshipHunterAspectStrategy(ai) {}
        std::string getName() override { return "aspect marksmanship raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };
}