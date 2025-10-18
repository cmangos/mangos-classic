
#include "playerbot/playerbot.h"
#include "HunterMultipliers.h"
#include "SurvivalHunterStrategy.h"

using namespace ai;

#ifdef MANGOSBOT_ZERO // Vanilla

void SurvivalHunterStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HunterStrategy::InitCombatTriggers(triggers);
}

void SurvivalHunterStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HunterStrategy::InitNonCombatTriggers(triggers);
}

void SurvivalHunterStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    HunterStrategy::InitReactionTriggers(triggers);
}

void SurvivalHunterStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    HunterStrategy::InitDeadTriggers(triggers);
}

void SurvivalHunterPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterStrategy::InitCombatTriggers(triggers);
    HunterPveStrategy::InitCombatTriggers(triggers);
}

void SurvivalHunterPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterStrategy::InitNonCombatTriggers(triggers);
    HunterPveStrategy::InitNonCombatTriggers(triggers);
}

void SurvivalHunterPveStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterStrategy::InitReactionTriggers(triggers);
    HunterPveStrategy::InitReactionTriggers(triggers);
}

void SurvivalHunterPveStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterStrategy::InitDeadTriggers(triggers);
    HunterPveStrategy::InitDeadTriggers(triggers);
}

void SurvivalHunterPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterStrategy::InitCombatTriggers(triggers);
    HunterPvpStrategy::InitCombatTriggers(triggers);
}

void SurvivalHunterPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterStrategy::InitNonCombatTriggers(triggers);
    HunterPvpStrategy::InitNonCombatTriggers(triggers);
}

void SurvivalHunterPvpStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterStrategy::InitReactionTriggers(triggers);
    HunterPvpStrategy::InitReactionTriggers(triggers);
}

void SurvivalHunterPvpStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterStrategy::InitDeadTriggers(triggers);
    HunterPvpStrategy::InitDeadTriggers(triggers);
}

void SurvivalHunterRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterStrategy::InitCombatTriggers(triggers);
    HunterRaidStrategy::InitCombatTriggers(triggers);
}

void SurvivalHunterRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterStrategy::InitNonCombatTriggers(triggers);
    HunterRaidStrategy::InitNonCombatTriggers(triggers);
}

void SurvivalHunterRaidStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterStrategy::InitReactionTriggers(triggers);
    HunterRaidStrategy::InitReactionTriggers(triggers);
}

void SurvivalHunterRaidStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterStrategy::InitDeadTriggers(triggers);
    HunterRaidStrategy::InitDeadTriggers(triggers);
}

void SurvivalHunterAoeStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HunterAoeStrategy::InitCombatTriggers(triggers);
}

void SurvivalHunterAoeStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HunterAoeStrategy::InitNonCombatTriggers(triggers);
}

void SurvivalHunterAoePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterAoeStrategy::InitCombatTriggers(triggers);
    HunterAoePveStrategy::InitCombatTriggers(triggers);
}

void SurvivalHunterAoePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterAoeStrategy::InitNonCombatTriggers(triggers);
    HunterAoePveStrategy::InitNonCombatTriggers(triggers);
}

void SurvivalHunterAoePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterAoeStrategy::InitCombatTriggers(triggers);
    HunterAoePvpStrategy::InitCombatTriggers(triggers);
}

void SurvivalHunterAoePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterAoeStrategy::InitNonCombatTriggers(triggers);
    HunterAoePvpStrategy::InitNonCombatTriggers(triggers);
}

void SurvivalHunterAoeRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterAoeStrategy::InitCombatTriggers(triggers);
    HunterAoeRaidStrategy::InitCombatTriggers(triggers);
}

void SurvivalHunterAoeRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterAoeStrategy::InitNonCombatTriggers(triggers);
    HunterAoeRaidStrategy::InitNonCombatTriggers(triggers);
}

void SurvivalHunterBuffStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HunterBuffStrategy::InitCombatTriggers(triggers);
}

void SurvivalHunterBuffStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HunterBuffStrategy::InitNonCombatTriggers(triggers);
}

void SurvivalHunterBuffPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterBuffStrategy::InitCombatTriggers(triggers);
    HunterBuffPveStrategy::InitCombatTriggers(triggers);
}

void SurvivalHunterBuffPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterBuffStrategy::InitNonCombatTriggers(triggers);
    HunterBuffPveStrategy::InitNonCombatTriggers(triggers);
}

void SurvivalHunterBuffPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterBuffStrategy::InitCombatTriggers(triggers);
    HunterBuffPvpStrategy::InitCombatTriggers(triggers);
}

void SurvivalHunterBuffPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterBuffStrategy::InitNonCombatTriggers(triggers);
    HunterBuffPvpStrategy::InitNonCombatTriggers(triggers);
}

void SurvivalHunterBuffRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterBuffStrategy::InitCombatTriggers(triggers);
    HunterBuffRaidStrategy::InitCombatTriggers(triggers);
}

void SurvivalHunterBuffRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterBuffStrategy::InitNonCombatTriggers(triggers);
    HunterBuffRaidStrategy::InitNonCombatTriggers(triggers);
}

void SurvivalHunterBoostStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HunterBoostStrategy::InitCombatTriggers(triggers);
}

void SurvivalHunterBoostStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HunterBoostStrategy::InitNonCombatTriggers(triggers);
}

void SurvivalHunterBoostPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterBoostStrategy::InitCombatTriggers(triggers);
    HunterBoostPveStrategy::InitCombatTriggers(triggers);
}

void SurvivalHunterBoostPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterBoostStrategy::InitNonCombatTriggers(triggers);
    HunterBoostPveStrategy::InitNonCombatTriggers(triggers);
}

void SurvivalHunterBoostPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterBoostStrategy::InitCombatTriggers(triggers);
    HunterBoostPvpStrategy::InitCombatTriggers(triggers);
}

void SurvivalHunterBoostPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterBoostStrategy::InitNonCombatTriggers(triggers);
    HunterBoostPvpStrategy::InitNonCombatTriggers(triggers);
}

void SurvivalHunterBoostRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterBoostStrategy::InitCombatTriggers(triggers);
    HunterBoostRaidStrategy::InitCombatTriggers(triggers);
}

void SurvivalHunterBoostRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterBoostStrategy::InitNonCombatTriggers(triggers);
    HunterBoostRaidStrategy::InitNonCombatTriggers(triggers);
}

void SurvivalHunterCcStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HunterCcStrategy::InitCombatTriggers(triggers);
}

void SurvivalHunterCcStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HunterCcStrategy::InitNonCombatTriggers(triggers);
}

void SurvivalHunterCcPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterCcStrategy::InitCombatTriggers(triggers);
    HunterCcPveStrategy::InitCombatTriggers(triggers);
}

void SurvivalHunterCcPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterCcStrategy::InitNonCombatTriggers(triggers);
    HunterCcPveStrategy::InitNonCombatTriggers(triggers);
}

void SurvivalHunterCcPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterCcStrategy::InitCombatTriggers(triggers);
    HunterCcPvpStrategy::InitCombatTriggers(triggers);
}

void SurvivalHunterCcPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterCcStrategy::InitNonCombatTriggers(triggers);
    HunterCcPvpStrategy::InitNonCombatTriggers(triggers);
}

void SurvivalHunterCcRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterCcStrategy::InitCombatTriggers(triggers);
    HunterCcRaidStrategy::InitCombatTriggers(triggers);
}

void SurvivalHunterCcRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterCcStrategy::InitNonCombatTriggers(triggers);
    HunterCcRaidStrategy::InitNonCombatTriggers(triggers);
}

void SurvivalHunterStingStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HunterStingStrategy::InitCombatTriggers(triggers);
}

void SurvivalHunterStingPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterStingStrategy::InitCombatTriggers(triggers);
    HunterStingPveStrategy::InitCombatTriggers(triggers);
}

void SurvivalHunterStingPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterStingStrategy::InitCombatTriggers(triggers);
    HunterStingPvpStrategy::InitCombatTriggers(triggers);
}

void SurvivalHunterStingRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterStingStrategy::InitCombatTriggers(triggers);
    HunterStingRaidStrategy::InitCombatTriggers(triggers);
}

void SurvivalHunterAspectStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HunterAspectStrategy::InitCombatTriggers(triggers);
}

void SurvivalHunterAspectStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HunterAspectStrategy::InitNonCombatTriggers(triggers);
}

void SurvivalHunterAspectPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterAspectStrategy::InitCombatTriggers(triggers);
    HunterAspectPveStrategy::InitCombatTriggers(triggers);
}

void SurvivalHunterAspectPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterAspectStrategy::InitNonCombatTriggers(triggers);
    HunterAspectPveStrategy::InitNonCombatTriggers(triggers);
}

void SurvivalHunterAspectPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterAspectStrategy::InitCombatTriggers(triggers);
    HunterAspectPvpStrategy::InitCombatTriggers(triggers);
}

void SurvivalHunterAspectPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterAspectStrategy::InitNonCombatTriggers(triggers);
    HunterAspectPvpStrategy::InitNonCombatTriggers(triggers);
}

void SurvivalHunterAspectRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterAspectStrategy::InitCombatTriggers(triggers);
    HunterAspectRaidStrategy::InitCombatTriggers(triggers);
}

void SurvivalHunterAspectRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterAspectStrategy::InitNonCombatTriggers(triggers);
    HunterAspectRaidStrategy::InitNonCombatTriggers(triggers);
}

#endif
#ifdef MANGOSBOT_ONE // TBC

void SurvivalHunterStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HunterStrategy::InitCombatTriggers(triggers);
}

void SurvivalHunterStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HunterStrategy::InitNonCombatTriggers(triggers);
}

void SurvivalHunterStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    HunterStrategy::InitReactionTriggers(triggers);
}

void SurvivalHunterStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    HunterStrategy::InitDeadTriggers(triggers);
}

void SurvivalHunterPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterStrategy::InitCombatTriggers(triggers);
    HunterPveStrategy::InitCombatTriggers(triggers);
}

void SurvivalHunterPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterStrategy::InitNonCombatTriggers(triggers);
    HunterPveStrategy::InitNonCombatTriggers(triggers);
}

void SurvivalHunterPveStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterStrategy::InitReactionTriggers(triggers);
    HunterPveStrategy::InitReactionTriggers(triggers);
}

void SurvivalHunterPveStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterStrategy::InitDeadTriggers(triggers);
    HunterPveStrategy::InitDeadTriggers(triggers);
}

void SurvivalHunterPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterStrategy::InitCombatTriggers(triggers);
    HunterPvpStrategy::InitCombatTriggers(triggers);
}

void SurvivalHunterPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterStrategy::InitNonCombatTriggers(triggers);
    HunterPvpStrategy::InitNonCombatTriggers(triggers);
}

void SurvivalHunterPvpStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterStrategy::InitReactionTriggers(triggers);
    HunterPvpStrategy::InitReactionTriggers(triggers);
}

void SurvivalHunterPvpStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterStrategy::InitDeadTriggers(triggers);
    HunterPvpStrategy::InitDeadTriggers(triggers);
}

void SurvivalHunterRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterStrategy::InitCombatTriggers(triggers);
    HunterRaidStrategy::InitCombatTriggers(triggers);
}

void SurvivalHunterRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterStrategy::InitNonCombatTriggers(triggers);
    HunterRaidStrategy::InitNonCombatTriggers(triggers);
}

void SurvivalHunterRaidStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterStrategy::InitReactionTriggers(triggers);
    HunterRaidStrategy::InitReactionTriggers(triggers);
}

void SurvivalHunterRaidStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterStrategy::InitDeadTriggers(triggers);
    HunterRaidStrategy::InitDeadTriggers(triggers);
}

void SurvivalHunterAoeStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HunterAoeStrategy::InitCombatTriggers(triggers);
}

void SurvivalHunterAoeStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HunterAoeStrategy::InitNonCombatTriggers(triggers);
}

void SurvivalHunterAoePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterAoeStrategy::InitCombatTriggers(triggers);
    HunterAoePveStrategy::InitCombatTriggers(triggers);
}

void SurvivalHunterAoePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterAoeStrategy::InitNonCombatTriggers(triggers);
    HunterAoePveStrategy::InitNonCombatTriggers(triggers);
}

void SurvivalHunterAoePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterAoeStrategy::InitCombatTriggers(triggers);
    HunterAoePvpStrategy::InitCombatTriggers(triggers);
}

void SurvivalHunterAoePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterAoeStrategy::InitNonCombatTriggers(triggers);
    HunterAoePvpStrategy::InitNonCombatTriggers(triggers);
}

void SurvivalHunterAoeRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterAoeStrategy::InitCombatTriggers(triggers);
    HunterAoeRaidStrategy::InitCombatTriggers(triggers);
}

void SurvivalHunterAoeRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterAoeStrategy::InitNonCombatTriggers(triggers);
    HunterAoeRaidStrategy::InitNonCombatTriggers(triggers);
}

void SurvivalHunterBuffStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HunterBuffStrategy::InitCombatTriggers(triggers);
}

void SurvivalHunterBuffStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HunterBuffStrategy::InitNonCombatTriggers(triggers);
}

void SurvivalHunterBuffPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterBuffStrategy::InitCombatTriggers(triggers);
    HunterBuffPveStrategy::InitCombatTriggers(triggers);
}

void SurvivalHunterBuffPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterBuffStrategy::InitNonCombatTriggers(triggers);
    HunterBuffPveStrategy::InitNonCombatTriggers(triggers);
}

void SurvivalHunterBuffPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterBuffStrategy::InitCombatTriggers(triggers);
    HunterBuffPvpStrategy::InitCombatTriggers(triggers);
}

void SurvivalHunterBuffPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterBuffStrategy::InitNonCombatTriggers(triggers);
    HunterBuffPvpStrategy::InitNonCombatTriggers(triggers);
}

void SurvivalHunterBuffRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterBuffStrategy::InitCombatTriggers(triggers);
    HunterBuffRaidStrategy::InitCombatTriggers(triggers);
}

void SurvivalHunterBuffRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterBuffStrategy::InitNonCombatTriggers(triggers);
    HunterBuffRaidStrategy::InitNonCombatTriggers(triggers);
}

void SurvivalHunterBoostStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HunterBoostStrategy::InitCombatTriggers(triggers);
}

void SurvivalHunterBoostStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HunterBoostStrategy::InitNonCombatTriggers(triggers);
}

void SurvivalHunterBoostPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterBoostStrategy::InitCombatTriggers(triggers);
    HunterBoostPveStrategy::InitCombatTriggers(triggers);
}

void SurvivalHunterBoostPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterBoostStrategy::InitNonCombatTriggers(triggers);
    HunterBoostPveStrategy::InitNonCombatTriggers(triggers);
}

void SurvivalHunterBoostPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterBoostStrategy::InitCombatTriggers(triggers);
    HunterBoostPvpStrategy::InitCombatTriggers(triggers);
}

void SurvivalHunterBoostPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterBoostStrategy::InitNonCombatTriggers(triggers);
    HunterBoostPvpStrategy::InitNonCombatTriggers(triggers);
}

void SurvivalHunterBoostRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterBoostStrategy::InitCombatTriggers(triggers);
    HunterBoostRaidStrategy::InitCombatTriggers(triggers);
}

void SurvivalHunterBoostRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterBoostStrategy::InitNonCombatTriggers(triggers);
    HunterBoostRaidStrategy::InitNonCombatTriggers(triggers);
}

void SurvivalHunterCcStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HunterCcStrategy::InitCombatTriggers(triggers);
}

void SurvivalHunterCcStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HunterCcStrategy::InitNonCombatTriggers(triggers);
}

void SurvivalHunterCcPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterCcStrategy::InitCombatTriggers(triggers);
    HunterCcPveStrategy::InitCombatTriggers(triggers);
}

void SurvivalHunterCcPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterCcStrategy::InitNonCombatTriggers(triggers);
    HunterCcPveStrategy::InitNonCombatTriggers(triggers);
}

void SurvivalHunterCcPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterCcStrategy::InitCombatTriggers(triggers);
    HunterCcPvpStrategy::InitCombatTriggers(triggers);
}

void SurvivalHunterCcPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterCcStrategy::InitNonCombatTriggers(triggers);
    HunterCcPvpStrategy::InitNonCombatTriggers(triggers);
}

void SurvivalHunterCcRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterCcStrategy::InitCombatTriggers(triggers);
    HunterCcRaidStrategy::InitCombatTriggers(triggers);
}

void SurvivalHunterCcRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterCcStrategy::InitNonCombatTriggers(triggers);
    HunterCcRaidStrategy::InitNonCombatTriggers(triggers);
}

void SurvivalHunterStingStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HunterStingStrategy::InitCombatTriggers(triggers);
}

void SurvivalHunterStingPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterStingStrategy::InitCombatTriggers(triggers);
    HunterStingPveStrategy::InitCombatTriggers(triggers);
}

void SurvivalHunterStingPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterStingStrategy::InitCombatTriggers(triggers);
    HunterStingPvpStrategy::InitCombatTriggers(triggers);
}

void SurvivalHunterStingRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterStingStrategy::InitCombatTriggers(triggers);
    HunterStingRaidStrategy::InitCombatTriggers(triggers);
}

void SurvivalHunterAspectStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HunterAspectStrategy::InitCombatTriggers(triggers);
}

void SurvivalHunterAspectStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HunterAspectStrategy::InitNonCombatTriggers(triggers);
}

void SurvivalHunterAspectPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterAspectStrategy::InitCombatTriggers(triggers);
    HunterAspectPveStrategy::InitCombatTriggers(triggers);
}

void SurvivalHunterAspectPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterAspectStrategy::InitNonCombatTriggers(triggers);
    HunterAspectPveStrategy::InitNonCombatTriggers(triggers);
}

void SurvivalHunterAspectPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterAspectStrategy::InitCombatTriggers(triggers);
    HunterAspectPvpStrategy::InitCombatTriggers(triggers);
}

void SurvivalHunterAspectPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterAspectStrategy::InitNonCombatTriggers(triggers);
    HunterAspectPvpStrategy::InitNonCombatTriggers(triggers);
}

void SurvivalHunterAspectRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterAspectStrategy::InitCombatTriggers(triggers);
    HunterAspectRaidStrategy::InitCombatTriggers(triggers);
}

void SurvivalHunterAspectRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterAspectStrategy::InitNonCombatTriggers(triggers);
    HunterAspectRaidStrategy::InitNonCombatTriggers(triggers);
}

#endif
#ifdef MANGOSBOT_TWO // WOTLK

void SurvivalHunterStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HunterStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "black arrow",
        NextAction::array(0, new NextAction("black arrow", ACTION_HIGH + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "black arrow on snare target",
        NextAction::array(0, new NextAction("black arrow", ACTION_HIGH + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "explosive shot",
        NextAction::array(0, new NextAction("explosive shot", ACTION_NORMAL + 3), NULL)));
}

void SurvivalHunterStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HunterStrategy::InitNonCombatTriggers(triggers);
}

void SurvivalHunterStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    HunterStrategy::InitReactionTriggers(triggers);
}

void SurvivalHunterStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    HunterStrategy::InitDeadTriggers(triggers);
}

void SurvivalHunterPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterStrategy::InitCombatTriggers(triggers);
    HunterPveStrategy::InitCombatTriggers(triggers);
}

void SurvivalHunterPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterStrategy::InitNonCombatTriggers(triggers);
    HunterPveStrategy::InitNonCombatTriggers(triggers);
}

void SurvivalHunterPveStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterStrategy::InitReactionTriggers(triggers);
    HunterPveStrategy::InitReactionTriggers(triggers);
}

void SurvivalHunterPveStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterStrategy::InitDeadTriggers(triggers);
    HunterPveStrategy::InitDeadTriggers(triggers);
}

void SurvivalHunterPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterStrategy::InitCombatTriggers(triggers);
    HunterPvpStrategy::InitCombatTriggers(triggers);
}

void SurvivalHunterPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterStrategy::InitNonCombatTriggers(triggers);
    HunterPvpStrategy::InitNonCombatTriggers(triggers);
}

void SurvivalHunterPvpStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterStrategy::InitReactionTriggers(triggers);
    HunterPvpStrategy::InitReactionTriggers(triggers);
}

void SurvivalHunterPvpStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterStrategy::InitDeadTriggers(triggers);
    HunterPvpStrategy::InitDeadTriggers(triggers);
}

void SurvivalHunterRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterStrategy::InitCombatTriggers(triggers);
    HunterRaidStrategy::InitCombatTriggers(triggers);
}

void SurvivalHunterRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterStrategy::InitNonCombatTriggers(triggers);
    HunterRaidStrategy::InitNonCombatTriggers(triggers);
}

void SurvivalHunterRaidStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterStrategy::InitReactionTriggers(triggers);
    HunterRaidStrategy::InitReactionTriggers(triggers);
}

void SurvivalHunterRaidStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterStrategy::InitDeadTriggers(triggers);
    HunterRaidStrategy::InitDeadTriggers(triggers);
}

void SurvivalHunterAoeStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HunterAoeStrategy::InitCombatTriggers(triggers);
}

void SurvivalHunterAoeStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HunterAoeStrategy::InitNonCombatTriggers(triggers);
}

void SurvivalHunterAoePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterAoeStrategy::InitCombatTriggers(triggers);
    HunterAoePveStrategy::InitCombatTriggers(triggers);
}

void SurvivalHunterAoePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterAoeStrategy::InitNonCombatTriggers(triggers);
    HunterAoePveStrategy::InitNonCombatTriggers(triggers);
}

void SurvivalHunterAoePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterAoeStrategy::InitCombatTriggers(triggers);
    HunterAoePvpStrategy::InitCombatTriggers(triggers);
}

void SurvivalHunterAoePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterAoeStrategy::InitNonCombatTriggers(triggers);
    HunterAoePvpStrategy::InitNonCombatTriggers(triggers);
}

void SurvivalHunterAoeRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterAoeStrategy::InitCombatTriggers(triggers);
    HunterAoeRaidStrategy::InitCombatTriggers(triggers);
}

void SurvivalHunterAoeRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterAoeStrategy::InitNonCombatTriggers(triggers);
    HunterAoeRaidStrategy::InitNonCombatTriggers(triggers);
}

void SurvivalHunterBuffStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HunterBuffStrategy::InitCombatTriggers(triggers);
}

void SurvivalHunterBuffStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HunterBuffStrategy::InitNonCombatTriggers(triggers);
}

void SurvivalHunterBuffPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterBuffStrategy::InitCombatTriggers(triggers);
    HunterBuffPveStrategy::InitCombatTriggers(triggers);
}

void SurvivalHunterBuffPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterBuffStrategy::InitNonCombatTriggers(triggers);
    HunterBuffPveStrategy::InitNonCombatTriggers(triggers);
}

void SurvivalHunterBuffPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterBuffStrategy::InitCombatTriggers(triggers);
    HunterBuffPvpStrategy::InitCombatTriggers(triggers);
}

void SurvivalHunterBuffPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterBuffStrategy::InitNonCombatTriggers(triggers);
    HunterBuffPvpStrategy::InitNonCombatTriggers(triggers);
}

void SurvivalHunterBuffRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterBuffStrategy::InitCombatTriggers(triggers);
    HunterBuffRaidStrategy::InitCombatTriggers(triggers);
}

void SurvivalHunterBuffRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterBuffStrategy::InitNonCombatTriggers(triggers);
    HunterBuffRaidStrategy::InitNonCombatTriggers(triggers);
}

void SurvivalHunterBoostStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HunterBoostStrategy::InitCombatTriggers(triggers);
}

void SurvivalHunterBoostStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HunterBoostStrategy::InitNonCombatTriggers(triggers);
}

void SurvivalHunterBoostPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterBoostStrategy::InitCombatTriggers(triggers);
    HunterBoostPveStrategy::InitCombatTriggers(triggers);
}

void SurvivalHunterBoostPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterBoostStrategy::InitNonCombatTriggers(triggers);
    HunterBoostPveStrategy::InitNonCombatTriggers(triggers);
}

void SurvivalHunterBoostPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterBoostStrategy::InitCombatTriggers(triggers);
    HunterBoostPvpStrategy::InitCombatTriggers(triggers);
}

void SurvivalHunterBoostPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterBoostStrategy::InitNonCombatTriggers(triggers);
    HunterBoostPvpStrategy::InitNonCombatTriggers(triggers);
}

void SurvivalHunterBoostRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterBoostStrategy::InitCombatTriggers(triggers);
    HunterBoostRaidStrategy::InitCombatTriggers(triggers);
}

void SurvivalHunterBoostRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterBoostStrategy::InitNonCombatTriggers(triggers);
    HunterBoostRaidStrategy::InitNonCombatTriggers(triggers);
}

void SurvivalHunterCcStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HunterCcStrategy::InitCombatTriggers(triggers);
}

void SurvivalHunterCcStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HunterCcStrategy::InitNonCombatTriggers(triggers);
}

void SurvivalHunterCcPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterCcStrategy::InitCombatTriggers(triggers);
    HunterCcPveStrategy::InitCombatTriggers(triggers);
}

void SurvivalHunterCcPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterCcStrategy::InitNonCombatTriggers(triggers);
    HunterCcPveStrategy::InitNonCombatTriggers(triggers);
}

void SurvivalHunterCcPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterCcStrategy::InitCombatTriggers(triggers);
    HunterCcPvpStrategy::InitCombatTriggers(triggers);
}

void SurvivalHunterCcPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterCcStrategy::InitNonCombatTriggers(triggers);
    HunterCcPvpStrategy::InitNonCombatTriggers(triggers);
}

void SurvivalHunterCcRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterCcStrategy::InitCombatTriggers(triggers);
    HunterCcRaidStrategy::InitCombatTriggers(triggers);
}

void SurvivalHunterCcRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterCcStrategy::InitNonCombatTriggers(triggers);
    HunterCcRaidStrategy::InitNonCombatTriggers(triggers);
}

void SurvivalHunterStingStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HunterStingStrategy::InitCombatTriggers(triggers);
}

void SurvivalHunterStingPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterStingStrategy::InitCombatTriggers(triggers);
    HunterStingPveStrategy::InitCombatTriggers(triggers);
}

void SurvivalHunterStingPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterStingStrategy::InitCombatTriggers(triggers);
    HunterStingPvpStrategy::InitCombatTriggers(triggers);
}

void SurvivalHunterStingRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterStingStrategy::InitCombatTriggers(triggers);
    HunterStingRaidStrategy::InitCombatTriggers(triggers);
}

void SurvivalHunterAspectStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HunterAspectStrategy::InitCombatTriggers(triggers);
}

void SurvivalHunterAspectStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HunterAspectStrategy::InitNonCombatTriggers(triggers);
}

void SurvivalHunterAspectPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterAspectStrategy::InitCombatTriggers(triggers);
    HunterAspectPveStrategy::InitCombatTriggers(triggers);
}

void SurvivalHunterAspectPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterAspectStrategy::InitNonCombatTriggers(triggers);
    HunterAspectPveStrategy::InitNonCombatTriggers(triggers);
}

void SurvivalHunterAspectPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterAspectStrategy::InitCombatTriggers(triggers);
    HunterAspectPvpStrategy::InitCombatTriggers(triggers);
}

void SurvivalHunterAspectPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterAspectStrategy::InitNonCombatTriggers(triggers);
    HunterAspectPvpStrategy::InitNonCombatTriggers(triggers);
}

void SurvivalHunterAspectRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterAspectStrategy::InitCombatTriggers(triggers);
    HunterAspectRaidStrategy::InitCombatTriggers(triggers);
}

void SurvivalHunterAspectRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    SurvivalHunterAspectStrategy::InitNonCombatTriggers(triggers);
    HunterAspectRaidStrategy::InitNonCombatTriggers(triggers);
}

#endif