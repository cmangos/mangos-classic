#pragma once
#include "RogueStrategy.h"

namespace ai
{
    class CombatRoguePlaceholderStrategy : public SpecPlaceholderStrategy
    {
    public:
        CombatRoguePlaceholderStrategy(PlayerbotAI* ai) : SpecPlaceholderStrategy(ai) {}
        int GetType() override { return STRATEGY_TYPE_DPS | STRATEGY_TYPE_MELEE; }
        std::string getName() override { return "combat"; }
    };

    class CombatRogueStrategy : public RogueStrategy
    {
    public:
        CombatRogueStrategy(PlayerbotAI* ai) : RogueStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class CombatRoguePveStrategy : public CombatRogueStrategy
    {
    public:
        CombatRoguePveStrategy(PlayerbotAI* ai) : CombatRogueStrategy(ai) {}
        std::string getName() override { return "combat pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
        void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class CombatRoguePvpStrategy : public CombatRogueStrategy
    {
    public:
        CombatRoguePvpStrategy(PlayerbotAI* ai) : CombatRogueStrategy(ai) {}
        std::string getName() override { return "combat pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
        void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class CombatRogueRaidStrategy : public CombatRogueStrategy
    {
    public:
        CombatRogueRaidStrategy(PlayerbotAI* ai) : CombatRogueStrategy(ai) {}
        std::string getName() override { return "combat raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
        void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class CombatRogueAoeStrategy : public RogueAoeStrategy
    {
    public:
        CombatRogueAoeStrategy(PlayerbotAI* ai) : RogueAoeStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class CombatRogueAoePveStrategy : public CombatRogueAoeStrategy
    {
    public:
        CombatRogueAoePveStrategy(PlayerbotAI* ai) : CombatRogueAoeStrategy(ai) {}
        std::string getName() override { return "aoe combat pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class CombatRogueAoePvpStrategy : public CombatRogueAoeStrategy
    {
    public:
        CombatRogueAoePvpStrategy(PlayerbotAI* ai) : CombatRogueAoeStrategy(ai) {}
        std::string getName() override { return "aoe combat pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class CombatRogueAoeRaidStrategy : public CombatRogueAoeStrategy
    {
    public:
        CombatRogueAoeRaidStrategy(PlayerbotAI* ai) : CombatRogueAoeStrategy(ai) {}
        std::string getName() override { return "aoe combat raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class CombatRogueBuffStrategy : public RogueBuffStrategy
    {
    public:
        CombatRogueBuffStrategy(PlayerbotAI* ai) : RogueBuffStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class CombatRogueBuffPveStrategy : public CombatRogueBuffStrategy
    {
    public:
        CombatRogueBuffPveStrategy(PlayerbotAI* ai) : CombatRogueBuffStrategy(ai) {}
        std::string getName() override { return "buff combat pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class CombatRogueBuffPvpStrategy : public CombatRogueBuffStrategy
    {
    public:
        CombatRogueBuffPvpStrategy(PlayerbotAI* ai) : CombatRogueBuffStrategy(ai) {}
        std::string getName() override { return "buff combat pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class CombatRogueBuffRaidStrategy : public CombatRogueBuffStrategy
    {
    public:
        CombatRogueBuffRaidStrategy(PlayerbotAI* ai) : CombatRogueBuffStrategy(ai) {}
        std::string getName() override { return "buff combat raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class CombatRogueBoostStrategy : public RogueBoostStrategy
    {
    public:
        CombatRogueBoostStrategy(PlayerbotAI* ai) : RogueBoostStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class CombatRogueBoostPveStrategy : public CombatRogueBoostStrategy
    {
    public:
        CombatRogueBoostPveStrategy(PlayerbotAI* ai) : CombatRogueBoostStrategy(ai) {}
        std::string getName() override { return "boost combat pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class CombatRogueBoostPvpStrategy : public CombatRogueBoostStrategy
    {
    public:
        CombatRogueBoostPvpStrategy(PlayerbotAI* ai) : CombatRogueBoostStrategy(ai) {}
        std::string getName() override { return "boost combat pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class CombatRogueBoostRaidStrategy : public CombatRogueBoostStrategy
    {
    public:
        CombatRogueBoostRaidStrategy(PlayerbotAI* ai) : CombatRogueBoostStrategy(ai) {}
        std::string getName() override { return "boost combat raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class CombatRogueCcStrategy : public RogueCcStrategy
    {
    public:
        CombatRogueCcStrategy(PlayerbotAI* ai) : RogueCcStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class CombatRogueCcPveStrategy : public CombatRogueCcStrategy
    {
    public:
        CombatRogueCcPveStrategy(PlayerbotAI* ai) : CombatRogueCcStrategy(ai) {}
        std::string getName() override { return "cc combat pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class CombatRogueCcPvpStrategy : public CombatRogueCcStrategy
    {
    public:
        CombatRogueCcPvpStrategy(PlayerbotAI* ai) : CombatRogueCcStrategy(ai) {}
        std::string getName() override { return "cc combat pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class CombatRogueCcRaidStrategy : public CombatRogueCcStrategy
    {
    public:
        CombatRogueCcRaidStrategy(PlayerbotAI* ai) : CombatRogueCcStrategy(ai) {}
        std::string getName() override { return "cc combat raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class CombatRogueStealthStrategy : public RogueStealthStrategy
    {
    public:
        CombatRogueStealthStrategy(PlayerbotAI* ai) : RogueStealthStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class CombatRogueStealthPveStrategy : public CombatRogueStealthStrategy
    {
    public:
        CombatRogueStealthPveStrategy(PlayerbotAI* ai) : CombatRogueStealthStrategy(ai) {}
        std::string getName() override { return "stealth combat pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class CombatRogueStealthPvpStrategy : public CombatRogueStealthStrategy
    {
    public:
        CombatRogueStealthPvpStrategy(PlayerbotAI* ai) : CombatRogueStealthStrategy(ai) {}
        std::string getName() override { return "stealth combat pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class CombatRogueStealthRaidStrategy : public CombatRogueStealthStrategy
    {
    public:
        CombatRogueStealthRaidStrategy(PlayerbotAI* ai) : CombatRogueStealthStrategy(ai) {}
        std::string getName() override { return "stealth combat raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class CombatRoguePoisonsStrategy : public RoguePoisonsStrategy
    {
    public:
        CombatRoguePoisonsStrategy(PlayerbotAI* ai) : RoguePoisonsStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class CombatRoguePoisonsPveStrategy : public CombatRoguePoisonsStrategy
    {
    public:
        CombatRoguePoisonsPveStrategy(PlayerbotAI* ai) : CombatRoguePoisonsStrategy(ai) {}
        std::string getName() override { return "poisons combat pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class CombatRoguePoisonsPvpStrategy : public CombatRoguePoisonsStrategy
    {
    public:
        CombatRoguePoisonsPvpStrategy(PlayerbotAI* ai) : CombatRoguePoisonsStrategy(ai) {}
        std::string getName() override { return "poisons combat pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class CombatRoguePoisonsRaidStrategy : public CombatRoguePoisonsStrategy
    {
    public:
        CombatRoguePoisonsRaidStrategy(PlayerbotAI* ai) : CombatRoguePoisonsStrategy(ai) {}
        std::string getName() override { return "poisons combat raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };
}