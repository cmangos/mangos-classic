
#include "playerbot/playerbot.h"
#include "DruidMultipliers.h"
#include "DpsFeralDruidStrategy.h"

using namespace ai;

class DpsFeralDruidStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    DpsFeralDruidStrategyActionNodeFactory()
    {
        creators["survival instincts"] = &survival_instincts;
        creators["omen of clarity"] = &omen_of_clarity;
        creators["feral charge - cat"] = &feral_charge_cat;
        creators["shred"] = &shred;
        creators["mangle (cat)"] = &mangle_cat;
        creators["powershift"] = &powershift;
    }

private:
    ACTION_NODE_A(survival_instincts, "survival instincts", "barskin");

    ACTION_NODE_P(omen_of_clarity, "omen of clarity", "caster form");

    ACTION_NODE_A(feral_charge_cat, "feral charge - cat", "reach melee");

    ACTION_NODE_A(shred, "shred", "mangle (cat)");

    ACTION_NODE_A(mangle_cat, "mangle (cat)", "claw");

    ACTION_NODE_A(pounce, "pounce", "ravage");

    ACTION_NODE_A(ravage, "ravage", "shred");

    ACTION_NODE_C(powershift, "caster form", "cat form");
};

DpsFeralDruidStrategy::DpsFeralDruidStrategy(PlayerbotAI* ai) : DruidStrategy(ai)
{
    actionNodeFactories.Add(new DpsFeralDruidStrategyActionNodeFactory());
}

class DpsFeralDruidStealthStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    DpsFeralDruidStealthStrategyActionNodeFactory()
    {
        creators["pounce"] = &pounce;
        creators["ravage"] = &ravage;
        creators["shred"] = &shred;
        creators["mangle (cat)"] = &mangle_cat;
        creators["feral charge - cat"] = &feral_charge_cat;
    }

private:
    ACTION_NODE_A(feral_charge_cat, "feral charge - cat", "reach melee");

    ACTION_NODE_A(shred, "shred", "mangle (cat)");

    ACTION_NODE_A(mangle_cat, "mangle (cat)", "claw");

    ACTION_NODE_A(pounce, "pounce", "ravage");

    ACTION_NODE_A(ravage, "ravage", "shred");
};

DpsFeralDruidStealthStrategy::DpsFeralDruidStealthStrategy(PlayerbotAI* ai) : StealthStrategy(ai)
{
    actionNodeFactories.Add(new DpsFeralDruidStealthStrategyActionNodeFactory());
}

#ifdef MANGOSBOT_ZERO // Vanilla

NextAction** DpsFeralDruidStrategy::GetDefaultCombatActions()
{
    return NextAction::array(0, new NextAction("melee", ACTION_IDLE), NULL);
}

void DpsFeralDruidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
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
        "faerie fire (feral)",
        NextAction::array(0, new NextAction("faerie fire (feral)", ACTION_NORMAL + 5), NULL)));

    triggers.push_back(new TriggerNode(
        "ferocious bite",
        NextAction::array(0, new NextAction("ferocious bite", ACTION_NORMAL + 3), NULL)));

    triggers.push_back(new TriggerNode(
        "behind target",
        NextAction::array(0, new NextAction("shred", ACTION_NORMAL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "not behind target",
        NextAction::array(0, new NextAction("mangle (cat)", ACTION_NORMAL), NULL)));
}

void DpsFeralDruidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DruidStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "low health",
        NextAction::array(0, new NextAction("regrowth", ACTION_MEDIUM_HEAL), NULL)));
}

void DpsFeralDruidStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    DruidStrategy::InitReactionTriggers(triggers);
}

void DpsFeralDruidStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    DruidStrategy::InitDeadTriggers(triggers);
}

void DpsFeralDruidPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidStrategy::InitCombatTriggers(triggers);
    DruidPveStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "rip",
        NextAction::array(0, new NextAction("rip", ACTION_NORMAL + 4), NULL)));

    triggers.push_back(new TriggerNode(
        "rake",
        NextAction::array(0, new NextAction("rake", ACTION_NORMAL + 2), NULL)));
}

void DpsFeralDruidPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidStrategy::InitNonCombatTriggers(triggers);
    DruidPveStrategy::InitNonCombatTriggers(triggers);
}

void DpsFeralDruidPveStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidStrategy::InitReactionTriggers(triggers);
    DruidPveStrategy::InitReactionTriggers(triggers);
}

void DpsFeralDruidPveStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidStrategy::InitDeadTriggers(triggers);
    DruidPveStrategy::InitDeadTriggers(triggers);
}

void DpsFeralDruidPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidStrategy::InitCombatTriggers(triggers);
    DruidPvpStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "enemy out of melee",
        NextAction::array(0, new NextAction("dash", ACTION_HIGH), NULL)));

    triggers.push_back(new TriggerNode(
        "player has flag",
        NextAction::array(0, new NextAction("dash", ACTION_HIGH), NULL)));

    triggers.push_back(new TriggerNode(
        "enemy flagcarrier near",
        NextAction::array(0, new NextAction("dash", ACTION_HIGH), NULL)));

    triggers.push_back(new TriggerNode(
        "rip",
        NextAction::array(0, new NextAction("rip", ACTION_NORMAL + 4), NULL)));

    triggers.push_back(new TriggerNode(
        "rake",
        NextAction::array(0, new NextAction("rake", ACTION_NORMAL + 2), NULL)));
}

void DpsFeralDruidPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidStrategy::InitNonCombatTriggers(triggers);
    DruidPvpStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "player has flag",
        NextAction::array(0, new NextAction("dash", ACTION_HIGH), NULL)));

    triggers.push_back(new TriggerNode(
        "enemy flagcarrier near",
        NextAction::array(0, new NextAction("dash", ACTION_HIGH), NULL)));
}

void DpsFeralDruidPvpStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidStrategy::InitReactionTriggers(triggers);
    DruidPvpStrategy::InitReactionTriggers(triggers);
}

void DpsFeralDruidPvpStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidStrategy::InitDeadTriggers(triggers);
    DruidPvpStrategy::InitDeadTriggers(triggers);
}

void DpsFeralDruidRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidStrategy::InitCombatTriggers(triggers);
    DruidRaidStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "medium threat",
        NextAction::array(0, new NextAction("cower", ACTION_HIGH), NULL)));
}

void DpsFeralDruidRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidStrategy::InitNonCombatTriggers(triggers);
    DruidRaidStrategy::InitNonCombatTriggers(triggers);
}

void DpsFeralDruidRaidStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidStrategy::InitReactionTriggers(triggers);
    DruidRaidStrategy::InitReactionTriggers(triggers);
}

void DpsFeralDruidRaidStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidStrategy::InitDeadTriggers(triggers);
    DruidRaidStrategy::InitDeadTriggers(triggers);
}

void DpsFeralDruidAoeStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DruidAoeStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "melee medium aoe",
        NextAction::array(0, new NextAction("swipe (cat)", ACTION_HIGH), NULL)));
}

void DpsFeralDruidAoeStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DruidAoeStrategy::InitNonCombatTriggers(triggers);
}

void DpsFeralDruidAoePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidAoeStrategy::InitCombatTriggers(triggers);
    DruidAoePveStrategy::InitCombatTriggers(triggers);
}

void DpsFeralDruidAoePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidAoeStrategy::InitNonCombatTriggers(triggers);
    DruidAoePveStrategy::InitNonCombatTriggers(triggers);
}

void DpsFeralDruidAoePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidAoeStrategy::InitCombatTriggers(triggers);
    DruidAoePvpStrategy::InitCombatTriggers(triggers);
}

void DpsFeralDruidAoePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidAoeStrategy::InitNonCombatTriggers(triggers);
    DruidAoePvpStrategy::InitNonCombatTriggers(triggers);
}

void DpsFeralDruidAoeRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidAoeStrategy::InitCombatTriggers(triggers);
    DruidAoeRaidStrategy::InitCombatTriggers(triggers);
}

void DpsFeralDruidAoeRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidAoeStrategy::InitNonCombatTriggers(triggers);
    DruidAoeRaidStrategy::InitNonCombatTriggers(triggers);
}

void DpsFeralDruidBuffStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DruidBuffStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "cat form",
        NextAction::array(0, new NextAction("cat form", ACTION_MOVE), NULL)));

    triggers.push_back(new TriggerNode(
        "tiger's fury",
        NextAction::array(0, new NextAction("tiger's fury", ACTION_HIGH), NULL)));
}

void DpsFeralDruidBuffStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DruidBuffStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "omen of clarity",
        NextAction::array(0, new NextAction("omen of clarity", ACTION_NORMAL), NULL)));
}

void DpsFeralDruidBuffPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidBuffStrategy::InitCombatTriggers(triggers);
    DruidBuffPveStrategy::InitCombatTriggers(triggers);
}

void DpsFeralDruidBuffPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidBuffStrategy::InitNonCombatTriggers(triggers);
    DruidBuffPveStrategy::InitNonCombatTriggers(triggers);
}

void DpsFeralDruidBuffPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidBuffStrategy::InitCombatTriggers(triggers);
    DruidBuffPvpStrategy::InitCombatTriggers(triggers);
}

void DpsFeralDruidBuffPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidBuffStrategy::InitNonCombatTriggers(triggers);
    DruidBuffPvpStrategy::InitNonCombatTriggers(triggers);
}

void DpsFeralDruidBuffRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidBuffStrategy::InitCombatTriggers(triggers);
    DruidBuffRaidStrategy::InitCombatTriggers(triggers);
}

void DpsFeralDruidBuffRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidBuffStrategy::InitNonCombatTriggers(triggers);
    DruidBuffRaidStrategy::InitNonCombatTriggers(triggers);
}

void DpsFeralDruidBoostStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DruidBoostStrategy::InitCombatTriggers(triggers);
}

void DpsFeralDruidBoostStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DruidBoostStrategy::InitNonCombatTriggers(triggers);
}

void DpsFeralDruidBoostPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidBoostStrategy::InitCombatTriggers(triggers);
    DruidBoostPveStrategy::InitCombatTriggers(triggers);
}

void DpsFeralDruidBoostPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidBoostStrategy::InitNonCombatTriggers(triggers);
    DruidBoostPveStrategy::InitNonCombatTriggers(triggers);
}

void DpsFeralDruidBoostPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidBoostStrategy::InitCombatTriggers(triggers);
    DruidBoostPvpStrategy::InitCombatTriggers(triggers);
}

void DpsFeralDruidBoostPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidBoostStrategy::InitNonCombatTriggers(triggers);
    DruidBoostPvpStrategy::InitNonCombatTriggers(triggers);
}

void DpsFeralDruidBoostRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidBoostStrategy::InitCombatTriggers(triggers);
    DruidBoostRaidStrategy::InitCombatTriggers(triggers);
}

void DpsFeralDruidBoostRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidBoostStrategy::InitNonCombatTriggers(triggers);
    DruidBoostRaidStrategy::InitNonCombatTriggers(triggers);
}

void DpsFeralDruidCcStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DruidCcStrategy::InitCombatTriggers(triggers);
}

void DpsFeralDruidCcStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DruidCcStrategy::InitNonCombatTriggers(triggers);
}

void DpsFeralDruidCcPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidCcStrategy::InitCombatTriggers(triggers);
    DruidCcPveStrategy::InitCombatTriggers(triggers);
}

void DpsFeralDruidCcPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidCcStrategy::InitNonCombatTriggers(triggers);
    DruidCcPveStrategy::InitNonCombatTriggers(triggers);
}

void DpsFeralDruidCcPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidCcStrategy::InitCombatTriggers(triggers);
    DruidCcPvpStrategy::InitCombatTriggers(triggers);
}

void DpsFeralDruidCcPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidCcStrategy::InitNonCombatTriggers(triggers);
    DruidCcPvpStrategy::InitNonCombatTriggers(triggers);
}

void DpsFeralDruidCcRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidCcStrategy::InitCombatTriggers(triggers);
    DruidCcRaidStrategy::InitCombatTriggers(triggers);
}

void DpsFeralDruidCcRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidCcStrategy::InitNonCombatTriggers(triggers);
    DruidCcRaidStrategy::InitNonCombatTriggers(triggers);
}

void DpsFeralDruidCureStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DruidCureStrategy::InitCombatTriggers(triggers);
}

void DpsFeralDruidCureStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DruidCureStrategy::InitNonCombatTriggers(triggers);
}

void DpsFeralDruidCurePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidCureStrategy::InitCombatTriggers(triggers);
    DruidCurePveStrategy::InitCombatTriggers(triggers);
}

void DpsFeralDruidCurePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidCureStrategy::InitNonCombatTriggers(triggers);
    DruidCurePveStrategy::InitNonCombatTriggers(triggers);
}

void DpsFeralDruidCurePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidCureStrategy::InitCombatTriggers(triggers);
    DruidCurePvpStrategy::InitCombatTriggers(triggers);
}

void DpsFeralDruidCurePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidCureStrategy::InitNonCombatTriggers(triggers);
    DruidCurePvpStrategy::InitNonCombatTriggers(triggers);
}

void DpsFeralDruidCureRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidCureStrategy::InitCombatTriggers(triggers);
    DruidCureRaidStrategy::InitCombatTriggers(triggers);
}

void DpsFeralDruidCureRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidCureStrategy::InitNonCombatTriggers(triggers);
    DruidCureRaidStrategy::InitNonCombatTriggers(triggers);
}

void DpsFeralDruidStealthStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    StealthStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "stealth",
        NextAction::array(0, new NextAction("stealth", ACTION_EMERGENCY), NULL)));

    triggers.push_back(new TriggerNode(
        "no stealth",
        NextAction::array(0, new NextAction("check stealth", ACTION_EMERGENCY), NULL)));

    triggers.push_back(new TriggerNode(
        "behind target",
        NextAction::array(0, new NextAction("pounce", ACTION_NORMAL + 3), NULL)));

    triggers.push_back(new TriggerNode(
        "not behind target",
        NextAction::array(0, new NextAction("ravage", ACTION_NORMAL + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "kick",
        NextAction::array(0, new NextAction("pounce", ACTION_NORMAL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "unstealth",
        NextAction::array(0, new NextAction("unstealth", ACTION_NORMAL), NULL)));
}

void DpsFeralDruidStealthStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    StealthStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "no stealth",
        NextAction::array(0, new NextAction("check stealth", ACTION_HIGH), NULL)));

    triggers.push_back(new TriggerNode(
        "unstealth",
        NextAction::array(0, new NextAction("unstealth", ACTION_NORMAL), NULL)));
}

void DpsFeralDruidStealthPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidStealthStrategy::InitCombatTriggers(triggers);
    StealthPveStrategy::InitCombatTriggers(triggers);
}

void DpsFeralDruidStealthPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidStealthStrategy::InitNonCombatTriggers(triggers);
    StealthPveStrategy::InitNonCombatTriggers(triggers);
}

void DpsFeralDruidStealthPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidStealthStrategy::InitCombatTriggers(triggers);
    StealthPvpStrategy::InitCombatTriggers(triggers);
}

void DpsFeralDruidStealthPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidStealthStrategy::InitNonCombatTriggers(triggers);
    StealthPvpStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "enemy flagcarrier near",
        NextAction::array(0, new NextAction("dash", ACTION_HIGH), 
                             new NextAction("unstealth", ACTION_HIGH), NULL)));
}

void DpsFeralDruidStealthRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidStealthStrategy::InitCombatTriggers(triggers);
    StealthRaidStrategy::InitCombatTriggers(triggers);
}

void DpsFeralDruidStealthRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidStealthStrategy::InitNonCombatTriggers(triggers);
    StealthRaidStrategy::InitNonCombatTriggers(triggers);
}

#endif

#ifdef MANGOSBOT_ONE // TBC

NextAction** DpsFeralDruidStrategy::GetDefaultCombatActions()
{
    return NextAction::array(0, new NextAction("melee", ACTION_IDLE), NULL);
}

void DpsFeralDruidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
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
        "faerie fire (feral)",
        NextAction::array(0, new NextAction("faerie fire (feral)", ACTION_HIGH + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "rip",
        NextAction::array(0, new NextAction("rip", ACTION_HIGH + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "ferocious bite",
        NextAction::array(0, new NextAction("ferocious bite", ACTION_HIGH), NULL)));

    triggers.push_back(new TriggerNode(
        "rake",
        NextAction::array(0, new NextAction("rake", ACTION_NORMAL + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "behind target",
        NextAction::array(0, new NextAction("shred", ACTION_NORMAL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "not behind target",
        NextAction::array(0, new NextAction("mangle (cat)", ACTION_NORMAL), NULL)));
}

void DpsFeralDruidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DruidStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "low health",
        NextAction::array(0, new NextAction("regrowth", ACTION_MEDIUM_HEAL), NULL)));
}

void DpsFeralDruidStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    DruidStrategy::InitReactionTriggers(triggers);
}

void DpsFeralDruidStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    DruidStrategy::InitDeadTriggers(triggers);
}

void DpsFeralDruidPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidStrategy::InitCombatTriggers(triggers);
    DruidPveStrategy::InitCombatTriggers(triggers);
}

void DpsFeralDruidPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidStrategy::InitNonCombatTriggers(triggers);
    DruidPveStrategy::InitNonCombatTriggers(triggers);
}

void DpsFeralDruidPveStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidStrategy::InitReactionTriggers(triggers);
    DruidPveStrategy::InitReactionTriggers(triggers);
}

void DpsFeralDruidPveStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidStrategy::InitDeadTriggers(triggers);
    DruidPveStrategy::InitDeadTriggers(triggers);
}

void DpsFeralDruidPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidStrategy::InitCombatTriggers(triggers);
    DruidPvpStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "enemy out of melee",
        NextAction::array(0, new NextAction("dash", ACTION_HIGH), NULL)));

    triggers.push_back(new TriggerNode(
        "player has flag",
        NextAction::array(0, new NextAction("dash", ACTION_HIGH), NULL)));

    triggers.push_back(new TriggerNode(
        "enemy flagcarrier near",
        NextAction::array(0, new NextAction("dash", ACTION_HIGH), NULL)));
}

void DpsFeralDruidPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidStrategy::InitNonCombatTriggers(triggers);
    DruidPvpStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "player has flag",
        NextAction::array(0, new NextAction("dash", ACTION_HIGH), NULL)));

    triggers.push_back(new TriggerNode(
        "enemy flagcarrier near",
        NextAction::array(0, new NextAction("dash", ACTION_HIGH), NULL)));
}

void DpsFeralDruidPvpStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidStrategy::InitReactionTriggers(triggers);
    DruidPvpStrategy::InitReactionTriggers(triggers);
}

void DpsFeralDruidPvpStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidStrategy::InitDeadTriggers(triggers);
    DruidPvpStrategy::InitDeadTriggers(triggers);
}

void DpsFeralDruidRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidStrategy::InitCombatTriggers(triggers);
    DruidRaidStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "medium threat",
        NextAction::array(0, new NextAction("cower", ACTION_HIGH), NULL)));
}

void DpsFeralDruidRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidStrategy::InitNonCombatTriggers(triggers);
    DruidRaidStrategy::InitNonCombatTriggers(triggers);
}

void DpsFeralDruidRaidStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidStrategy::InitReactionTriggers(triggers);
    DruidRaidStrategy::InitReactionTriggers(triggers);
}

void DpsFeralDruidRaidStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidStrategy::InitDeadTriggers(triggers);
    DruidRaidStrategy::InitDeadTriggers(triggers);
}

void DpsFeralDruidAoeStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DruidAoeStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "melee medium aoe",
        NextAction::array(0, new NextAction("swipe (cat)", ACTION_HIGH), NULL)));
}

void DpsFeralDruidAoeStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DruidAoeStrategy::InitNonCombatTriggers(triggers);
}

void DpsFeralDruidAoePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidAoeStrategy::InitCombatTriggers(triggers);
    DruidAoePveStrategy::InitCombatTriggers(triggers);
}

void DpsFeralDruidAoePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidAoeStrategy::InitNonCombatTriggers(triggers);
    DruidAoePveStrategy::InitNonCombatTriggers(triggers);
}

void DpsFeralDruidAoePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidAoeStrategy::InitCombatTriggers(triggers);
    DruidAoePvpStrategy::InitCombatTriggers(triggers);
}

void DpsFeralDruidAoePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidAoeStrategy::InitNonCombatTriggers(triggers);
    DruidAoePvpStrategy::InitNonCombatTriggers(triggers);
}

void DpsFeralDruidAoeRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidAoeStrategy::InitCombatTriggers(triggers);
    DruidAoeRaidStrategy::InitCombatTriggers(triggers);
}

void DpsFeralDruidAoeRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidAoeStrategy::InitNonCombatTriggers(triggers);
    DruidAoeRaidStrategy::InitNonCombatTriggers(triggers);
}

void DpsFeralDruidBuffStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DruidBuffStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "cat form",
        NextAction::array(0, new NextAction("cat form", ACTION_MOVE), NULL)));

    triggers.push_back(new TriggerNode(
        "tiger's fury",
        NextAction::array(0, new NextAction("tiger's fury", ACTION_HIGH), NULL)));
}

void DpsFeralDruidBuffStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DruidBuffStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "omen of clarity",
        NextAction::array(0, new NextAction("omen of clarity", ACTION_NORMAL), NULL)));
}

void DpsFeralDruidBuffPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidBuffStrategy::InitCombatTriggers(triggers);
    DruidBuffPveStrategy::InitCombatTriggers(triggers);
}

void DpsFeralDruidBuffPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidBuffStrategy::InitNonCombatTriggers(triggers);
    DruidBuffPveStrategy::InitNonCombatTriggers(triggers);
}

void DpsFeralDruidBuffPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidBuffStrategy::InitCombatTriggers(triggers);
    DruidBuffPvpStrategy::InitCombatTriggers(triggers);
}

void DpsFeralDruidBuffPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidBuffStrategy::InitNonCombatTriggers(triggers);
    DruidBuffPvpStrategy::InitNonCombatTriggers(triggers);
}

void DpsFeralDruidBuffRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidBuffStrategy::InitCombatTriggers(triggers);
    DruidBuffRaidStrategy::InitCombatTriggers(triggers);
}

void DpsFeralDruidBuffRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidBuffStrategy::InitNonCombatTriggers(triggers);
    DruidBuffRaidStrategy::InitNonCombatTriggers(triggers);
}

void DpsFeralDruidBoostStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DruidBoostStrategy::InitCombatTriggers(triggers);
}

void DpsFeralDruidBoostStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DruidBoostStrategy::InitNonCombatTriggers(triggers);
}

void DpsFeralDruidBoostPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidBoostStrategy::InitCombatTriggers(triggers);
    DruidBoostPveStrategy::InitCombatTriggers(triggers);
}

void DpsFeralDruidBoostPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidBoostStrategy::InitNonCombatTriggers(triggers);
    DruidBoostPveStrategy::InitNonCombatTriggers(triggers);
}

void DpsFeralDruidBoostPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidBoostStrategy::InitCombatTriggers(triggers);
    DruidBoostPvpStrategy::InitCombatTriggers(triggers);
}

void DpsFeralDruidBoostPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidBoostStrategy::InitNonCombatTriggers(triggers);
    DruidBoostPvpStrategy::InitNonCombatTriggers(triggers);
}

void DpsFeralDruidBoostRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidBoostStrategy::InitCombatTriggers(triggers);
    DruidBoostRaidStrategy::InitCombatTriggers(triggers);
}

void DpsFeralDruidBoostRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidBoostStrategy::InitNonCombatTriggers(triggers);
    DruidBoostRaidStrategy::InitNonCombatTriggers(triggers);
}

void DpsFeralDruidCcStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DruidCcStrategy::InitCombatTriggers(triggers);
}

void DpsFeralDruidCcStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DruidCcStrategy::InitNonCombatTriggers(triggers);
}

void DpsFeralDruidCcPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidCcStrategy::InitCombatTriggers(triggers);
    DruidCcPveStrategy::InitCombatTriggers(triggers);
}

void DpsFeralDruidCcPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidCcStrategy::InitNonCombatTriggers(triggers);
    DruidCcPveStrategy::InitNonCombatTriggers(triggers);
}

void DpsFeralDruidCcPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidCcStrategy::InitCombatTriggers(triggers);
    DruidCcPvpStrategy::InitCombatTriggers(triggers);
}

void DpsFeralDruidCcPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidCcStrategy::InitNonCombatTriggers(triggers);
    DruidCcPvpStrategy::InitNonCombatTriggers(triggers);
}

void DpsFeralDruidCcRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidCcStrategy::InitCombatTriggers(triggers);
    DruidCcRaidStrategy::InitCombatTriggers(triggers);
}

void DpsFeralDruidCcRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidCcStrategy::InitNonCombatTriggers(triggers);
    DruidCcRaidStrategy::InitNonCombatTriggers(triggers);
}

void DpsFeralDruidCureStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DruidCureStrategy::InitCombatTriggers(triggers);
}

void DpsFeralDruidCureStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DruidCureStrategy::InitNonCombatTriggers(triggers);
}

void DpsFeralDruidCurePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidCureStrategy::InitCombatTriggers(triggers);
    DruidCurePveStrategy::InitCombatTriggers(triggers);
}

void DpsFeralDruidCurePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidCureStrategy::InitNonCombatTriggers(triggers);
    DruidCurePveStrategy::InitNonCombatTriggers(triggers);
}

void DpsFeralDruidCurePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidCureStrategy::InitCombatTriggers(triggers);
    DruidCurePvpStrategy::InitCombatTriggers(triggers);
}

void DpsFeralDruidCurePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidCureStrategy::InitNonCombatTriggers(triggers);
    DruidCurePvpStrategy::InitNonCombatTriggers(triggers);
}

void DpsFeralDruidCureRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidCureStrategy::InitCombatTriggers(triggers);
    DruidCureRaidStrategy::InitCombatTriggers(triggers);
}

void DpsFeralDruidCureRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidCureStrategy::InitNonCombatTriggers(triggers);
    DruidCureRaidStrategy::InitNonCombatTriggers(triggers);
}

void DpsFeralDruidStealthStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    StealthStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "stealth",
        NextAction::array(0, new NextAction("stealth", ACTION_EMERGENCY), NULL)));

    triggers.push_back(new TriggerNode(
        "no stealth",
        NextAction::array(0, new NextAction("check stealth", ACTION_EMERGENCY), NULL)));

    triggers.push_back(new TriggerNode(
        "behind target",
        NextAction::array(0, new NextAction("pounce", ACTION_NORMAL + 3), NULL)));

    triggers.push_back(new TriggerNode(
        "not behind target",
        NextAction::array(0, new NextAction("ravage", ACTION_NORMAL + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "kick",
        NextAction::array(0, new NextAction("pounce", ACTION_NORMAL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "unstealth",
        NextAction::array(0, new NextAction("unstealth", ACTION_NORMAL), NULL)));
}

void DpsFeralDruidStealthStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    StealthStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "no stealth",
        NextAction::array(0, new NextAction("check stealth", ACTION_HIGH), NULL)));

    triggers.push_back(new TriggerNode(
        "unstealth",
        NextAction::array(0, new NextAction("unstealth", ACTION_NORMAL), NULL)));
}

void DpsFeralDruidStealthPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidStealthStrategy::InitCombatTriggers(triggers);
    StealthPveStrategy::InitCombatTriggers(triggers);
}

void DpsFeralDruidStealthPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidStealthStrategy::InitNonCombatTriggers(triggers);
    StealthPveStrategy::InitNonCombatTriggers(triggers);
}

void DpsFeralDruidStealthPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidStealthStrategy::InitCombatTriggers(triggers);
    StealthPvpStrategy::InitCombatTriggers(triggers);
}

void DpsFeralDruidStealthPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidStealthStrategy::InitNonCombatTriggers(triggers);
    StealthPvpStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "enemy flagcarrier near",
        NextAction::array(0, new NextAction("dash", ACTION_HIGH), 
                             new NextAction("unstealth", ACTION_HIGH), NULL)));
}

void DpsFeralDruidStealthRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidStealthStrategy::InitCombatTriggers(triggers);
    StealthRaidStrategy::InitCombatTriggers(triggers);
}

void DpsFeralDruidStealthRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidStealthStrategy::InitNonCombatTriggers(triggers);
    StealthRaidStrategy::InitNonCombatTriggers(triggers);
}

#endif

#ifdef MANGOSBOT_TWO // WOTLK

NextAction** DpsFeralDruidStrategy::GetDefaultCombatActions()
{
    return NextAction::array(0, new NextAction("melee", ACTION_IDLE), NULL);
}

void DpsFeralDruidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
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
        "enemy out of melee",
        NextAction::array(0, new NextAction("feral charge - cat", ACTION_MOVE), NULL)));

    triggers.push_back(new TriggerNode(
        "faerie fire (feral)",
        NextAction::array(0, new NextAction("faerie fire (feral)", ACTION_HIGH + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "rip",
        NextAction::array(0, new NextAction("rip", ACTION_HIGH + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "ferocious bite",
        NextAction::array(0, new NextAction("ferocious bite", ACTION_HIGH), NULL)));

    triggers.push_back(new TriggerNode(
        "rake",
        NextAction::array(0, new NextAction("rake", ACTION_NORMAL + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "behind target",
        NextAction::array(0, new NextAction("shred", ACTION_NORMAL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "not behind target",
        NextAction::array(0, new NextAction("mangle (cat)", ACTION_NORMAL), NULL)));
}

void DpsFeralDruidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DruidStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "low health",
        NextAction::array(0, new NextAction("regrowth", ACTION_MEDIUM_HEAL), NULL)));
}

void DpsFeralDruidStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    DruidStrategy::InitReactionTriggers(triggers);
}

void DpsFeralDruidStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    DruidStrategy::InitDeadTriggers(triggers);
}

void DpsFeralDruidPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidStrategy::InitCombatTriggers(triggers);
    DruidPveStrategy::InitCombatTriggers(triggers);
}

void DpsFeralDruidPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidStrategy::InitNonCombatTriggers(triggers);
    DruidPveStrategy::InitNonCombatTriggers(triggers);
}

void DpsFeralDruidPveStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidStrategy::InitReactionTriggers(triggers);
    DruidPveStrategy::InitReactionTriggers(triggers);
}

void DpsFeralDruidPveStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidStrategy::InitDeadTriggers(triggers);
    DruidPveStrategy::InitDeadTriggers(triggers);
}

void DpsFeralDruidPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidStrategy::InitCombatTriggers(triggers);
    DruidPvpStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "enemy out of melee",
        NextAction::array(0, new NextAction("dash", ACTION_HIGH), NULL)));

    triggers.push_back(new TriggerNode(
        "player has flag",
        NextAction::array(0, new NextAction("dash", ACTION_HIGH), NULL)));

    triggers.push_back(new TriggerNode(
        "enemy flagcarrier near",
        NextAction::array(0, new NextAction("dash", ACTION_HIGH), NULL)));
}

void DpsFeralDruidPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidStrategy::InitNonCombatTriggers(triggers);
    DruidPvpStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "player has flag",
        NextAction::array(0, new NextAction("dash", ACTION_HIGH), NULL)));

    triggers.push_back(new TriggerNode(
        "enemy flagcarrier near",
        NextAction::array(0, new NextAction("dash", ACTION_HIGH), NULL)));
}

void DpsFeralDruidPvpStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidStrategy::InitReactionTriggers(triggers);
    DruidPvpStrategy::InitReactionTriggers(triggers);
}

void DpsFeralDruidPvpStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidStrategy::InitDeadTriggers(triggers);
    DruidPvpStrategy::InitDeadTriggers(triggers);
}

void DpsFeralDruidRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidStrategy::InitCombatTriggers(triggers);
    DruidRaidStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "medium threat",
        NextAction::array(0, new NextAction("cower", ACTION_HIGH), NULL)));
}

void DpsFeralDruidRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidStrategy::InitNonCombatTriggers(triggers);
    DruidRaidStrategy::InitNonCombatTriggers(triggers);
}

void DpsFeralDruidRaidStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidStrategy::InitReactionTriggers(triggers);
    DruidRaidStrategy::InitReactionTriggers(triggers);
}

void DpsFeralDruidRaidStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidStrategy::InitDeadTriggers(triggers);
    DruidRaidStrategy::InitDeadTriggers(triggers);
}

void DpsFeralDruidAoeStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DruidAoeStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "melee medium aoe",
        NextAction::array(0, new NextAction("swipe (cat)", ACTION_HIGH), NULL)));
}

void DpsFeralDruidAoeStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DruidAoeStrategy::InitNonCombatTriggers(triggers);
}

void DpsFeralDruidAoePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidAoeStrategy::InitCombatTriggers(triggers);
    DruidAoePveStrategy::InitCombatTriggers(triggers);
}

void DpsFeralDruidAoePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidAoeStrategy::InitNonCombatTriggers(triggers);
    DruidAoePveStrategy::InitNonCombatTriggers(triggers);
}

void DpsFeralDruidAoePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidAoeStrategy::InitCombatTriggers(triggers);
    DruidAoePvpStrategy::InitCombatTriggers(triggers);
}

void DpsFeralDruidAoePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidAoeStrategy::InitNonCombatTriggers(triggers);
    DruidAoePvpStrategy::InitNonCombatTriggers(triggers);
}

void DpsFeralDruidAoeRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidAoeStrategy::InitCombatTriggers(triggers);
    DruidAoeRaidStrategy::InitCombatTriggers(triggers);
}

void DpsFeralDruidAoeRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidAoeStrategy::InitNonCombatTriggers(triggers);
    DruidAoeRaidStrategy::InitNonCombatTriggers(triggers);
}

void DpsFeralDruidBuffStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DruidBuffStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "cat form",
        NextAction::array(0, new NextAction("cat form", ACTION_MOVE), NULL)));

    triggers.push_back(new TriggerNode(
        "tiger's fury",
        NextAction::array(0, new NextAction("tiger's fury", ACTION_HIGH), NULL)));
}

void DpsFeralDruidBuffStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DruidBuffStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "omen of clarity",
        NextAction::array(0, new NextAction("omen of clarity", ACTION_NORMAL), NULL)));
}

void DpsFeralDruidBuffPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidBuffStrategy::InitCombatTriggers(triggers);
    DruidBuffPveStrategy::InitCombatTriggers(triggers);
}

void DpsFeralDruidBuffPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidBuffStrategy::InitNonCombatTriggers(triggers);
    DruidBuffPveStrategy::InitNonCombatTriggers(triggers);
}

void DpsFeralDruidBuffPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidBuffStrategy::InitCombatTriggers(triggers);
    DruidBuffPvpStrategy::InitCombatTriggers(triggers);
}

void DpsFeralDruidBuffPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidBuffStrategy::InitNonCombatTriggers(triggers);
    DruidBuffPvpStrategy::InitNonCombatTriggers(triggers);
}

void DpsFeralDruidBuffRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidBuffStrategy::InitCombatTriggers(triggers);
    DruidBuffRaidStrategy::InitCombatTriggers(triggers);
}

void DpsFeralDruidBuffRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidBuffStrategy::InitNonCombatTriggers(triggers);
    DruidBuffRaidStrategy::InitNonCombatTriggers(triggers);
}

void DpsFeralDruidBoostStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DruidBoostStrategy::InitCombatTriggers(triggers);
}

void DpsFeralDruidBoostStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DruidBoostStrategy::InitNonCombatTriggers(triggers);
}

void DpsFeralDruidBoostPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidBoostStrategy::InitCombatTriggers(triggers);
    DruidBoostPveStrategy::InitCombatTriggers(triggers);
}

void DpsFeralDruidBoostPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidBoostStrategy::InitNonCombatTriggers(triggers);
    DruidBoostPveStrategy::InitNonCombatTriggers(triggers);
}

void DpsFeralDruidBoostPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidBoostStrategy::InitCombatTriggers(triggers);
    DruidBoostPvpStrategy::InitCombatTriggers(triggers);
}

void DpsFeralDruidBoostPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidBoostStrategy::InitNonCombatTriggers(triggers);
    DruidBoostPvpStrategy::InitNonCombatTriggers(triggers);
}

void DpsFeralDruidBoostRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidBoostStrategy::InitCombatTriggers(triggers);
    DruidBoostRaidStrategy::InitCombatTriggers(triggers);
}

void DpsFeralDruidBoostRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidBoostStrategy::InitNonCombatTriggers(triggers);
    DruidBoostRaidStrategy::InitNonCombatTriggers(triggers);
}

void DpsFeralDruidCcStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DruidCcStrategy::InitCombatTriggers(triggers);
}

void DpsFeralDruidCcStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DruidCcStrategy::InitNonCombatTriggers(triggers);
}

void DpsFeralDruidCcPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidCcStrategy::InitCombatTriggers(triggers);
    DruidCcPveStrategy::InitCombatTriggers(triggers);
}

void DpsFeralDruidCcPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidCcStrategy::InitNonCombatTriggers(triggers);
    DruidCcPveStrategy::InitNonCombatTriggers(triggers);
}

void DpsFeralDruidCcPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidCcStrategy::InitCombatTriggers(triggers);
    DruidCcPvpStrategy::InitCombatTriggers(triggers);
}

void DpsFeralDruidCcPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidCcStrategy::InitNonCombatTriggers(triggers);
    DruidCcPvpStrategy::InitNonCombatTriggers(triggers);
}

void DpsFeralDruidCcRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidCcStrategy::InitCombatTriggers(triggers);
    DruidCcRaidStrategy::InitCombatTriggers(triggers);
}

void DpsFeralDruidCcRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidCcStrategy::InitNonCombatTriggers(triggers);
    DruidCcRaidStrategy::InitNonCombatTriggers(triggers);
}

void DpsFeralDruidCureStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DruidCureStrategy::InitCombatTriggers(triggers);
}

void DpsFeralDruidCureStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DruidCureStrategy::InitNonCombatTriggers(triggers);
}

void DpsFeralDruidCurePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidCureStrategy::InitCombatTriggers(triggers);
    DruidCurePveStrategy::InitCombatTriggers(triggers);
}

void DpsFeralDruidCurePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidCureStrategy::InitNonCombatTriggers(triggers);
    DruidCurePveStrategy::InitNonCombatTriggers(triggers);
}

void DpsFeralDruidCurePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidCureStrategy::InitCombatTriggers(triggers);
    DruidCurePvpStrategy::InitCombatTriggers(triggers);
}

void DpsFeralDruidCurePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidCureStrategy::InitNonCombatTriggers(triggers);
    DruidCurePvpStrategy::InitNonCombatTriggers(triggers);
}

void DpsFeralDruidCureRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidCureStrategy::InitCombatTriggers(triggers);
    DruidCureRaidStrategy::InitCombatTriggers(triggers);
}

void DpsFeralDruidCureRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidCureStrategy::InitNonCombatTriggers(triggers);
    DruidCureRaidStrategy::InitNonCombatTriggers(triggers);
}

void DpsFeralDruidStealthStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    StealthStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "stealth",
        NextAction::array(0, new NextAction("stealth", ACTION_EMERGENCY), NULL)));

    triggers.push_back(new TriggerNode(
        "no stealth",
        NextAction::array(0, new NextAction("check stealth", ACTION_EMERGENCY), NULL)));

    triggers.push_back(new TriggerNode(
        "behind target",
        NextAction::array(0, new NextAction("pounce", ACTION_NORMAL + 3), NULL)));

    triggers.push_back(new TriggerNode(
        "not behind target",
        NextAction::array(0, new NextAction("ravage", ACTION_NORMAL + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "kick",
        NextAction::array(0, new NextAction("pounce", ACTION_NORMAL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "unstealth",
        NextAction::array(0, new NextAction("unstealth", ACTION_NORMAL), NULL)));
}

void DpsFeralDruidStealthStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    StealthStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "no stealth",
        NextAction::array(0, new NextAction("check stealth", ACTION_HIGH), NULL)));

    triggers.push_back(new TriggerNode(
        "unstealth",
        NextAction::array(0, new NextAction("unstealth", ACTION_NORMAL), NULL)));
}

void DpsFeralDruidStealthPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidStealthStrategy::InitCombatTriggers(triggers);
    StealthPveStrategy::InitCombatTriggers(triggers);
}

void DpsFeralDruidStealthPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidStealthStrategy::InitNonCombatTriggers(triggers);
    StealthPveStrategy::InitNonCombatTriggers(triggers);
}

void DpsFeralDruidStealthPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidStealthStrategy::InitCombatTriggers(triggers);
    StealthPvpStrategy::InitCombatTriggers(triggers);
}

void DpsFeralDruidStealthPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidStealthStrategy::InitNonCombatTriggers(triggers);
    StealthPvpStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "enemy flagcarrier near",
        NextAction::array(0, new NextAction("dash", ACTION_HIGH), 
                             new NextAction("unstealth", ACTION_HIGH), NULL)));
}

void DpsFeralDruidStealthRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidStealthStrategy::InitCombatTriggers(triggers);
    StealthRaidStrategy::InitCombatTriggers(triggers);
}

void DpsFeralDruidStealthRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DpsFeralDruidStealthStrategy::InitNonCombatTriggers(triggers);
    StealthRaidStrategy::InitNonCombatTriggers(triggers);
}

#endif

class DpsFeralDruidStealthedStrategyMultiplier : public Multiplier
{
public:
    DpsFeralDruidStealthedStrategyMultiplier(PlayerbotAI* ai) : Multiplier(ai, "stealthed") {}

    float GetValue(Action* action) override
    {
        // Only allow stealth actions
        const std::string& actionName = action->getName();
        if ((actionName == "unstealth") ||
            (actionName == "check stealth") ||
            (actionName == "feral charge - cat") ||
            (actionName == "claw") ||
            (actionName == "mangle (cat)") ||
            (actionName == "pounce") ||
            (actionName == "ravage") ||
            (actionName == "shred") ||
            (actionName == "food") ||
            (actionName == "drink") ||
            (actionName == "dps assist") ||
            (actionName == "select new target") ||
            (actionName == "follow") ||
            (actionName == "reach melee") ||
            (actionName == "set behind") ||
            (actionName == "set combat state") ||
            (actionName == "set non combat state") ||
            (actionName == "set dead state") ||
            (actionName == "update pvp strats") ||
            (actionName == "update pve strats") ||
            (actionName == "update raid strats"))
        {
            return 1.0f;
        }

        return 0.0f;
    }
};

void DpsFeralDruidStealthedStrategy::InitCombatMultipliers(std::list<Multiplier*>& multipliers)
{
    multipliers.push_back(new DpsFeralDruidStealthedStrategyMultiplier(ai));
}

void DpsFeralDruidStealthedStrategy::InitNonCombatMultipliers(std::list<Multiplier*>& multipliers)
{
    DpsFeralDruidStealthedStrategy::InitCombatMultipliers(multipliers);
}

void DpsFeralDruidPowershiftStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "powershift",
        NextAction::array(0, new NextAction("powershift", ACTION_MOVE), NULL)));
}
