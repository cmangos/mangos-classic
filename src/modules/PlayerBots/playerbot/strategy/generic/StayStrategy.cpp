
#include "playerbot/playerbot.h"
#include "playerbot/strategy/values/PositionValue.h"
#include "StayStrategy.h"

using namespace ai;

NextAction** StayStrategy::GetDefaultNonCombatActions()
{
    return NextAction::array(0, new NextAction("stay", 1.0f), NULL);
}

ai::NextAction** StayStrategy::GetDefaultCombatActions()
{
    return GetDefaultNonCombatActions();
}

void StayStrategy::OnStrategyAdded(BotState state)
{
    if (state == ai->GetState() && ai->GetBot()->GetMotionMaster()->GetCurrentMovementGeneratorType() != IDLE_MOTION_TYPE)
    {
        ai->StopMoving();
    }
}

void StayStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "return to stay position",
        NextAction::array(0, new NextAction("return to stay position", 1.5f), NULL)));
}

void StayStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    InitNonCombatTriggers(triggers);
}

void SitStrategy::InitNonCombatTriggers(std::list<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode(
        "sit",
        NextAction::array(0, new NextAction("sit", 1.5f), NULL)));
}
