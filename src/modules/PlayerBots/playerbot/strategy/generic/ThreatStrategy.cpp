
#include "playerbot/playerbot.h"
#include "ThreatStrategy.h"
#include "playerbot/PlayerbotAIConfig.h"
#include "playerbot/strategy/actions/GenericSpellActions.h"

using namespace ai;

float ThreatMultiplier::GetValue(Action* action)
{
    if (action == NULL || action->getThreatType() == ActionThreatType::ACTION_THREAT_NONE)
        return 1.0f;

    if (!AI_VALUE(bool, "group"))
        return 1.0f;

    if (action->getThreatType() == ActionThreatType::ACTION_THREAT_AOE)
    {
        uint8 threat = AI_VALUE2(uint8, "threat", "aoe");
        if (threat >= 50)
            return 0.0f;
    }    
    

    if (ai->HasStrategy("debug threat", BotState::BOT_STATE_COMBAT))
    {
        if (ai->GetMaster())
        {
            if (AI_VALUE2(bool, "trigger active", "high threat"))
            {
                ai->GetMaster()->GetSession()->SendPlaySpellVisual(ai->GetBot()->GetObjectGuid(), 6372);
            }
            else if (AI_VALUE2(bool, "trigger active", "medium threat"))
            {
                ai->GetMaster()->GetSession()->SendPlaySpellVisual(ai->GetBot()->GetObjectGuid(), 5036);
            }
        }
    }

    if (AI_VALUE2(bool, "trigger active", "high threat"))
        return 0.0;

    return 1.0f;
}

void ThreatStrategy::InitCombatMultipliers(std::list<Multiplier*> &multipliers)
{
    multipliers.push_back(new ThreatMultiplier(ai));
}
