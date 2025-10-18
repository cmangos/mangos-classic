#pragma once
#include "WarlockStrategy.h"

namespace ai
{
    class DemonologyWarlockPlaceholderStrategy : public SpecPlaceholderStrategy
    {
    public:
        DemonologyWarlockPlaceholderStrategy(PlayerbotAI* ai) : SpecPlaceholderStrategy(ai) {}
        int GetType() override { return STRATEGY_TYPE_DPS | STRATEGY_TYPE_RANGED; }
        std::string getName() override { return "demonology"; }
    };

    class DemonologyWarlockStrategy : public WarlockStrategy
    {
    public:
        DemonologyWarlockStrategy(PlayerbotAI* ai);

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;

        virtual NextAction** GetDefaultCombatActions() override;
    };

    class DemonologyWarlockPveStrategy : public DemonologyWarlockStrategy
    {
    public:
        DemonologyWarlockPveStrategy(PlayerbotAI* ai) : DemonologyWarlockStrategy(ai) {}
        std::string getName() override { return "demonology pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
        void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class DemonologyWarlockPvpStrategy : public DemonologyWarlockStrategy
    {
    public:
        DemonologyWarlockPvpStrategy(PlayerbotAI* ai) : DemonologyWarlockStrategy(ai) {}
        std::string getName() override { return "demonology pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
        void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class DemonologyWarlockRaidStrategy : public DemonologyWarlockStrategy
    {
    public:
        DemonologyWarlockRaidStrategy(PlayerbotAI* ai) : DemonologyWarlockStrategy(ai) {}
        std::string getName() override { return "demonology raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
        void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class DemonologyWarlockAoeStrategy : public WarlockAoeStrategy
    {
    public:
        DemonologyWarlockAoeStrategy(PlayerbotAI* ai) : WarlockAoeStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class DemonologyWarlockAoePveStrategy : public DemonologyWarlockAoeStrategy
    {
    public:
        DemonologyWarlockAoePveStrategy(PlayerbotAI* ai) : DemonologyWarlockAoeStrategy(ai) {}
        std::string getName() override { return "aoe demonology pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class DemonologyWarlockAoePvpStrategy : public DemonologyWarlockAoeStrategy
    {
    public:
        DemonologyWarlockAoePvpStrategy(PlayerbotAI* ai) : DemonologyWarlockAoeStrategy(ai) {}
        std::string getName() override { return "aoe demonology pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class DemonologyWarlockAoeRaidStrategy : public DemonologyWarlockAoeStrategy
    {
    public:
        DemonologyWarlockAoeRaidStrategy(PlayerbotAI* ai) : DemonologyWarlockAoeStrategy(ai) {}
        std::string getName() override { return "aoe demonology raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class DemonologyWarlockBuffStrategy : public WarlockBuffStrategy
    {
    public:
        DemonologyWarlockBuffStrategy(PlayerbotAI* ai) : WarlockBuffStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class DemonologyWarlockBuffPveStrategy : public DemonologyWarlockBuffStrategy
    {
    public:
        DemonologyWarlockBuffPveStrategy(PlayerbotAI* ai) : DemonologyWarlockBuffStrategy(ai) {}
        std::string getName() override { return "buff demonology pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class DemonologyWarlockBuffPvpStrategy : public DemonologyWarlockBuffStrategy
    {
    public:
        DemonologyWarlockBuffPvpStrategy(PlayerbotAI* ai) : DemonologyWarlockBuffStrategy(ai) {}
        std::string getName() override { return "buff demonology pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class DemonologyWarlockBuffRaidStrategy : public DemonologyWarlockBuffStrategy
    {
    public:
        DemonologyWarlockBuffRaidStrategy(PlayerbotAI* ai) : DemonologyWarlockBuffStrategy(ai) {}
        std::string getName() override { return "buff demonology raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class DemonologyWarlockBoostStrategy : public WarlockBoostStrategy
    {
    public:
        DemonologyWarlockBoostStrategy(PlayerbotAI* ai) : WarlockBoostStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class DemonologyWarlockBoostPveStrategy : public DemonologyWarlockBoostStrategy
    {
    public:
        DemonologyWarlockBoostPveStrategy(PlayerbotAI* ai) : DemonologyWarlockBoostStrategy(ai) {}
        std::string getName() override { return "boost demonology pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class DemonologyWarlockBoostPvpStrategy : public DemonologyWarlockBoostStrategy
    {
    public:
        DemonologyWarlockBoostPvpStrategy(PlayerbotAI* ai) : DemonologyWarlockBoostStrategy(ai) {}
        std::string getName() override { return "boost demonology pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class DemonologyWarlockBoostRaidStrategy : public DemonologyWarlockBoostStrategy
    {
    public:
        DemonologyWarlockBoostRaidStrategy(PlayerbotAI* ai) : DemonologyWarlockBoostStrategy(ai) {}
        std::string getName() override { return "boost demonology raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class DemonologyWarlockCcStrategy : public WarlockCcStrategy
    {
    public:
        DemonologyWarlockCcStrategy(PlayerbotAI* ai) : WarlockCcStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class DemonologyWarlockCcPveStrategy : public DemonologyWarlockCcStrategy
    {
    public:
        DemonologyWarlockCcPveStrategy(PlayerbotAI* ai) : DemonologyWarlockCcStrategy(ai) {}
        std::string getName() override { return "cc demonology pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class DemonologyWarlockCcPvpStrategy : public DemonologyWarlockCcStrategy
    {
    public:
        DemonologyWarlockCcPvpStrategy(PlayerbotAI* ai) : DemonologyWarlockCcStrategy(ai) {}
        std::string getName() override { return "cc demonology pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class DemonologyWarlockCcRaidStrategy : public DemonologyWarlockCcStrategy
    {
    public:
        DemonologyWarlockCcRaidStrategy(PlayerbotAI* ai) : DemonologyWarlockCcStrategy(ai) {}
        std::string getName() override { return "cc demonology raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class DemonologyWarlockPetStrategy : public WarlockPetStrategy
    {
    public:
        DemonologyWarlockPetStrategy(PlayerbotAI* ai) : WarlockPetStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class DemonologyWarlockPetPveStrategy : public DemonologyWarlockPetStrategy
    {
    public:
        DemonologyWarlockPetPveStrategy(PlayerbotAI* ai) : DemonologyWarlockPetStrategy(ai) {}
        std::string getName() override { return "pet demonology pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class DemonologyWarlockPetPvpStrategy : public DemonologyWarlockPetStrategy
    {
    public:
        DemonologyWarlockPetPvpStrategy(PlayerbotAI* ai) : DemonologyWarlockPetStrategy(ai) {}
        std::string getName() override { return "pet demonology pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class DemonologyWarlockPetRaidStrategy : public DemonologyWarlockPetStrategy
    {
    public:
        DemonologyWarlockPetRaidStrategy(PlayerbotAI* ai) : DemonologyWarlockPetStrategy(ai) {}
        std::string getName() override { return "pet demonology raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class DemonologyWarlockCursesStrategy : public WarlockCursesStrategy
    {
    public:
        DemonologyWarlockCursesStrategy(PlayerbotAI* ai) : WarlockCursesStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class DemonologyWarlockCursesPveStrategy : public DemonologyWarlockCursesStrategy
    {
    public:
        DemonologyWarlockCursesPveStrategy(PlayerbotAI* ai) : DemonologyWarlockCursesStrategy(ai) {}
        std::string getName() override { return "curse demonology pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class DemonologyWarlockCursesPvpStrategy : public DemonologyWarlockCursesStrategy
    {
    public:
        DemonologyWarlockCursesPvpStrategy(PlayerbotAI* ai) : DemonologyWarlockCursesStrategy(ai) {}
        std::string getName() override { return "curse demonology pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class DemonologyWarlockCursesRaidStrategy : public DemonologyWarlockCursesStrategy
    {
    public:
        DemonologyWarlockCursesRaidStrategy(PlayerbotAI* ai) : DemonologyWarlockCursesStrategy(ai) {}
        std::string getName() override { return "curse demonology raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };
}