#pragma once
#include "WarlockStrategy.h"

namespace ai
{
    class AfflictionWarlockPlaceholderStrategy : public SpecPlaceholderStrategy
    {
    public:
        AfflictionWarlockPlaceholderStrategy(PlayerbotAI* ai) : SpecPlaceholderStrategy(ai) {}
        int GetType() override { return STRATEGY_TYPE_DPS | STRATEGY_TYPE_RANGED; }
        std::string getName() override { return "affliction"; }
    };

    class AfflictionWarlockStrategy : public WarlockStrategy
    {
    public:
        AfflictionWarlockStrategy(PlayerbotAI* ai) : WarlockStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;

        virtual NextAction** GetDefaultCombatActions() override;
    };

    class AfflictionWarlockPveStrategy : public AfflictionWarlockStrategy
    {
    public:
        AfflictionWarlockPveStrategy(PlayerbotAI* ai) : AfflictionWarlockStrategy(ai) {}
        std::string getName() override { return "affliction pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
        void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class AfflictionWarlockPvpStrategy : public AfflictionWarlockStrategy
    {
    public:
        AfflictionWarlockPvpStrategy(PlayerbotAI* ai) : AfflictionWarlockStrategy(ai) {}
        std::string getName() override { return "affliction pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
        void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class AfflictionWarlockRaidStrategy : public AfflictionWarlockStrategy
    {
    public:
        AfflictionWarlockRaidStrategy(PlayerbotAI* ai) : AfflictionWarlockStrategy(ai) {}
        std::string getName() override { return "affliction raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
        void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class AfflictionWarlockAoeStrategy : public WarlockAoeStrategy
    {
    public:
        AfflictionWarlockAoeStrategy(PlayerbotAI* ai) : WarlockAoeStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class AfflictionWarlockAoePveStrategy : public AfflictionWarlockAoeStrategy
    {
    public:
        AfflictionWarlockAoePveStrategy(PlayerbotAI* ai) : AfflictionWarlockAoeStrategy(ai) {}
        std::string getName() override { return "aoe affliction pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class AfflictionWarlockAoePvpStrategy : public AfflictionWarlockAoeStrategy
    {
    public:
        AfflictionWarlockAoePvpStrategy(PlayerbotAI* ai) : AfflictionWarlockAoeStrategy(ai) {}
        std::string getName() override { return "aoe affliction pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class AfflictionWarlockAoeRaidStrategy : public AfflictionWarlockAoeStrategy
    {
    public:
        AfflictionWarlockAoeRaidStrategy(PlayerbotAI* ai) : AfflictionWarlockAoeStrategy(ai) {}
        std::string getName() override { return "aoe affliction raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class AfflictionWarlockBuffStrategy : public WarlockBuffStrategy
    {
    public:
        AfflictionWarlockBuffStrategy(PlayerbotAI* ai) : WarlockBuffStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class AfflictionWarlockBuffPveStrategy : public AfflictionWarlockBuffStrategy
    {
    public:
        AfflictionWarlockBuffPveStrategy(PlayerbotAI* ai) : AfflictionWarlockBuffStrategy(ai) {}
        std::string getName() override { return "buff affliction pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class AfflictionWarlockBuffPvpStrategy : public AfflictionWarlockBuffStrategy
    {
    public:
        AfflictionWarlockBuffPvpStrategy(PlayerbotAI* ai) : AfflictionWarlockBuffStrategy(ai) {}
        std::string getName() override { return "buff affliction pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class AfflictionWarlockBuffRaidStrategy : public AfflictionWarlockBuffStrategy
    {
    public:
        AfflictionWarlockBuffRaidStrategy(PlayerbotAI* ai) : AfflictionWarlockBuffStrategy(ai) {}
        std::string getName() override { return "buff affliction raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class AfflictionWarlockBoostStrategy : public WarlockBoostStrategy
    {
    public:
        AfflictionWarlockBoostStrategy(PlayerbotAI* ai) : WarlockBoostStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class AfflictionWarlockBoostPveStrategy : public AfflictionWarlockBoostStrategy
    {
    public:
        AfflictionWarlockBoostPveStrategy(PlayerbotAI* ai) : AfflictionWarlockBoostStrategy(ai) {}
        std::string getName() override { return "boost affliction pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class AfflictionWarlockBoostPvpStrategy : public AfflictionWarlockBoostStrategy
    {
    public:
        AfflictionWarlockBoostPvpStrategy(PlayerbotAI* ai) : AfflictionWarlockBoostStrategy(ai) {}
        std::string getName() override { return "boost affliction pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class AfflictionWarlockBoostRaidStrategy : public AfflictionWarlockBoostStrategy
    {
    public:
        AfflictionWarlockBoostRaidStrategy(PlayerbotAI* ai) : AfflictionWarlockBoostStrategy(ai) {}
        std::string getName() override { return "boost affliction raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class AfflictionWarlockCcStrategy : public WarlockCcStrategy
    {
    public:
        AfflictionWarlockCcStrategy(PlayerbotAI* ai) : WarlockCcStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class AfflictionWarlockCcPveStrategy : public AfflictionWarlockCcStrategy
    {
    public:
        AfflictionWarlockCcPveStrategy(PlayerbotAI* ai) : AfflictionWarlockCcStrategy(ai) {}
        std::string getName() override { return "cc affliction pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class AfflictionWarlockCcPvpStrategy : public AfflictionWarlockCcStrategy
    {
    public:
        AfflictionWarlockCcPvpStrategy(PlayerbotAI* ai) : AfflictionWarlockCcStrategy(ai) {}
        std::string getName() override { return "cc affliction pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class AfflictionWarlockCcRaidStrategy : public AfflictionWarlockCcStrategy
    {
    public:
        AfflictionWarlockCcRaidStrategy(PlayerbotAI* ai) : AfflictionWarlockCcStrategy(ai) {}
        std::string getName() override { return "cc affliction raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class AfflictionWarlockPetStrategy : public WarlockPetStrategy
    {
    public:
        AfflictionWarlockPetStrategy(PlayerbotAI* ai) : WarlockPetStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class AfflictionWarlockPetPveStrategy : public AfflictionWarlockPetStrategy
    {
    public:
        AfflictionWarlockPetPveStrategy(PlayerbotAI* ai) : AfflictionWarlockPetStrategy(ai) {}
        std::string getName() override { return "pet affliction pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class AfflictionWarlockPetPvpStrategy : public AfflictionWarlockPetStrategy
    {
    public:
        AfflictionWarlockPetPvpStrategy(PlayerbotAI* ai) : AfflictionWarlockPetStrategy(ai) {}
        std::string getName() override { return "pet affliction pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class AfflictionWarlockPetRaidStrategy : public AfflictionWarlockPetStrategy
    {
    public:
        AfflictionWarlockPetRaidStrategy(PlayerbotAI* ai) : AfflictionWarlockPetStrategy(ai) {}
        std::string getName() override { return "pet affliction raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class AfflictionWarlockCursesStrategy : public WarlockCursesStrategy
    {
    public:
        AfflictionWarlockCursesStrategy(PlayerbotAI* ai) : WarlockCursesStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class AfflictionWarlockCursesPveStrategy : public AfflictionWarlockCursesStrategy
    {
    public:
        AfflictionWarlockCursesPveStrategy(PlayerbotAI* ai) : AfflictionWarlockCursesStrategy(ai) {}
        std::string getName() override { return "curse affliction pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class AfflictionWarlockCursesPvpStrategy : public AfflictionWarlockCursesStrategy
    {
    public:
        AfflictionWarlockCursesPvpStrategy(PlayerbotAI* ai) : AfflictionWarlockCursesStrategy(ai) {}
        std::string getName() override { return "curse affliction pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class AfflictionWarlockCursesRaidStrategy : public AfflictionWarlockCursesStrategy
    {
    public:
        AfflictionWarlockCursesRaidStrategy(PlayerbotAI* ai) : AfflictionWarlockCursesStrategy(ai) {}
        std::string getName() override { return "curse affliction raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };
}