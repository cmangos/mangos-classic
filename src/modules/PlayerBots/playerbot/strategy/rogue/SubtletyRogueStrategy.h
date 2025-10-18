#pragma once
#include "RogueStrategy.h"

namespace ai
{
    class SubtletyRoguePlaceholderStrategy : public SpecPlaceholderStrategy
    {
    public:
        SubtletyRoguePlaceholderStrategy(PlayerbotAI* ai) : SpecPlaceholderStrategy(ai) {}
        int GetType() override { return STRATEGY_TYPE_DPS | STRATEGY_TYPE_MELEE; }
        std::string getName() override { return "subtlety"; }
    };

    class SubtletyRogueStrategy : public RogueStrategy
    {
    public:
        SubtletyRogueStrategy(PlayerbotAI* ai);

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class SubtletyRoguePveStrategy : public SubtletyRogueStrategy
    {
    public:
        SubtletyRoguePveStrategy(PlayerbotAI* ai) : SubtletyRogueStrategy(ai) {}
        std::string getName() override { return "subtlety pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
        void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class SubtletyRoguePvpStrategy : public SubtletyRogueStrategy
    {
    public:
        SubtletyRoguePvpStrategy(PlayerbotAI* ai) : SubtletyRogueStrategy(ai) {}
        std::string getName() override { return "subtlety pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
        void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class SubtletyRogueRaidStrategy : public SubtletyRogueStrategy
    {
    public:
        SubtletyRogueRaidStrategy(PlayerbotAI* ai) : SubtletyRogueStrategy(ai) {}
        std::string getName() override { return "subtlety raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
        void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class SubtletyRogueAoeStrategy : public RogueAoeStrategy
    {
    public:
        SubtletyRogueAoeStrategy(PlayerbotAI* ai) : RogueAoeStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class SubtletyRogueAoePveStrategy : public SubtletyRogueAoeStrategy
    {
    public:
        SubtletyRogueAoePveStrategy(PlayerbotAI* ai) : SubtletyRogueAoeStrategy(ai) {}
        std::string getName() override { return "aoe subtlety pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class SubtletyRogueAoePvpStrategy : public SubtletyRogueAoeStrategy
    {
    public:
        SubtletyRogueAoePvpStrategy(PlayerbotAI* ai) : SubtletyRogueAoeStrategy(ai) {}
        std::string getName() override { return "aoe subtlety pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class SubtletyRogueAoeRaidStrategy : public SubtletyRogueAoeStrategy
    {
    public:
        SubtletyRogueAoeRaidStrategy(PlayerbotAI* ai) : SubtletyRogueAoeStrategy(ai) {}
        std::string getName() override { return "aoe subtlety raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class SubtletyRogueBuffStrategy : public RogueBuffStrategy
    {
    public:
        SubtletyRogueBuffStrategy(PlayerbotAI* ai) : RogueBuffStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class SubtletyRogueBuffPveStrategy : public SubtletyRogueBuffStrategy
    {
    public:
        SubtletyRogueBuffPveStrategy(PlayerbotAI* ai) : SubtletyRogueBuffStrategy(ai) {}
        std::string getName() override { return "buff subtlety pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class SubtletyRogueBuffPvpStrategy : public SubtletyRogueBuffStrategy
    {
    public:
        SubtletyRogueBuffPvpStrategy(PlayerbotAI* ai) : SubtletyRogueBuffStrategy(ai) {}
        std::string getName() override { return "buff subtlety pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class SubtletyRogueBuffRaidStrategy : public SubtletyRogueBuffStrategy
    {
    public:
        SubtletyRogueBuffRaidStrategy(PlayerbotAI* ai) : SubtletyRogueBuffStrategy(ai) {}
        std::string getName() override { return "buff subtlety raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class SubtletyRogueBoostStrategy : public RogueBoostStrategy
    {
    public:
        SubtletyRogueBoostStrategy(PlayerbotAI* ai) : RogueBoostStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class SubtletyRogueBoostPveStrategy : public SubtletyRogueBoostStrategy
    {
    public:
        SubtletyRogueBoostPveStrategy(PlayerbotAI* ai) : SubtletyRogueBoostStrategy(ai) {}
        std::string getName() override { return "boost subtlety pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class SubtletyRogueBoostPvpStrategy : public SubtletyRogueBoostStrategy
    {
    public:
        SubtletyRogueBoostPvpStrategy(PlayerbotAI* ai) : SubtletyRogueBoostStrategy(ai) {}
        std::string getName() override { return "boost subtlety pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class SubtletyRogueBoostRaidStrategy : public SubtletyRogueBoostStrategy
    {
    public:
        SubtletyRogueBoostRaidStrategy(PlayerbotAI* ai) : SubtletyRogueBoostStrategy(ai) {}
        std::string getName() override { return "boost subtlety raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class SubtletyRogueCcStrategy : public RogueCcStrategy
    {
    public:
        SubtletyRogueCcStrategy(PlayerbotAI* ai) : RogueCcStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class SubtletyRogueCcPveStrategy : public SubtletyRogueCcStrategy
    {
    public:
        SubtletyRogueCcPveStrategy(PlayerbotAI* ai) : SubtletyRogueCcStrategy(ai) {}
        std::string getName() override { return "cc subtlety pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class SubtletyRogueCcPvpStrategy : public SubtletyRogueCcStrategy
    {
    public:
        SubtletyRogueCcPvpStrategy(PlayerbotAI* ai) : SubtletyRogueCcStrategy(ai) {}
        std::string getName() override { return "cc subtlety pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class SubtletyRogueCcRaidStrategy : public SubtletyRogueCcStrategy
    {
    public:
        SubtletyRogueCcRaidStrategy(PlayerbotAI* ai) : SubtletyRogueCcStrategy(ai) {}
        std::string getName() override { return "cc subtlety raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class SubtletyRogueStealthStrategy : public RogueStealthStrategy
    {
    public:
        SubtletyRogueStealthStrategy(PlayerbotAI* ai) : RogueStealthStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class SubtletyRogueStealthPveStrategy : public SubtletyRogueStealthStrategy
    {
    public:
        SubtletyRogueStealthPveStrategy(PlayerbotAI* ai) : SubtletyRogueStealthStrategy(ai) {}
        std::string getName() override { return "stealth subtlety pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class SubtletyRogueStealthPvpStrategy : public SubtletyRogueStealthStrategy
    {
    public:
        SubtletyRogueStealthPvpStrategy(PlayerbotAI* ai) : SubtletyRogueStealthStrategy(ai) {}
        std::string getName() override { return "stealth subtlety pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class SubtletyRogueStealthRaidStrategy : public SubtletyRogueStealthStrategy
    {
    public:
        SubtletyRogueStealthRaidStrategy(PlayerbotAI* ai) : SubtletyRogueStealthStrategy(ai) {}
        std::string getName() override { return "stealth subtlety raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class SubtletyRoguePoisonsStrategy : public RoguePoisonsStrategy
    {
    public:
        SubtletyRoguePoisonsStrategy(PlayerbotAI* ai) : RoguePoisonsStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class SubtletyRoguePoisonsPveStrategy : public SubtletyRoguePoisonsStrategy
    {
    public:
        SubtletyRoguePoisonsPveStrategy(PlayerbotAI* ai) : SubtletyRoguePoisonsStrategy(ai) {}
        std::string getName() override { return "poisons subtlety pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class SubtletyRoguePoisonsPvpStrategy : public SubtletyRoguePoisonsStrategy
    {
    public:
        SubtletyRoguePoisonsPvpStrategy(PlayerbotAI* ai) : SubtletyRoguePoisonsStrategy(ai) {}
        std::string getName() override { return "poisons subtlety pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class SubtletyRoguePoisonsRaidStrategy : public SubtletyRoguePoisonsStrategy
    {
    public:
        SubtletyRoguePoisonsRaidStrategy(PlayerbotAI* ai) : SubtletyRoguePoisonsStrategy(ai) {}
        std::string getName() override { return "poisons subtlety raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };
}