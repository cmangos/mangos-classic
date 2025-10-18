
#include "playerbot/playerbot.h"
#include "TravelTriggers.h"

#include "playerbot/PlayerbotAIConfig.h"
#include "playerbot/TravelMgr.h"
#include "playerbot/ServerFacade.h"
using namespace ai;

bool HasNearbyQuestTakerTrigger::IsActive()
{
    TravelTarget* target = AI_VALUE(TravelTarget*, "travel target");
    if (target->GetStatus() == TravelStatus::TRAVEL_STATUS_WORK) //We are not currently working on a target.
        return false;

    if (target->GetExpiredTime() < 2 * MINUTE) //The target was set more than 2 minutes ago.
        return false;

    return AI_VALUE(bool, "has nearby quest taker");
}

bool NearDarkPortalTrigger::IsActive()
{
    return sServerFacade.GetAreaId(bot) == 72;
}

bool AtDarkPortalAzerothTrigger::IsActive()
{
    if (sServerFacade.GetAreaId(bot) == 72)
    {
        if (sServerFacade.GetDistance2d(bot, -11906.9f, -3208.53f) < 20.0f)
        {
            return true;
        }
    }
    return false;
}

bool AtDarkPortalOutlandTrigger::IsActive()
{
    if (sServerFacade.GetAreaId(bot) == 3539)
    {
        if (sServerFacade.GetDistance2d(bot, -248.1939f, 921.919f) < 10.0f)
        {
            return true;
        }
    }
    return false;
}
