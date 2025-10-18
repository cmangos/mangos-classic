
#include "playerbot/playerbot.h"
#include "KarazhanDungeonStrategies.h"
#include "DungeonMultipliers.h"

using namespace ai;

void KarazhanDungeonStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
	triggers.push_back(new TriggerNode(
		"start netherspite fight",
		NextAction::array(0, new NextAction("enable netherspite fight strategy", 100.0f), NULL)));

	triggers.push_back(new TriggerNode(
		"start prince malchezaar fight",
		NextAction::array(0, new NextAction("enable prince malchezaar fight strategy", 100.0f), NULL)));
}

void NetherspiteFightStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
	triggers.push_back(new TriggerNode(
		"void zone too close",
		NextAction::array(0, new NextAction("move away from void zone", 100.0f), NULL)));

	triggers.push_back(new TriggerNode(
		"add nether portal - perseverence for tank",
		NextAction::array(0, new NextAction("add nether portal - perseverence for tank", 101.0f), NULL)));

	triggers.push_back(new TriggerNode(
		"remove nether portal buffs from netherspite",
		NextAction::array(0, new NextAction("remove nether portal buffs from netherspite", 101.0f), NULL)));

	triggers.push_back(new TriggerNode(
		"nether portal - perseverence",
		NextAction::array(0, new NextAction("remove nether portal - perseverence", 101.0f), NULL)));

	triggers.push_back(new TriggerNode(
		"nether portal - serenity",
		NextAction::array(0, new NextAction("remove nether portal - serenity", 101.0f), NULL)));

	triggers.push_back(new TriggerNode(
		"nether portal - dominance",
		NextAction::array(0, new NextAction("remove nether portal - dominance", 101.0f), NULL)));
}

void NetherspiteFightStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
	triggers.push_back(new TriggerNode(
		"end netherspite fight",
		NextAction::array(0, new NextAction("disable netherspite fight strategy", 100.0f), NULL)));
}

void NetherspiteFightStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
	triggers.push_back(new TriggerNode(
		"end netherspite fight",
		NextAction::array(0, new NextAction("disable netherspite fight strategy", 100.0f), NULL)));
}

void PrinceMalchezaarFightStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
	triggers.push_back(new TriggerNode(
		"netherspite infernal too close",
		NextAction::array(0, new NextAction("move away from netherspite infernal", 100.0f), NULL)));
}

void PrinceMalchezaarFightStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
	triggers.push_back(new TriggerNode(
		"end prince malchezaar fight",
		NextAction::array(0, new NextAction("disable prince malchezaar fight strategy", 100.0f), NULL)));
}

void PrinceMalchezaarFightStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
	triggers.push_back(new TriggerNode(
		"end prince malchezaar fight",
		NextAction::array(0, new NextAction("disable prince malchezaar fight strategy", 100.0f), NULL)));
}
