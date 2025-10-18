
#include "playerbot/playerbot.h"
#include "RpgStrategy.h"
#include "playerbot/strategy/actions/RpgSubActions.h"

using namespace ai;

float RpgActionMultiplier::GetValue(Action* action)
{
    if (action == NULL) return 1.0f;

    std::string nextAction = AI_VALUE(std::string, "next rpg action");
    std::string name = action->getName();

    if (dynamic_cast<RpgEnabled*>(action))
    {
        if (!nextAction.empty() && name != nextAction)
            return 0.1f;
        else if(nextAction.empty() && name == "rpg cancel")
            return 0.1f;
        else
            return 1.0f - (0.1f * urand(0, 2));
    }

    return 1.0f;
}

void RpgStrategy::OnStrategyAdded(BotState state)
{
    ai->ChangeStrategy("+rpg quest,+rpg vendor,+rpg explore,+rpg maintenance,+rpg player,+rpg bg,+rpg guild", state);
}
void RpgStrategy::OnStrategyRemoved(BotState state) 
{
    ai->ChangeStrategy("-rpg quest,-rpg vendor,-rpg explore,-rpg maintenance,-rpg player,-rpg bg,-rpg guild", state);
}

void RpgStrategy::InitNonCombatTriggers(std::list<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode(
        "no rpg target",
        NextAction::array(0, new NextAction("choose rpg target", 5.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "far from rpg target",
        NextAction::array(0, new NextAction("move to rpg target", 5.0f), NULL)));

    //triggers.push_back(new TriggerNode(
    //    "near rpg target",
    //    NextAction::array(0, new NextAction("rpg", 1.1f), NULL)));

    //Sub actions
    triggers.push_back(new TriggerNode(
        "rpg wander",
        NextAction::array(0, new NextAction("rpg cancel", 1.001f), NULL)));

    triggers.push_back(new TriggerNode(
        "rpg",
        NextAction::array(0, new NextAction("rpg stay", 1.001f), NULL)));

    triggers.push_back(new TriggerNode(
        "rpg",
        NextAction::array(0, new NextAction("rpg work", 1.001f), NULL)));

    triggers.push_back(new TriggerNode(
        "rpg",
        NextAction::array(0, new NextAction("rpg emote", 1.001f), NULL)));

    triggers.push_back(new TriggerNode(
        "rpg",
        NextAction::array(0, new NextAction("rpg cancel", 1.001f), NULL)));

    triggers.push_back(new TriggerNode(
        "rpg heal",
        NextAction::array(0, new NextAction("rpg heal", 1.025f), NULL)));

    triggers.push_back(new TriggerNode(
        "rpg use",
        NextAction::array(0, new NextAction("rpg use", 1.001f), NULL)));

    triggers.push_back(new TriggerNode(
        "rpg ai chat",
        NextAction::array(0, new NextAction("rpg ai chat", 1.003f), NULL)));   
}

void RpgStrategy::InitNonCombatMultipliers(std::list<Multiplier*>& multipliers)
{
    multipliers.push_back(new RpgActionMultiplier(ai));
}

void RpgQuestStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "rpg start quest",
        NextAction::array(0, new NextAction("rpg start quest", 1.080f), NULL)));

    triggers.push_back(new TriggerNode(
        "rpg end quest",
        NextAction::array(0, new NextAction("rpg end quest", 1.090f), NULL)));

    triggers.push_back(new TriggerNode(
        "rpg repeat quest",
        NextAction::array(0, new NextAction("rpg start quest", 1.030f), new NextAction("rpg end quest", 1.030f), NULL)));

    triggers.push_back(new TriggerNode(
        "rpg spell click",
        NextAction::array(0, new NextAction("rpg spell click", 1.003f), NULL)));

    triggers.push_back(new TriggerNode(
        "rpg gossip talk",
        NextAction::array(0, new NextAction("rpg gossip talk", 1.003f), NULL)));

    triggers.push_back(new TriggerNode(
        "rpg quest use",
        NextAction::array(0, new NextAction("rpg use", 1.003f), NULL)));
}

void RpgVendorStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "rpg buy",
        NextAction::array(0, new NextAction("rpg buy", 1.030f), NULL)));

    triggers.push_back(new TriggerNode(
        "rpg ah sell",
        NextAction::array(0, new NextAction("rpg ah sell", 1.110f), NULL)));

    triggers.push_back(new TriggerNode(
        "rpg ah buy",
        NextAction::array(0, new NextAction("rpg ah buy", 1.035f), NULL)));

    triggers.push_back(new TriggerNode(
        "rpg get mail",
        NextAction::array(0, new NextAction("rpg get mail", 1.070f), NULL)));

    triggers.push_back(new TriggerNode(
        "rpg sell",
        NextAction::array(0, new NextAction("rpg sell", 1.100f), NULL)));
}

void RpgExploreStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "rpg taxi",
        NextAction::array(0, new NextAction("rpg taxi", 1.005f), NULL)));

    triggers.push_back(new TriggerNode(
        "rpg discover",
        NextAction::array(0, new NextAction("rpg discover", 1.110f), NULL)));

    triggers.push_back(new TriggerNode(
        "rpg home bind",
        NextAction::array(0, new NextAction("rpg home bind", 1.060f), NULL)));
}

void RpgMaintenanceStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "rpg repair",
        NextAction::array(0, new NextAction("rpg repair", 1.095f), NULL)));

    triggers.push_back(new TriggerNode(
        "rpg train",
        NextAction::array(0, new NextAction("rpg train", 1.080f), NULL)));
}

void RpgGuildStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "rpg buy petition",
        NextAction::array(0, new NextAction("rpg buy petition", 1.040f), NULL)));
}

void RpgBgStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "rpg queue bg",
        NextAction::array(0, new NextAction("rpg queue bg", 1.085f), NULL)));
}

void RpgPlayerStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "rpg trade useful",
        NextAction::array(0, new NextAction("rpg trade useful", 1.030f), NULL)));

    triggers.push_back(new TriggerNode(
        "rpg enchant",
        NextAction::array(0, new NextAction("rpg enchant", 1.029f), NULL)));

    triggers.push_back(new TriggerNode(
        "rpg duel",
        NextAction::array(0, new NextAction("rpg duel", 1.010f), NULL)));
}

void RpgCraftStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "rpg spell",
        NextAction::array(0, new NextAction("rpg spell", 1.001f), NULL)));

    triggers.push_back(new TriggerNode(
        "rpg craft",
        NextAction::array(0, new NextAction("rpg craft", ai->HasCheat(BotCheatMask::item) ? 1.001f : 1.010f), NULL)));

    triggers.push_back(new TriggerNode(
        "rpg item",
        NextAction::array(0, new NextAction("rpg item", 1.003f), NULL)));

    triggers.push_back(new TriggerNode(
        "castnc",
        NextAction::array(0, new NextAction("cast custom nc spell", 0.9f), NULL)));
}

void RpgJumpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "random jump",
        NextAction::array(0, new NextAction("jump::random", ACTION_PASSTROUGH), NULL)));
}

void RpgJumpStrategy::InitCombatTriggers(std::list<TriggerNode *> &triggers)
{
    InitNonCombatTriggers(triggers);
}

void RpgJumpStrategy::InitReactionTriggers(std::list<TriggerNode *> &triggers)
{
    InitNonCombatTriggers(triggers);
}
