#pragma once
#include "WarlockStrategy.h"

namespace ai
{
    class DestructionWarlockPlaceholderStrategy : public SpecPlaceholderStrategy
    {
    public:
        DestructionWarlockPlaceholderStrategy(PlayerbotAI* ai) : SpecPlaceholderStrategy(ai) {}
        int GetType() override { return STRATEGY_TYPE_DPS | STRATEGY_TYPE_RANGED; }
        std::string getName() override { return "destruction"; }
    };

    class DestructionWarlockStrategy : public WarlockStrategy
    {
    public:
        DestructionWarlockStrategy(PlayerbotAI* ai) : WarlockStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;

        virtual NextAction** GetDefaultCombatActions() override;
    };

    class DestructionWarlockPveStrategy : public DestructionWarlockStrategy
    {
    public:
        DestructionWarlockPveStrategy(PlayerbotAI* ai) : DestructionWarlockStrategy(ai) {}
        std::string getName() override { return "destruction pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
        void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class DestructionWarlockPvpStrategy : public DestructionWarlockStrategy
    {
    public:
        DestructionWarlockPvpStrategy(PlayerbotAI* ai) : DestructionWarlockStrategy(ai) {}
        std::string getName() override { return "destruction pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
        void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class DestructionWarlockRaidStrategy : public DestructionWarlockStrategy
    {
    public:
        DestructionWarlockRaidStrategy(PlayerbotAI* ai) : DestructionWarlockStrategy(ai) {}
        std::string getName() override { return "destruction raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
        void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class DestructionWarlockAoeStrategy : public WarlockAoeStrategy
    {
    public:
        DestructionWarlockAoeStrategy(PlayerbotAI* ai) : WarlockAoeStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class DestructionWarlockAoePveStrategy : public DestructionWarlockAoeStrategy
    {
    public:
        DestructionWarlockAoePveStrategy(PlayerbotAI* ai) : DestructionWarlockAoeStrategy(ai) {}
        std::string getName() override { return "aoe destruction pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class DestructionWarlockAoePvpStrategy : public DestructionWarlockAoeStrategy
    {
    public:
        DestructionWarlockAoePvpStrategy(PlayerbotAI* ai) : DestructionWarlockAoeStrategy(ai) {}
        std::string getName() override { return "aoe destruction pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class DestructionWarlockAoeRaidStrategy : public DestructionWarlockAoeStrategy
    {
    public:
        DestructionWarlockAoeRaidStrategy(PlayerbotAI* ai) : DestructionWarlockAoeStrategy(ai) {}
        std::string getName() override { return "aoe destruction raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class DestructionWarlockBuffStrategy : public WarlockBuffStrategy
    {
    public:
        DestructionWarlockBuffStrategy(PlayerbotAI* ai) : WarlockBuffStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class DestructionWarlockBuffPveStrategy : public DestructionWarlockBuffStrategy
    {
    public:
        DestructionWarlockBuffPveStrategy(PlayerbotAI* ai) : DestructionWarlockBuffStrategy(ai) {}
        std::string getName() override { return "buff destruction pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class DestructionWarlockBuffPvpStrategy : public DestructionWarlockBuffStrategy
    {
    public:
        DestructionWarlockBuffPvpStrategy(PlayerbotAI* ai) : DestructionWarlockBuffStrategy(ai) {}
        std::string getName() override { return "buff destruction pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class DestructionWarlockBuffRaidStrategy : public DestructionWarlockBuffStrategy
    {
    public:
        DestructionWarlockBuffRaidStrategy(PlayerbotAI* ai) : DestructionWarlockBuffStrategy(ai) {}
        std::string getName() override { return "buff destruction raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class DestructionWarlockBoostStrategy : public WarlockBoostStrategy
    {
    public:
        DestructionWarlockBoostStrategy(PlayerbotAI* ai) : WarlockBoostStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class DestructionWarlockBoostPveStrategy : public DestructionWarlockBoostStrategy
    {
    public:
        DestructionWarlockBoostPveStrategy(PlayerbotAI* ai) : DestructionWarlockBoostStrategy(ai) {}
        std::string getName() override { return "boost destruction pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class DestructionWarlockBoostPvpStrategy : public DestructionWarlockBoostStrategy
    {
    public:
        DestructionWarlockBoostPvpStrategy(PlayerbotAI* ai) : DestructionWarlockBoostStrategy(ai) {}
        std::string getName() override { return "boost destruction pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class DestructionWarlockBoostRaidStrategy : public DestructionWarlockBoostStrategy
    {
    public:
        DestructionWarlockBoostRaidStrategy(PlayerbotAI* ai) : DestructionWarlockBoostStrategy(ai) {}
        std::string getName() override { return "boost destruction raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class DestructionWarlockCcStrategy : public WarlockCcStrategy
    {
    public:
        DestructionWarlockCcStrategy(PlayerbotAI* ai) : WarlockCcStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class DestructionWarlockCcPveStrategy : public DestructionWarlockCcStrategy
    {
    public:
        DestructionWarlockCcPveStrategy(PlayerbotAI* ai) : DestructionWarlockCcStrategy(ai) {}
        std::string getName() override { return "cc destruction pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class DestructionWarlockCcPvpStrategy : public DestructionWarlockCcStrategy
    {
    public:
        DestructionWarlockCcPvpStrategy(PlayerbotAI* ai) : DestructionWarlockCcStrategy(ai) {}
        std::string getName() override { return "cc destruction pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class DestructionWarlockCcRaidStrategy : public DestructionWarlockCcStrategy
    {
    public:
        DestructionWarlockCcRaidStrategy(PlayerbotAI* ai) : DestructionWarlockCcStrategy(ai) {}
        std::string getName() override { return "cc destruction raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class DestructionWarlockPetStrategy : public WarlockPetStrategy
    {
    public:
        DestructionWarlockPetStrategy(PlayerbotAI* ai) : WarlockPetStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class DestructionWarlockPetPveStrategy : public DestructionWarlockPetStrategy
    {
    public:
        DestructionWarlockPetPveStrategy(PlayerbotAI* ai) : DestructionWarlockPetStrategy(ai) {}
        std::string getName() override { return "pet destruction pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class DestructionWarlockPetPvpStrategy : public DestructionWarlockPetStrategy
    {
    public:
        DestructionWarlockPetPvpStrategy(PlayerbotAI* ai) : DestructionWarlockPetStrategy(ai) {}
        std::string getName() override { return "pet destruction pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class DestructionWarlockPetRaidStrategy : public DestructionWarlockPetStrategy
    {
    public:
        DestructionWarlockPetRaidStrategy(PlayerbotAI* ai) : DestructionWarlockPetStrategy(ai) {}
        std::string getName() override { return "pet destruction raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class DestructionWarlockCursesStrategy : public WarlockCursesStrategy
    {
    public:
        DestructionWarlockCursesStrategy(PlayerbotAI* ai) : WarlockCursesStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class DestructionWarlockCursesPveStrategy : public DestructionWarlockCursesStrategy
    {
    public:
        DestructionWarlockCursesPveStrategy(PlayerbotAI* ai) : DestructionWarlockCursesStrategy(ai) {}
        std::string getName() override { return "curse destruction pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class DestructionWarlockCursesPvpStrategy : public DestructionWarlockCursesStrategy
    {
    public:
        DestructionWarlockCursesPvpStrategy(PlayerbotAI* ai) : DestructionWarlockCursesStrategy(ai) {}
        std::string getName() override { return "curse destruction pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class DestructionWarlockCursesRaidStrategy : public DestructionWarlockCursesStrategy
    {
    public:
        DestructionWarlockCursesRaidStrategy(PlayerbotAI* ai) : DestructionWarlockCursesStrategy(ai) {}
        std::string getName() override { return "curse destruction raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };
}