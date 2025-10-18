#include "playerbot/playerbot.h"
#include "PriestMultipliers.h"
#include "HolyPriestStrategy.h"

using namespace ai;

class HolyPriestStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    HolyPriestStrategyActionNodeFactory()
    {
        creators["lightwell"] = &lightwell;
        creators["circle of healing"] = &circle_of_healing;
    }

private:
    ACTION_NODE_A(lightwell, "lightwell", "circle of healing");

    ACTION_NODE_A(circle_of_healing, "circle of healing", "prayer of healing");
};

HolyPriestStrategy::HolyPriestStrategy(PlayerbotAI* ai) : PriestStrategy(ai)
{
    actionNodeFactories.Add(new HolyPriestStrategyActionNodeFactory());
}

#ifdef MANGOSBOT_ZERO // Vanilla

void HolyPriestStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "party member critical health",
        NextAction::array(0, new NextAction("power word: shield on party", ACTION_CRITICAL_HEAL + 1),
                             new NextAction("flash heal on party", ACTION_CRITICAL_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "party member low health",
        NextAction::array(0, new NextAction("power word: shield on party", ACTION_MEDIUM_HEAL + 2),
                             new NextAction("greater heal on party", ACTION_MEDIUM_HEAL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "party member medium health",
        NextAction::array(0, new NextAction("greater heal on party", ACTION_MEDIUM_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "party member almost full health",
        NextAction::array(0, new NextAction("renew on party", ACTION_LIGHT_HEAL), NULL)));
}

void HolyPriestStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestStrategy::InitNonCombatTriggers(triggers);
}

void HolyPriestStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    PriestStrategy::InitReactionTriggers(triggers);
}

void HolyPriestStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    PriestStrategy::InitDeadTriggers(triggers);
}

void HolyPriestPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestStrategy::InitCombatTriggers(triggers);
    PriestPveStrategy::InitCombatTriggers(triggers);
}

void HolyPriestPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestStrategy::InitNonCombatTriggers(triggers);
    PriestPveStrategy::InitNonCombatTriggers(triggers);
}

void HolyPriestPveStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestStrategy::InitReactionTriggers(triggers);
    PriestPveStrategy::InitReactionTriggers(triggers);
}

void HolyPriestPveStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestStrategy::InitDeadTriggers(triggers);
    PriestPveStrategy::InitDeadTriggers(triggers);
}

void HolyPriestPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestStrategy::InitCombatTriggers(triggers);
    PriestPvpStrategy::InitCombatTriggers(triggers);
}

void HolyPriestPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestStrategy::InitNonCombatTriggers(triggers);
    PriestPvpStrategy::InitNonCombatTriggers(triggers);
}

void HolyPriestPvpStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestStrategy::InitReactionTriggers(triggers);
    PriestPvpStrategy::InitReactionTriggers(triggers);
}

void HolyPriestPvpStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestStrategy::InitDeadTriggers(triggers);
    PriestPvpStrategy::InitDeadTriggers(triggers);
}

void HolyPriestRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestStrategy::InitCombatTriggers(triggers);
    PriestRaidStrategy::InitCombatTriggers(triggers);
}

void HolyPriestRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestStrategy::InitNonCombatTriggers(triggers);
    PriestRaidStrategy::InitNonCombatTriggers(triggers);
}

void HolyPriestRaidStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestStrategy::InitReactionTriggers(triggers);
    PriestRaidStrategy::InitReactionTriggers(triggers);
}

void HolyPriestRaidStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestStrategy::InitDeadTriggers(triggers);
    PriestRaidStrategy::InitDeadTriggers(triggers);
}

void HolyPriestAoeStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestAoeStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "medium aoe heal",
        NextAction::array(0, new NextAction("circle of healing", ACTION_MEDIUM_HEAL), NULL)));
}

void HolyPriestAoeStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestAoeStrategy::InitNonCombatTriggers(triggers);
}

void HolyPriestAoePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestAoeStrategy::InitCombatTriggers(triggers);
    PriestAoePveStrategy::InitCombatTriggers(triggers);
}

void HolyPriestAoePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestAoeStrategy::InitNonCombatTriggers(triggers);
    PriestAoePveStrategy::InitNonCombatTriggers(triggers);
}

void HolyPriestAoePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestAoeStrategy::InitCombatTriggers(triggers);
    PriestAoePvpStrategy::InitCombatTriggers(triggers);
}

void HolyPriestAoePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestAoeStrategy::InitNonCombatTriggers(triggers);
    PriestAoePvpStrategy::InitNonCombatTriggers(triggers);
}

void HolyPriestAoeRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestAoeStrategy::InitCombatTriggers(triggers);
    PriestAoeRaidStrategy::InitCombatTriggers(triggers);
}

void HolyPriestAoeRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestAoeStrategy::InitNonCombatTriggers(triggers);
    PriestAoeRaidStrategy::InitNonCombatTriggers(triggers);
}

void HolyPriestBuffStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestBuffStrategy::InitCombatTriggers(triggers);
}

void HolyPriestBuffStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestBuffStrategy::InitNonCombatTriggers(triggers);
}

void HolyPriestBuffPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestBuffStrategy::InitCombatTriggers(triggers);
    PriestBuffPveStrategy::InitCombatTriggers(triggers);
}

void HolyPriestBuffPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestBuffStrategy::InitNonCombatTriggers(triggers);
    PriestBuffPveStrategy::InitNonCombatTriggers(triggers);
}

void HolyPriestBuffPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestBuffStrategy::InitCombatTriggers(triggers);
    PriestBuffPvpStrategy::InitCombatTriggers(triggers);
}

void HolyPriestBuffPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestBuffStrategy::InitNonCombatTriggers(triggers);
    PriestBuffPvpStrategy::InitNonCombatTriggers(triggers);
}

void HolyPriestBuffRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestBuffStrategy::InitCombatTriggers(triggers);
    PriestBuffRaidStrategy::InitCombatTriggers(triggers);
}

void HolyPriestBuffRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestBuffStrategy::InitNonCombatTriggers(triggers);
    PriestBuffRaidStrategy::InitNonCombatTriggers(triggers);
}

void HolyPriestBoostStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestBoostStrategy::InitCombatTriggers(triggers);
}

void HolyPriestBoostStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestBoostStrategy::InitNonCombatTriggers(triggers);
}

void HolyPriestBoostPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestBoostStrategy::InitCombatTriggers(triggers);
    PriestBoostPveStrategy::InitCombatTriggers(triggers);
}

void HolyPriestBoostPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestBoostStrategy::InitNonCombatTriggers(triggers);
    PriestBoostPveStrategy::InitNonCombatTriggers(triggers);
}

void HolyPriestBoostPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestBoostStrategy::InitCombatTriggers(triggers);
    PriestBoostPvpStrategy::InitCombatTriggers(triggers);
}

void HolyPriestBoostPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestBoostStrategy::InitNonCombatTriggers(triggers);
    PriestBoostPvpStrategy::InitNonCombatTriggers(triggers);
}

void HolyPriestBoostRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestBoostStrategy::InitCombatTriggers(triggers);
    PriestBoostRaidStrategy::InitCombatTriggers(triggers);
}

void HolyPriestBoostRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestBoostStrategy::InitNonCombatTriggers(triggers);
    PriestBoostRaidStrategy::InitNonCombatTriggers(triggers);
}

void HolyPriestCcStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestCcStrategy::InitCombatTriggers(triggers);
}

void HolyPriestCcStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestCcStrategy::InitNonCombatTriggers(triggers);
}

void HolyPriestCcPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestCcStrategy::InitCombatTriggers(triggers);
    PriestCcPveStrategy::InitCombatTriggers(triggers);
}

void HolyPriestCcPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestCcStrategy::InitNonCombatTriggers(triggers);
    PriestCcPveStrategy::InitNonCombatTriggers(triggers);
}

void HolyPriestCcPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestCcStrategy::InitCombatTriggers(triggers);
    PriestCcPvpStrategy::InitCombatTriggers(triggers);
}

void HolyPriestCcPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestCcStrategy::InitNonCombatTriggers(triggers);
    PriestCcPvpStrategy::InitNonCombatTriggers(triggers);
}

void HolyPriestCcRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestCcStrategy::InitCombatTriggers(triggers);
    PriestCcRaidStrategy::InitCombatTriggers(triggers);
}

void HolyPriestCcRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestCcStrategy::InitNonCombatTriggers(triggers);
    PriestCcRaidStrategy::InitNonCombatTriggers(triggers);
}

void HolyPriestCureStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestCureStrategy::InitCombatTriggers(triggers);
}

void HolyPriestCureStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestCureStrategy::InitNonCombatTriggers(triggers);
}

void HolyPriestCurePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestCureStrategy::InitCombatTriggers(triggers);
    PriestCurePveStrategy::InitCombatTriggers(triggers);
}

void HolyPriestCurePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestCureStrategy::InitNonCombatTriggers(triggers);
    PriestCurePveStrategy::InitNonCombatTriggers(triggers);
}

void HolyPriestCurePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestCureStrategy::InitCombatTriggers(triggers);
    PriestCurePvpStrategy::InitCombatTriggers(triggers);
}

void HolyPriestCurePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestCureStrategy::InitNonCombatTriggers(triggers);
    PriestCurePvpStrategy::InitNonCombatTriggers(triggers);
}

void HolyPriestCureRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestCureStrategy::InitCombatTriggers(triggers);
    PriestCureRaidStrategy::InitCombatTriggers(triggers);
}

void HolyPriestCureRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestCureStrategy::InitNonCombatTriggers(triggers);
    PriestCureRaidStrategy::InitNonCombatTriggers(triggers);
}

#endif
#ifdef MANGOSBOT_ONE // TBC

void HolyPriestStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "party member critical health",
        NextAction::array(0, new NextAction("power word: shield on party", ACTION_CRITICAL_HEAL + 1),
                             new NextAction("flash heal on party", ACTION_CRITICAL_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "party member low health",
        NextAction::array(0, new NextAction("power word: shield on party", ACTION_MEDIUM_HEAL + 2),
                             new NextAction("greater heal on party", ACTION_MEDIUM_HEAL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "party member medium health",
        NextAction::array(0, new NextAction("greater heal on party", ACTION_MEDIUM_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "party member almost full health",
        NextAction::array(0, new NextAction("renew on party", ACTION_LIGHT_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "binding heal",
        NextAction::array(0, new NextAction("binding heal", ACTION_LIGHT_HEAL), NULL)));
}

void HolyPriestStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestStrategy::InitNonCombatTriggers(triggers);
}

void HolyPriestStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    PriestStrategy::InitReactionTriggers(triggers);
}

void HolyPriestStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    PriestStrategy::InitDeadTriggers(triggers);
}

void HolyPriestPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestStrategy::InitCombatTriggers(triggers);
    PriestPveStrategy::InitCombatTriggers(triggers);
}

void HolyPriestPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestStrategy::InitNonCombatTriggers(triggers);
    PriestPveStrategy::InitNonCombatTriggers(triggers);
}

void HolyPriestPveStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestStrategy::InitReactionTriggers(triggers);
    PriestPveStrategy::InitReactionTriggers(triggers);
}

void HolyPriestPveStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestStrategy::InitDeadTriggers(triggers);
    PriestPveStrategy::InitDeadTriggers(triggers);
}

void HolyPriestPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestStrategy::InitCombatTriggers(triggers);
    PriestPvpStrategy::InitCombatTriggers(triggers);
}

void HolyPriestPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestStrategy::InitNonCombatTriggers(triggers);
    PriestPvpStrategy::InitNonCombatTriggers(triggers);
}

void HolyPriestPvpStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestStrategy::InitReactionTriggers(triggers);
    PriestPvpStrategy::InitReactionTriggers(triggers);
}

void HolyPriestPvpStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestStrategy::InitDeadTriggers(triggers);
    PriestPvpStrategy::InitDeadTriggers(triggers);
}

void HolyPriestRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestStrategy::InitCombatTriggers(triggers);
    PriestRaidStrategy::InitCombatTriggers(triggers);
}

void HolyPriestRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestStrategy::InitNonCombatTriggers(triggers);
    PriestRaidStrategy::InitNonCombatTriggers(triggers);
}

void HolyPriestRaidStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestStrategy::InitReactionTriggers(triggers);
    PriestRaidStrategy::InitReactionTriggers(triggers);
}

void HolyPriestRaidStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestStrategy::InitDeadTriggers(triggers);
    PriestRaidStrategy::InitDeadTriggers(triggers);
}

void HolyPriestAoeStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestAoeStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "medium aoe heal",
        NextAction::array(0, new NextAction("prayer of mending", ACTION_MEDIUM_HEAL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "medium aoe heal",
        NextAction::array(0, new NextAction("circle of healing", ACTION_MEDIUM_HEAL), NULL)));
}

void HolyPriestAoeStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestAoeStrategy::InitNonCombatTriggers(triggers);
}

void HolyPriestAoePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestAoeStrategy::InitCombatTriggers(triggers);
    PriestAoePveStrategy::InitCombatTriggers(triggers);
}

void HolyPriestAoePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestAoeStrategy::InitNonCombatTriggers(triggers);
    PriestAoePveStrategy::InitNonCombatTriggers(triggers);
}

void HolyPriestAoePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestAoeStrategy::InitCombatTriggers(triggers);
    PriestAoePvpStrategy::InitCombatTriggers(triggers);
}

void HolyPriestAoePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestAoeStrategy::InitNonCombatTriggers(triggers);
    PriestAoePvpStrategy::InitNonCombatTriggers(triggers);
}

void HolyPriestAoeRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestAoeStrategy::InitCombatTriggers(triggers);
    PriestAoeRaidStrategy::InitCombatTriggers(triggers);
}

void HolyPriestAoeRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestAoeStrategy::InitNonCombatTriggers(triggers);
    PriestAoeRaidStrategy::InitNonCombatTriggers(triggers);
}

void HolyPriestBuffStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestBuffStrategy::InitCombatTriggers(triggers);
}

void HolyPriestBuffStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestBuffStrategy::InitNonCombatTriggers(triggers);
}

void HolyPriestBuffPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestBuffStrategy::InitCombatTriggers(triggers);
    PriestBuffPveStrategy::InitCombatTriggers(triggers);
}

void HolyPriestBuffPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestBuffStrategy::InitNonCombatTriggers(triggers);
    PriestBuffPveStrategy::InitNonCombatTriggers(triggers);
}

void HolyPriestBuffPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestBuffStrategy::InitCombatTriggers(triggers);
    PriestBuffPvpStrategy::InitCombatTriggers(triggers);
}

void HolyPriestBuffPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestBuffStrategy::InitNonCombatTriggers(triggers);
    PriestBuffPvpStrategy::InitNonCombatTriggers(triggers);
}

void HolyPriestBuffRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestBuffStrategy::InitCombatTriggers(triggers);
    PriestBuffRaidStrategy::InitCombatTriggers(triggers);
}

void HolyPriestBuffRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestBuffStrategy::InitNonCombatTriggers(triggers);
    PriestBuffRaidStrategy::InitNonCombatTriggers(triggers);
}

void HolyPriestBoostStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestBoostStrategy::InitCombatTriggers(triggers);
}

void HolyPriestBoostStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestBoostStrategy::InitNonCombatTriggers(triggers);
}

void HolyPriestBoostPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestBoostStrategy::InitCombatTriggers(triggers);
    PriestBoostPveStrategy::InitCombatTriggers(triggers);
}

void HolyPriestBoostPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestBoostStrategy::InitNonCombatTriggers(triggers);
    PriestBoostPveStrategy::InitNonCombatTriggers(triggers);
}

void HolyPriestBoostPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestBoostStrategy::InitCombatTriggers(triggers);
    PriestBoostPvpStrategy::InitCombatTriggers(triggers);
}

void HolyPriestBoostPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestBoostStrategy::InitNonCombatTriggers(triggers);
    PriestBoostPvpStrategy::InitNonCombatTriggers(triggers);
}

void HolyPriestBoostRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestBoostStrategy::InitCombatTriggers(triggers);
    PriestBoostRaidStrategy::InitCombatTriggers(triggers);
}

void HolyPriestBoostRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestBoostStrategy::InitNonCombatTriggers(triggers);
    PriestBoostRaidStrategy::InitNonCombatTriggers(triggers);
}

void HolyPriestCcStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestCcStrategy::InitCombatTriggers(triggers);
}

void HolyPriestCcStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestCcStrategy::InitNonCombatTriggers(triggers);
}

void HolyPriestCcPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestCcStrategy::InitCombatTriggers(triggers);
    PriestCcPveStrategy::InitCombatTriggers(triggers);
}

void HolyPriestCcPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestCcStrategy::InitNonCombatTriggers(triggers);
    PriestCcPveStrategy::InitNonCombatTriggers(triggers);
}

void HolyPriestCcPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestCcStrategy::InitCombatTriggers(triggers);
    PriestCcPvpStrategy::InitCombatTriggers(triggers);
}

void HolyPriestCcPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestCcStrategy::InitNonCombatTriggers(triggers);
    PriestCcPvpStrategy::InitNonCombatTriggers(triggers);
}

void HolyPriestCcRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestCcStrategy::InitCombatTriggers(triggers);
    PriestCcRaidStrategy::InitCombatTriggers(triggers);
}

void HolyPriestCcRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestCcStrategy::InitNonCombatTriggers(triggers);
    PriestCcRaidStrategy::InitNonCombatTriggers(triggers);
}

void HolyPriestCureStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestCureStrategy::InitCombatTriggers(triggers);
}

void HolyPriestCureStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestCureStrategy::InitNonCombatTriggers(triggers);
}

void HolyPriestCurePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestCureStrategy::InitCombatTriggers(triggers);
    PriestCurePveStrategy::InitCombatTriggers(triggers);
}

void HolyPriestCurePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestCureStrategy::InitNonCombatTriggers(triggers);
    PriestCurePveStrategy::InitNonCombatTriggers(triggers);
}

void HolyPriestCurePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestCureStrategy::InitCombatTriggers(triggers);
    PriestCurePvpStrategy::InitCombatTriggers(triggers);
}

void HolyPriestCurePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestCureStrategy::InitNonCombatTriggers(triggers);
    PriestCurePvpStrategy::InitNonCombatTriggers(triggers);
}

void HolyPriestCureRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestCureStrategy::InitCombatTriggers(triggers);
    PriestCureRaidStrategy::InitCombatTriggers(triggers);
}

void HolyPriestCureRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestCureStrategy::InitNonCombatTriggers(triggers);
    PriestCureRaidStrategy::InitNonCombatTriggers(triggers);
}

#endif
#ifdef MANGOSBOT_TWO // WOTLK

void HolyPriestStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "party member critical health",
        NextAction::array(0, new NextAction("power word: shield on party", ACTION_CRITICAL_HEAL + 1),
                             new NextAction("flash heal on party", ACTION_CRITICAL_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "party member low health",
        NextAction::array(0, new NextAction("power word: shield on party", ACTION_MEDIUM_HEAL + 2),
                             new NextAction("greater heal on party", ACTION_MEDIUM_HEAL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "party member medium health",
        NextAction::array(0, new NextAction("greater heal on party", ACTION_MEDIUM_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "party member almost full health",
        NextAction::array(0, new NextAction("renew on party", ACTION_LIGHT_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "binding heal",
        NextAction::array(0, new NextAction("binding heal", ACTION_LIGHT_HEAL), NULL)));
}

void HolyPriestStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestStrategy::InitNonCombatTriggers(triggers);
}

void HolyPriestStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    PriestStrategy::InitReactionTriggers(triggers);
}

void HolyPriestStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    PriestStrategy::InitDeadTriggers(triggers);
}

void HolyPriestPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestStrategy::InitCombatTriggers(triggers);
    PriestPveStrategy::InitCombatTriggers(triggers);
}

void HolyPriestPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestStrategy::InitNonCombatTriggers(triggers);
    PriestPveStrategy::InitNonCombatTriggers(triggers);
}

void HolyPriestPveStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestStrategy::InitReactionTriggers(triggers);
    PriestPveStrategy::InitReactionTriggers(triggers);
}

void HolyPriestPveStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestStrategy::InitDeadTriggers(triggers);
    PriestPveStrategy::InitDeadTriggers(triggers);
}

void HolyPriestPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestStrategy::InitCombatTriggers(triggers);
    PriestPvpStrategy::InitCombatTriggers(triggers);
}

void HolyPriestPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestStrategy::InitNonCombatTriggers(triggers);
    PriestPvpStrategy::InitNonCombatTriggers(triggers);
}

void HolyPriestPvpStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestStrategy::InitReactionTriggers(triggers);
    PriestPvpStrategy::InitReactionTriggers(triggers);
}

void HolyPriestPvpStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestStrategy::InitDeadTriggers(triggers);
    PriestPvpStrategy::InitDeadTriggers(triggers);
}

void HolyPriestRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestStrategy::InitCombatTriggers(triggers);
    PriestRaidStrategy::InitCombatTriggers(triggers);
}

void HolyPriestRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestStrategy::InitNonCombatTriggers(triggers);
    PriestRaidStrategy::InitNonCombatTriggers(triggers);
}

void HolyPriestRaidStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestStrategy::InitReactionTriggers(triggers);
    PriestRaidStrategy::InitReactionTriggers(triggers);
}

void HolyPriestRaidStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestStrategy::InitDeadTriggers(triggers);
    PriestRaidStrategy::InitDeadTriggers(triggers);
}

void HolyPriestAoeStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestAoeStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "medium aoe heal",
        NextAction::array(0, new NextAction("prayer of mending", ACTION_MEDIUM_HEAL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "medium aoe heal",
        NextAction::array(0, new NextAction("circle of healing", ACTION_MEDIUM_HEAL), NULL)));
}

void HolyPriestAoeStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestAoeStrategy::InitNonCombatTriggers(triggers);
}

void HolyPriestAoePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestAoeStrategy::InitCombatTriggers(triggers);
    PriestAoePveStrategy::InitCombatTriggers(triggers);
}

void HolyPriestAoePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestAoeStrategy::InitNonCombatTriggers(triggers);
    PriestAoePveStrategy::InitNonCombatTriggers(triggers);
}

void HolyPriestAoePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestAoeStrategy::InitCombatTriggers(triggers);
    PriestAoePvpStrategy::InitCombatTriggers(triggers);
}

void HolyPriestAoePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestAoeStrategy::InitNonCombatTriggers(triggers);
    PriestAoePvpStrategy::InitNonCombatTriggers(triggers);
}

void HolyPriestAoeRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestAoeStrategy::InitCombatTriggers(triggers);
    PriestAoeRaidStrategy::InitCombatTriggers(triggers);
}

void HolyPriestAoeRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestAoeStrategy::InitNonCombatTriggers(triggers);
    PriestAoeRaidStrategy::InitNonCombatTriggers(triggers);
}

void HolyPriestBuffStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestBuffStrategy::InitCombatTriggers(triggers);
}

void HolyPriestBuffStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestBuffStrategy::InitNonCombatTriggers(triggers);
}

void HolyPriestBuffPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestBuffStrategy::InitCombatTriggers(triggers);
    PriestBuffPveStrategy::InitCombatTriggers(triggers);
}

void HolyPriestBuffPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestBuffStrategy::InitNonCombatTriggers(triggers);
    PriestBuffPveStrategy::InitNonCombatTriggers(triggers);
}

void HolyPriestBuffPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestBuffStrategy::InitCombatTriggers(triggers);
    PriestBuffPvpStrategy::InitCombatTriggers(triggers);
}

void HolyPriestBuffPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestBuffStrategy::InitNonCombatTriggers(triggers);
    PriestBuffPvpStrategy::InitNonCombatTriggers(triggers);
}

void HolyPriestBuffRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestBuffStrategy::InitCombatTriggers(triggers);
    PriestBuffRaidStrategy::InitCombatTriggers(triggers);
}

void HolyPriestBuffRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestBuffStrategy::InitNonCombatTriggers(triggers);
    PriestBuffRaidStrategy::InitNonCombatTriggers(triggers);
}

void HolyPriestBoostStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestBoostStrategy::InitCombatTriggers(triggers);
}

void HolyPriestBoostStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestBoostStrategy::InitNonCombatTriggers(triggers);
}

void HolyPriestBoostPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestBoostStrategy::InitCombatTriggers(triggers);
    PriestBoostPveStrategy::InitCombatTriggers(triggers);
}

void HolyPriestBoostPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestBoostStrategy::InitNonCombatTriggers(triggers);
    PriestBoostPveStrategy::InitNonCombatTriggers(triggers);
}

void HolyPriestBoostPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestBoostStrategy::InitCombatTriggers(triggers);
    PriestBoostPvpStrategy::InitCombatTriggers(triggers);
}

void HolyPriestBoostPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestBoostStrategy::InitNonCombatTriggers(triggers);
    PriestBoostPvpStrategy::InitNonCombatTriggers(triggers);
}

void HolyPriestBoostRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestBoostStrategy::InitCombatTriggers(triggers);
    PriestBoostRaidStrategy::InitCombatTriggers(triggers);
}

void HolyPriestBoostRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestBoostStrategy::InitNonCombatTriggers(triggers);
    PriestBoostRaidStrategy::InitNonCombatTriggers(triggers);
}

void HolyPriestCcStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestCcStrategy::InitCombatTriggers(triggers);
}

void HolyPriestCcStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestCcStrategy::InitNonCombatTriggers(triggers);
}

void HolyPriestCcPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestCcStrategy::InitCombatTriggers(triggers);
    PriestCcPveStrategy::InitCombatTriggers(triggers);
}

void HolyPriestCcPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestCcStrategy::InitNonCombatTriggers(triggers);
    PriestCcPveStrategy::InitNonCombatTriggers(triggers);
}

void HolyPriestCcPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestCcStrategy::InitCombatTriggers(triggers);
    PriestCcPvpStrategy::InitCombatTriggers(triggers);
}

void HolyPriestCcPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestCcStrategy::InitNonCombatTriggers(triggers);
    PriestCcPvpStrategy::InitNonCombatTriggers(triggers);
}

void HolyPriestCcRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestCcStrategy::InitCombatTriggers(triggers);
    PriestCcRaidStrategy::InitCombatTriggers(triggers);
}

void HolyPriestCcRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestCcStrategy::InitNonCombatTriggers(triggers);
    PriestCcRaidStrategy::InitNonCombatTriggers(triggers);
}

void HolyPriestCureStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestCureStrategy::InitCombatTriggers(triggers);
}

void HolyPriestCureStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestCureStrategy::InitNonCombatTriggers(triggers);
}

void HolyPriestCurePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestCureStrategy::InitCombatTriggers(triggers);
    PriestCurePveStrategy::InitCombatTriggers(triggers);
}

void HolyPriestCurePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestCureStrategy::InitNonCombatTriggers(triggers);
    PriestCurePveStrategy::InitNonCombatTriggers(triggers);
}

void HolyPriestCurePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestCureStrategy::InitCombatTriggers(triggers);
    PriestCurePvpStrategy::InitCombatTriggers(triggers);
}

void HolyPriestCurePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestCureStrategy::InitNonCombatTriggers(triggers);
    PriestCurePvpStrategy::InitNonCombatTriggers(triggers);
}

void HolyPriestCureRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestCureStrategy::InitCombatTriggers(triggers);
    PriestCureRaidStrategy::InitCombatTriggers(triggers);
}

void HolyPriestCureRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPriestCureStrategy::InitNonCombatTriggers(triggers);
    PriestCureRaidStrategy::InitNonCombatTriggers(triggers);
}

#endif