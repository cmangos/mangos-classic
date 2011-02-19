/*
 * Copyright (C) 2005-2011 MaNGOS <http://getmangos.com/>
 * Copyright (C) 2009-2011 MaNGOSZero <http://github.com/mangoszero/mangoszero/>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "MapInstanced.h"
#include "ObjectMgr.h"
#include "MapManager.h"
#include "BattleGround.h"
#include "VMapFactory.h"
#include "InstanceSaveMgr.h"
#include "World.h"

MapInstanced::MapInstanced(uint32 id, time_t expiry) : Map(id, expiry, 0)
{
    // initialize instanced maps list
    m_InstancedMaps.clear();
    // fill with zero
    memset(&GridMapReference, 0, MAX_NUMBER_OF_GRIDS*MAX_NUMBER_OF_GRIDS*sizeof(uint16));
}

void MapInstanced::InitVisibilityDistance()
{
    if(m_InstancedMaps.empty())
        return;
    //initialize visibility distances for all instance copies
    for (InstancedMaps::iterator i = m_InstancedMaps.begin(); i != m_InstancedMaps.end(); ++i)
    {
        (*i).second->InitVisibilityDistance();
    }
}

void MapInstanced::Update(const uint32& t)
{
    // take care of loaded GridMaps (when unused, unload it!)
    Map::Update(t);

    // update the instanced maps
    InstancedMaps::iterator i = m_InstancedMaps.begin();

    while (i != m_InstancedMaps.end())
    {
        if(i->second->CanUnload(t))
        {
            DestroyInstance(i);                             // iterator incremented
        }
        else
        {
            // update only here, because it may schedule some bad things before delete
            i->second->Update(t);
            ++i;
        }
    }
}

void MapInstanced::RemoveAllObjectsInRemoveList()
{
    for (InstancedMaps::iterator i = m_InstancedMaps.begin(); i != m_InstancedMaps.end(); ++i)
    {
        i->second->RemoveAllObjectsInRemoveList();
    }

    Map::RemoveAllObjectsInRemoveList();
}

void MapInstanced::UnloadAll(bool pForce)
{
    // Unload instanced maps
    for (InstancedMaps::iterator i = m_InstancedMaps.begin(); i != m_InstancedMaps.end(); ++i)
        i->second->UnloadAll(pForce);

    // Delete the maps only after everything is unloaded to prevent crashes
    for (InstancedMaps::iterator i = m_InstancedMaps.begin(); i != m_InstancedMaps.end(); ++i)
        delete i->second;

    m_InstancedMaps.clear();

    // Unload own grids (just dummy(placeholder) grids, neccesary to unload GridMaps!)
    Map::UnloadAll(pForce);
}

/// returns a new or existing Instance
/// in case of battlegrounds it will only return an existing map, those maps are created by bg-system
Map* MapInstanced::CreateInstance(Player * player)
{
    Map* map;
    uint32 NewInstanceId;                                   // instanceId of the resulting map

    if(IsBattleGround())
    {
        // find existing bg map for player
        NewInstanceId = player->GetBattleGroundId();
        MANGOS_ASSERT(NewInstanceId);
        map = _FindMap(NewInstanceId);
        MANGOS_ASSERT(map);
    }
    else if (InstanceSave* pSave = player->GetBoundInstanceSaveForSelfOrGroup(GetId()))
    {
        // solo/perm/group
        NewInstanceId = pSave->GetInstanceId();
        map = _FindMap(NewInstanceId);
        // it is possible that the save exists but the map doesn't
        if (!map)
            map = CreateInstanceMap(NewInstanceId, pSave);
    }
    else
    {
        // if no instanceId via group members or instance saves is found
        // the instance will be created for the first time
        NewInstanceId = sMapMgr.GenerateInstanceId();

        map = CreateInstanceMap(NewInstanceId);
    }

    return map;
}

InstanceMap* MapInstanced::CreateInstanceMap(uint32 InstanceId, InstanceSave *save)
{
    // load/create a map
    Guard guard(*this);

    // make sure we have a valid map id
    const MapEntry* entry = sMapStore.LookupEntry(GetId());
    if (!entry)
    {
        sLog.outError("CreateInstanceMap: no entry for map %d", GetId());
        MANGOS_ASSERT(false);
    }
    if (!ObjectMgr::GetInstanceTemplate(GetId()))
    {
        sLog.outError("CreateInstanceMap: no instance template for map %d", GetId());
        MANGOS_ASSERT(false);
    }

    DEBUG_LOG("MapInstanced::CreateInstance: %s map instance %d for %d created", save?"":"new ", InstanceId, GetId());

    InstanceMap *map = new InstanceMap(GetId(), GetGridExpiry(), InstanceId, this);
    MANGOS_ASSERT(map->IsDungeon());

    bool load_data = save != NULL;
    map->CreateInstanceData(load_data);

    m_InstancedMaps[InstanceId] = map;
    return map;
}

BattleGroundMap* MapInstanced::CreateBattleGroundMap(uint32 InstanceId, BattleGround* bg)
{
    // load/create a map
    Guard guard(*this);

    DEBUG_LOG("MapInstanced::CreateBattleGroundMap: instance:%d for map:%d and bgType:%d created.", InstanceId, GetId(), bg->GetTypeID());

    BattleGroundMap *map = new BattleGroundMap(GetId(), GetGridExpiry(), InstanceId, this);
    MANGOS_ASSERT(map->IsBattleGround());
    map->SetBG(bg);
    bg->SetBgMap(map);

    m_InstancedMaps[InstanceId] = map;
    return map;
}

void MapInstanced::DestroyInstance(uint32 InstanceId)
{
    InstancedMaps::iterator itr = m_InstancedMaps.find(InstanceId);
    if(itr != m_InstancedMaps.end())
        DestroyInstance(itr);
}

// increments the iterator after erase
void MapInstanced::DestroyInstance(InstancedMaps::iterator &itr)
{
    itr->second->UnloadAll(true);
    // should only unload VMaps if this is the last instance and grid unloading is enabled
    if(m_InstancedMaps.size() <= 1 && sWorld.getConfig(CONFIG_BOOL_GRID_UNLOAD))
    {
        VMAP::VMapFactory::createOrGetVMapManager()->unloadMap(itr->second->GetId());
        // in that case, unload grids of the base map, too
        // so in the next map creation, (EnsureGridCreated actually) VMaps will be reloaded
        Map::UnloadAll(true);
    }
    // erase map
    delete itr->second;
    m_InstancedMaps.erase(itr++);
}
