
#include "playerbot/playerbot.h"
#include "MageMultipliers.h"
#include "ArcaneMageStrategy.h"

using namespace ai;

class ArcaneMageStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    ArcaneMageStrategyActionNodeFactory()
    {
        creators["arcane blast"] = &arcane_blast;
        creators["arcane barrage"] = &arcane_barrage;
        creators["arcane missiles"] = &arcane_missiles;
    }

private:
    ACTION_NODE_A(arcane_blast, "arcane blast", "arcane missiles");

    ACTION_NODE_A(arcane_barrage, "arcane barrage", "arcane missiles");

    ACTION_NODE_A(arcane_missiles, "arcane missiles", "shoot");
};

ArcaneMageStrategy::ArcaneMageStrategy(PlayerbotAI* ai) : MageStrategy(ai)
{
    actionNodeFactories.Add(new ArcaneMageStrategyActionNodeFactory());
}

#ifdef MANGOSBOT_ZERO // Vanilla

NextAction** ArcaneMageStrategy::GetDefaultCombatActions()
{
    return NextAction::array(0, new NextAction("arcane missiles", ACTION_IDLE), NULL);
}

void ArcaneMageStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    MageStrategy::InitCombatTriggers(triggers);
}

void ArcaneMageStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    MageStrategy::InitNonCombatTriggers(triggers);
}

void ArcaneMageStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    MageStrategy::InitReactionTriggers(triggers);
}

void ArcaneMageStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    MageStrategy::InitDeadTriggers(triggers);
}

void ArcaneMagePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageStrategy::InitCombatTriggers(triggers);
    MagePveStrategy::InitCombatTriggers(triggers);
}

void ArcaneMagePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageStrategy::InitNonCombatTriggers(triggers);
    MagePveStrategy::InitNonCombatTriggers(triggers);
}

void ArcaneMagePveStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageStrategy::InitReactionTriggers(triggers);
    MagePveStrategy::InitReactionTriggers(triggers);
}

void ArcaneMagePveStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageStrategy::InitDeadTriggers(triggers);
    MagePveStrategy::InitDeadTriggers(triggers);
}

void ArcaneMagePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageStrategy::InitCombatTriggers(triggers);
    MagePvpStrategy::InitCombatTriggers(triggers);
}

void ArcaneMagePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageStrategy::InitNonCombatTriggers(triggers);
    MagePvpStrategy::InitNonCombatTriggers(triggers);
}

void ArcaneMagePvpStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageStrategy::InitReactionTriggers(triggers);
    MagePvpStrategy::InitReactionTriggers(triggers);
}

void ArcaneMagePvpStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageStrategy::InitDeadTriggers(triggers);
    MagePvpStrategy::InitDeadTriggers(triggers);
}

void ArcaneMageRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageStrategy::InitCombatTriggers(triggers);
    MageRaidStrategy::InitCombatTriggers(triggers);
}

void ArcaneMageRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageStrategy::InitNonCombatTriggers(triggers);
    MageRaidStrategy::InitNonCombatTriggers(triggers);
}

void ArcaneMageRaidStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageStrategy::InitReactionTriggers(triggers);
    MageRaidStrategy::InitReactionTriggers(triggers);
}

void ArcaneMageRaidStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageStrategy::InitDeadTriggers(triggers);
    MageRaidStrategy::InitDeadTriggers(triggers);
}

void ArcaneMageAoeStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    MageAoeStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "enemy too close for spell",
        NextAction::array(0, new NextAction("arcane explosion", ACTION_HIGH), NULL)));
}

void ArcaneMageAoeStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    MageAoeStrategy::InitNonCombatTriggers(triggers);
}

void ArcaneMageAoePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageAoeStrategy::InitCombatTriggers(triggers);
    MageAoePveStrategy::InitCombatTriggers(triggers);
}

void ArcaneMageAoePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageAoeStrategy::InitNonCombatTriggers(triggers);
    MageAoePveStrategy::InitNonCombatTriggers(triggers);
}

void ArcaneMageAoePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageAoeStrategy::InitCombatTriggers(triggers);
    MageAoePvpStrategy::InitCombatTriggers(triggers);
}

void ArcaneMageAoePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageAoeStrategy::InitNonCombatTriggers(triggers);
    MageAoePvpStrategy::InitNonCombatTriggers(triggers);
}

void ArcaneMageAoeRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageAoeStrategy::InitCombatTriggers(triggers);
    MageAoeRaidStrategy::InitCombatTriggers(triggers);
}

void ArcaneMageAoeRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageAoeStrategy::InitNonCombatTriggers(triggers);
    MageAoeRaidStrategy::InitNonCombatTriggers(triggers);
}

void ArcaneMageBuffStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    MageBuffStrategy::InitCombatTriggers(triggers);
}

void ArcaneMageBuffStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    MageBuffStrategy::InitNonCombatTriggers(triggers);
}

void ArcaneMageBuffPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageBuffStrategy::InitCombatTriggers(triggers);
    MageBuffPveStrategy::InitCombatTriggers(triggers);
}

void ArcaneMageBuffPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageBuffStrategy::InitNonCombatTriggers(triggers);
    MageBuffPveStrategy::InitNonCombatTriggers(triggers);
}

void ArcaneMageBuffPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageBuffStrategy::InitCombatTriggers(triggers);
    MageBuffPvpStrategy::InitCombatTriggers(triggers);
}

void ArcaneMageBuffPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageBuffStrategy::InitNonCombatTriggers(triggers);
    MageBuffPvpStrategy::InitNonCombatTriggers(triggers);
}

void ArcaneMageBuffRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageBuffStrategy::InitCombatTriggers(triggers);
    MageBuffRaidStrategy::InitCombatTriggers(triggers);
}

void ArcaneMageBuffRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageBuffStrategy::InitNonCombatTriggers(triggers);
    MageBuffRaidStrategy::InitNonCombatTriggers(triggers);
}

void ArcaneMageBoostStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    MageBoostStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "arcane power",
        NextAction::array(0, new NextAction("arcane power", ACTION_HIGH + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "presence of mind",
        NextAction::array(0, new NextAction("presence of mind", ACTION_HIGH + 1), NULL)));
}

void ArcaneMageBoostStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    MageBoostStrategy::InitNonCombatTriggers(triggers);
}

void ArcaneMageBoostPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageBoostStrategy::InitCombatTriggers(triggers);
    MageBoostPveStrategy::InitCombatTriggers(triggers);
}

void ArcaneMageBoostPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageBoostStrategy::InitNonCombatTriggers(triggers);
    MageBoostPveStrategy::InitNonCombatTriggers(triggers);
}

void ArcaneMageBoostPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageBoostStrategy::InitCombatTriggers(triggers);
    MageBoostPvpStrategy::InitCombatTriggers(triggers);
}

void ArcaneMageBoostPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageBoostStrategy::InitNonCombatTriggers(triggers);
    MageBoostPvpStrategy::InitNonCombatTriggers(triggers);
}

void ArcaneMageBoostRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageBoostStrategy::InitCombatTriggers(triggers);
    MageBoostRaidStrategy::InitCombatTriggers(triggers);
}

void ArcaneMageBoostRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageBoostStrategy::InitNonCombatTriggers(triggers);
    MageBoostRaidStrategy::InitNonCombatTriggers(triggers);
}

void ArcaneMageCcStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    MageCcStrategy::InitCombatTriggers(triggers);
}

void ArcaneMageCcStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    MageCcStrategy::InitNonCombatTriggers(triggers);
}

void ArcaneMageCcPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageCcStrategy::InitCombatTriggers(triggers);
    MageCcPveStrategy::InitCombatTriggers(triggers);
}

void ArcaneMageCcPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageCcStrategy::InitNonCombatTriggers(triggers);
    MageCcPveStrategy::InitNonCombatTriggers(triggers);
}

void ArcaneMageCcPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageCcStrategy::InitCombatTriggers(triggers);
    MageCcPvpStrategy::InitCombatTriggers(triggers);
}

void ArcaneMageCcPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageCcStrategy::InitNonCombatTriggers(triggers);
    MageCcPvpStrategy::InitNonCombatTriggers(triggers);
}

void ArcaneMageCcRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageCcStrategy::InitCombatTriggers(triggers);
    MageCcRaidStrategy::InitCombatTriggers(triggers);
}

void ArcaneMageCcRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageCcStrategy::InitNonCombatTriggers(triggers);
    MageCcRaidStrategy::InitNonCombatTriggers(triggers);
}

void ArcaneMageCureStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    MageCureStrategy::InitCombatTriggers(triggers);
}

void ArcaneMageCureStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    MageCureStrategy::InitNonCombatTriggers(triggers);
}

void ArcaneMageCurePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageCureStrategy::InitCombatTriggers(triggers);
    MageCurePveStrategy::InitCombatTriggers(triggers);
}

void ArcaneMageCurePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageCureStrategy::InitNonCombatTriggers(triggers);
    MageCurePveStrategy::InitNonCombatTriggers(triggers);
}

void ArcaneMageCurePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageCureStrategy::InitCombatTriggers(triggers);
    MageCurePvpStrategy::InitCombatTriggers(triggers);
}

void ArcaneMageCurePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageCureStrategy::InitNonCombatTriggers(triggers);
    MageCurePvpStrategy::InitNonCombatTriggers(triggers);
}

void ArcaneMageCureRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageCureStrategy::InitCombatTriggers(triggers);
    MageCureRaidStrategy::InitCombatTriggers(triggers);
}

void ArcaneMageCureRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageCureStrategy::InitNonCombatTriggers(triggers);
    MageCureRaidStrategy::InitNonCombatTriggers(triggers);
}

#endif
#ifdef MANGOSBOT_ONE // TBC

NextAction** ArcaneMageStrategy::GetDefaultCombatActions()
{
    return NextAction::array(0, new NextAction("arcane missiles", ACTION_IDLE), NULL);
}

void ArcaneMageStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    MageStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "arcane blast",
        NextAction::array(0, new NextAction("arcane blast", ACTION_NORMAL), NULL)));
}

void ArcaneMageStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    MageStrategy::InitNonCombatTriggers(triggers);
}

void ArcaneMageStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    MageStrategy::InitReactionTriggers(triggers);
}

void ArcaneMageStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    MageStrategy::InitDeadTriggers(triggers);
}

void ArcaneMagePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageStrategy::InitCombatTriggers(triggers);
    MagePveStrategy::InitCombatTriggers(triggers);
}

void ArcaneMagePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageStrategy::InitNonCombatTriggers(triggers);
    MagePveStrategy::InitNonCombatTriggers(triggers);
}

void ArcaneMagePveStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageStrategy::InitReactionTriggers(triggers);
    MagePveStrategy::InitReactionTriggers(triggers);
}

void ArcaneMagePveStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageStrategy::InitDeadTriggers(triggers);
    MagePveStrategy::InitDeadTriggers(triggers);
}

void ArcaneMagePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageStrategy::InitCombatTriggers(triggers);
    MagePvpStrategy::InitCombatTriggers(triggers);
}

void ArcaneMagePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageStrategy::InitNonCombatTriggers(triggers);
    MagePvpStrategy::InitNonCombatTriggers(triggers);
}

void ArcaneMagePvpStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageStrategy::InitReactionTriggers(triggers);
    MagePvpStrategy::InitReactionTriggers(triggers);
}

void ArcaneMagePvpStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageStrategy::InitDeadTriggers(triggers);
    MagePvpStrategy::InitDeadTriggers(triggers);
}

void ArcaneMageRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageStrategy::InitCombatTriggers(triggers);
    MageRaidStrategy::InitCombatTriggers(triggers);
}

void ArcaneMageRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageStrategy::InitNonCombatTriggers(triggers);
    MageRaidStrategy::InitNonCombatTriggers(triggers);
}

void ArcaneMageRaidStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageStrategy::InitReactionTriggers(triggers);
    MageRaidStrategy::InitReactionTriggers(triggers);
}

void ArcaneMageRaidStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageStrategy::InitDeadTriggers(triggers);
    MageRaidStrategy::InitDeadTriggers(triggers);
}

void ArcaneMageAoeStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    MageAoeStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "enemy too close for spell",
        NextAction::array(0, new NextAction("arcane explosion", ACTION_HIGH), NULL)));
}

void ArcaneMageAoeStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    MageAoeStrategy::InitNonCombatTriggers(triggers);
}

void ArcaneMageAoePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageAoeStrategy::InitCombatTriggers(triggers);
    MageAoePveStrategy::InitCombatTriggers(triggers);
}

void ArcaneMageAoePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageAoeStrategy::InitNonCombatTriggers(triggers);
    MageAoePveStrategy::InitNonCombatTriggers(triggers);
}

void ArcaneMageAoePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageAoeStrategy::InitCombatTriggers(triggers);
    MageAoePvpStrategy::InitCombatTriggers(triggers);
}

void ArcaneMageAoePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageAoeStrategy::InitNonCombatTriggers(triggers);
    MageAoePvpStrategy::InitNonCombatTriggers(triggers);
}

void ArcaneMageAoeRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageAoeStrategy::InitCombatTriggers(triggers);
    MageAoeRaidStrategy::InitCombatTriggers(triggers);
}

void ArcaneMageAoeRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageAoeStrategy::InitNonCombatTriggers(triggers);
    MageAoeRaidStrategy::InitNonCombatTriggers(triggers);
}

void ArcaneMageBuffStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    MageBuffStrategy::InitCombatTriggers(triggers);
}

void ArcaneMageBuffStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    MageBuffStrategy::InitNonCombatTriggers(triggers);
}

void ArcaneMageBuffPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageBuffStrategy::InitCombatTriggers(triggers);
    MageBuffPveStrategy::InitCombatTriggers(triggers);
}

void ArcaneMageBuffPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageBuffStrategy::InitNonCombatTriggers(triggers);
    MageBuffPveStrategy::InitNonCombatTriggers(triggers);
}

void ArcaneMageBuffPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageBuffStrategy::InitCombatTriggers(triggers);
    MageBuffPvpStrategy::InitCombatTriggers(triggers);
}

void ArcaneMageBuffPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageBuffStrategy::InitNonCombatTriggers(triggers);
    MageBuffPvpStrategy::InitNonCombatTriggers(triggers);
}

void ArcaneMageBuffRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageBuffStrategy::InitCombatTriggers(triggers);
    MageBuffRaidStrategy::InitCombatTriggers(triggers);
}

void ArcaneMageBuffRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageBuffStrategy::InitNonCombatTriggers(triggers);
    MageBuffRaidStrategy::InitNonCombatTriggers(triggers);
}

void ArcaneMageBoostStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    MageBoostStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "arcane power",
        NextAction::array(0, new NextAction("arcane power", ACTION_HIGH + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "presence of mind",
        NextAction::array(0, new NextAction("presence of mind", ACTION_HIGH + 1), NULL)));
}

void ArcaneMageBoostStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    MageBoostStrategy::InitNonCombatTriggers(triggers);
}

void ArcaneMageBoostPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageBoostStrategy::InitCombatTriggers(triggers);
    MageBoostPveStrategy::InitCombatTriggers(triggers);
}

void ArcaneMageBoostPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageBoostStrategy::InitNonCombatTriggers(triggers);
    MageBoostPveStrategy::InitNonCombatTriggers(triggers);
}

void ArcaneMageBoostPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageBoostStrategy::InitCombatTriggers(triggers);
    MageBoostPvpStrategy::InitCombatTriggers(triggers);
}

void ArcaneMageBoostPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageBoostStrategy::InitNonCombatTriggers(triggers);
    MageBoostPvpStrategy::InitNonCombatTriggers(triggers);
}

void ArcaneMageBoostRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageBoostStrategy::InitCombatTriggers(triggers);
    MageBoostRaidStrategy::InitCombatTriggers(triggers);
}

void ArcaneMageBoostRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageBoostStrategy::InitNonCombatTriggers(triggers);
    MageBoostRaidStrategy::InitNonCombatTriggers(triggers);
}

void ArcaneMageCcStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    MageCcStrategy::InitCombatTriggers(triggers);
}

void ArcaneMageCcStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    MageCcStrategy::InitNonCombatTriggers(triggers);
}

void ArcaneMageCcPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageCcStrategy::InitCombatTriggers(triggers);
    MageCcPveStrategy::InitCombatTriggers(triggers);
}

void ArcaneMageCcPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageCcStrategy::InitNonCombatTriggers(triggers);
    MageCcPveStrategy::InitNonCombatTriggers(triggers);
}

void ArcaneMageCcPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageCcStrategy::InitCombatTriggers(triggers);
    MageCcPvpStrategy::InitCombatTriggers(triggers);
}

void ArcaneMageCcPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageCcStrategy::InitNonCombatTriggers(triggers);
    MageCcPvpStrategy::InitNonCombatTriggers(triggers);
}

void ArcaneMageCcRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageCcStrategy::InitCombatTriggers(triggers);
    MageCcRaidStrategy::InitCombatTriggers(triggers);
}

void ArcaneMageCcRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageCcStrategy::InitNonCombatTriggers(triggers);
    MageCcRaidStrategy::InitNonCombatTriggers(triggers);
}

void ArcaneMageCureStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    MageCureStrategy::InitCombatTriggers(triggers);
}

void ArcaneMageCureStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    MageCureStrategy::InitNonCombatTriggers(triggers);
}

void ArcaneMageCurePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageCureStrategy::InitCombatTriggers(triggers);
    MageCurePveStrategy::InitCombatTriggers(triggers);
}

void ArcaneMageCurePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageCureStrategy::InitNonCombatTriggers(triggers);
    MageCurePveStrategy::InitNonCombatTriggers(triggers);
}

void ArcaneMageCurePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageCureStrategy::InitCombatTriggers(triggers);
    MageCurePvpStrategy::InitCombatTriggers(triggers);
}

void ArcaneMageCurePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageCureStrategy::InitNonCombatTriggers(triggers);
    MageCurePvpStrategy::InitNonCombatTriggers(triggers);
}

void ArcaneMageCureRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageCureStrategy::InitCombatTriggers(triggers);
    MageCureRaidStrategy::InitCombatTriggers(triggers);
}

void ArcaneMageCureRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageCureStrategy::InitNonCombatTriggers(triggers);
    MageCureRaidStrategy::InitNonCombatTriggers(triggers);
}

#endif
#ifdef MANGOSBOT_TWO // WOTLK

NextAction** ArcaneMageStrategy::GetDefaultCombatActions()
{
    return NextAction::array(0, new NextAction("arcane missiles", ACTION_IDLE), NULL);
}

void ArcaneMageStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    MageStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "arcane blast",
        NextAction::array(0, new NextAction("arcane blast", ACTION_NORMAL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "missile barrage",
        NextAction::array(0, new NextAction("arcane missiles", ACTION_NORMAL), NULL)));
}

void ArcaneMageStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    MageStrategy::InitNonCombatTriggers(triggers);
}

void ArcaneMageStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    MageStrategy::InitReactionTriggers(triggers);
}

void ArcaneMageStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    MageStrategy::InitDeadTriggers(triggers);
}

void ArcaneMagePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageStrategy::InitCombatTriggers(triggers);
    MagePveStrategy::InitCombatTriggers(triggers);
}

void ArcaneMagePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageStrategy::InitNonCombatTriggers(triggers);
    MagePveStrategy::InitNonCombatTriggers(triggers);
}

void ArcaneMagePveStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageStrategy::InitReactionTriggers(triggers);
    MagePveStrategy::InitReactionTriggers(triggers);
}

void ArcaneMagePveStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageStrategy::InitDeadTriggers(triggers);
    MagePveStrategy::InitDeadTriggers(triggers);
}

void ArcaneMagePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageStrategy::InitCombatTriggers(triggers);
    MagePvpStrategy::InitCombatTriggers(triggers);
}

void ArcaneMagePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageStrategy::InitNonCombatTriggers(triggers);
    MagePvpStrategy::InitNonCombatTriggers(triggers);
}

void ArcaneMagePvpStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageStrategy::InitReactionTriggers(triggers);
    MagePvpStrategy::InitReactionTriggers(triggers);
}

void ArcaneMagePvpStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageStrategy::InitDeadTriggers(triggers);
    MagePvpStrategy::InitDeadTriggers(triggers);
}

void ArcaneMageRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageStrategy::InitCombatTriggers(triggers);
    MageRaidStrategy::InitCombatTriggers(triggers);
}

void ArcaneMageRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageStrategy::InitNonCombatTriggers(triggers);
    MageRaidStrategy::InitNonCombatTriggers(triggers);
}

void ArcaneMageRaidStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageStrategy::InitReactionTriggers(triggers);
    MageRaidStrategy::InitReactionTriggers(triggers);
}

void ArcaneMageRaidStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageStrategy::InitDeadTriggers(triggers);
    MageRaidStrategy::InitDeadTriggers(triggers);
}

void ArcaneMageAoeStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    MageAoeStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "enemy too close for spell",
        NextAction::array(0, new NextAction("arcane explosion", ACTION_HIGH), NULL)));
}

void ArcaneMageAoeStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    MageAoeStrategy::InitNonCombatTriggers(triggers);
}

void ArcaneMageAoePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageAoeStrategy::InitCombatTriggers(triggers);
    MageAoePveStrategy::InitCombatTriggers(triggers);
}

void ArcaneMageAoePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageAoeStrategy::InitNonCombatTriggers(triggers);
    MageAoePveStrategy::InitNonCombatTriggers(triggers);
}

void ArcaneMageAoePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageAoeStrategy::InitCombatTriggers(triggers);
    MageAoePvpStrategy::InitCombatTriggers(triggers);
}

void ArcaneMageAoePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageAoeStrategy::InitNonCombatTriggers(triggers);
    MageAoePvpStrategy::InitNonCombatTriggers(triggers);
}

void ArcaneMageAoeRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageAoeStrategy::InitCombatTriggers(triggers);
    MageAoeRaidStrategy::InitCombatTriggers(triggers);
}

void ArcaneMageAoeRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageAoeStrategy::InitNonCombatTriggers(triggers);
    MageAoeRaidStrategy::InitNonCombatTriggers(triggers);
}

void ArcaneMageBuffStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    MageBuffStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "arcane power",
        NextAction::array(0, new NextAction("arcane power", ACTION_HIGH + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "presence of mind",
        NextAction::array(0, new NextAction("presence of mind", ACTION_HIGH + 1), NULL)));
}

void ArcaneMageBuffStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    MageBuffStrategy::InitNonCombatTriggers(triggers);
}

void ArcaneMageBuffPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageBuffStrategy::InitCombatTriggers(triggers);
    MageBuffPveStrategy::InitCombatTriggers(triggers);
}

void ArcaneMageBuffPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageBuffStrategy::InitNonCombatTriggers(triggers);
    MageBuffPveStrategy::InitNonCombatTriggers(triggers);
}

void ArcaneMageBuffPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageBuffStrategy::InitCombatTriggers(triggers);
    MageBuffPvpStrategy::InitCombatTriggers(triggers);
}

void ArcaneMageBuffPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageBuffStrategy::InitNonCombatTriggers(triggers);
    MageBuffPvpStrategy::InitNonCombatTriggers(triggers);
}

void ArcaneMageBuffRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageBuffStrategy::InitCombatTriggers(triggers);
    MageBuffRaidStrategy::InitCombatTriggers(triggers);
}

void ArcaneMageBuffRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageBuffStrategy::InitNonCombatTriggers(triggers);
    MageBuffRaidStrategy::InitNonCombatTriggers(triggers);
}

void ArcaneMageBoostStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    MageBoostStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "arcane power",
        NextAction::array(0, new NextAction("arcane power", ACTION_HIGH + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "presence of mind",
        NextAction::array(0, new NextAction("presence of mind", ACTION_HIGH + 1), NULL)));
}

void ArcaneMageBoostStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    MageBoostStrategy::InitNonCombatTriggers(triggers);
}

void ArcaneMageBoostPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageBoostStrategy::InitCombatTriggers(triggers);
    MageBoostPveStrategy::InitCombatTriggers(triggers);
}

void ArcaneMageBoostPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageBoostStrategy::InitNonCombatTriggers(triggers);
    MageBoostPveStrategy::InitNonCombatTriggers(triggers);
}

void ArcaneMageBoostPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageBoostStrategy::InitCombatTriggers(triggers);
    MageBoostPvpStrategy::InitCombatTriggers(triggers);
}

void ArcaneMageBoostPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageBoostStrategy::InitNonCombatTriggers(triggers);
    MageBoostPvpStrategy::InitNonCombatTriggers(triggers);
}

void ArcaneMageBoostRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageBoostStrategy::InitCombatTriggers(triggers);
    MageBoostRaidStrategy::InitCombatTriggers(triggers);
}

void ArcaneMageBoostRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageBoostStrategy::InitNonCombatTriggers(triggers);
    MageBoostRaidStrategy::InitNonCombatTriggers(triggers);
}

void ArcaneMageCcStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    MageCcStrategy::InitCombatTriggers(triggers);
}

void ArcaneMageCcStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    MageCcStrategy::InitNonCombatTriggers(triggers);
}

void ArcaneMageCcPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageCcStrategy::InitCombatTriggers(triggers);
    MageCcPveStrategy::InitCombatTriggers(triggers);
}

void ArcaneMageCcPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageCcStrategy::InitNonCombatTriggers(triggers);
    MageCcPveStrategy::InitNonCombatTriggers(triggers);
}

void ArcaneMageCcPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageCcStrategy::InitCombatTriggers(triggers);
    MageCcPvpStrategy::InitCombatTriggers(triggers);
}

void ArcaneMageCcPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageCcStrategy::InitNonCombatTriggers(triggers);
    MageCcPvpStrategy::InitNonCombatTriggers(triggers);
}

void ArcaneMageCcRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageCcStrategy::InitCombatTriggers(triggers);
    MageCcRaidStrategy::InitCombatTriggers(triggers);
}

void ArcaneMageCcRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageCcStrategy::InitNonCombatTriggers(triggers);
    MageCcRaidStrategy::InitNonCombatTriggers(triggers);
}

void ArcaneMageCureStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    MageCureStrategy::InitCombatTriggers(triggers);
}

void ArcaneMageCureStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    MageCureStrategy::InitNonCombatTriggers(triggers);
}

void ArcaneMageCurePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageCureStrategy::InitCombatTriggers(triggers);
    MageCurePveStrategy::InitCombatTriggers(triggers);
}

void ArcaneMageCurePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageCureStrategy::InitNonCombatTriggers(triggers);
    MageCurePveStrategy::InitNonCombatTriggers(triggers);
}

void ArcaneMageCurePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageCureStrategy::InitCombatTriggers(triggers);
    MageCurePvpStrategy::InitCombatTriggers(triggers);
}

void ArcaneMageCurePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageCureStrategy::InitNonCombatTriggers(triggers);
    MageCurePvpStrategy::InitNonCombatTriggers(triggers);
}

void ArcaneMageCureRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageCureStrategy::InitCombatTriggers(triggers);
    MageCureRaidStrategy::InitCombatTriggers(triggers);
}

void ArcaneMageCureRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArcaneMageCureStrategy::InitNonCombatTriggers(triggers);
    MageCureRaidStrategy::InitNonCombatTriggers(triggers);
}

#endif