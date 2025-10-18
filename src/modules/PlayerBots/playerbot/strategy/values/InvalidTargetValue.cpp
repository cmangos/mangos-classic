
#include "playerbot/playerbot.h"
#include "InvalidTargetValue.h"
#include "PossibleAttackTargetsValue.h"
#include "EnemyPlayerValue.h"
#include "playerbot/PlayerbotAIConfig.h"
#include "playerbot/ServerFacade.h"

using namespace ai;

bool InvalidTargetValue::Calculate()
{
    Unit* target = AI_VALUE(Unit*, qualifier);
    if (!target || !target->IsInWorld() || target->GetMapId() != bot->GetMapId())
    {
        return true;
    }

    Unit* duelTarget = AI_VALUE(Unit*, "duel target");
    if (duelTarget && duelTarget == target)
    {
        return false;
    }

    if (qualifier == "current target")
    {
        if (target->GetObjectGuid() != bot->GetSelectionGuid())
        {
            return true;
        }
    }

    const bool validTarget = PossibleAttackTargetsValue::IsValid(target, bot);
    if (!validTarget)
    {
        std::list<ObjectGuid> attackers = AI_VALUE(std::list<ObjectGuid>, "possible attack targets");
        if (std::find(attackers.begin(), attackers.end(), target->GetObjectGuid()) != attackers.end())
        {
            return false;
        }
    }

    return !validTarget;
}