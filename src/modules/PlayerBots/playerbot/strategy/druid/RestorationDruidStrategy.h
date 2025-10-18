#pragma once
#include "DruidStrategy.h"

namespace ai
{
    class RestorationDruidPlaceholderStrategy : public SpecPlaceholderStrategy
    {
    public:
        RestorationDruidPlaceholderStrategy(PlayerbotAI* ai) : SpecPlaceholderStrategy(ai) {}
        int GetType() override { return STRATEGY_TYPE_HEAL | STRATEGY_TYPE_RANGED; }
        std::string getName() override { return "restoration"; }
    };

    class RestorationDruidStrategy : public DruidStrategy
    {
    public:
        RestorationDruidStrategy(PlayerbotAI* ai) : DruidStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class RestorationDruidPveStrategy : public RestorationDruidStrategy
    {
    public:
        RestorationDruidPveStrategy(PlayerbotAI* ai) : RestorationDruidStrategy(ai) {}
        std::string getName() override { return "restoration pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
        void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class RestorationDruidPvpStrategy : public RestorationDruidStrategy
    {
    public:
        RestorationDruidPvpStrategy(PlayerbotAI* ai) : RestorationDruidStrategy(ai) {}
        std::string getName() override { return "restoration pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
        void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class RestorationDruidRaidStrategy : public RestorationDruidStrategy
    {
    public:
        RestorationDruidRaidStrategy(PlayerbotAI* ai) : RestorationDruidStrategy(ai) {}
        std::string getName() override { return "restoration raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
        void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class RestorationDruidAoeStrategy : public DruidAoeStrategy
    {
    public:
        RestorationDruidAoeStrategy(PlayerbotAI* ai) : DruidAoeStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class RestorationDruidAoePveStrategy : public RestorationDruidAoeStrategy
    {
    public:
        RestorationDruidAoePveStrategy(PlayerbotAI* ai) : RestorationDruidAoeStrategy(ai) {}
        std::string getName() override { return "aoe restoration pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class RestorationDruidAoePvpStrategy : public RestorationDruidAoeStrategy
    {
    public:
        RestorationDruidAoePvpStrategy(PlayerbotAI* ai) : RestorationDruidAoeStrategy(ai) {}
        std::string getName() override { return "aoe restoration pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class RestorationDruidAoeRaidStrategy : public RestorationDruidAoeStrategy
    {
    public:
        RestorationDruidAoeRaidStrategy(PlayerbotAI* ai) : RestorationDruidAoeStrategy(ai) {}
        std::string getName() override { return "aoe restoration raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class RestorationDruidBuffStrategy : public DruidBuffStrategy
    {
    public:
        RestorationDruidBuffStrategy(PlayerbotAI* ai) : DruidBuffStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class RestorationDruidBuffPveStrategy : public RestorationDruidBuffStrategy
    {
    public:
        RestorationDruidBuffPveStrategy(PlayerbotAI* ai) : RestorationDruidBuffStrategy(ai) {}
        std::string getName() override { return "buff restoration pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class RestorationDruidBuffPvpStrategy : public RestorationDruidBuffStrategy
    {
    public:
        RestorationDruidBuffPvpStrategy(PlayerbotAI* ai) : RestorationDruidBuffStrategy(ai) {}
        std::string getName() override { return "buff restoration pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class RestorationDruidBuffRaidStrategy : public RestorationDruidBuffStrategy
    {
    public:
        RestorationDruidBuffRaidStrategy(PlayerbotAI* ai) : RestorationDruidBuffStrategy(ai) {}
        std::string getName() override { return "buff restoration raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class RestorationDruidBoostStrategy : public DruidBoostStrategy
    {
    public:
        RestorationDruidBoostStrategy(PlayerbotAI* ai) : DruidBoostStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class RestorationDruidBoostPveStrategy : public RestorationDruidBoostStrategy
    {
    public:
        RestorationDruidBoostPveStrategy(PlayerbotAI* ai) : RestorationDruidBoostStrategy(ai) {}
        std::string getName() override { return "boost restoration pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class RestorationDruidBoostPvpStrategy : public RestorationDruidBoostStrategy
    {
    public:
        RestorationDruidBoostPvpStrategy(PlayerbotAI* ai) : RestorationDruidBoostStrategy(ai) {}
        std::string getName() override { return "boost restoration pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class RestorationDruidBoostRaidStrategy : public RestorationDruidBoostStrategy
    {
    public:
        RestorationDruidBoostRaidStrategy(PlayerbotAI* ai) : RestorationDruidBoostStrategy(ai) {}
        std::string getName() override { return "boost restoration raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class RestorationDruidCcStrategy : public DruidCcStrategy
    {
    public:
        RestorationDruidCcStrategy(PlayerbotAI* ai) : DruidCcStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class RestorationDruidCcPveStrategy : public RestorationDruidCcStrategy
    {
    public:
        RestorationDruidCcPveStrategy(PlayerbotAI* ai) : RestorationDruidCcStrategy(ai) {}
        std::string getName() override { return "cc restoration pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class RestorationDruidCcPvpStrategy : public RestorationDruidCcStrategy
    {
    public:
        RestorationDruidCcPvpStrategy(PlayerbotAI* ai) : RestorationDruidCcStrategy(ai) {}
        std::string getName() override { return "cc restoration pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class RestorationDruidCcRaidStrategy : public RestorationDruidCcStrategy
    {
    public:
        RestorationDruidCcRaidStrategy(PlayerbotAI* ai) : RestorationDruidCcStrategy(ai) {}
        std::string getName() override { return "cc restoration raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class RestorationDruidCureStrategy : public DruidCureStrategy
    {
    public:
        RestorationDruidCureStrategy(PlayerbotAI* ai) : DruidCureStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class RestorationDruidCurePveStrategy : public RestorationDruidCureStrategy
    {
    public:
        RestorationDruidCurePveStrategy(PlayerbotAI* ai) : RestorationDruidCureStrategy(ai) {}
        std::string getName() override { return "cure restoration pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class RestorationDruidCurePvpStrategy : public RestorationDruidCureStrategy
    {
    public:
        RestorationDruidCurePvpStrategy(PlayerbotAI* ai) : RestorationDruidCureStrategy(ai) {}
        std::string getName() override { return "cure restoration pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class RestorationDruidCureRaidStrategy : public RestorationDruidCureStrategy
    {
    public:
        RestorationDruidCureRaidStrategy(PlayerbotAI* ai) : RestorationDruidCureStrategy(ai) {}
        std::string getName() override { return "cure restoration raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };
}
