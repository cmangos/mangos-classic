#pragma once
#include "DruidStrategy.h"

namespace ai
{
    class TankFeralDruidPlaceholderStrategy : public SpecPlaceholderStrategy
    {
    public:
        TankFeralDruidPlaceholderStrategy(PlayerbotAI* ai) : SpecPlaceholderStrategy(ai) {}
        int GetType() override { return STRATEGY_TYPE_TANK | STRATEGY_TYPE_MELEE; }
        std::string getName() override { return "tank feral"; }
    };

    class TankFeralDruidStrategy : public DruidStrategy
    {
    public:
        TankFeralDruidStrategy(PlayerbotAI* ai);

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
    
        virtual NextAction** GetDefaultCombatActions() override;
    };

    class TankFeralDruidPveStrategy : public TankFeralDruidStrategy
    {
    public:
        TankFeralDruidPveStrategy(PlayerbotAI* ai) : TankFeralDruidStrategy(ai) {}
        std::string getName() override { return "tank feral pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
        void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class TankFeralDruidPvpStrategy : public TankFeralDruidStrategy
    {
    public:
        TankFeralDruidPvpStrategy(PlayerbotAI* ai) : TankFeralDruidStrategy(ai) {}
        std::string getName() override { return "tank feral pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
        void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class TankFeralDruidRaidStrategy : public TankFeralDruidStrategy
    {
    public:
        TankFeralDruidRaidStrategy(PlayerbotAI* ai) : TankFeralDruidStrategy(ai) {}
        std::string getName() override { return "tank feral raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
        void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class TankFeralDruidAoeStrategy : public DruidAoeStrategy
    {
    public:
        TankFeralDruidAoeStrategy(PlayerbotAI* ai) : DruidAoeStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class TankFeralDruidAoePveStrategy : public TankFeralDruidAoeStrategy
    {
    public:
        TankFeralDruidAoePveStrategy(PlayerbotAI* ai) : TankFeralDruidAoeStrategy(ai) {}
        std::string getName() override { return "aoe tank feral pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class TankFeralDruidAoePvpStrategy : public TankFeralDruidAoeStrategy
    {
    public:
        TankFeralDruidAoePvpStrategy(PlayerbotAI* ai) : TankFeralDruidAoeStrategy(ai) {}
        std::string getName() override { return "aoe tank feral pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class TankFeralDruidAoeRaidStrategy : public TankFeralDruidAoeStrategy
    {
    public:
        TankFeralDruidAoeRaidStrategy(PlayerbotAI* ai) : TankFeralDruidAoeStrategy(ai) {}
        std::string getName() override { return "aoe tank feral raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class TankFeralDruidBuffStrategy : public DruidBuffStrategy
    {
    public:
        TankFeralDruidBuffStrategy(PlayerbotAI* ai) : DruidBuffStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class TankFeralDruidBuffPveStrategy : public TankFeralDruidBuffStrategy
    {
    public:
        TankFeralDruidBuffPveStrategy(PlayerbotAI* ai) : TankFeralDruidBuffStrategy(ai) {}
        std::string getName() override { return "buff tank feral pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class TankFeralDruidBuffPvpStrategy : public TankFeralDruidBuffStrategy
    {
    public:
        TankFeralDruidBuffPvpStrategy(PlayerbotAI* ai) : TankFeralDruidBuffStrategy(ai) {}
        std::string getName() override { return "buff tank feral pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class TankFeralDruidBuffRaidStrategy : public TankFeralDruidBuffStrategy
    {
    public:
        TankFeralDruidBuffRaidStrategy(PlayerbotAI* ai) : TankFeralDruidBuffStrategy(ai) {}
        std::string getName() override { return "buff tank feral raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class TankFeralDruidBoostStrategy : public DruidBoostStrategy
    {
    public:
        TankFeralDruidBoostStrategy(PlayerbotAI* ai) : DruidBoostStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class TankFeralDruidBoostPveStrategy : public TankFeralDruidBoostStrategy
    {
    public:
        TankFeralDruidBoostPveStrategy(PlayerbotAI* ai) : TankFeralDruidBoostStrategy(ai) {}
        std::string getName() override { return "boost tank feral pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class TankFeralDruidBoostPvpStrategy : public TankFeralDruidBoostStrategy
    {
    public:
        TankFeralDruidBoostPvpStrategy(PlayerbotAI* ai) : TankFeralDruidBoostStrategy(ai) {}
        std::string getName() override { return "boost tank feral pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class TankFeralDruidBoostRaidStrategy : public TankFeralDruidBoostStrategy
    {
    public:
        TankFeralDruidBoostRaidStrategy(PlayerbotAI* ai) : TankFeralDruidBoostStrategy(ai) {}
        std::string getName() override { return "boost tank feral raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class TankFeralDruidCcStrategy : public DruidCcStrategy
    {
    public:
        TankFeralDruidCcStrategy(PlayerbotAI* ai) : DruidCcStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class TankFeralDruidCcPveStrategy : public TankFeralDruidCcStrategy
    {
    public:
        TankFeralDruidCcPveStrategy(PlayerbotAI* ai) : TankFeralDruidCcStrategy(ai) {}
        std::string getName() override { return "cc tank feral pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class TankFeralDruidCcPvpStrategy : public TankFeralDruidCcStrategy
    {
    public:
        TankFeralDruidCcPvpStrategy(PlayerbotAI* ai) : TankFeralDruidCcStrategy(ai) {}
        std::string getName() override { return "cc tank feral pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class TankFeralDruidCcRaidStrategy : public TankFeralDruidCcStrategy
    {
    public:
        TankFeralDruidCcRaidStrategy(PlayerbotAI* ai) : TankFeralDruidCcStrategy(ai) {}
        std::string getName() override { return "cc tank feral raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class TankFeralDruidCureStrategy : public DruidCureStrategy
    {
    public:
        TankFeralDruidCureStrategy(PlayerbotAI* ai) : DruidCureStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class TankFeralDruidCurePveStrategy : public TankFeralDruidCureStrategy
    {
    public:
        TankFeralDruidCurePveStrategy(PlayerbotAI* ai) : TankFeralDruidCureStrategy(ai) {}
        std::string getName() override { return "cure tank feral pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class TankFeralDruidCurePvpStrategy : public TankFeralDruidCureStrategy
    {
    public:
        TankFeralDruidCurePvpStrategy(PlayerbotAI* ai) : TankFeralDruidCureStrategy(ai) {}
        std::string getName() override { return "cure tank feral pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class TankFeralDruidCureRaidStrategy : public TankFeralDruidCureStrategy
    {
    public:
        TankFeralDruidCureRaidStrategy(PlayerbotAI* ai) : TankFeralDruidCureStrategy(ai) {}
        std::string getName() override { return "cure tank feral raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };
}
