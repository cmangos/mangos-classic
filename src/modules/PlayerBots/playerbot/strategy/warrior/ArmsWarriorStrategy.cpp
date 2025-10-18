
#include "playerbot/playerbot.h"
#include "ArmsWarriorStrategy.h"

using namespace ai;

class ArmsWarriorStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    ArmsWarriorStrategyActionNodeFactory()
    {
        creators["charge"] = &charge;
        creators["death wish"] = &death_wish;
        creators["berserker rage fear"] = &berserker_rage_fear;
        creators["piercing howl"] = &piercing_howl;
        creators["mocking blow"] = &mocking_blow;
        creators["heroic strike"] = &heroic_strike;
        creators["whirlwind"] = &whirlwind;
        creators["sweeping strikes"] = &sweeping_strikes;
    }

private:
    ACTION_NODE_A(charge, "charge", "reach melee");

    ACTION_NODE_A(death_wish, "death wish", "bloodrage");

    ACTION_NODE_A(piercing_howl, "piercing howl", "mocking blow");

    ACTION_NODE_A(mocking_blow, "mocking blow", "hamstring");

    ACTION_NODE_A(heroic_strike, "heroic strike", "melee");

    ACTION_NODE_A(berserker_rage_fear, "berserker rage", "death wish");

    ACTION_NODE_P(whirlwind, "whirlwind", "berserker stance");

    ACTION_NODE_P(sweeping_strikes, "sweeping strikes", "battle stance");
};

ArmsWarriorStrategy::ArmsWarriorStrategy(PlayerbotAI* ai) : WarriorStrategy(ai)
{
    actionNodeFactories.Add(new ArmsWarriorStrategyActionNodeFactory());
}

#ifdef MANGOSBOT_ZERO // Vanilla

NextAction** ArmsWarriorStrategy::GetDefaultCombatActions()
{
    return NextAction::array(0, new NextAction("melee", ACTION_IDLE), NULL);
}

void ArmsWarriorStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarriorStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "critical health",
        NextAction::array(0, new NextAction("intimidating shout", ACTION_EMERGENCY), NULL)));

    triggers.push_back(new TriggerNode(
        "enemy out of melee",
        NextAction::array(0, new NextAction("charge", ACTION_MOVE + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "target critical health",
        NextAction::array(0, new NextAction("execute", ACTION_HIGH + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "overpower",
        NextAction::array(0, new NextAction("overpower", ACTION_HIGH), NULL)));

    triggers.push_back(new TriggerNode(
        "mortal strike",
        NextAction::array(0, new NextAction("mortal strike", ACTION_NORMAL + 3), NULL)));

    triggers.push_back(new TriggerNode(
        "whirlwind",
        NextAction::array(0, new NextAction("whirlwind", ACTION_NORMAL + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "heroic strike",
        NextAction::array(0, new NextAction("heroic strike", ACTION_NORMAL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "rend",
        NextAction::array(0, new NextAction("rend", ACTION_NORMAL), NULL)));
}

void ArmsWarriorStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarriorStrategy::InitNonCombatTriggers(triggers);
}

void ArmsWarriorStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    WarriorStrategy::InitReactionTriggers(triggers);
}

void ArmsWarriorStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    WarriorStrategy::InitDeadTriggers(triggers);
}

void ArmsWarriorPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArmsWarriorStrategy::InitCombatTriggers(triggers);
    WarriorPveStrategy::InitCombatTriggers(triggers);
}

void ArmsWarriorPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArmsWarriorStrategy::InitNonCombatTriggers(triggers);
    WarriorPveStrategy::InitNonCombatTriggers(triggers);
}

void ArmsWarriorPveStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ArmsWarriorStrategy::InitReactionTriggers(triggers);
    WarriorPveStrategy::InitReactionTriggers(triggers);
}

void ArmsWarriorPveStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ArmsWarriorStrategy::InitDeadTriggers(triggers);
    WarriorPveStrategy::InitDeadTriggers(triggers);
}

void ArmsWarriorPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArmsWarriorStrategy::InitCombatTriggers(triggers);
    WarriorPvpStrategy::InitCombatTriggers(triggers);
}

void ArmsWarriorPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArmsWarriorStrategy::InitNonCombatTriggers(triggers);
    WarriorPvpStrategy::InitNonCombatTriggers(triggers);
}

void ArmsWarriorPvpStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ArmsWarriorStrategy::InitReactionTriggers(triggers);
    WarriorPvpStrategy::InitReactionTriggers(triggers);
}

void ArmsWarriorPvpStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ArmsWarriorStrategy::InitDeadTriggers(triggers);
    WarriorPvpStrategy::InitDeadTriggers(triggers);
}

void ArmsWarriorRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArmsWarriorStrategy::InitCombatTriggers(triggers);
    WarriorRaidStrategy::InitCombatTriggers(triggers);
}

void ArmsWarriorRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArmsWarriorStrategy::InitNonCombatTriggers(triggers);
    WarriorRaidStrategy::InitNonCombatTriggers(triggers);
}

void ArmsWarriorRaidStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ArmsWarriorStrategy::InitReactionTriggers(triggers);
    WarriorRaidStrategy::InitReactionTriggers(triggers);
}

void ArmsWarriorRaidStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ArmsWarriorStrategy::InitDeadTriggers(triggers);
    WarriorRaidStrategy::InitDeadTriggers(triggers);
}

void ArmsWarriorAoeStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarriorAoeStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "melee light aoe",
        NextAction::array(0, new NextAction("sweeping strikes", ACTION_HIGH + 5), NULL)));

    triggers.push_back(new TriggerNode(
        "melee light aoe",
        NextAction::array(0, new NextAction("whirlwind", ACTION_HIGH + 4), NULL)));

}

void ArmsWarriorAoeStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarriorAoeStrategy::InitNonCombatTriggers(triggers);
}

void ArmsWarriorAoePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArmsWarriorAoeStrategy::InitCombatTriggers(triggers);
    WarriorAoePveStrategy::InitCombatTriggers(triggers);
}

void ArmsWarriorAoePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArmsWarriorAoeStrategy::InitNonCombatTriggers(triggers);
    WarriorAoePveStrategy::InitNonCombatTriggers(triggers);
}

void ArmsWarriorAoePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArmsWarriorAoeStrategy::InitCombatTriggers(triggers);
    WarriorAoePvpStrategy::InitCombatTriggers(triggers);
}

void ArmsWarriorAoePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArmsWarriorAoeStrategy::InitNonCombatTriggers(triggers);
    WarriorAoePvpStrategy::InitNonCombatTriggers(triggers);
}

void ArmsWarriorAoeRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArmsWarriorAoeStrategy::InitCombatTriggers(triggers);
    WarriorAoeRaidStrategy::InitCombatTriggers(triggers);
}

void ArmsWarriorAoeRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArmsWarriorAoeStrategy::InitNonCombatTriggers(triggers);
    WarriorAoeRaidStrategy::InitNonCombatTriggers(triggers);
}

void ArmsWarriorBuffStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarriorBuffStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "berserker stance",
        NextAction::array(0, new NextAction("berserker stance", ACTION_NORMAL), NULL)));

    triggers.push_back(new TriggerNode(
        "feared",
        NextAction::array(0, new NextAction("berserker rage fear", ACTION_INTERRUPT), NULL)));

}

void ArmsWarriorBuffStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarriorBuffStrategy::InitNonCombatTriggers(triggers);
}

void ArmsWarriorBuffPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArmsWarriorBuffStrategy::InitCombatTriggers(triggers);
    WarriorBuffPveStrategy::InitCombatTriggers(triggers);
}

void ArmsWarriorBuffPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArmsWarriorBuffStrategy::InitNonCombatTriggers(triggers);
    WarriorBuffPveStrategy::InitNonCombatTriggers(triggers);
}

void ArmsWarriorBuffPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArmsWarriorBuffStrategy::InitCombatTriggers(triggers);
    WarriorBuffPvpStrategy::InitCombatTriggers(triggers);
}

void ArmsWarriorBuffPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArmsWarriorBuffStrategy::InitNonCombatTriggers(triggers);
    WarriorBuffPvpStrategy::InitNonCombatTriggers(triggers);
}

void ArmsWarriorBuffRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArmsWarriorBuffStrategy::InitCombatTriggers(triggers);
    WarriorBuffRaidStrategy::InitCombatTriggers(triggers);
}

void ArmsWarriorBuffRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArmsWarriorBuffStrategy::InitNonCombatTriggers(triggers);
    WarriorBuffRaidStrategy::InitNonCombatTriggers(triggers);
}

void ArmsWarriorBoostStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarriorBoostStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "recklessness",
        NextAction::array(0, new NextAction("recklessness", ACTION_HIGH), NULL)));
}

void ArmsWarriorBoostStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarriorBoostStrategy::InitNonCombatTriggers(triggers);
}

void ArmsWarriorBoostPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArmsWarriorBoostStrategy::InitCombatTriggers(triggers);
    WarriorBoostPveStrategy::InitCombatTriggers(triggers);
}

void ArmsWarriorBoostPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArmsWarriorBoostStrategy::InitNonCombatTriggers(triggers);
    WarriorBoostPveStrategy::InitNonCombatTriggers(triggers);
}

void ArmsWarriorBoostPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArmsWarriorBoostStrategy::InitCombatTriggers(triggers);
    WarriorBoostPvpStrategy::InitCombatTriggers(triggers);
}

void ArmsWarriorBoostPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArmsWarriorBoostStrategy::InitNonCombatTriggers(triggers);
    WarriorBoostPvpStrategy::InitNonCombatTriggers(triggers);
}

void ArmsWarriorBoostRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArmsWarriorBoostStrategy::InitCombatTriggers(triggers);
    WarriorBoostRaidStrategy::InitCombatTriggers(triggers);
}

void ArmsWarriorBoostRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArmsWarriorBoostStrategy::InitNonCombatTriggers(triggers);
    WarriorBoostRaidStrategy::InitNonCombatTriggers(triggers);
}

void ArmsWarriorCcStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarriorCcStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "hamstring",
        NextAction::array(0, new NextAction("piercing howl", ACTION_HIGH), NULL)));
}

void ArmsWarriorCcStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarriorCcStrategy::InitNonCombatTriggers(triggers);
}

void ArmsWarriorCcPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArmsWarriorCcStrategy::InitCombatTriggers(triggers);
    WarriorCcPveStrategy::InitCombatTriggers(triggers);
}

void ArmsWarriorCcPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArmsWarriorCcStrategy::InitNonCombatTriggers(triggers);
    WarriorCcPveStrategy::InitNonCombatTriggers(triggers);
}

void ArmsWarriorCcPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArmsWarriorCcStrategy::InitCombatTriggers(triggers);
    WarriorCcPvpStrategy::InitCombatTriggers(triggers);
}

void ArmsWarriorCcPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArmsWarriorCcStrategy::InitNonCombatTriggers(triggers);
    WarriorCcPvpStrategy::InitNonCombatTriggers(triggers);
}

void ArmsWarriorCcRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArmsWarriorCcStrategy::InitCombatTriggers(triggers);
    WarriorCcRaidStrategy::InitCombatTriggers(triggers);
}

void ArmsWarriorCcRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArmsWarriorCcStrategy::InitNonCombatTriggers(triggers);
    WarriorCcRaidStrategy::InitNonCombatTriggers(triggers);
}

#endif
#ifdef MANGOSBOT_ONE // TBC


NextAction** ArmsWarriorStrategy::GetDefaultCombatActions()
{
    return NextAction::array(0, new NextAction("melee", ACTION_IDLE), NULL);
}

void ArmsWarriorStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarriorStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "critical health",
        NextAction::array(0, new NextAction("intimidating shout", ACTION_EMERGENCY), NULL)));

    triggers.push_back(new TriggerNode(
        "enemy out of melee",
        NextAction::array(0, new NextAction("charge", ACTION_MOVE), NULL)));

    triggers.push_back(new TriggerNode(
        "target critical health",
        NextAction::array(0, new NextAction("execute", ACTION_HIGH + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "overpower",
        NextAction::array(0, new NextAction("overpower", ACTION_HIGH), NULL)));

    triggers.push_back(new TriggerNode(
        "heroic strike",
        NextAction::array(0, new NextAction("heroic strike", ACTION_NORMAL + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "mortal strike",
        NextAction::array(0, new NextAction("mortal strike", ACTION_NORMAL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "rend",
        NextAction::array(0, new NextAction("rend", ACTION_NORMAL), NULL)));
}

void ArmsWarriorStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarriorStrategy::InitNonCombatTriggers(triggers);
}

void ArmsWarriorStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    WarriorStrategy::InitReactionTriggers(triggers);
}

void ArmsWarriorStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    WarriorStrategy::InitDeadTriggers(triggers);
}

void ArmsWarriorPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArmsWarriorStrategy::InitCombatTriggers(triggers);
    WarriorPveStrategy::InitCombatTriggers(triggers);
}

void ArmsWarriorPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArmsWarriorStrategy::InitNonCombatTriggers(triggers);
    WarriorPveStrategy::InitNonCombatTriggers(triggers);
}

void ArmsWarriorPveStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ArmsWarriorStrategy::InitReactionTriggers(triggers);
    WarriorPveStrategy::InitReactionTriggers(triggers);
}

void ArmsWarriorPveStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ArmsWarriorStrategy::InitDeadTriggers(triggers);
    WarriorPveStrategy::InitDeadTriggers(triggers);
}

void ArmsWarriorPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArmsWarriorStrategy::InitCombatTriggers(triggers);
    WarriorPvpStrategy::InitCombatTriggers(triggers);
}

void ArmsWarriorPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArmsWarriorStrategy::InitNonCombatTriggers(triggers);
    WarriorPvpStrategy::InitNonCombatTriggers(triggers);
}

void ArmsWarriorPvpStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ArmsWarriorStrategy::InitReactionTriggers(triggers);
    WarriorPvpStrategy::InitReactionTriggers(triggers);
}

void ArmsWarriorPvpStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ArmsWarriorStrategy::InitDeadTriggers(triggers);
    WarriorPvpStrategy::InitDeadTriggers(triggers);
}

void ArmsWarriorRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArmsWarriorStrategy::InitCombatTriggers(triggers);
    WarriorRaidStrategy::InitCombatTriggers(triggers);
}

void ArmsWarriorRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArmsWarriorStrategy::InitNonCombatTriggers(triggers);
    WarriorRaidStrategy::InitNonCombatTriggers(triggers);
}

void ArmsWarriorRaidStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ArmsWarriorStrategy::InitReactionTriggers(triggers);
    WarriorRaidStrategy::InitReactionTriggers(triggers);
}

void ArmsWarriorRaidStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ArmsWarriorStrategy::InitDeadTriggers(triggers);
    WarriorRaidStrategy::InitDeadTriggers(triggers);
}

void ArmsWarriorAoeStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarriorAoeStrategy::InitCombatTriggers(triggers);
}

void ArmsWarriorAoeStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarriorAoeStrategy::InitNonCombatTriggers(triggers);
}

void ArmsWarriorAoePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArmsWarriorAoeStrategy::InitCombatTriggers(triggers);
    WarriorAoePveStrategy::InitCombatTriggers(triggers);
}

void ArmsWarriorAoePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArmsWarriorAoeStrategy::InitNonCombatTriggers(triggers);
    WarriorAoePveStrategy::InitNonCombatTriggers(triggers);
}

void ArmsWarriorAoePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArmsWarriorAoeStrategy::InitCombatTriggers(triggers);
    WarriorAoePvpStrategy::InitCombatTriggers(triggers);
}

void ArmsWarriorAoePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArmsWarriorAoeStrategy::InitNonCombatTriggers(triggers);
    WarriorAoePvpStrategy::InitNonCombatTriggers(triggers);
}

void ArmsWarriorAoeRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArmsWarriorAoeStrategy::InitCombatTriggers(triggers);
    WarriorAoeRaidStrategy::InitCombatTriggers(triggers);
}

void ArmsWarriorAoeRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArmsWarriorAoeStrategy::InitNonCombatTriggers(triggers);
    WarriorAoeRaidStrategy::InitNonCombatTriggers(triggers);
}

void ArmsWarriorBuffStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarriorBuffStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "berserker stance",
        NextAction::array(0, new NextAction("berserker stance", ACTION_MOVE), NULL)));

    triggers.push_back(new TriggerNode(
        "death wish",
        NextAction::array(0, new NextAction("death wish", ACTION_HIGH), NULL)));

    triggers.push_back(new TriggerNode(
        "feared",
        NextAction::array(0, new NextAction("berserker rage fear", ACTION_INTERRUPT), NULL)));
}

void ArmsWarriorBuffStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarriorBuffStrategy::InitNonCombatTriggers(triggers);
}

void ArmsWarriorBuffPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArmsWarriorBuffStrategy::InitCombatTriggers(triggers);
    WarriorBuffPveStrategy::InitCombatTriggers(triggers);
}

void ArmsWarriorBuffPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArmsWarriorBuffStrategy::InitNonCombatTriggers(triggers);
    WarriorBuffPveStrategy::InitNonCombatTriggers(triggers);
}

void ArmsWarriorBuffPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArmsWarriorBuffStrategy::InitCombatTriggers(triggers);
    WarriorBuffPvpStrategy::InitCombatTriggers(triggers);
}

void ArmsWarriorBuffPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArmsWarriorBuffStrategy::InitNonCombatTriggers(triggers);
    WarriorBuffPvpStrategy::InitNonCombatTriggers(triggers);
}

void ArmsWarriorBuffRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArmsWarriorBuffStrategy::InitCombatTriggers(triggers);
    WarriorBuffRaidStrategy::InitCombatTriggers(triggers);
}

void ArmsWarriorBuffRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArmsWarriorBuffStrategy::InitNonCombatTriggers(triggers);
    WarriorBuffRaidStrategy::InitNonCombatTriggers(triggers);
}

void ArmsWarriorBoostStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarriorBoostStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "recklessness",
        NextAction::array(0, new NextAction("recklessness", ACTION_HIGH), NULL)));
}

void ArmsWarriorBoostStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarriorBoostStrategy::InitNonCombatTriggers(triggers);
}

void ArmsWarriorBoostPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArmsWarriorBoostStrategy::InitCombatTriggers(triggers);
    WarriorBoostPveStrategy::InitCombatTriggers(triggers);
}

void ArmsWarriorBoostPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArmsWarriorBoostStrategy::InitNonCombatTriggers(triggers);
    WarriorBoostPveStrategy::InitNonCombatTriggers(triggers);
}

void ArmsWarriorBoostPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArmsWarriorBoostStrategy::InitCombatTriggers(triggers);
    WarriorBoostPvpStrategy::InitCombatTriggers(triggers);
}

void ArmsWarriorBoostPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArmsWarriorBoostStrategy::InitNonCombatTriggers(triggers);
    WarriorBoostPvpStrategy::InitNonCombatTriggers(triggers);
}

void ArmsWarriorBoostRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArmsWarriorBoostStrategy::InitCombatTriggers(triggers);
    WarriorBoostRaidStrategy::InitCombatTriggers(triggers);
}

void ArmsWarriorBoostRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArmsWarriorBoostStrategy::InitNonCombatTriggers(triggers);
    WarriorBoostRaidStrategy::InitNonCombatTriggers(triggers);
}

void ArmsWarriorCcStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarriorCcStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "hamstring",
        NextAction::array(0, new NextAction("piercing howl", ACTION_HIGH), NULL)));
}

void ArmsWarriorCcStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarriorCcStrategy::InitNonCombatTriggers(triggers);
}

void ArmsWarriorCcPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArmsWarriorCcStrategy::InitCombatTriggers(triggers);
    WarriorCcPveStrategy::InitCombatTriggers(triggers);
}

void ArmsWarriorCcPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArmsWarriorCcStrategy::InitNonCombatTriggers(triggers);
    WarriorCcPveStrategy::InitNonCombatTriggers(triggers);
}

void ArmsWarriorCcPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArmsWarriorCcStrategy::InitCombatTriggers(triggers);
    WarriorCcPvpStrategy::InitCombatTriggers(triggers);
}

void ArmsWarriorCcPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArmsWarriorCcStrategy::InitNonCombatTriggers(triggers);
    WarriorCcPvpStrategy::InitNonCombatTriggers(triggers);
}

void ArmsWarriorCcRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArmsWarriorCcStrategy::InitCombatTriggers(triggers);
    WarriorCcRaidStrategy::InitCombatTriggers(triggers);
}

void ArmsWarriorCcRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArmsWarriorCcStrategy::InitNonCombatTriggers(triggers);
    WarriorCcRaidStrategy::InitNonCombatTriggers(triggers);
}

#endif
#ifdef MANGOSBOT_TWO // WOTLK

NextAction** ArmsWarriorStrategy::GetDefaultCombatActions()
{
    return NextAction::array(0, new NextAction("melee", ACTION_IDLE), NULL);
}

void ArmsWarriorStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarriorStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "critical health",
        NextAction::array(0, new NextAction("intimidating shout", ACTION_EMERGENCY), NULL)));

    triggers.push_back(new TriggerNode(
        "enemy out of melee",
        NextAction::array(0, new NextAction("charge", ACTION_MOVE), NULL)));

    triggers.push_back(new TriggerNode(
        "target critical health",
        NextAction::array(0, new NextAction("execute", ACTION_HIGH + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "sudden death",
        NextAction::array(0, new NextAction("execute", ACTION_HIGH + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "overpower",
        NextAction::array(0, new NextAction("overpower", ACTION_HIGH), NULL)));

    triggers.push_back(new TriggerNode(
        "taste for blood",
        NextAction::array(0, new NextAction("overpower", ACTION_HIGH), NULL)));

    triggers.push_back(new TriggerNode(
        "heroic strike",
        NextAction::array(0, new NextAction("heroic strike", ACTION_NORMAL + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "mortal strike",
        NextAction::array(0, new NextAction("mortal strike", ACTION_NORMAL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "rend",
        NextAction::array(0, new NextAction("rend", ACTION_NORMAL), NULL)));
}

void ArmsWarriorStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarriorStrategy::InitNonCombatTriggers(triggers);
}

void ArmsWarriorStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    WarriorStrategy::InitReactionTriggers(triggers);
}

void ArmsWarriorStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    WarriorStrategy::InitDeadTriggers(triggers);
}

void ArmsWarriorPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArmsWarriorStrategy::InitCombatTriggers(triggers);
    WarriorPveStrategy::InitCombatTriggers(triggers);
}

void ArmsWarriorPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArmsWarriorStrategy::InitNonCombatTriggers(triggers);
    WarriorPveStrategy::InitNonCombatTriggers(triggers);
}

void ArmsWarriorPveStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ArmsWarriorStrategy::InitReactionTriggers(triggers);
    WarriorPveStrategy::InitReactionTriggers(triggers);
}

void ArmsWarriorPveStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ArmsWarriorStrategy::InitDeadTriggers(triggers);
    WarriorPveStrategy::InitDeadTriggers(triggers);
}

void ArmsWarriorPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArmsWarriorStrategy::InitCombatTriggers(triggers);
    WarriorPvpStrategy::InitCombatTriggers(triggers);
}

void ArmsWarriorPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArmsWarriorStrategy::InitNonCombatTriggers(triggers);
    WarriorPvpStrategy::InitNonCombatTriggers(triggers);
}

void ArmsWarriorPvpStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ArmsWarriorStrategy::InitReactionTriggers(triggers);
    WarriorPvpStrategy::InitReactionTriggers(triggers);
}

void ArmsWarriorPvpStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ArmsWarriorStrategy::InitDeadTriggers(triggers);
    WarriorPvpStrategy::InitDeadTriggers(triggers);
}

void ArmsWarriorRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArmsWarriorStrategy::InitCombatTriggers(triggers);
    WarriorRaidStrategy::InitCombatTriggers(triggers);
}

void ArmsWarriorRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArmsWarriorStrategy::InitNonCombatTriggers(triggers);
    WarriorRaidStrategy::InitNonCombatTriggers(triggers);
}

void ArmsWarriorRaidStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ArmsWarriorStrategy::InitReactionTriggers(triggers);
    WarriorRaidStrategy::InitReactionTriggers(triggers);
}

void ArmsWarriorRaidStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ArmsWarriorStrategy::InitDeadTriggers(triggers);
    WarriorRaidStrategy::InitDeadTriggers(triggers);
}

void ArmsWarriorAoeStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarriorAoeStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "melee medium aoe",
        NextAction::array(0, new NextAction("bladestorm", ACTION_HIGH + 5), NULL)));

    triggers.push_back(new TriggerNode(
        "melee light aoe",
        NextAction::array(0, new NextAction("sweeping strikes", ACTION_HIGH + 4), NULL)));
}

void ArmsWarriorAoeStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarriorAoeStrategy::InitNonCombatTriggers(triggers);
}

void ArmsWarriorAoePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArmsWarriorAoeStrategy::InitCombatTriggers(triggers);
    WarriorAoePveStrategy::InitCombatTriggers(triggers);
}

void ArmsWarriorAoePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArmsWarriorAoeStrategy::InitNonCombatTriggers(triggers);
    WarriorAoePveStrategy::InitNonCombatTriggers(triggers);
}

void ArmsWarriorAoePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArmsWarriorAoeStrategy::InitCombatTriggers(triggers);
    WarriorAoePvpStrategy::InitCombatTriggers(triggers);
}

void ArmsWarriorAoePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArmsWarriorAoeStrategy::InitNonCombatTriggers(triggers);
    WarriorAoePvpStrategy::InitNonCombatTriggers(triggers);
}

void ArmsWarriorAoeRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArmsWarriorAoeStrategy::InitCombatTriggers(triggers);
    WarriorAoeRaidStrategy::InitCombatTriggers(triggers);
}

void ArmsWarriorAoeRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArmsWarriorAoeStrategy::InitNonCombatTriggers(triggers);
    WarriorAoeRaidStrategy::InitNonCombatTriggers(triggers);
}

void ArmsWarriorBuffStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarriorBuffStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "berserker stance",
        NextAction::array(0, new NextAction("berserker stance", ACTION_MOVE), NULL)));

    triggers.push_back(new TriggerNode(
        "feared",
        NextAction::array(0, new NextAction("berserker rage fear", ACTION_INTERRUPT), NULL)));
}

void ArmsWarriorBuffStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarriorBuffStrategy::InitNonCombatTriggers(triggers);
}

void ArmsWarriorBuffPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArmsWarriorBuffStrategy::InitCombatTriggers(triggers);
    WarriorBuffPveStrategy::InitCombatTriggers(triggers);
}

void ArmsWarriorBuffPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArmsWarriorBuffStrategy::InitNonCombatTriggers(triggers);
    WarriorBuffPveStrategy::InitNonCombatTriggers(triggers);
}

void ArmsWarriorBuffPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArmsWarriorBuffStrategy::InitCombatTriggers(triggers);
    WarriorBuffPvpStrategy::InitCombatTriggers(triggers);
}

void ArmsWarriorBuffPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArmsWarriorBuffStrategy::InitNonCombatTriggers(triggers);
    WarriorBuffPvpStrategy::InitNonCombatTriggers(triggers);
}

void ArmsWarriorBuffRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArmsWarriorBuffStrategy::InitCombatTriggers(triggers);
    WarriorBuffRaidStrategy::InitCombatTriggers(triggers);
}

void ArmsWarriorBuffRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArmsWarriorBuffStrategy::InitNonCombatTriggers(triggers);
    WarriorBuffRaidStrategy::InitNonCombatTriggers(triggers);
}

void ArmsWarriorBoostStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarriorBoostStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "recklessness",
        NextAction::array(0, new NextAction("recklessness", ACTION_HIGH), NULL)));
}

void ArmsWarriorBoostStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarriorBoostStrategy::InitNonCombatTriggers(triggers);
}

void ArmsWarriorBoostPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArmsWarriorBoostStrategy::InitCombatTriggers(triggers);
    WarriorBoostPveStrategy::InitCombatTriggers(triggers);
}

void ArmsWarriorBoostPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArmsWarriorBoostStrategy::InitNonCombatTriggers(triggers);
    WarriorBoostPveStrategy::InitNonCombatTriggers(triggers);
}

void ArmsWarriorBoostPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArmsWarriorBoostStrategy::InitCombatTriggers(triggers);
    WarriorBoostPvpStrategy::InitCombatTriggers(triggers);
}

void ArmsWarriorBoostPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArmsWarriorBoostStrategy::InitNonCombatTriggers(triggers);
    WarriorBoostPvpStrategy::InitNonCombatTriggers(triggers);
}

void ArmsWarriorBoostRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArmsWarriorBoostStrategy::InitCombatTriggers(triggers);
    WarriorBoostRaidStrategy::InitCombatTriggers(triggers);
}

void ArmsWarriorBoostRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArmsWarriorBoostStrategy::InitNonCombatTriggers(triggers);
    WarriorBoostRaidStrategy::InitNonCombatTriggers(triggers);
}

void ArmsWarriorCcStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarriorCcStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "hamstring",
        NextAction::array(0, new NextAction("piercing howl", ACTION_HIGH), NULL)));
}

void ArmsWarriorCcStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarriorCcStrategy::InitNonCombatTriggers(triggers);
}

void ArmsWarriorCcPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArmsWarriorCcStrategy::InitCombatTriggers(triggers);
    WarriorCcPveStrategy::InitCombatTriggers(triggers);
}

void ArmsWarriorCcPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArmsWarriorCcStrategy::InitNonCombatTriggers(triggers);
    WarriorCcPveStrategy::InitNonCombatTriggers(triggers);
}

void ArmsWarriorCcPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArmsWarriorCcStrategy::InitCombatTriggers(triggers);
    WarriorCcPvpStrategy::InitCombatTriggers(triggers);
}

void ArmsWarriorCcPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArmsWarriorCcStrategy::InitNonCombatTriggers(triggers);
    WarriorCcPvpStrategy::InitNonCombatTriggers(triggers);
}

void ArmsWarriorCcRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArmsWarriorCcStrategy::InitCombatTriggers(triggers);
    WarriorCcRaidStrategy::InitCombatTriggers(triggers);
}

void ArmsWarriorCcRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ArmsWarriorCcStrategy::InitNonCombatTriggers(triggers);
    WarriorCcRaidStrategy::InitNonCombatTriggers(triggers);
}

#endif

class WarriorSweepingStrikesPveMultiplier : public Multiplier
{
public:
    WarriorSweepingStrikesPveMultiplier(PlayerbotAI* ai) : Multiplier(ai, "aoe arms pve") {}

    float GetValue(Action* action) override
    {
        // Disable Berserker Stance
        const std::string& actionName = action->getName();
        if ((actionName == "berserker stance" ||
            actionName == "whirlwind" ||
            actionName == "mortal strike" ||
            actionName == "heroic strike" ||
            actionName == "cleave") &&
            AI_VALUE2(bool, "trigger active", "melee light aoe") &&
            bot->HasSpell(12292) &&
            bot->IsSpellReady(12292) &&
            !bot->HasAura(12292))
        {
            return 0.0f;
        }

        // Disable Battle Stance spam if SS is on CD
        if ((actionName == "battle stance") &&
            AI_VALUE2(bool, "trigger active", "melee light aoe") &&
            bot->HasSpell(12292) &&
            !bot->IsSpellReady(12292))
        {
            return 0.0f;
        }

        return 1.0f;
    }
};

class WarriorSweepingStrikesRaidMultiplier : public Multiplier
{
public:
    WarriorSweepingStrikesRaidMultiplier(PlayerbotAI* ai) : Multiplier(ai, "aoe arms raid") {}

    float GetValue(Action* action) override
    {
        // Disable Berserker Stance
        const std::string& actionName = action->getName();
        if ((actionName == "berserker stance" ||
            actionName == "whirlwind" ||
            actionName == "mortal strike" ||
            actionName == "heroic strike" ||
            actionName == "cleave") &&
            AI_VALUE2(bool, "trigger active", "melee light aoe") &&
            bot->HasSpell(12292) &&
            bot->IsSpellReady(12292) &&
            !bot->HasAura(12292))
        {
            return 0.0f;
        }

        // Disable Battle Stance spam if SS is on CD
        if ((actionName == "battle stance") &&
            AI_VALUE2(bool, "trigger active", "melee light aoe") &&
            bot->HasSpell(12292) &&
            !bot->IsSpellReady(12292))
        {
            return 0.0f;
        }

        return 1.0f;
    }
};

class WarriorSweepingStrikesPvpMultiplier : public Multiplier
{
public:
    WarriorSweepingStrikesPvpMultiplier(PlayerbotAI* ai) : Multiplier(ai, "aoe arms pvp") {}

    float GetValue(Action* action) override
    {
        // Disable Berserker Stance
        const std::string& actionName = action->getName();
        if ((actionName == "berserker stance" ||
            actionName == "whirlwind" ||
            actionName == "mortal strike" ||
            actionName == "heroic strike" ||
            actionName == "cleave") &&
            AI_VALUE2(bool, "trigger active", "melee light aoe") &&
            bot->HasSpell(12292) &&
            bot->IsSpellReady(12292) &&
            !bot->HasAura(12292))
        {
            return 0.0f;
        }

        // Disable Battle Stance spam if SS is on CD
        if ((actionName == "battle stance") &&
            AI_VALUE2(bool, "trigger active", "melee light aoe") &&
            bot->HasSpell(12292) &&
            !bot->IsSpellReady(12292))
        {
            return 0.0f;
        }

        return 1.0f;
    }
};

void ArmsWarriorAoeStrategy::InitCombatMultipliers(std::list<Multiplier*>& multipliers)
{
    multipliers.push_back(new WarriorSweepingStrikesPveMultiplier(ai));
    multipliers.push_back(new WarriorSweepingStrikesRaidMultiplier(ai));
    multipliers.push_back(new WarriorSweepingStrikesPvpMultiplier(ai));
}
