
#include "playerbot/playerbot.h"
#include "MageMultipliers.h"
#include "FrostMageStrategy.h"

using namespace ai;

#ifdef MANGOSBOT_ZERO // Vanilla

NextAction** FrostMageStrategy::GetDefaultCombatActions()
{
    return NextAction::array(0, new NextAction("frostbolt", ACTION_IDLE), NULL);
}

void FrostMageStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    MageStrategy::InitCombatTriggers(triggers);
}

void FrostMageStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    MageStrategy::InitNonCombatTriggers(triggers);
}

void FrostMageStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    MageStrategy::InitReactionTriggers(triggers);
}

void FrostMageStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    MageStrategy::InitDeadTriggers(triggers);
}

void FrostMagePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageStrategy::InitCombatTriggers(triggers);
    MagePveStrategy::InitCombatTriggers(triggers);
}

void FrostMagePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageStrategy::InitNonCombatTriggers(triggers);
    MagePveStrategy::InitNonCombatTriggers(triggers);
}

void FrostMagePveStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageStrategy::InitReactionTriggers(triggers);
    MagePveStrategy::InitReactionTriggers(triggers);
}

void FrostMagePveStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageStrategy::InitDeadTriggers(triggers);
    MagePveStrategy::InitDeadTriggers(triggers);
}

void FrostMagePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageStrategy::InitCombatTriggers(triggers);
    MagePvpStrategy::InitCombatTriggers(triggers);
}

void FrostMagePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageStrategy::InitNonCombatTriggers(triggers);
    MagePvpStrategy::InitNonCombatTriggers(triggers);
}

void FrostMagePvpStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageStrategy::InitReactionTriggers(triggers);
    MagePvpStrategy::InitReactionTriggers(triggers);
}

void FrostMagePvpStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageStrategy::InitDeadTriggers(triggers);
    MagePvpStrategy::InitDeadTriggers(triggers);
}

void FrostMageRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageStrategy::InitCombatTriggers(triggers);
    MageRaidStrategy::InitCombatTriggers(triggers);
}

void FrostMageRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageStrategy::InitNonCombatTriggers(triggers);
    MageRaidStrategy::InitNonCombatTriggers(triggers);
}

void FrostMageRaidStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageStrategy::InitReactionTriggers(triggers);
    MageRaidStrategy::InitReactionTriggers(triggers);
}

void FrostMageRaidStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageStrategy::InitDeadTriggers(triggers);
    MageRaidStrategy::InitDeadTriggers(triggers);
}

void FrostMageAoeStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    MageAoeStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "ranged medium aoe",
        NextAction::array(0, new NextAction("blizzard", ACTION_HIGH + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "ranged medium aoe",
        NextAction::array(0, new NextAction("flamestrike", ACTION_HIGH), NULL)));
}

void FrostMageAoeStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    MageAoeStrategy::InitNonCombatTriggers(triggers);
}

void FrostMageAoePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageAoeStrategy::InitCombatTriggers(triggers);
    MageAoePveStrategy::InitCombatTriggers(triggers);
}

void FrostMageAoePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageAoeStrategy::InitNonCombatTriggers(triggers);
    MageAoePveStrategy::InitNonCombatTriggers(triggers);
}

void FrostMageAoePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageAoeStrategy::InitCombatTriggers(triggers);
    MageAoePvpStrategy::InitCombatTriggers(triggers);
}

void FrostMageAoePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageAoeStrategy::InitNonCombatTriggers(triggers);
    MageAoePvpStrategy::InitNonCombatTriggers(triggers);
}

void FrostMageAoeRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageAoeStrategy::InitCombatTriggers(triggers);
    MageAoeRaidStrategy::InitCombatTriggers(triggers);
}

void FrostMageAoeRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageAoeStrategy::InitNonCombatTriggers(triggers);
    MageAoeRaidStrategy::InitNonCombatTriggers(triggers);
}

void FrostMageBuffStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    MageBuffStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "ice barrier",
        NextAction::array(0, new NextAction("ice barrier", ACTION_HIGH), NULL)));
}

void FrostMageBuffStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    MageBuffStrategy::InitNonCombatTriggers(triggers);
}

void FrostMageBuffPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageBuffStrategy::InitCombatTriggers(triggers);
    MageBuffPveStrategy::InitCombatTriggers(triggers);
}

void FrostMageBuffPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageBuffStrategy::InitNonCombatTriggers(triggers);
    MageBuffPveStrategy::InitNonCombatTriggers(triggers);
}

void FrostMageBuffPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageBuffStrategy::InitCombatTriggers(triggers);
    MageBuffPvpStrategy::InitCombatTriggers(triggers);
}

void FrostMageBuffPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageBuffStrategy::InitNonCombatTriggers(triggers);
    MageBuffPvpStrategy::InitNonCombatTriggers(triggers);
}

void FrostMageBuffRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageBuffStrategy::InitCombatTriggers(triggers);
    MageBuffRaidStrategy::InitCombatTriggers(triggers);
}

void FrostMageBuffRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageBuffStrategy::InitNonCombatTriggers(triggers);
    MageBuffRaidStrategy::InitNonCombatTriggers(triggers);
}

void FrostMageBoostStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    MageBoostStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "cold snap",
        NextAction::array(0, new NextAction("cold snap", ACTION_HIGH + 3), NULL)));
}

void FrostMageBoostStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    MageBoostStrategy::InitNonCombatTriggers(triggers);
}

void FrostMageBoostPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageBoostStrategy::InitCombatTriggers(triggers);
    MageBoostPveStrategy::InitCombatTriggers(triggers);
}

void FrostMageBoostPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageBoostStrategy::InitNonCombatTriggers(triggers);
    MageBoostPveStrategy::InitNonCombatTriggers(triggers);
}

void FrostMageBoostPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageBoostStrategy::InitCombatTriggers(triggers);
    MageBoostPvpStrategy::InitCombatTriggers(triggers);
}

void FrostMageBoostPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageBoostStrategy::InitNonCombatTriggers(triggers);
    MageBoostPvpStrategy::InitNonCombatTriggers(triggers);
}

void FrostMageBoostRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageBoostStrategy::InitCombatTriggers(triggers);
    MageBoostRaidStrategy::InitCombatTriggers(triggers);
}

void FrostMageBoostRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageBoostStrategy::InitNonCombatTriggers(triggers);
    MageBoostRaidStrategy::InitNonCombatTriggers(triggers);
}

void FrostMageCcStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    MageCcStrategy::InitCombatTriggers(triggers);
}

void FrostMageCcStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    MageCcStrategy::InitNonCombatTriggers(triggers);
}

void FrostMageCcPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageCcStrategy::InitCombatTriggers(triggers);
    MageCcPveStrategy::InitCombatTriggers(triggers);
}

void FrostMageCcPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageCcStrategy::InitNonCombatTriggers(triggers);
    MageCcPveStrategy::InitNonCombatTriggers(triggers);
}

void FrostMageCcPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageCcStrategy::InitCombatTriggers(triggers);
    MageCcPvpStrategy::InitCombatTriggers(triggers);
}

void FrostMageCcPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageCcStrategy::InitNonCombatTriggers(triggers);
    MageCcPvpStrategy::InitNonCombatTriggers(triggers);
}

void FrostMageCcRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageCcStrategy::InitCombatTriggers(triggers);
    MageCcRaidStrategy::InitCombatTriggers(triggers);
}

void FrostMageCcRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageCcStrategy::InitNonCombatTriggers(triggers);
    MageCcRaidStrategy::InitNonCombatTriggers(triggers);
}

void FrostMageCureStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    MageCureStrategy::InitCombatTriggers(triggers);
}

void FrostMageCureStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    MageCureStrategy::InitNonCombatTriggers(triggers);
}

void FrostMageCurePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageCureStrategy::InitCombatTriggers(triggers);
    MageCurePveStrategy::InitCombatTriggers(triggers);
}

void FrostMageCurePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageCureStrategy::InitNonCombatTriggers(triggers);
    MageCurePveStrategy::InitNonCombatTriggers(triggers);
}

void FrostMageCurePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageCureStrategy::InitCombatTriggers(triggers);
    MageCurePvpStrategy::InitCombatTriggers(triggers);
}

void FrostMageCurePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageCureStrategy::InitNonCombatTriggers(triggers);
    MageCurePvpStrategy::InitNonCombatTriggers(triggers);
}

void FrostMageCureRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageCureStrategy::InitCombatTriggers(triggers);
    MageCureRaidStrategy::InitCombatTriggers(triggers);
}

void FrostMageCureRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageCureStrategy::InitNonCombatTriggers(triggers);
    MageCureRaidStrategy::InitNonCombatTriggers(triggers);
}

#endif
#ifdef MANGOSBOT_ONE // TBC

NextAction** FrostMageStrategy::GetDefaultCombatActions()
{
    return NextAction::array(0, new NextAction("frostbolt", ACTION_IDLE), NULL);
}

void FrostMageStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    MageStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "ice lance",
        NextAction::array(0, new NextAction("ice lance", ACTION_NORMAL), NULL)));
}

void FrostMageStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    MageStrategy::InitNonCombatTriggers(triggers);
}

void FrostMageStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    MageStrategy::InitReactionTriggers(triggers);
}

void FrostMageStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    MageStrategy::InitDeadTriggers(triggers);
}

void FrostMagePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageStrategy::InitCombatTriggers(triggers);
    MagePveStrategy::InitCombatTriggers(triggers);
}

void FrostMagePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageStrategy::InitNonCombatTriggers(triggers);
    MagePveStrategy::InitNonCombatTriggers(triggers);
}

void FrostMagePveStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageStrategy::InitReactionTriggers(triggers);
    MagePveStrategy::InitReactionTriggers(triggers);
}

void FrostMagePveStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageStrategy::InitDeadTriggers(triggers);
    MagePveStrategy::InitDeadTriggers(triggers);
}

void FrostMagePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageStrategy::InitCombatTriggers(triggers);
    MagePvpStrategy::InitCombatTriggers(triggers);
}

void FrostMagePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageStrategy::InitNonCombatTriggers(triggers);
    MagePvpStrategy::InitNonCombatTriggers(triggers);
}

void FrostMagePvpStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageStrategy::InitReactionTriggers(triggers);
    MagePvpStrategy::InitReactionTriggers(triggers);
}

void FrostMagePvpStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageStrategy::InitDeadTriggers(triggers);
    MagePvpStrategy::InitDeadTriggers(triggers);
}

void FrostMageRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageStrategy::InitCombatTriggers(triggers);
    MageRaidStrategy::InitCombatTriggers(triggers);
}

void FrostMageRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageStrategy::InitNonCombatTriggers(triggers);
    MageRaidStrategy::InitNonCombatTriggers(triggers);
}

void FrostMageRaidStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageStrategy::InitReactionTriggers(triggers);
    MageRaidStrategy::InitReactionTriggers(triggers);
}

void FrostMageRaidStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageStrategy::InitDeadTriggers(triggers);
    MageRaidStrategy::InitDeadTriggers(triggers);
}

void FrostMageAoeStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    MageAoeStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "ranged medium aoe",
        NextAction::array(0, new NextAction("blizzard", ACTION_HIGH + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "ranged medium aoe",
        NextAction::array(0, new NextAction("flamestrike", ACTION_HIGH), NULL)));
}

void FrostMageAoeStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    MageAoeStrategy::InitNonCombatTriggers(triggers);
}

void FrostMageAoePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageAoeStrategy::InitCombatTriggers(triggers);
    MageAoePveStrategy::InitCombatTriggers(triggers);
}

void FrostMageAoePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageAoeStrategy::InitNonCombatTriggers(triggers);
    MageAoePveStrategy::InitNonCombatTriggers(triggers);
}

void FrostMageAoePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageAoeStrategy::InitCombatTriggers(triggers);
    MageAoePvpStrategy::InitCombatTriggers(triggers);
}

void FrostMageAoePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageAoeStrategy::InitNonCombatTriggers(triggers);
    MageAoePvpStrategy::InitNonCombatTriggers(triggers);
}

void FrostMageAoeRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageAoeStrategy::InitCombatTriggers(triggers);
    MageAoeRaidStrategy::InitCombatTriggers(triggers);
}

void FrostMageAoeRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageAoeStrategy::InitNonCombatTriggers(triggers);
    MageAoeRaidStrategy::InitNonCombatTriggers(triggers);
}

void FrostMageBuffStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    MageBuffStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "ice barrier",
        NextAction::array(0, new NextAction("ice barrier", ACTION_HIGH), NULL)));
}

void FrostMageBuffStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    MageBuffStrategy::InitNonCombatTriggers(triggers);
}

void FrostMageBuffPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageBuffStrategy::InitCombatTriggers(triggers);
    MageBuffPveStrategy::InitCombatTriggers(triggers);
}

void FrostMageBuffPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageBuffStrategy::InitNonCombatTriggers(triggers);
    MageBuffPveStrategy::InitNonCombatTriggers(triggers);
}

void FrostMageBuffPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageBuffStrategy::InitCombatTriggers(triggers);
    MageBuffPvpStrategy::InitCombatTriggers(triggers);
}

void FrostMageBuffPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageBuffStrategy::InitNonCombatTriggers(triggers);
    MageBuffPvpStrategy::InitNonCombatTriggers(triggers);
}

void FrostMageBuffRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageBuffStrategy::InitCombatTriggers(triggers);
    MageBuffRaidStrategy::InitCombatTriggers(triggers);
}

void FrostMageBuffRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageBuffStrategy::InitNonCombatTriggers(triggers);
    MageBuffRaidStrategy::InitNonCombatTriggers(triggers);
}

void FrostMageBoostStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    MageBoostStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "cold snap",
        NextAction::array(0, new NextAction("cold snap", ACTION_HIGH + 3), NULL)));

    triggers.push_back(new TriggerNode(
        "summon water elemental",
        NextAction::array(0, new NextAction("summon water elemental", ACTION_HIGH + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "icy veins",
        NextAction::array(0, new NextAction("icy veins", ACTION_HIGH + 1), NULL)));
}

void FrostMageBoostStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    MageBoostStrategy::InitNonCombatTriggers(triggers);
}

void FrostMageBoostPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageBoostStrategy::InitCombatTriggers(triggers);
    MageBoostPveStrategy::InitCombatTriggers(triggers);
}

void FrostMageBoostPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageBoostStrategy::InitNonCombatTriggers(triggers);
    MageBoostPveStrategy::InitNonCombatTriggers(triggers);
}

void FrostMageBoostPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageBoostStrategy::InitCombatTriggers(triggers);
    MageBoostPvpStrategy::InitCombatTriggers(triggers);
}

void FrostMageBoostPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageBoostStrategy::InitNonCombatTriggers(triggers);
    MageBoostPvpStrategy::InitNonCombatTriggers(triggers);
}

void FrostMageBoostRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageBoostStrategy::InitCombatTriggers(triggers);
    MageBoostRaidStrategy::InitCombatTriggers(triggers);
}

void FrostMageBoostRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageBoostStrategy::InitNonCombatTriggers(triggers);
    MageBoostRaidStrategy::InitNonCombatTriggers(triggers);
}

void FrostMageCcStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    MageCcStrategy::InitCombatTriggers(triggers);
}

void FrostMageCcStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    MageCcStrategy::InitNonCombatTriggers(triggers);
}

void FrostMageCcPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageCcStrategy::InitCombatTriggers(triggers);
    MageCcPveStrategy::InitCombatTriggers(triggers);
}

void FrostMageCcPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageCcStrategy::InitNonCombatTriggers(triggers);
    MageCcPveStrategy::InitNonCombatTriggers(triggers);
}

void FrostMageCcPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageCcStrategy::InitCombatTriggers(triggers);
    MageCcPvpStrategy::InitCombatTriggers(triggers);
}

void FrostMageCcPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageCcStrategy::InitNonCombatTriggers(triggers);
    MageCcPvpStrategy::InitNonCombatTriggers(triggers);
}

void FrostMageCcRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageCcStrategy::InitCombatTriggers(triggers);
    MageCcRaidStrategy::InitCombatTriggers(triggers);
}

void FrostMageCcRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageCcStrategy::InitNonCombatTriggers(triggers);
    MageCcRaidStrategy::InitNonCombatTriggers(triggers);
}

void FrostMageCureStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    MageCureStrategy::InitCombatTriggers(triggers);
}

void FrostMageCureStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    MageCureStrategy::InitNonCombatTriggers(triggers);
}

void FrostMageCurePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageCureStrategy::InitCombatTriggers(triggers);
    MageCurePveStrategy::InitCombatTriggers(triggers);
}

void FrostMageCurePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageCureStrategy::InitNonCombatTriggers(triggers);
    MageCurePveStrategy::InitNonCombatTriggers(triggers);
}

void FrostMageCurePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageCureStrategy::InitCombatTriggers(triggers);
    MageCurePvpStrategy::InitCombatTriggers(triggers);
}

void FrostMageCurePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageCureStrategy::InitNonCombatTriggers(triggers);
    MageCurePvpStrategy::InitNonCombatTriggers(triggers);
}

void FrostMageCureRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageCureStrategy::InitCombatTriggers(triggers);
    MageCureRaidStrategy::InitCombatTriggers(triggers);
}

void FrostMageCureRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageCureStrategy::InitNonCombatTriggers(triggers);
    MageCureRaidStrategy::InitNonCombatTriggers(triggers);
}

#endif
#ifdef MANGOSBOT_TWO // WOTLK

NextAction** FrostMageStrategy::GetDefaultCombatActions()
{
    return NextAction::array(0, new NextAction("frostbolt", ACTION_IDLE), NULL);
}

void FrostMageStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    MageStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "free fireball",
        NextAction::array(0, new NextAction("frostfire bolt", ACTION_NORMAL + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "fingers of frost",
        NextAction::array(0, new NextAction("deep freeze", ACTION_NORMAL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "ice lance",
        NextAction::array(0, new NextAction("ice lance", ACTION_NORMAL), NULL)));
}

void FrostMageStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    MageStrategy::InitNonCombatTriggers(triggers);
}

void FrostMageStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    MageStrategy::InitReactionTriggers(triggers);
}

void FrostMageStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    MageStrategy::InitDeadTriggers(triggers);
}

void FrostMagePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageStrategy::InitCombatTriggers(triggers);
    MagePveStrategy::InitCombatTriggers(triggers);
}

void FrostMagePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageStrategy::InitNonCombatTriggers(triggers);
    MagePveStrategy::InitNonCombatTriggers(triggers);
}

void FrostMagePveStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageStrategy::InitReactionTriggers(triggers);
    MagePveStrategy::InitReactionTriggers(triggers);
}

void FrostMagePveStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageStrategy::InitDeadTriggers(triggers);
    MagePveStrategy::InitDeadTriggers(triggers);
}

void FrostMagePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageStrategy::InitCombatTriggers(triggers);
    MagePvpStrategy::InitCombatTriggers(triggers);
}

void FrostMagePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageStrategy::InitNonCombatTriggers(triggers);
    MagePvpStrategy::InitNonCombatTriggers(triggers);
}

void FrostMagePvpStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageStrategy::InitReactionTriggers(triggers);
    MagePvpStrategy::InitReactionTriggers(triggers);
}

void FrostMagePvpStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageStrategy::InitDeadTriggers(triggers);
    MagePvpStrategy::InitDeadTriggers(triggers);
}

void FrostMageRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageStrategy::InitCombatTriggers(triggers);
    MageRaidStrategy::InitCombatTriggers(triggers);
}

void FrostMageRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageStrategy::InitNonCombatTriggers(triggers);
    MageRaidStrategy::InitNonCombatTriggers(triggers);
}

void FrostMageRaidStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageStrategy::InitReactionTriggers(triggers);
    MageRaidStrategy::InitReactionTriggers(triggers);
}

void FrostMageRaidStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageStrategy::InitDeadTriggers(triggers);
    MageRaidStrategy::InitDeadTriggers(triggers);
}

void FrostMageAoeStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    MageAoeStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "ranged medium aoe",
        NextAction::array(0, new NextAction("blizzard", ACTION_HIGH + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "ranged medium aoe",
        NextAction::array(0, new NextAction("flamestrike", ACTION_HIGH), NULL)));
}

void FrostMageAoeStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    MageAoeStrategy::InitNonCombatTriggers(triggers);
}

void FrostMageAoePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageAoeStrategy::InitCombatTriggers(triggers);
    MageAoePveStrategy::InitCombatTriggers(triggers);
}

void FrostMageAoePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageAoeStrategy::InitNonCombatTriggers(triggers);
    MageAoePveStrategy::InitNonCombatTriggers(triggers);
}

void FrostMageAoePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageAoeStrategy::InitCombatTriggers(triggers);
    MageAoePvpStrategy::InitCombatTriggers(triggers);
}

void FrostMageAoePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageAoeStrategy::InitNonCombatTriggers(triggers);
    MageAoePvpStrategy::InitNonCombatTriggers(triggers);
}

void FrostMageAoeRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageAoeStrategy::InitCombatTriggers(triggers);
    MageAoeRaidStrategy::InitCombatTriggers(triggers);
}

void FrostMageAoeRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageAoeStrategy::InitNonCombatTriggers(triggers);
    MageAoeRaidStrategy::InitNonCombatTriggers(triggers);
}

void FrostMageBuffStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    MageBuffStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "ice barrier",
        NextAction::array(0, new NextAction("ice barrier", ACTION_HIGH), NULL)));
}

void FrostMageBuffStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    MageBuffStrategy::InitNonCombatTriggers(triggers);
}

void FrostMageBuffPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageBuffStrategy::InitCombatTriggers(triggers);
    MageBuffPveStrategy::InitCombatTriggers(triggers);
}

void FrostMageBuffPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageBuffStrategy::InitNonCombatTriggers(triggers);
    MageBuffPveStrategy::InitNonCombatTriggers(triggers);
}

void FrostMageBuffPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageBuffStrategy::InitCombatTriggers(triggers);
    MageBuffPvpStrategy::InitCombatTriggers(triggers);
}

void FrostMageBuffPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageBuffStrategy::InitNonCombatTriggers(triggers);
    MageBuffPvpStrategy::InitNonCombatTriggers(triggers);
}

void FrostMageBuffRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageBuffStrategy::InitCombatTriggers(triggers);
    MageBuffRaidStrategy::InitCombatTriggers(triggers);
}

void FrostMageBuffRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageBuffStrategy::InitNonCombatTriggers(triggers);
    MageBuffRaidStrategy::InitNonCombatTriggers(triggers);
}

void FrostMageBoostStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    MageBoostStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "cold snap",
        NextAction::array(0, new NextAction("cold snap", ACTION_HIGH + 3), NULL)));

    triggers.push_back(new TriggerNode(
        "summon water elemental",
        NextAction::array(0, new NextAction("summon water elemental", ACTION_HIGH + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "icy veins",
        NextAction::array(0, new NextAction("icy veins", ACTION_HIGH + 1), NULL)));
}

void FrostMageBoostStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    MageBoostStrategy::InitNonCombatTriggers(triggers);
}

void FrostMageBoostPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageBoostStrategy::InitCombatTriggers(triggers);
    MageBoostPveStrategy::InitCombatTriggers(triggers);
}

void FrostMageBoostPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageBoostStrategy::InitNonCombatTriggers(triggers);
    MageBoostPveStrategy::InitNonCombatTriggers(triggers);
}

void FrostMageBoostPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageBoostStrategy::InitCombatTriggers(triggers);
    MageBoostPvpStrategy::InitCombatTriggers(triggers);
}

void FrostMageBoostPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageBoostStrategy::InitNonCombatTriggers(triggers);
    MageBoostPvpStrategy::InitNonCombatTriggers(triggers);
}

void FrostMageBoostRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageBoostStrategy::InitCombatTriggers(triggers);
    MageBoostRaidStrategy::InitCombatTriggers(triggers);
}

void FrostMageBoostRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageBoostStrategy::InitNonCombatTriggers(triggers);
    MageBoostRaidStrategy::InitNonCombatTriggers(triggers);
}

void FrostMageCcStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    MageCcStrategy::InitCombatTriggers(triggers);
}

void FrostMageCcStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    MageCcStrategy::InitNonCombatTriggers(triggers);
}

void FrostMageCcPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageCcStrategy::InitCombatTriggers(triggers);
    MageCcPveStrategy::InitCombatTriggers(triggers);
}

void FrostMageCcPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageCcStrategy::InitNonCombatTriggers(triggers);
    MageCcPveStrategy::InitNonCombatTriggers(triggers);
}

void FrostMageCcPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageCcStrategy::InitCombatTriggers(triggers);
    MageCcPvpStrategy::InitCombatTriggers(triggers);
}

void FrostMageCcPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageCcStrategy::InitNonCombatTriggers(triggers);
    MageCcPvpStrategy::InitNonCombatTriggers(triggers);
}

void FrostMageCcRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageCcStrategy::InitCombatTriggers(triggers);
    MageCcRaidStrategy::InitCombatTriggers(triggers);
}

void FrostMageCcRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageCcStrategy::InitNonCombatTriggers(triggers);
    MageCcRaidStrategy::InitNonCombatTriggers(triggers);
}

void FrostMageCureStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    MageCureStrategy::InitCombatTriggers(triggers);
}

void FrostMageCureStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    MageCureStrategy::InitNonCombatTriggers(triggers);
}

void FrostMageCurePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageCureStrategy::InitCombatTriggers(triggers);
    MageCurePveStrategy::InitCombatTriggers(triggers);
}

void FrostMageCurePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageCureStrategy::InitNonCombatTriggers(triggers);
    MageCurePveStrategy::InitNonCombatTriggers(triggers);
}

void FrostMageCurePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageCureStrategy::InitCombatTriggers(triggers);
    MageCurePvpStrategy::InitCombatTriggers(triggers);
}

void FrostMageCurePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageCureStrategy::InitNonCombatTriggers(triggers);
    MageCurePvpStrategy::InitNonCombatTriggers(triggers);
}

void FrostMageCureRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageCureStrategy::InitCombatTriggers(triggers);
    MageCureRaidStrategy::InitCombatTriggers(triggers);
}

void FrostMageCureRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    FrostMageCureStrategy::InitNonCombatTriggers(triggers);
    MageCureRaidStrategy::InitNonCombatTriggers(triggers);
}

#endif