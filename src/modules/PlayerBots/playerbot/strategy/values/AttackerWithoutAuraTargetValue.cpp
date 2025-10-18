
#include "playerbot/playerbot.h"
#include "AttackerWithoutAuraTargetValue.h"
#include "playerbot/PlayerbotAIConfig.h"

using namespace ai;

Unit* AttackerWithoutAuraTargetValue::Calculate()
{
    std::list<ObjectGuid> attackers = ai->GetAiObjectContext()->GetValue<std::list<ObjectGuid>>("possible attack targets")->Get();
    Unit* target = ai->GetAiObjectContext()->GetValue<Unit*>("current target")->Get();
    for (std::list<ObjectGuid>::iterator i = attackers.begin(); i != attackers.end(); ++i)
    {
        Unit* unit = ai->GetUnit(*i);
        if (!unit || unit == target)
            continue;

        if (bot->GetDistance(unit) > ai->GetRange("spell"))
            continue;

        if (!ai->HasAura(qualifier, unit))
            return unit;
    }

    return NULL;
}
