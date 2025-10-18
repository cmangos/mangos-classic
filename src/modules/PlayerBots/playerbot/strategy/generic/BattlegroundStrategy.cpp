
#include "playerbot/playerbot.h"
#include "BattlegroundStrategy.h"

using namespace ai;

void BGStrategy::InitNonCombatTriggers(std::list<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode(
        "random",
        NextAction::array(0, new NextAction("bg join", relevance), NULL)));

    triggers.push_back(new TriggerNode(
        "bg invite active",
        NextAction::array(0, new NextAction("bg status check", relevance), NULL)));
}

void BattlegroundStrategy::InitNonCombatTriggers(std::list<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode(
        "bg waiting",
        NextAction::array(0, new NextAction("bg move to start", 1.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "player has flag",
        NextAction::array(0, new NextAction("jump::position bg objective", 3.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "bg active",
        NextAction::array(0, new NextAction("check mount state", 2.0f), new NextAction("bg move to objective", 1.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "very often",
        NextAction::array(0, new NextAction("bg check objective", 10.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "bg active",
        NextAction::array(0, new NextAction("bg check flag", ACTION_HIGH), NULL)));

    triggers.push_back(new TriggerNode(
        "bg ended",
        NextAction::array(0, new NextAction("bg leave", ACTION_HIGH), NULL)));

    /*triggers.push_back(new TriggerNode(
        "enemy flagcarrier near",
        NextAction::array(0, new NextAction("attack enemy flag carrier", 80.0f), NULL)));*/

    /*triggers.push_back(new TriggerNode(
        "team flagcarrier near",
        NextAction::array(0, new NextAction("bg protect fc", 40.0f), NULL)));*/

    /*triggers.push_back(new TriggerNode(
        "player has flag",
        NextAction::array(0, new NextAction("bg move to objective", 90.0f), NULL)));*/
}

void WarsongStrategy::InitNonCombatTriggers(std::list<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode(
        "bg active",
        NextAction::array(0, new NextAction("bg check flag", 70.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "often",
        NextAction::array(0, new NextAction("bg use buff", 30.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "low health",
        NextAction::array(0, new NextAction("bg use buff", 30.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "low mana",
        NextAction::array(0, new NextAction("bg use buff", 30.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "enemy flagcarrier near",
        NextAction::array(0, new NextAction("attack enemy flag carrier", 80.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "player has flag",
        NextAction::array(0,
            new NextAction("jump::position bg objective", 80.5f),
            new NextAction("bg move to objective", 80.0f),
            NULL)));

    triggers.push_back(new TriggerNode(
        "player has flag",
        NextAction::array(0, new NextAction("rocket boots", 81.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "very often",
        NextAction::array(0, new NextAction("bg banner", 10.0f), NULL)));
}

void WarsongStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    InitNonCombatTriggers(triggers);
}

void AlteracStrategy::InitNonCombatTriggers(std::list<TriggerNode*> &triggers)
{
}

void AlteracStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "very often",
        NextAction::array(0, new NextAction("bg banner", ACTION_NORMAL), NULL)));
}

void ArathiStrategy::InitNonCombatTriggers(std::list<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode(
        "bg active",
        NextAction::array(0, new NextAction("bg check flag", 70.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "often",
        NextAction::array(0, new NextAction("bg use buff", 30.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "low health",
        NextAction::array(0, new NextAction("bg use buff", 30.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "low mana",
        NextAction::array(0, new NextAction("bg use buff", 30.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "very often",
        NextAction::array(0, new NextAction("bg banner", 10.0f), NULL)));
}

void ArathiStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    InitNonCombatTriggers(triggers);
}

void EyeStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "bg active",
        NextAction::array(0, new NextAction("bg check flag", 70.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "often",
        NextAction::array(0, new NextAction("bg use buff", 30.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "low health",
        NextAction::array(0, new NextAction("bg use buff", 30.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "low mana",
        NextAction::array(0, new NextAction("bg use buff", 30.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "enemy flagcarrier near",
        NextAction::array(0, new NextAction("attack enemy flag carrier", 80.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "player has flag",
        NextAction::array(0, new NextAction("bg move to objective", 80.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "player has flag",
        NextAction::array(0, new NextAction("rocket boots", 81.0f), NULL)));
}

void EyeStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    InitNonCombatTriggers(triggers);
}

void IsleStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "bg active",
        NextAction::array(0, new NextAction("bg check flag", 70.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "timer",
        NextAction::array(0, new NextAction("enter vehicle", 85.0f), NULL)));

    /*triggers.push_back(new TriggerNode(
        "random",
        NextAction::array(0, new NextAction("leave vehicle", 80.0f), NULL)));*/

    triggers.push_back(new TriggerNode(
        "in vehicle",
        NextAction::array(0, new NextAction("hurl boulder", 70.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "in vehicle",
        NextAction::array(0, new NextAction("fire cannon", 70.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "in vehicle",
        NextAction::array(0, new NextAction("napalm", 70.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "enemy is close",
        NextAction::array(0, new NextAction("steam blast", 80.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "in vehicle",
        NextAction::array(0, new NextAction("ram", 70.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "enemy is close",
        NextAction::array(0, new NextAction("ram", 79.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "enemy out of melee",
        NextAction::array(0, new NextAction("steam rush", 81.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "in vehicle",
        NextAction::array(0, new NextAction("incendiary rocket", 70.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "in vehicle",
        NextAction::array(0, new NextAction("rocket blast", 70.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "in vehicle",
        NextAction::array(0, new NextAction("blade salvo", 71.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "in vehicle",
        NextAction::array(0, new NextAction("glaive throw", 70.0f), NULL)));
}

void IsleStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    InitNonCombatTriggers(triggers);
}

void ArenaStrategy::InitNonCombatTriggers(std::list<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode(
        "no possible targets",
        NextAction::array(0, new NextAction("arena tactics", 1.0f), NULL)));
}

void ArenaStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    InitNonCombatTriggers(triggers);
}
