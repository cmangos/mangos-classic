
#include "playerbot/playerbot.h"
#include "WarlockTriggers.h"
#include "WarlockMultipliers.h"
#include "AfflictionWarlockStrategy.h"

using namespace ai;

#ifdef MANGOSBOT_ZERO // Vanilla

ai::NextAction** AfflictionWarlockStrategy::GetDefaultCombatActions()
{
    return NextAction::array(0, new NextAction("shadow bolt", ACTION_IDLE), NULL);
}

void AfflictionWarlockStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarlockStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "low mana",
        NextAction::array(0, new NextAction("dark pact", ACTION_HIGH + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "siphon life",
        NextAction::array(0, new NextAction("siphon life", ACTION_NORMAL), NULL)));
}

void AfflictionWarlockStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarlockStrategy::InitNonCombatTriggers(triggers);
}

void AfflictionWarlockStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    WarlockStrategy::InitReactionTriggers(triggers);
}

void AfflictionWarlockStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    WarlockStrategy::InitDeadTriggers(triggers);
}

void AfflictionWarlockPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockStrategy::InitCombatTriggers(triggers);
    WarlockPveStrategy::InitCombatTriggers(triggers);
}

void AfflictionWarlockPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockStrategy::InitNonCombatTriggers(triggers);
    WarlockPveStrategy::InitNonCombatTriggers(triggers);
}

void AfflictionWarlockPveStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockStrategy::InitReactionTriggers(triggers);
    WarlockPveStrategy::InitReactionTriggers(triggers);
}

void AfflictionWarlockPveStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockStrategy::InitDeadTriggers(triggers);
    WarlockPveStrategy::InitDeadTriggers(triggers);
}

void AfflictionWarlockPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockStrategy::InitCombatTriggers(triggers);
    WarlockPvpStrategy::InitCombatTriggers(triggers);
}

void AfflictionWarlockPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockStrategy::InitNonCombatTriggers(triggers);
    WarlockPvpStrategy::InitNonCombatTriggers(triggers);
}

void AfflictionWarlockPvpStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockStrategy::InitReactionTriggers(triggers);
    WarlockPvpStrategy::InitReactionTriggers(triggers);
}

void AfflictionWarlockPvpStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockStrategy::InitDeadTriggers(triggers);
    WarlockPvpStrategy::InitDeadTriggers(triggers);
}

void AfflictionWarlockRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockStrategy::InitCombatTriggers(triggers);
    WarlockRaidStrategy::InitCombatTriggers(triggers);
}

void AfflictionWarlockRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockStrategy::InitNonCombatTriggers(triggers);
    WarlockRaidStrategy::InitNonCombatTriggers(triggers);
}

void AfflictionWarlockRaidStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockStrategy::InitReactionTriggers(triggers);
    WarlockRaidStrategy::InitReactionTriggers(triggers);
}

void AfflictionWarlockRaidStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockStrategy::InitDeadTriggers(triggers);
    WarlockRaidStrategy::InitDeadTriggers(triggers);
}

void AfflictionWarlockAoeStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarlockAoeStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "siphon life on attacker",
        NextAction::array(0, new NextAction("siphon life on attacker", ACTION_HIGH + 3), NULL)));
}

void AfflictionWarlockAoeStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarlockAoeStrategy::InitNonCombatTriggers(triggers);
}

void AfflictionWarlockAoePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockAoeStrategy::InitCombatTriggers(triggers);
    WarlockAoePveStrategy::InitCombatTriggers(triggers);
}

void AfflictionWarlockAoePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockAoeStrategy::InitNonCombatTriggers(triggers);
    WarlockAoePveStrategy::InitNonCombatTriggers(triggers);
}

void AfflictionWarlockAoePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockAoeStrategy::InitCombatTriggers(triggers);
    WarlockAoePvpStrategy::InitCombatTriggers(triggers);
}

void AfflictionWarlockAoePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockAoeStrategy::InitNonCombatTriggers(triggers);
    WarlockAoePvpStrategy::InitNonCombatTriggers(triggers);
}

void AfflictionWarlockAoeRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockAoeStrategy::InitCombatTriggers(triggers);
    WarlockAoeRaidStrategy::InitCombatTriggers(triggers);
}

void AfflictionWarlockAoeRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockAoeStrategy::InitNonCombatTriggers(triggers);
    WarlockAoeRaidStrategy::InitNonCombatTriggers(triggers);
}

void AfflictionWarlockBuffStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarlockBuffStrategy::InitCombatTriggers(triggers);
}

void AfflictionWarlockBuffStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarlockBuffStrategy::InitNonCombatTriggers(triggers);
}

void AfflictionWarlockBuffPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockBuffStrategy::InitCombatTriggers(triggers);
    WarlockBuffPveStrategy::InitCombatTriggers(triggers);
}

void AfflictionWarlockBuffPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockBuffStrategy::InitNonCombatTriggers(triggers);
    WarlockBuffPveStrategy::InitNonCombatTriggers(triggers);
}

void AfflictionWarlockBuffPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockBuffStrategy::InitCombatTriggers(triggers);
    WarlockBuffPvpStrategy::InitCombatTriggers(triggers);
}

void AfflictionWarlockBuffPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockBuffStrategy::InitNonCombatTriggers(triggers);
    WarlockBuffPvpStrategy::InitNonCombatTriggers(triggers);
}

void AfflictionWarlockBuffRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockBuffStrategy::InitCombatTriggers(triggers);
    WarlockBuffRaidStrategy::InitCombatTriggers(triggers);
}

void AfflictionWarlockBuffRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockBuffStrategy::InitNonCombatTriggers(triggers);
    WarlockBuffRaidStrategy::InitNonCombatTriggers(triggers);
}

void AfflictionWarlockBoostStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarlockBoostStrategy::InitCombatTriggers(triggers);
}

void AfflictionWarlockBoostStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarlockBoostStrategy::InitNonCombatTriggers(triggers);
}

void AfflictionWarlockBoostPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockBoostStrategy::InitCombatTriggers(triggers);
    WarlockBoostPveStrategy::InitCombatTriggers(triggers);
}

void AfflictionWarlockBoostPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockBoostStrategy::InitNonCombatTriggers(triggers);
    WarlockBoostPveStrategy::InitNonCombatTriggers(triggers);
}

void AfflictionWarlockBoostPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockBoostStrategy::InitCombatTriggers(triggers);
    WarlockBoostPvpStrategy::InitCombatTriggers(triggers);
}

void AfflictionWarlockBoostPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockBoostStrategy::InitNonCombatTriggers(triggers);
    WarlockBoostPvpStrategy::InitNonCombatTriggers(triggers);
}

void AfflictionWarlockBoostRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockBoostStrategy::InitCombatTriggers(triggers);
    WarlockBoostRaidStrategy::InitCombatTriggers(triggers);
}

void AfflictionWarlockBoostRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockBoostStrategy::InitNonCombatTriggers(triggers);
    WarlockBoostRaidStrategy::InitNonCombatTriggers(triggers);
}

void AfflictionWarlockCcStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarlockCcStrategy::InitCombatTriggers(triggers);
}

void AfflictionWarlockCcStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarlockCcStrategy::InitNonCombatTriggers(triggers);
}

void AfflictionWarlockCcPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockCcStrategy::InitCombatTriggers(triggers);
    WarlockCcPveStrategy::InitCombatTriggers(triggers);
}

void AfflictionWarlockCcPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockCcStrategy::InitNonCombatTriggers(triggers);
    WarlockCcPveStrategy::InitNonCombatTriggers(triggers);
}

void AfflictionWarlockCcPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockCcStrategy::InitCombatTriggers(triggers);
    WarlockCcPvpStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "enemy ten yards",
        NextAction::array(0, new NextAction("curse of exhaustion", ACTION_INTERRUPT), NULL)));
}

void AfflictionWarlockCcPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockCcStrategy::InitNonCombatTriggers(triggers);
    WarlockCcPvpStrategy::InitNonCombatTriggers(triggers);
}

void AfflictionWarlockCcRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockCcStrategy::InitCombatTriggers(triggers);
    WarlockCcRaidStrategy::InitCombatTriggers(triggers);
}

void AfflictionWarlockCcRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockCcStrategy::InitNonCombatTriggers(triggers);
    WarlockCcRaidStrategy::InitNonCombatTriggers(triggers);
}

void AfflictionWarlockPetStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarlockPetStrategy::InitNonCombatTriggers(triggers);
}

void AfflictionWarlockPetStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarlockPetStrategy::InitNonCombatTriggers(triggers);
}

void AfflictionWarlockPetPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockPetStrategy::InitCombatTriggers(triggers);
    WarlockPetPveStrategy::InitCombatTriggers(triggers);
}

void AfflictionWarlockPetPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockPetStrategy::InitNonCombatTriggers(triggers);
    WarlockPetPveStrategy::InitNonCombatTriggers(triggers);
}

void AfflictionWarlockPetPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockPetStrategy::InitCombatTriggers(triggers);
    WarlockPetPvpStrategy::InitCombatTriggers(triggers);
}

void AfflictionWarlockPetPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockPetStrategy::InitNonCombatTriggers(triggers);
    WarlockPetPvpStrategy::InitNonCombatTriggers(triggers);
}

void AfflictionWarlockPetRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockPetStrategy::InitCombatTriggers(triggers);
    WarlockPetRaidStrategy::InitCombatTriggers(triggers);
}

void AfflictionWarlockPetRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockPetStrategy::InitNonCombatTriggers(triggers);
    WarlockPetRaidStrategy::InitNonCombatTriggers(triggers);
}

void AfflictionWarlockCursesStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarlockCursesStrategy::InitCombatTriggers(triggers);
}

void AfflictionWarlockCursesPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockCursesStrategy::InitCombatTriggers(triggers);
    WarlockCursesPveStrategy::InitCombatTriggers(triggers);
}

void AfflictionWarlockCursesPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockCursesStrategy::InitCombatTriggers(triggers);
    WarlockCursesPvpStrategy::InitCombatTriggers(triggers);
}

void AfflictionWarlockCursesRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockCursesStrategy::InitCombatTriggers(triggers);
    WarlockCursesRaidStrategy::InitCombatTriggers(triggers);
}

#endif
#ifdef MANGOSBOT_ONE // TBC

ai::NextAction** AfflictionWarlockStrategy::GetDefaultCombatActions()
{
    return NextAction::array(0, new NextAction("shadow bolt", ACTION_IDLE), NULL);
}

void AfflictionWarlockStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarlockStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "low mana",
        NextAction::array(0, new NextAction("dark pact", ACTION_HIGH + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "unstable affliction",
        NextAction::array(0, new NextAction("unstable affliction", ACTION_NORMAL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "siphon life",
        NextAction::array(0, new NextAction("siphon life", ACTION_NORMAL), NULL)));
}

void AfflictionWarlockStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarlockStrategy::InitNonCombatTriggers(triggers);
}

void AfflictionWarlockStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    WarlockStrategy::InitReactionTriggers(triggers);
}

void AfflictionWarlockStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    WarlockStrategy::InitDeadTriggers(triggers);
}

void AfflictionWarlockPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockStrategy::InitCombatTriggers(triggers);
    WarlockPveStrategy::InitCombatTriggers(triggers);
}

void AfflictionWarlockPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockStrategy::InitNonCombatTriggers(triggers);
    WarlockPveStrategy::InitNonCombatTriggers(triggers);
}

void AfflictionWarlockPveStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockStrategy::InitReactionTriggers(triggers);
    WarlockPveStrategy::InitReactionTriggers(triggers);
}

void AfflictionWarlockPveStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockStrategy::InitDeadTriggers(triggers);
    WarlockPveStrategy::InitDeadTriggers(triggers);
}

void AfflictionWarlockPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockStrategy::InitCombatTriggers(triggers);
    WarlockPvpStrategy::InitCombatTriggers(triggers);
}

void AfflictionWarlockPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockStrategy::InitNonCombatTriggers(triggers);
    WarlockPvpStrategy::InitNonCombatTriggers(triggers);
}

void AfflictionWarlockPvpStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockStrategy::InitReactionTriggers(triggers);
    WarlockPvpStrategy::InitReactionTriggers(triggers);
}

void AfflictionWarlockPvpStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockStrategy::InitDeadTriggers(triggers);
    WarlockPvpStrategy::InitDeadTriggers(triggers);
}

void AfflictionWarlockRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockStrategy::InitCombatTriggers(triggers);
    WarlockRaidStrategy::InitCombatTriggers(triggers);
}

void AfflictionWarlockRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockStrategy::InitNonCombatTriggers(triggers);
    WarlockRaidStrategy::InitNonCombatTriggers(triggers);
}

void AfflictionWarlockRaidStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockStrategy::InitReactionTriggers(triggers);
    WarlockRaidStrategy::InitReactionTriggers(triggers);
}

void AfflictionWarlockRaidStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockStrategy::InitDeadTriggers(triggers);
    WarlockRaidStrategy::InitDeadTriggers(triggers);
}

void AfflictionWarlockAoeStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarlockAoeStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "unstable affliction on attacker",
        NextAction::array(0, new NextAction("unstable affliction on attacker", ACTION_HIGH + 4), NULL)));

    triggers.push_back(new TriggerNode(
        "siphon life on attacker",
        NextAction::array(0, new NextAction("siphon life on attacker", ACTION_HIGH + 3), NULL)));
}

void AfflictionWarlockAoeStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarlockAoeStrategy::InitNonCombatTriggers(triggers);
}

void AfflictionWarlockAoePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockAoeStrategy::InitCombatTriggers(triggers);
    WarlockAoePveStrategy::InitCombatTriggers(triggers);
}

void AfflictionWarlockAoePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockAoeStrategy::InitNonCombatTriggers(triggers);
    WarlockAoePveStrategy::InitNonCombatTriggers(triggers);
}

void AfflictionWarlockAoePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockAoeStrategy::InitCombatTriggers(triggers);
    WarlockAoePvpStrategy::InitCombatTriggers(triggers);
}

void AfflictionWarlockAoePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockAoeStrategy::InitNonCombatTriggers(triggers);
    WarlockAoePvpStrategy::InitNonCombatTriggers(triggers);
}

void AfflictionWarlockAoeRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockAoeStrategy::InitCombatTriggers(triggers);
    WarlockAoeRaidStrategy::InitCombatTriggers(triggers);
}

void AfflictionWarlockAoeRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockAoeStrategy::InitNonCombatTriggers(triggers);
    WarlockAoeRaidStrategy::InitNonCombatTriggers(triggers);
}

void AfflictionWarlockBuffStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarlockBuffStrategy::InitCombatTriggers(triggers);
}

void AfflictionWarlockBuffStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarlockBuffStrategy::InitNonCombatTriggers(triggers);
}

void AfflictionWarlockBuffPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockBuffStrategy::InitCombatTriggers(triggers);
    WarlockBuffPveStrategy::InitCombatTriggers(triggers);
}

void AfflictionWarlockBuffPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockBuffStrategy::InitNonCombatTriggers(triggers);
    WarlockBuffPveStrategy::InitNonCombatTriggers(triggers);
}

void AfflictionWarlockBuffPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockBuffStrategy::InitCombatTriggers(triggers);
    WarlockBuffPvpStrategy::InitCombatTriggers(triggers);
}

void AfflictionWarlockBuffPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockBuffStrategy::InitNonCombatTriggers(triggers);
    WarlockBuffPvpStrategy::InitNonCombatTriggers(triggers);
}

void AfflictionWarlockBuffRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockBuffStrategy::InitCombatTriggers(triggers);
    WarlockBuffRaidStrategy::InitCombatTriggers(triggers);
}

void AfflictionWarlockBuffRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockBuffStrategy::InitNonCombatTriggers(triggers);
    WarlockBuffRaidStrategy::InitNonCombatTriggers(triggers);
}

void AfflictionWarlockBoostStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarlockBoostStrategy::InitCombatTriggers(triggers);
}

void AfflictionWarlockBoostStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarlockBoostStrategy::InitNonCombatTriggers(triggers);
}

void AfflictionWarlockBoostPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockBoostStrategy::InitCombatTriggers(triggers);
    WarlockBoostPveStrategy::InitCombatTriggers(triggers);
}

void AfflictionWarlockBoostPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockBoostStrategy::InitNonCombatTriggers(triggers);
    WarlockBoostPveStrategy::InitNonCombatTriggers(triggers);
}

void AfflictionWarlockBoostPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockBoostStrategy::InitCombatTriggers(triggers);
    WarlockBoostPvpStrategy::InitCombatTriggers(triggers);
}

void AfflictionWarlockBoostPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockBoostStrategy::InitNonCombatTriggers(triggers);
    WarlockBoostPvpStrategy::InitNonCombatTriggers(triggers);
}

void AfflictionWarlockBoostRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockBoostStrategy::InitCombatTriggers(triggers);
    WarlockBoostRaidStrategy::InitCombatTriggers(triggers);
}

void AfflictionWarlockBoostRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockBoostStrategy::InitNonCombatTriggers(triggers);
    WarlockBoostRaidStrategy::InitNonCombatTriggers(triggers);
}

void AfflictionWarlockCcStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarlockCcStrategy::InitCombatTriggers(triggers);
}

void AfflictionWarlockCcStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarlockCcStrategy::InitNonCombatTriggers(triggers);
}

void AfflictionWarlockCcPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockCcStrategy::InitCombatTriggers(triggers);
    WarlockCcPveStrategy::InitCombatTriggers(triggers);
}

void AfflictionWarlockCcPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockCcStrategy::InitNonCombatTriggers(triggers);
    WarlockCcPveStrategy::InitNonCombatTriggers(triggers);
}

void AfflictionWarlockCcPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockCcStrategy::InitCombatTriggers(triggers);
    WarlockCcPvpStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "enemy ten yards",
        NextAction::array(0, new NextAction("curse of exhaustion", ACTION_INTERRUPT), NULL)));
}

void AfflictionWarlockCcPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockCcStrategy::InitNonCombatTriggers(triggers);
    WarlockCcPvpStrategy::InitNonCombatTriggers(triggers);
}

void AfflictionWarlockCcRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockCcStrategy::InitCombatTriggers(triggers);
    WarlockCcRaidStrategy::InitCombatTriggers(triggers);
}

void AfflictionWarlockCcRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockCcStrategy::InitNonCombatTriggers(triggers);
    WarlockCcRaidStrategy::InitNonCombatTriggers(triggers);
}

void AfflictionWarlockPetStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarlockPetStrategy::InitNonCombatTriggers(triggers);
}

void AfflictionWarlockPetStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarlockPetStrategy::InitNonCombatTriggers(triggers);
}

void AfflictionWarlockPetPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockPetStrategy::InitCombatTriggers(triggers);
    WarlockPetPveStrategy::InitCombatTriggers(triggers);
}

void AfflictionWarlockPetPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockPetStrategy::InitNonCombatTriggers(triggers);
    WarlockPetPveStrategy::InitNonCombatTriggers(triggers);
}

void AfflictionWarlockPetPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockPetStrategy::InitCombatTriggers(triggers);
    WarlockPetPvpStrategy::InitCombatTriggers(triggers);
}

void AfflictionWarlockPetPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockPetStrategy::InitNonCombatTriggers(triggers);
    WarlockPetPvpStrategy::InitNonCombatTriggers(triggers);
}

void AfflictionWarlockPetRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockPetStrategy::InitCombatTriggers(triggers);
    WarlockPetRaidStrategy::InitCombatTriggers(triggers);
}

void AfflictionWarlockPetRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockPetStrategy::InitNonCombatTriggers(triggers);
    WarlockPetRaidStrategy::InitNonCombatTriggers(triggers);
}

void AfflictionWarlockCursesStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarlockCursesStrategy::InitCombatTriggers(triggers);
}

void AfflictionWarlockCursesPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockCursesStrategy::InitCombatTriggers(triggers);
    WarlockCursesPveStrategy::InitCombatTriggers(triggers);
}

void AfflictionWarlockCursesPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockCursesStrategy::InitCombatTriggers(triggers);
    WarlockCursesPvpStrategy::InitCombatTriggers(triggers);
}

void AfflictionWarlockCursesRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockCursesStrategy::InitCombatTriggers(triggers);
    WarlockCursesRaidStrategy::InitCombatTriggers(triggers);
}

#endif
#ifdef MANGOSBOT_TWO // WOTLK

ai::NextAction** AfflictionWarlockStrategy::GetDefaultCombatActions()
{
    return NextAction::array(0, new NextAction("shadow bolt", ACTION_IDLE), NULL);
}

void AfflictionWarlockStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarlockStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "low mana",
        NextAction::array(0, new NextAction("dark pact", ACTION_HIGH + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "unstable affliction",
        NextAction::array(0, new NextAction("unstable affliction", ACTION_NORMAL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "siphon life",
        NextAction::array(0, new NextAction("siphon life", ACTION_NORMAL), NULL)));
}

void AfflictionWarlockStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarlockStrategy::InitNonCombatTriggers(triggers);
}

void AfflictionWarlockStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    WarlockStrategy::InitReactionTriggers(triggers);
}

void AfflictionWarlockStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    WarlockStrategy::InitDeadTriggers(triggers);
}

void AfflictionWarlockPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockStrategy::InitCombatTriggers(triggers);
    WarlockPveStrategy::InitCombatTriggers(triggers);
}

void AfflictionWarlockPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockStrategy::InitNonCombatTriggers(triggers);
    WarlockPveStrategy::InitNonCombatTriggers(triggers);
}

void AfflictionWarlockPveStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockStrategy::InitReactionTriggers(triggers);
    WarlockPveStrategy::InitReactionTriggers(triggers);
}

void AfflictionWarlockPveStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockStrategy::InitDeadTriggers(triggers);
    WarlockPveStrategy::InitDeadTriggers(triggers);
}

void AfflictionWarlockPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockStrategy::InitCombatTriggers(triggers);
    WarlockPvpStrategy::InitCombatTriggers(triggers);
}

void AfflictionWarlockPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockStrategy::InitNonCombatTriggers(triggers);
    WarlockPvpStrategy::InitNonCombatTriggers(triggers);
}

void AfflictionWarlockPvpStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockStrategy::InitReactionTriggers(triggers);
    WarlockPvpStrategy::InitReactionTriggers(triggers);
}

void AfflictionWarlockPvpStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockStrategy::InitDeadTriggers(triggers);
    WarlockPvpStrategy::InitDeadTriggers(triggers);
}

void AfflictionWarlockRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockStrategy::InitCombatTriggers(triggers);
    WarlockRaidStrategy::InitCombatTriggers(triggers);
}

void AfflictionWarlockRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockStrategy::InitNonCombatTriggers(triggers);
    WarlockRaidStrategy::InitNonCombatTriggers(triggers);
}

void AfflictionWarlockRaidStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockStrategy::InitReactionTriggers(triggers);
    WarlockRaidStrategy::InitReactionTriggers(triggers);
}

void AfflictionWarlockRaidStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockStrategy::InitDeadTriggers(triggers);
    WarlockRaidStrategy::InitDeadTriggers(triggers);
}

void AfflictionWarlockAoeStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarlockAoeStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "unstable affliction on attacker",
        NextAction::array(0, new NextAction("unstable affliction on attacker", ACTION_HIGH + 4), NULL)));

    triggers.push_back(new TriggerNode(
        "siphon life on attacker",
        NextAction::array(0, new NextAction("siphon life on attacker", ACTION_HIGH + 3), NULL)));
}

void AfflictionWarlockAoeStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarlockAoeStrategy::InitNonCombatTriggers(triggers);
}

void AfflictionWarlockAoePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockAoeStrategy::InitCombatTriggers(triggers);
    WarlockAoePveStrategy::InitCombatTriggers(triggers);
}

void AfflictionWarlockAoePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockAoeStrategy::InitNonCombatTriggers(triggers);
    WarlockAoePveStrategy::InitNonCombatTriggers(triggers);
}

void AfflictionWarlockAoePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockAoeStrategy::InitCombatTriggers(triggers);
    WarlockAoePvpStrategy::InitCombatTriggers(triggers);
}

void AfflictionWarlockAoePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockAoeStrategy::InitNonCombatTriggers(triggers);
    WarlockAoePvpStrategy::InitNonCombatTriggers(triggers);
}

void AfflictionWarlockAoeRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockAoeStrategy::InitCombatTriggers(triggers);
    WarlockAoeRaidStrategy::InitCombatTriggers(triggers);
}

void AfflictionWarlockAoeRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockAoeStrategy::InitNonCombatTriggers(triggers);
    WarlockAoeRaidStrategy::InitNonCombatTriggers(triggers);
}

void AfflictionWarlockBuffStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarlockBuffStrategy::InitCombatTriggers(triggers);
}

void AfflictionWarlockBuffStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarlockBuffStrategy::InitNonCombatTriggers(triggers);
}

void AfflictionWarlockBuffPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockBuffStrategy::InitCombatTriggers(triggers);
    WarlockBuffPveStrategy::InitCombatTriggers(triggers);
}

void AfflictionWarlockBuffPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockBuffStrategy::InitNonCombatTriggers(triggers);
    WarlockBuffPveStrategy::InitNonCombatTriggers(triggers);
}

void AfflictionWarlockBuffPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockBuffStrategy::InitCombatTriggers(triggers);
    WarlockBuffPvpStrategy::InitCombatTriggers(triggers);
}

void AfflictionWarlockBuffPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockBuffStrategy::InitNonCombatTriggers(triggers);
    WarlockBuffPvpStrategy::InitNonCombatTriggers(triggers);
}

void AfflictionWarlockBuffRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockBuffStrategy::InitCombatTriggers(triggers);
    WarlockBuffRaidStrategy::InitCombatTriggers(triggers);
}

void AfflictionWarlockBuffRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockBuffStrategy::InitNonCombatTriggers(triggers);
    WarlockBuffRaidStrategy::InitNonCombatTriggers(triggers);
}

void AfflictionWarlockBoostStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarlockBoostStrategy::InitCombatTriggers(triggers);
}

void AfflictionWarlockBoostStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarlockBoostStrategy::InitNonCombatTriggers(triggers);
}

void AfflictionWarlockBoostPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockBoostStrategy::InitCombatTriggers(triggers);
    WarlockBoostPveStrategy::InitCombatTriggers(triggers);
}

void AfflictionWarlockBoostPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockBoostStrategy::InitNonCombatTriggers(triggers);
    WarlockBoostPveStrategy::InitNonCombatTriggers(triggers);
}

void AfflictionWarlockBoostPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockBoostStrategy::InitCombatTriggers(triggers);
    WarlockBoostPvpStrategy::InitCombatTriggers(triggers);
}

void AfflictionWarlockBoostPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockBoostStrategy::InitNonCombatTriggers(triggers);
    WarlockBoostPvpStrategy::InitNonCombatTriggers(triggers);
}

void AfflictionWarlockBoostRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockBoostStrategy::InitCombatTriggers(triggers);
    WarlockBoostRaidStrategy::InitCombatTriggers(triggers);
}

void AfflictionWarlockBoostRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockBoostStrategy::InitNonCombatTriggers(triggers);
    WarlockBoostRaidStrategy::InitNonCombatTriggers(triggers);
}

void AfflictionWarlockCcStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarlockCcStrategy::InitCombatTriggers(triggers);
}

void AfflictionWarlockCcStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarlockCcStrategy::InitNonCombatTriggers(triggers);
}

void AfflictionWarlockCcPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockCcStrategy::InitCombatTriggers(triggers);
    WarlockCcPveStrategy::InitCombatTriggers(triggers);
}

void AfflictionWarlockCcPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockCcStrategy::InitNonCombatTriggers(triggers);
    WarlockCcPveStrategy::InitNonCombatTriggers(triggers);
}

void AfflictionWarlockCcPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockCcStrategy::InitCombatTriggers(triggers);
    WarlockCcPvpStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "enemy ten yards",
        NextAction::array(0, new NextAction("curse of exhaustion", ACTION_INTERRUPT), NULL)));
}

void AfflictionWarlockCcPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockCcStrategy::InitNonCombatTriggers(triggers);
    WarlockCcPvpStrategy::InitNonCombatTriggers(triggers);
}

void AfflictionWarlockCcRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockCcStrategy::InitCombatTriggers(triggers);
    WarlockCcRaidStrategy::InitCombatTriggers(triggers);
}

void AfflictionWarlockCcRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockCcStrategy::InitNonCombatTriggers(triggers);
    WarlockCcRaidStrategy::InitNonCombatTriggers(triggers);
}

void AfflictionWarlockPetStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarlockPetStrategy::InitNonCombatTriggers(triggers);
}

void AfflictionWarlockPetStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarlockPetStrategy::InitNonCombatTriggers(triggers);
}

void AfflictionWarlockPetPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockPetStrategy::InitCombatTriggers(triggers);
    WarlockPetPveStrategy::InitCombatTriggers(triggers);
}

void AfflictionWarlockPetPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockPetStrategy::InitNonCombatTriggers(triggers);
    WarlockPetPveStrategy::InitNonCombatTriggers(triggers);
}

void AfflictionWarlockPetPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockPetStrategy::InitCombatTriggers(triggers);
    WarlockPetPvpStrategy::InitCombatTriggers(triggers);
}

void AfflictionWarlockPetPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockPetStrategy::InitNonCombatTriggers(triggers);
    WarlockPetPvpStrategy::InitNonCombatTriggers(triggers);
}

void AfflictionWarlockPetRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockPetStrategy::InitCombatTriggers(triggers);
    WarlockPetRaidStrategy::InitCombatTriggers(triggers);
}

void AfflictionWarlockPetRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockPetStrategy::InitNonCombatTriggers(triggers);
    WarlockPetRaidStrategy::InitNonCombatTriggers(triggers);
}

void AfflictionWarlockCursesStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarlockCursesStrategy::InitCombatTriggers(triggers);
}

void AfflictionWarlockCursesPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockCursesStrategy::InitCombatTriggers(triggers);
    WarlockCursesPveStrategy::InitCombatTriggers(triggers);
}

void AfflictionWarlockCursesPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockCursesStrategy::InitCombatTriggers(triggers);
    WarlockCursesPvpStrategy::InitCombatTriggers(triggers);
}

void AfflictionWarlockCursesRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AfflictionWarlockCursesStrategy::InitCombatTriggers(triggers);
    WarlockCursesRaidStrategy::InitCombatTriggers(triggers);
}

#endif