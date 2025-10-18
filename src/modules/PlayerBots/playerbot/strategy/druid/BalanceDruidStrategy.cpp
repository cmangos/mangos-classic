
#include "playerbot/playerbot.h"
#include "DruidMultipliers.h"
#include "BalanceDruidStrategy.h"

using namespace ai;

class BalanceDruidStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    BalanceDruidStrategyActionNodeFactory()
    {
        creators["starfall"] = &starfall;
        creators["starfire"] = &starfire;
        creators["innervate"] = &innervate;
    }

private:

    static ActionNode* innervate(PlayerbotAI* ai)
    {
        return new ActionNode("innervate",
            /*P*/ NextAction::array(0, new NextAction("caster form"), NULL),
            /*A*/ NextAction::array(0, new NextAction("mana potion"), NULL),
            /*C*/ NULL);
    }

    ACTION_NODE_A(starfall, "starfall", "hurricane");

    ACTION_NODE_A(starfire, "starfire", "wrath");
};

BalanceDruidStrategy::BalanceDruidStrategy(PlayerbotAI* ai) : DruidStrategy(ai)
{
    actionNodeFactories.Add(new BalanceDruidStrategyActionNodeFactory());
}

#ifdef MANGOSBOT_ZERO // Vanilla

NextAction** BalanceDruidStrategy::GetDefaultCombatActions()
{
    return NextAction::array(0, new NextAction("wrath", ACTION_IDLE), NULL);
}

void BalanceDruidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DruidStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "rebirth",
        NextAction::array(0, new NextAction("rebirth", ACTION_EMERGENCY), NULL)));

    triggers.push_back(new TriggerNode(
        "critical health",
        NextAction::array(0, new NextAction("regrowth", ACTION_CRITICAL_HEAL + 1),
                             new NextAction("healing touch", ACTION_CRITICAL_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "low health",
        NextAction::array(0, new NextAction("regrowth", ACTION_MEDIUM_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "faerie fire",
        NextAction::array(0, new NextAction("faerie fire", ACTION_NORMAL + 3), NULL)));
}

void BalanceDruidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DruidStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "low health",
        NextAction::array(0, new NextAction("regrowth", ACTION_MEDIUM_HEAL), NULL)));
}

void BalanceDruidStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    DruidStrategy::InitReactionTriggers(triggers);
}

void BalanceDruidStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    DruidStrategy::InitDeadTriggers(triggers);
}

void BalanceDruidPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidStrategy::InitCombatTriggers(triggers);
    DruidPveStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "insect swarm",
        NextAction::array(0, new NextAction("insect swarm", ACTION_NORMAL + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "moonfire",
        NextAction::array(0, new NextAction("moonfire", ACTION_NORMAL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "very often",
        NextAction::array(0, new NextAction("starfire", ACTION_NORMAL), NULL)));
}

void BalanceDruidPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidStrategy::InitNonCombatTriggers(triggers);
    DruidPveStrategy::InitNonCombatTriggers(triggers);
}

void BalanceDruidPveStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidStrategy::InitReactionTriggers(triggers);
    DruidPveStrategy::InitReactionTriggers(triggers);
}

void BalanceDruidPveStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidStrategy::InitDeadTriggers(triggers);
    DruidPveStrategy::InitDeadTriggers(triggers);
}

void BalanceDruidPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidStrategy::InitCombatTriggers(triggers);
    DruidPvpStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "insect swarm",
        NextAction::array(0, new NextAction("insect swarm", ACTION_NORMAL + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "moonfire",
        NextAction::array(0, new NextAction("moonfire", ACTION_NORMAL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "very often",
        NextAction::array(0, new NextAction("starfire", ACTION_NORMAL), NULL)));
}

void BalanceDruidPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidStrategy::InitNonCombatTriggers(triggers);
    DruidPvpStrategy::InitNonCombatTriggers(triggers);
}

void BalanceDruidPvpStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidStrategy::InitReactionTriggers(triggers);
    DruidPvpStrategy::InitReactionTriggers(triggers);
}

void BalanceDruidPvpStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidStrategy::InitDeadTriggers(triggers);
    DruidPvpStrategy::InitDeadTriggers(triggers);
}

ai::NextAction** BalanceDruidRaidStrategy::GetDefaultCombatActions()
{
    return NextAction::array(0, new NextAction("starfire", ACTION_IDLE), NULL);
}

void BalanceDruidRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidStrategy::InitCombatTriggers(triggers);
    DruidRaidStrategy::InitCombatTriggers(triggers);
}

void BalanceDruidRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidStrategy::InitNonCombatTriggers(triggers);
    DruidRaidStrategy::InitNonCombatTriggers(triggers);
}

void BalanceDruidRaidStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidStrategy::InitReactionTriggers(triggers);
    DruidRaidStrategy::InitReactionTriggers(triggers);
}

void BalanceDruidRaidStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidStrategy::InitDeadTriggers(triggers);
    DruidRaidStrategy::InitDeadTriggers(triggers);
}

void BalanceDruidAoeStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DruidAoeStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "ranged high aoe",
        NextAction::array(0, new NextAction("starfall", ACTION_HIGH), NULL)));
}

void BalanceDruidAoeStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DruidAoeStrategy::InitNonCombatTriggers(triggers);
}

void BalanceDruidAoePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidAoeStrategy::InitCombatTriggers(triggers);
    DruidAoePveStrategy::InitCombatTriggers(triggers);
}

void BalanceDruidAoePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidAoeStrategy::InitNonCombatTriggers(triggers);
    DruidAoePveStrategy::InitNonCombatTriggers(triggers);
}

void BalanceDruidAoePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidAoeStrategy::InitCombatTriggers(triggers);
    DruidAoePvpStrategy::InitCombatTriggers(triggers);
}

void BalanceDruidAoePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidAoeStrategy::InitNonCombatTriggers(triggers);
    DruidAoePvpStrategy::InitNonCombatTriggers(triggers);
}

void BalanceDruidAoeRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidAoeStrategy::InitCombatTriggers(triggers);
    DruidAoeRaidStrategy::InitCombatTriggers(triggers);
}

void BalanceDruidAoeRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidAoeStrategy::InitNonCombatTriggers(triggers);
    DruidAoeRaidStrategy::InitNonCombatTriggers(triggers);
}

void BalanceDruidBuffStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DruidBuffStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "moonkin form",
        NextAction::array(0, new NextAction("moonkin form", ACTION_MOVE), NULL)));

    triggers.push_back(new TriggerNode(
        "innervate",
        NextAction::array(0, new NextAction("innervate", ACTION_HIGH), NULL)));
}

void BalanceDruidBuffStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DruidBuffStrategy::InitNonCombatTriggers(triggers);
}

void BalanceDruidBuffPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidBuffStrategy::InitCombatTriggers(triggers);
    DruidBuffPveStrategy::InitCombatTriggers(triggers);
}

void BalanceDruidBuffPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidBuffStrategy::InitNonCombatTriggers(triggers);
    DruidBuffPveStrategy::InitNonCombatTriggers(triggers);
}

void BalanceDruidBuffPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidBuffStrategy::InitCombatTriggers(triggers);
    DruidBuffPvpStrategy::InitCombatTriggers(triggers);
}

void BalanceDruidBuffPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidBuffStrategy::InitNonCombatTriggers(triggers);
    DruidBuffPvpStrategy::InitNonCombatTriggers(triggers);
}

void BalanceDruidBuffRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidBuffStrategy::InitCombatTriggers(triggers);
    DruidBuffRaidStrategy::InitCombatTriggers(triggers);
}

void BalanceDruidBuffRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidBuffStrategy::InitNonCombatTriggers(triggers);
    DruidBuffRaidStrategy::InitNonCombatTriggers(triggers);
}

void BalanceDruidBoostStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DruidBoostStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "nature's swiftness",
        NextAction::array(0, new NextAction("nature's swiftness", ACTION_HIGH), NULL)));
}

void BalanceDruidBoostStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DruidBoostStrategy::InitNonCombatTriggers(triggers);
}

void BalanceDruidBoostPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidBoostStrategy::InitCombatTriggers(triggers);
    DruidBoostPveStrategy::InitCombatTriggers(triggers);
}

void BalanceDruidBoostPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidBoostStrategy::InitNonCombatTriggers(triggers);
    DruidBoostPveStrategy::InitNonCombatTriggers(triggers);
}

void BalanceDruidBoostPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidBoostStrategy::InitCombatTriggers(triggers);
    DruidBoostPvpStrategy::InitCombatTriggers(triggers);
}

void BalanceDruidBoostPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidBoostStrategy::InitNonCombatTriggers(triggers);
    DruidBoostPvpStrategy::InitNonCombatTriggers(triggers);
}

void BalanceDruidBoostRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidBoostStrategy::InitCombatTriggers(triggers);
    DruidBoostRaidStrategy::InitCombatTriggers(triggers);
}

void BalanceDruidBoostRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidBoostStrategy::InitNonCombatTriggers(triggers);
    DruidBoostRaidStrategy::InitNonCombatTriggers(triggers);
}

void BalanceDruidCcStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DruidCcStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "entangling roots",
        NextAction::array(0, new NextAction("entangling roots on cc", ACTION_INTERRUPT), NULL)));

    triggers.push_back(new TriggerNode(
        "entangling roots kite",
        NextAction::array(0, new NextAction("entangling roots", ACTION_INTERRUPT), NULL)));

    triggers.push_back(new TriggerNode(
        "enemy five yards",
        NextAction::array(0, new NextAction("nature's grasp", ACTION_HIGH), NULL)));
}

void BalanceDruidCcStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DruidCcStrategy::InitNonCombatTriggers(triggers);
}

void BalanceDruidCcPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidCcStrategy::InitCombatTriggers(triggers);
    DruidCcPveStrategy::InitCombatTriggers(triggers);
}

void BalanceDruidCcPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidCcStrategy::InitNonCombatTriggers(triggers);
    DruidCcPveStrategy::InitNonCombatTriggers(triggers);
}

void BalanceDruidCcPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidCcStrategy::InitCombatTriggers(triggers);
    DruidCcPvpStrategy::InitCombatTriggers(triggers);
}

void BalanceDruidCcPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidCcStrategy::InitNonCombatTriggers(triggers);
    DruidCcPvpStrategy::InitNonCombatTriggers(triggers);
}

void BalanceDruidCcRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidCcStrategy::InitCombatTriggers(triggers);
    DruidCcRaidStrategy::InitCombatTriggers(triggers);
}

void BalanceDruidCcRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidCcStrategy::InitNonCombatTriggers(triggers);
    DruidCcRaidStrategy::InitNonCombatTriggers(triggers);
}

void BalanceDruidCureStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DruidCureStrategy::InitCombatTriggers(triggers);
}

void BalanceDruidCureStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DruidCureStrategy::InitNonCombatTriggers(triggers);
}

void BalanceDruidCurePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidCureStrategy::InitCombatTriggers(triggers);
    DruidCurePveStrategy::InitCombatTriggers(triggers);
}

void BalanceDruidCurePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidCureStrategy::InitNonCombatTriggers(triggers);
    DruidCurePveStrategy::InitNonCombatTriggers(triggers);
}

void BalanceDruidCurePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidCureStrategy::InitCombatTriggers(triggers);
    DruidCurePvpStrategy::InitCombatTriggers(triggers);
}

void BalanceDruidCurePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidCureStrategy::InitNonCombatTriggers(triggers);
    DruidCurePvpStrategy::InitNonCombatTriggers(triggers);
}

void BalanceDruidCureRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidCureStrategy::InitCombatTriggers(triggers);
    DruidCureRaidStrategy::InitCombatTriggers(triggers);
}

void BalanceDruidCureRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidCureStrategy::InitNonCombatTriggers(triggers);
    DruidCureRaidStrategy::InitNonCombatTriggers(triggers);
}

#endif

#ifdef MANGOSBOT_ONE // TBC

NextAction** BalanceDruidStrategy::GetDefaultCombatActions()
{
    return NextAction::array(0, new NextAction("wrath", ACTION_IDLE), NULL);
}

void BalanceDruidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DruidStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "rebirth",
        NextAction::array(0, new NextAction("rebirth", ACTION_EMERGENCY), NULL)));

    triggers.push_back(new TriggerNode(
        "critical health",
        NextAction::array(0, new NextAction("regrowth", ACTION_CRITICAL_HEAL + 1),
                             new NextAction("healing touch", ACTION_CRITICAL_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "low health",
        NextAction::array(0, new NextAction("regrowth", ACTION_MEDIUM_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "faerie fire",
        NextAction::array(0, new NextAction("faerie fire", ACTION_NORMAL + 3), NULL)));

    triggers.push_back(new TriggerNode(
        "insect swarm",
        NextAction::array(0, new NextAction("insect swarm", ACTION_NORMAL + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "moonfire",
        NextAction::array(0, new NextAction("moonfire", ACTION_NORMAL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "often",
        NextAction::array(0, new NextAction("starfire", ACTION_NORMAL), NULL)));
}

void BalanceDruidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DruidStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "low health",
        NextAction::array(0, new NextAction("regrowth", ACTION_MEDIUM_HEAL), NULL)));
}

void BalanceDruidStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    DruidStrategy::InitReactionTriggers(triggers);
}

void BalanceDruidStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    DruidStrategy::InitDeadTriggers(triggers);
}

void BalanceDruidPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidStrategy::InitCombatTriggers(triggers);
    DruidPveStrategy::InitCombatTriggers(triggers);
}

void BalanceDruidPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidStrategy::InitNonCombatTriggers(triggers);
    DruidPveStrategy::InitNonCombatTriggers(triggers);
}

void BalanceDruidPveStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidStrategy::InitReactionTriggers(triggers);
    DruidPveStrategy::InitReactionTriggers(triggers);
}

void BalanceDruidPveStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidStrategy::InitDeadTriggers(triggers);
    DruidPveStrategy::InitDeadTriggers(triggers);
}

void BalanceDruidPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidStrategy::InitCombatTriggers(triggers);
    DruidPvpStrategy::InitCombatTriggers(triggers);
}

void BalanceDruidPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidStrategy::InitNonCombatTriggers(triggers);
    DruidPvpStrategy::InitNonCombatTriggers(triggers);
}

void BalanceDruidPvpStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidStrategy::InitReactionTriggers(triggers);
    DruidPvpStrategy::InitReactionTriggers(triggers);
}

void BalanceDruidPvpStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidStrategy::InitDeadTriggers(triggers);
    DruidPvpStrategy::InitDeadTriggers(triggers);
}

ai::NextAction** BalanceDruidRaidStrategy::GetDefaultCombatActions()
{
    return NextAction::array(0, new NextAction("starfire", ACTION_IDLE), NULL);
}

void BalanceDruidRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidStrategy::InitCombatTriggers(triggers);
    DruidRaidStrategy::InitCombatTriggers(triggers);
}

void BalanceDruidRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidStrategy::InitNonCombatTriggers(triggers);
    DruidRaidStrategy::InitNonCombatTriggers(triggers);
}

void BalanceDruidRaidStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidStrategy::InitReactionTriggers(triggers);
    DruidRaidStrategy::InitReactionTriggers(triggers);
}

void BalanceDruidRaidStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidStrategy::InitDeadTriggers(triggers);
    DruidRaidStrategy::InitDeadTriggers(triggers);
}

void BalanceDruidAoeStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DruidAoeStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "ranged high aoe",
        NextAction::array(0, new NextAction("starfall", ACTION_HIGH), NULL)));
}

void BalanceDruidAoeStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DruidAoeStrategy::InitNonCombatTriggers(triggers);
}

void BalanceDruidAoePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidAoeStrategy::InitCombatTriggers(triggers);
    DruidAoePveStrategy::InitCombatTriggers(triggers);
}

void BalanceDruidAoePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidAoeStrategy::InitNonCombatTriggers(triggers);
    DruidAoePveStrategy::InitNonCombatTriggers(triggers);
}

void BalanceDruidAoePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidAoeStrategy::InitCombatTriggers(triggers);
    DruidAoePvpStrategy::InitCombatTriggers(triggers);
}

void BalanceDruidAoePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidAoeStrategy::InitNonCombatTriggers(triggers);
    DruidAoePvpStrategy::InitNonCombatTriggers(triggers);
}

void BalanceDruidAoeRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidAoeStrategy::InitCombatTriggers(triggers);
    DruidAoeRaidStrategy::InitCombatTriggers(triggers);
}

void BalanceDruidAoeRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidAoeStrategy::InitNonCombatTriggers(triggers);
    DruidAoeRaidStrategy::InitNonCombatTriggers(triggers);
}

void BalanceDruidBuffStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DruidBuffStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "moonkin form",
        NextAction::array(0, new NextAction("moonkin form", ACTION_MOVE), NULL)));

    triggers.push_back(new TriggerNode(
        "innervate",
        NextAction::array(0, new NextAction("innervate", ACTION_HIGH), NULL)));
}

void BalanceDruidBuffStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DruidBuffStrategy::InitNonCombatTriggers(triggers);
}

void BalanceDruidBuffPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidBuffStrategy::InitCombatTriggers(triggers);
    DruidBuffPveStrategy::InitCombatTriggers(triggers);
}

void BalanceDruidBuffPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidBuffStrategy::InitNonCombatTriggers(triggers);
    DruidBuffPveStrategy::InitNonCombatTriggers(triggers);
}

void BalanceDruidBuffPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidBuffStrategy::InitCombatTriggers(triggers);
    DruidBuffPvpStrategy::InitCombatTriggers(triggers);
}

void BalanceDruidBuffPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidBuffStrategy::InitNonCombatTriggers(triggers);
    DruidBuffPvpStrategy::InitNonCombatTriggers(triggers);
}

void BalanceDruidBuffRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidBuffStrategy::InitCombatTriggers(triggers);
    DruidBuffRaidStrategy::InitCombatTriggers(triggers);
}

void BalanceDruidBuffRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidBuffStrategy::InitNonCombatTriggers(triggers);
    DruidBuffRaidStrategy::InitNonCombatTriggers(triggers);
}

void BalanceDruidBoostStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DruidBoostStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "nature's swiftness",
        NextAction::array(0, new NextAction("nature's swiftness", ACTION_HIGH), NULL)));
}

void BalanceDruidBoostStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DruidBoostStrategy::InitNonCombatTriggers(triggers);
}

void BalanceDruidBoostPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidBoostStrategy::InitCombatTriggers(triggers);
    DruidBoostPveStrategy::InitCombatTriggers(triggers);
}

void BalanceDruidBoostPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidBoostStrategy::InitNonCombatTriggers(triggers);
    DruidBoostPveStrategy::InitNonCombatTriggers(triggers);
}

void BalanceDruidBoostPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidBoostStrategy::InitCombatTriggers(triggers);
    DruidBoostPvpStrategy::InitCombatTriggers(triggers);
}

void BalanceDruidBoostPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidBoostStrategy::InitNonCombatTriggers(triggers);
    DruidBoostPvpStrategy::InitNonCombatTriggers(triggers);
}

void BalanceDruidBoostRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidBoostStrategy::InitCombatTriggers(triggers);
    DruidBoostRaidStrategy::InitCombatTriggers(triggers);
}

void BalanceDruidBoostRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidBoostStrategy::InitNonCombatTriggers(triggers);
    DruidBoostRaidStrategy::InitNonCombatTriggers(triggers);
}

void BalanceDruidCcStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DruidCcStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "entangling roots",
        NextAction::array(0, new NextAction("entangling roots on cc", ACTION_INTERRUPT), NULL)));

    triggers.push_back(new TriggerNode(
        "entangling roots kite",
        NextAction::array(0, new NextAction("entangling roots", ACTION_INTERRUPT), NULL)));

    triggers.push_back(new TriggerNode(
        "enemy five yards",
        NextAction::array(0, new NextAction("nature's grasp", ACTION_HIGH), NULL)));
}

void BalanceDruidCcStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DruidCcStrategy::InitNonCombatTriggers(triggers);
}

void BalanceDruidCcPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidCcStrategy::InitCombatTriggers(triggers);
    DruidCcPveStrategy::InitCombatTriggers(triggers);
}

void BalanceDruidCcPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidCcStrategy::InitNonCombatTriggers(triggers);
    DruidCcPveStrategy::InitNonCombatTriggers(triggers);
}

void BalanceDruidCcPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidCcStrategy::InitCombatTriggers(triggers);
    DruidCcPvpStrategy::InitCombatTriggers(triggers);
}

void BalanceDruidCcPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidCcStrategy::InitNonCombatTriggers(triggers);
    DruidCcPvpStrategy::InitNonCombatTriggers(triggers);
}

void BalanceDruidCcRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidCcStrategy::InitCombatTriggers(triggers);
    DruidCcRaidStrategy::InitCombatTriggers(triggers);
}

void BalanceDruidCcRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidCcStrategy::InitNonCombatTriggers(triggers);
    DruidCcRaidStrategy::InitNonCombatTriggers(triggers);
}

void BalanceDruidCureStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DruidCureStrategy::InitCombatTriggers(triggers);
}

void BalanceDruidCureStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DruidCureStrategy::InitNonCombatTriggers(triggers);
}

void BalanceDruidCurePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidCureStrategy::InitCombatTriggers(triggers);
    DruidCurePveStrategy::InitCombatTriggers(triggers);
}

void BalanceDruidCurePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidCureStrategy::InitNonCombatTriggers(triggers);
    DruidCurePveStrategy::InitNonCombatTriggers(triggers);
}

void BalanceDruidCurePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidCureStrategy::InitCombatTriggers(triggers);
    DruidCurePvpStrategy::InitCombatTriggers(triggers);
}

void BalanceDruidCurePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidCureStrategy::InitNonCombatTriggers(triggers);
    DruidCurePvpStrategy::InitNonCombatTriggers(triggers);
}

void BalanceDruidCureRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidCureStrategy::InitCombatTriggers(triggers);
    DruidCureRaidStrategy::InitCombatTriggers(triggers);
}

void BalanceDruidCureRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidCureStrategy::InitNonCombatTriggers(triggers);
    DruidCureRaidStrategy::InitNonCombatTriggers(triggers);
}

#endif

#ifdef MANGOSBOT_TWO // WOTLK

NextAction** BalanceDruidStrategy::GetDefaultCombatActions()
{
    return NextAction::array(0, new NextAction("wrath", ACTION_IDLE), NULL);
}

void BalanceDruidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DruidStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "rebirth",
        NextAction::array(0, new NextAction("rebirth", ACTION_EMERGENCY), NULL)));

    triggers.push_back(new TriggerNode(
        "critical health",
        NextAction::array(0, new NextAction("regrowth", ACTION_CRITICAL_HEAL + 1),
                             new NextAction("healing touch", ACTION_CRITICAL_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "low health",
        NextAction::array(0, new NextAction("regrowth", ACTION_MEDIUM_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "eclipse (solar)",
        NextAction::array(0, new NextAction("wrath", ACTION_HIGH), NULL)));

    triggers.push_back(new TriggerNode(
        "eclipse (lunar)",
        NextAction::array(0, new NextAction("starfire", ACTION_HIGH), NULL)));

    triggers.push_back(new TriggerNode(
        "faerie fire",
        NextAction::array(0, new NextAction("faerie fire", ACTION_NORMAL + 3), NULL)));

    triggers.push_back(new TriggerNode(
        "insect swarm",
        NextAction::array(0, new NextAction("insect swarm", ACTION_NORMAL + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "moonfire",
        NextAction::array(0, new NextAction("moonfire", ACTION_NORMAL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "often",
        NextAction::array(0, new NextAction("starfire", ACTION_NORMAL), NULL)));
}

void BalanceDruidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DruidStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "low health",
        NextAction::array(0, new NextAction("regrowth", ACTION_MEDIUM_HEAL), NULL)));
}

void BalanceDruidStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    DruidStrategy::InitReactionTriggers(triggers);
}

void BalanceDruidStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    DruidStrategy::InitDeadTriggers(triggers);
}

void BalanceDruidPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidStrategy::InitCombatTriggers(triggers);
    DruidPveStrategy::InitCombatTriggers(triggers);
}

void BalanceDruidPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidStrategy::InitNonCombatTriggers(triggers);
    DruidPveStrategy::InitNonCombatTriggers(triggers);
}

void BalanceDruidPveStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidStrategy::InitReactionTriggers(triggers);
    DruidPveStrategy::InitReactionTriggers(triggers);
}

void BalanceDruidPveStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidStrategy::InitDeadTriggers(triggers);
    DruidPveStrategy::InitDeadTriggers(triggers);
}

void BalanceDruidPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidStrategy::InitCombatTriggers(triggers);
    DruidPvpStrategy::InitCombatTriggers(triggers);
}

void BalanceDruidPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidStrategy::InitNonCombatTriggers(triggers);
    DruidPvpStrategy::InitNonCombatTriggers(triggers);
}

void BalanceDruidPvpStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidStrategy::InitReactionTriggers(triggers);
    DruidPvpStrategy::InitReactionTriggers(triggers);
}

void BalanceDruidPvpStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidStrategy::InitDeadTriggers(triggers);
    DruidPvpStrategy::InitDeadTriggers(triggers);
}

ai::NextAction** BalanceDruidRaidStrategy::GetDefaultCombatActions()
{
    return NextAction::array(0, new NextAction("starfire", ACTION_IDLE), NULL);
}

void BalanceDruidRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidStrategy::InitCombatTriggers(triggers);
    DruidRaidStrategy::InitCombatTriggers(triggers);
}

void BalanceDruidRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidStrategy::InitNonCombatTriggers(triggers);
    DruidRaidStrategy::InitNonCombatTriggers(triggers);
}

void BalanceDruidRaidStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidStrategy::InitReactionTriggers(triggers);
    DruidRaidStrategy::InitReactionTriggers(triggers);
}

void BalanceDruidRaidStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidStrategy::InitDeadTriggers(triggers);
    DruidRaidStrategy::InitDeadTriggers(triggers);
}

void BalanceDruidAoeStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DruidAoeStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "ranged high aoe",
        NextAction::array(0, new NextAction("starfall", ACTION_HIGH), NULL)));
}

void BalanceDruidAoeStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DruidAoeStrategy::InitNonCombatTriggers(triggers);
}

void BalanceDruidAoePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidAoeStrategy::InitCombatTriggers(triggers);
    DruidAoePveStrategy::InitCombatTriggers(triggers);
}

void BalanceDruidAoePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidAoeStrategy::InitNonCombatTriggers(triggers);
    DruidAoePveStrategy::InitNonCombatTriggers(triggers);
}

void BalanceDruidAoePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidAoeStrategy::InitCombatTriggers(triggers);
    DruidAoePvpStrategy::InitCombatTriggers(triggers);
}

void BalanceDruidAoePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidAoeStrategy::InitNonCombatTriggers(triggers);
    DruidAoePvpStrategy::InitNonCombatTriggers(triggers);
}

void BalanceDruidAoeRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidAoeStrategy::InitCombatTriggers(triggers);
    DruidAoeRaidStrategy::InitCombatTriggers(triggers);
}

void BalanceDruidAoeRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidAoeStrategy::InitNonCombatTriggers(triggers);
    DruidAoeRaidStrategy::InitNonCombatTriggers(triggers);
}

void BalanceDruidBuffStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DruidBuffStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "moonkin form",
        NextAction::array(0, new NextAction("moonkin form", ACTION_MOVE), NULL)));

    triggers.push_back(new TriggerNode(
        "innervate",
        NextAction::array(0, new NextAction("innervate", ACTION_HIGH), NULL)));
}

void BalanceDruidBuffStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DruidBuffStrategy::InitNonCombatTriggers(triggers);
}

void BalanceDruidBuffPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidBuffStrategy::InitCombatTriggers(triggers);
    DruidBuffPveStrategy::InitCombatTriggers(triggers);
}

void BalanceDruidBuffPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidBuffStrategy::InitNonCombatTriggers(triggers);
    DruidBuffPveStrategy::InitNonCombatTriggers(triggers);
}

void BalanceDruidBuffPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidBuffStrategy::InitCombatTriggers(triggers);
    DruidBuffPvpStrategy::InitCombatTriggers(triggers);
}

void BalanceDruidBuffPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidBuffStrategy::InitNonCombatTriggers(triggers);
    DruidBuffPvpStrategy::InitNonCombatTriggers(triggers);
}

void BalanceDruidBuffRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidBuffStrategy::InitCombatTriggers(triggers);
    DruidBuffRaidStrategy::InitCombatTriggers(triggers);
}

void BalanceDruidBuffRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidBuffStrategy::InitNonCombatTriggers(triggers);
    DruidBuffRaidStrategy::InitNonCombatTriggers(triggers);
}

void BalanceDruidBoostStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DruidBoostStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "nature's swiftness",
        NextAction::array(0, new NextAction("nature's swiftness", ACTION_HIGH), NULL)));
}

void BalanceDruidBoostStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DruidBoostStrategy::InitNonCombatTriggers(triggers);
}

void BalanceDruidBoostPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidBoostStrategy::InitCombatTriggers(triggers);
    DruidBoostPveStrategy::InitCombatTriggers(triggers);
}

void BalanceDruidBoostPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidBoostStrategy::InitNonCombatTriggers(triggers);
    DruidBoostPveStrategy::InitNonCombatTriggers(triggers);
}

void BalanceDruidBoostPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidBoostStrategy::InitCombatTriggers(triggers);
    DruidBoostPvpStrategy::InitCombatTriggers(triggers);
}

void BalanceDruidBoostPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidBoostStrategy::InitNonCombatTriggers(triggers);
    DruidBoostPvpStrategy::InitNonCombatTriggers(triggers);
}

void BalanceDruidBoostRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidBoostStrategy::InitCombatTriggers(triggers);
    DruidBoostRaidStrategy::InitCombatTriggers(triggers);
}

void BalanceDruidBoostRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidBoostStrategy::InitNonCombatTriggers(triggers);
    DruidBoostRaidStrategy::InitNonCombatTriggers(triggers);
}

void BalanceDruidCcStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DruidCcStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "entangling roots",
        NextAction::array(0, new NextAction("entangling roots on cc", ACTION_INTERRUPT), NULL)));

    triggers.push_back(new TriggerNode(
        "entangling roots kite",
        NextAction::array(0, new NextAction("entangling roots", ACTION_INTERRUPT), NULL)));

    triggers.push_back(new TriggerNode(
        "enemy five yards",
        NextAction::array(0, new NextAction("nature's grasp", ACTION_HIGH), NULL)));
}

void BalanceDruidCcStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DruidCcStrategy::InitNonCombatTriggers(triggers);
}

void BalanceDruidCcPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidCcStrategy::InitCombatTriggers(triggers);
    DruidCcPveStrategy::InitCombatTriggers(triggers);
}

void BalanceDruidCcPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidCcStrategy::InitNonCombatTriggers(triggers);
    DruidCcPveStrategy::InitNonCombatTriggers(triggers);
}

void BalanceDruidCcPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidCcStrategy::InitCombatTriggers(triggers);
    DruidCcPvpStrategy::InitCombatTriggers(triggers);
}

void BalanceDruidCcPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidCcStrategy::InitNonCombatTriggers(triggers);
    DruidCcPvpStrategy::InitNonCombatTriggers(triggers);
}

void BalanceDruidCcRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidCcStrategy::InitCombatTriggers(triggers);
    DruidCcRaidStrategy::InitCombatTriggers(triggers);
}

void BalanceDruidCcRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidCcStrategy::InitNonCombatTriggers(triggers);
    DruidCcRaidStrategy::InitNonCombatTriggers(triggers);
}

void BalanceDruidCureStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DruidCureStrategy::InitCombatTriggers(triggers);
}

void BalanceDruidCureStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DruidCureStrategy::InitNonCombatTriggers(triggers);
}

void BalanceDruidCurePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidCureStrategy::InitCombatTriggers(triggers);
    DruidCurePveStrategy::InitCombatTriggers(triggers);
}

void BalanceDruidCurePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidCureStrategy::InitNonCombatTriggers(triggers);
    DruidCurePveStrategy::InitNonCombatTriggers(triggers);
}

void BalanceDruidCurePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidCureStrategy::InitCombatTriggers(triggers);
    DruidCurePvpStrategy::InitCombatTriggers(triggers);
}

void BalanceDruidCurePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidCureStrategy::InitNonCombatTriggers(triggers);
    DruidCurePvpStrategy::InitNonCombatTriggers(triggers);
}

void BalanceDruidCureRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidCureStrategy::InitCombatTriggers(triggers);
    DruidCureRaidStrategy::InitCombatTriggers(triggers);
}

void BalanceDruidCureRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BalanceDruidCureStrategy::InitNonCombatTriggers(triggers);
    DruidCureRaidStrategy::InitNonCombatTriggers(triggers);
}

#endif