
#include "playerbot/playerbot.h"
#include "FollowMasterStrategy.h"

using namespace ai;

void FollowMasterStrategy::InitNonCombatTriggers(std::list<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode(
        "out of react range",
        NextAction::array(0, new NextAction("check mount state", ACTION_HIGH), new NextAction("flee to master", ACTION_HIGH), NULL)));

    triggers.push_back(new TriggerNode(
        "update follow",
        NextAction::array(0, new NextAction("follow", ACTION_IDLE), NULL)));
}

void FollowMasterStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "out of react range",
        NextAction::array(0, new NextAction("check mount state", ACTION_HIGH), new NextAction("flee to master", ACTION_HIGH), NULL)));

    triggers.push_back(new TriggerNode(
        "update follow",
        NextAction::array(0, new NextAction("follow", ACTION_IDLE), NULL)));
}

void FollowMasterStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    InitNonCombatTriggers(triggers);
}

void FollowMasterStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "stop follow",
        NextAction::array(0, new NextAction("stop follow", ACTION_PASSTROUGH), NULL)));
}

void FollowMasterStrategy::OnStrategyAdded(BotState state)
{
    if (state != BotState::BOT_STATE_REACTION)
    {
        ai->ChangeStrategy("+follow", BotState::BOT_STATE_REACTION);
    }
}

void FollowMasterStrategy::OnStrategyRemoved(BotState state)
{
    if (state == ai->GetState() && ai->GetBot()->GetMotionMaster()->GetCurrentMovementGeneratorType() == FOLLOW_MOTION_TYPE)
    {
        ai->StopMoving();
    }

    if (state == BotState::BOT_STATE_REACTION)
        return;

    BotState checkState1, checkState2;

    if (state == BotState::BOT_STATE_COMBAT)
    {
        checkState1 = BotState::BOT_STATE_NON_COMBAT;
        checkState2 = BotState::BOT_STATE_DEAD;
    }
    else if (state == BotState::BOT_STATE_NON_COMBAT)
    {
        checkState1 = BotState::BOT_STATE_COMBAT;
        checkState2 = BotState::BOT_STATE_DEAD;
    }
    else
    {
        checkState1 = BotState::BOT_STATE_NON_COMBAT;
        checkState2 = BotState::BOT_STATE_COMBAT;
    }

    if (!ai->HasStrategy("follow", checkState1) && !ai->HasStrategy("follow", checkState2))
    {
        ai->ChangeStrategy("-follow", BotState::BOT_STATE_REACTION);
    }
}
