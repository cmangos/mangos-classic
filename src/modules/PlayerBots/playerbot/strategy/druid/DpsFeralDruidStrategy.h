#pragma once
#include "DruidStrategy.h"

namespace ai
{
    class DpsFeralDruidPlaceholderStrategy : public SpecPlaceholderStrategy
    {
    public:
        DpsFeralDruidPlaceholderStrategy(PlayerbotAI* ai) : SpecPlaceholderStrategy(ai) {}
        int GetType() override { return STRATEGY_TYPE_DPS | STRATEGY_TYPE_MELEE; }
        std::string getName() override { return "dps feral"; }
    };

    class DpsFeralDruidStrategy : public DruidStrategy
    {
    public:
        DpsFeralDruidStrategy(PlayerbotAI* ai);

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
    
        virtual NextAction** GetDefaultCombatActions() override;
    };

    class DpsFeralDruidPveStrategy : public DpsFeralDruidStrategy
    {
    public:
        DpsFeralDruidPveStrategy(PlayerbotAI* ai) : DpsFeralDruidStrategy(ai) {}
        std::string getName() override { return "dps feral pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
        void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class DpsFeralDruidPvpStrategy : public DpsFeralDruidStrategy
    {
    public:
        DpsFeralDruidPvpStrategy(PlayerbotAI* ai) : DpsFeralDruidStrategy(ai) {}
        std::string getName() override { return "dps feral pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
        void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class DpsFeralDruidRaidStrategy : public DpsFeralDruidStrategy
    {
    public:
        DpsFeralDruidRaidStrategy(PlayerbotAI* ai) : DpsFeralDruidStrategy(ai) {}
        std::string getName() override { return "dps feral raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
        void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class DpsFeralDruidAoeStrategy : public DruidAoeStrategy
    {
    public:
        DpsFeralDruidAoeStrategy(PlayerbotAI* ai) : DruidAoeStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class DpsFeralDruidAoePveStrategy : public DpsFeralDruidAoeStrategy
    {
    public:
        DpsFeralDruidAoePveStrategy(PlayerbotAI* ai) : DpsFeralDruidAoeStrategy(ai) {}
        std::string getName() override { return "aoe dps feral pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class DpsFeralDruidAoePvpStrategy : public DpsFeralDruidAoeStrategy
    {
    public:
        DpsFeralDruidAoePvpStrategy(PlayerbotAI* ai) : DpsFeralDruidAoeStrategy(ai) {}
        std::string getName() override { return "aoe dps feral pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class DpsFeralDruidAoeRaidStrategy : public DpsFeralDruidAoeStrategy
    {
    public:
        DpsFeralDruidAoeRaidStrategy(PlayerbotAI* ai) : DpsFeralDruidAoeStrategy(ai) {}
        std::string getName() override { return "aoe dps feral raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class DpsFeralDruidBuffStrategy : public DruidBuffStrategy
    {
    public:
        DpsFeralDruidBuffStrategy(PlayerbotAI* ai) : DruidBuffStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class DpsFeralDruidBuffPveStrategy : public DpsFeralDruidBuffStrategy
    {
    public:
        DpsFeralDruidBuffPveStrategy(PlayerbotAI* ai) : DpsFeralDruidBuffStrategy(ai) {}
        std::string getName() override { return "buff dps feral pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class DpsFeralDruidBuffPvpStrategy : public DpsFeralDruidBuffStrategy
    {
    public:
        DpsFeralDruidBuffPvpStrategy(PlayerbotAI* ai) : DpsFeralDruidBuffStrategy(ai) {}
        std::string getName() override { return "buff dps feral pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class DpsFeralDruidBuffRaidStrategy : public DpsFeralDruidBuffStrategy
    {
    public:
        DpsFeralDruidBuffRaidStrategy(PlayerbotAI* ai) : DpsFeralDruidBuffStrategy(ai) {}
        std::string getName() override { return "buff dps feral raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class DpsFeralDruidBoostStrategy : public DruidBoostStrategy
    {
    public:
        DpsFeralDruidBoostStrategy(PlayerbotAI* ai) : DruidBoostStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class DpsFeralDruidBoostPveStrategy : public DpsFeralDruidBoostStrategy
    {
    public:
        DpsFeralDruidBoostPveStrategy(PlayerbotAI* ai) : DpsFeralDruidBoostStrategy(ai) {}
        std::string getName() override { return "boost dps feral pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class DpsFeralDruidBoostPvpStrategy : public DpsFeralDruidBoostStrategy
    {
    public:
        DpsFeralDruidBoostPvpStrategy(PlayerbotAI* ai) : DpsFeralDruidBoostStrategy(ai) {}
        std::string getName() override { return "boost dps feral pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class DpsFeralDruidBoostRaidStrategy : public DpsFeralDruidBoostStrategy
    {
    public:
        DpsFeralDruidBoostRaidStrategy(PlayerbotAI* ai) : DpsFeralDruidBoostStrategy(ai) {}
        std::string getName() override { return "boost dps feral raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class DpsFeralDruidCcStrategy : public DruidCcStrategy
    {
    public:
        DpsFeralDruidCcStrategy(PlayerbotAI* ai) : DruidCcStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class DpsFeralDruidCcPveStrategy : public DpsFeralDruidCcStrategy
    {
    public:
        DpsFeralDruidCcPveStrategy(PlayerbotAI* ai) : DpsFeralDruidCcStrategy(ai) {}
        std::string getName() override { return "cc dps feral pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class DpsFeralDruidCcPvpStrategy : public DpsFeralDruidCcStrategy
    {
    public:
        DpsFeralDruidCcPvpStrategy(PlayerbotAI* ai) : DpsFeralDruidCcStrategy(ai) {}
        std::string getName() override { return "cc dps feral pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class DpsFeralDruidCcRaidStrategy : public DpsFeralDruidCcStrategy
    {
    public:
        DpsFeralDruidCcRaidStrategy(PlayerbotAI* ai) : DpsFeralDruidCcStrategy(ai) {}
        std::string getName() override { return "cc dps feral raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class DpsFeralDruidCureStrategy : public DruidCureStrategy
    {
    public:
        DpsFeralDruidCureStrategy(PlayerbotAI* ai) : DruidCureStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class DpsFeralDruidCurePveStrategy : public DpsFeralDruidCureStrategy
    {
    public:
        DpsFeralDruidCurePveStrategy(PlayerbotAI* ai) : DpsFeralDruidCureStrategy(ai) {}
        std::string getName() override { return "cure dps feral pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class DpsFeralDruidCurePvpStrategy : public DpsFeralDruidCureStrategy
    {
    public:
        DpsFeralDruidCurePvpStrategy(PlayerbotAI* ai) : DpsFeralDruidCureStrategy(ai) {}
        std::string getName() override { return "cure dps feral pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class DpsFeralDruidCureRaidStrategy : public DpsFeralDruidCureStrategy
    {
    public:
        DpsFeralDruidCureRaidStrategy(PlayerbotAI* ai) : DpsFeralDruidCureStrategy(ai) {}
        std::string getName() override { return "cure dps feral raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class DpsFeralDruidStealthStrategy : public StealthStrategy
    {
    public:
        DpsFeralDruidStealthStrategy(PlayerbotAI* ai);

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class DpsFeralDruidStealthPvpStrategy : public DpsFeralDruidStealthStrategy
    {
    public:
        DpsFeralDruidStealthPvpStrategy(PlayerbotAI* ai) : DpsFeralDruidStealthStrategy(ai) {}
        std::string getName() override { return "stealth dps feral pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class DpsFeralDruidStealthPveStrategy : public DpsFeralDruidStealthStrategy
    {
    public:
        DpsFeralDruidStealthPveStrategy(PlayerbotAI* ai) : DpsFeralDruidStealthStrategy(ai) {}
        std::string getName() override { return "stealth dps feral pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class DpsFeralDruidStealthRaidStrategy : public DpsFeralDruidStealthStrategy
    {
    public:
        DpsFeralDruidStealthRaidStrategy(PlayerbotAI* ai) : DpsFeralDruidStealthStrategy(ai) {}
        std::string getName() override { return "stealth dps feral raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class DpsFeralDruidStealthedStrategy : public StealthedStrategy
    {
    public:
        DpsFeralDruidStealthedStrategy(PlayerbotAI* ai) : StealthedStrategy(ai) {}

    private:
        void InitCombatMultipliers(std::list<Multiplier*>& multipliers) override;
        void InitNonCombatMultipliers(std::list<Multiplier*>& multipliers) override;
    };

    class DpsFeralDruidPowershiftStrategy : public Strategy
    {
    public:
        DpsFeralDruidPowershiftStrategy(PlayerbotAI* ai) : Strategy(ai) {}
        std::string getName() override { return "powershift"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };
}