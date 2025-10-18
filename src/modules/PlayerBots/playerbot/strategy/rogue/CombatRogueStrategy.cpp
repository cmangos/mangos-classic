
#include "playerbot/playerbot.h"
#include "CombatRogueStrategy.h"

using namespace ai;

#ifdef MANGOSBOT_ZERO // Vanilla

void CombatRogueStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RogueStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "riposte",
        NextAction::array(0, new NextAction("riposte", ACTION_HIGH + 3), NULL)));

    triggers.push_back(new TriggerNode(
        "slice and dice",
        NextAction::array(0, new NextAction("slice and dice", ACTION_HIGH + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "eviscerate",
        NextAction::array(0, new NextAction("eviscerate", ACTION_HIGH + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "rupture",
        NextAction::array(0, new NextAction("rupture", ACTION_HIGH), NULL)));

    triggers.push_back(new TriggerNode(
        "sinister strike",
        NextAction::array(0, new NextAction("sinister strike", ACTION_NORMAL), NULL)));
}

void CombatRogueStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RogueStrategy::InitNonCombatTriggers(triggers);
}

void CombatRogueStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    RogueStrategy::InitReactionTriggers(triggers);
}

void CombatRogueStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    RogueStrategy::InitDeadTriggers(triggers);
}

void CombatRoguePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueStrategy::InitCombatTriggers(triggers);
    RoguePveStrategy::InitCombatTriggers(triggers);
}

void CombatRoguePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueStrategy::InitNonCombatTriggers(triggers);
    RoguePveStrategy::InitNonCombatTriggers(triggers);
}

void CombatRoguePveStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueStrategy::InitReactionTriggers(triggers);
    RoguePveStrategy::InitReactionTriggers(triggers);
}

void CombatRoguePveStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueStrategy::InitDeadTriggers(triggers);
    RoguePveStrategy::InitDeadTriggers(triggers);
}

void CombatRoguePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueStrategy::InitCombatTriggers(triggers);
    RoguePvpStrategy::InitCombatTriggers(triggers);
}

void CombatRoguePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueStrategy::InitNonCombatTriggers(triggers);
    RoguePvpStrategy::InitNonCombatTriggers(triggers);
}

void CombatRoguePvpStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueStrategy::InitReactionTriggers(triggers);
    RoguePvpStrategy::InitReactionTriggers(triggers);
}

void CombatRoguePvpStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueStrategy::InitDeadTriggers(triggers);
    RoguePvpStrategy::InitDeadTriggers(triggers);
}

void CombatRogueRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueStrategy::InitCombatTriggers(triggers);
    RogueRaidStrategy::InitCombatTriggers(triggers);
}

void CombatRogueRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueStrategy::InitNonCombatTriggers(triggers);
    RogueRaidStrategy::InitNonCombatTriggers(triggers);
}

void CombatRogueRaidStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueStrategy::InitReactionTriggers(triggers);
    RogueRaidStrategy::InitReactionTriggers(triggers);
}

void CombatRogueRaidStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueStrategy::InitDeadTriggers(triggers);
    RogueRaidStrategy::InitDeadTriggers(triggers);
}

void CombatRogueAoeStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RogueAoeStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "melee light aoe",
        NextAction::array(0, new NextAction("blade flurry", ACTION_HIGH + 4), NULL)));

    triggers.push_back(new TriggerNode(
        "melee light aoe",
        NextAction::array(0, new NextAction("oil of immolation", ACTION_HIGH + 3), NULL)));
}

void CombatRogueAoeStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RogueAoeStrategy::InitNonCombatTriggers(triggers);
}

void CombatRogueAoePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueAoeStrategy::InitCombatTriggers(triggers);
    RogueAoePveStrategy::InitCombatTriggers(triggers);
}

void CombatRogueAoePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueAoeStrategy::InitNonCombatTriggers(triggers);
    RogueAoePveStrategy::InitNonCombatTriggers(triggers);
}

void CombatRogueAoePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueAoeStrategy::InitCombatTriggers(triggers);
    RogueAoePvpStrategy::InitCombatTriggers(triggers);
}

void CombatRogueAoePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueAoeStrategy::InitNonCombatTriggers(triggers);
    RogueAoePvpStrategy::InitNonCombatTriggers(triggers);
}

void CombatRogueAoeRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueAoeStrategy::InitCombatTriggers(triggers);
    RogueAoeRaidStrategy::InitCombatTriggers(triggers);
}

void CombatRogueAoeRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueAoeStrategy::InitNonCombatTriggers(triggers);
    RogueAoeRaidStrategy::InitNonCombatTriggers(triggers);
}

void CombatRogueBuffStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RogueBuffStrategy::InitCombatTriggers(triggers);
}

void CombatRogueBuffStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RogueBuffStrategy::InitNonCombatTriggers(triggers);
}

void CombatRogueBuffPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueBuffStrategy::InitCombatTriggers(triggers);
    RogueBuffPveStrategy::InitCombatTriggers(triggers);
}

void CombatRogueBuffPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueBuffStrategy::InitNonCombatTriggers(triggers);
    RogueBuffPveStrategy::InitNonCombatTriggers(triggers);
}

void CombatRogueBuffPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueBuffStrategy::InitCombatTriggers(triggers);
    RogueBuffPvpStrategy::InitCombatTriggers(triggers);
}

void CombatRogueBuffPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueBuffStrategy::InitNonCombatTriggers(triggers);
    RogueBuffPvpStrategy::InitNonCombatTriggers(triggers);
}

void CombatRogueBuffRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueBuffStrategy::InitCombatTriggers(triggers);
    RogueBuffRaidStrategy::InitCombatTriggers(triggers);
}

void CombatRogueBuffRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueBuffStrategy::InitNonCombatTriggers(triggers);
    RogueBuffRaidStrategy::InitNonCombatTriggers(triggers);
}

void CombatRogueBoostStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RogueBoostStrategy::InitCombatTriggers(triggers);
}

void CombatRogueBoostStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RogueBoostStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "adrenaline rush",
        NextAction::array(0, new NextAction("adrenaline rush", ACTION_HIGH + 5), NULL)));

    triggers.push_back(new TriggerNode(
        "blade flurry",
        NextAction::array(0, new NextAction("blade flurry", ACTION_HIGH + 4), NULL)));
}

void CombatRogueBoostPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueBoostStrategy::InitCombatTriggers(triggers);
    RogueBoostPveStrategy::InitCombatTriggers(triggers);
}

void CombatRogueBoostPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueBoostStrategy::InitNonCombatTriggers(triggers);
    RogueBoostPveStrategy::InitNonCombatTriggers(triggers);
}

void CombatRogueBoostPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueBoostStrategy::InitCombatTriggers(triggers);
    RogueBoostPvpStrategy::InitCombatTriggers(triggers);
}

void CombatRogueBoostPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueBoostStrategy::InitNonCombatTriggers(triggers);
    RogueBoostPvpStrategy::InitNonCombatTriggers(triggers);
}

void CombatRogueBoostRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueBoostStrategy::InitCombatTriggers(triggers);
    RogueBoostRaidStrategy::InitCombatTriggers(triggers);
}

void CombatRogueBoostRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueBoostStrategy::InitNonCombatTriggers(triggers);
    RogueBoostRaidStrategy::InitNonCombatTriggers(triggers);
}

void CombatRogueCcStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RogueCcStrategy::InitCombatTriggers(triggers);
}

void CombatRogueCcStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RogueCcStrategy::InitNonCombatTriggers(triggers);
}

void CombatRogueCcPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueCcStrategy::InitCombatTriggers(triggers);
    RogueCcPveStrategy::InitCombatTriggers(triggers);
}

void CombatRogueCcPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueCcStrategy::InitNonCombatTriggers(triggers);
    RogueCcPveStrategy::InitNonCombatTriggers(triggers);
}

void CombatRogueCcPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueCcStrategy::InitCombatTriggers(triggers);
    RogueCcPvpStrategy::InitCombatTriggers(triggers);
}

void CombatRogueCcPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueCcStrategy::InitNonCombatTriggers(triggers);
    RogueCcPvpStrategy::InitNonCombatTriggers(triggers);
}

void CombatRogueCcRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueCcStrategy::InitCombatTriggers(triggers);
    RogueCcRaidStrategy::InitCombatTriggers(triggers);
}

void CombatRogueCcRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueCcStrategy::InitNonCombatTriggers(triggers);
    RogueCcRaidStrategy::InitNonCombatTriggers(triggers);
}

void CombatRogueStealthStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RogueStealthStrategy::InitCombatTriggers(triggers);
}

void CombatRogueStealthStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RogueStealthStrategy::InitNonCombatTriggers(triggers);
}

void CombatRogueStealthPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueStealthStrategy::InitCombatTriggers(triggers);
    RogueStealthPveStrategy::InitCombatTriggers(triggers);
}

void CombatRogueStealthPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueStealthStrategy::InitNonCombatTriggers(triggers);
    RogueStealthPveStrategy::InitNonCombatTriggers(triggers);
}

void CombatRogueStealthPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueStealthStrategy::InitCombatTriggers(triggers);
    RogueStealthPvpStrategy::InitCombatTriggers(triggers);
}

void CombatRogueStealthPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueStealthStrategy::InitNonCombatTriggers(triggers);
    RogueStealthPvpStrategy::InitNonCombatTriggers(triggers);
}

void CombatRogueStealthRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueStealthStrategy::InitCombatTriggers(triggers);
    RogueStealthRaidStrategy::InitCombatTriggers(triggers);
}

void CombatRogueStealthRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueStealthStrategy::InitNonCombatTriggers(triggers);
    RogueStealthRaidStrategy::InitNonCombatTriggers(triggers);
}

void CombatRoguePoisonsStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RoguePoisonsStrategy::InitCombatTriggers(triggers);
}

void CombatRoguePoisonsStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RoguePoisonsStrategy::InitNonCombatTriggers(triggers);
}

void CombatRoguePoisonsPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRoguePoisonsStrategy::InitCombatTriggers(triggers);
    RoguePoisonsPveStrategy::InitCombatTriggers(triggers);
}

void CombatRoguePoisonsPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRoguePoisonsStrategy::InitNonCombatTriggers(triggers);
    RoguePoisonsPveStrategy::InitNonCombatTriggers(triggers);
}

void CombatRoguePoisonsPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRoguePoisonsStrategy::InitCombatTriggers(triggers);
    RoguePoisonsPvpStrategy::InitCombatTriggers(triggers);
}

void CombatRoguePoisonsPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRoguePoisonsStrategy::InitNonCombatTriggers(triggers);
    RoguePoisonsPvpStrategy::InitNonCombatTriggers(triggers);
}

void CombatRoguePoisonsRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRoguePoisonsStrategy::InitCombatTriggers(triggers);
    RoguePoisonsRaidStrategy::InitCombatTriggers(triggers);
}

void CombatRoguePoisonsRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRoguePoisonsStrategy::InitNonCombatTriggers(triggers);
    RoguePoisonsRaidStrategy::InitNonCombatTriggers(triggers);
}

#endif
#ifdef MANGOSBOT_ONE // TBC

void CombatRogueStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RogueStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "riposte",
        NextAction::array(0, new NextAction("riposte", ACTION_HIGH + 3), NULL)));

    triggers.push_back(new TriggerNode(
        "slice and dice",
        NextAction::array(0, new NextAction("slice and dice", ACTION_HIGH + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "eviscerate",
        NextAction::array(0, new NextAction("eviscerate", ACTION_HIGH + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "rupture",
        NextAction::array(0, new NextAction("rupture", ACTION_HIGH), NULL)));

    triggers.push_back(new TriggerNode(
        "sinister strike",
        NextAction::array(0, new NextAction("sinister strike", ACTION_NORMAL), NULL)));
}

void CombatRogueStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RogueStrategy::InitNonCombatTriggers(triggers);
}

void CombatRogueStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    RogueStrategy::InitReactionTriggers(triggers);
}

void CombatRogueStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    RogueStrategy::InitDeadTriggers(triggers);
}

void CombatRoguePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueStrategy::InitCombatTriggers(triggers);
    RoguePveStrategy::InitCombatTriggers(triggers);
}

void CombatRoguePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueStrategy::InitNonCombatTriggers(triggers);
    RoguePveStrategy::InitNonCombatTriggers(triggers);
}

void CombatRoguePveStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueStrategy::InitReactionTriggers(triggers);
    RoguePveStrategy::InitReactionTriggers(triggers);
}

void CombatRoguePveStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueStrategy::InitDeadTriggers(triggers);
    RoguePveStrategy::InitDeadTriggers(triggers);
}

void CombatRoguePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueStrategy::InitCombatTriggers(triggers);
    RoguePvpStrategy::InitCombatTriggers(triggers);
}

void CombatRoguePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueStrategy::InitNonCombatTriggers(triggers);
    RoguePvpStrategy::InitNonCombatTriggers(triggers);
}

void CombatRoguePvpStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueStrategy::InitReactionTriggers(triggers);
    RoguePvpStrategy::InitReactionTriggers(triggers);
}

void CombatRoguePvpStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueStrategy::InitDeadTriggers(triggers);
    RoguePvpStrategy::InitDeadTriggers(triggers);
}

void CombatRogueRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueStrategy::InitCombatTriggers(triggers);
    RogueRaidStrategy::InitCombatTriggers(triggers);
}

void CombatRogueRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueStrategy::InitNonCombatTriggers(triggers);
    RogueRaidStrategy::InitNonCombatTriggers(triggers);
}

void CombatRogueRaidStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueStrategy::InitReactionTriggers(triggers);
    RogueRaidStrategy::InitReactionTriggers(triggers);
}

void CombatRogueRaidStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueStrategy::InitDeadTriggers(triggers);
    RogueRaidStrategy::InitDeadTriggers(triggers);
}

void CombatRogueAoeStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RogueAoeStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "melee light aoe",
        NextAction::array(0, new NextAction("blade flurry", ACTION_HIGH + 4), NULL)));

    triggers.push_back(new TriggerNode(
        "melee light aoe",
        NextAction::array(0, new NextAction("oil of immolation", ACTION_HIGH + 4), NULL)));
}

void CombatRogueAoeStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RogueAoeStrategy::InitNonCombatTriggers(triggers);
}

void CombatRogueAoePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueAoeStrategy::InitCombatTriggers(triggers);
    RogueAoePveStrategy::InitCombatTriggers(triggers);
}

void CombatRogueAoePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueAoeStrategy::InitNonCombatTriggers(triggers);
    RogueAoePveStrategy::InitNonCombatTriggers(triggers);
}

void CombatRogueAoePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueAoeStrategy::InitCombatTriggers(triggers);
    RogueAoePvpStrategy::InitCombatTriggers(triggers);
}

void CombatRogueAoePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueAoeStrategy::InitNonCombatTriggers(triggers);
    RogueAoePvpStrategy::InitNonCombatTriggers(triggers);
}

void CombatRogueAoeRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueAoeStrategy::InitCombatTriggers(triggers);
    RogueAoeRaidStrategy::InitCombatTriggers(triggers);
}

void CombatRogueAoeRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueAoeStrategy::InitNonCombatTriggers(triggers);
    RogueAoeRaidStrategy::InitNonCombatTriggers(triggers);
}

void CombatRogueBuffStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RogueBuffStrategy::InitCombatTriggers(triggers);
}

void CombatRogueBuffStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RogueBuffStrategy::InitNonCombatTriggers(triggers);
}

void CombatRogueBuffPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueBuffStrategy::InitCombatTriggers(triggers);
    RogueBuffPveStrategy::InitCombatTriggers(triggers);
}

void CombatRogueBuffPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueBuffStrategy::InitNonCombatTriggers(triggers);
    RogueBuffPveStrategy::InitNonCombatTriggers(triggers);
}

void CombatRogueBuffPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueBuffStrategy::InitCombatTriggers(triggers);
    RogueBuffPvpStrategy::InitCombatTriggers(triggers);
}

void CombatRogueBuffPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueBuffStrategy::InitNonCombatTriggers(triggers);
    RogueBuffPvpStrategy::InitNonCombatTriggers(triggers);
}

void CombatRogueBuffRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueBuffStrategy::InitCombatTriggers(triggers);
    RogueBuffRaidStrategy::InitCombatTriggers(triggers);
}

void CombatRogueBuffRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueBuffStrategy::InitNonCombatTriggers(triggers);
    RogueBuffRaidStrategy::InitNonCombatTriggers(triggers);
}

void CombatRogueBoostStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RogueBoostStrategy::InitCombatTriggers(triggers);
}

void CombatRogueBoostStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RogueBoostStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "adrenaline rush",
        NextAction::array(0, new NextAction("adrenaline rush", ACTION_HIGH + 5), NULL)));

    triggers.push_back(new TriggerNode(
        "blade flurry",
        NextAction::array(0, new NextAction("blade flurry", ACTION_HIGH + 4), NULL)));
}

void CombatRogueBoostPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueBoostStrategy::InitCombatTriggers(triggers);
    RogueBoostPveStrategy::InitCombatTriggers(triggers);
}

void CombatRogueBoostPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueBoostStrategy::InitNonCombatTriggers(triggers);
    RogueBoostPveStrategy::InitNonCombatTriggers(triggers);
}

void CombatRogueBoostPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueBoostStrategy::InitCombatTriggers(triggers);
    RogueBoostPvpStrategy::InitCombatTriggers(triggers);
}

void CombatRogueBoostPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueBoostStrategy::InitNonCombatTriggers(triggers);
    RogueBoostPvpStrategy::InitNonCombatTriggers(triggers);
}

void CombatRogueBoostRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueBoostStrategy::InitCombatTriggers(triggers);
    RogueBoostRaidStrategy::InitCombatTriggers(triggers);
}

void CombatRogueBoostRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueBoostStrategy::InitNonCombatTriggers(triggers);
    RogueBoostRaidStrategy::InitNonCombatTriggers(triggers);
}

void CombatRogueCcStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RogueCcStrategy::InitCombatTriggers(triggers);
}

void CombatRogueCcStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RogueCcStrategy::InitNonCombatTriggers(triggers);
}

void CombatRogueCcPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueCcStrategy::InitCombatTriggers(triggers);
    RogueCcPveStrategy::InitCombatTriggers(triggers);
}

void CombatRogueCcPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueCcStrategy::InitNonCombatTriggers(triggers);
    RogueCcPveStrategy::InitNonCombatTriggers(triggers);
}

void CombatRogueCcPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueCcStrategy::InitCombatTriggers(triggers);
    RogueCcPvpStrategy::InitCombatTriggers(triggers);
}

void CombatRogueCcPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueCcStrategy::InitNonCombatTriggers(triggers);
    RogueCcPvpStrategy::InitNonCombatTriggers(triggers);
}

void CombatRogueCcRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueCcStrategy::InitCombatTriggers(triggers);
    RogueCcRaidStrategy::InitCombatTriggers(triggers);
}

void CombatRogueCcRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueCcStrategy::InitNonCombatTriggers(triggers);
    RogueCcRaidStrategy::InitNonCombatTriggers(triggers);
}

void CombatRogueStealthStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RogueStealthStrategy::InitCombatTriggers(triggers);
}

void CombatRogueStealthStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RogueStealthStrategy::InitNonCombatTriggers(triggers);
}

void CombatRogueStealthPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueStealthStrategy::InitCombatTriggers(triggers);
    RogueStealthPveStrategy::InitCombatTriggers(triggers);
}

void CombatRogueStealthPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueStealthStrategy::InitNonCombatTriggers(triggers);
    RogueStealthPveStrategy::InitNonCombatTriggers(triggers);
}

void CombatRogueStealthPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueStealthStrategy::InitCombatTriggers(triggers);
    RogueStealthPvpStrategy::InitCombatTriggers(triggers);
}

void CombatRogueStealthPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueStealthStrategy::InitNonCombatTriggers(triggers);
    RogueStealthPvpStrategy::InitNonCombatTriggers(triggers);
}

void CombatRogueStealthRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueStealthStrategy::InitCombatTriggers(triggers);
    RogueStealthRaidStrategy::InitCombatTriggers(triggers);
}

void CombatRogueStealthRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueStealthStrategy::InitNonCombatTriggers(triggers);
    RogueStealthRaidStrategy::InitNonCombatTriggers(triggers);
}

void CombatRoguePoisonsStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RoguePoisonsStrategy::InitCombatTriggers(triggers);
}

void CombatRoguePoisonsStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RoguePoisonsStrategy::InitNonCombatTriggers(triggers);
}

void CombatRoguePoisonsPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRoguePoisonsStrategy::InitCombatTriggers(triggers);
    RoguePoisonsPveStrategy::InitCombatTriggers(triggers);
}

void CombatRoguePoisonsPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRoguePoisonsStrategy::InitNonCombatTriggers(triggers);
    RoguePoisonsPveStrategy::InitNonCombatTriggers(triggers);
}

void CombatRoguePoisonsPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRoguePoisonsStrategy::InitCombatTriggers(triggers);
    RoguePoisonsPvpStrategy::InitCombatTriggers(triggers);
}

void CombatRoguePoisonsPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRoguePoisonsStrategy::InitNonCombatTriggers(triggers);
    RoguePoisonsPvpStrategy::InitNonCombatTriggers(triggers);
}

void CombatRoguePoisonsRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRoguePoisonsStrategy::InitCombatTriggers(triggers);
    RoguePoisonsRaidStrategy::InitCombatTriggers(triggers);
}

void CombatRoguePoisonsRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRoguePoisonsStrategy::InitNonCombatTriggers(triggers);
    RoguePoisonsRaidStrategy::InitNonCombatTriggers(triggers);
}

#endif
#ifdef MANGOSBOT_TWO // WOTLK

void CombatRogueStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RogueStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "riposte",
        NextAction::array(0, new NextAction("riposte", ACTION_HIGH + 3), NULL)));

    triggers.push_back(new TriggerNode(
        "slice and dice",
        NextAction::array(0, new NextAction("slice and dice", ACTION_HIGH + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "eviscerate",
        NextAction::array(0, new NextAction("eviscerate", ACTION_HIGH + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "rupture",
        NextAction::array(0, new NextAction("rupture", ACTION_HIGH), NULL)));

    triggers.push_back(new TriggerNode(
        "sinister strike",
        NextAction::array(0, new NextAction("sinister strike", ACTION_NORMAL), NULL)));
}

void CombatRogueStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RogueStrategy::InitNonCombatTriggers(triggers);
}

void CombatRogueStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    RogueStrategy::InitReactionTriggers(triggers);
}

void CombatRogueStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    RogueStrategy::InitDeadTriggers(triggers);
}

void CombatRoguePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueStrategy::InitCombatTriggers(triggers);
    RoguePveStrategy::InitCombatTriggers(triggers);
}

void CombatRoguePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueStrategy::InitNonCombatTriggers(triggers);
    RoguePveStrategy::InitNonCombatTriggers(triggers);
}

void CombatRoguePveStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueStrategy::InitReactionTriggers(triggers);
    RoguePveStrategy::InitReactionTriggers(triggers);
}

void CombatRoguePveStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueStrategy::InitDeadTriggers(triggers);
    RoguePveStrategy::InitDeadTriggers(triggers);
}

void CombatRoguePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueStrategy::InitCombatTriggers(triggers);
    RoguePvpStrategy::InitCombatTriggers(triggers);
}

void CombatRoguePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueStrategy::InitNonCombatTriggers(triggers);
    RoguePvpStrategy::InitNonCombatTriggers(triggers);
}

void CombatRoguePvpStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueStrategy::InitReactionTriggers(triggers);
    RoguePvpStrategy::InitReactionTriggers(triggers);
}

void CombatRoguePvpStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueStrategy::InitDeadTriggers(triggers);
    RoguePvpStrategy::InitDeadTriggers(triggers);
}

void CombatRogueRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueStrategy::InitCombatTriggers(triggers);
    RogueRaidStrategy::InitCombatTriggers(triggers);
}

void CombatRogueRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueStrategy::InitNonCombatTriggers(triggers);
    RogueRaidStrategy::InitNonCombatTriggers(triggers);
}

void CombatRogueRaidStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueStrategy::InitReactionTriggers(triggers);
    RogueRaidStrategy::InitReactionTriggers(triggers);
}

void CombatRogueRaidStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueStrategy::InitDeadTriggers(triggers);
    RogueRaidStrategy::InitDeadTriggers(triggers);
}

void CombatRogueAoeStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RogueAoeStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "melee light aoe",
        NextAction::array(0, new NextAction("blade flurry", ACTION_HIGH + 4), NULL)));

    triggers.push_back(new TriggerNode(
        "melee light aoe",
        NextAction::array(0, new NextAction("oil of immolation", ACTION_HIGH + 4), NULL)));
}

void CombatRogueAoeStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RogueAoeStrategy::InitNonCombatTriggers(triggers);
}

void CombatRogueAoePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueAoeStrategy::InitCombatTriggers(triggers);
    RogueAoePveStrategy::InitCombatTriggers(triggers);
}

void CombatRogueAoePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueAoeStrategy::InitNonCombatTriggers(triggers);
    RogueAoePveStrategy::InitNonCombatTriggers(triggers);
}

void CombatRogueAoePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueAoeStrategy::InitCombatTriggers(triggers);
    RogueAoePvpStrategy::InitCombatTriggers(triggers);
}

void CombatRogueAoePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueAoeStrategy::InitNonCombatTriggers(triggers);
    RogueAoePvpStrategy::InitNonCombatTriggers(triggers);
}

void CombatRogueAoeRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueAoeStrategy::InitCombatTriggers(triggers);
    RogueAoeRaidStrategy::InitCombatTriggers(triggers);
}

void CombatRogueAoeRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueAoeStrategy::InitNonCombatTriggers(triggers);
    RogueAoeRaidStrategy::InitNonCombatTriggers(triggers);
}

void CombatRogueBuffStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RogueBuffStrategy::InitCombatTriggers(triggers);
}

void CombatRogueBuffStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RogueBuffStrategy::InitNonCombatTriggers(triggers);
}

void CombatRogueBuffPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueBuffStrategy::InitCombatTriggers(triggers);
    RogueBuffPveStrategy::InitCombatTriggers(triggers);
}

void CombatRogueBuffPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueBuffStrategy::InitNonCombatTriggers(triggers);
    RogueBuffPveStrategy::InitNonCombatTriggers(triggers);
}

void CombatRogueBuffPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueBuffStrategy::InitCombatTriggers(triggers);
    RogueBuffPvpStrategy::InitCombatTriggers(triggers);
}

void CombatRogueBuffPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueBuffStrategy::InitNonCombatTriggers(triggers);
    RogueBuffPvpStrategy::InitNonCombatTriggers(triggers);
}

void CombatRogueBuffRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueBuffStrategy::InitCombatTriggers(triggers);
    RogueBuffRaidStrategy::InitCombatTriggers(triggers);
}

void CombatRogueBuffRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueBuffStrategy::InitNonCombatTriggers(triggers);
    RogueBuffRaidStrategy::InitNonCombatTriggers(triggers);
}

void CombatRogueBoostStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RogueBoostStrategy::InitCombatTriggers(triggers);
}

void CombatRogueBoostStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RogueBoostStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "adrenaline rush",
        NextAction::array(0, new NextAction("adrenaline rush", ACTION_HIGH + 5), NULL)));

    triggers.push_back(new TriggerNode(
        "blade flurry",
        NextAction::array(0, new NextAction("blade flurry", ACTION_HIGH + 4), NULL)));

    triggers.push_back(new TriggerNode(
        "killing spree",
        NextAction::array(0, new NextAction("killing spree", ACTION_HIGH + 4), NULL)));
}

void CombatRogueBoostPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueBoostStrategy::InitCombatTriggers(triggers);
    RogueBoostPveStrategy::InitCombatTriggers(triggers);
}

void CombatRogueBoostPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueBoostStrategy::InitNonCombatTriggers(triggers);
    RogueBoostPveStrategy::InitNonCombatTriggers(triggers);
}

void CombatRogueBoostPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueBoostStrategy::InitCombatTriggers(triggers);
    RogueBoostPvpStrategy::InitCombatTriggers(triggers);
}

void CombatRogueBoostPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueBoostStrategy::InitNonCombatTriggers(triggers);
    RogueBoostPvpStrategy::InitNonCombatTriggers(triggers);
}

void CombatRogueBoostRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueBoostStrategy::InitCombatTriggers(triggers);
    RogueBoostRaidStrategy::InitCombatTriggers(triggers);
}

void CombatRogueBoostRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueBoostStrategy::InitNonCombatTriggers(triggers);
    RogueBoostRaidStrategy::InitNonCombatTriggers(triggers);
}

void CombatRogueCcStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RogueCcStrategy::InitCombatTriggers(triggers);
}

void CombatRogueCcStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RogueCcStrategy::InitNonCombatTriggers(triggers);
}

void CombatRogueCcPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueCcStrategy::InitCombatTriggers(triggers);
    RogueCcPveStrategy::InitCombatTriggers(triggers);
}

void CombatRogueCcPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueCcStrategy::InitNonCombatTriggers(triggers);
    RogueCcPveStrategy::InitNonCombatTriggers(triggers);
}

void CombatRogueCcPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueCcStrategy::InitCombatTriggers(triggers);
    RogueCcPvpStrategy::InitCombatTriggers(triggers);
}

void CombatRogueCcPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueCcStrategy::InitNonCombatTriggers(triggers);
    RogueCcPvpStrategy::InitNonCombatTriggers(triggers);
}

void CombatRogueCcRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueCcStrategy::InitCombatTriggers(triggers);
    RogueCcRaidStrategy::InitCombatTriggers(triggers);
}

void CombatRogueCcRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueCcStrategy::InitNonCombatTriggers(triggers);
    RogueCcRaidStrategy::InitNonCombatTriggers(triggers);
}

void CombatRogueStealthStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RogueStealthStrategy::InitCombatTriggers(triggers);
}

void CombatRogueStealthStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RogueStealthStrategy::InitNonCombatTriggers(triggers);
}

void CombatRogueStealthPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueStealthStrategy::InitCombatTriggers(triggers);
    RogueStealthPveStrategy::InitCombatTriggers(triggers);
}

void CombatRogueStealthPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueStealthStrategy::InitNonCombatTriggers(triggers);
    RogueStealthPveStrategy::InitNonCombatTriggers(triggers);
}

void CombatRogueStealthPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueStealthStrategy::InitCombatTriggers(triggers);
    RogueStealthPvpStrategy::InitCombatTriggers(triggers);
}

void CombatRogueStealthPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueStealthStrategy::InitNonCombatTriggers(triggers);
    RogueStealthPvpStrategy::InitNonCombatTriggers(triggers);
}

void CombatRogueStealthRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueStealthStrategy::InitCombatTriggers(triggers);
    RogueStealthRaidStrategy::InitCombatTriggers(triggers);
}

void CombatRogueStealthRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRogueStealthStrategy::InitNonCombatTriggers(triggers);
    RogueStealthRaidStrategy::InitNonCombatTriggers(triggers);
}

void CombatRoguePoisonsStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RoguePoisonsStrategy::InitCombatTriggers(triggers);
}

void CombatRoguePoisonsStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RoguePoisonsStrategy::InitNonCombatTriggers(triggers);
}

void CombatRoguePoisonsPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRoguePoisonsStrategy::InitCombatTriggers(triggers);
    RoguePoisonsPveStrategy::InitCombatTriggers(triggers);
}

void CombatRoguePoisonsPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRoguePoisonsStrategy::InitNonCombatTriggers(triggers);
    RoguePoisonsPveStrategy::InitNonCombatTriggers(triggers);
}

void CombatRoguePoisonsPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRoguePoisonsStrategy::InitCombatTriggers(triggers);
    RoguePoisonsPvpStrategy::InitCombatTriggers(triggers);
}

void CombatRoguePoisonsPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRoguePoisonsStrategy::InitNonCombatTriggers(triggers);
    RoguePoisonsPvpStrategy::InitNonCombatTriggers(triggers);
}

void CombatRoguePoisonsRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRoguePoisonsStrategy::InitCombatTriggers(triggers);
    RoguePoisonsRaidStrategy::InitCombatTriggers(triggers);
}

void CombatRoguePoisonsRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CombatRoguePoisonsStrategy::InitNonCombatTriggers(triggers);
    RoguePoisonsRaidStrategy::InitNonCombatTriggers(triggers);
}

#endif