
#include "playerbot/playerbot.h"
#include "HunterMultipliers.h"
#include "BeastMasteryHunterStrategy.h"

using namespace ai;

#ifdef MANGOSBOT_ZERO // Vanilla

void BeastMasteryHunterStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HunterStrategy::InitCombatTriggers(triggers);
}

void BeastMasteryHunterStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HunterStrategy::InitNonCombatTriggers(triggers);
}

void BeastMasteryHunterStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    HunterStrategy::InitReactionTriggers(triggers);
}

void BeastMasteryHunterStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    HunterStrategy::InitDeadTriggers(triggers);
}

void BeastMasteryHunterPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterStrategy::InitCombatTriggers(triggers);
    HunterPveStrategy::InitCombatTriggers(triggers);
}

void BeastMasteryHunterPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterStrategy::InitNonCombatTriggers(triggers);
    HunterPveStrategy::InitNonCombatTriggers(triggers);
}

void BeastMasteryHunterPveStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterStrategy::InitReactionTriggers(triggers);
    HunterPveStrategy::InitReactionTriggers(triggers);
}

void BeastMasteryHunterPveStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterStrategy::InitDeadTriggers(triggers);
    HunterPveStrategy::InitDeadTriggers(triggers);
}

void BeastMasteryHunterPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterStrategy::InitCombatTriggers(triggers);
    HunterPvpStrategy::InitCombatTriggers(triggers);
}

void BeastMasteryHunterPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterStrategy::InitNonCombatTriggers(triggers);
    HunterPvpStrategy::InitNonCombatTriggers(triggers);
}

void BeastMasteryHunterPvpStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterStrategy::InitReactionTriggers(triggers);
    HunterPvpStrategy::InitReactionTriggers(triggers);
}

void BeastMasteryHunterPvpStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterStrategy::InitDeadTriggers(triggers);
    HunterPvpStrategy::InitDeadTriggers(triggers);
}

void BeastMasteryHunterRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterStrategy::InitCombatTriggers(triggers);
    HunterRaidStrategy::InitCombatTriggers(triggers);
}

void BeastMasteryHunterRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterStrategy::InitNonCombatTriggers(triggers);
    HunterRaidStrategy::InitNonCombatTriggers(triggers);
}

void BeastMasteryHunterRaidStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterStrategy::InitReactionTriggers(triggers);
    HunterRaidStrategy::InitReactionTriggers(triggers);
}

void BeastMasteryHunterRaidStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterStrategy::InitDeadTriggers(triggers);
    HunterRaidStrategy::InitDeadTriggers(triggers);
}

void BeastMasteryHunterAoeStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HunterAoeStrategy::InitCombatTriggers(triggers);
}

void BeastMasteryHunterAoeStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HunterAoeStrategy::InitNonCombatTriggers(triggers);
}

void BeastMasteryHunterAoePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterAoeStrategy::InitCombatTriggers(triggers);
    HunterAoePveStrategy::InitCombatTriggers(triggers);
}

void BeastMasteryHunterAoePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterAoeStrategy::InitNonCombatTriggers(triggers);
    HunterAoePveStrategy::InitNonCombatTriggers(triggers);
}

void BeastMasteryHunterAoePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterAoeStrategy::InitCombatTriggers(triggers);
    HunterAoePvpStrategy::InitCombatTriggers(triggers);
}

void BeastMasteryHunterAoePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterAoeStrategy::InitNonCombatTriggers(triggers);
    HunterAoePvpStrategy::InitNonCombatTriggers(triggers);
}

void BeastMasteryHunterAoeRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterAoeStrategy::InitCombatTriggers(triggers);
    HunterAoeRaidStrategy::InitCombatTriggers(triggers);
}

void BeastMasteryHunterAoeRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterAoeStrategy::InitNonCombatTriggers(triggers);
    HunterAoeRaidStrategy::InitNonCombatTriggers(triggers);
}

void BeastMasteryHunterBuffStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HunterBuffStrategy::InitCombatTriggers(triggers);
}

void BeastMasteryHunterBuffStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HunterBuffStrategy::InitNonCombatTriggers(triggers);
}

void BeastMasteryHunterBuffPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterBuffStrategy::InitCombatTriggers(triggers);
    HunterBuffPveStrategy::InitCombatTriggers(triggers);
}

void BeastMasteryHunterBuffPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterBuffStrategy::InitNonCombatTriggers(triggers);
    HunterBuffPveStrategy::InitNonCombatTriggers(triggers);
}

void BeastMasteryHunterBuffPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterBuffStrategy::InitCombatTriggers(triggers);
    HunterBuffPvpStrategy::InitCombatTriggers(triggers);
}

void BeastMasteryHunterBuffPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterBuffStrategy::InitNonCombatTriggers(triggers);
    HunterBuffPvpStrategy::InitNonCombatTriggers(triggers);
}

void BeastMasteryHunterBuffRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterBuffStrategy::InitCombatTriggers(triggers);
    HunterBuffRaidStrategy::InitCombatTriggers(triggers);
}

void BeastMasteryHunterBuffRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterBuffStrategy::InitNonCombatTriggers(triggers);
    HunterBuffRaidStrategy::InitNonCombatTriggers(triggers);
}

void BeastMasteryHunterBoostStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HunterBoostStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "bestial wrath",
        NextAction::array(0, new NextAction("bestial wrath", ACTION_HIGH + 6), NULL)));
}

void BeastMasteryHunterBoostStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HunterBoostStrategy::InitNonCombatTriggers(triggers);
}

void BeastMasteryHunterBoostPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterBoostStrategy::InitCombatTriggers(triggers);
    HunterBoostPveStrategy::InitCombatTriggers(triggers);
}

void BeastMasteryHunterBoostPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterBoostStrategy::InitNonCombatTriggers(triggers);
    HunterBoostPveStrategy::InitNonCombatTriggers(triggers);
}

void BeastMasteryHunterBoostPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterBoostStrategy::InitCombatTriggers(triggers);
    HunterBoostPvpStrategy::InitCombatTriggers(triggers);
}

void BeastMasteryHunterBoostPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterBoostStrategy::InitNonCombatTriggers(triggers);
    HunterBoostPvpStrategy::InitNonCombatTriggers(triggers);
}

void BeastMasteryHunterBoostRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterBoostStrategy::InitCombatTriggers(triggers);
    HunterBoostRaidStrategy::InitCombatTriggers(triggers);
}

void BeastMasteryHunterBoostRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterBoostStrategy::InitNonCombatTriggers(triggers);
    HunterBoostRaidStrategy::InitNonCombatTriggers(triggers);
}

void BeastMasteryHunterCcStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HunterCcStrategy::InitCombatTriggers(triggers);
}

void BeastMasteryHunterCcStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HunterCcStrategy::InitNonCombatTriggers(triggers);
}

void BeastMasteryHunterCcPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterCcStrategy::InitCombatTriggers(triggers);
    HunterCcPveStrategy::InitCombatTriggers(triggers);
}

void BeastMasteryHunterCcPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterCcStrategy::InitNonCombatTriggers(triggers);
    HunterCcPveStrategy::InitNonCombatTriggers(triggers);
}

void BeastMasteryHunterCcPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterCcStrategy::InitCombatTriggers(triggers);
    HunterCcPvpStrategy::InitCombatTriggers(triggers);
}

void BeastMasteryHunterCcPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterCcStrategy::InitNonCombatTriggers(triggers);
    HunterCcPvpStrategy::InitNonCombatTriggers(triggers);
}

void BeastMasteryHunterCcRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterCcStrategy::InitCombatTriggers(triggers);
    HunterCcRaidStrategy::InitCombatTriggers(triggers);
}

void BeastMasteryHunterCcRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterCcStrategy::InitNonCombatTriggers(triggers);
    HunterCcRaidStrategy::InitNonCombatTriggers(triggers);
}

void BeastMasteryHunterStingStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HunterStingStrategy::InitCombatTriggers(triggers);
}

void BeastMasteryHunterStingPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterStingStrategy::InitCombatTriggers(triggers);
    HunterStingPveStrategy::InitCombatTriggers(triggers);
}

void BeastMasteryHunterStingPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterStingStrategy::InitCombatTriggers(triggers);
    HunterStingPvpStrategy::InitCombatTriggers(triggers);
}

void BeastMasteryHunterStingRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterStingStrategy::InitCombatTriggers(triggers);
    HunterStingRaidStrategy::InitCombatTriggers(triggers);
}

void BeastMasteryHunterAspectStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HunterAspectStrategy::InitCombatTriggers(triggers);
}

void BeastMasteryHunterAspectStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HunterAspectStrategy::InitNonCombatTriggers(triggers);
}

void BeastMasteryHunterAspectPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterAspectStrategy::InitCombatTriggers(triggers);
    HunterAspectPveStrategy::InitCombatTriggers(triggers);
}

void BeastMasteryHunterAspectPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterAspectStrategy::InitNonCombatTriggers(triggers);
    HunterAspectPveStrategy::InitNonCombatTriggers(triggers);
}

void BeastMasteryHunterAspectPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterAspectStrategy::InitCombatTriggers(triggers);
    HunterAspectPvpStrategy::InitCombatTriggers(triggers);
}

void BeastMasteryHunterAspectPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterAspectStrategy::InitNonCombatTriggers(triggers);
    HunterAspectPvpStrategy::InitNonCombatTriggers(triggers);
}

void BeastMasteryHunterAspectRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterAspectStrategy::InitCombatTriggers(triggers);
    HunterAspectRaidStrategy::InitCombatTriggers(triggers);
}

void BeastMasteryHunterAspectRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterAspectStrategy::InitNonCombatTriggers(triggers);
    HunterAspectRaidStrategy::InitNonCombatTriggers(triggers);
}

#endif
#ifdef MANGOSBOT_ONE // TBC

void BeastMasteryHunterStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HunterStrategy::InitCombatTriggers(triggers);
}

void BeastMasteryHunterStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HunterStrategy::InitNonCombatTriggers(triggers);
}

void BeastMasteryHunterStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    HunterStrategy::InitReactionTriggers(triggers);
}

void BeastMasteryHunterStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    HunterStrategy::InitDeadTriggers(triggers);
}

void BeastMasteryHunterPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterStrategy::InitCombatTriggers(triggers);
    HunterPveStrategy::InitCombatTriggers(triggers);
}

void BeastMasteryHunterPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterStrategy::InitNonCombatTriggers(triggers);
    HunterPveStrategy::InitNonCombatTriggers(triggers);
}

void BeastMasteryHunterPveStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterStrategy::InitReactionTriggers(triggers);
    HunterPveStrategy::InitReactionTriggers(triggers);
}

void BeastMasteryHunterPveStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterStrategy::InitDeadTriggers(triggers);
    HunterPveStrategy::InitDeadTriggers(triggers);
}

void BeastMasteryHunterPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterStrategy::InitCombatTriggers(triggers);
    HunterPvpStrategy::InitCombatTriggers(triggers);
}

void BeastMasteryHunterPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterStrategy::InitNonCombatTriggers(triggers);
    HunterPvpStrategy::InitNonCombatTriggers(triggers);
}

void BeastMasteryHunterPvpStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterStrategy::InitReactionTriggers(triggers);
    HunterPvpStrategy::InitReactionTriggers(triggers);
}

void BeastMasteryHunterPvpStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterStrategy::InitDeadTriggers(triggers);
    HunterPvpStrategy::InitDeadTriggers(triggers);
}

void BeastMasteryHunterRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterStrategy::InitCombatTriggers(triggers);
    HunterRaidStrategy::InitCombatTriggers(triggers);
}

void BeastMasteryHunterRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterStrategy::InitNonCombatTriggers(triggers);
    HunterRaidStrategy::InitNonCombatTriggers(triggers);
}

void BeastMasteryHunterRaidStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterStrategy::InitReactionTriggers(triggers);
    HunterRaidStrategy::InitReactionTriggers(triggers);
}

void BeastMasteryHunterRaidStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterStrategy::InitDeadTriggers(triggers);
    HunterRaidStrategy::InitDeadTriggers(triggers);
}

void BeastMasteryHunterAoeStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HunterAoeStrategy::InitCombatTriggers(triggers);
}

void BeastMasteryHunterAoeStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HunterAoeStrategy::InitNonCombatTriggers(triggers);
}

void BeastMasteryHunterAoePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterAoeStrategy::InitCombatTriggers(triggers);
    HunterAoePveStrategy::InitCombatTriggers(triggers);
}

void BeastMasteryHunterAoePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterAoeStrategy::InitNonCombatTriggers(triggers);
    HunterAoePveStrategy::InitNonCombatTriggers(triggers);
}

void BeastMasteryHunterAoePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterAoeStrategy::InitCombatTriggers(triggers);
    HunterAoePvpStrategy::InitCombatTriggers(triggers);
}

void BeastMasteryHunterAoePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterAoeStrategy::InitNonCombatTriggers(triggers);
    HunterAoePvpStrategy::InitNonCombatTriggers(triggers);
}

void BeastMasteryHunterAoeRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterAoeStrategy::InitCombatTriggers(triggers);
    HunterAoeRaidStrategy::InitCombatTriggers(triggers);
}

void BeastMasteryHunterAoeRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterAoeStrategy::InitNonCombatTriggers(triggers);
    HunterAoeRaidStrategy::InitNonCombatTriggers(triggers);
}

void BeastMasteryHunterBuffStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HunterBuffStrategy::InitCombatTriggers(triggers);
}

void BeastMasteryHunterBuffStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HunterBuffStrategy::InitNonCombatTriggers(triggers);
}

void BeastMasteryHunterBuffPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterBuffStrategy::InitCombatTriggers(triggers);
    HunterBuffPveStrategy::InitCombatTriggers(triggers);
}

void BeastMasteryHunterBuffPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterBuffStrategy::InitNonCombatTriggers(triggers);
    HunterBuffPveStrategy::InitNonCombatTriggers(triggers);
}

void BeastMasteryHunterBuffPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterBuffStrategy::InitCombatTriggers(triggers);
    HunterBuffPvpStrategy::InitCombatTriggers(triggers);
}

void BeastMasteryHunterBuffPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterBuffStrategy::InitNonCombatTriggers(triggers);
    HunterBuffPvpStrategy::InitNonCombatTriggers(triggers);
}

void BeastMasteryHunterBuffRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterBuffStrategy::InitCombatTriggers(triggers);
    HunterBuffRaidStrategy::InitCombatTriggers(triggers);
}

void BeastMasteryHunterBuffRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterBuffStrategy::InitNonCombatTriggers(triggers);
    HunterBuffRaidStrategy::InitNonCombatTriggers(triggers);
}

void BeastMasteryHunterBoostStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HunterBoostStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "bestial wrath",
        NextAction::array(0, new NextAction("bestial wrath", ACTION_HIGH + 6), NULL)));
}

void BeastMasteryHunterBoostStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HunterBoostStrategy::InitNonCombatTriggers(triggers);
}

void BeastMasteryHunterBoostPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterBoostStrategy::InitCombatTriggers(triggers);
    HunterBoostPveStrategy::InitCombatTriggers(triggers);
}

void BeastMasteryHunterBoostPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterBoostStrategy::InitNonCombatTriggers(triggers);
    HunterBoostPveStrategy::InitNonCombatTriggers(triggers);
}

void BeastMasteryHunterBoostPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterBoostStrategy::InitCombatTriggers(triggers);
    HunterBoostPvpStrategy::InitCombatTriggers(triggers);
}

void BeastMasteryHunterBoostPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterBoostStrategy::InitNonCombatTriggers(triggers);
    HunterBoostPvpStrategy::InitNonCombatTriggers(triggers);
}

void BeastMasteryHunterBoostRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterBoostStrategy::InitCombatTriggers(triggers);
    HunterBoostRaidStrategy::InitCombatTriggers(triggers);
}

void BeastMasteryHunterBoostRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterBoostStrategy::InitNonCombatTriggers(triggers);
    HunterBoostRaidStrategy::InitNonCombatTriggers(triggers);
}

void BeastMasteryHunterCcStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HunterCcStrategy::InitCombatTriggers(triggers);
}

void BeastMasteryHunterCcStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HunterCcStrategy::InitNonCombatTriggers(triggers);
}

void BeastMasteryHunterCcPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterCcStrategy::InitCombatTriggers(triggers);
    HunterCcPveStrategy::InitCombatTriggers(triggers);
}

void BeastMasteryHunterCcPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterCcStrategy::InitNonCombatTriggers(triggers);
    HunterCcPveStrategy::InitNonCombatTriggers(triggers);
}

void BeastMasteryHunterCcPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterCcStrategy::InitCombatTriggers(triggers);
    HunterCcPvpStrategy::InitCombatTriggers(triggers);
}

void BeastMasteryHunterCcPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterCcStrategy::InitNonCombatTriggers(triggers);
    HunterCcPvpStrategy::InitNonCombatTriggers(triggers);
}

void BeastMasteryHunterCcRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterCcStrategy::InitCombatTriggers(triggers);
    HunterCcRaidStrategy::InitCombatTriggers(triggers);
}

void BeastMasteryHunterCcRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterCcStrategy::InitNonCombatTriggers(triggers);
    HunterCcRaidStrategy::InitNonCombatTriggers(triggers);
}

void BeastMasteryHunterStingStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HunterStingStrategy::InitCombatTriggers(triggers);
}

void BeastMasteryHunterStingPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterStingStrategy::InitCombatTriggers(triggers);
    HunterStingPveStrategy::InitCombatTriggers(triggers);
}

void BeastMasteryHunterStingPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterStingStrategy::InitCombatTriggers(triggers);
    HunterStingPvpStrategy::InitCombatTriggers(triggers);
}

void BeastMasteryHunterStingRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterStingStrategy::InitCombatTriggers(triggers);
    HunterStingRaidStrategy::InitCombatTriggers(triggers);
}

void BeastMasteryHunterAspectStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HunterAspectStrategy::InitCombatTriggers(triggers);
}

void BeastMasteryHunterAspectStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HunterAspectStrategy::InitNonCombatTriggers(triggers);
}

void BeastMasteryHunterAspectPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterAspectStrategy::InitCombatTriggers(triggers);
    HunterAspectPveStrategy::InitCombatTriggers(triggers);
}

void BeastMasteryHunterAspectPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterAspectStrategy::InitNonCombatTriggers(triggers);
    HunterAspectPveStrategy::InitNonCombatTriggers(triggers);
}

void BeastMasteryHunterAspectPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterAspectStrategy::InitCombatTriggers(triggers);
    HunterAspectPvpStrategy::InitCombatTriggers(triggers);
}

void BeastMasteryHunterAspectPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterAspectStrategy::InitNonCombatTriggers(triggers);
    HunterAspectPvpStrategy::InitNonCombatTriggers(triggers);
}

void BeastMasteryHunterAspectRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterAspectStrategy::InitCombatTriggers(triggers);
    HunterAspectRaidStrategy::InitCombatTriggers(triggers);
}

void BeastMasteryHunterAspectRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterAspectStrategy::InitNonCombatTriggers(triggers);
    HunterAspectRaidStrategy::InitNonCombatTriggers(triggers);
}

#endif
#ifdef MANGOSBOT_TWO // WOTLK

void BeastMasteryHunterStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HunterStrategy::InitCombatTriggers(triggers);
}

void BeastMasteryHunterStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HunterStrategy::InitNonCombatTriggers(triggers);
}

void BeastMasteryHunterStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    HunterStrategy::InitReactionTriggers(triggers);
}

void BeastMasteryHunterStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    HunterStrategy::InitDeadTriggers(triggers);
}

void BeastMasteryHunterPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterStrategy::InitCombatTriggers(triggers);
    HunterPveStrategy::InitCombatTriggers(triggers);
}

void BeastMasteryHunterPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterStrategy::InitNonCombatTriggers(triggers);
    HunterPveStrategy::InitNonCombatTriggers(triggers);
}

void BeastMasteryHunterPveStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterStrategy::InitReactionTriggers(triggers);
    HunterPveStrategy::InitReactionTriggers(triggers);
}

void BeastMasteryHunterPveStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterStrategy::InitDeadTriggers(triggers);
    HunterPveStrategy::InitDeadTriggers(triggers);
}

void BeastMasteryHunterPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterStrategy::InitCombatTriggers(triggers);
    HunterPvpStrategy::InitCombatTriggers(triggers);
}

void BeastMasteryHunterPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterStrategy::InitNonCombatTriggers(triggers);
    HunterPvpStrategy::InitNonCombatTriggers(triggers);
}

void BeastMasteryHunterPvpStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterStrategy::InitReactionTriggers(triggers);
    HunterPvpStrategy::InitReactionTriggers(triggers);
}

void BeastMasteryHunterPvpStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterStrategy::InitDeadTriggers(triggers);
    HunterPvpStrategy::InitDeadTriggers(triggers);
}

void BeastMasteryHunterRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterStrategy::InitCombatTriggers(triggers);
    HunterRaidStrategy::InitCombatTriggers(triggers);
}

void BeastMasteryHunterRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterStrategy::InitNonCombatTriggers(triggers);
    HunterRaidStrategy::InitNonCombatTriggers(triggers);
}

void BeastMasteryHunterRaidStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterStrategy::InitReactionTriggers(triggers);
    HunterRaidStrategy::InitReactionTriggers(triggers);
}

void BeastMasteryHunterRaidStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterStrategy::InitDeadTriggers(triggers);
    HunterRaidStrategy::InitDeadTriggers(triggers);
}

void BeastMasteryHunterAoeStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HunterAoeStrategy::InitCombatTriggers(triggers);
}

void BeastMasteryHunterAoeStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HunterAoeStrategy::InitNonCombatTriggers(triggers);
}

void BeastMasteryHunterAoePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterAoeStrategy::InitCombatTriggers(triggers);
    HunterAoePveStrategy::InitCombatTriggers(triggers);
}

void BeastMasteryHunterAoePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterAoeStrategy::InitNonCombatTriggers(triggers);
    HunterAoePveStrategy::InitNonCombatTriggers(triggers);
}

void BeastMasteryHunterAoePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterAoeStrategy::InitCombatTriggers(triggers);
    HunterAoePvpStrategy::InitCombatTriggers(triggers);
}

void BeastMasteryHunterAoePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterAoeStrategy::InitNonCombatTriggers(triggers);
    HunterAoePvpStrategy::InitNonCombatTriggers(triggers);
}

void BeastMasteryHunterAoeRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterAoeStrategy::InitCombatTriggers(triggers);
    HunterAoeRaidStrategy::InitCombatTriggers(triggers);
}

void BeastMasteryHunterAoeRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterAoeStrategy::InitNonCombatTriggers(triggers);
    HunterAoeRaidStrategy::InitNonCombatTriggers(triggers);
}

void BeastMasteryHunterBuffStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HunterBuffStrategy::InitCombatTriggers(triggers);
}

void BeastMasteryHunterBuffStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HunterBuffStrategy::InitNonCombatTriggers(triggers);
}

void BeastMasteryHunterBuffPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterBuffStrategy::InitCombatTriggers(triggers);
    HunterBuffPveStrategy::InitCombatTriggers(triggers);
}

void BeastMasteryHunterBuffPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterBuffStrategy::InitNonCombatTriggers(triggers);
    HunterBuffPveStrategy::InitNonCombatTriggers(triggers);
}

void BeastMasteryHunterBuffPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterBuffStrategy::InitCombatTriggers(triggers);
    HunterBuffPvpStrategy::InitCombatTriggers(triggers);
}

void BeastMasteryHunterBuffPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterBuffStrategy::InitNonCombatTriggers(triggers);
    HunterBuffPvpStrategy::InitNonCombatTriggers(triggers);
}

void BeastMasteryHunterBuffRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterBuffStrategy::InitCombatTriggers(triggers);
    HunterBuffRaidStrategy::InitCombatTriggers(triggers);
}

void BeastMasteryHunterBuffRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterBuffStrategy::InitNonCombatTriggers(triggers);
    HunterBuffRaidStrategy::InitNonCombatTriggers(triggers);
}

void BeastMasteryHunterBoostStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HunterBoostStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "bestial wrath",
        NextAction::array(0, new NextAction("bestial wrath", ACTION_HIGH + 6), NULL)));
}

void BeastMasteryHunterBoostStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HunterBoostStrategy::InitNonCombatTriggers(triggers);
}

void BeastMasteryHunterBoostPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterBoostStrategy::InitCombatTriggers(triggers);
    HunterBoostPveStrategy::InitCombatTriggers(triggers);
}

void BeastMasteryHunterBoostPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterBoostStrategy::InitNonCombatTriggers(triggers);
    HunterBoostPveStrategy::InitNonCombatTriggers(triggers);
}

void BeastMasteryHunterBoostPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterBoostStrategy::InitCombatTriggers(triggers);
    HunterBoostPvpStrategy::InitCombatTriggers(triggers);
}

void BeastMasteryHunterBoostPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterBoostStrategy::InitNonCombatTriggers(triggers);
    HunterBoostPvpStrategy::InitNonCombatTriggers(triggers);
}

void BeastMasteryHunterBoostRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterBoostStrategy::InitCombatTriggers(triggers);
    HunterBoostRaidStrategy::InitCombatTriggers(triggers);
}

void BeastMasteryHunterBoostRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterBoostStrategy::InitNonCombatTriggers(triggers);
    HunterBoostRaidStrategy::InitNonCombatTriggers(triggers);
}

void BeastMasteryHunterCcStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HunterCcStrategy::InitCombatTriggers(triggers);
}

void BeastMasteryHunterCcStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HunterCcStrategy::InitNonCombatTriggers(triggers);
}

void BeastMasteryHunterCcPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterCcStrategy::InitCombatTriggers(triggers);
    HunterCcPveStrategy::InitCombatTriggers(triggers);
}

void BeastMasteryHunterCcPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterCcStrategy::InitNonCombatTriggers(triggers);
    HunterCcPveStrategy::InitNonCombatTriggers(triggers);
}

void BeastMasteryHunterCcPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterCcStrategy::InitCombatTriggers(triggers);
    HunterCcPvpStrategy::InitCombatTriggers(triggers);
}

void BeastMasteryHunterCcPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterCcStrategy::InitNonCombatTriggers(triggers);
    HunterCcPvpStrategy::InitNonCombatTriggers(triggers);
}

void BeastMasteryHunterCcRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterCcStrategy::InitCombatTriggers(triggers);
    HunterCcRaidStrategy::InitCombatTriggers(triggers);
}

void BeastMasteryHunterCcRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterCcStrategy::InitNonCombatTriggers(triggers);
    HunterCcRaidStrategy::InitNonCombatTriggers(triggers);
}

void BeastMasteryHunterStingStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HunterStingStrategy::InitCombatTriggers(triggers);
}

void BeastMasteryHunterStingPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterStingStrategy::InitCombatTriggers(triggers);
    HunterStingPveStrategy::InitCombatTriggers(triggers);
}

void BeastMasteryHunterStingPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterStingStrategy::InitCombatTriggers(triggers);
    HunterStingPvpStrategy::InitCombatTriggers(triggers);
}

void BeastMasteryHunterStingRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterStingStrategy::InitCombatTriggers(triggers);
    HunterStingRaidStrategy::InitCombatTriggers(triggers);
}

void BeastMasteryHunterAspectStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HunterAspectStrategy::InitCombatTriggers(triggers);
}

void BeastMasteryHunterAspectStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HunterAspectStrategy::InitNonCombatTriggers(triggers);
}

void BeastMasteryHunterAspectPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterAspectStrategy::InitCombatTriggers(triggers);
    HunterAspectPveStrategy::InitCombatTriggers(triggers);
}

void BeastMasteryHunterAspectPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterAspectStrategy::InitNonCombatTriggers(triggers);
    HunterAspectPveStrategy::InitNonCombatTriggers(triggers);
}

void BeastMasteryHunterAspectPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterAspectStrategy::InitCombatTriggers(triggers);
    HunterAspectPvpStrategy::InitCombatTriggers(triggers);
}

void BeastMasteryHunterAspectPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterAspectStrategy::InitNonCombatTriggers(triggers);
    HunterAspectPvpStrategy::InitNonCombatTriggers(triggers);
}

void BeastMasteryHunterAspectRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterAspectStrategy::InitCombatTriggers(triggers);
    HunterAspectRaidStrategy::InitCombatTriggers(triggers);
}

void BeastMasteryHunterAspectRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BeastMasteryHunterAspectStrategy::InitNonCombatTriggers(triggers);
    HunterAspectRaidStrategy::InitNonCombatTriggers(triggers);
}

#endif