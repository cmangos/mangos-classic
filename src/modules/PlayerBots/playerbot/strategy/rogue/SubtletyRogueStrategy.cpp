
#include "playerbot/playerbot.h"
#include "SubtletyRogueStrategy.h"

using namespace ai;

class SubtletyRogueStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    SubtletyRogueStrategyActionNodeFactory()
    {
        creators["hemorrhage back"] = &hemorrhage_back;
        creators["hemorrhage front"] = &hemorrhage_front;
        creators["hemorrhage"] = &hemorrhage;
        creators["backstab"] = &backstab;
    }

private:
    ACTION_NODE_A(hemorrhage_back, "backstab", "hemorrhage");

    ACTION_NODE_A(hemorrhage_front, "hemorrhage", "sinister strike");

    ACTION_NODE_A(hemorrhage, "hemorrhage", "sinister strike");

    ACTION_NODE_A(backstab, "backstab", "sinister strike");
};

SubtletyRogueStrategy::SubtletyRogueStrategy(PlayerbotAI* ai) : RogueStrategy(ai)
{
    actionNodeFactories.Add(new SubtletyRogueStrategyActionNodeFactory());
}

#ifdef MANGOSBOT_ZERO // Vanilla

void SubtletyRogueStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RogueStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "eviscerate",
        NextAction::array(0, new NextAction("eviscerate", ACTION_HIGH + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "slice and dice",
        NextAction::array(0, new NextAction("slice and dice", ACTION_HIGH + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "3 combo",
        NextAction::array(0, new NextAction("rupture", ACTION_HIGH), NULL)));

    triggers.push_back(new TriggerNode(
        "not behind target",
        NextAction::array(0, new NextAction("ghostly strike", ACTION_NORMAL + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "behind target",
        NextAction::array(0, new NextAction("hemorrhage back", ACTION_NORMAL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "not behind target",
        NextAction::array(0, new NextAction("hemorrhage front", ACTION_NORMAL), NULL)));
}

void SubtletyRogueStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RogueStrategy::InitNonCombatTriggers(triggers);
}

void SubtletyRogueStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    RogueStrategy::InitReactionTriggers(triggers);
}

void SubtletyRogueStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    RogueStrategy::InitDeadTriggers(triggers);
}

void SubtletyRoguePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueStrategy::InitCombatTriggers(triggers);
    RoguePveStrategy::InitCombatTriggers(triggers);
}

void SubtletyRoguePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueStrategy::InitNonCombatTriggers(triggers);
    RoguePveStrategy::InitNonCombatTriggers(triggers);
}

void SubtletyRoguePveStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueStrategy::InitReactionTriggers(triggers);
    RoguePveStrategy::InitReactionTriggers(triggers);
}

void SubtletyRoguePveStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueStrategy::InitDeadTriggers(triggers);
    RoguePveStrategy::InitDeadTriggers(triggers);
}

void SubtletyRoguePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueStrategy::InitCombatTriggers(triggers);
    RoguePvpStrategy::InitCombatTriggers(triggers);
}

void SubtletyRoguePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueStrategy::InitNonCombatTriggers(triggers);
    RoguePvpStrategy::InitNonCombatTriggers(triggers);
}

void SubtletyRoguePvpStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueStrategy::InitReactionTriggers(triggers);
    RoguePvpStrategy::InitReactionTriggers(triggers);
}

void SubtletyRoguePvpStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueStrategy::InitDeadTriggers(triggers);
    RoguePvpStrategy::InitDeadTriggers(triggers);
}

void SubtletyRogueRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueStrategy::InitCombatTriggers(triggers);
    RogueRaidStrategy::InitCombatTriggers(triggers);
}

void SubtletyRogueRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueStrategy::InitNonCombatTriggers(triggers);
    RogueRaidStrategy::InitNonCombatTriggers(triggers);
}

void SubtletyRogueRaidStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueStrategy::InitReactionTriggers(triggers);
    RogueRaidStrategy::InitReactionTriggers(triggers);
}

void SubtletyRogueRaidStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueStrategy::InitDeadTriggers(triggers);
    RogueRaidStrategy::InitDeadTriggers(triggers);
}

void SubtletyRogueAoeStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RogueAoeStrategy::InitCombatTriggers(triggers);
}

void SubtletyRogueAoeStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RogueAoeStrategy::InitNonCombatTriggers(triggers);
}

void SubtletyRogueAoePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueAoeStrategy::InitCombatTriggers(triggers);
    RogueAoePveStrategy::InitCombatTriggers(triggers);
}

void SubtletyRogueAoePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueAoeStrategy::InitNonCombatTriggers(triggers);
    RogueAoePveStrategy::InitNonCombatTriggers(triggers);
}

void SubtletyRogueAoePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueAoeStrategy::InitCombatTriggers(triggers);
    RogueAoePvpStrategy::InitCombatTriggers(triggers);
}

void SubtletyRogueAoePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueAoeStrategy::InitNonCombatTriggers(triggers);
    RogueAoePvpStrategy::InitNonCombatTriggers(triggers);
}

void SubtletyRogueAoeRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueAoeStrategy::InitCombatTriggers(triggers);
    RogueAoeRaidStrategy::InitCombatTriggers(triggers);
}

void SubtletyRogueAoeRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueAoeStrategy::InitNonCombatTriggers(triggers);
    RogueAoeRaidStrategy::InitNonCombatTriggers(triggers);
}

void SubtletyRogueBuffStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RogueBuffStrategy::InitCombatTriggers(triggers);
}

void SubtletyRogueBuffStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RogueBuffStrategy::InitNonCombatTriggers(triggers);
}

void SubtletyRogueBuffPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueBuffStrategy::InitCombatTriggers(triggers);
    RogueBuffPveStrategy::InitCombatTriggers(triggers);
}

void SubtletyRogueBuffPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueBuffStrategy::InitNonCombatTriggers(triggers);
    RogueBuffPveStrategy::InitNonCombatTriggers(triggers);
}

void SubtletyRogueBuffPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueBuffStrategy::InitCombatTriggers(triggers);
    RogueBuffPvpStrategy::InitCombatTriggers(triggers);
}

void SubtletyRogueBuffPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueBuffStrategy::InitNonCombatTriggers(triggers);
    RogueBuffPvpStrategy::InitNonCombatTriggers(triggers);
}

void SubtletyRogueBuffRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueBuffStrategy::InitCombatTriggers(triggers);
    RogueBuffRaidStrategy::InitCombatTriggers(triggers);
}

void SubtletyRogueBuffRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueBuffStrategy::InitNonCombatTriggers(triggers);
    RogueBuffRaidStrategy::InitNonCombatTriggers(triggers);
}

void SubtletyRogueBoostStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RogueBoostStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "very often",
        NextAction::array(0, new NextAction("preparation", ACTION_HIGH + 3), NULL)));
}

void SubtletyRogueBoostStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RogueBoostStrategy::InitNonCombatTriggers(triggers);
}

void SubtletyRogueBoostPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueBoostStrategy::InitCombatTriggers(triggers);
    RogueBoostPveStrategy::InitCombatTriggers(triggers);
}

void SubtletyRogueBoostPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueBoostStrategy::InitNonCombatTriggers(triggers);
    RogueBoostPveStrategy::InitNonCombatTriggers(triggers);
}

void SubtletyRogueBoostPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueBoostStrategy::InitCombatTriggers(triggers);
    RogueBoostPvpStrategy::InitCombatTriggers(triggers);
}

void SubtletyRogueBoostPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueBoostStrategy::InitNonCombatTriggers(triggers);
    RogueBoostPvpStrategy::InitNonCombatTriggers(triggers);
}

void SubtletyRogueBoostRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueBoostStrategy::InitCombatTriggers(triggers);
    RogueBoostRaidStrategy::InitCombatTriggers(triggers);
}

void SubtletyRogueBoostRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueBoostStrategy::InitNonCombatTriggers(triggers);
    RogueBoostRaidStrategy::InitNonCombatTriggers(triggers);
}

void SubtletyRogueCcStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RogueCcStrategy::InitCombatTriggers(triggers);
}

void SubtletyRogueCcStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RogueCcStrategy::InitNonCombatTriggers(triggers);
}

void SubtletyRogueCcPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueCcStrategy::InitCombatTriggers(triggers);
    RogueCcPveStrategy::InitCombatTriggers(triggers);
}

void SubtletyRogueCcPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueCcStrategy::InitNonCombatTriggers(triggers);
    RogueCcPveStrategy::InitNonCombatTriggers(triggers);
}

void SubtletyRogueCcPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueCcStrategy::InitCombatTriggers(triggers);
    RogueCcPvpStrategy::InitCombatTriggers(triggers);
}

void SubtletyRogueCcPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueCcStrategy::InitNonCombatTriggers(triggers);
    RogueCcPvpStrategy::InitNonCombatTriggers(triggers);
}

void SubtletyRogueCcRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueCcStrategy::InitCombatTriggers(triggers);
    RogueCcRaidStrategy::InitCombatTriggers(triggers);
}

void SubtletyRogueCcRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueCcStrategy::InitNonCombatTriggers(triggers);
    RogueCcRaidStrategy::InitNonCombatTriggers(triggers);
}

void SubtletyRogueStealthStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RogueStealthStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "very often",
        NextAction::array(0, new NextAction("premeditation", ACTION_HIGH + 1), NULL)));
}

void SubtletyRogueStealthStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RogueStealthStrategy::InitNonCombatTriggers(triggers);
}

void SubtletyRogueStealthPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueStealthStrategy::InitCombatTriggers(triggers);
    RogueStealthPveStrategy::InitCombatTriggers(triggers);
}

void SubtletyRogueStealthPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueStealthStrategy::InitNonCombatTriggers(triggers);
    RogueStealthPveStrategy::InitNonCombatTriggers(triggers);
}

void SubtletyRogueStealthPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueStealthStrategy::InitCombatTriggers(triggers);
    RogueStealthPvpStrategy::InitCombatTriggers(triggers);
}

void SubtletyRogueStealthPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueStealthStrategy::InitNonCombatTriggers(triggers);
    RogueStealthPvpStrategy::InitNonCombatTriggers(triggers);
}

void SubtletyRogueStealthRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueStealthStrategy::InitCombatTriggers(triggers);
    RogueStealthRaidStrategy::InitCombatTriggers(triggers);
}

void SubtletyRogueStealthRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueStealthStrategy::InitNonCombatTriggers(triggers);
    RogueStealthRaidStrategy::InitNonCombatTriggers(triggers);
}

void SubtletyRoguePoisonsStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RoguePoisonsStrategy::InitCombatTriggers(triggers);
}

void SubtletyRoguePoisonsStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RoguePoisonsStrategy::InitNonCombatTriggers(triggers);
}

void SubtletyRoguePoisonsPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRoguePoisonsStrategy::InitCombatTriggers(triggers);
    RoguePoisonsPveStrategy::InitCombatTriggers(triggers);
}

void SubtletyRoguePoisonsPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRoguePoisonsStrategy::InitNonCombatTriggers(triggers);
    RoguePoisonsPveStrategy::InitNonCombatTriggers(triggers);
}

void SubtletyRoguePoisonsPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRoguePoisonsStrategy::InitCombatTriggers(triggers);
    RoguePoisonsPvpStrategy::InitCombatTriggers(triggers);
}

void SubtletyRoguePoisonsPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRoguePoisonsStrategy::InitNonCombatTriggers(triggers);
    RoguePoisonsPvpStrategy::InitNonCombatTriggers(triggers);
}

void SubtletyRoguePoisonsRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRoguePoisonsStrategy::InitCombatTriggers(triggers);
    RoguePoisonsRaidStrategy::InitCombatTriggers(triggers);
}

void SubtletyRoguePoisonsRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRoguePoisonsStrategy::InitNonCombatTriggers(triggers);
    RoguePoisonsRaidStrategy::InitNonCombatTriggers(triggers);
}

#endif
#ifdef MANGOSBOT_ONE // TBC

void SubtletyRogueStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RogueStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "enemy out of melee",
        NextAction::array(0, new NextAction("shadowstep", ACTION_HIGH + 3), NULL)));

    triggers.push_back(new TriggerNode(
        "eviscerate",
        NextAction::array(0, new NextAction("eviscerate", ACTION_HIGH + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "slice and dice",
        NextAction::array(0, new NextAction("slice and dice", ACTION_HIGH + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "3 combo",
        NextAction::array(0, new NextAction("rupture", ACTION_HIGH), NULL)));

    triggers.push_back(new TriggerNode(
        "not behind target",
        NextAction::array(0, new NextAction("ghostly strike", ACTION_NORMAL + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "behind target",
        NextAction::array(0, new NextAction("hemorrhage back", ACTION_NORMAL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "not behind target",
        NextAction::array(0, new NextAction("hemorrhage front", ACTION_NORMAL), NULL)));
}

void SubtletyRogueStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RogueStrategy::InitNonCombatTriggers(triggers);
}

void SubtletyRogueStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    RogueStrategy::InitReactionTriggers(triggers);
}

void SubtletyRogueStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    RogueStrategy::InitDeadTriggers(triggers);
}

void SubtletyRoguePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueStrategy::InitCombatTriggers(triggers);
    RoguePveStrategy::InitCombatTriggers(triggers);
}

void SubtletyRoguePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueStrategy::InitNonCombatTriggers(triggers);
    RoguePveStrategy::InitNonCombatTriggers(triggers);
}

void SubtletyRoguePveStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueStrategy::InitReactionTriggers(triggers);
    RoguePveStrategy::InitReactionTriggers(triggers);
}

void SubtletyRoguePveStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueStrategy::InitDeadTriggers(triggers);
    RoguePveStrategy::InitDeadTriggers(triggers);
}

void SubtletyRoguePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueStrategy::InitCombatTriggers(triggers);
    RoguePvpStrategy::InitCombatTriggers(triggers);
}

void SubtletyRoguePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueStrategy::InitNonCombatTriggers(triggers);
    RoguePvpStrategy::InitNonCombatTriggers(triggers);
}

void SubtletyRoguePvpStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueStrategy::InitReactionTriggers(triggers);
    RoguePvpStrategy::InitReactionTriggers(triggers);
}

void SubtletyRoguePvpStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueStrategy::InitDeadTriggers(triggers);
    RoguePvpStrategy::InitDeadTriggers(triggers);
}

void SubtletyRogueRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueStrategy::InitCombatTriggers(triggers);
    RogueRaidStrategy::InitCombatTriggers(triggers);
}

void SubtletyRogueRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueStrategy::InitNonCombatTriggers(triggers);
    RogueRaidStrategy::InitNonCombatTriggers(triggers);
}

void SubtletyRogueRaidStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueStrategy::InitReactionTriggers(triggers);
    RogueRaidStrategy::InitReactionTriggers(triggers);
}

void SubtletyRogueRaidStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueStrategy::InitDeadTriggers(triggers);
    RogueRaidStrategy::InitDeadTriggers(triggers);
}

void SubtletyRogueAoeStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RogueAoeStrategy::InitCombatTriggers(triggers);
}

void SubtletyRogueAoeStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RogueAoeStrategy::InitNonCombatTriggers(triggers);
}

void SubtletyRogueAoePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueAoeStrategy::InitCombatTriggers(triggers);
    RogueAoePveStrategy::InitCombatTriggers(triggers);
}

void SubtletyRogueAoePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueAoeStrategy::InitNonCombatTriggers(triggers);
    RogueAoePveStrategy::InitNonCombatTriggers(triggers);
}

void SubtletyRogueAoePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueAoeStrategy::InitCombatTriggers(triggers);
    RogueAoePvpStrategy::InitCombatTriggers(triggers);
}

void SubtletyRogueAoePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueAoeStrategy::InitNonCombatTriggers(triggers);
    RogueAoePvpStrategy::InitNonCombatTriggers(triggers);
}

void SubtletyRogueAoeRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueAoeStrategy::InitCombatTriggers(triggers);
    RogueAoeRaidStrategy::InitCombatTriggers(triggers);
}

void SubtletyRogueAoeRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueAoeStrategy::InitNonCombatTriggers(triggers);
    RogueAoeRaidStrategy::InitNonCombatTriggers(triggers);
}

void SubtletyRogueBuffStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RogueBuffStrategy::InitCombatTriggers(triggers);
}

void SubtletyRogueBuffStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RogueBuffStrategy::InitNonCombatTriggers(triggers);
}

void SubtletyRogueBuffPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueBuffStrategy::InitCombatTriggers(triggers);
    RogueBuffPveStrategy::InitCombatTriggers(triggers);
}

void SubtletyRogueBuffPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueBuffStrategy::InitNonCombatTriggers(triggers);
    RogueBuffPveStrategy::InitNonCombatTriggers(triggers);
}

void SubtletyRogueBuffPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueBuffStrategy::InitCombatTriggers(triggers);
    RogueBuffPvpStrategy::InitCombatTriggers(triggers);
}

void SubtletyRogueBuffPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueBuffStrategy::InitNonCombatTriggers(triggers);
    RogueBuffPvpStrategy::InitNonCombatTriggers(triggers);
}

void SubtletyRogueBuffRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueBuffStrategy::InitCombatTriggers(triggers);
    RogueBuffRaidStrategy::InitCombatTriggers(triggers);
}

void SubtletyRogueBuffRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueBuffStrategy::InitNonCombatTriggers(triggers);
    RogueBuffRaidStrategy::InitNonCombatTriggers(triggers);
}

void SubtletyRogueBoostStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RogueBoostStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "very often",
        NextAction::array(0, new NextAction("preparation", ACTION_HIGH + 3), NULL)));
}

void SubtletyRogueBoostStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RogueBoostStrategy::InitNonCombatTriggers(triggers);
}

void SubtletyRogueBoostPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueBoostStrategy::InitCombatTriggers(triggers);
    RogueBoostPveStrategy::InitCombatTriggers(triggers);
}

void SubtletyRogueBoostPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueBoostStrategy::InitNonCombatTriggers(triggers);
    RogueBoostPveStrategy::InitNonCombatTriggers(triggers);
}

void SubtletyRogueBoostPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueBoostStrategy::InitCombatTriggers(triggers);
    RogueBoostPvpStrategy::InitCombatTriggers(triggers);
}

void SubtletyRogueBoostPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueBoostStrategy::InitNonCombatTriggers(triggers);
    RogueBoostPvpStrategy::InitNonCombatTriggers(triggers);
}

void SubtletyRogueBoostRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueBoostStrategy::InitCombatTriggers(triggers);
    RogueBoostRaidStrategy::InitCombatTriggers(triggers);
}

void SubtletyRogueBoostRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueBoostStrategy::InitNonCombatTriggers(triggers);
    RogueBoostRaidStrategy::InitNonCombatTriggers(triggers);
}

void SubtletyRogueCcStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RogueCcStrategy::InitCombatTriggers(triggers);
}

void SubtletyRogueCcStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RogueCcStrategy::InitNonCombatTriggers(triggers);
}

void SubtletyRogueCcPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueCcStrategy::InitCombatTriggers(triggers);
    RogueCcPveStrategy::InitCombatTriggers(triggers);
}

void SubtletyRogueCcPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueCcStrategy::InitNonCombatTriggers(triggers);
    RogueCcPveStrategy::InitNonCombatTriggers(triggers);
}

void SubtletyRogueCcPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueCcStrategy::InitCombatTriggers(triggers);
    RogueCcPvpStrategy::InitCombatTriggers(triggers);
}

void SubtletyRogueCcPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueCcStrategy::InitNonCombatTriggers(triggers);
    RogueCcPvpStrategy::InitNonCombatTriggers(triggers);
}

void SubtletyRogueCcRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueCcStrategy::InitCombatTriggers(triggers);
    RogueCcRaidStrategy::InitCombatTriggers(triggers);
}

void SubtletyRogueCcRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueCcStrategy::InitNonCombatTriggers(triggers);
    RogueCcRaidStrategy::InitNonCombatTriggers(triggers);
}

void SubtletyRogueStealthStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RogueStealthStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "enemy out of melee",
        NextAction::array(0, new NextAction("shadowstep", ACTION_HIGH + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "very often",
        NextAction::array(0, new NextAction("premeditation", ACTION_HIGH + 1), NULL)));
}

void SubtletyRogueStealthStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RogueStealthStrategy::InitNonCombatTriggers(triggers);
}

void SubtletyRogueStealthPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueStealthStrategy::InitCombatTriggers(triggers);
    RogueStealthPveStrategy::InitCombatTriggers(triggers);
}

void SubtletyRogueStealthPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueStealthStrategy::InitNonCombatTriggers(triggers);
    RogueStealthPveStrategy::InitNonCombatTriggers(triggers);
}

void SubtletyRogueStealthPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueStealthStrategy::InitCombatTriggers(triggers);
    RogueStealthPvpStrategy::InitCombatTriggers(triggers);
}

void SubtletyRogueStealthPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueStealthStrategy::InitNonCombatTriggers(triggers);
    RogueStealthPvpStrategy::InitNonCombatTriggers(triggers);
}

void SubtletyRogueStealthRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueStealthStrategy::InitCombatTriggers(triggers);
    RogueStealthRaidStrategy::InitCombatTriggers(triggers);
}

void SubtletyRogueStealthRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueStealthStrategy::InitNonCombatTriggers(triggers);
    RogueStealthRaidStrategy::InitNonCombatTriggers(triggers);
}

void SubtletyRoguePoisonsStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RoguePoisonsStrategy::InitCombatTriggers(triggers);
}

void SubtletyRoguePoisonsStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RoguePoisonsStrategy::InitNonCombatTriggers(triggers);
}

void SubtletyRoguePoisonsPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRoguePoisonsStrategy::InitCombatTriggers(triggers);
    RoguePoisonsPveStrategy::InitCombatTriggers(triggers);
}

void SubtletyRoguePoisonsPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRoguePoisonsStrategy::InitNonCombatTriggers(triggers);
    RoguePoisonsPveStrategy::InitNonCombatTriggers(triggers);
}

void SubtletyRoguePoisonsPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRoguePoisonsStrategy::InitCombatTriggers(triggers);
    RoguePoisonsPvpStrategy::InitCombatTriggers(triggers);
}

void SubtletyRoguePoisonsPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRoguePoisonsStrategy::InitNonCombatTriggers(triggers);
    RoguePoisonsPvpStrategy::InitNonCombatTriggers(triggers);
}

void SubtletyRoguePoisonsRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRoguePoisonsStrategy::InitCombatTriggers(triggers);
    RoguePoisonsRaidStrategy::InitCombatTriggers(triggers);
}

void SubtletyRoguePoisonsRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRoguePoisonsStrategy::InitNonCombatTriggers(triggers);
    RoguePoisonsRaidStrategy::InitNonCombatTriggers(triggers);
}

#endif
#ifdef MANGOSBOT_TWO // WOTLK

void SubtletyRogueStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RogueStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "enemy out of melee",
        NextAction::array(0, new NextAction("shadowstep", ACTION_HIGH + 3), NULL)));

    triggers.push_back(new TriggerNode(
        "eviscerate",
        NextAction::array(0, new NextAction("eviscerate", ACTION_HIGH + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "slice and dice",
        NextAction::array(0, new NextAction("slice and dice", ACTION_HIGH + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "3 combo",
        NextAction::array(0, new NextAction("rupture", ACTION_HIGH), NULL)));

    triggers.push_back(new TriggerNode(
        "not behind target",
        NextAction::array(0, new NextAction("ghostly strike", ACTION_NORMAL + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "behind target",
        NextAction::array(0, new NextAction("hemorrhage back", ACTION_NORMAL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "not behind target",
        NextAction::array(0, new NextAction("hemorrhage front", ACTION_NORMAL), NULL)));
}

void SubtletyRogueStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RogueStrategy::InitNonCombatTriggers(triggers);
}

void SubtletyRogueStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    RogueStrategy::InitReactionTriggers(triggers);
}

void SubtletyRogueStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    RogueStrategy::InitDeadTriggers(triggers);
}

void SubtletyRoguePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueStrategy::InitCombatTriggers(triggers);
    RoguePveStrategy::InitCombatTriggers(triggers);
}

void SubtletyRoguePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueStrategy::InitNonCombatTriggers(triggers);
    RoguePveStrategy::InitNonCombatTriggers(triggers);
}

void SubtletyRoguePveStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueStrategy::InitReactionTriggers(triggers);
    RoguePveStrategy::InitReactionTriggers(triggers);
}

void SubtletyRoguePveStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueStrategy::InitDeadTriggers(triggers);
    RoguePveStrategy::InitDeadTriggers(triggers);
}

void SubtletyRoguePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueStrategy::InitCombatTriggers(triggers);
    RoguePvpStrategy::InitCombatTriggers(triggers);
}

void SubtletyRoguePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueStrategy::InitNonCombatTriggers(triggers);
    RoguePvpStrategy::InitNonCombatTriggers(triggers);
}

void SubtletyRoguePvpStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueStrategy::InitReactionTriggers(triggers);
    RoguePvpStrategy::InitReactionTriggers(triggers);
}

void SubtletyRoguePvpStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueStrategy::InitDeadTriggers(triggers);
    RoguePvpStrategy::InitDeadTriggers(triggers);
}

void SubtletyRogueRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueStrategy::InitCombatTriggers(triggers);
    RogueRaidStrategy::InitCombatTriggers(triggers);
}

void SubtletyRogueRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueStrategy::InitNonCombatTriggers(triggers);
    RogueRaidStrategy::InitNonCombatTriggers(triggers);
}

void SubtletyRogueRaidStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueStrategy::InitReactionTriggers(triggers);
    RogueRaidStrategy::InitReactionTriggers(triggers);
}

void SubtletyRogueRaidStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueStrategy::InitDeadTriggers(triggers);
    RogueRaidStrategy::InitDeadTriggers(triggers);
}

void SubtletyRogueAoeStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RogueAoeStrategy::InitCombatTriggers(triggers);
}

void SubtletyRogueAoeStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RogueAoeStrategy::InitNonCombatTriggers(triggers);
}

void SubtletyRogueAoePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueAoeStrategy::InitCombatTriggers(triggers);
    RogueAoePveStrategy::InitCombatTriggers(triggers);
}

void SubtletyRogueAoePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueAoeStrategy::InitNonCombatTriggers(triggers);
    RogueAoePveStrategy::InitNonCombatTriggers(triggers);
}

void SubtletyRogueAoePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueAoeStrategy::InitCombatTriggers(triggers);
    RogueAoePvpStrategy::InitCombatTriggers(triggers);
}

void SubtletyRogueAoePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueAoeStrategy::InitNonCombatTriggers(triggers);
    RogueAoePvpStrategy::InitNonCombatTriggers(triggers);
}

void SubtletyRogueAoeRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueAoeStrategy::InitCombatTriggers(triggers);
    RogueAoeRaidStrategy::InitCombatTriggers(triggers);
}

void SubtletyRogueAoeRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueAoeStrategy::InitNonCombatTriggers(triggers);
    RogueAoeRaidStrategy::InitNonCombatTriggers(triggers);
}

void SubtletyRogueBuffStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RogueBuffStrategy::InitCombatTriggers(triggers);
}

void SubtletyRogueBuffStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RogueBuffStrategy::InitNonCombatTriggers(triggers);
}

void SubtletyRogueBuffPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueBuffStrategy::InitCombatTriggers(triggers);
    RogueBuffPveStrategy::InitCombatTriggers(triggers);
}

void SubtletyRogueBuffPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueBuffStrategy::InitNonCombatTriggers(triggers);
    RogueBuffPveStrategy::InitNonCombatTriggers(triggers);
}

void SubtletyRogueBuffPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueBuffStrategy::InitCombatTriggers(triggers);
    RogueBuffPvpStrategy::InitCombatTriggers(triggers);
}

void SubtletyRogueBuffPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueBuffStrategy::InitNonCombatTriggers(triggers);
    RogueBuffPvpStrategy::InitNonCombatTriggers(triggers);
}

void SubtletyRogueBuffRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueBuffStrategy::InitCombatTriggers(triggers);
    RogueBuffRaidStrategy::InitCombatTriggers(triggers);
}

void SubtletyRogueBuffRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueBuffStrategy::InitNonCombatTriggers(triggers);
    RogueBuffRaidStrategy::InitNonCombatTriggers(triggers);
}

void SubtletyRogueBoostStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RogueBoostStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "very often",
        NextAction::array(0, new NextAction("preparation", ACTION_HIGH + 3), NULL)));
}

void SubtletyRogueBoostStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RogueBoostStrategy::InitNonCombatTriggers(triggers);
}

void SubtletyRogueBoostPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueBoostStrategy::InitCombatTriggers(triggers);
    RogueBoostPveStrategy::InitCombatTriggers(triggers);
}

void SubtletyRogueBoostPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueBoostStrategy::InitNonCombatTriggers(triggers);
    RogueBoostPveStrategy::InitNonCombatTriggers(triggers);
}

void SubtletyRogueBoostPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueBoostStrategy::InitCombatTriggers(triggers);
    RogueBoostPvpStrategy::InitCombatTriggers(triggers);
}

void SubtletyRogueBoostPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueBoostStrategy::InitNonCombatTriggers(triggers);
    RogueBoostPvpStrategy::InitNonCombatTriggers(triggers);
}

void SubtletyRogueBoostRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueBoostStrategy::InitCombatTriggers(triggers);
    RogueBoostRaidStrategy::InitCombatTriggers(triggers);
}

void SubtletyRogueBoostRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueBoostStrategy::InitNonCombatTriggers(triggers);
    RogueBoostRaidStrategy::InitNonCombatTriggers(triggers);
}

void SubtletyRogueCcStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RogueCcStrategy::InitCombatTriggers(triggers);
}

void SubtletyRogueCcStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RogueCcStrategy::InitNonCombatTriggers(triggers);
}

void SubtletyRogueCcPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueCcStrategy::InitCombatTriggers(triggers);
    RogueCcPveStrategy::InitCombatTriggers(triggers);
}

void SubtletyRogueCcPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueCcStrategy::InitNonCombatTriggers(triggers);
    RogueCcPveStrategy::InitNonCombatTriggers(triggers);
}

void SubtletyRogueCcPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueCcStrategy::InitCombatTriggers(triggers);
    RogueCcPvpStrategy::InitCombatTriggers(triggers);
}

void SubtletyRogueCcPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueCcStrategy::InitNonCombatTriggers(triggers);
    RogueCcPvpStrategy::InitNonCombatTriggers(triggers);
}

void SubtletyRogueCcRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueCcStrategy::InitCombatTriggers(triggers);
    RogueCcRaidStrategy::InitCombatTriggers(triggers);
}

void SubtletyRogueCcRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueCcStrategy::InitNonCombatTriggers(triggers);
    RogueCcRaidStrategy::InitNonCombatTriggers(triggers);
}

void SubtletyRogueStealthStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RogueStealthStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "enemy out of melee",
        NextAction::array(0, new NextAction("shadowstep", ACTION_HIGH + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "very often",
        NextAction::array(0, new NextAction("premeditation", ACTION_HIGH + 1), NULL)));
}

void SubtletyRogueStealthStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RogueStealthStrategy::InitNonCombatTriggers(triggers);
}

void SubtletyRogueStealthPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueStealthStrategy::InitCombatTriggers(triggers);
    RogueStealthPveStrategy::InitCombatTriggers(triggers);
}

void SubtletyRogueStealthPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueStealthStrategy::InitNonCombatTriggers(triggers);
    RogueStealthPveStrategy::InitNonCombatTriggers(triggers);
}

void SubtletyRogueStealthPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueStealthStrategy::InitCombatTriggers(triggers);
    RogueStealthPvpStrategy::InitCombatTriggers(triggers);
}

void SubtletyRogueStealthPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueStealthStrategy::InitNonCombatTriggers(triggers);
    RogueStealthPvpStrategy::InitNonCombatTriggers(triggers);
}

void SubtletyRogueStealthRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueStealthStrategy::InitCombatTriggers(triggers);
    RogueStealthRaidStrategy::InitCombatTriggers(triggers);
}

void SubtletyRogueStealthRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRogueStealthStrategy::InitNonCombatTriggers(triggers);
    RogueStealthRaidStrategy::InitNonCombatTriggers(triggers);
}

void SubtletyRoguePoisonsStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RoguePoisonsStrategy::InitCombatTriggers(triggers);
}

void SubtletyRoguePoisonsStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RoguePoisonsStrategy::InitNonCombatTriggers(triggers);
}

void SubtletyRoguePoisonsPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRoguePoisonsStrategy::InitCombatTriggers(triggers);
    RoguePoisonsPveStrategy::InitCombatTriggers(triggers);
}

void SubtletyRoguePoisonsPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRoguePoisonsStrategy::InitNonCombatTriggers(triggers);
    RoguePoisonsPveStrategy::InitNonCombatTriggers(triggers);
}

void SubtletyRoguePoisonsPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRoguePoisonsStrategy::InitCombatTriggers(triggers);
    RoguePoisonsPvpStrategy::InitCombatTriggers(triggers);
}

void SubtletyRoguePoisonsPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRoguePoisonsStrategy::InitNonCombatTriggers(triggers);
    RoguePoisonsPvpStrategy::InitNonCombatTriggers(triggers);
}

void SubtletyRoguePoisonsRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRoguePoisonsStrategy::InitCombatTriggers(triggers);
    RoguePoisonsRaidStrategy::InitCombatTriggers(triggers);
}

void SubtletyRoguePoisonsRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SubtletyRoguePoisonsStrategy::InitNonCombatTriggers(triggers);
    RoguePoisonsRaidStrategy::InitNonCombatTriggers(triggers);
}

#endif