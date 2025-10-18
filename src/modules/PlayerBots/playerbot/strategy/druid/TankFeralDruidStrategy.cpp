
#include "playerbot/playerbot.h"
#include "DruidMultipliers.h"
#include "TankFeralDruidStrategy.h"

using namespace ai;

class TankFeralDruidStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    TankFeralDruidStrategyActionNodeFactory()
    {
        creators["survival instincts"] = &survival_instincts;
        creators["omen of clarity"] = &omen_of_clarity;
        creators["feral charge - bear"] = &feral_charge_bear;
        creators["mangle (bear)"] = &mangle_bear;
    }

private:
    ACTION_NODE_A(survival_instincts, "survival instincts", "barskin");

    ACTION_NODE_P(omen_of_clarity, "omen of clarity", "caster form");

    ACTION_NODE_A(feral_charge_bear, "feral charge - bear", "reach melee");

    ACTION_NODE_A(mangle_bear, "mangle (bear)", "maul");
};

TankFeralDruidStrategy::TankFeralDruidStrategy(PlayerbotAI* ai) : DruidStrategy(ai)
{
    actionNodeFactories.Add(new TankFeralDruidStrategyActionNodeFactory());
}

#ifdef MANGOSBOT_ZERO // Vanilla

NextAction** TankFeralDruidStrategy::GetDefaultCombatActions()
{
    return NextAction::array(0, new NextAction("melee", ACTION_IDLE), NULL);
}

void TankFeralDruidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DruidStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "has blessing of salvation",
        NextAction::array(0, new NextAction("remove blessing of salvation", ACTION_EMERGENCY), NULL)));

    triggers.push_back(new TriggerNode(
        "has greater blessing of salvation",
        NextAction::array(0, new NextAction("remove greater blessing of salvation", ACTION_EMERGENCY), NULL)));

    triggers.push_back(new TriggerNode(
        "low health",
        NextAction::array(0, new NextAction("frenzied regeneration", ACTION_LIGHT_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "enemy out of melee",
        NextAction::array(0, new NextAction("feral charge - bear", ACTION_MOVE + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "lose aggro",
        NextAction::array(0, new NextAction("growl", ACTION_MOVE), NULL)));

    triggers.push_back(new TriggerNode(
        "faerie fire (feral)",
        NextAction::array(0, new NextAction("faerie fire (feral)", ACTION_HIGH), NULL)));

    triggers.push_back(new TriggerNode(
        "medium rage available",
        NextAction::array(0, new NextAction("mangle (bear)", ACTION_NORMAL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "enemy too close for melee",
        NextAction::array(0, new NextAction("move out of enemy contact", ACTION_NORMAL), NULL)));
}

void TankFeralDruidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DruidStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "has blessing of salvation",
        NextAction::array(0, new NextAction("remove blessing of salvation", ACTION_EMERGENCY), NULL)));

    triggers.push_back(new TriggerNode(
        "has greater blessing of salvation",
        NextAction::array(0, new NextAction("remove greater blessing of salvation", ACTION_EMERGENCY), NULL)));
}

void TankFeralDruidStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    DruidStrategy::InitReactionTriggers(triggers);
}

void TankFeralDruidStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    DruidStrategy::InitDeadTriggers(triggers);
}

void TankFeralDruidPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
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

    TankFeralDruidStrategy::InitCombatTriggers(triggers);
    DruidPveStrategy::InitCombatTriggers(triggers);
}

void TankFeralDruidPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidStrategy::InitNonCombatTriggers(triggers);
    DruidPveStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "low health",
        NextAction::array(0, new NextAction("regrowth", ACTION_MEDIUM_HEAL), NULL)));
}

void TankFeralDruidPveStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidStrategy::InitReactionTriggers(triggers);
    DruidPveStrategy::InitReactionTriggers(triggers);
}

void TankFeralDruidPveStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidStrategy::InitDeadTriggers(triggers);
    DruidPveStrategy::InitDeadTriggers(triggers);
}

void TankFeralDruidPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
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

    TankFeralDruidStrategy::InitCombatTriggers(triggers);
    DruidPvpStrategy::InitCombatTriggers(triggers);
}

void TankFeralDruidPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidStrategy::InitNonCombatTriggers(triggers);
    DruidPvpStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "low health",
        NextAction::array(0, new NextAction("regrowth", ACTION_MEDIUM_HEAL), NULL)));
}

void TankFeralDruidPvpStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidStrategy::InitReactionTriggers(triggers);
    DruidPvpStrategy::InitReactionTriggers(triggers);
}

void TankFeralDruidPvpStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidStrategy::InitDeadTriggers(triggers);
    DruidPvpStrategy::InitDeadTriggers(triggers);
}

void TankFeralDruidRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidStrategy::InitCombatTriggers(triggers);
    DruidRaidStrategy::InitCombatTriggers(triggers);
}

void TankFeralDruidRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidStrategy::InitNonCombatTriggers(triggers);
    DruidRaidStrategy::InitNonCombatTriggers(triggers);
}

void TankFeralDruidRaidStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidStrategy::InitReactionTriggers(triggers);
    DruidRaidStrategy::InitReactionTriggers(triggers);
}

void TankFeralDruidRaidStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidStrategy::InitDeadTriggers(triggers);
    DruidRaidStrategy::InitDeadTriggers(triggers);
}

void TankFeralDruidAoeStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DruidAoeStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "melee medium aoe",
        NextAction::array(0, new NextAction("challenging roar", ACTION_HIGH + 3), NULL)));

    triggers.push_back(new TriggerNode(
        "melee medium aoe",
        NextAction::array(0, new NextAction("demoralizing roar", ACTION_HIGH + 2),
                             new NextAction("swipe (bear)", ACTION_HIGH + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "melee light aoe",
        NextAction::array(0, new NextAction("swipe (bear)", ACTION_HIGH), NULL)));
}

void TankFeralDruidAoeStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DruidAoeStrategy::InitNonCombatTriggers(triggers);
}

void TankFeralDruidAoePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidAoeStrategy::InitCombatTriggers(triggers);
    DruidAoePveStrategy::InitCombatTriggers(triggers);
}

void TankFeralDruidAoePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidAoeStrategy::InitNonCombatTriggers(triggers);
    DruidAoePveStrategy::InitNonCombatTriggers(triggers);
}

void TankFeralDruidAoePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidAoeStrategy::InitCombatTriggers(triggers);
    DruidAoePvpStrategy::InitCombatTriggers(triggers);
}

void TankFeralDruidAoePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidAoeStrategy::InitNonCombatTriggers(triggers);
    DruidAoePvpStrategy::InitNonCombatTriggers(triggers);
}

void TankFeralDruidAoeRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidAoeStrategy::InitCombatTriggers(triggers);
    DruidAoeRaidStrategy::InitCombatTriggers(triggers);
}

void TankFeralDruidAoeRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidAoeStrategy::InitNonCombatTriggers(triggers);
    DruidAoeRaidStrategy::InitNonCombatTriggers(triggers);
}

void TankFeralDruidBuffStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DruidBuffStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "bear form",
        NextAction::array(0, new NextAction("dire bear form", ACTION_MOVE + 9), NULL)));

    triggers.push_back(new TriggerNode(
        "enrage",
        NextAction::array(0, new NextAction("enrage", ACTION_HIGH), NULL)));
}

void TankFeralDruidBuffStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DruidBuffStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "omen of clarity",
        NextAction::array(0, new NextAction("omen of clarity", ACTION_NORMAL), NULL)));
}

void TankFeralDruidBuffPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidBuffStrategy::InitCombatTriggers(triggers);
    DruidBuffPveStrategy::InitCombatTriggers(triggers);
}

void TankFeralDruidBuffPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidBuffStrategy::InitNonCombatTriggers(triggers);
    DruidBuffPveStrategy::InitNonCombatTriggers(triggers);
}

void TankFeralDruidBuffPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidBuffStrategy::InitCombatTriggers(triggers);
    DruidBuffPvpStrategy::InitCombatTriggers(triggers);
}

void TankFeralDruidBuffPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidBuffStrategy::InitNonCombatTriggers(triggers);
    DruidBuffPvpStrategy::InitNonCombatTriggers(triggers);
}

void TankFeralDruidBuffRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidBuffStrategy::InitCombatTriggers(triggers);
    DruidBuffRaidStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "often",
        NextAction::array(0, new NextAction("stoneshield potion", ACTION_HIGH), NULL)));
}

void TankFeralDruidBuffRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidBuffStrategy::InitNonCombatTriggers(triggers);
    DruidBuffRaidStrategy::InitNonCombatTriggers(triggers);
}

void TankFeralDruidBoostStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DruidBoostStrategy::InitCombatTriggers(triggers);
}

void TankFeralDruidBoostStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DruidBoostStrategy::InitNonCombatTriggers(triggers);
}

void TankFeralDruidBoostPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidBoostStrategy::InitCombatTriggers(triggers);
    DruidBoostPveStrategy::InitCombatTriggers(triggers);
}

void TankFeralDruidBoostPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidBoostStrategy::InitNonCombatTriggers(triggers);
    DruidBoostPveStrategy::InitNonCombatTriggers(triggers);
}

void TankFeralDruidBoostPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidBoostStrategy::InitCombatTriggers(triggers);
    DruidBoostPvpStrategy::InitCombatTriggers(triggers);
}

void TankFeralDruidBoostPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidBoostStrategy::InitNonCombatTriggers(triggers);
    DruidBoostPvpStrategy::InitNonCombatTriggers(triggers);
}

void TankFeralDruidBoostRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidBoostStrategy::InitCombatTriggers(triggers);
    DruidBoostRaidStrategy::InitCombatTriggers(triggers);
}

void TankFeralDruidBoostRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidBoostStrategy::InitNonCombatTriggers(triggers);
    DruidBoostRaidStrategy::InitNonCombatTriggers(triggers);
}

void TankFeralDruidCcStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DruidCcStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "bash",
        NextAction::array(0, new NextAction("bash", ACTION_INTERRUPT), NULL)));

    triggers.push_back(new TriggerNode(
        "bash on enemy healer",
        NextAction::array(0, new NextAction("bash on enemy healer", ACTION_INTERRUPT), NULL)));
}

void TankFeralDruidCcStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DruidCcStrategy::InitNonCombatTriggers(triggers);
}

void TankFeralDruidCcPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidCcStrategy::InitCombatTriggers(triggers);
    DruidCcPveStrategy::InitCombatTriggers(triggers);
}

void TankFeralDruidCcPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidCcStrategy::InitNonCombatTriggers(triggers);
    DruidCcPveStrategy::InitNonCombatTriggers(triggers);
}

void TankFeralDruidCcPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidCcStrategy::InitCombatTriggers(triggers);
    DruidCcPvpStrategy::InitCombatTriggers(triggers);
}

void TankFeralDruidCcPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidCcStrategy::InitNonCombatTriggers(triggers);
    DruidCcPvpStrategy::InitNonCombatTriggers(triggers);
}

void TankFeralDruidCcRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidCcStrategy::InitCombatTriggers(triggers);
    DruidCcRaidStrategy::InitCombatTriggers(triggers);
}

void TankFeralDruidCcRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidCcStrategy::InitNonCombatTriggers(triggers);
    DruidCcRaidStrategy::InitNonCombatTriggers(triggers);
}

void TankFeralDruidCureStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DruidCureStrategy::InitCombatTriggers(triggers);
}

void TankFeralDruidCureStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DruidCureStrategy::InitNonCombatTriggers(triggers);
}

void TankFeralDruidCurePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidCureStrategy::InitCombatTriggers(triggers);
    DruidCurePveStrategy::InitCombatTriggers(triggers);
}

void TankFeralDruidCurePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidCureStrategy::InitNonCombatTriggers(triggers);
    DruidCurePveStrategy::InitNonCombatTriggers(triggers);
}

void TankFeralDruidCurePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidCureStrategy::InitCombatTriggers(triggers);
    DruidCurePvpStrategy::InitCombatTriggers(triggers);
}

void TankFeralDruidCurePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidCureStrategy::InitNonCombatTriggers(triggers);
    DruidCurePvpStrategy::InitNonCombatTriggers(triggers);
}

void TankFeralDruidCureRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidCureStrategy::InitCombatTriggers(triggers);
    DruidCureRaidStrategy::InitCombatTriggers(triggers);
}

void TankFeralDruidCureRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidCureStrategy::InitNonCombatTriggers(triggers);
    DruidCureRaidStrategy::InitNonCombatTriggers(triggers);
}

#endif

#ifdef MANGOSBOT_ONE // TBC

NextAction** TankFeralDruidStrategy::GetDefaultCombatActions()
{
    return NextAction::array(0, new NextAction("melee", ACTION_IDLE), NULL);
}

void TankFeralDruidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DruidStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "has blessing of salvation",
        NextAction::array(0, new NextAction("remove blessing of salvation", ACTION_EMERGENCY), NULL)));

    triggers.push_back(new TriggerNode(
        "has greater blessing of salvation",
        NextAction::array(0, new NextAction("remove greater blessing of salvation", ACTION_EMERGENCY), NULL)));

    triggers.push_back(new TriggerNode(
        "critical health",
        NextAction::array(0, new NextAction("survival instincts", ACTION_EMERGENCY), NULL)));

    triggers.push_back(new TriggerNode(
        "low health",
        NextAction::array(0, new NextAction("frenzied regeneration", ACTION_LIGHT_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "enemy out of melee",
        NextAction::array(0, new NextAction("feral charge - bear", ACTION_MOVE + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "lose aggro",
        NextAction::array(0, new NextAction("growl", ACTION_MOVE), NULL)));

    triggers.push_back(new TriggerNode(
        "faerie fire (feral)",
        NextAction::array(0, new NextAction("faerie fire (feral)", ACTION_HIGH + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "lacerate",
        NextAction::array(0, new NextAction("lacerate", ACTION_HIGH), NULL)));

    triggers.push_back(new TriggerNode(
        "medium rage available",
        NextAction::array(0, new NextAction("mangle (bear)", ACTION_NORMAL + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "enemy too close for melee",
        NextAction::array(0, new NextAction("move out of enemy contact", ACTION_NORMAL), NULL)));
}

void TankFeralDruidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DruidStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "has blessing of salvation",
        NextAction::array(0, new NextAction("remove blessing of salvation", ACTION_EMERGENCY), NULL)));

    triggers.push_back(new TriggerNode(
        "has greater blessing of salvation",
        NextAction::array(0, new NextAction("remove greater blessing of salvation", ACTION_EMERGENCY), NULL)));
}

void TankFeralDruidStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    DruidStrategy::InitReactionTriggers(triggers);
}

void TankFeralDruidStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    DruidStrategy::InitDeadTriggers(triggers);
}

void TankFeralDruidPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidStrategy::InitCombatTriggers(triggers);
    DruidPveStrategy::InitCombatTriggers(triggers);

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
}

void TankFeralDruidPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidStrategy::InitNonCombatTriggers(triggers);
    DruidPveStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "low health",
        NextAction::array(0, new NextAction("regrowth", ACTION_MEDIUM_HEAL), NULL)));
}

void TankFeralDruidPveStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidStrategy::InitReactionTriggers(triggers);
    DruidPveStrategy::InitReactionTriggers(triggers);
}

void TankFeralDruidPveStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidStrategy::InitDeadTriggers(triggers);
    DruidPveStrategy::InitDeadTriggers(triggers);
}

void TankFeralDruidPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidStrategy::InitCombatTriggers(triggers);
    DruidPvpStrategy::InitCombatTriggers(triggers);

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
}

void TankFeralDruidPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidStrategy::InitNonCombatTriggers(triggers);
    DruidPvpStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "low health",
        NextAction::array(0, new NextAction("regrowth", ACTION_MEDIUM_HEAL), NULL)));
}

void TankFeralDruidPvpStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidStrategy::InitReactionTriggers(triggers);
    DruidPvpStrategy::InitReactionTriggers(triggers);
}

void TankFeralDruidPvpStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidStrategy::InitDeadTriggers(triggers);
    DruidPvpStrategy::InitDeadTriggers(triggers);
}

void TankFeralDruidRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidStrategy::InitCombatTriggers(triggers);
    DruidRaidStrategy::InitCombatTriggers(triggers);
}

void TankFeralDruidRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidStrategy::InitNonCombatTriggers(triggers);
    DruidRaidStrategy::InitNonCombatTriggers(triggers);
}

void TankFeralDruidRaidStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidStrategy::InitReactionTriggers(triggers);
    DruidRaidStrategy::InitReactionTriggers(triggers);
}

void TankFeralDruidRaidStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidStrategy::InitDeadTriggers(triggers);
    DruidRaidStrategy::InitDeadTriggers(triggers);
}

void TankFeralDruidAoeStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DruidAoeStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "melee medium aoe",
        NextAction::array(0, new NextAction("challenging roar", ACTION_HIGH + 3), NULL)));

    triggers.push_back(new TriggerNode(
        "melee medium aoe",
        NextAction::array(0, new NextAction("demoralizing roar", ACTION_HIGH + 2),
                             new NextAction("swipe (bear)", ACTION_HIGH + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "melee light aoe",
        NextAction::array(0, new NextAction("swipe (bear)", ACTION_HIGH), NULL)));
}

void TankFeralDruidAoeStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DruidAoeStrategy::InitNonCombatTriggers(triggers);
}

void TankFeralDruidAoePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidAoeStrategy::InitCombatTriggers(triggers);
    DruidAoePveStrategy::InitCombatTriggers(triggers);
}

void TankFeralDruidAoePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidAoeStrategy::InitNonCombatTriggers(triggers);
    DruidAoePveStrategy::InitNonCombatTriggers(triggers);
}

void TankFeralDruidAoePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidAoeStrategy::InitCombatTriggers(triggers);
    DruidAoePvpStrategy::InitCombatTriggers(triggers);
}

void TankFeralDruidAoePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidAoeStrategy::InitNonCombatTriggers(triggers);
    DruidAoePvpStrategy::InitNonCombatTriggers(triggers);
}

void TankFeralDruidAoeRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidAoeStrategy::InitCombatTriggers(triggers);
    DruidAoeRaidStrategy::InitCombatTriggers(triggers);
}

void TankFeralDruidAoeRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidAoeStrategy::InitNonCombatTriggers(triggers);
    DruidAoeRaidStrategy::InitNonCombatTriggers(triggers);
}

void TankFeralDruidBuffStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DruidBuffStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "bear form",
        NextAction::array(0, new NextAction("dire bear form", ACTION_MOVE), NULL)));

    triggers.push_back(new TriggerNode(
        "enrage",
        NextAction::array(0, new NextAction("enrage", ACTION_HIGH), NULL)));
}

void TankFeralDruidBuffStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DruidBuffStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "omen of clarity",
        NextAction::array(0, new NextAction("omen of clarity", ACTION_NORMAL), NULL)));
}

void TankFeralDruidBuffPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidBuffStrategy::InitCombatTriggers(triggers);
    DruidBuffPveStrategy::InitCombatTriggers(triggers);
}

void TankFeralDruidBuffPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidBuffStrategy::InitNonCombatTriggers(triggers);
    DruidBuffPveStrategy::InitNonCombatTriggers(triggers);
}

void TankFeralDruidBuffPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidBuffStrategy::InitCombatTriggers(triggers);
    DruidBuffPvpStrategy::InitCombatTriggers(triggers);
}

void TankFeralDruidBuffPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidBuffStrategy::InitNonCombatTriggers(triggers);
    DruidBuffPvpStrategy::InitNonCombatTriggers(triggers);
}

void TankFeralDruidBuffRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidBuffStrategy::InitCombatTriggers(triggers);
    DruidBuffRaidStrategy::InitCombatTriggers(triggers);
}

void TankFeralDruidBuffRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidBuffStrategy::InitNonCombatTriggers(triggers);
    DruidBuffRaidStrategy::InitNonCombatTriggers(triggers);
}

void TankFeralDruidBoostStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DruidBoostStrategy::InitCombatTriggers(triggers);
}

void TankFeralDruidBoostStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DruidBoostStrategy::InitNonCombatTriggers(triggers);
}

void TankFeralDruidBoostPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidBoostStrategy::InitCombatTriggers(triggers);
    DruidBoostPveStrategy::InitCombatTriggers(triggers);
}

void TankFeralDruidBoostPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidBoostStrategy::InitNonCombatTriggers(triggers);
    DruidBoostPveStrategy::InitNonCombatTriggers(triggers);
}

void TankFeralDruidBoostPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidBoostStrategy::InitCombatTriggers(triggers);
    DruidBoostPvpStrategy::InitCombatTriggers(triggers);
}

void TankFeralDruidBoostPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidBoostStrategy::InitNonCombatTriggers(triggers);
    DruidBoostPvpStrategy::InitNonCombatTriggers(triggers);
}

void TankFeralDruidBoostRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidBoostStrategy::InitCombatTriggers(triggers);
    DruidBoostRaidStrategy::InitCombatTriggers(triggers);
}

void TankFeralDruidBoostRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidBoostStrategy::InitNonCombatTriggers(triggers);
    DruidBoostRaidStrategy::InitNonCombatTriggers(triggers);
}

void TankFeralDruidCcStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DruidCcStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "bash",
        NextAction::array(0, new NextAction("bash", ACTION_INTERRUPT), NULL)));

    triggers.push_back(new TriggerNode(
        "bash on enemy healer",
        NextAction::array(0, new NextAction("bash on enemy healer", ACTION_INTERRUPT), NULL)));
}

void TankFeralDruidCcStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DruidCcStrategy::InitNonCombatTriggers(triggers);
}

void TankFeralDruidCcPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidCcStrategy::InitCombatTriggers(triggers);
    DruidCcPveStrategy::InitCombatTriggers(triggers);
}

void TankFeralDruidCcPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidCcStrategy::InitNonCombatTriggers(triggers);
    DruidCcPveStrategy::InitNonCombatTriggers(triggers);
}

void TankFeralDruidCcPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidCcStrategy::InitCombatTriggers(triggers);
    DruidCcPvpStrategy::InitCombatTriggers(triggers);
}

void TankFeralDruidCcPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidCcStrategy::InitNonCombatTriggers(triggers);
    DruidCcPvpStrategy::InitNonCombatTriggers(triggers);
}

void TankFeralDruidCcRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidCcStrategy::InitCombatTriggers(triggers);
    DruidCcRaidStrategy::InitCombatTriggers(triggers);
}

void TankFeralDruidCcRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidCcStrategy::InitNonCombatTriggers(triggers);
    DruidCcRaidStrategy::InitNonCombatTriggers(triggers);
}

void TankFeralDruidCureStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DruidCureStrategy::InitCombatTriggers(triggers);
}

void TankFeralDruidCureStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DruidCureStrategy::InitNonCombatTriggers(triggers);
}

void TankFeralDruidCurePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidCureStrategy::InitCombatTriggers(triggers);
    DruidCurePveStrategy::InitCombatTriggers(triggers);
}

void TankFeralDruidCurePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidCureStrategy::InitNonCombatTriggers(triggers);
    DruidCurePveStrategy::InitNonCombatTriggers(triggers);
}

void TankFeralDruidCurePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidCureStrategy::InitCombatTriggers(triggers);
    DruidCurePvpStrategy::InitCombatTriggers(triggers);
}

void TankFeralDruidCurePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidCureStrategy::InitNonCombatTriggers(triggers);
    DruidCurePvpStrategy::InitNonCombatTriggers(triggers);
}

void TankFeralDruidCureRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidCureStrategy::InitCombatTriggers(triggers);
    DruidCureRaidStrategy::InitCombatTriggers(triggers);
}

void TankFeralDruidCureRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidCureStrategy::InitNonCombatTriggers(triggers);
    DruidCureRaidStrategy::InitNonCombatTriggers(triggers);
}

#endif

#ifdef MANGOSBOT_TWO // WOTLK

NextAction** TankFeralDruidStrategy::GetDefaultCombatActions()
{
    return NextAction::array(0, new NextAction("melee", ACTION_IDLE), NULL);
}

void TankFeralDruidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DruidStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "has blessing of salvation",
        NextAction::array(0, new NextAction("remove blessing of salvation", ACTION_EMERGENCY), NULL)));

    triggers.push_back(new TriggerNode(
        "has greater blessing of salvation",
        NextAction::array(0, new NextAction("remove greater blessing of salvation", ACTION_EMERGENCY), NULL)));

    triggers.push_back(new TriggerNode(
        "critical health",
        NextAction::array(0, new NextAction("survival instincts", ACTION_EMERGENCY), NULL)));

    triggers.push_back(new TriggerNode(
        "low health",
        NextAction::array(0, new NextAction("frenzied regeneration", ACTION_LIGHT_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "enemy out of melee",
        NextAction::array(0, new NextAction("feral charge - bear", ACTION_MOVE + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "lose aggro",
        NextAction::array(0, new NextAction("growl", ACTION_MOVE), NULL)));

    triggers.push_back(new TriggerNode(
        "faerie fire (feral)",
        NextAction::array(0, new NextAction("faerie fire (feral)", ACTION_HIGH + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "lacerate",
        NextAction::array(0, new NextAction("lacerate", ACTION_HIGH), NULL)));

    triggers.push_back(new TriggerNode(
        "medium rage available",
        NextAction::array(0, new NextAction("mangle (bear)", ACTION_NORMAL + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "enemy too close for melee",
        NextAction::array(0, new NextAction("move out of enemy contact", ACTION_NORMAL), NULL)));
}

void TankFeralDruidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DruidStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "has blessing of salvation",
        NextAction::array(0, new NextAction("remove blessing of salvation", ACTION_EMERGENCY), NULL)));

    triggers.push_back(new TriggerNode(
        "has greater blessing of salvation",
        NextAction::array(0, new NextAction("remove greater blessing of salvation", ACTION_EMERGENCY), NULL)));
}

void TankFeralDruidStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    DruidStrategy::InitReactionTriggers(triggers);
}

void TankFeralDruidStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    DruidStrategy::InitDeadTriggers(triggers);
}

void TankFeralDruidPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidStrategy::InitCombatTriggers(triggers);
    DruidPveStrategy::InitCombatTriggers(triggers);

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
}

void TankFeralDruidPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidStrategy::InitNonCombatTriggers(triggers);
    DruidPveStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "low health",
        NextAction::array(0, new NextAction("regrowth", ACTION_MEDIUM_HEAL), NULL)));
}

void TankFeralDruidPveStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidStrategy::InitReactionTriggers(triggers);
    DruidPveStrategy::InitReactionTriggers(triggers);
}

void TankFeralDruidPveStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidStrategy::InitDeadTriggers(triggers);
    DruidPveStrategy::InitDeadTriggers(triggers);
}

void TankFeralDruidPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidStrategy::InitCombatTriggers(triggers);
    DruidPvpStrategy::InitCombatTriggers(triggers);

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
}

void TankFeralDruidPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidStrategy::InitNonCombatTriggers(triggers);
    DruidPvpStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "low health",
        NextAction::array(0, new NextAction("regrowth", ACTION_MEDIUM_HEAL), NULL)));
}

void TankFeralDruidPvpStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidStrategy::InitReactionTriggers(triggers);
    DruidPvpStrategy::InitReactionTriggers(triggers);
}

void TankFeralDruidPvpStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidStrategy::InitDeadTriggers(triggers);
    DruidPvpStrategy::InitDeadTriggers(triggers);
}

void TankFeralDruidRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidStrategy::InitCombatTriggers(triggers);
    DruidRaidStrategy::InitCombatTriggers(triggers);
}

void TankFeralDruidRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidStrategy::InitNonCombatTriggers(triggers);
    DruidRaidStrategy::InitNonCombatTriggers(triggers);
}

void TankFeralDruidRaidStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidStrategy::InitReactionTriggers(triggers);
    DruidRaidStrategy::InitReactionTriggers(triggers);
}

void TankFeralDruidRaidStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidStrategy::InitDeadTriggers(triggers);
    DruidRaidStrategy::InitDeadTriggers(triggers);
}

void TankFeralDruidAoeStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DruidAoeStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "melee medium aoe",
        NextAction::array(0, new NextAction("challenging roar", ACTION_HIGH + 3), NULL)));

    triggers.push_back(new TriggerNode(
        "melee medium aoe",
        NextAction::array(0, new NextAction("demoralizing roar", ACTION_HIGH + 2),
                             new NextAction("swipe (bear)", ACTION_HIGH + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "melee light aoe",
        NextAction::array(0, new NextAction("swipe (bear)", ACTION_HIGH), NULL)));
}

void TankFeralDruidAoeStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DruidAoeStrategy::InitNonCombatTriggers(triggers);
}

void TankFeralDruidAoePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidAoeStrategy::InitCombatTriggers(triggers);
    DruidAoePveStrategy::InitCombatTriggers(triggers);
}

void TankFeralDruidAoePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidAoeStrategy::InitNonCombatTriggers(triggers);
    DruidAoePveStrategy::InitNonCombatTriggers(triggers);
}

void TankFeralDruidAoePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidAoeStrategy::InitCombatTriggers(triggers);
    DruidAoePvpStrategy::InitCombatTriggers(triggers);
}

void TankFeralDruidAoePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidAoeStrategy::InitNonCombatTriggers(triggers);
    DruidAoePvpStrategy::InitNonCombatTriggers(triggers);
}

void TankFeralDruidAoeRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidAoeStrategy::InitCombatTriggers(triggers);
    DruidAoeRaidStrategy::InitCombatTriggers(triggers);
}

void TankFeralDruidAoeRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidAoeStrategy::InitNonCombatTriggers(triggers);
    DruidAoeRaidStrategy::InitNonCombatTriggers(triggers);
}

void TankFeralDruidBuffStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DruidBuffStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "bear form",
        NextAction::array(0, new NextAction("dire bear form", ACTION_MOVE), NULL)));

    triggers.push_back(new TriggerNode(
        "enrage",
        NextAction::array(0, new NextAction("enrage", ACTION_HIGH), NULL)));
}

void TankFeralDruidBuffStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DruidBuffStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "omen of clarity",
        NextAction::array(0, new NextAction("omen of clarity", ACTION_NORMAL), NULL)));
}

void TankFeralDruidBuffPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidBuffStrategy::InitCombatTriggers(triggers);
    DruidBuffPveStrategy::InitCombatTriggers(triggers);
}

void TankFeralDruidBuffPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidBuffStrategy::InitNonCombatTriggers(triggers);
    DruidBuffPveStrategy::InitNonCombatTriggers(triggers);
}

void TankFeralDruidBuffPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidBuffStrategy::InitCombatTriggers(triggers);
    DruidBuffPvpStrategy::InitCombatTriggers(triggers);
}

void TankFeralDruidBuffPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidBuffStrategy::InitNonCombatTriggers(triggers);
    DruidBuffPvpStrategy::InitNonCombatTriggers(triggers);
}

void TankFeralDruidBuffRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidBuffStrategy::InitCombatTriggers(triggers);
    DruidBuffRaidStrategy::InitCombatTriggers(triggers);
}

void TankFeralDruidBuffRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidBuffStrategy::InitNonCombatTriggers(triggers);
    DruidBuffRaidStrategy::InitNonCombatTriggers(triggers);
}

void TankFeralDruidBoostStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DruidBoostStrategy::InitCombatTriggers(triggers);
}

void TankFeralDruidBoostStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DruidBoostStrategy::InitNonCombatTriggers(triggers);
}

void TankFeralDruidBoostPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidBoostStrategy::InitCombatTriggers(triggers);
    DruidBoostPveStrategy::InitCombatTriggers(triggers);
}

void TankFeralDruidBoostPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidBoostStrategy::InitNonCombatTriggers(triggers);
    DruidBoostPveStrategy::InitNonCombatTriggers(triggers);
}

void TankFeralDruidBoostPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidBoostStrategy::InitCombatTriggers(triggers);
    DruidBoostPvpStrategy::InitCombatTriggers(triggers);
}

void TankFeralDruidBoostPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidBoostStrategy::InitNonCombatTriggers(triggers);
    DruidBoostPvpStrategy::InitNonCombatTriggers(triggers);
}

void TankFeralDruidBoostRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidBoostStrategy::InitCombatTriggers(triggers);
    DruidBoostRaidStrategy::InitCombatTriggers(triggers);
}

void TankFeralDruidBoostRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidBoostStrategy::InitNonCombatTriggers(triggers);
    DruidBoostRaidStrategy::InitNonCombatTriggers(triggers);
}

void TankFeralDruidCcStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DruidCcStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "bash",
        NextAction::array(0, new NextAction("bash", ACTION_INTERRUPT), NULL)));

    triggers.push_back(new TriggerNode(
        "bash on enemy healer",
        NextAction::array(0, new NextAction("bash on enemy healer", ACTION_INTERRUPT), NULL)));
}

void TankFeralDruidCcStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DruidCcStrategy::InitNonCombatTriggers(triggers);
}

void TankFeralDruidCcPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidCcStrategy::InitCombatTriggers(triggers);
    DruidCcPveStrategy::InitCombatTriggers(triggers);
}

void TankFeralDruidCcPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidCcStrategy::InitNonCombatTriggers(triggers);
    DruidCcPveStrategy::InitNonCombatTriggers(triggers);
}

void TankFeralDruidCcPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidCcStrategy::InitCombatTriggers(triggers);
    DruidCcPvpStrategy::InitCombatTriggers(triggers);
}

void TankFeralDruidCcPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidCcStrategy::InitNonCombatTriggers(triggers);
    DruidCcPvpStrategy::InitNonCombatTriggers(triggers);
}

void TankFeralDruidCcRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidCcStrategy::InitCombatTriggers(triggers);
    DruidCcRaidStrategy::InitCombatTriggers(triggers);
}

void TankFeralDruidCcRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidCcStrategy::InitNonCombatTriggers(triggers);
    DruidCcRaidStrategy::InitNonCombatTriggers(triggers);
}

void TankFeralDruidCureStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DruidCureStrategy::InitCombatTriggers(triggers);
}

void TankFeralDruidCureStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DruidCureStrategy::InitNonCombatTriggers(triggers);
}

void TankFeralDruidCurePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidCureStrategy::InitCombatTriggers(triggers);
    DruidCurePveStrategy::InitCombatTriggers(triggers);
}

void TankFeralDruidCurePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidCureStrategy::InitNonCombatTriggers(triggers);
    DruidCurePveStrategy::InitNonCombatTriggers(triggers);
}

void TankFeralDruidCurePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidCureStrategy::InitCombatTriggers(triggers);
    DruidCurePvpStrategy::InitCombatTriggers(triggers);
}

void TankFeralDruidCurePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidCureStrategy::InitNonCombatTriggers(triggers);
    DruidCurePvpStrategy::InitNonCombatTriggers(triggers);
}

void TankFeralDruidCureRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidCureStrategy::InitCombatTriggers(triggers);
    DruidCureRaidStrategy::InitCombatTriggers(triggers);
}

void TankFeralDruidCureRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    TankFeralDruidCureStrategy::InitNonCombatTriggers(triggers);
    DruidCureRaidStrategy::InitNonCombatTriggers(triggers);
}

#endif