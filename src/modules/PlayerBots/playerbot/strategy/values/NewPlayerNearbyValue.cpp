
#include "playerbot/playerbot.h"
#include "NewPlayerNearbyValue.h"

using namespace ai;

ObjectGuid NewPlayerNearbyValue::Calculate()
{
    std::list<ObjectGuid> players = ai->GetAiObjectContext()->GetValue<std::list<ObjectGuid> >("nearest friendly players")->Get();
    std::set<ObjectGuid>& alreadySeenPlayers = ai->GetAiObjectContext()->GetValue<std::set<ObjectGuid>& >("already seen players")->Get();
    for (std::list<ObjectGuid>::iterator i = players.begin(); i != players.end(); ++i)
    {
        ObjectGuid guid = *i;
        if (alreadySeenPlayers.find(guid) == alreadySeenPlayers.end())
            return guid;
    }

    return ObjectGuid();
}
