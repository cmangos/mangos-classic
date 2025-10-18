
#include "playerbot/playerbot.h"
#include "DungeonStrategy.h"

using namespace ai;

void DungeonStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    // Add this combat triggers in case the bot gets summoned into the dungeon and goes straight into combat
    triggers.push_back(new TriggerNode(
        "enter onyxia's lair",
        NextAction::array(0, new NextAction("enable onyxia's lair strategy", 100.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "enter molten core",
        NextAction::array(0, new NextAction("enable molten core strategy", 100.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "enter karazhan",
        NextAction::array(0, new NextAction("enable karazhan strategy", 100.0f), NULL)));
}

void DungeonStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "enter onyxia's lair",
        NextAction::array(0, new NextAction("enable onyxia's lair strategy", 100.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "leave onyxia's lair",
        NextAction::array(0, new NextAction("disable onyxia's lair strategy", 100.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "enter molten core",
        NextAction::array(0, new NextAction("enable molten core strategy", 100.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "leave molten core",
        NextAction::array(0, new NextAction("disable molten core strategy", 100.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "enter karazhan",
        NextAction::array(0, new NextAction("enable karazhan strategy", 100.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "leave karazhan",
        NextAction::array(0, new NextAction("disable karazhan strategy", 100.0f), NULL)));
}