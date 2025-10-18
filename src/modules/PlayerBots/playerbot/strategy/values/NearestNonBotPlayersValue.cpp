
#include "playerbot/playerbot.h"
#include "NearestNonBotPlayersValue.h"

#include "Grids/GridNotifiers.h"
#include "Grids/GridNotifiersImpl.h"
#include "Grids/CellImpl.h"

using namespace ai;
using namespace MaNGOS;

void NearestNonBotPlayersValue::FindUnits(std::list<Unit*> &targets)
{
    AnyUnitInObjectRangeCheck u_check(bot, range);
    UnitListSearcher<AnyUnitInObjectRangeCheck> searcher(targets, u_check);
    Cell::VisitAllObjects(bot, searcher, range);
}

bool NearestNonBotPlayersValue::AcceptUnit(Unit* unit)
{
    ObjectGuid guid = unit->GetObjectGuid();
#ifdef MANGOS
    return guid.IsPlayer() && !((Player*)unit)->GetPlayerbotAI() && (!((Player*)unit)->isGameMaster() || ((Player*)unit)->isGMVisible());
#endif
#ifdef CMANGOS
    return guid.IsPlayer() && !((Player*)unit)->GetPlayerbotAI() && (!((Player*)unit)->IsGameMaster() || ((Player*)unit)->isGMVisible());
#endif
}
