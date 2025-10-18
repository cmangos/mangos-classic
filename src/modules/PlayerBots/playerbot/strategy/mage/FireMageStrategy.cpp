
#include "playerbot/playerbot.h"
#include "MageMultipliers.h"
#include "FireMageStrategy.h"

using namespace ai;

class FireMageStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    FireMageStrategyActionNodeFactory()
    {
        creators["fire blast"] = &fire_blast;
    }
private:
    ACTION_NODE_A(fire_blast, "fire blast", "scorch");
};

FireMageStrategy::FireMageStrategy(PlayerbotAI* ai) : MageStrategy(ai)
{
    actionNodeFactories.Add(new FireMageStrategyActionNodeFactory());
}

#ifdef MANGOSBOT_ZERO // Vanilla

NextAction** FireMageStrategy::GetDefaultCombatActions()
{
    return NextAction::array(0, new NextAction("fireball", ACTION_IDLE), NULL);
}

void FireMageStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    MageStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "no fire vulnerability",
        NextAction::array(0, new NextAction("scorch", ACTION_NORMAL + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "pyroblast",
        NextAction::array(0, new NextAction("pyroblast", ACTION_NORMAL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "fire spells locked",
        NextAction::array(0, new NextAction("frostbolt", ACTION_NORMAL), NULL)));
}

void FireMageStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    MageStrategy::InitNonCombatTriggers(triggers);
}

void FireMageStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    MageStrategy::InitReactionTriggers(triggers);
}

void FireMageStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    MageStrategy::InitDeadTriggers(triggers);
}

void FireMagePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageStrategy::InitCombatTriggers(triggers);
    MagePveStrategy::InitCombatTriggers(triggers);
}

void FireMagePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageStrategy::InitNonCombatTriggers(triggers);
    MagePveStrategy::InitNonCombatTriggers(triggers);
}

void FireMagePveStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageStrategy::InitReactionTriggers(triggers);
    MagePveStrategy::InitReactionTriggers(triggers);
}

void FireMagePveStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageStrategy::InitDeadTriggers(triggers);
    MagePveStrategy::InitDeadTriggers(triggers);
}

void FireMagePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageStrategy::InitCombatTriggers(triggers);
    MagePvpStrategy::InitCombatTriggers(triggers);
}

void FireMagePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageStrategy::InitNonCombatTriggers(triggers);
    MagePvpStrategy::InitNonCombatTriggers(triggers);
}

void FireMagePvpStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageStrategy::InitReactionTriggers(triggers);
    MagePvpStrategy::InitReactionTriggers(triggers);
}

void FireMagePvpStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageStrategy::InitDeadTriggers(triggers);
    MagePvpStrategy::InitDeadTriggers(triggers);
}

void FireMageRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageStrategy::InitCombatTriggers(triggers);
    MageRaidStrategy::InitCombatTriggers(triggers);
}

void FireMageRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageStrategy::InitNonCombatTriggers(triggers);
    MageRaidStrategy::InitNonCombatTriggers(triggers);
}

void FireMageRaidStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageStrategy::InitReactionTriggers(triggers);
    MageRaidStrategy::InitReactionTriggers(triggers);
}

void FireMageRaidStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageStrategy::InitDeadTriggers(triggers);
    MageRaidStrategy::InitDeadTriggers(triggers);
}

void FireMageAoeStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    MageAoeStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "ranged medium aoe",
        NextAction::array(0, new NextAction("flamestrike", ACTION_HIGH + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "fire spells locked",
        NextAction::array(0, new NextAction("blizzard", ACTION_HIGH), NULL)));
}

void FireMageAoeStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    MageAoeStrategy::InitNonCombatTriggers(triggers);
}

void FireMageAoePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageAoeStrategy::InitCombatTriggers(triggers);
    MageAoePveStrategy::InitCombatTriggers(triggers);
}

void FireMageAoePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageAoeStrategy::InitNonCombatTriggers(triggers);
    MageAoePveStrategy::InitNonCombatTriggers(triggers);
}

void FireMageAoePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageAoeStrategy::InitCombatTriggers(triggers);
    MageAoePvpStrategy::InitCombatTriggers(triggers);
}

void FireMageAoePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageAoeStrategy::InitNonCombatTriggers(triggers);
    MageAoePvpStrategy::InitNonCombatTriggers(triggers);
}

void FireMageAoeRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageAoeStrategy::InitCombatTriggers(triggers);
    MageAoeRaidStrategy::InitCombatTriggers(triggers);
}

void FireMageAoeRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageAoeStrategy::InitNonCombatTriggers(triggers);
    MageAoeRaidStrategy::InitNonCombatTriggers(triggers);
}

void FireMageBuffStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    MageBuffStrategy::InitCombatTriggers(triggers);
}

void FireMageBuffStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    MageBuffStrategy::InitNonCombatTriggers(triggers);
}

void FireMageBuffPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageBuffStrategy::InitCombatTriggers(triggers);
    MageBuffPveStrategy::InitCombatTriggers(triggers);
}

void FireMageBuffPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageBuffStrategy::InitNonCombatTriggers(triggers);
    MageBuffPveStrategy::InitNonCombatTriggers(triggers);
}

void FireMageBuffPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageBuffStrategy::InitCombatTriggers(triggers);
    MageBuffPvpStrategy::InitCombatTriggers(triggers);
}

void FireMageBuffPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageBuffStrategy::InitNonCombatTriggers(triggers);
    MageBuffPvpStrategy::InitNonCombatTriggers(triggers);
}

void FireMageBuffRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageBuffStrategy::InitCombatTriggers(triggers);
    MageBuffRaidStrategy::InitCombatTriggers(triggers);
}

void FireMageBuffRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageBuffStrategy::InitNonCombatTriggers(triggers);
    MageBuffRaidStrategy::InitNonCombatTriggers(triggers);
}

void FireMageBoostStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    MageBoostStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "combustion",
        NextAction::array(0, new NextAction("combustion", ACTION_HIGH + 1), NULL)));
}

void FireMageBoostStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    MageBoostStrategy::InitNonCombatTriggers(triggers);
}

void FireMageBoostPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageBoostStrategy::InitCombatTriggers(triggers);
    MageBoostPveStrategy::InitCombatTriggers(triggers);
}

void FireMageBoostPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageBoostStrategy::InitNonCombatTriggers(triggers);
    MageBoostPveStrategy::InitNonCombatTriggers(triggers);
}

void FireMageBoostPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageBoostStrategy::InitCombatTriggers(triggers);
    MageBoostPvpStrategy::InitCombatTriggers(triggers);
}

void FireMageBoostPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageBoostStrategy::InitNonCombatTriggers(triggers);
    MageBoostPvpStrategy::InitNonCombatTriggers(triggers);
}

void FireMageBoostRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageBoostStrategy::InitCombatTriggers(triggers);
    MageBoostRaidStrategy::InitCombatTriggers(triggers);
}

void FireMageBoostRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageBoostStrategy::InitNonCombatTriggers(triggers);
    MageBoostRaidStrategy::InitNonCombatTriggers(triggers);
}

void FireMageCcStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    MageCcStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "enemy ten yards",
        NextAction::array(0, new NextAction("blast wave", ACTION_INTERRUPT), NULL)));
}

void FireMageCcStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    MageCcStrategy::InitNonCombatTriggers(triggers);
}

void FireMageCcPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageCcStrategy::InitCombatTriggers(triggers);
    MageCcPveStrategy::InitCombatTriggers(triggers);
}

void FireMageCcPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageCcStrategy::InitNonCombatTriggers(triggers);
    MageCcPveStrategy::InitNonCombatTriggers(triggers);
}

void FireMageCcPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageCcStrategy::InitCombatTriggers(triggers);
    MageCcPvpStrategy::InitCombatTriggers(triggers);
}

void FireMageCcPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageCcStrategy::InitNonCombatTriggers(triggers);
    MageCcPvpStrategy::InitNonCombatTriggers(triggers);
}

void FireMageCcRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageCcStrategy::InitCombatTriggers(triggers);
    MageCcRaidStrategy::InitCombatTriggers(triggers);
}

void FireMageCcRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageCcStrategy::InitNonCombatTriggers(triggers);
    MageCcRaidStrategy::InitNonCombatTriggers(triggers);
}

void FireMageCureStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    MageCureStrategy::InitCombatTriggers(triggers);
}

void FireMageCureStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    MageCureStrategy::InitNonCombatTriggers(triggers);
}

void FireMageCurePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageCureStrategy::InitCombatTriggers(triggers);
    MageCurePveStrategy::InitCombatTriggers(triggers);
}

void FireMageCurePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageCureStrategy::InitNonCombatTriggers(triggers);
    MageCurePveStrategy::InitNonCombatTriggers(triggers);
}

void FireMageCurePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageCureStrategy::InitCombatTriggers(triggers);
    MageCurePvpStrategy::InitCombatTriggers(triggers);
}

void FireMageCurePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageCureStrategy::InitNonCombatTriggers(triggers);
    MageCurePvpStrategy::InitNonCombatTriggers(triggers);
}

void FireMageCureRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageCureStrategy::InitCombatTriggers(triggers);
    MageCureRaidStrategy::InitCombatTriggers(triggers);
}

void FireMageCureRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageCureStrategy::InitNonCombatTriggers(triggers);
    MageCureRaidStrategy::InitNonCombatTriggers(triggers);
}

#endif
#ifdef MANGOSBOT_ONE // TBC

NextAction** FireMageStrategy::GetDefaultCombatActions()
{
    return NextAction::array(0, new NextAction("fireball", ACTION_IDLE), NULL);
}

void FireMageStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    MageStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "no fire vulnerability",
        NextAction::array(0, new NextAction("scorch", ACTION_NORMAL + 4), NULL)));

    triggers.push_back(new TriggerNode(
        "living bomb",
        NextAction::array(0, new NextAction("living bomb", ACTION_NORMAL + 3), NULL)));

    triggers.push_back(new TriggerNode(
        "fire spells locked",
        NextAction::array(0, new NextAction("frostbolt", ACTION_NORMAL), NULL)));
}

void FireMageStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    MageStrategy::InitNonCombatTriggers(triggers);
}

void FireMageStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    MageStrategy::InitReactionTriggers(triggers);
}

void FireMageStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    MageStrategy::InitDeadTriggers(triggers);
}

void FireMagePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageStrategy::InitCombatTriggers(triggers);
    MagePveStrategy::InitCombatTriggers(triggers);
}

void FireMagePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageStrategy::InitNonCombatTriggers(triggers);
    MagePveStrategy::InitNonCombatTriggers(triggers);
}

void FireMagePveStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageStrategy::InitReactionTriggers(triggers);
    MagePveStrategy::InitReactionTriggers(triggers);
}

void FireMagePveStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageStrategy::InitDeadTriggers(triggers);
    MagePveStrategy::InitDeadTriggers(triggers);
}

void FireMagePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageStrategy::InitCombatTriggers(triggers);
    MagePvpStrategy::InitCombatTriggers(triggers);
}

void FireMagePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageStrategy::InitNonCombatTriggers(triggers);
    MagePvpStrategy::InitNonCombatTriggers(triggers);
}

void FireMagePvpStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageStrategy::InitReactionTriggers(triggers);
    MagePvpStrategy::InitReactionTriggers(triggers);
}

void FireMagePvpStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageStrategy::InitDeadTriggers(triggers);
    MagePvpStrategy::InitDeadTriggers(triggers);
}

void FireMageRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageStrategy::InitCombatTriggers(triggers);
    MageRaidStrategy::InitCombatTriggers(triggers);
}

void FireMageRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageStrategy::InitNonCombatTriggers(triggers);
    MageRaidStrategy::InitNonCombatTriggers(triggers);
}

void FireMageRaidStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageStrategy::InitReactionTriggers(triggers);
    MageRaidStrategy::InitReactionTriggers(triggers);
}

void FireMageRaidStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageStrategy::InitDeadTriggers(triggers);
    MageRaidStrategy::InitDeadTriggers(triggers);
}

void FireMageAoeStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    MageAoeStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "living bomb",
        NextAction::array(0, new NextAction("living bomb", ACTION_HIGH + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "ranged medium aoe",
        NextAction::array(0, new NextAction("flamestrike", ACTION_HIGH + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "fire spells locked",
        NextAction::array(0, new NextAction("blizzard", ACTION_HIGH), NULL)));
}

void FireMageAoeStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    MageAoeStrategy::InitNonCombatTriggers(triggers);
}

void FireMageAoePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageAoeStrategy::InitCombatTriggers(triggers);
    MageAoePveStrategy::InitCombatTriggers(triggers);
}

void FireMageAoePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageAoeStrategy::InitNonCombatTriggers(triggers);
    MageAoePveStrategy::InitNonCombatTriggers(triggers);
}

void FireMageAoePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageAoeStrategy::InitCombatTriggers(triggers);
    MageAoePvpStrategy::InitCombatTriggers(triggers);
}

void FireMageAoePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageAoeStrategy::InitNonCombatTriggers(triggers);
    MageAoePvpStrategy::InitNonCombatTriggers(triggers);
}

void FireMageAoeRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageAoeStrategy::InitCombatTriggers(triggers);
    MageAoeRaidStrategy::InitCombatTriggers(triggers);
}

void FireMageAoeRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageAoeStrategy::InitNonCombatTriggers(triggers);
    MageAoeRaidStrategy::InitNonCombatTriggers(triggers);
}

void FireMageBuffStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    MageBuffStrategy::InitCombatTriggers(triggers);
}

void FireMageBuffStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    MageBuffStrategy::InitNonCombatTriggers(triggers);
}

void FireMageBuffPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageBuffStrategy::InitCombatTriggers(triggers);
    MageBuffPveStrategy::InitCombatTriggers(triggers);
}

void FireMageBuffPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageBuffStrategy::InitNonCombatTriggers(triggers);
    MageBuffPveStrategy::InitNonCombatTriggers(triggers);
}

void FireMageBuffPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageBuffStrategy::InitCombatTriggers(triggers);
    MageBuffPvpStrategy::InitCombatTriggers(triggers);
}

void FireMageBuffPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageBuffStrategy::InitNonCombatTriggers(triggers);
    MageBuffPvpStrategy::InitNonCombatTriggers(triggers);
}

void FireMageBuffRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageBuffStrategy::InitCombatTriggers(triggers);
    MageBuffRaidStrategy::InitCombatTriggers(triggers);
}

void FireMageBuffRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageBuffStrategy::InitNonCombatTriggers(triggers);
    MageBuffRaidStrategy::InitNonCombatTriggers(triggers);
}

void FireMageBoostStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    MageBoostStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "combustion",
        NextAction::array(0, new NextAction("combustion", ACTION_HIGH + 1), NULL)));
}

void FireMageBoostStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    MageBoostStrategy::InitNonCombatTriggers(triggers);
}

void FireMageBoostPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageBoostStrategy::InitCombatTriggers(triggers);
    MageBoostPveStrategy::InitCombatTriggers(triggers);
}

void FireMageBoostPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageBoostStrategy::InitNonCombatTriggers(triggers);
    MageBoostPveStrategy::InitNonCombatTriggers(triggers);
}

void FireMageBoostPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageBoostStrategy::InitCombatTriggers(triggers);
    MageBoostPvpStrategy::InitCombatTriggers(triggers);
}

void FireMageBoostPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageBoostStrategy::InitNonCombatTriggers(triggers);
    MageBoostPvpStrategy::InitNonCombatTriggers(triggers);
}

void FireMageBoostRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageBoostStrategy::InitCombatTriggers(triggers);
    MageBoostRaidStrategy::InitCombatTriggers(triggers);
}

void FireMageBoostRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageBoostStrategy::InitNonCombatTriggers(triggers);
    MageBoostRaidStrategy::InitNonCombatTriggers(triggers);
}

void FireMageCcStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    MageCcStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "enemy too close for spell",
        NextAction::array(0, new NextAction("dragon's breath", ACTION_EMERGENCY), NULL)));

    triggers.push_back(new TriggerNode(
        "enemy ten yards",
        NextAction::array(0, new NextAction("blast wave", ACTION_INTERRUPT), NULL)));
}

void FireMageCcStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    MageCcStrategy::InitNonCombatTriggers(triggers);
}

void FireMageCcPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageCcStrategy::InitCombatTriggers(triggers);
    MageCcPveStrategy::InitCombatTriggers(triggers);
}

void FireMageCcPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageCcStrategy::InitNonCombatTriggers(triggers);
    MageCcPveStrategy::InitNonCombatTriggers(triggers);
}

void FireMageCcPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageCcStrategy::InitCombatTriggers(triggers);
    MageCcPvpStrategy::InitCombatTriggers(triggers);
}

void FireMageCcPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageCcStrategy::InitNonCombatTriggers(triggers);
    MageCcPvpStrategy::InitNonCombatTriggers(triggers);
}

void FireMageCcRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageCcStrategy::InitCombatTriggers(triggers);
    MageCcRaidStrategy::InitCombatTriggers(triggers);
}

void FireMageCcRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageCcStrategy::InitNonCombatTriggers(triggers);
    MageCcRaidStrategy::InitNonCombatTriggers(triggers);
}

void FireMageCureStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    MageCureStrategy::InitCombatTriggers(triggers);
}

void FireMageCureStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    MageCureStrategy::InitNonCombatTriggers(triggers);
}

void FireMageCurePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageCureStrategy::InitCombatTriggers(triggers);
    MageCurePveStrategy::InitCombatTriggers(triggers);
}

void FireMageCurePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageCureStrategy::InitNonCombatTriggers(triggers);
    MageCurePveStrategy::InitNonCombatTriggers(triggers);
}

void FireMageCurePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageCureStrategy::InitCombatTriggers(triggers);
    MageCurePvpStrategy::InitCombatTriggers(triggers);
}

void FireMageCurePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageCureStrategy::InitNonCombatTriggers(triggers);
    MageCurePvpStrategy::InitNonCombatTriggers(triggers);
}

void FireMageCureRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageCureStrategy::InitCombatTriggers(triggers);
    MageCureRaidStrategy::InitCombatTriggers(triggers);
}

void FireMageCureRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageCureStrategy::InitNonCombatTriggers(triggers);
    MageCureRaidStrategy::InitNonCombatTriggers(triggers);
}

#endif
#ifdef MANGOSBOT_TWO // WOTLK

NextAction** FireMageStrategy::GetDefaultCombatActions()
{
    return NextAction::array(0, new NextAction("fireball", ACTION_IDLE), NULL);
}

void FireMageStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    MageStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "living bomb",
        NextAction::array(0, new NextAction("living bomb", ACTION_NORMAL + 3), NULL)));

    triggers.push_back(new TriggerNode(
        "hot streak",
        NextAction::array(0, new NextAction("pyroblast", ACTION_NORMAL + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "no improved scorch",
        NextAction::array(0, new NextAction("scorch", ACTION_NORMAL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "fire spells locked",
        NextAction::array(0, new NextAction("frostbolt", ACTION_NORMAL), NULL)));
}

void FireMageStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    MageStrategy::InitNonCombatTriggers(triggers);
}

void FireMageStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    MageStrategy::InitReactionTriggers(triggers);
}

void FireMageStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    MageStrategy::InitDeadTriggers(triggers);
}

void FireMagePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageStrategy::InitCombatTriggers(triggers);
    MagePveStrategy::InitCombatTriggers(triggers);
}

void FireMagePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageStrategy::InitNonCombatTriggers(triggers);
    MagePveStrategy::InitNonCombatTriggers(triggers);
}

void FireMagePveStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageStrategy::InitReactionTriggers(triggers);
    MagePveStrategy::InitReactionTriggers(triggers);
}

void FireMagePveStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageStrategy::InitDeadTriggers(triggers);
    MagePveStrategy::InitDeadTriggers(triggers);
}

void FireMagePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageStrategy::InitCombatTriggers(triggers);
    MagePvpStrategy::InitCombatTriggers(triggers);
}

void FireMagePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageStrategy::InitNonCombatTriggers(triggers);
    MagePvpStrategy::InitNonCombatTriggers(triggers);
}

void FireMagePvpStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageStrategy::InitReactionTriggers(triggers);
    MagePvpStrategy::InitReactionTriggers(triggers);
}

void FireMagePvpStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageStrategy::InitDeadTriggers(triggers);
    MagePvpStrategy::InitDeadTriggers(triggers);
}

void FireMageRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageStrategy::InitCombatTriggers(triggers);
    MageRaidStrategy::InitCombatTriggers(triggers);
}

void FireMageRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageStrategy::InitNonCombatTriggers(triggers);
    MageRaidStrategy::InitNonCombatTriggers(triggers);
}

void FireMageRaidStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageStrategy::InitReactionTriggers(triggers);
    MageRaidStrategy::InitReactionTriggers(triggers);
}

void FireMageRaidStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageStrategy::InitDeadTriggers(triggers);
    MageRaidStrategy::InitDeadTriggers(triggers);
}

void FireMageAoeStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    MageAoeStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "living bomb",
        NextAction::array(0, new NextAction("living bomb", ACTION_HIGH + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "ranged medium aoe",
        NextAction::array(0, new NextAction("flamestrike", ACTION_HIGH + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "fire spells locked",
        NextAction::array(0, new NextAction("blizzard", ACTION_HIGH), NULL)));
}

void FireMageAoeStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    MageAoeStrategy::InitNonCombatTriggers(triggers);
}

void FireMageAoePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageAoeStrategy::InitCombatTriggers(triggers);
    MageAoePveStrategy::InitCombatTriggers(triggers);
}

void FireMageAoePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageAoeStrategy::InitNonCombatTriggers(triggers);
    MageAoePveStrategy::InitNonCombatTriggers(triggers);
}

void FireMageAoePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageAoeStrategy::InitCombatTriggers(triggers);
    MageAoePvpStrategy::InitCombatTriggers(triggers);
}

void FireMageAoePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageAoeStrategy::InitNonCombatTriggers(triggers);
    MageAoePvpStrategy::InitNonCombatTriggers(triggers);
}

void FireMageAoeRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageAoeStrategy::InitCombatTriggers(triggers);
    MageAoeRaidStrategy::InitCombatTriggers(triggers);
}

void FireMageAoeRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageAoeStrategy::InitNonCombatTriggers(triggers);
    MageAoeRaidStrategy::InitNonCombatTriggers(triggers);
}

void FireMageBuffStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    MageBuffStrategy::InitCombatTriggers(triggers);
}

void FireMageBuffStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    MageBuffStrategy::InitNonCombatTriggers(triggers);
}

void FireMageBuffPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageBuffStrategy::InitCombatTriggers(triggers);
    MageBuffPveStrategy::InitCombatTriggers(triggers);
}

void FireMageBuffPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageBuffStrategy::InitNonCombatTriggers(triggers);
    MageBuffPveStrategy::InitNonCombatTriggers(triggers);
}

void FireMageBuffPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageBuffStrategy::InitCombatTriggers(triggers);
    MageBuffPvpStrategy::InitCombatTriggers(triggers);
}

void FireMageBuffPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageBuffStrategy::InitNonCombatTriggers(triggers);
    MageBuffPvpStrategy::InitNonCombatTriggers(triggers);
}

void FireMageBuffRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageBuffStrategy::InitCombatTriggers(triggers);
    MageBuffRaidStrategy::InitCombatTriggers(triggers);
}

void FireMageBuffRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageBuffStrategy::InitNonCombatTriggers(triggers);
    MageBuffRaidStrategy::InitNonCombatTriggers(triggers);
}

void FireMageBoostStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    MageBoostStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "combustion",
        NextAction::array(0, new NextAction("combustion", ACTION_HIGH + 1), NULL)));
}

void FireMageBoostStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    MageBoostStrategy::InitNonCombatTriggers(triggers);
}

void FireMageBoostPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageBoostStrategy::InitCombatTriggers(triggers);
    MageBoostPveStrategy::InitCombatTriggers(triggers);
}

void FireMageBoostPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageBoostStrategy::InitNonCombatTriggers(triggers);
    MageBoostPveStrategy::InitNonCombatTriggers(triggers);
}

void FireMageBoostPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageBoostStrategy::InitCombatTriggers(triggers);
    MageBoostPvpStrategy::InitCombatTriggers(triggers);
}

void FireMageBoostPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageBoostStrategy::InitNonCombatTriggers(triggers);
    MageBoostPvpStrategy::InitNonCombatTriggers(triggers);
}

void FireMageBoostRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageBoostStrategy::InitCombatTriggers(triggers);
    MageBoostRaidStrategy::InitCombatTriggers(triggers);
}

void FireMageBoostRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageBoostStrategy::InitNonCombatTriggers(triggers);
    MageBoostRaidStrategy::InitNonCombatTriggers(triggers);
}

void FireMageCcStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    MageCcStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "enemy too close for spell",
        NextAction::array(0, new NextAction("dragon's breath", ACTION_EMERGENCY), NULL)));

    triggers.push_back(new TriggerNode(
        "enemy ten yards",
        NextAction::array(0, new NextAction("blast wave", ACTION_INTERRUPT), NULL)));
}

void FireMageCcStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    MageCcStrategy::InitNonCombatTriggers(triggers);
}

void FireMageCcPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageCcStrategy::InitCombatTriggers(triggers);
    MageCcPveStrategy::InitCombatTriggers(triggers);
}

void FireMageCcPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageCcStrategy::InitNonCombatTriggers(triggers);
    MageCcPveStrategy::InitNonCombatTriggers(triggers);
}

void FireMageCcPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageCcStrategy::InitCombatTriggers(triggers);
    MageCcPvpStrategy::InitCombatTriggers(triggers);
}

void FireMageCcPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageCcStrategy::InitNonCombatTriggers(triggers);
    MageCcPvpStrategy::InitNonCombatTriggers(triggers);
}

void FireMageCcRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageCcStrategy::InitCombatTriggers(triggers);
    MageCcRaidStrategy::InitCombatTriggers(triggers);
}

void FireMageCcRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageCcStrategy::InitNonCombatTriggers(triggers);
    MageCcRaidStrategy::InitNonCombatTriggers(triggers);
}

void FireMageCureStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    MageCureStrategy::InitCombatTriggers(triggers);
}

void FireMageCureStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    MageCureStrategy::InitNonCombatTriggers(triggers);
}

void FireMageCurePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageCureStrategy::InitCombatTriggers(triggers);
    MageCurePveStrategy::InitCombatTriggers(triggers);
}

void FireMageCurePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageCureStrategy::InitNonCombatTriggers(triggers);
    MageCurePveStrategy::InitNonCombatTriggers(triggers);
}

void FireMageCurePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageCureStrategy::InitCombatTriggers(triggers);
    MageCurePvpStrategy::InitCombatTriggers(triggers);
}

void FireMageCurePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageCureStrategy::InitNonCombatTriggers(triggers);
    MageCurePvpStrategy::InitNonCombatTriggers(triggers);
}

void FireMageCureRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageCureStrategy::InitCombatTriggers(triggers);
    MageCureRaidStrategy::InitCombatTriggers(triggers);
}

void FireMageCureRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FireMageCureStrategy::InitNonCombatTriggers(triggers);
    MageCureRaidStrategy::InitNonCombatTriggers(triggers);
}

#endif