#pragma once
#include "ShamanStrategy.h"

namespace ai
{
    class RestorationShamanPlaceholderStrategy : public SpecPlaceholderStrategy
    {
    public:
        RestorationShamanPlaceholderStrategy(PlayerbotAI* ai) : SpecPlaceholderStrategy(ai) {}
        int GetType() override { return STRATEGY_TYPE_HEAL | STRATEGY_TYPE_RANGED; }
        std::string getName() override { return "restoration"; }
    };

    class RestorationShamanStrategy : public ShamanStrategy
    {
    public:
        RestorationShamanStrategy(PlayerbotAI* ai);

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class RestorationShamanPveStrategy : public RestorationShamanStrategy
    {
    public:
        RestorationShamanPveStrategy(PlayerbotAI* ai) : RestorationShamanStrategy(ai) {}
        std::string getName() override { return "restoration pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
        void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class RestorationShamanPvpStrategy : public RestorationShamanStrategy
    {
    public:
        RestorationShamanPvpStrategy(PlayerbotAI* ai) : RestorationShamanStrategy(ai) {}
        std::string getName() override { return "restoration pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
        void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class RestorationShamanRaidStrategy : public RestorationShamanStrategy
    {
    public:
        RestorationShamanRaidStrategy(PlayerbotAI* ai) : RestorationShamanStrategy(ai) {}
        std::string getName() override { return "restoration raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
        void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class RestorationShamanAoeStrategy : public ShamanAoeStrategy
    {
    public:
        RestorationShamanAoeStrategy(PlayerbotAI* ai) : ShamanAoeStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class RestorationShamanAoePveStrategy : public RestorationShamanAoeStrategy
    {
    public:
        RestorationShamanAoePveStrategy(PlayerbotAI* ai) : RestorationShamanAoeStrategy(ai) {}
        std::string getName() override { return "aoe restoration pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class RestorationShamanAoePvpStrategy : public RestorationShamanAoeStrategy
    {
    public:
        RestorationShamanAoePvpStrategy(PlayerbotAI* ai) : RestorationShamanAoeStrategy(ai) {}
        std::string getName() override { return "aoe restoration pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class RestorationShamanAoeRaidStrategy : public RestorationShamanAoeStrategy
    {
    public:
        RestorationShamanAoeRaidStrategy(PlayerbotAI* ai) : RestorationShamanAoeStrategy(ai) {}
        std::string getName() override { return "aoe restoration raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class RestorationShamanCcStrategy : public ShamanCcStrategy
    {
    public:
        RestorationShamanCcStrategy(PlayerbotAI* ai) : ShamanCcStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class RestorationShamanCcPveStrategy : public RestorationShamanCcStrategy
    {
    public:
        RestorationShamanCcPveStrategy(PlayerbotAI* ai) : RestorationShamanCcStrategy(ai) {}
        std::string getName() override { return "cc restoration pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class RestorationShamanCcPvpStrategy : public RestorationShamanCcStrategy
    {
    public:
        RestorationShamanCcPvpStrategy(PlayerbotAI* ai) : RestorationShamanCcStrategy(ai) {}
        std::string getName() override { return "cc restoration pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class RestorationShamanCcRaidStrategy : public RestorationShamanCcStrategy
    {
    public:
        RestorationShamanCcRaidStrategy(PlayerbotAI* ai) : RestorationShamanCcStrategy(ai) {}
        std::string getName() override { return "cc restoration raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class RestorationShamanCureStrategy : public ShamanCureStrategy
    {
    public:
        RestorationShamanCureStrategy(PlayerbotAI* ai) : ShamanCureStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class RestorationShamanCurePveStrategy : public RestorationShamanCureStrategy
    {
    public:
        RestorationShamanCurePveStrategy(PlayerbotAI* ai) : RestorationShamanCureStrategy(ai) {}
        std::string getName() override { return "cure restoration pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class RestorationShamanCurePvpStrategy : public RestorationShamanCureStrategy
    {
    public:
        RestorationShamanCurePvpStrategy(PlayerbotAI* ai) : RestorationShamanCureStrategy(ai) {}
        std::string getName() override { return "cure restoration pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class RestorationShamanCureRaidStrategy : public RestorationShamanCureStrategy
    {
    public:
        RestorationShamanCureRaidStrategy(PlayerbotAI* ai) : RestorationShamanCureStrategy(ai) {}
        std::string getName() override { return "cure restoration raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class RestorationShamanTotemsStrategy : public ShamanTotemsStrategy
    {
    public:
        RestorationShamanTotemsStrategy(PlayerbotAI* ai) : ShamanTotemsStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class RestorationShamanTotemsPveStrategy : public RestorationShamanTotemsStrategy
    {
    public:
        RestorationShamanTotemsPveStrategy(PlayerbotAI* ai) : RestorationShamanTotemsStrategy(ai) {}
        std::string getName() override { return "totems restoration pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class RestorationShamanTotemsPvpStrategy : public RestorationShamanTotemsStrategy
    {
    public:
        RestorationShamanTotemsPvpStrategy(PlayerbotAI* ai) : RestorationShamanTotemsStrategy(ai) {}
        std::string getName() override { return "totems restoration pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class RestorationShamanTotemsRaidStrategy : public RestorationShamanTotemsStrategy
    {
    public:
        RestorationShamanTotemsRaidStrategy(PlayerbotAI* ai) : RestorationShamanTotemsStrategy(ai) {}
        std::string getName() override { return "totems restoration raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class RestorationShamanBuffStrategy : public ShamanBuffStrategy
    {
    public:
        RestorationShamanBuffStrategy(PlayerbotAI* ai) : ShamanBuffStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class RestorationShamanBuffPveStrategy : public RestorationShamanBuffStrategy
    {
    public:
        RestorationShamanBuffPveStrategy(PlayerbotAI* ai) : RestorationShamanBuffStrategy(ai) {}
        std::string getName() override { return "buff restoration pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class RestorationShamanBuffPvpStrategy : public RestorationShamanBuffStrategy
    {
    public:
        RestorationShamanBuffPvpStrategy(PlayerbotAI* ai) : RestorationShamanBuffStrategy(ai) {}
        std::string getName() override { return "buff restoration pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class RestorationShamanBuffRaidStrategy : public RestorationShamanBuffStrategy
    {
    public:
        RestorationShamanBuffRaidStrategy(PlayerbotAI* ai) : RestorationShamanBuffStrategy(ai) {}
        std::string getName() override { return "buff restoration raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class RestorationShamanBoostStrategy : public ShamanBoostStrategy
    {
    public:
        RestorationShamanBoostStrategy(PlayerbotAI* ai) : ShamanBoostStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class RestorationShamanBoostPveStrategy : public RestorationShamanBoostStrategy
    {
    public:
        RestorationShamanBoostPveStrategy(PlayerbotAI* ai) : RestorationShamanBoostStrategy(ai) {}
        std::string getName() override { return "boost restoration pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class RestorationShamanBoostPvpStrategy : public RestorationShamanBoostStrategy
    {
    public:
        RestorationShamanBoostPvpStrategy(PlayerbotAI* ai) : RestorationShamanBoostStrategy(ai) {}
        std::string getName() override { return "boost restoration pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class RestorationShamanBoostRaidStrategy : public RestorationShamanBoostStrategy
    {
    public:
        RestorationShamanBoostRaidStrategy(PlayerbotAI* ai) : RestorationShamanBoostStrategy(ai) {}
        std::string getName() override { return "boost restoration raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };
}
