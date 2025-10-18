#pragma once
#include "ShamanStrategy.h"

namespace ai
{
    class EnhancementShamanPlaceholderStrategy : public SpecPlaceholderStrategy
    {
    public:
        EnhancementShamanPlaceholderStrategy(PlayerbotAI* ai) : SpecPlaceholderStrategy(ai) {}
        int GetType() override { return STRATEGY_TYPE_DPS | STRATEGY_TYPE_MELEE; }
        std::string getName() override { return "enhancement"; }
    };

    class EnhancementShamanStrategy : public ShamanStrategy
    {
    public:
        EnhancementShamanStrategy(PlayerbotAI* ai);

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;

        virtual NextAction** GetDefaultCombatActions() override;
    };

    class EnhancementShamanPveStrategy : public EnhancementShamanStrategy
    {
    public:
        EnhancementShamanPveStrategy(PlayerbotAI* ai) : EnhancementShamanStrategy(ai) {}
        std::string getName() override { return "enhancement pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
        void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class EnhancementShamanPvpStrategy : public EnhancementShamanStrategy
    {
    public:
        EnhancementShamanPvpStrategy(PlayerbotAI* ai) : EnhancementShamanStrategy(ai) {}
        std::string getName() override { return "enhancement pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
        void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class EnhancementShamanRaidStrategy : public EnhancementShamanStrategy
    {
    public:
        EnhancementShamanRaidStrategy(PlayerbotAI* ai) : EnhancementShamanStrategy(ai) {}
        std::string getName() override { return "enhancement raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
        void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class EnhancementShamanAoeStrategy : public ShamanAoeStrategy
    {
    public:
        EnhancementShamanAoeStrategy(PlayerbotAI* ai) : ShamanAoeStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class EnhancementShamanAoePveStrategy : public EnhancementShamanAoeStrategy
    {
    public:
        EnhancementShamanAoePveStrategy(PlayerbotAI* ai) : EnhancementShamanAoeStrategy(ai) {}
        std::string getName() override { return "aoe enhancement pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class EnhancementShamanAoePvpStrategy : public EnhancementShamanAoeStrategy
    {
    public:
        EnhancementShamanAoePvpStrategy(PlayerbotAI* ai) : EnhancementShamanAoeStrategy(ai) {}
        std::string getName() override { return "aoe enhancement pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class EnhancementShamanAoeRaidStrategy : public EnhancementShamanAoeStrategy
    {
    public:
        EnhancementShamanAoeRaidStrategy(PlayerbotAI* ai) : EnhancementShamanAoeStrategy(ai) {}
        std::string getName() override { return "aoe enhancement raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class EnhancementShamanCcStrategy : public ShamanCcStrategy
    {
    public:
        EnhancementShamanCcStrategy(PlayerbotAI* ai) : ShamanCcStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class EnhancementShamanCcPveStrategy : public EnhancementShamanCcStrategy
    {
    public:
        EnhancementShamanCcPveStrategy(PlayerbotAI* ai) : EnhancementShamanCcStrategy(ai) {}
        std::string getName() override { return "cc enhancement pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class EnhancementShamanCcPvpStrategy : public EnhancementShamanCcStrategy
    {
    public:
        EnhancementShamanCcPvpStrategy(PlayerbotAI* ai) : EnhancementShamanCcStrategy(ai) {}
        std::string getName() override { return "cc enhancement pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class EnhancementShamanCcRaidStrategy : public EnhancementShamanCcStrategy
    {
    public:
        EnhancementShamanCcRaidStrategy(PlayerbotAI* ai) : EnhancementShamanCcStrategy(ai) {}
        std::string getName() override { return "cc enhancement raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class EnhancementShamanCureStrategy : public ShamanCureStrategy
    {
    public:
        EnhancementShamanCureStrategy(PlayerbotAI* ai) : ShamanCureStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class EnhancementShamanCurePveStrategy : public EnhancementShamanCureStrategy
    {
    public:
        EnhancementShamanCurePveStrategy(PlayerbotAI* ai) : EnhancementShamanCureStrategy(ai) {}
        std::string getName() override { return "cure enhancement pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class EnhancementShamanCurePvpStrategy : public EnhancementShamanCureStrategy
    {
    public:
        EnhancementShamanCurePvpStrategy(PlayerbotAI* ai) : EnhancementShamanCureStrategy(ai) {}
        std::string getName() override { return "cure enhancement pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class EnhancementShamanCureRaidStrategy : public EnhancementShamanCureStrategy
    {
    public:
        EnhancementShamanCureRaidStrategy(PlayerbotAI* ai) : EnhancementShamanCureStrategy(ai) {}
        std::string getName() override { return "cure enhancement raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class EnhancementShamanTotemsStrategy : public ShamanTotemsStrategy
    {
    public:
        EnhancementShamanTotemsStrategy(PlayerbotAI* ai) : ShamanTotemsStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class EnhancementShamanTotemsPveStrategy : public EnhancementShamanTotemsStrategy
    {
    public:
        EnhancementShamanTotemsPveStrategy(PlayerbotAI* ai) : EnhancementShamanTotemsStrategy(ai) {}
        std::string getName() override { return "totems enhancement pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class EnhancementShamanTotemsPvpStrategy : public EnhancementShamanTotemsStrategy
    {
    public:
        EnhancementShamanTotemsPvpStrategy(PlayerbotAI* ai) : EnhancementShamanTotemsStrategy(ai) {}
        std::string getName() override { return "totems enhancement pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class EnhancementShamanTotemsRaidStrategy : public EnhancementShamanTotemsStrategy
    {
    public:
        EnhancementShamanTotemsRaidStrategy(PlayerbotAI* ai) : EnhancementShamanTotemsStrategy(ai) {}
        std::string getName() override { return "totems enhancement raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class EnhancementShamanBuffStrategy : public ShamanBuffStrategy
    {
    public:
        EnhancementShamanBuffStrategy(PlayerbotAI* ai) : ShamanBuffStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class EnhancementShamanBuffPveStrategy : public EnhancementShamanBuffStrategy
    {
    public:
        EnhancementShamanBuffPveStrategy(PlayerbotAI* ai) : EnhancementShamanBuffStrategy(ai) {}
        std::string getName() override { return "buff enhancement pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class EnhancementShamanBuffPvpStrategy : public EnhancementShamanBuffStrategy
    {
    public:
        EnhancementShamanBuffPvpStrategy(PlayerbotAI* ai) : EnhancementShamanBuffStrategy(ai) {}
        std::string getName() override { return "buff enhancement pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class EnhancementShamanBuffRaidStrategy : public EnhancementShamanBuffStrategy
    {
    public:
        EnhancementShamanBuffRaidStrategy(PlayerbotAI* ai) : EnhancementShamanBuffStrategy(ai) {}
        std::string getName() override { return "buff enhancement raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class EnhancementShamanBoostStrategy : public ShamanBoostStrategy
    {
    public:
        EnhancementShamanBoostStrategy(PlayerbotAI* ai) : ShamanBoostStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class EnhancementShamanBoostPveStrategy : public EnhancementShamanBoostStrategy
    {
    public:
        EnhancementShamanBoostPveStrategy(PlayerbotAI* ai) : EnhancementShamanBoostStrategy(ai) {}
        std::string getName() override { return "boost enhancement pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class EnhancementShamanBoostPvpStrategy : public EnhancementShamanBoostStrategy
    {
    public:
        EnhancementShamanBoostPvpStrategy(PlayerbotAI* ai) : EnhancementShamanBoostStrategy(ai) {}
        std::string getName() override { return "boost enhancement pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class EnhancementShamanBoostRaidStrategy : public EnhancementShamanBoostStrategy
    {
    public:
        EnhancementShamanBoostRaidStrategy(PlayerbotAI* ai) : EnhancementShamanBoostStrategy(ai) {}
        std::string getName() override { return "boost enhancement raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };
}
