/* This file is part of the ScriptDev2 Project. See AUTHORS file for Copyright information
 * This program is free software licensed under GPL version 2
 * Please see the included DOCS/LICENSE.TXT for more information */

#include "AI/ScriptDevAI/PreCompiledHeader.h"
#include "Grids/Cell.h"
#include "Grids/CellImpl.h"
#include "Grids/GridNotifiers.h"
#include "Grids/GridNotifiersImpl.h"

// return closest GO in grid, with range from pSource
GameObject* GetClosestGameObjectWithEntry(WorldObject* pSource, uint32 uiEntry, float fMaxSearchRange)
{
    GameObject* pGo = nullptr;

    MaNGOS::NearestGameObjectEntryInObjectRangeCheck go_check(*pSource, uiEntry, fMaxSearchRange);
    MaNGOS::GameObjectLastSearcher<MaNGOS::NearestGameObjectEntryInObjectRangeCheck> searcher(pGo, go_check);

    Cell::VisitGridObjects(pSource, searcher, fMaxSearchRange);

    return pGo;
}

// return closest creature alive in grid, with range from pSource
Creature* GetClosestCreatureWithEntry(WorldObject* pSource, uint32 uiEntry, float fMaxSearchRange, bool bOnlyAlive/*=true*/, bool bOnlyDead/*=false*/, bool bExcludeSelf/*=false*/)
{
    Creature* pCreature = nullptr;

    MaNGOS::NearestCreatureEntryWithLiveStateInObjectRangeCheck creature_check(*pSource, uiEntry, bOnlyAlive, bOnlyDead, fMaxSearchRange, bExcludeSelf);
    MaNGOS::CreatureLastSearcher<MaNGOS::NearestCreatureEntryWithLiveStateInObjectRangeCheck> searcher(pCreature, creature_check);

    Cell::VisitGridObjects(pSource, searcher, fMaxSearchRange);

    return pCreature;
}

void GetGameObjectListWithEntryInGrid(std::list<GameObject*>& lList , WorldObject* pSource, uint32 uiEntry, float fMaxSearchRange)
{
    MaNGOS::GameObjectEntryInPosRangeCheck check(*pSource, uiEntry, pSource->GetPositionX(), pSource->GetPositionY(), pSource->GetPositionZ(), fMaxSearchRange);
    MaNGOS::GameObjectListSearcher<MaNGOS::GameObjectEntryInPosRangeCheck> searcher(lList, check);

    Cell::VisitGridObjects(pSource, searcher, fMaxSearchRange);
}

void GetCreatureListWithEntryInGrid(std::list<Creature*>& lList, WorldObject* pSource, uint32 uiEntry, float fMaxSearchRange)
{
    MaNGOS::AllCreaturesOfEntryInRangeCheck check(pSource, uiEntry, fMaxSearchRange);
    MaNGOS::CreatureListSearcher<MaNGOS::AllCreaturesOfEntryInRangeCheck> searcher(lList, check);

    Cell::VisitGridObjects(pSource, searcher, fMaxSearchRange);
}

void GetPlayerListWithEntryInWorld(std::list<Player*>& lList, WorldObject* pSource, float fMaxSearchRange)
{
    MaNGOS::AnyPlayerInObjectRangeCheck check(pSource, fMaxSearchRange);
    MaNGOS::PlayerListSearcher<MaNGOS::AnyPlayerInObjectRangeCheck> searcher(lList, check);

    Cell::VisitWorldObjects(pSource, searcher, fMaxSearchRange);
}
