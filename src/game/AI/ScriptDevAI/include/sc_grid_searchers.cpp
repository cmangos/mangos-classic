/* This file is part of the ScriptDev2 Project. See AUTHORS file for Copyright information
 * This program is free software licensed under GPL version 2
 * Please see the included DOCS/LICENSE.TXT for more information */

#include "AI/ScriptDevAI/include/precompiled.h"
#include "Grids/Cell.h"
#include "Grids/CellImpl.h"
#include "Grids/GridNotifiers.h"
#include "Grids/GridNotifiersImpl.h"

// return closest GO in grid, with range from pSource
GameObject* GetClosestGameObjectWithEntry(WorldObject* source, uint32 entry, float maxSearchRange)
{
    GameObject* go = nullptr;

    MaNGOS::NearestGameObjectEntryInObjectRangeCheck go_check(*source, entry, maxSearchRange);
    MaNGOS::GameObjectLastSearcher<MaNGOS::NearestGameObjectEntryInObjectRangeCheck> searcher(go, go_check);

    Cell::VisitGridObjects(source, searcher, maxSearchRange);

    return go;
}

// return closest creature alive in grid, with range from pSource
Creature* GetClosestCreatureWithEntry(WorldObject* source, uint32 entry, float maxSearchRange, bool onlyAlive/*=true*/, bool onlyDead/*=false*/, bool excludeSelf/*=false*/)
{
    Creature* creature = nullptr;

    MaNGOS::NearestCreatureEntryWithLiveStateInObjectRangeCheck creature_check(*source, entry, onlyAlive, onlyDead, maxSearchRange, excludeSelf);
    MaNGOS::CreatureLastSearcher<MaNGOS::NearestCreatureEntryWithLiveStateInObjectRangeCheck> searcher(creature, creature_check);

    Cell::VisitGridObjects(source, searcher, maxSearchRange);

    return creature;
}

void GetGameObjectListWithEntryInGrid(GameObjectList& goList, WorldObject* source, uint32 entry, float maxSearchRange)
{
    MaNGOS::GameObjectEntryInPosRangeCheck check(*source, entry, source->GetPositionX(), source->GetPositionY(), source->GetPositionZ(), maxSearchRange);
    MaNGOS::GameObjectListSearcher<MaNGOS::GameObjectEntryInPosRangeCheck> searcher(goList, check);

    Cell::VisitGridObjects(source, searcher, maxSearchRange);
}

void GetCreatureListWithEntryInGrid(CreatureList& creatureList, WorldObject* source, uint32 entry, float maxSearchRange)
{
    MaNGOS::AllCreaturesOfEntryInRangeCheck check(source, entry, maxSearchRange);
    MaNGOS::CreatureListSearcher<MaNGOS::AllCreaturesOfEntryInRangeCheck> searcher(creatureList, check);

    Cell::VisitGridObjects(source, searcher, maxSearchRange);
}

void GetPlayerListWithEntryInWorld(PlayerList& playerList, WorldObject* source, float maxSearchRange)
{
    MaNGOS::AnyPlayerInObjectRangeCheck check(source, maxSearchRange);
    MaNGOS::PlayerListSearcher<MaNGOS::AnyPlayerInObjectRangeCheck> searcher(playerList, check);

    Cell::VisitWorldObjects(source, searcher, maxSearchRange);
}
