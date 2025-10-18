#pragma once
#include "ShamanStrategy.h"

namespace ai
{
    class ElementalShamanPlaceholderStrategy : public SpecPlaceholderStrategy
    {
    public:
        ElementalShamanPlaceholderStrategy(PlayerbotAI* ai) : SpecPlaceholderStrategy(ai) {}
        int GetType() override { return STRATEGY_TYPE_DPS | STRATEGY_TYPE_RANGED; }
        std::string getName() override { return "elemental"; }
    };

    class ElementalShamanStrategy : public ShamanStrategy
    {
    public:
        ElementalShamanStrategy(PlayerbotAI* ai);

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;

        virtual NextAction** GetDefaultCombatActions() override;
    };

    class ElementalShamanPveStrategy : public ElementalShamanStrategy
    {
    public:
        ElementalShamanPveStrategy(PlayerbotAI* ai) : ElementalShamanStrategy(ai) {}
        std::string getName() override { return "elemental pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
        void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class ElementalShamanPvpStrategy : public ElementalShamanStrategy
    {
    public:
        ElementalShamanPvpStrategy(PlayerbotAI* ai) : ElementalShamanStrategy(ai) {}
        std::string getName() override { return "elemental pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
        void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class ElementalShamanRaidStrategy : public ElementalShamanStrategy
    {
    public:
        ElementalShamanRaidStrategy(PlayerbotAI* ai) : ElementalShamanStrategy(ai) {}
        std::string getName() override { return "elemental raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
        void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class ElementalShamanAoeStrategy : public ShamanAoeStrategy
    {
    public:
        ElementalShamanAoeStrategy(PlayerbotAI* ai) : ShamanAoeStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class ElementalShamanAoePveStrategy : public ElementalShamanAoeStrategy
    {
    public:
        ElementalShamanAoePveStrategy(PlayerbotAI* ai) : ElementalShamanAoeStrategy(ai) {}
        std::string getName() override { return "aoe elemental pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class ElementalShamanAoePvpStrategy : public ElementalShamanAoeStrategy
    {
    public:
        ElementalShamanAoePvpStrategy(PlayerbotAI* ai) : ElementalShamanAoeStrategy(ai) {}
        std::string getName() override { return "aoe elemental pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class ElementalShamanAoeRaidStrategy : public ElementalShamanAoeStrategy
    {
    public:
        ElementalShamanAoeRaidStrategy(PlayerbotAI* ai) : ElementalShamanAoeStrategy(ai) {}
        std::string getName() override { return "aoe elemental raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class ElementalShamanCcStrategy : public ShamanCcStrategy
    {
    public:
        ElementalShamanCcStrategy(PlayerbotAI* ai) : ShamanCcStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class ElementalShamanCcPveStrategy : public ElementalShamanCcStrategy
    {
    public:
        ElementalShamanCcPveStrategy(PlayerbotAI* ai) : ElementalShamanCcStrategy(ai) {}
        std::string getName() override { return "cc elemental pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class ElementalShamanCcPvpStrategy : public ElementalShamanCcStrategy
    {
    public:
        ElementalShamanCcPvpStrategy(PlayerbotAI* ai) : ElementalShamanCcStrategy(ai) {}
        std::string getName() override { return "cc elemental pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class ElementalShamanCcRaidStrategy : public ElementalShamanCcStrategy
    {
    public:
        ElementalShamanCcRaidStrategy(PlayerbotAI* ai) : ElementalShamanCcStrategy(ai) {}
        std::string getName() override { return "cc elemental raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class ElementalShamanCureStrategy : public ShamanCureStrategy
    {
    public:
        ElementalShamanCureStrategy(PlayerbotAI* ai) : ShamanCureStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class ElementalShamanCurePveStrategy : public ElementalShamanCureStrategy
    {
    public:
        ElementalShamanCurePveStrategy(PlayerbotAI* ai) : ElementalShamanCureStrategy(ai) {}
        std::string getName() override { return "cure elemental pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class ElementalShamanCurePvpStrategy : public ElementalShamanCureStrategy
    {
    public:
        ElementalShamanCurePvpStrategy(PlayerbotAI* ai) : ElementalShamanCureStrategy(ai) {}
        std::string getName() override { return "cure elemental pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class ElementalShamanCureRaidStrategy : public ElementalShamanCureStrategy
    {
    public:
        ElementalShamanCureRaidStrategy(PlayerbotAI* ai) : ElementalShamanCureStrategy(ai) {}
        std::string getName() override { return "cure elemental raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class ElementalShamanTotemsStrategy : public ShamanTotemsStrategy
    {
    public:
        ElementalShamanTotemsStrategy(PlayerbotAI* ai) : ShamanTotemsStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class ElementalShamanTotemsPveStrategy : public ElementalShamanTotemsStrategy
    {
    public:
        ElementalShamanTotemsPveStrategy(PlayerbotAI* ai) : ElementalShamanTotemsStrategy(ai) {}
        std::string getName() override { return "totems elemental pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class ElementalShamanTotemsPvpStrategy : public ElementalShamanTotemsStrategy
    {
    public:
        ElementalShamanTotemsPvpStrategy(PlayerbotAI* ai) : ElementalShamanTotemsStrategy(ai) {}
        std::string getName() override { return "totems elemental pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class ElementalShamanTotemsRaidStrategy : public ElementalShamanTotemsStrategy
    {
    public:
        ElementalShamanTotemsRaidStrategy(PlayerbotAI* ai) : ElementalShamanTotemsStrategy(ai) {}
        std::string getName() override { return "totems elemental raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class ElementalShamanBuffStrategy : public ShamanBuffStrategy
    {
    public:
        ElementalShamanBuffStrategy(PlayerbotAI* ai) : ShamanBuffStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class ElementalShamanBuffPveStrategy : public ElementalShamanBuffStrategy
    {
    public:
        ElementalShamanBuffPveStrategy(PlayerbotAI* ai) : ElementalShamanBuffStrategy(ai) {}
        std::string getName() override { return "buff elemental pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class ElementalShamanBuffPvpStrategy : public ElementalShamanBuffStrategy
    {
    public:
        ElementalShamanBuffPvpStrategy(PlayerbotAI* ai) : ElementalShamanBuffStrategy(ai) {}
        std::string getName() override { return "buff elemental pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class ElementalShamanBuffRaidStrategy : public ElementalShamanBuffStrategy
    {
    public:
        ElementalShamanBuffRaidStrategy(PlayerbotAI* ai) : ElementalShamanBuffStrategy(ai) {}
        std::string getName() override { return "buff elemental raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class ElementalShamanBoostStrategy : public ShamanBoostStrategy
    {
    public:
        ElementalShamanBoostStrategy(PlayerbotAI* ai) : ShamanBoostStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class ElementalShamanBoostPveStrategy : public ElementalShamanBoostStrategy
    {
    public:
        ElementalShamanBoostPveStrategy(PlayerbotAI* ai) : ElementalShamanBoostStrategy(ai) {}
        std::string getName() override { return "boost elemental pve"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class ElementalShamanBoostPvpStrategy : public ElementalShamanBoostStrategy
    {
    public:
        ElementalShamanBoostPvpStrategy(PlayerbotAI* ai) : ElementalShamanBoostStrategy(ai) {}
        std::string getName() override { return "boost elemental pvp"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class ElementalShamanBoostRaidStrategy : public ElementalShamanBoostStrategy
    {
    public:
        ElementalShamanBoostRaidStrategy(PlayerbotAI* ai) : ElementalShamanBoostStrategy(ai) {}
        std::string getName() override { return "boost elemental raid"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };
}
