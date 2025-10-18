
#include "playerbot/playerbot.h"
#include "QuestStrategies.h"

using namespace ai;

QuestStrategy::QuestStrategy(PlayerbotAI* ai) : PassTroughStrategy(ai)
{
    supported.push_back("accept quest");
}

void QuestStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PassTroughStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode("quest share", NextAction::array(0, new NextAction("accept quest share", relevance), NULL)));

#ifdef MANGOSBOT_TWO
    triggers.push_back(
        new TriggerNode("val::and::{need quest objective::{12680,0},trigger active::in vehicle}", NextAction::array(0, new NextAction("deliver stolen horse", 1.0f), NULL)));

    triggers.push_back(
        new TriggerNode("val::and::{need quest objective::{12687,0},trigger active::in vehicle}", NextAction::array(0, new NextAction("horsemans call", 1.0f), NULL)));

    triggers.push_back(new TriggerNode("val::and::{need quest objective::{12701,0},trigger active::in vehicle::Scarlet Cannon}",
                                       NextAction::array(0, new NextAction("scarlet cannon", 1.0f), NULL)));

    triggers.push_back(new TriggerNode("val::and::{need quest objective::{12701,0},trigger active::in vehicle::Scarlet Cannon}",
                                       NextAction::array(0, new NextAction("electro - magnetic pulse", 2.0f), NULL)));

    triggers.push_back(new TriggerNode("val::and::{not::need quest objective::{12701,0},trigger active::in vehicle::Scarlet Cannon}",
                                       NextAction::array(0, new NextAction("skeletal gryphon escape", 1.0f), NULL)));

    triggers.push_back(new TriggerNode("val::and::{need quest objective::12779,trigger active::in vehicle::Frostbrood Vanquisher}",
                                       NextAction::array(0, new NextAction("frozen deathbolt", 80.0f), NULL)));

    triggers.push_back(new TriggerNode("val::and::{need quest objective::12779,trigger active::in vehicle::Frostbrood Vanquisher}",
                                       NextAction::array(0, new NextAction("devour humanoid", 81.0f), NULL)));

    triggers.push_back(new TriggerNode("val::need quest objective::13165", NextAction::array(0, new NextAction("cast::death gate", 80.0f), NULL)));

    triggers.push_back(new TriggerNode("val::need quest objective::13165", NextAction::array(0, new NextAction("use::death gate", 81.0f), NULL)));
#endif
};

void QuestStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PassTroughStrategy::InitCombatTriggers(triggers);

#ifdef MANGOSBOT_TWO
    triggers.push_back(new TriggerNode("val::and::{need quest objective::{12701,0},trigger active::in vehicle::Scarlet Cannon}",
                                       NextAction::array(0, new NextAction("scarlet cannon", 1.0f), NULL)));

    triggers.push_back(new TriggerNode("val::and::{need quest objective::{12701,0},trigger active::in vehicle::Scarlet Cannon}",
                                       NextAction::array(0, new NextAction("electro - magnetic pulse", 2.0f), NULL)));

    triggers.push_back(new TriggerNode("val::and::{not::need quest objective::{12701,0},trigger active::in vehicle::Scarlet Cannon}",
                                       NextAction::array(0, new NextAction("skeletal gryphon escape", 1.0f), NULL)));

    triggers.push_back(new TriggerNode("val::and::{need quest objective::12779,trigger active::in vehicle::Frostbrood Vanquisher}",
                                       NextAction::array(0, new NextAction("frozen deathbolt", 81.0f), NULL)));

    triggers.push_back(new TriggerNode("val::and::{need quest objective::12779,trigger active::in vehicle::Frostbrood Vanquisher}",
                                       NextAction::array(0, new NextAction("devour humanoid", 80.0f), NULL)));
#endif
};

void DefaultQuestStrategy::InitNonCombatTriggers(std::list<TriggerNode*> &triggers)
{
    QuestStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "use game object",
        NextAction::array(0,
            new NextAction("talk to quest giver", relevance), NULL)));

    triggers.push_back(new TriggerNode(
        "gossip hello",
        NextAction::array(0,
            new NextAction("talk to quest giver", relevance), NULL)));

    triggers.push_back(new TriggerNode(
        "complete quest",
        NextAction::array(0, new NextAction("talk to quest giver", relevance), NULL)));
}

void AcceptAllQuestsStrategy::InitNonCombatTriggers(std::list<TriggerNode*> &triggers)
{
    QuestStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "use game object",
        NextAction::array(0,
            new NextAction("talk to quest giver", relevance), new NextAction("accept all quests", relevance), NULL)));

    triggers.push_back(new TriggerNode(
        "gossip hello",
        NextAction::array(0,
            new NextAction("talk to quest giver", relevance), new NextAction("accept all quests", relevance), NULL)));

    triggers.push_back(new TriggerNode(
        "complete quest",
        NextAction::array(0, 
            new NextAction("talk to quest giver", relevance), new NextAction("accept all quests", relevance), NULL)));
}
