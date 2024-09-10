/*
 * This file is part of the CMaNGOS Project. See AUTHORS file for Copyright information
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

#include "Maps/MapManager.h"
#include "Maps/MapPersistentStateMgr.h"
#include "Policies/Singleton.h"
#include "Database/DatabaseEnv.h"
#include "Log/Log.h"
#include "Entities/Transports.h"
#include "Maps/GridDefines.h"
#include "World/World.h"
#include "Grids/CellImpl.h"
#include "Globals/ObjectMgr.h"
#include "Maps/MapWorkers.h"
#include "BattleGround/BattleGroundMgr.h"
#include <future>

#define CLASS_LOCK MaNGOS::ClassLevelLockable<MapManager, std::recursive_mutex>
INSTANTIATE_SINGLETON_2(MapManager, CLASS_LOCK);
INSTANTIATE_CLASS_MUTEX(MapManager, std::recursive_mutex);

MapManager::MapManager()
    : i_gridCleanUpDelay(sWorld.getConfig(CONFIG_UINT32_INTERVAL_GRIDCLEAN))
{
    i_timer.SetInterval(sWorld.getConfig(CONFIG_UINT32_INTERVAL_MAPUPDATE));
}

MapManager::~MapManager()
{
    i_maps.clear();

    DeleteStateMachine();
}

void MapManager::Initialize()
{
    InitStateMachine();
    InitMaxInstanceId();
    CreateContinents();

    int num_threads(sWorld.getConfig(CONFIG_UINT32_NUM_MAP_THREADS));
    if (num_threads > 0)
        m_updater.activate(num_threads);
}

void MapManager::InitStateMachine()
{
    si_GridStates[GRID_STATE_INVALID] = new InvalidState;
    si_GridStates[GRID_STATE_ACTIVE] = new ActiveState;
    si_GridStates[GRID_STATE_IDLE] = new IdleState;
    si_GridStates[GRID_STATE_REMOVAL] = new RemovalState;
}

void MapManager::DeleteStateMachine()
{
    delete si_GridStates[GRID_STATE_INVALID];
    delete si_GridStates[GRID_STATE_ACTIVE];
    delete si_GridStates[GRID_STATE_IDLE];
    delete si_GridStates[GRID_STATE_REMOVAL];
}

void MapManager::UpdateGridState(grid_state_t state, Map& map, NGridType& ngrid, GridInfo& ginfo, const uint32& x, const uint32& y, const uint32& t_diff)
{
    // TODO: The grid state array itself is static and therefore 100% safe, however, the data
    // the state classes in it accesses is not, since grids are shared across maps (for example
    // in instances), so some sort of locking will be necessary later.

    si_GridStates[state]->Update(map, ngrid, ginfo, x, y, t_diff);
}

void MapManager::InitializeVisibilityDistanceInfo()
{
    for (auto& i_map : i_maps)
        i_map.second->InitVisibilityDistance();
}

void MapManager::CreateContinents()
{
    std::vector<std::future<void>> futures;
    uint32 continents[] = { 0, 1 };
    for (auto id : continents)
    {
        Map* m = new WorldMap(id, i_gridCleanUpDelay, 0);
        // add map into container
        MaNGOS::unique_trackable_ptr<Map>& ptr = i_maps[MapID(id)];
        ptr.reset(m);
        m->SetWeakPtr(ptr);

        // non-instanceable maps always expected have saved state
        futures.push_back(std::async(std::launch::async, std::bind(&Map::Initialize, m, true)));
    }

    for (auto& futurItr : futures)
        futurItr.wait();
}

/// @param id - MapId of the to be created map. @param obj WorldObject for which the map is to be created. Must be player for Instancable maps.
Map* MapManager::CreateMap(uint32 id, const WorldObject* obj)
{
    Guard _guard(*this);

    Map* m = nullptr;

    const MapEntry* entry = sMapStore.LookupEntry(id);
    if (!entry)
        return nullptr;

    if (entry->Instanceable())
    {
        MANGOS_ASSERT(obj && obj->GetTypeId() == TYPEID_PLAYER);
        // create DungeonMap object
        m = CreateInstance(id, (Player*)obj);
    }
    else
    {
        uint32 instanceId = 0;
        // create regular non-instanceable map
        m = FindMap(id);
        if (m == nullptr)
        {
            std::lock_guard<std::mutex> lock(m_lock);
            m = new WorldMap(id, i_gridCleanUpDelay, instanceId);
            // add map into container
            MaNGOS::unique_trackable_ptr<Map>& ptr = i_maps[MapID(id, instanceId)];
            ptr.reset(m);
            m->SetWeakPtr(ptr);

            // non-instanceable maps always expected have saved state
            m->Initialize();
        }
    }

    return m;
}

Map* MapManager::CreateBgMap(uint32 mapid, uint32 instanceId, BattleGround* bg)
{
    sTerrainMgr.LoadTerrain(mapid);

    Guard _guard(*this);
    return CreateBattleGroundMap(mapid, instanceId, bg);
}

Map* MapManager::FindMap(uint32 mapid, uint32 instanceId) const
{
    Guard guard(*this);

    MapMapType::const_iterator iter = i_maps.find(MapID(mapid, instanceId));
    if (iter == i_maps.end())
        return nullptr;

    // this is a small workaround for transports
    if (instanceId == 0 && iter->second->Instanceable())
    {
        assert(false);
        return nullptr;
    }

    return iter->second.get();
}

void MapManager::DeleteInstance(uint32 mapid, uint32 instanceId)
{
    Guard _guard(*this);

    MapMapType::iterator iter = i_maps.find(MapID(mapid, instanceId));
    if (iter != i_maps.end())
    {
        if (iter->second->Instanceable())
        {
            auto node = i_maps.extract(iter);

            node.mapped()->UnloadAll(true);
        }
    }
}

void MapManager::Update(uint32 diff)
{
    i_timer.Update(diff);
    if (!i_timer.Passed())
        return;

    for (auto& map : i_maps)
    {
        if (m_updater.activated())
            m_updater.schedule_update(new MapUpdateWorker(*map.second, (uint32)i_timer.GetCurrent(), m_updater));
        else
            map.second->Update((uint32)i_timer.GetCurrent());
    }

    if (m_updater.activated())
        m_updater.wait();

    // remove all maps which can be unloaded
    MapMapType::iterator iter = i_maps.begin();
    while (iter != i_maps.end())
    {
        // check if map can be unloaded
        if (iter->second->CanUnload((uint32)i_timer.GetCurrent()))
        {
            auto node = i_maps.extract(iter++);

            node.mapped()->UnloadAll(true);
        }
        else
            ++iter;
    }

    i_timer.SetCurrent(0);
}

void MapManager::RemoveAllObjectsInRemoveList()
{
    for (auto& i_map : i_maps)
        i_map.second->RemoveAllObjectsInRemoveList();
}

bool MapManager::ExistMapAndVMap(uint32 mapid, float x, float y)
{
    GridPair p = MaNGOS::ComputeGridPair(x, y);

    int gx = 63 - p.x_coord;
    int gy = 63 - p.y_coord;

    return GridMap::ExistMap(mapid, gx, gy) && GridMap::ExistVMap(mapid, gx, gy);
}

bool MapManager::IsValidMAP(uint32 mapid)
{
    MapEntry const* mEntry = sMapStore.LookupEntry(mapid);
    return mEntry && (!mEntry->IsDungeon() || ObjectMgr::GetInstanceTemplate(mapid));
    // TODO: add check for battleground template
}

void MapManager::UnloadAll()
{
    for (auto& i_map : i_maps)
        i_map.second->UnloadAll(true);

    i_maps.clear();

    if (m_updater.activated())
        m_updater.deactivate();

    TerrainManager::Instance().UnloadAll();
}

void MapManager::InitMaxInstanceId()
{
    i_MaxInstanceId = 0;

    auto queryResult = CharacterDatabase.Query("SELECT MAX(id) FROM instance");
    if (queryResult)
    {
        i_MaxInstanceId = queryResult->Fetch()[0].GetUInt32();
    }
}

uint32 MapManager::GetNumInstances()
{
    std::lock_guard<std::mutex> lock(m_lock);

    uint32 ret = 0;
    for (auto& i_map : i_maps)
    {
        Map* map = i_map.second.get();
        if (!map->IsDungeon()) continue;
        ret += 1;
    }
    return ret;
}

uint32 MapManager::GetNumPlayersInInstances()
{
    std::lock_guard<std::mutex> lock(m_lock);
    uint32 ret = 0;
    for (auto& i_map : i_maps)
    {
        Map* map = i_map.second.get();
        if (!map->IsDungeon()) continue;
        ret += map->GetPlayers().getSize();
    }
    return ret;
}

///// returns a new or existing Instance
///// in case of battlegrounds it will only return an existing map, those maps are created by bg-system
Map* MapManager::CreateInstance(uint32 id, Player* player)
{
    Map* map = nullptr;
    Map* pNewMap = nullptr;
    uint32 NewInstanceId = 0;                               // instanceId of the resulting map
    const MapEntry* entry = sMapStore.LookupEntry(id);

    if (entry->IsBattleGround())
    {
        // find existing bg map for player
        NewInstanceId = player->GetBattleGroundId();
        MANGOS_ASSERT(NewInstanceId);
        map = FindMap(id, NewInstanceId);
        MANGOS_ASSERT(map);
    }
    else if (DungeonPersistentState* pSave = player->GetBoundInstanceSaveForSelfOrGroup(id))
    {
        // solo/perm/group
        NewInstanceId = pSave->GetInstanceId();
        map = FindMap(id, NewInstanceId);
        // it is possible that the save exists but the map doesn't
        if (!map)
            pNewMap = CreateDungeonMap(id, NewInstanceId, pSave);
    }
    else
    {
        // if no instanceId via group members or instance saves is found
        // the instance will be created for the first time
        NewInstanceId = GenerateInstanceId();

        pNewMap = CreateDungeonMap(id, NewInstanceId);
    }

    // add a new map object into the registry
    if (pNewMap)
    {
        MaNGOS::unique_trackable_ptr<Map>& ptr = i_maps[MapID(id, NewInstanceId)];
        ptr.reset(pNewMap);
        pNewMap->SetWeakPtr(ptr);
        map = pNewMap;
    }

    return map;
}

DungeonMap* MapManager::CreateDungeonMap(uint32 id, uint32 InstanceId, DungeonPersistentState* save)
{
    // make sure we have a valid map id
    const MapEntry* entry = sMapStore.LookupEntry(id);
    if (!entry)
    {
        sLog.outError("CreateDungeonMap: no entry for map %d", id);
        MANGOS_ASSERT(false);
    }
    if (!ObjectMgr::GetInstanceTemplate(id))
    {
        sLog.outError("CreateDungeonMap: no instance template for map %d", id);
        MANGOS_ASSERT(false);
    }

    DEBUG_LOG("MapInstanced::CreateInstanceMap: %s map instance %d for %d created", save ? "" : "new ", InstanceId, id);

    DungeonMap* map = new DungeonMap(id, i_gridCleanUpDelay, InstanceId);

    // Dungeons can have saved instance data
    bool load_data = save != nullptr;
    map->Initialize(load_data);

    return map;
}

BattleGroundMap* MapManager::CreateBattleGroundMap(uint32 id, uint32 InstanceId, BattleGround* bg)
{
    DEBUG_LOG("MapInstanced::CreateBattleGroundMap: instance:%d for map:%d and bgType:%d created.", InstanceId, id, bg->GetTypeId());

    BattleGroundMap* map = new BattleGroundMap(id, i_gridCleanUpDelay, InstanceId);
    MANGOS_ASSERT(map->IsBattleGround());
    map->SetBG(bg);
    bg->SetBgMap(map);

    // add map into map container
    MaNGOS::unique_trackable_ptr<Map>& ptr = i_maps[MapID(id, InstanceId)];
    ptr.reset(map);
    map->SetWeakPtr(ptr);

    // BGs/Arenas not have saved instance data
    map->Initialize(false);

    return map;
}

void MapManager::DoForAllMapsWithMapId(uint32 mapId, std::function<void(Map*)> worker)
{
    MapMapType::const_iterator start = i_maps.lower_bound(MapID(mapId, 0));
    MapMapType::const_iterator end = i_maps.lower_bound(MapID(mapId + 1, 0));
    for (MapMapType::const_iterator itr = start; itr != end; ++itr)
        worker(itr->second.get());
}

void MapManager::DoForAllMaps(const std::function<void(Map*)>& worker)
{
    for (MapMapType::const_iterator itr = i_maps.begin(); itr != i_maps.end(); ++itr)
        worker(itr->second.get());
}
