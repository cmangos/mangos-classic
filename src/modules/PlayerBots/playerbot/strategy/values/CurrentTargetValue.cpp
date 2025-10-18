
#include "playerbot/playerbot.h"
#include "CurrentTargetValue.h"

#include "playerbot/ServerFacade.h"
using namespace ai;

Unit* CurrentTargetValue::Get()
{
    if (selection.IsEmpty())
        return NULL;

    Unit* unit = sObjectAccessor.GetUnit(*bot, selection);
    if (unit && !bot->IsWithinDistInMap(unit, sPlayerbotAIConfig.sightDistance))
        return NULL;

    return unit;
}

void CurrentTargetValue::Set(Unit* target)
{
    selection = target ? target->GetObjectGuid() : ObjectGuid();
}
