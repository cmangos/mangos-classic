
#include "playerbot/playerbot.h"
#include "MoltenCoreDungeonStrategies.h"
#include "DungeonMultipliers.h"

using namespace ai;

void MoltenCoreDungeonStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "start magmadar fight",
        NextAction::array(0, new NextAction("enable magmadar fight strategy", 100.0f), NULL)));
}

void MoltenCoreDungeonStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    /*
    triggers.push_back(new TriggerNode(
        "val::and::{"
        "action possible::use id::17333,"
        "has object::go usable filter::go trapped filter::entry filter::{gos in sight,mc runes},"
        "not::has object::entry filter::{gos close,mc runes}"
        "}",
        NextAction::array(0, new NextAction("move to::entry filter::{gos in sight,mc runes}", 1.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "val::has object::go usable filter::entry filter::{gos close,mc runes}",
        NextAction::array(0, new NextAction("use id::{17333,entry filter::{gos close,mc runes}}", 1.0f), NULL)));
        */

    triggers.push_back(new TriggerNode(
        "mc rune in sight",
        NextAction::array(0, new NextAction("move to mc rune", 1.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "mc rune close",
        NextAction::array(0, new NextAction("douse mc rune", 1.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "fire protection potion ready",
        NextAction::array(0, new NextAction("fire protection potion", 100.0f), NULL)));
}

void MagmadarFightStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    Player* bot = ai->GetBot();
    if (ai->IsRanged(bot) || ai->IsHeal(bot))
    {
        triggers.push_back(new TriggerNode(
            "magmadar too close",
            NextAction::array(0, new NextAction("move away from magmadar", 100.0f), NULL)));
    }

    triggers.push_back(new TriggerNode(
        "fire protection potion ready",
        NextAction::array(0, new NextAction("fire protection potion", 100.0f), NULL)));
}

void MagmadarFightStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "end magmadar fight",
        NextAction::array(0, new NextAction("disable magmadar fight strategy", 100.0f), NULL)));
}

void MagmadarFightStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "end magmadar fight",
        NextAction::array(0, new NextAction("disable magmadar fight strategy", 100.0f), NULL)));
}

void MagmadarFightStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "magmadar lava bomb",
        NextAction::array(0, new NextAction("move away from hazard", 100.0f), NULL)));
}

void MagmadarFightStrategy::InitCombatMultipliers(std::list<Multiplier*>& multipliers)
{
    Player* bot = ai->GetBot();
    if (ai->IsRanged(bot) || ai->IsHeal(bot))
    {
        multipliers.push_back(new PreventMoveAwayFromCreatureOnReachToCastMultiplier(ai));
    }
}
