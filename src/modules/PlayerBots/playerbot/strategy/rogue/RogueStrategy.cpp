
#include "playerbot/playerbot.h"
#include "RogueMultipliers.h"
#include "RogueStrategy.h"

using namespace ai;

class RogueStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    RogueStrategyActionNodeFactory()
    {
        creators["sinister strike"] = &sinister_strike;
        creators["kick"] = &kick;
        creators["kidney shot"] = &kidney_shot;
        creators["slice and dice"] = &slice_and_dice;
        creators["eviscerate"] = &eviscerate;
        creators["sap"] = &sap;
    }

private:
    ACTION_NODE_A(sinister_strike, "sinister strike", "melee");

    ACTION_NODE_A(kick, "kick", "kidney shot");

    ACTION_NODE_A(kidney_shot, "kidney shot", "gouge");

    ACTION_NODE_A(eviscerate, "eviscerate", "rupture");

    ACTION_NODE_A(slice_and_dice, "slice and dice", "eviscerate");

    ACTION_NODE_A(sap, "sap", "blind");
};

class RogueStealthStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    RogueStealthStrategyActionNodeFactory()
    {
        creators["ambush"] = &ambush;
        creators["backstab"] = &backstab;
        creators["cheap shot"] = &cheap_shot;
    }

private:
    ACTION_NODE_A(ambush, "ambush", "backstab");

    ACTION_NODE_A(backstab, "backstab", "cheap shot");

    ACTION_NODE_A(cheap_shot, "cheap shot", "sinister strike");
};

RogueStrategy::RogueStrategy(PlayerbotAI* ai) : ClassStrategy(ai)
{
    actionNodeFactories.Add(new RogueStrategyActionNodeFactory());
}

RogueStealthStrategy::RogueStealthStrategy(PlayerbotAI* ai) : StealthStrategy(ai)
{
    actionNodeFactories.Add(new RogueStealthStrategyActionNodeFactory());
}

#ifdef MANGOSBOT_ZERO // Vanilla

NextAction** RogueStrategy::GetDefaultCombatActions()
{
    return NextAction::array(0, new NextAction("melee", ACTION_IDLE), NULL);
}

void RogueStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "critical health",
        NextAction::array(0, new NextAction("blind", ACTION_EMERGENCY + 2), 
                             new NextAction("vanish", ACTION_EMERGENCY + 1), 
                             new NextAction("vanish", ACTION_EMERGENCY), NULL)));

    triggers.push_back(new TriggerNode(
        "low health",
        NextAction::array(0, new NextAction("evasion", ACTION_EMERGENCY), 
                             new NextAction("feint", ACTION_EMERGENCY), NULL)));

    triggers.push_back(new TriggerNode(
        "sprint",
        NextAction::array(0, new NextAction("sprint", ACTION_HIGH + 1), NULL)));
}

void RogueStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassStrategy::InitNonCombatTriggers(triggers);
}

void RogueStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ClassStrategy::InitReactionTriggers(triggers);
}

void RogueStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ClassStrategy::InitDeadTriggers(triggers);
}

void RoguePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPvpStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "player has flag",
        NextAction::array(0, new NextAction("sprint", ACTION_HIGH), NULL)));

    triggers.push_back(new TriggerNode(
        "enemy flagcarrier near",
        NextAction::array(0, new NextAction("sprint", ACTION_HIGH), NULL)));
}

void RoguePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPvpStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "enemy flagcarrier near",
        NextAction::array(0, new NextAction("sprint", ACTION_HIGH), NULL)));

    triggers.push_back(new TriggerNode(
        "player has flag",
        NextAction::array(0, new NextAction("sprint", ACTION_HIGH), NULL)));
}

void RoguePvpStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPvpStrategy::InitReactionTriggers(triggers);
}

void RoguePvpStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPvpStrategy::InitDeadTriggers(triggers);
}

void RoguePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPveStrategy::InitCombatTriggers(triggers);
}

void RoguePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPveStrategy::InitNonCombatTriggers(triggers);
}

void RoguePveStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPveStrategy::InitReactionTriggers(triggers);
}

void RoguePveStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPveStrategy::InitDeadTriggers(triggers);
}

void RogueRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassRaidStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "medium threat",
        NextAction::array(0, new NextAction("feint", ACTION_DISPEL), NULL)));

    triggers.push_back(new TriggerNode(
        "high threat",
        NextAction::array(0, new NextAction("vanish", ACTION_EMERGENCY), NULL)));
}

void RogueRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassRaidStrategy::InitNonCombatTriggers(triggers);
}

void RogueRaidStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ClassRaidStrategy::InitReactionTriggers(triggers);
}

void RogueRaidStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ClassRaidStrategy::InitDeadTriggers(triggers);
}

void RogueAoeStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoeStrategy::InitCombatTriggers(triggers);
}

void RogueAoeStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoeStrategy::InitNonCombatTriggers(triggers);
}

void RogueAoePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoePvpStrategy::InitCombatTriggers(triggers);
}

void RogueAoePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoePvpStrategy::InitNonCombatTriggers(triggers);
}

void RogueAoePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoePveStrategy::InitCombatTriggers(triggers);
}

void RogueAoePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoePveStrategy::InitNonCombatTriggers(triggers);
}

void RogueAoeRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoeRaidStrategy::InitCombatTriggers(triggers);
}

void RogueAoeRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoeRaidStrategy::InitNonCombatTriggers(triggers);
}

void RogueBuffStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffStrategy::InitCombatTriggers(triggers);
}

void RogueBuffStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffStrategy::InitNonCombatTriggers(triggers);
}

void RogueBuffPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffPvpStrategy::InitCombatTriggers(triggers);
}

void RogueBuffPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffPvpStrategy::InitNonCombatTriggers(triggers);
}

void RogueBuffPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffPveStrategy::InitCombatTriggers(triggers);
}

void RogueBuffPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffPveStrategy::InitNonCombatTriggers(triggers);
}

void RogueBuffRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffRaidStrategy::InitCombatTriggers(triggers);
}

void RogueBuffRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffRaidStrategy::InitNonCombatTriggers(triggers);
}

void RogueBoostStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostStrategy::InitCombatTriggers(triggers);
}

void RogueBoostStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostStrategy::InitNonCombatTriggers(triggers);
}

void RogueBoostPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostPvpStrategy::InitCombatTriggers(triggers);
}

void RogueBoostPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostPvpStrategy::InitNonCombatTriggers(triggers);
}

void RogueBoostPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostPveStrategy::InitCombatTriggers(triggers);
}

void RogueBoostPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostPveStrategy::InitNonCombatTriggers(triggers);
}

void RogueBoostRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostRaidStrategy::InitCombatTriggers(triggers);
}

void RogueBoostRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostRaidStrategy::InitNonCombatTriggers(triggers);
}

void RogueCcStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "kick on enemy healer",
        NextAction::array(0, new NextAction("kick on enemy healer", ACTION_INTERRUPT + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "kick",
        NextAction::array(0, new NextAction("kick", ACTION_INTERRUPT + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "sap",
        NextAction::array(0, new NextAction("sap", ACTION_INTERRUPT + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "medium health",
        NextAction::array(0, new NextAction("gouge", ACTION_INTERRUPT), NULL)));
}

void RogueCcStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcStrategy::InitNonCombatTriggers(triggers);
}

void RogueCcPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcPvpStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "enemy out of melee",
        NextAction::array(0, new NextAction("throw grenade", ACTION_INTERRUPT), NULL)));

    triggers.push_back(new TriggerNode(
        "kick",
        NextAction::array(0, new NextAction("throw grenade", ACTION_INTERRUPT), NULL)));
}

void RogueCcPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcPvpStrategy::InitNonCombatTriggers(triggers);
}

void RogueCcPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcPveStrategy::InitCombatTriggers(triggers);
}

void RogueCcPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcPveStrategy::InitNonCombatTriggers(triggers);
}

void RogueCcRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcRaidStrategy::InitCombatTriggers(triggers);
}

void RogueCcRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcRaidStrategy::InitNonCombatTriggers(triggers);
}

void RogueStealthStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
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
        NextAction::array(0, new NextAction("ambush", ACTION_NORMAL + 3), NULL)));

    triggers.push_back(new TriggerNode(
        "not behind target",
        NextAction::array(0, new NextAction("cheap shot", ACTION_NORMAL + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "kick",
        NextAction::array(0, new NextAction("cheap shot", ACTION_NORMAL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "unstealth",
        NextAction::array(0, new NextAction("unstealth", ACTION_NORMAL), NULL)));
}

void RogueStealthStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    StealthStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "no stealth",
        NextAction::array(0, new NextAction("check stealth", ACTION_HIGH), NULL)));

    triggers.push_back(new TriggerNode(
        "unstealth",
        NextAction::array(0, new NextAction("unstealth", ACTION_NORMAL), NULL)));
}

void RogueStealthPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    StealthPvpStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "enemy flagcarrier near",
        NextAction::array(0, new NextAction("sprint", ACTION_HIGH), 
                             new NextAction("unstealth", ACTION_HIGH), NULL)));
}

void RogueStealthPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    StealthPvpStrategy::InitNonCombatTriggers(triggers);
}

void RogueStealthPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    StealthPveStrategy::InitCombatTriggers(triggers);
}

void RogueStealthPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    StealthPveStrategy::InitNonCombatTriggers(triggers);
}

void RogueStealthRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    StealthRaidStrategy::InitCombatTriggers(triggers);
}

void RogueStealthRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    StealthRaidStrategy::InitNonCombatTriggers(triggers);
}

void RoguePoisonsStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void RoguePoisonsStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void RoguePoisonsPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void RoguePoisonsPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "apply mind poison main hand",
        NextAction::array(0, new NextAction("apply mind poison main hand", ACTION_NORMAL), NULL)));

    triggers.push_back(new TriggerNode(
        "apply crippling poison off hand",
        NextAction::array(0, new NextAction("apply crippling poison off hand", ACTION_NORMAL), NULL)));
}

void RoguePoisonsPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void RoguePoisonsPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "apply instant poison main hand",
        NextAction::array(0, new NextAction("apply instant poison main hand", ACTION_NORMAL), NULL)));

    triggers.push_back(new TriggerNode(
        "apply instant poison off hand",
        NextAction::array(0, new NextAction("apply instant poison off hand", ACTION_NORMAL), NULL)));
}

void RoguePoisonsRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    
}

void RoguePoisonsRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "apply instant poison main hand",
        NextAction::array(0, new NextAction("apply instant poison main hand", ACTION_NORMAL), NULL)));

    triggers.push_back(new TriggerNode(
        "apply deadly poison off hand",
        NextAction::array(0, new NextAction("apply deadly poison off hand", ACTION_NORMAL), NULL)));
}

#endif
#ifdef MANGOSBOT_ONE // TBC

NextAction** RogueStrategy::GetDefaultCombatActions()
{
    return NextAction::array(0, new NextAction("melee", ACTION_NORMAL), NULL);
}

void RogueStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "low health",
        NextAction::array(0, new NextAction("blind", ACTION_EMERGENCY + 2), 
                             new NextAction("vanish", ACTION_EMERGENCY + 1), 
                             new NextAction("vanish", ACTION_EMERGENCY), NULL)));

    triggers.push_back(new TriggerNode(
        "low health",
        NextAction::array(0, new NextAction("evasion", ACTION_EMERGENCY), 
                             new NextAction("feint", ACTION_EMERGENCY), NULL)));

    triggers.push_back(new TriggerNode(
        "sprint",
        NextAction::array(0, new NextAction("sprint", ACTION_HIGH + 1), NULL)));
}

void RogueStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassStrategy::InitNonCombatTriggers(triggers);
}

void RogueStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ClassStrategy::InitReactionTriggers(triggers);
}

void RogueStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ClassStrategy::InitDeadTriggers(triggers);
}

void RoguePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPvpStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "player has flag",
        NextAction::array(0, new NextAction("sprint", ACTION_HIGH), NULL)));

    triggers.push_back(new TriggerNode(
        "enemy flagcarrier near",
        NextAction::array(0, new NextAction("sprint", ACTION_HIGH), NULL)));
}

void RoguePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPvpStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "player has flag",
        NextAction::array(0, new NextAction("sprint", ACTION_HIGH), NULL)));

    triggers.push_back(new TriggerNode(
        "enemy flagcarrier near",
        NextAction::array(0, new NextAction("sprint", ACTION_HIGH), NULL)));
}

void RoguePvpStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPvpStrategy::InitReactionTriggers(triggers);
}

void RoguePvpStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPvpStrategy::InitDeadTriggers(triggers);
}

void RoguePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPveStrategy::InitCombatTriggers(triggers);
}

void RoguePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPveStrategy::InitNonCombatTriggers(triggers);
}

void RoguePveStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPveStrategy::InitReactionTriggers(triggers);
}

void RoguePveStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPveStrategy::InitDeadTriggers(triggers);
}

void RogueRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassRaidStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "medium threat",
        NextAction::array(0, new NextAction("feint", ACTION_DISPEL), NULL)));
}

void RogueRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassRaidStrategy::InitNonCombatTriggers(triggers);
}

void RogueRaidStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ClassRaidStrategy::InitReactionTriggers(triggers);
}

void RogueRaidStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ClassRaidStrategy::InitDeadTriggers(triggers);
}

void RogueAoeStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoeStrategy::InitCombatTriggers(triggers);
}

void RogueAoeStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoeStrategy::InitNonCombatTriggers(triggers);
}

void RogueAoePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoePvpStrategy::InitCombatTriggers(triggers);
}

void RogueAoePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoePvpStrategy::InitNonCombatTriggers(triggers);
}

void RogueAoePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoePveStrategy::InitCombatTriggers(triggers);
}

void RogueAoePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoePveStrategy::InitNonCombatTriggers(triggers);
}

void RogueAoeRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoeRaidStrategy::InitCombatTriggers(triggers);
}

void RogueAoeRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoeRaidStrategy::InitNonCombatTriggers(triggers);
}

void RogueBuffStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffStrategy::InitCombatTriggers(triggers);
}

void RogueBuffStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffStrategy::InitNonCombatTriggers(triggers);
}

void RogueBuffPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffPvpStrategy::InitCombatTriggers(triggers);
}

void RogueBuffPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffPvpStrategy::InitNonCombatTriggers(triggers);
}

void RogueBuffPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffPveStrategy::InitCombatTriggers(triggers);
}

void RogueBuffPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffPveStrategy::InitNonCombatTriggers(triggers);
}

void RogueBuffRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffRaidStrategy::InitCombatTriggers(triggers);
}

void RogueBuffRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffRaidStrategy::InitNonCombatTriggers(triggers);
}

void RogueBoostStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostStrategy::InitCombatTriggers(triggers);
}

void RogueBoostStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostStrategy::InitNonCombatTriggers(triggers);
}

void RogueBoostPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostPvpStrategy::InitCombatTriggers(triggers);
}

void RogueBoostPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostPvpStrategy::InitNonCombatTriggers(triggers);
}

void RogueBoostPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostPveStrategy::InitCombatTriggers(triggers);
}

void RogueBoostPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostPveStrategy::InitNonCombatTriggers(triggers);
}

void RogueBoostRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostRaidStrategy::InitCombatTriggers(triggers);
}

void RogueBoostRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostRaidStrategy::InitNonCombatTriggers(triggers);
}

void RogueCcStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "kick on enemy healer",
        NextAction::array(0, new NextAction("kick on enemy healer", ACTION_INTERRUPT + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "kick",
        NextAction::array(0, new NextAction("kick", ACTION_INTERRUPT + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "sap",
        NextAction::array(0, new NextAction("sap", ACTION_INTERRUPT + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "medium health",
        NextAction::array(0, new NextAction("gouge", ACTION_INTERRUPT), NULL)));
}

void RogueCcStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcStrategy::InitNonCombatTriggers(triggers);
}

void RogueCcPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcPvpStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "enemy out of melee",
        NextAction::array(0, new NextAction("throw grenade", ACTION_INTERRUPT), NULL)));

    triggers.push_back(new TriggerNode(
        "kick",
        NextAction::array(0, new NextAction("throw grenade", ACTION_INTERRUPT), NULL)));
}

void RogueCcPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcPvpStrategy::InitNonCombatTriggers(triggers);
}

void RogueCcPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcPveStrategy::InitCombatTriggers(triggers);
}

void RogueCcPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcPveStrategy::InitNonCombatTriggers(triggers);
}

void RogueCcRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcRaidStrategy::InitCombatTriggers(triggers);
}

void RogueCcRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcRaidStrategy::InitNonCombatTriggers(triggers);
}

void RogueStealthStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "stealth",
        NextAction::array(0, new NextAction("stealth", ACTION_EMERGENCY), NULL)));

    triggers.push_back(new TriggerNode(
        "no stealth",
        NextAction::array(0, new NextAction("check stealth", ACTION_EMERGENCY), NULL)));

    triggers.push_back(new TriggerNode(
        "behind target",
        NextAction::array(0, new NextAction("ambush", ACTION_NORMAL + 3), NULL)));

    triggers.push_back(new TriggerNode(
        "not behind target",
        NextAction::array(0, new NextAction("cheap shot", ACTION_NORMAL + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "kick",
        NextAction::array(0, new NextAction("cheap shot", ACTION_NORMAL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "unstealth",
        NextAction::array(0, new NextAction("unstealth", ACTION_NORMAL), NULL)));
}

void RogueStealthStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "no stealth",
        NextAction::array(0, new NextAction("check stealth", ACTION_HIGH), NULL)));

    triggers.push_back(new TriggerNode(
        "unstealth",
        NextAction::array(0, new NextAction("unstealth", ACTION_NORMAL), NULL)));
}

void RogueStealthPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "enemy flagcarrier near",
        NextAction::array(0, new NextAction("sprint", ACTION_HIGH),
                             new NextAction("unstealth", ACTION_HIGH), NULL)));
}

void RogueStealthPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void RogueStealthPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void RogueStealthPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void RogueStealthRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void RogueStealthRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void RoguePoisonsStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void RoguePoisonsStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void RoguePoisonsPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void RoguePoisonsPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "apply mind poison main hand",
        NextAction::array(0, new NextAction("apply mind poison main hand", ACTION_NORMAL), NULL)));

    triggers.push_back(new TriggerNode(
        "apply crippling poison off hand",
        NextAction::array(0, new NextAction("apply crippling poison off hand", ACTION_NORMAL), NULL)));
}

void RoguePoisonsPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void RoguePoisonsPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "apply instant poison main hand",
        NextAction::array(0, new NextAction("apply instant poison main hand", ACTION_NORMAL), NULL)));

    triggers.push_back(new TriggerNode(
        "apply instant poison off hand",
        NextAction::array(0, new NextAction("apply instant poison off hand", ACTION_NORMAL), NULL)));
}

void RoguePoisonsRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void RoguePoisonsRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "apply instant poison main hand",
        NextAction::array(0, new NextAction("apply instant poison main hand", ACTION_NORMAL), NULL)));

    triggers.push_back(new TriggerNode(
        "apply deadly poison off hand",
        NextAction::array(0, new NextAction("apply deadly poison off hand", ACTION_NORMAL), NULL)));
}

#endif
#ifdef MANGOSBOT_TWO // WOTLK

NextAction** RogueStrategy::GetDefaultCombatActions()
{
    return NextAction::array(0, new NextAction("melee", ACTION_NORMAL), NULL);
}

void RogueStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassStrategy::InitCombatTriggers(triggers);

        triggers.push_back(new TriggerNode(
        "low health",
        NextAction::array(0, new NextAction("blind", ACTION_EMERGENCY + 2), 
                             new NextAction("vanish", ACTION_EMERGENCY + 1), 
                             new NextAction("vanish", ACTION_EMERGENCY), NULL)));

    triggers.push_back(new TriggerNode(
        "low health",
        NextAction::array(0, new NextAction("evasion", ACTION_EMERGENCY), 
                             new NextAction("feint", ACTION_EMERGENCY), NULL)));

    triggers.push_back(new TriggerNode(
        "cloak of shadows",
        NextAction::array(0, new NextAction("cloak of shadows", ACTION_INTERRUPT + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "sprint",
        NextAction::array(0, new NextAction("sprint", ACTION_HIGH + 1), NULL)));
}

void RogueStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassStrategy::InitNonCombatTriggers(triggers);
}

void RogueStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ClassStrategy::InitReactionTriggers(triggers);
}

void RogueStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ClassStrategy::InitDeadTriggers(triggers);
}

void RoguePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPvpStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "player has flag",
        NextAction::array(0, new NextAction("sprint", ACTION_HIGH), NULL)));

    triggers.push_back(new TriggerNode(
        "enemy flagcarrier near",
        NextAction::array(0, new NextAction("sprint", ACTION_HIGH), NULL)));
}

void RoguePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPvpStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "player has flag",
        NextAction::array(0, new NextAction("sprint", ACTION_HIGH), NULL)));

    triggers.push_back(new TriggerNode(
        "enemy flagcarrier near",
        NextAction::array(0, new NextAction("sprint", ACTION_HIGH), NULL)));
}

void RoguePvpStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPvpStrategy::InitReactionTriggers(triggers);
}

void RoguePvpStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPvpStrategy::InitDeadTriggers(triggers);
}

void RoguePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPveStrategy::InitCombatTriggers(triggers);
}

void RoguePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPveStrategy::InitNonCombatTriggers(triggers);
}

void RoguePveStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPveStrategy::InitReactionTriggers(triggers);
}

void RoguePveStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPveStrategy::InitDeadTriggers(triggers);
}

void RogueRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassRaidStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "medium threat",
        NextAction::array(0, new NextAction("feint", ACTION_DISPEL), NULL)));
}

void RogueRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassRaidStrategy::InitNonCombatTriggers(triggers);
}

void RogueRaidStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ClassRaidStrategy::InitReactionTriggers(triggers);
}

void RogueRaidStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ClassRaidStrategy::InitDeadTriggers(triggers);
}

void RogueAoeStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoeStrategy::InitCombatTriggers(triggers);
}

void RogueAoeStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoeStrategy::InitNonCombatTriggers(triggers);
}

void RogueAoePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoePvpStrategy::InitCombatTriggers(triggers);
}

void RogueAoePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoePvpStrategy::InitNonCombatTriggers(triggers);
}

void RogueAoePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoePveStrategy::InitCombatTriggers(triggers);
}

void RogueAoePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoePveStrategy::InitNonCombatTriggers(triggers);
}

void RogueAoeRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoeRaidStrategy::InitCombatTriggers(triggers);
}

void RogueAoeRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoeRaidStrategy::InitNonCombatTriggers(triggers);
}

void RogueBuffStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffStrategy::InitCombatTriggers(triggers);
}

void RogueBuffStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffStrategy::InitNonCombatTriggers(triggers);
}

void RogueBuffPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffPvpStrategy::InitCombatTriggers(triggers);
}

void RogueBuffPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffPvpStrategy::InitNonCombatTriggers(triggers);
}

void RogueBuffPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffPveStrategy::InitCombatTriggers(triggers);
}

void RogueBuffPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffPveStrategy::InitNonCombatTriggers(triggers);
}

void RogueBuffRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffRaidStrategy::InitCombatTriggers(triggers);
}

void RogueBuffRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffRaidStrategy::InitNonCombatTriggers(triggers);
}

void RogueBoostStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostStrategy::InitCombatTriggers(triggers);
}

void RogueBoostStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostStrategy::InitNonCombatTriggers(triggers);
}

void RogueBoostPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostPvpStrategy::InitCombatTriggers(triggers);
}

void RogueBoostPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostPvpStrategy::InitNonCombatTriggers(triggers);
}

void RogueBoostPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostPveStrategy::InitCombatTriggers(triggers);
}

void RogueBoostPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostPveStrategy::InitNonCombatTriggers(triggers);
}

void RogueBoostRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostRaidStrategy::InitCombatTriggers(triggers);
}

void RogueBoostRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostRaidStrategy::InitNonCombatTriggers(triggers);
}

void RogueCcStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "kick on enemy healer",
        NextAction::array(0, new NextAction("kick on enemy healer", ACTION_INTERRUPT + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "kick",
        NextAction::array(0, new NextAction("kick", ACTION_INTERRUPT + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "sap",
        NextAction::array(0, new NextAction("sap", ACTION_INTERRUPT + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "medium health",
        NextAction::array(0, new NextAction("gouge", ACTION_INTERRUPT), NULL)));
}

void RogueCcStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcStrategy::InitNonCombatTriggers(triggers);
}

void RogueCcPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcPvpStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "enemy out of melee",
        NextAction::array(0, new NextAction("throw grenade", ACTION_INTERRUPT), NULL)));

    triggers.push_back(new TriggerNode(
        "kick",
        NextAction::array(0, new NextAction("throw grenade", ACTION_INTERRUPT), NULL)));
}

void RogueCcPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcPvpStrategy::InitNonCombatTriggers(triggers);
}

void RogueCcPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcPveStrategy::InitCombatTriggers(triggers);
}

void RogueCcPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcPveStrategy::InitNonCombatTriggers(triggers);
}

void RogueCcRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcRaidStrategy::InitCombatTriggers(triggers);
}

void RogueCcRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcRaidStrategy::InitNonCombatTriggers(triggers);
}

void RogueStealthStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "stealth",
        NextAction::array(0, new NextAction("stealth", ACTION_EMERGENCY), NULL)));

    triggers.push_back(new TriggerNode(
        "no stealth",
        NextAction::array(0, new NextAction("check stealth", ACTION_EMERGENCY), NULL)));

    triggers.push_back(new TriggerNode(
        "behind target",
        NextAction::array(0, new NextAction("ambush", ACTION_NORMAL + 3), NULL)));

    triggers.push_back(new TriggerNode(
        "not behind target",
        NextAction::array(0, new NextAction("cheap shot", ACTION_NORMAL + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "kick",
        NextAction::array(0, new NextAction("cheap shot", ACTION_NORMAL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "unstealth",
        NextAction::array(0, new NextAction("unstealth", ACTION_NORMAL), NULL)));
}

void RogueStealthStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "no stealth",
        NextAction::array(0, new NextAction("check stealth", ACTION_HIGH), NULL)));

    triggers.push_back(new TriggerNode(
        "unstealth",
        NextAction::array(0, new NextAction("unstealth", ACTION_NORMAL), NULL)));
}

void RogueStealthPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "enemy flagcarrier near",
        NextAction::array(0, new NextAction("sprint", ACTION_HIGH),
                             new NextAction("unstealth", ACTION_HIGH), NULL)));
}

void RogueStealthPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void RogueStealthPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void RogueStealthPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void RogueStealthRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void RogueStealthRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void RoguePoisonsStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void RoguePoisonsStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void RoguePoisonsPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void RoguePoisonsPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "apply mind poison main hand",
        NextAction::array(0, new NextAction("apply mind poison main hand", ACTION_NORMAL), NULL)));

    triggers.push_back(new TriggerNode(
        "apply crippling poison off hand",
        NextAction::array(0, new NextAction("apply crippling poison off hand", ACTION_NORMAL), NULL)));
}

void RoguePoisonsPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void RoguePoisonsPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "apply instant poison main hand",
        NextAction::array(0, new NextAction("apply instant poison main hand", ACTION_NORMAL), NULL)));

    triggers.push_back(new TriggerNode(
        "apply instant poison off hand",
        NextAction::array(0, new NextAction("apply instant poison off hand", ACTION_NORMAL), NULL)));
}

void RoguePoisonsRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void RoguePoisonsRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "apply instant poison main hand",
        NextAction::array(0, new NextAction("apply instant poison main hand", ACTION_NORMAL), NULL)));

    triggers.push_back(new TriggerNode(
        "apply deadly poison off hand",
        NextAction::array(0, new NextAction("apply deadly poison off hand", ACTION_NORMAL), NULL)));
}

#endif

void RogueManualPoisonStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        triggerName,
        NextAction::array(0, new NextAction(actionName, ACTION_NORMAL), NULL)));
}

class RogueStealthedStrategyMultiplier : public Multiplier
{
public:
    RogueStealthedStrategyMultiplier(PlayerbotAI* ai) : Multiplier(ai, "stealthed") {}

    float GetValue(Action* action) override
    {
        // Only allow stealth actions
        const std::string& actionName = action->getName();
        if ((actionName == "unstealth") ||
            (actionName == "check stealth") ||
            (actionName == "cold blood") ||
            (actionName == "preparation") ||
            (actionName == "premeditation") ||
            (actionName == "shadowstep") ||
            (actionName == "sprint") ||
            (actionName == "ambush") ||
            (actionName == "cheap shot") ||
            (actionName == "sap") ||
            (actionName == "garrote") ||
            (actionName == "food") ||
            (actionName == "backstab") ||
            (actionName == "sinister strike") ||
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
            (actionName == "update raid strats") ||
            (actionName == "accept invitation") ||
            (actionName == "bg status") ||
            (actionName == "bg leave") ||
            (actionName == "check mount state") ||
            (actionName == "loot roll") ||
            (actionName == "auto loot roll") ||
            (actionName == "attack anything") ||
            (actionName == "choose rpg target") ||
            (actionName == "move to rpg target") ||
            (actionName == "travel"))
        {
            return 1.0f;
        }

        return 0.0f;
    }
};

void RogueStealthedStrategy::InitCombatMultipliers(std::list<Multiplier*>& multipliers)
{
    multipliers.push_back(new RogueStealthedStrategyMultiplier(ai));
}

void RogueStealthedStrategy::InitNonCombatMultipliers(std::list<Multiplier*>& multipliers)
{
    RogueStealthedStrategy::InitCombatMultipliers(multipliers);
}
