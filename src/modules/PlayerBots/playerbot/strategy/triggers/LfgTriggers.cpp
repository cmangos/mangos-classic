
#include "playerbot/playerbot.h"
#include "LfgTriggers.h"

using namespace ai;

bool LfgProposalActiveTrigger::IsActive()
{
    return AI_VALUE(uint32, "lfg proposal");
}

bool UnknownDungeonTrigger::IsActive()
{
    return ai->HasActivePlayerMaster() &&
        ai->GetMaster() &&
        ai->GetMaster()->IsInWorld() &&
        ai->GetMaster()->GetMap()->IsDungeon() &&
        bot->GetMapId() == ai->GetMaster()->GetMapId();
}
