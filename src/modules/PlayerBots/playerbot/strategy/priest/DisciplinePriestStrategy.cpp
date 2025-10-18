#include "playerbot/playerbot.h"
#include "PriestMultipliers.h"
#include "DisciplinePriestStrategy.h"

using namespace ai;

class DisciplinePriestStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    DisciplinePriestStrategyActionNodeFactory()
    {
        
    }

private:

};

DisciplinePriestStrategy::DisciplinePriestStrategy(PlayerbotAI* ai) : PriestStrategy(ai)
{
    actionNodeFactories.Add(new DisciplinePriestStrategyActionNodeFactory());
}

#ifdef MANGOSBOT_ZERO // Vanilla

void DisciplinePriestStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
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

void DisciplinePriestStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestStrategy::InitNonCombatTriggers(triggers);
}

void DisciplinePriestStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    PriestStrategy::InitReactionTriggers(triggers);
}

void DisciplinePriestStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    PriestStrategy::InitDeadTriggers(triggers);
}

void DisciplinePriestPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestStrategy::InitCombatTriggers(triggers);
    PriestPveStrategy::InitCombatTriggers(triggers);
}

void DisciplinePriestPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestStrategy::InitNonCombatTriggers(triggers);
    PriestPveStrategy::InitNonCombatTriggers(triggers);
}

void DisciplinePriestPveStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestStrategy::InitReactionTriggers(triggers);
    PriestPveStrategy::InitReactionTriggers(triggers);
}

void DisciplinePriestPveStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestStrategy::InitDeadTriggers(triggers);
    PriestPveStrategy::InitDeadTriggers(triggers);
}

void DisciplinePriestPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestStrategy::InitCombatTriggers(triggers);
    PriestPvpStrategy::InitCombatTriggers(triggers);
}

void DisciplinePriestPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestStrategy::InitNonCombatTriggers(triggers);
    PriestPvpStrategy::InitNonCombatTriggers(triggers);
}

void DisciplinePriestPvpStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestStrategy::InitReactionTriggers(triggers);
    PriestPvpStrategy::InitReactionTriggers(triggers);
}

void DisciplinePriestPvpStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestStrategy::InitDeadTriggers(triggers);
    PriestPvpStrategy::InitDeadTriggers(triggers);
}

void DisciplinePriestRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestStrategy::InitCombatTriggers(triggers);
    PriestRaidStrategy::InitCombatTriggers(triggers);
}

void DisciplinePriestRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestStrategy::InitNonCombatTriggers(triggers);
    PriestRaidStrategy::InitNonCombatTriggers(triggers);
}

void DisciplinePriestRaidStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestStrategy::InitReactionTriggers(triggers);
    PriestRaidStrategy::InitReactionTriggers(triggers);
}

void DisciplinePriestRaidStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestStrategy::InitDeadTriggers(triggers);
    PriestRaidStrategy::InitDeadTriggers(triggers);
}

void DisciplinePriestAoeStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestAoeStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "medium aoe heal",
        NextAction::array(0, new NextAction("prayer of healing", ACTION_MEDIUM_HEAL), NULL)));
}

void DisciplinePriestAoeStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestAoeStrategy::InitNonCombatTriggers(triggers);
}

void DisciplinePriestAoePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestAoeStrategy::InitCombatTriggers(triggers);
    PriestAoePveStrategy::InitCombatTriggers(triggers);
}

void DisciplinePriestAoePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestAoeStrategy::InitNonCombatTriggers(triggers);
    PriestAoePveStrategy::InitNonCombatTriggers(triggers);
}

void DisciplinePriestAoePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestAoeStrategy::InitCombatTriggers(triggers);
    PriestAoePvpStrategy::InitCombatTriggers(triggers);
}

void DisciplinePriestAoePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestAoeStrategy::InitNonCombatTriggers(triggers);
    PriestAoePvpStrategy::InitNonCombatTriggers(triggers);
}

void DisciplinePriestAoeRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestAoeStrategy::InitCombatTriggers(triggers);
    PriestAoeRaidStrategy::InitCombatTriggers(triggers);
}

void DisciplinePriestAoeRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestAoeStrategy::InitNonCombatTriggers(triggers);
    PriestAoeRaidStrategy::InitNonCombatTriggers(triggers);
}

void DisciplinePriestBuffStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestBuffStrategy::InitCombatTriggers(triggers);
}

void DisciplinePriestBuffStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestBuffStrategy::InitNonCombatTriggers(triggers);
}

void DisciplinePriestBuffPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestBuffStrategy::InitCombatTriggers(triggers);
    PriestBuffPveStrategy::InitCombatTriggers(triggers);
}

void DisciplinePriestBuffPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestBuffStrategy::InitNonCombatTriggers(triggers);
    PriestBuffPveStrategy::InitNonCombatTriggers(triggers);
}

void DisciplinePriestBuffPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestBuffStrategy::InitCombatTriggers(triggers);
    PriestBuffPvpStrategy::InitCombatTriggers(triggers);
}

void DisciplinePriestBuffPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestBuffStrategy::InitNonCombatTriggers(triggers);
    PriestBuffPvpStrategy::InitNonCombatTriggers(triggers);
}

void DisciplinePriestBuffRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestBuffStrategy::InitCombatTriggers(triggers);
    PriestBuffRaidStrategy::InitCombatTriggers(triggers);
}

void DisciplinePriestBuffRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestBuffStrategy::InitNonCombatTriggers(triggers);
    PriestBuffRaidStrategy::InitNonCombatTriggers(triggers);
}

void DisciplinePriestBoostStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestBoostStrategy::InitCombatTriggers(triggers);
}

void DisciplinePriestBoostStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestBoostStrategy::InitNonCombatTriggers(triggers);
}

void DisciplinePriestBoostPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestBoostStrategy::InitCombatTriggers(triggers);
    PriestBoostPveStrategy::InitCombatTriggers(triggers);
}

void DisciplinePriestBoostPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestBoostStrategy::InitNonCombatTriggers(triggers);
    PriestBoostPveStrategy::InitNonCombatTriggers(triggers);
}

void DisciplinePriestBoostPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestBoostStrategy::InitCombatTriggers(triggers);
    PriestBoostPvpStrategy::InitCombatTriggers(triggers);
}

void DisciplinePriestBoostPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestBoostStrategy::InitNonCombatTriggers(triggers);
    PriestBoostPvpStrategy::InitNonCombatTriggers(triggers);
}

void DisciplinePriestBoostRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestBoostStrategy::InitCombatTriggers(triggers);
    PriestBoostRaidStrategy::InitCombatTriggers(triggers);
}

void DisciplinePriestBoostRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestBoostStrategy::InitNonCombatTriggers(triggers);
    PriestBoostRaidStrategy::InitNonCombatTriggers(triggers);
}

void DisciplinePriestCcStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestCcStrategy::InitCombatTriggers(triggers);
}

void DisciplinePriestCcStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestCcStrategy::InitNonCombatTriggers(triggers);
}

void DisciplinePriestCcPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestCcStrategy::InitCombatTriggers(triggers);
    PriestCcPveStrategy::InitCombatTriggers(triggers);
}

void DisciplinePriestCcPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestCcStrategy::InitNonCombatTriggers(triggers);
    PriestCcPveStrategy::InitNonCombatTriggers(triggers);
}

void DisciplinePriestCcPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestCcStrategy::InitCombatTriggers(triggers);
    PriestCcPvpStrategy::InitCombatTriggers(triggers);
}

void DisciplinePriestCcPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestCcStrategy::InitNonCombatTriggers(triggers);
    PriestCcPvpStrategy::InitNonCombatTriggers(triggers);
}

void DisciplinePriestCcRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestCcStrategy::InitCombatTriggers(triggers);
    PriestCcRaidStrategy::InitCombatTriggers(triggers);
}

void DisciplinePriestCcRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestCcStrategy::InitNonCombatTriggers(triggers);
    PriestCcRaidStrategy::InitNonCombatTriggers(triggers);
}

void DisciplinePriestCureStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestCureStrategy::InitCombatTriggers(triggers);
}

void DisciplinePriestCureStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestCureStrategy::InitNonCombatTriggers(triggers);
}

void DisciplinePriestCurePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestCureStrategy::InitCombatTriggers(triggers);
    PriestCurePveStrategy::InitCombatTriggers(triggers);
}

void DisciplinePriestCurePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestCureStrategy::InitNonCombatTriggers(triggers);
    PriestCurePveStrategy::InitNonCombatTriggers(triggers);
}

void DisciplinePriestCurePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestCureStrategy::InitCombatTriggers(triggers);
    PriestCurePvpStrategy::InitCombatTriggers(triggers);
}

void DisciplinePriestCurePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestCureStrategy::InitNonCombatTriggers(triggers);
    PriestCurePvpStrategy::InitNonCombatTriggers(triggers);
}

void DisciplinePriestCureRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestCureStrategy::InitCombatTriggers(triggers);
    PriestCureRaidStrategy::InitCombatTriggers(triggers);
}

void DisciplinePriestCureRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestCureStrategy::InitNonCombatTriggers(triggers);
    PriestCureRaidStrategy::InitNonCombatTriggers(triggers);
}

#endif
#ifdef MANGOSBOT_ONE // TBC

void DisciplinePriestStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "protect party member",
        NextAction::array(0, new NextAction("pain suppression on party", ACTION_EMERGENCY), NULL)));

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

void DisciplinePriestStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestStrategy::InitNonCombatTriggers(triggers);
}

void DisciplinePriestStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    PriestStrategy::InitReactionTriggers(triggers);
}

void DisciplinePriestStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    PriestStrategy::InitDeadTriggers(triggers);
}

void DisciplinePriestPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestStrategy::InitCombatTriggers(triggers);
    PriestPveStrategy::InitCombatTriggers(triggers);
}

void DisciplinePriestPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestStrategy::InitNonCombatTriggers(triggers);
    PriestPveStrategy::InitNonCombatTriggers(triggers);
}

void DisciplinePriestPveStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestStrategy::InitReactionTriggers(triggers);
    PriestPveStrategy::InitReactionTriggers(triggers);
}

void DisciplinePriestPveStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestStrategy::InitDeadTriggers(triggers);
    PriestPveStrategy::InitDeadTriggers(triggers);
}

void DisciplinePriestPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestStrategy::InitCombatTriggers(triggers);
    PriestPvpStrategy::InitCombatTriggers(triggers);
}

void DisciplinePriestPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestStrategy::InitNonCombatTriggers(triggers);
    PriestPvpStrategy::InitNonCombatTriggers(triggers);
}

void DisciplinePriestPvpStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestStrategy::InitReactionTriggers(triggers);
    PriestPvpStrategy::InitReactionTriggers(triggers);
}

void DisciplinePriestPvpStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestStrategy::InitDeadTriggers(triggers);
    PriestPvpStrategy::InitDeadTriggers(triggers);
}

void DisciplinePriestRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestStrategy::InitCombatTriggers(triggers);
    PriestRaidStrategy::InitCombatTriggers(triggers);
}

void DisciplinePriestRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestStrategy::InitNonCombatTriggers(triggers);
    PriestRaidStrategy::InitNonCombatTriggers(triggers);
}

void DisciplinePriestRaidStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestStrategy::InitReactionTriggers(triggers);
    PriestRaidStrategy::InitReactionTriggers(triggers);
}

void DisciplinePriestRaidStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestStrategy::InitDeadTriggers(triggers);
    PriestRaidStrategy::InitDeadTriggers(triggers);
}

void DisciplinePriestAoeStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestAoeStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "medium aoe heal",
        NextAction::array(0, new NextAction("prayer of mending", ACTION_MEDIUM_HEAL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "medium aoe heal",
        NextAction::array(0, new NextAction("prayer of healing", ACTION_MEDIUM_HEAL), NULL)));
}

void DisciplinePriestAoeStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestAoeStrategy::InitNonCombatTriggers(triggers);
}

void DisciplinePriestAoePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestAoeStrategy::InitCombatTriggers(triggers);
    PriestAoePveStrategy::InitCombatTriggers(triggers);
}

void DisciplinePriestAoePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestAoeStrategy::InitNonCombatTriggers(triggers);
    PriestAoePveStrategy::InitNonCombatTriggers(triggers);
}

void DisciplinePriestAoePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestAoeStrategy::InitCombatTriggers(triggers);
    PriestAoePvpStrategy::InitCombatTriggers(triggers);
}

void DisciplinePriestAoePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestAoeStrategy::InitNonCombatTriggers(triggers);
    PriestAoePvpStrategy::InitNonCombatTriggers(triggers);
}

void DisciplinePriestAoeRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestAoeStrategy::InitCombatTriggers(triggers);
    PriestAoeRaidStrategy::InitCombatTriggers(triggers);
}

void DisciplinePriestAoeRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestAoeStrategy::InitNonCombatTriggers(triggers);
    PriestAoeRaidStrategy::InitNonCombatTriggers(triggers);
}

void DisciplinePriestBuffStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestBuffStrategy::InitCombatTriggers(triggers);
}

void DisciplinePriestBuffStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestBuffStrategy::InitNonCombatTriggers(triggers);
}

void DisciplinePriestBuffPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestBuffStrategy::InitCombatTriggers(triggers);
    PriestBuffPveStrategy::InitCombatTriggers(triggers);
}

void DisciplinePriestBuffPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestBuffStrategy::InitNonCombatTriggers(triggers);
    PriestBuffPveStrategy::InitNonCombatTriggers(triggers);
}

void DisciplinePriestBuffPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestBuffStrategy::InitCombatTriggers(triggers);
    PriestBuffPvpStrategy::InitCombatTriggers(triggers);
}

void DisciplinePriestBuffPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestBuffStrategy::InitNonCombatTriggers(triggers);
    PriestBuffPvpStrategy::InitNonCombatTriggers(triggers);
}

void DisciplinePriestBuffRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestBuffStrategy::InitCombatTriggers(triggers);
    PriestBuffRaidStrategy::InitCombatTriggers(triggers);
}

void DisciplinePriestBuffRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestBuffStrategy::InitNonCombatTriggers(triggers);
    PriestBuffRaidStrategy::InitNonCombatTriggers(triggers);
}

void DisciplinePriestBoostStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestBoostStrategy::InitCombatTriggers(triggers);
}

void DisciplinePriestBoostStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestBoostStrategy::InitNonCombatTriggers(triggers);
}

void DisciplinePriestBoostPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestBoostStrategy::InitCombatTriggers(triggers);
    PriestBoostPveStrategy::InitCombatTriggers(triggers);
}

void DisciplinePriestBoostPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestBoostStrategy::InitNonCombatTriggers(triggers);
    PriestBoostPveStrategy::InitNonCombatTriggers(triggers);
}

void DisciplinePriestBoostPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestBoostStrategy::InitCombatTriggers(triggers);
    PriestBoostPvpStrategy::InitCombatTriggers(triggers);
}

void DisciplinePriestBoostPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestBoostStrategy::InitNonCombatTriggers(triggers);
    PriestBoostPvpStrategy::InitNonCombatTriggers(triggers);
}

void DisciplinePriestBoostRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestBoostStrategy::InitCombatTriggers(triggers);
    PriestBoostRaidStrategy::InitCombatTriggers(triggers);
}

void DisciplinePriestBoostRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestBoostStrategy::InitNonCombatTriggers(triggers);
    PriestBoostRaidStrategy::InitNonCombatTriggers(triggers);
}

void DisciplinePriestCcStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestCcStrategy::InitCombatTriggers(triggers);
}

void DisciplinePriestCcStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestCcStrategy::InitNonCombatTriggers(triggers);
}

void DisciplinePriestCcPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestCcStrategy::InitCombatTriggers(triggers);
    PriestCcPveStrategy::InitCombatTriggers(triggers);
}

void DisciplinePriestCcPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestCcStrategy::InitNonCombatTriggers(triggers);
    PriestCcPveStrategy::InitNonCombatTriggers(triggers);
}

void DisciplinePriestCcPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestCcStrategy::InitCombatTriggers(triggers);
    PriestCcPvpStrategy::InitCombatTriggers(triggers);
}

void DisciplinePriestCcPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestCcStrategy::InitNonCombatTriggers(triggers);
    PriestCcPvpStrategy::InitNonCombatTriggers(triggers);
}

void DisciplinePriestCcRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestCcStrategy::InitCombatTriggers(triggers);
    PriestCcRaidStrategy::InitCombatTriggers(triggers);
}

void DisciplinePriestCcRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestCcStrategy::InitNonCombatTriggers(triggers);
    PriestCcRaidStrategy::InitNonCombatTriggers(triggers);
}

void DisciplinePriestCureStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestCureStrategy::InitCombatTriggers(triggers);
}

void DisciplinePriestCureStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestCureStrategy::InitNonCombatTriggers(triggers);
}

void DisciplinePriestCurePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestCureStrategy::InitCombatTriggers(triggers);
    PriestCurePveStrategy::InitCombatTriggers(triggers);
}

void DisciplinePriestCurePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestCureStrategy::InitNonCombatTriggers(triggers);
    PriestCurePveStrategy::InitNonCombatTriggers(triggers);
}

void DisciplinePriestCurePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestCureStrategy::InitCombatTriggers(triggers);
    PriestCurePvpStrategy::InitCombatTriggers(triggers);
}

void DisciplinePriestCurePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestCureStrategy::InitNonCombatTriggers(triggers);
    PriestCurePvpStrategy::InitNonCombatTriggers(triggers);
}

void DisciplinePriestCureRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestCureStrategy::InitCombatTriggers(triggers);
    PriestCureRaidStrategy::InitCombatTriggers(triggers);
}

void DisciplinePriestCureRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestCureStrategy::InitNonCombatTriggers(triggers);
    PriestCureRaidStrategy::InitNonCombatTriggers(triggers);
}

#endif
#ifdef MANGOSBOT_TWO // WOTLK

void DisciplinePriestStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "protect party member",
        NextAction::array(0, new NextAction("pain suppression on party", ACTION_EMERGENCY), NULL)));

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

void DisciplinePriestStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestStrategy::InitNonCombatTriggers(triggers);
}

void DisciplinePriestStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    PriestStrategy::InitReactionTriggers(triggers);
}

void DisciplinePriestStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    PriestStrategy::InitDeadTriggers(triggers);
}

void DisciplinePriestPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestStrategy::InitCombatTriggers(triggers);
    PriestPveStrategy::InitCombatTriggers(triggers);
}

void DisciplinePriestPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestStrategy::InitNonCombatTriggers(triggers);
    PriestPveStrategy::InitNonCombatTriggers(triggers);
}

void DisciplinePriestPveStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestStrategy::InitReactionTriggers(triggers);
    PriestPveStrategy::InitReactionTriggers(triggers);
}

void DisciplinePriestPveStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestStrategy::InitDeadTriggers(triggers);
    PriestPveStrategy::InitDeadTriggers(triggers);
}

void DisciplinePriestPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestStrategy::InitCombatTriggers(triggers);
    PriestPvpStrategy::InitCombatTriggers(triggers);
}

void DisciplinePriestPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestStrategy::InitNonCombatTriggers(triggers);
    PriestPvpStrategy::InitNonCombatTriggers(triggers);
}

void DisciplinePriestPvpStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestStrategy::InitReactionTriggers(triggers);
    PriestPvpStrategy::InitReactionTriggers(triggers);
}

void DisciplinePriestPvpStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestStrategy::InitDeadTriggers(triggers);
    PriestPvpStrategy::InitDeadTriggers(triggers);
}

void DisciplinePriestRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestStrategy::InitCombatTriggers(triggers);
    PriestRaidStrategy::InitCombatTriggers(triggers);
}

void DisciplinePriestRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestStrategy::InitNonCombatTriggers(triggers);
    PriestRaidStrategy::InitNonCombatTriggers(triggers);
}

void DisciplinePriestRaidStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestStrategy::InitReactionTriggers(triggers);
    PriestRaidStrategy::InitReactionTriggers(triggers);
}

void DisciplinePriestRaidStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestStrategy::InitDeadTriggers(triggers);
    PriestRaidStrategy::InitDeadTriggers(triggers);
}

void DisciplinePriestAoeStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestAoeStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "medium aoe heal",
        NextAction::array(0, new NextAction("prayer of mending", ACTION_MEDIUM_HEAL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "medium aoe heal",
        NextAction::array(0, new NextAction("prayer of healing", ACTION_MEDIUM_HEAL), NULL)));
}

void DisciplinePriestAoeStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestAoeStrategy::InitNonCombatTriggers(triggers);
}

void DisciplinePriestAoePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestAoeStrategy::InitCombatTriggers(triggers);
    PriestAoePveStrategy::InitCombatTriggers(triggers);
}

void DisciplinePriestAoePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestAoeStrategy::InitNonCombatTriggers(triggers);
    PriestAoePveStrategy::InitNonCombatTriggers(triggers);
}

void DisciplinePriestAoePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestAoeStrategy::InitCombatTriggers(triggers);
    PriestAoePvpStrategy::InitCombatTriggers(triggers);
}

void DisciplinePriestAoePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestAoeStrategy::InitNonCombatTriggers(triggers);
    PriestAoePvpStrategy::InitNonCombatTriggers(triggers);
}

void DisciplinePriestAoeRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestAoeStrategy::InitCombatTriggers(triggers);
    PriestAoeRaidStrategy::InitCombatTriggers(triggers);
}

void DisciplinePriestAoeRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestAoeStrategy::InitNonCombatTriggers(triggers);
    PriestAoeRaidStrategy::InitNonCombatTriggers(triggers);
}

void DisciplinePriestBuffStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestBuffStrategy::InitCombatTriggers(triggers);
}

void DisciplinePriestBuffStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestBuffStrategy::InitNonCombatTriggers(triggers);
}

void DisciplinePriestBuffPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestBuffStrategy::InitCombatTriggers(triggers);
    PriestBuffPveStrategy::InitCombatTriggers(triggers);
}

void DisciplinePriestBuffPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestBuffStrategy::InitNonCombatTriggers(triggers);
    PriestBuffPveStrategy::InitNonCombatTriggers(triggers);
}

void DisciplinePriestBuffPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestBuffStrategy::InitCombatTriggers(triggers);
    PriestBuffPvpStrategy::InitCombatTriggers(triggers);
}

void DisciplinePriestBuffPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestBuffStrategy::InitNonCombatTriggers(triggers);
    PriestBuffPvpStrategy::InitNonCombatTriggers(triggers);
}

void DisciplinePriestBuffRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestBuffStrategy::InitCombatTriggers(triggers);
    PriestBuffRaidStrategy::InitCombatTriggers(triggers);
}

void DisciplinePriestBuffRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestBuffStrategy::InitNonCombatTriggers(triggers);
    PriestBuffRaidStrategy::InitNonCombatTriggers(triggers);
}

void DisciplinePriestBoostStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestBoostStrategy::InitCombatTriggers(triggers);
}

void DisciplinePriestBoostStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestBoostStrategy::InitNonCombatTriggers(triggers);
}

void DisciplinePriestBoostPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestBoostStrategy::InitCombatTriggers(triggers);
    PriestBoostPveStrategy::InitCombatTriggers(triggers);
}

void DisciplinePriestBoostPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestBoostStrategy::InitNonCombatTriggers(triggers);
    PriestBoostPveStrategy::InitNonCombatTriggers(triggers);
}

void DisciplinePriestBoostPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestBoostStrategy::InitCombatTriggers(triggers);
    PriestBoostPvpStrategy::InitCombatTriggers(triggers);
}

void DisciplinePriestBoostPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestBoostStrategy::InitNonCombatTriggers(triggers);
    PriestBoostPvpStrategy::InitNonCombatTriggers(triggers);
}

void DisciplinePriestBoostRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestBoostStrategy::InitCombatTriggers(triggers);
    PriestBoostRaidStrategy::InitCombatTriggers(triggers);
}

void DisciplinePriestBoostRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestBoostStrategy::InitNonCombatTriggers(triggers);
    PriestBoostRaidStrategy::InitNonCombatTriggers(triggers);
}

void DisciplinePriestCcStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestCcStrategy::InitCombatTriggers(triggers);
}

void DisciplinePriestCcStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestCcStrategy::InitNonCombatTriggers(triggers);
}

void DisciplinePriestCcPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestCcStrategy::InitCombatTriggers(triggers);
    PriestCcPveStrategy::InitCombatTriggers(triggers);
}

void DisciplinePriestCcPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestCcStrategy::InitNonCombatTriggers(triggers);
    PriestCcPveStrategy::InitNonCombatTriggers(triggers);
}

void DisciplinePriestCcPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestCcStrategy::InitCombatTriggers(triggers);
    PriestCcPvpStrategy::InitCombatTriggers(triggers);
}

void DisciplinePriestCcPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestCcStrategy::InitNonCombatTriggers(triggers);
    PriestCcPvpStrategy::InitNonCombatTriggers(triggers);
}

void DisciplinePriestCcRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestCcStrategy::InitCombatTriggers(triggers);
    PriestCcRaidStrategy::InitCombatTriggers(triggers);
}

void DisciplinePriestCcRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestCcStrategy::InitNonCombatTriggers(triggers);
    PriestCcRaidStrategy::InitNonCombatTriggers(triggers);
}

void DisciplinePriestCureStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestCureStrategy::InitCombatTriggers(triggers);
}

void DisciplinePriestCureStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestCureStrategy::InitNonCombatTriggers(triggers);
}

void DisciplinePriestCurePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestCureStrategy::InitCombatTriggers(triggers);
    PriestCurePveStrategy::InitCombatTriggers(triggers);
}

void DisciplinePriestCurePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestCureStrategy::InitNonCombatTriggers(triggers);
    PriestCurePveStrategy::InitNonCombatTriggers(triggers);
}

void DisciplinePriestCurePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestCureStrategy::InitCombatTriggers(triggers);
    PriestCurePvpStrategy::InitCombatTriggers(triggers);
}

void DisciplinePriestCurePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestCureStrategy::InitNonCombatTriggers(triggers);
    PriestCurePvpStrategy::InitNonCombatTriggers(triggers);
}

void DisciplinePriestCureRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestCureStrategy::InitCombatTriggers(triggers);
    PriestCureRaidStrategy::InitCombatTriggers(triggers);
}

void DisciplinePriestCureRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DisciplinePriestCureStrategy::InitNonCombatTriggers(triggers);
    PriestCureRaidStrategy::InitNonCombatTriggers(triggers);
}

#endif