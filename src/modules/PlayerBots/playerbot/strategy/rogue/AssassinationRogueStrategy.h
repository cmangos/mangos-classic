#pragma once
#include "RogueStrategy.h"

namespace ai
{
    class AssassinationRoguePlaceholderStrategy : public SpecPlaceholderStrategy
    {
    public:
        AssassinationRoguePlaceholderStrategy(PlayerbotAI* ai) : SpecPlaceholderStrategy(ai) {}
        int GetType() override { return STRATEGY_TYPE_DPS | STRATEGY_TYPE_MELEE; }
        std::string getName() override { return "assassination"; }
    };

    class AssassinationRogueStrategy : public RogueStrategy
    {
    public:
        AssassinationRogueStrategy(PlayerbotAI* ai);

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class AssassinationRoguePveStrategy : public AssassinationRogueStrategy
    {
    public:
        AssassinationRoguePveStrategy(PlayerbotAI* ai) : AssassinationRogueStrategy(ai) {}
        std::string getName() override { return "assassination pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
        void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class AssassinationRoguePvpStrategy : public AssassinationRogueStrategy
    {
    public:
        AssassinationRoguePvpStrategy(PlayerbotAI* ai) : AssassinationRogueStrategy(ai) {}
        std::string getName() override { return "assassination pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
        void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class AssassinationRogueRaidStrategy : public AssassinationRogueStrategy
    {
    public:
        AssassinationRogueRaidStrategy(PlayerbotAI* ai) : AssassinationRogueStrategy(ai) {}
        std::string getName() override { return "assassination raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
        void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class AssassinationRogueAoeStrategy : public RogueAoeStrategy
    {
    public:
        AssassinationRogueAoeStrategy(PlayerbotAI* ai) : RogueAoeStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class AssassinationRogueAoePveStrategy : public AssassinationRogueAoeStrategy
    {
    public:
        AssassinationRogueAoePveStrategy(PlayerbotAI* ai) : AssassinationRogueAoeStrategy(ai) {}
        std::string getName() override { return "aoe assassination pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class AssassinationRogueAoePvpStrategy : public AssassinationRogueAoeStrategy
    {
    public:
        AssassinationRogueAoePvpStrategy(PlayerbotAI* ai) : AssassinationRogueAoeStrategy(ai) {}
        std::string getName() override { return "aoe assassination pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class AssassinationRogueAoeRaidStrategy : public AssassinationRogueAoeStrategy
    {
    public:
        AssassinationRogueAoeRaidStrategy(PlayerbotAI* ai) : AssassinationRogueAoeStrategy(ai) {}
        std::string getName() override { return "aoe assassination raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class AssassinationRogueBuffStrategy : public RogueBuffStrategy
    {
    public:
        AssassinationRogueBuffStrategy(PlayerbotAI* ai) : RogueBuffStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class AssassinationRogueBuffPveStrategy : public AssassinationRogueBuffStrategy
    {
    public:
        AssassinationRogueBuffPveStrategy(PlayerbotAI* ai) : AssassinationRogueBuffStrategy(ai) {}
        std::string getName() override { return "buff assassination pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class AssassinationRogueBuffPvpStrategy : public AssassinationRogueBuffStrategy
    {
    public:
        AssassinationRogueBuffPvpStrategy(PlayerbotAI* ai) : AssassinationRogueBuffStrategy(ai) {}
        std::string getName() override { return "buff assassination pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class AssassinationRogueBuffRaidStrategy : public AssassinationRogueBuffStrategy
    {
    public:
        AssassinationRogueBuffRaidStrategy(PlayerbotAI* ai) : AssassinationRogueBuffStrategy(ai) {}
        std::string getName() override { return "buff assassination raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class AssassinationRogueBoostStrategy : public RogueBoostStrategy
    {
    public:
        AssassinationRogueBoostStrategy(PlayerbotAI* ai) : RogueBoostStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class AssassinationRogueBoostPveStrategy : public AssassinationRogueBoostStrategy
    {
    public:
        AssassinationRogueBoostPveStrategy(PlayerbotAI* ai) : AssassinationRogueBoostStrategy(ai) {}
        std::string getName() override { return "boost assassination pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class AssassinationRogueBoostPvpStrategy : public AssassinationRogueBoostStrategy
    {
    public:
        AssassinationRogueBoostPvpStrategy(PlayerbotAI* ai) : AssassinationRogueBoostStrategy(ai) {}
        std::string getName() override { return "boost assassination pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class AssassinationRogueBoostRaidStrategy : public AssassinationRogueBoostStrategy
    {
    public:
        AssassinationRogueBoostRaidStrategy(PlayerbotAI* ai) : AssassinationRogueBoostStrategy(ai) {}
        std::string getName() override { return "boost assassination raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class AssassinationRogueCcStrategy : public RogueCcStrategy
    {
    public:
        AssassinationRogueCcStrategy(PlayerbotAI* ai) : RogueCcStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class AssassinationRogueCcPveStrategy : public AssassinationRogueCcStrategy
    {
    public:
        AssassinationRogueCcPveStrategy(PlayerbotAI* ai) : AssassinationRogueCcStrategy(ai) {}
        std::string getName() override { return "cc assassination pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class AssassinationRogueCcPvpStrategy : public AssassinationRogueCcStrategy
    {
    public:
        AssassinationRogueCcPvpStrategy(PlayerbotAI* ai) : AssassinationRogueCcStrategy(ai) {}
        std::string getName() override { return "cc assassination pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class AssassinationRogueCcRaidStrategy : public AssassinationRogueCcStrategy
    {
    public:
        AssassinationRogueCcRaidStrategy(PlayerbotAI* ai) : AssassinationRogueCcStrategy(ai) {}
        std::string getName() override { return "cc assassination raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class AssassinationRogueStealthStrategy : public RogueStealthStrategy
    {
    public:
        AssassinationRogueStealthStrategy(PlayerbotAI* ai) : RogueStealthStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class AssassinationRogueStealthPveStrategy : public AssassinationRogueStealthStrategy
    {
    public:
        AssassinationRogueStealthPveStrategy(PlayerbotAI* ai) : AssassinationRogueStealthStrategy(ai) {}
        std::string getName() override { return "stealth assassination pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class AssassinationRogueStealthPvpStrategy : public AssassinationRogueStealthStrategy
    {
    public:
        AssassinationRogueStealthPvpStrategy(PlayerbotAI* ai) : AssassinationRogueStealthStrategy(ai) {}
        std::string getName() override { return "stealth assassination pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class AssassinationRogueStealthRaidStrategy : public AssassinationRogueStealthStrategy
    {
    public:
        AssassinationRogueStealthRaidStrategy(PlayerbotAI* ai) : AssassinationRogueStealthStrategy(ai) {}
        std::string getName() override { return "stealth assassination raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class AssassinationRoguePoisonsStrategy : public RoguePoisonsStrategy
    {
    public:
        AssassinationRoguePoisonsStrategy(PlayerbotAI* ai) : RoguePoisonsStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class AssassinationRoguePoisonsPveStrategy : public AssassinationRoguePoisonsStrategy
    {
    public:
        AssassinationRoguePoisonsPveStrategy(PlayerbotAI* ai) : AssassinationRoguePoisonsStrategy(ai) {}
        std::string getName() override { return "poisons assassination pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class AssassinationRoguePoisonsPvpStrategy : public AssassinationRoguePoisonsStrategy
    {
    public:
        AssassinationRoguePoisonsPvpStrategy(PlayerbotAI* ai) : AssassinationRoguePoisonsStrategy(ai) {}
        std::string getName() override { return "poisons assassination pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class AssassinationRoguePoisonsRaidStrategy : public AssassinationRoguePoisonsStrategy
    {
    public:
        AssassinationRoguePoisonsRaidStrategy(PlayerbotAI* ai) : AssassinationRoguePoisonsStrategy(ai) {}
        std::string getName() override { return "poisons assassination raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };
}