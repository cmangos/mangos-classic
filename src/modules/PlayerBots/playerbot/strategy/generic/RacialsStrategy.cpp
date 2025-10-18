
#include "playerbot/playerbot.h"
#include "RacialsStrategy.h"

using namespace ai;

void RacialsStrategy::InitNonCombatTriggers(std::list<TriggerNode*> &triggers)
{
	triggers.push_back(new TriggerNode(
		"low health", 
		NextAction::array(0, new NextAction("gift of the naaru", 71.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "melee medium aoe",
        NextAction::array(0, new NextAction("war stomp", 71.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "war stomp",
        NextAction::array(0, new NextAction("war stomp", 71.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "cannibalize",
        NextAction::array(0, new NextAction("cannibalize", 71.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "perception",
        NextAction::array(0, new NextAction("perception", 71.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "rooted",
        NextAction::array(0, new NextAction("escape artist", 71.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "will of the forsaken",
        NextAction::array(0, new NextAction("will of the forsaken", 71.0f), NULL)));

    /*triggers.push_back(new TriggerNode(
        "shadowmeld",
        NextAction::array(0, new NextAction("shadowmeld", 71.0f), NULL)));*/

    triggers.push_back(new TriggerNode(
        "berserking",
        NextAction::array(0, new NextAction("berserking", 58.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "blood fury",
        NextAction::array(0, new NextAction("blood fury", 71.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "stoneform",
        NextAction::array(0, new NextAction("stoneform", 71.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "mana tap",
        NextAction::array(0, new NextAction("mana tap", 71.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "arcane torrent",
        NextAction::array(0, new NextAction("arcane torrent", 71.0f), NULL)));
}

void RacialsStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    InitNonCombatTriggers(triggers);
}
