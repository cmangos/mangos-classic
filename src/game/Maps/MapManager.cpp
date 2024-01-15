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
#include "Log.h"
#include "Entities/Transports.h"
#include "Maps/GridDefines.h"
#include "World/World.h"
#include "Grids/CellImpl.h"
#include "Globals/ObjectMgr.h"
#include "Maps/MapWorkers.h"
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
    for (auto& i_map : i_maps)
        delete i_map.second;

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
        i_maps[MapID(id)] = m;

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
            i_maps[MapID(id, instanceId)] = m;

            // non-instanceable maps always expected have saved state
            m->Initialize();
        }
    }

    return m;
}

Map* MapManager::CreateBgMap(uint32 mapid, BattleGround* bg)
{
    sTerrainMgr.LoadTerrain(mapid);

    Guard _guard(*this);
    return CreateBattleGroundMap(mapid, sMapMgr.GenerateInstanceId(), bg);
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

    return iter->second;
}

void MapManager::DeleteInstance(uint32 mapid, uint32 instanceId)
{
    Guard _guard(*this);

    MapMapType::iterator iter = i_maps.find(MapID(mapid, instanceId));
    if (iter != i_maps.end())
    {
        Map* pMap = iter->second;
        if (pMap->Instanceable())
        {
            i_maps.erase(iter);

            pMap->UnloadAll(true);
            delete pMap;
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
        Map* pMap = iter->second;
        // check if map can be unloaded
        if (pMap->CanUnload((uint32)i_timer.GetCurrent()))
        {
            pMap->UnloadAll(true);
            delete pMap;

            i_maps.erase(iter++);
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

    while (!i_maps.empty())
    {
        delete i_maps.begin()->second;
        i_maps.erase(i_maps.begin());
    }

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
        Map* map = i_map.second;
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
        Map* map = i_map.second;
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
        i_maps[MapID(id, NewInstanceId)] = pNewMap;
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
    i_maps[MapID(id, InstanceId)] = map;

    // BGs/Arenas not have saved instance data
    map->Initialize(false);

    return map;
}

#ifdef ENABLE_MANGOSBOTS
bool IsNorthTo(float x, float y, float const* limits, int count /* last case is limits[2*count - 1] */)
{
    int insideCount = 0;
    for (int i = 0; i < count - 1; ++i)
    {
        if ((limits[2 * i + 1] < y && y < limits[2 * i + 3]) || (limits[2 * i + 1] > y && y > limits[2 * i + 3]))
        {
            float threshold = limits[2 * i] + (limits[2 * i + 2] - limits[2 * i]) * (y - limits[2 * i + 1]) / (limits[2 * i + 3] - limits[2 * i + 1]);
            if (x > threshold)
                ++insideCount;
        }
    }
    return insideCount % 2 == 1;
}

ContinentArea MapManager::GetContinentInstanceId(uint32 mapId, float x, float y, bool* transitionArea)
{
    if (transitionArea)
        *transitionArea = false;

    // Y = horizontal axis on wow ...
    switch (mapId)
    {
    case 0:
    {
        static float const topNorthSouthLimit[] = {
            2032.048340f, -6927.750000f,
            1634.863403f, -6157.505371f,
            1109.519775f, -5181.036133f,
            1315.204712f, -4096.020508f,
            1073.089233f, -3372.571533f,
             825.833191f, -3125.778809f,
             657.343994f, -2314.813232f,
             424.736145f, -1888.283691f,
             744.395813f, -1647.935425f,
            1424.160645f,  -654.948181f,
            1447.065308f,  -169.751358f,
            1208.715454f,   189.748703f,
            1596.240356f,   998.616699f,
            1577.923706f,  1293.419922f,
            1458.520264f,  1727.373291f,
            1591.916138f,  3728.139404f
        };
        static float const ironforgeAreaSouthLimit[] = {
            -7491.33f,  3093.74f,
            -7472.04f,  -391.88f,
            -6366.68f,  -730.10f,
            -6063.96f, -1411.76f,
            -6087.62f, -2190.21f,
            -6349.54f, -2533.66f,
            -6308.63f, -3049.32f,
            -6107.82f, -3345.30f,
            -6008.49f, -3590.52f,
            -5989.37f, -4312.29f,
            -5806.26f, -5864.11f
        };
        static float const stormwindAreaNorthLimit[] = {
             -8004.25f,  3714.11f,
             -8075.00f, -179.00f,
             -8638.00f, 169.00f,
             -9044.00f, 35.00f,
             -9068.00f, -125.00f,
             -9094.00f, -147.00f,
             -9206.00f, -290.00f,
             -9097.00f, -510.00f,
             -8739.00f, -501.00f,
             -8725.50f, -1618.45f,
             -9810.40f, -1698.41f,
            -10049.60f, -1740.40f,
            -10670.61f, -1692.51f,
            -10908.48f, -1563.87f,
            -13006.40f, -1622.80f,
            -12863.23f, -4798.42f
        };
        static float const stormwindAreaSouthLimit[] = {
             -8725.337891f,  3535.624023f,
             -9525.699219f,   910.132568f,
             -9796.953125f,   839.069580f,
             -9946.341797f,   743.102844f,
            -10287.361328f,   760.076477f,
            -10083.828125f,   380.389893f,
            -10148.072266f,    80.056450f,
            -10014.583984f,  -161.638519f,
             -9978.146484f,  -361.638031f,
             -9877.489258f,  -563.304871f,
             -9980.967773f, -1128.510498f,
             -9991.717773f, -1428.793213f,
             -9887.579102f, -1618.514038f,
            -10169.600586f, -1801.582031f,
             -9966.274414f, -2227.197754f,
             -9861.309570f, -2989.841064f,
             -9944.026367f, -3205.886963f,
             -9610.209961f, -3648.369385f,
             -7949.329590f, -4081.389404f,
             -7910.859375f, -5855.578125f
        };
        if (IsNorthTo(x, y, topNorthSouthLimit, sizeof(topNorthSouthLimit) / (2 * sizeof(float))))
            return MAP0_TOP_NORTH;
        if (x > -2521)
            return MAP0_MIDDLE_NORTH;
        if (IsNorthTo(x, y, ironforgeAreaSouthLimit, sizeof(ironforgeAreaSouthLimit) / (2 * sizeof(float))))
            return MAP0_IRONFORGE_AREA;
        if (IsNorthTo(x, y, stormwindAreaNorthLimit, sizeof(stormwindAreaNorthLimit) / (2 * sizeof(float))))
            return MAP0_MIDDLE;
        if (IsNorthTo(x, y, stormwindAreaSouthLimit, sizeof(stormwindAreaNorthLimit) / (2 * sizeof(float))))
            return MAP0_STORMWIND_AREA;
        return MAP0_SOUTH;
    }
    case 1:
    {
        static float const teldrassilSouthLimit[] = {
            7916.0f,   3475.0f,
            7916.0f,   1000.0f,
            8283.0f,   -501.0f,
            8804.0f,   -1098.0f
        };
        static float const northMiddleLimit[] = {
              -2280.00f,  4054.00f,
              -2401.00f,  2365.00f,
              -2432.00f,  1338.00f,
              -2286.00f,   769.00f,
              -2137.00f,   662.00f,
              -2044.54f,   489.86f,
              -1808.52f,   436.39f,
              -1754.85f,   504.55f,
              -1094.55f,   651.75f,
               -747.46f,   647.73f,
               -685.55f,   408.43f,
               -311.38f,   114.43f,
               -358.40f,  -587.42f,
               -377.92f,  -748.70f,
               -512.57f,  -919.49f,
               -280.65f, -1008.87f,
                -81.29f,  -930.89f,
                284.31f, -1105.39f,
                568.86f,  -892.28f,
               1211.09f, -1135.55f,
                879.60f, -2110.18f,
                788.96f, -2276.02f,
                899.68f, -2625.56f,
               1281.54f, -2689.42f,
               1521.82f, -3047.85f,
               1424.22f, -3365.69f,
               1694.11f, -3615.20f,
               2373.78f, -4019.96f,
               2388.13f, -5124.35f,
               2193.79f, -5484.38f,
               1703.57f, -5510.53f,
               1497.59f, -6376.56f,
               1368.00f, -8530.00f
        };
        static float const durotarSouthLimit[] = {
                2755.00f, -3766.00f,
                2225.00f, -3596.00f,
                1762.00f, -3746.00f,
                1564.00f, -3943.00f,
                1184.00f, -3915.00f,
                 737.00f, -3782.00f,
                 -75.00f, -3742.00f,
                -263.00f, -3836.00f,
                -173.00f, -4064.00f,
                 -81.00f, -4091.00f,
                 -49.00f, -4089.00f,
                 -16.00f, -4187.00f,
                  -5.00f, -4192.00f,
                 -14.00f, -4551.00f,
                -397.00f, -4601.00f,
                -522.00f, -4583.00f,
                -668.00f, -4539.00f,
                -790.00f, -4502.00f,
               -1176.00f, -4213.00f,
               -1387.00f, -4674.00f,
               -2243.00f, -6046.00f
        };
        static float const valleyoftrialsSouthLimit[] = {
                -324.00f, -3869.00f,
                -774.00f, -3992.00f,
                -965.00f, -4290.00f,
                -932.00f, -4349.00f,
                -828.00f, -4414.00f,
                -661.00f, -4541.00f,
                -521.00f, -4582.00f
        };
        static float const middleToSouthLimit[] = {
                    -2402.01f,      4255.70f,
                -2475.933105f,  3199.568359f, // Desolace
                -2344.124023f,  1756.164307f,
                -2826.438965f,   403.824738f, // Mulgore
                -3472.819580f,   182.522476f, // Feralas
                -4365.006836f, -1602.575439f, // the Barrens
                -4515.219727f, -1681.356079f,
                -4543.093750f, -1882.869385f, // Thousand Needles
                    -4824.16f,     -2310.11f,
                -5102.913574f, -2647.062744f,
                -5248.286621f, -3034.536377f,
                -5246.920898f, -3339.139893f,
                -5459.449707f, -4920.155273f, // Tanaris
                    -5437.00f,     -5863.00f
        };

        static float const orgrimmarSouthLimit[] = {
                2132.5076f, -3912.2478f,
                1944.4298f, -3855.2583f,
                1735.6906f, -3834.2417f,
                1654.3671f, -3380.9902f,
                1593.9861f, -3975.5413f,
                1439.2548f, -4249.6923f,
                1436.3106f, -4007.8950f,
                1393.3199f, -4196.0625f,
                1445.2428f, -4373.9052f,
                1407.2349f, -4429.4145f,
                1464.7142f, -4545.2875f,
                1584.1331f, -4596.8764f,
                1716.8065f, -4601.1323f,
                1875.8312f, -4788.7187f,
                1979.7647f, -4883.4585f,
                2219.1562f, -4854.3330f
        };

        static float const feralasThousandNeedlesSouthLimit[] = {
                -6495.4995f, -4711.981f,
                -6674.9995f, -4515.0019f,
                -6769.5717f, -4122.4272f,
                -6838.2651f, -3874.2792f,
                -6851.1314f, -3659.1179f,
                -6624.6845f, -3063.3843f,
                -6416.9067f, -2570.1301f,
                -5959.8466f, -2287.2634f,
                -5947.9135f, -1866.5028f,
                -5947.9135f,  -820.4881f,
                -5876.7114f,    -3.5138f,
                -5876.7114f,   917.6407f,
                -6099.3603f,  1153.2884f,
                -6021.8989f,  1638.1809f,
                -6091.6176f,  2335.8892f,
                -6744.9946f,  2393.4855f,
                -6973.8608f,  3077.0281f,
                -7068.7241f,  4376.2304f,
                -7142.1211f,  4808.4331f
        };

        if (IsNorthTo(x, y, teldrassilSouthLimit, sizeof(teldrassilSouthLimit) / (2 * sizeof(float))))
            return MAP1_TELDRASSIL;
        if (IsNorthTo(x, y, northMiddleLimit, sizeof(northMiddleLimit) / (2 * sizeof(float))))
            return MAP1_NORTH;
        if (IsNorthTo(x, y, orgrimmarSouthLimit, sizeof(orgrimmarSouthLimit) / (2 * sizeof(float))))
            return MAP1_ORGRIMMAR;
        if (IsNorthTo(x, y, durotarSouthLimit, sizeof(durotarSouthLimit) / (2 * sizeof(float))))
            return MAP1_DUROTAR;
        if (IsNorthTo(x, y, valleyoftrialsSouthLimit, sizeof(valleyoftrialsSouthLimit) / (2 * sizeof(float))))
            return MAP1_VALLEY;
        if (IsNorthTo(x, y, middleToSouthLimit, sizeof(middleToSouthLimit) / (2 * sizeof(float))))
            return MAP1_UPPER_MIDDLE;
        if (IsNorthTo(x, y, feralasThousandNeedlesSouthLimit, sizeof(feralasThousandNeedlesSouthLimit) / (2 * sizeof(float))))
            return MAP1_LOWER_MIDDLE;
        if (y > 15000.0f)
            return MAP1_GMISLAND;
        else
            return MAP1_SOUTH;
    }
    }
    return MAP_NO_AREA;
}
#endif

void MapManager::DoForAllMapsWithMapId(uint32 mapId, std::function<void(Map*)> worker)
{
    MapMapType::const_iterator start = i_maps.lower_bound(MapID(mapId, 0));
    MapMapType::const_iterator end = i_maps.lower_bound(MapID(mapId + 1, 0));
    for (MapMapType::const_iterator itr = start; itr != end; ++itr)
        worker(itr->second);
}

void MapManager::DoForAllMaps(const std::function<void(Map*)>& worker)
{
    for (MapMapType::const_iterator itr = i_maps.begin(); itr != i_maps.end(); ++itr)
        worker(itr->second);
}
