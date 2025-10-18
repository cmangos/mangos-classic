
#include "playerbot/playerbot.h"
#include "ClassStrategy.h"

using namespace ai;

#ifdef MANGOSBOT_ZERO // Vanilla

void ClassStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "invalid target",
        NextAction::array(0, new NextAction("select new target", ACTION_EMERGENCY), NULL)));

    triggers.push_back(new TriggerNode(
        "mounted",
        NextAction::array(0, new NextAction("check mount state", ACTION_EMERGENCY), NULL)));

    triggers.push_back(new TriggerNode(
        "very often",
        NextAction::array(0, new NextAction("use lightwell", ACTION_LIGHT_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "combat long stuck",
        NextAction::array(0, new NextAction("unstuck", ACTION_IDLE - 0.1f), NULL)));

    triggers.push_back(new TriggerNode(
        "combat stuck",
        NextAction::array(0, new NextAction("unstuck", ACTION_IDLE - 0.3f), NULL)));
}

void ClassStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    /*
    triggers.push_back(new TriggerNode(
        "vehicle near",
        NextAction::array(0, new NextAction("enter vehicle", 10.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "very often",
        NextAction::array(0, new NextAction("use lightwell", 80.0f), NULL)));
    */

    triggers.push_back(new TriggerNode(
        "very often",
        NextAction::array(0, new NextAction("check mount state", ACTION_IDLE), new NextAction("check values", ACTION_IDLE), NULL)));

    triggers.push_back(new TriggerNode(
        "near dark portal",
        NextAction::array(0, new NextAction("move to dark portal", ACTION_IDLE), NULL)));

    triggers.push_back(new TriggerNode(
        "at dark portal azeroth",
        NextAction::array(0, new NextAction("use dark portal azeroth", ACTION_IDLE), NULL)));

    triggers.push_back(new TriggerNode(
        "at dark portal outland",
        NextAction::array(0, new NextAction("move from dark portal", ACTION_IDLE), NULL)));
}

void ClassStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "combat start",
        NextAction::array(0, new NextAction("set combat state", ACTION_PASSTROUGH + 10), NULL)));

    triggers.push_back(new TriggerNode(
        "combat end",
        NextAction::array(0, new NextAction("set non combat state", ACTION_PASSTROUGH + 10), NULL)));

    triggers.push_back(new TriggerNode(
        "death",
        NextAction::array(0, new NextAction("set dead state", ACTION_PASSTROUGH + 10), NULL)));

    triggers.push_back(new TriggerNode(
        "resurrect",
        NextAction::array(0, new NextAction("set non combat state", ACTION_PASSTROUGH + 10), NULL)));
}

void ClassStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{

}

void ClassPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "random",
        NextAction::array(0, new NextAction("free action potion", ACTION_HIGH), NULL)));
}

void ClassPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void ClassPvpStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{

}

void ClassPvpStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{

}

void ClassPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void ClassPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void ClassPveStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{

}

void ClassPveStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{

}

void ClassRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void ClassRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void ClassRaidStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{

}

void ClassRaidStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{

}

void AoeStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "melee very high aoe",
        NextAction::array(0, new NextAction("goblin sapper", ACTION_HIGH + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "ranged medium aoe",
        NextAction::array(0, new NextAction("throw grenade", ACTION_HIGH), NULL)));
}

void AoeStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void AoePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void AoePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void AoePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void AoePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void AoeRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void AoeRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void CureStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void CureStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void CurePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void CurePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void CurePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void CurePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void CureRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void CureRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void CcStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void CcStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void CcPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void CcPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void CcPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void CcPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void CcRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void CcRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void BuffStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void BuffStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void BuffPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void BuffPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void BuffPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void BuffPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void BuffRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void BuffRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void BoostStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "often",
        NextAction::array(0, new NextAction("use trinket", ACTION_HIGH), NULL)));
}

void BoostStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void BoostPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void BoostPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void BoostPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void BoostPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void BoostRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void BoostRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void StealthStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void StealthStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void StealthPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void StealthPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void StealthPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void StealthPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void StealthRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void StealthRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void OffhealStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void OffhealStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void OffhealPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void OffhealPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void OffhealPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void OffhealPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void OffhealRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void OffhealRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void OffdpsStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void OffdpsStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void OffdpsPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void OffdpsPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void OffdpsPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void OffdpsPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void OffdpsRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void OffdpsRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}


#endif
#ifdef MANGOSBOT_ONE // TBC

void ClassStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "invalid target",
        NextAction::array(0, new NextAction("select new target", ACTION_EMERGENCY), NULL)));

    triggers.push_back(new TriggerNode(
        "mounted",
        NextAction::array(0, new NextAction("check mount state", ACTION_EMERGENCY), NULL)));

    triggers.push_back(new TriggerNode(
        "very often",
        NextAction::array(0, new NextAction("use lightwell", ACTION_LIGHT_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "combat long stuck",
        NextAction::array(0, new NextAction("unstuck", ACTION_IDLE - 0.1f), NULL)));

    triggers.push_back(new TriggerNode(
        "combat stuck",
        NextAction::array(0, new NextAction("unstuck", ACTION_IDLE - 0.3f), NULL)));
}

void ClassStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    /*
    triggers.push_back(new TriggerNode(
        "vehicle near",
        NextAction::array(0, new NextAction("enter vehicle", 10.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "very often",
        NextAction::array(0, new NextAction("use lightwell", 80.0f), NULL)));
    */

    triggers.push_back(new TriggerNode(
        "very often",
        NextAction::array(0, new NextAction("check mount state", ACTION_IDLE), new NextAction("check values", ACTION_IDLE), NULL)));

    triggers.push_back(new TriggerNode(
        "near dark portal",
        NextAction::array(0, new NextAction("move to dark portal", ACTION_IDLE), NULL)));

    triggers.push_back(new TriggerNode(
        "at dark portal azeroth",
        NextAction::array(0, new NextAction("use dark portal azeroth", ACTION_IDLE), NULL)));

    triggers.push_back(new TriggerNode(
        "at dark portal outland",
        NextAction::array(0, new NextAction("move from dark portal", ACTION_IDLE), NULL)));
}

void ClassStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "combat start",
        NextAction::array(0, new NextAction("set combat state", ACTION_PASSTROUGH + 10), NULL)));

    triggers.push_back(new TriggerNode(
        "combat end",
        NextAction::array(0, new NextAction("set non combat state", ACTION_PASSTROUGH + 10), NULL)));

    triggers.push_back(new TriggerNode(
        "death",
        NextAction::array(0, new NextAction("set dead state", ACTION_PASSTROUGH + 10), NULL)));

    triggers.push_back(new TriggerNode(
        "resurrect",
        NextAction::array(0, new NextAction("set non combat state", ACTION_PASSTROUGH + 10), NULL)));
}

void ClassStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{

}

void ClassPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void ClassPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void ClassPvpStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{

}

void ClassPvpStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{

}

void ClassPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void ClassPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void ClassPveStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{

}

void ClassPveStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{

}

void ClassRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void ClassRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void ClassRaidStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{

}

void ClassRaidStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{

}

void AoeStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "melee very high aoe",
        NextAction::array(0, new NextAction("goblin sapper", ACTION_HIGH + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "ranged medium aoe",
        NextAction::array(0, new NextAction("throw grenade", ACTION_HIGH), NULL)));
}

void AoeStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void AoePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void AoePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void AoePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void AoePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void AoeRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void AoeRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void CureStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void CureStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void CurePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void CurePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void CurePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void CurePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void CureRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void CureRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void CcStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void CcStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void CcPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void CcPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void CcPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void CcPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void CcRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void CcRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void BuffStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void BuffStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void BuffPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void BuffPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void BuffPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void BuffPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void BuffRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void BuffRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void BoostStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "often",
        NextAction::array(0, new NextAction("use trinket", ACTION_HIGH), NULL)));
}

void BoostStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void BoostPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void BoostPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void BoostPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void BoostPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void BoostRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void BoostRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void StealthStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void StealthStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void StealthPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void StealthPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void StealthPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void StealthPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void StealthRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void StealthRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void OffhealStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void OffhealStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void OffhealPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void OffhealPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void OffhealPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void OffhealPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void OffhealRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void OffhealRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void OffdpsStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void OffdpsStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void OffdpsPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void OffdpsPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void OffdpsPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void OffdpsPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void OffdpsRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void OffdpsRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

#endif
#ifdef MANGOSBOT_TWO // WOTLK

void ClassStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "invalid target",
        NextAction::array(0, new NextAction("select new target", ACTION_EMERGENCY), NULL)));

    triggers.push_back(new TriggerNode(
        "mounted",
        NextAction::array(0, new NextAction("check mount state", ACTION_EMERGENCY), NULL)));

    triggers.push_back(new TriggerNode(
        "very often",
        NextAction::array(0, new NextAction("use lightwell", ACTION_LIGHT_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "often",
        NextAction::array(0, new NextAction("use trinket", ACTION_HIGH), NULL)));

    triggers.push_back(new TriggerNode(
        "combat long stuck",
        NextAction::array(0, new NextAction("unstuck", ACTION_IDLE - 0.1f), NULL)));

    triggers.push_back(new TriggerNode(
        "combat stuck",
        NextAction::array(0, new NextAction("unstuck", ACTION_IDLE - 0.3f), NULL)));
}

void ClassStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    /*
    triggers.push_back(new TriggerNode(
        "vehicle near",
        NextAction::array(0, new NextAction("enter vehicle", 10.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "very often",
        NextAction::array(0, new NextAction("use lightwell", 80.0f), NULL)));
    */

    triggers.push_back(new TriggerNode(
        "very often",
        NextAction::array(0, new NextAction("check mount state", ACTION_IDLE), new NextAction("check values", ACTION_IDLE), NULL)));

    triggers.push_back(new TriggerNode(
        "near dark portal",
        NextAction::array(0, new NextAction("move to dark portal", ACTION_IDLE), NULL)));

    triggers.push_back(new TriggerNode(
        "at dark portal azeroth",
        NextAction::array(0, new NextAction("use dark portal azeroth", ACTION_IDLE), NULL)));

    triggers.push_back(new TriggerNode(
        "at dark portal outland",
        NextAction::array(0, new NextAction("move from dark portal", ACTION_IDLE), NULL)));
}

void ClassStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "combat start",
        NextAction::array(0, new NextAction("set combat state", ACTION_PASSTROUGH + 10), NULL)));

    triggers.push_back(new TriggerNode(
        "combat end",
        NextAction::array(0, new NextAction("set non combat state", ACTION_PASSTROUGH + 10), NULL)));

    triggers.push_back(new TriggerNode(
        "death",
        NextAction::array(0, new NextAction("set dead state", ACTION_PASSTROUGH + 10), NULL)));

    triggers.push_back(new TriggerNode(
        "resurrect",
        NextAction::array(0, new NextAction("set non combat state", ACTION_PASSTROUGH + 10), NULL)));
}

void ClassStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{

}

void ClassPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void ClassPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void ClassPvpStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{

}

void ClassPvpStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{

}

void ClassPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void ClassPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void ClassPveStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{

}

void ClassPveStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{

}

void ClassRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void ClassRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void ClassRaidStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{

}

void ClassRaidStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{

}

void AoeStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "melee very high aoe",
        NextAction::array(0, new NextAction("goblin sapper", ACTION_HIGH + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "ranged medium aoe",
        NextAction::array(0, new NextAction("throw grenade", ACTION_HIGH), NULL)));
}

void AoeStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void AoePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void AoePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void AoePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void AoePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void AoeRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void AoeRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void CureStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void CureStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void CurePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void CurePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void CurePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void CurePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void CureRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void CureRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void CcStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void CcStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void CcPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void CcPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void CcPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void CcPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void CcRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void CcRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void BuffStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void BuffStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void BuffPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void BuffPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void BuffPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void BuffPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void BuffRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void BuffRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void BoostStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "often",
        NextAction::array(0, new NextAction("use trinket", ACTION_HIGH), NULL)));
}

void BoostStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void BoostPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void BoostPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void BoostPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void BoostPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void BoostRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void BoostRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void StealthStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void StealthStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void StealthPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void StealthPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void StealthPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void StealthPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void StealthRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void StealthRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void OffhealStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void OffhealStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void OffhealPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void OffhealPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void OffhealPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void OffhealPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void OffhealRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void OffhealRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void OffdpsStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void OffdpsStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void OffdpsPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void OffdpsPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void OffdpsPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void OffdpsPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void OffdpsRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void OffdpsRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}


#endif

void SpecPlaceholderStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "in pve",
        NextAction::array(0, new NextAction("update pve strats", ACTION_PASSTROUGH), NULL)));

    triggers.push_back(new TriggerNode(
        "in pvp",
        NextAction::array(0, new NextAction("update pvp strats", ACTION_PASSTROUGH), NULL)));

    triggers.push_back(new TriggerNode(
        "in raid fight",
        NextAction::array(0, new NextAction("update raid strats", ACTION_PASSTROUGH), NULL)));
}

void SpecPlaceholderStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "in pve",
        NextAction::array(0, new NextAction("update pve strats", ACTION_PASSTROUGH), NULL)));

    triggers.push_back(new TriggerNode(
        "in pvp",
        NextAction::array(0, new NextAction("update pvp strats", ACTION_PASSTROUGH), NULL)));

    triggers.push_back(new TriggerNode(
        "in raid fight",
        NextAction::array(0, new NextAction("update raid strats", ACTION_PASSTROUGH), NULL)));
}
