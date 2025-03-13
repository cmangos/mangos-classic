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

#include "Maps/Map.h"
#include "Maps/MapManager.h"
#include "Entities/Player.h"
#include "Grids/GridNotifiers.h"
#include "Log/Log.h"
#include "Grids/ObjectGridLoader.h"
#include "Grids/CellImpl.h"
#include "GridDefines.h"
#include "Grids/GridNotifiersImpl.h"
#include "Maps/InstanceData.h"
#include "Maps/MapPersistentStateMgr.h"
#include "Entities/Transports.h"
#include "Globals/ObjectAccessor.h"
#include "Globals/ObjectMgr.h"
#include "World/World.h"
#include "Groups/Group.h"
#include "MapRefManager.h"
#include "Server/DBCEnums.h"
#include "VMapFactory.h"
#include "MotionGenerators/MoveMap.h"
#include "Chat/Chat.h"
#include "Weather/Weather.h"
#include "AI/ScriptDevAI/ScriptDevAIMgr.h"
#include "BattleGround/BattleGroundMgr.h"

#ifdef BUILD_METRICS
 #include "Metric/Metric.h"
#endif

#include <time.h>

#ifdef ENABLE_PLAYERBOTS
#include "playerbot/playerbot.h"
#endif

Map::~Map()
{
    UnloadAll(true);

    if (m_persistentState)
        m_persistentState->SetUsedByMapState(nullptr);         // field pointer can be deleted after this

    delete i_data;
    i_data = nullptr;

    // unload instance specific navigation data
    MMAP::MMapFactory::createOrGetMMapManager()->unloadMapInstance(m_TerrainData->GetMapId(), GetInstanceId());

    // release reference count
    if (m_TerrainData->Release())
        sTerrainMgr.UnloadTerrain(m_TerrainData->GetMapId());

    delete m_weatherSystem;
    m_weatherSystem = nullptr;

    for (auto m_Transport : m_transports)
        delete m_Transport;
}

uint32 Map::GetCurrentMSTime() const
{
    return World::GetCurrentMSTime();
}

TimePoint Map::GetCurrentClockTime() const
{
    return World::GetCurrentClockTime();
}

uint32 Map::GetCurrentDiff() const
{
    return World::GetCurrentDiff();
}

time_t Map::GetCurrentTime_t() const
{
    return m_curTime;
}

tm Map::GetCurrentTime_tm() const
{
    return m_curTimeTm;
}

GenericTransport* Map::GetTransport(ObjectGuid guid)
{
    // elevators also cause the client to send MOVEFLAG_ONTRANSPORT - just unmount if the guid can be found in the transport list
    for (auto& transport : m_transports)
    {
        if (transport->GetObjectGuid() == guid)
        {
            return transport;
        }
    }
    if (guid.GetEntry())
        if (GameObject* go = GetGameObject(guid))
            if (go->IsTransport())
                return static_cast<GenericTransport*>(go);
    return nullptr;
}

void Map::AddTransport(Transport* transport)
{
    m_transports.insert(transport);
}

void Map::RemoveTransport(Transport* transport)
{
    if (m_transportsIterator != m_transports.end())
    {
        auto itr = m_transports.find(transport);
        if (itr == m_transportsIterator)
            ++m_transportsIterator;
        m_transports.erase(transport);
    }
    else
        m_transports.erase(transport);
}

bool Map::CanSpawn(TypeID typeId, uint32 dbGuid)
{
    if (typeId == TYPEID_UNIT)
        return GetCreatureLinkingHolder()->CanSpawn(dbGuid, this, nullptr, 0.f, 0.f);
    else if (typeId == TYPEID_GAMEOBJECT)
        return true;
    return false;
}

void Map::LoadMapAndVMap(int gx, int gy)
{
    if (m_bLoadedGrids[gx][gy])
        return;

    if (m_TerrainData->Load(gx, gy))
        m_bLoadedGrids[gx][gy] = true;
}

Map::Map(uint32 id, time_t expiry, uint32 InstanceId)
    : i_mapEntry(sMapStore.LookupEntry(id)),
      i_id(id), i_InstanceId(InstanceId), m_unloadTimer(0),
      m_VisibleDistance(DEFAULT_VISIBILITY_DISTANCE), m_persistentState(nullptr),
      m_activeNonPlayersIter(m_activeNonPlayers.end()), m_onEventNotifiedIter(m_onEventNotifiedObjects.end()),
      i_gridExpiry(expiry), m_TerrainData(sTerrainMgr.LoadTerrain(id)),
      i_data(nullptr), i_script_id(0), m_transportsIterator(m_transports.begin()), m_spawnManager(*this),
#ifdef ENABLE_PLAYERBOTS
      m_activeZonesTimer(0), hasRealPlayers(false),
#endif
      m_variableManager(this)
{
    m_weatherSystem = new WeatherSystem(this);
}

void Map::Initialize(bool loadInstanceData /*= true*/)
{
    m_CreatureGuids.Set(sObjectMgr.GetFirstTemporaryCreatureLowGuid());
    m_GameObjectGuids.Set(sObjectMgr.GetFirstTemporaryGameObjectLowGuid());

    for (unsigned int j = 0; j < MAX_NUMBER_OF_GRIDS; ++j)
    {
        for (unsigned int idx = 0; idx < MAX_NUMBER_OF_GRIDS; ++idx)
        {
            // z code
            m_bLoadedGrids[idx][j] = false;
            setNGrid(nullptr, idx, j);
        }
    }

    // lets initialize visibility distance for map
    InitVisibilityDistance();

    // add reference for TerrainData object
    m_TerrainData->AddRef();
    CreateInstanceData(loadInstanceData);

    m_persistentState = sMapPersistentStateMgr.AddPersistentState(i_mapEntry, GetInstanceId(), 0, IsDungeon(), false);
    m_persistentState->SetUsedByMapState(this);
    m_persistentState->InitPools();

    m_graveyardManager.Init(this);

    m_variableManager.Initialize(m_persistentState->GetCompletedEncountersMask());

    m_spawnManager.Initialize();

    auto mmap = MMAP::MMapFactory::createOrGetMMapManager();
    if (mmap->IsEnabled())
    {
        MMAP::MMapFactory::createOrGetMMapManager()->loadMapInstance(sWorld.GetDataPath(), GetId(), GetInstanceId());
        if (sWorld.getConfig(CONFIG_BOOL_PRELOAD_MMAP_TILES))
            MMAP::MMapFactory::createOrGetMMapManager()->loadAllMapTiles(sWorld.GetDataPath(), GetId());
    }

    sObjectMgr.LoadActiveEntities(this);

    LoadTransports();
}

void Map::InitVisibilityDistance()
{
    // init visibility for continents
    m_VisibleDistance = World::GetMaxVisibleDistanceOnContinents();
}

// Template specialization of utility methods
template<class T>
void Map::AddToGrid(T* obj, NGridType* grid, Cell const& cell)
{
    (*grid)(cell.CellX(), cell.CellY()).AddGridObject<T>(obj);
}

template<>
void Map::AddToGrid(GameObject* obj, NGridType* grid, Cell const& cell)
{
    (*grid)(cell.CellX(), cell.CellY()).AddGridObject<GameObject>(obj);
    obj->SetCurrentCell(cell);
}

template<>
void Map::AddToGrid(Player* obj, NGridType* grid, Cell const& cell)
{
    (*grid)(cell.CellX(), cell.CellY()).AddWorldObject(obj);
}

template<>
void Map::AddToGrid(Corpse* obj, NGridType* grid, Cell const& cell)
{
    // add to world object registry in grid
    if (obj->GetType() != CORPSE_BONES)
    {
        (*grid)(cell.CellX(), cell.CellY()).AddWorldObject(obj);
    }
    // add to grid object store
    else
    {
        (*grid)(cell.CellX(), cell.CellY()).AddGridObject(obj);
    }
}

template<>
void Map::AddToGrid(Creature* obj, NGridType* grid, Cell const& cell)
{
    // add to world object registry in grid
    if (obj->IsPet())
    {
        (*grid)(cell.CellX(), cell.CellY()).AddWorldObject<Creature>(obj);
        obj->SetCurrentCell(cell);
    }
    // add to grid object store
    else
    {
        (*grid)(cell.CellX(), cell.CellY()).AddGridObject<Creature>(obj);
        obj->SetCurrentCell(cell);
    }
}

template<class T>
void Map::RemoveFromGrid(T* obj, NGridType* grid, Cell const& cell)
{
    (*grid)(cell.CellX(), cell.CellY()).RemoveGridObject<T>(obj);
}

template<>
void Map::RemoveFromGrid(Player* obj, NGridType* grid, Cell const& cell)
{
    (*grid)(cell.CellX(), cell.CellY()).RemoveWorldObject(obj);
}

template<>
void Map::RemoveFromGrid(Corpse* obj, NGridType* grid, Cell const& cell)
{
    // remove from world object registry in grid
    if (obj->GetType() != CORPSE_BONES)
    {
        (*grid)(cell.CellX(), cell.CellY()).RemoveWorldObject(obj);
    }
    // remove from grid object store
    else
    {
        (*grid)(cell.CellX(), cell.CellY()).RemoveGridObject(obj);
    }
}

template<>
void Map::RemoveFromGrid(Creature* obj, NGridType* grid, Cell const& cell)
{
    // remove from world object registry in grid
    if (obj->IsPet())
    {
        (*grid)(cell.CellX(), cell.CellY()).RemoveWorldObject<Creature>(obj);
    }
    // remove from grid object store
    else
    {
        (*grid)(cell.CellX(), cell.CellY()).RemoveGridObject<Creature>(obj);
    }
}

void Map::DeleteFromWorld(Player* pl)
{
    sObjectAccessor.RemoveObject(pl);
    delete pl;
}

void Map::EnsureGridCreated(const GridPair& p)
{
    if (!getNGrid(p.x_coord, p.y_coord))
    {
        setNGrid(new NGridType(p.x_coord * MAX_NUMBER_OF_GRIDS + p.y_coord, p.x_coord, p.y_coord, i_gridExpiry, sWorld.getConfig(CONFIG_BOOL_GRID_UNLOAD)),
                 p.x_coord, p.y_coord);

        // build a linkage between this map and NGridType
        buildNGridLinkage(getNGrid(p.x_coord, p.y_coord));

        getNGrid(p.x_coord, p.y_coord)->SetGridState(GRID_STATE_IDLE);

        // z coord
        int gx = (MAX_NUMBER_OF_GRIDS - 1) - p.x_coord;
        int gy = (MAX_NUMBER_OF_GRIDS - 1) - p.y_coord;

        if (!m_bLoadedGrids[gx][gy])
            LoadMapAndVMap(gx, gy);
    }
}

void Map::EnsureGridLoadedAtEnter(const Cell& cell, Player* player)
{
    NGridType* grid;

    if (EnsureGridLoaded(cell))
    {
        grid = getNGrid(cell.GridX(), cell.GridY());

        if (player)
        {
            DEBUG_FILTER_LOG(LOG_FILTER_PLAYER_MOVES, "Player %s enter cell[%u,%u] triggers of loading grid[%u,%u] on map %u", player->GetName(), cell.CellX(), cell.CellY(), cell.GridX(), cell.GridY(), i_id);
        }
        else
        {
            DEBUG_FILTER_LOG(LOG_FILTER_PLAYER_MOVES, "Active object nearby triggers of loading grid [%u,%u] on map %u", cell.GridX(), cell.GridY(), i_id);
        }

        ResetGridExpiry(*getNGrid(cell.GridX(), cell.GridY()), 0.1f);
        grid->SetGridState(GRID_STATE_ACTIVE);
    }
    else
        grid = getNGrid(cell.GridX(), cell.GridY());

    if (player)
        AddToGrid(player, grid, cell);
}

bool Map::EnsureGridLoaded(const Cell& cell)
{
    EnsureGridCreated(GridPair(cell.GridX(), cell.GridY()));
    NGridType* grid = getNGrid(cell.GridX(), cell.GridY());

    MANGOS_ASSERT(grid != nullptr);
    if (!isGridObjectDataLoaded(cell.GridX(), cell.GridY()))
    {
        // it's important to set it loaded before loading!
        // otherwise there is a possibility of infinity chain (grid loading will be called many times for the same grid)
        // possible scenario:
        // active object A(loaded with loader.LoadN call and added to the  map)
        // summons some active object B, while B added to map grid loading called again and so on..
        setGridObjectDataLoaded(true, cell.GridX(), cell.GridY());
        ObjectGridLoader loader(*grid, this, cell);
        loader.LoadN();

        // Add resurrectable corpses to world object list in grid
        sObjectAccessor.AddCorpsesToGrid(GridPair(cell.GridX(), cell.GridY()), (*grid)(cell.CellX(), cell.CellY()), this);
        return true;
    }

    return false;
}

uint32 Map::GetLoadedGridsCount()
{
    uint32 count = 0;
    for (uint32 i = 0; i < MAX_NUMBER_OF_GRIDS; ++i)
        for (uint32 k = 0; k < MAX_NUMBER_OF_GRIDS; ++k)
            if (i_grids[i][k] && i_grids[i][k]->GetGridState() == GRID_STATE_ACTIVE)
                ++count;
    return count;
}

void Map::ForceLoadGrid(float x, float y)
{
    if (!IsLoaded(x, y))
    {
        CellPair p = MaNGOS::ComputeCellPair(x, y);
        Cell cell(p);
        EnsureGridLoadedAtEnter(cell);
        getNGrid(cell.GridX(), cell.GridY())->setUnloadExplicitLock(true);
    }
}

void Map::CreatePlayerOnClient(Player* player)
{
    // update player state for other player and visa-versa
    CellPair p = MaNGOS::ComputeCellPair(player->GetPositionX(), player->GetPositionY());
    Cell cell(p);

    SendInitSelf(player);
    SendInitTransports(player);

    NGridType* grid = getNGrid(cell.GridX(), cell.GridY());
    player->GetViewPoint().Event_AddedToWorld(&(*grid)(cell.CellX(), cell.CellY()));
    UpdateObjectVisibility(player, cell, p);
}

bool Map::Add(Player* player)
{
    player->GetMapRef().link(this, player);
    player->SetMap(this);

    // update player state for other player and visa-versa
    CellPair p = MaNGOS::ComputeCellPair(player->GetPositionX(), player->GetPositionY());
    Cell cell(p);
    EnsureGridLoadedAtEnter(cell, player);
    player->AddToWorld();

    // TODO: Aggregate all of these into one packet

    SendInitSelf(player);
    SendInitTransports(player);

    NGridType* grid = getNGrid(cell.GridX(), cell.GridY());
    player->GetViewPoint().Event_AddedToWorld(&(*grid)(cell.CellX(), cell.CellY()));
    UpdateObjectVisibility(player, cell, p);

    if (i_data)
        i_data->OnPlayerEnter(player);

    return true;
}

template<class T>
void Map::Add(T* obj)
{
    MANGOS_ASSERT(obj);

    CellPair p = MaNGOS::ComputeCellPair(obj->GetPositionX(), obj->GetPositionY());
    if (p.x_coord >= TOTAL_NUMBER_OF_CELLS_PER_MAP || p.y_coord >= TOTAL_NUMBER_OF_CELLS_PER_MAP)
    {
        sLog.outError("Map::Add: Object (GUID: %u TypeId: %u) have invalid coordinates X:%f Y:%f grid cell [%u:%u]", obj->GetGUIDLow(), obj->GetTypeId(), obj->GetPositionX(), obj->GetPositionY(), p.x_coord, p.y_coord);
        return;
    }

    obj->SetMap(this);

    Cell cell(p);
    if (obj->isActiveObject())
        EnsureGridLoadedAtEnter(cell);
    else
        EnsureGridCreated(GridPair(cell.GridX(), cell.GridY()));

    NGridType* grid = getNGrid(cell.GridX(), cell.GridY());
    MANGOS_ASSERT(grid != nullptr);

    AddToGrid(obj, grid, cell);
    obj->AddToWorld();

    if (obj->isActiveObject())
        AddToActive(obj);

    DEBUG_FILTER_LOG(LOG_FILTER_CREATURE_MOVES, "%s enters grid[%u,%u]", obj->GetGuidStr().c_str(), cell.GridX(), cell.GridY());

    obj->GetViewPoint().Event_AddedToWorld(&(*grid)(cell.CellX(), cell.CellY()));
    obj->SetItsNewObject(true);
    UpdateObjectVisibility(obj, cell, p);
    obj->SetItsNewObject(false);
}

void Map::MessageBroadcast(Player const* player, WorldPacket const& msg, bool to_self)
{
    CellPair p = MaNGOS::ComputeCellPair(player->GetPositionX(), player->GetPositionY());

    if (p.x_coord >= TOTAL_NUMBER_OF_CELLS_PER_MAP || p.y_coord >= TOTAL_NUMBER_OF_CELLS_PER_MAP)
    {
        sLog.outError("Map::MessageBroadcast: Player (GUID: %u) have invalid coordinates X:%f Y:%f grid cell [%u:%u]", player->GetGUIDLow(), player->GetPositionX(), player->GetPositionY(), p.x_coord, p.y_coord);
        return;
    }

    Cell cell(p);
    cell.SetNoCreate();

    if (!loaded(GridPair(cell.data.Part.grid_x, cell.data.Part.grid_y)))
        return;

    MaNGOS::MessageDeliverer post_man(*player, msg, to_self);
    TypeContainerVisitor<MaNGOS::MessageDeliverer, WorldTypeMapContainer > message(post_man);
    cell.Visit(p, message, *this, *player, player->GetVisibilityData().GetVisibilityDistance());
}

void Map::MessageBroadcast(WorldObject const* obj, WorldPacket const& msg)
{
    CellPair p = MaNGOS::ComputeCellPair(obj->GetPositionX(), obj->GetPositionY());

    if (p.x_coord >= TOTAL_NUMBER_OF_CELLS_PER_MAP || p.y_coord >= TOTAL_NUMBER_OF_CELLS_PER_MAP)
    {
        sLog.outError("Map::MessageBroadcast: Object (GUID: %u TypeId: %u) have invalid coordinates X:%f Y:%f grid cell [%u:%u]", obj->GetGUIDLow(), obj->GetTypeId(), obj->GetPositionX(), obj->GetPositionY(), p.x_coord, p.y_coord);
        return;
    }

    Cell cell(p);
    cell.SetNoCreate();

    if (!loaded(GridPair(cell.data.Part.grid_x, cell.data.Part.grid_y)))
        return;

    // TODO: currently on continents when Visibility.Distance.InFlight > Visibility.Distance.Continents
    // we have alot of blinking mobs because monster move packet send is broken...
    MaNGOS::ObjectMessageDeliverer post_man(msg);
    TypeContainerVisitor<MaNGOS::ObjectMessageDeliverer, WorldTypeMapContainer > message(post_man);
    cell.Visit(p, message, *this, *obj, obj->GetVisibilityData().GetVisibilityDistance());
}

void Map::MessageDistBroadcast(Player const* player, WorldPacket const& msg, float dist, bool to_self, bool own_team_only)
{
    CellPair p = MaNGOS::ComputeCellPair(player->GetPositionX(), player->GetPositionY());

    if (p.x_coord >= TOTAL_NUMBER_OF_CELLS_PER_MAP || p.y_coord >= TOTAL_NUMBER_OF_CELLS_PER_MAP)
    {
        sLog.outError("Map::MessageBroadcast: Player (GUID: %u) have invalid coordinates X:%f Y:%f grid cell [%u:%u]", player->GetGUIDLow(), player->GetPositionX(), player->GetPositionY(), p.x_coord, p.y_coord);
        return;
    }

    Cell cell(p);
    cell.SetNoCreate();

    if (!loaded(GridPair(cell.data.Part.grid_x, cell.data.Part.grid_y)))
        return;

    MaNGOS::MessageDistDeliverer post_man(*player, msg, dist, to_self, own_team_only);
    TypeContainerVisitor<MaNGOS::MessageDistDeliverer, WorldTypeMapContainer > message(post_man);
    cell.Visit(p, message, *this, *player, dist);
}

void Map::MessageDistBroadcast(WorldObject const* obj, WorldPacket const& msg, float dist)
{
    CellPair p = MaNGOS::ComputeCellPair(obj->GetPositionX(), obj->GetPositionY());

    if (p.x_coord >= TOTAL_NUMBER_OF_CELLS_PER_MAP || p.y_coord >= TOTAL_NUMBER_OF_CELLS_PER_MAP)
    {
        sLog.outError("Map::MessageBroadcast: Object (GUID: %u TypeId: %u) have invalid coordinates X:%f Y:%f grid cell [%u:%u]", obj->GetGUIDLow(), obj->GetTypeId(), obj->GetPositionX(), obj->GetPositionY(), p.x_coord, p.y_coord);
        return;
    }

    Cell cell(p);
    cell.SetNoCreate();

    if (!loaded(GridPair(cell.data.Part.grid_x, cell.data.Part.grid_y)))
        return;

    MaNGOS::ObjectMessageDistDeliverer post_man(*obj, msg, dist);
    TypeContainerVisitor<MaNGOS::ObjectMessageDistDeliverer, WorldTypeMapContainer > message(post_man);
    cell.Visit(p, message, *this, *obj, dist);
}

void Map::MessageMapBroadcast(WorldObject const* /*obj*/, WorldPacket const& msg)
{
    Map::PlayerList const& pList = GetPlayers();
    for (const auto& itr : pList)
        itr.getSource()->SendDirectMessage(msg);
}

void Map::MessageMapBroadcastZone(WorldObject const* /*obj*/, WorldPacket const& msg, uint32 zoneId)
{
    Map::PlayerList const& pList = GetPlayers();
    for (const auto& itr : pList)
        if (itr.getSource()->GetZoneId() == zoneId)
            itr.getSource()->SendDirectMessage(msg);
}

void Map::MessageMapBroadcastArea(WorldObject const* /*obj*/, WorldPacket const& msg, uint32 areaId)
{
    Map::PlayerList const& pList = GetPlayers();
    for (const auto& itr : pList)
        if (itr.getSource()->GetAreaId() == areaId)
            itr.getSource()->SendDirectMessage(msg);
}

void Map::ExecuteDistWorker(WorldObject const* obj, float dist, std::function<void(Player*)> const& worker)
{
    CellPair p = MaNGOS::ComputeCellPair(obj->GetPositionX(), obj->GetPositionY());

    if (p.x_coord >= TOTAL_NUMBER_OF_CELLS_PER_MAP || p.y_coord >= TOTAL_NUMBER_OF_CELLS_PER_MAP)
    {
        sLog.outError("Map::ExecuteDistWorker: Object (GUID: %u TypeId: %u) have invalid coordinates X:%f Y:%f grid cell [%u:%u]", obj->GetGUIDLow(), obj->GetTypeId(), obj->GetPositionX(), obj->GetPositionY(), p.x_coord, p.y_coord);
        return;
    }

    Cell cell(p);
    cell.SetNoCreate();

    if (!loaded(GridPair(cell.data.Part.grid_x, cell.data.Part.grid_y)))
        return;

    MaNGOS::CameraDistLambdaWorker searcher(obj, dist, worker);
    TypeContainerVisitor<MaNGOS::CameraDistLambdaWorker, WorldTypeMapContainer > message(searcher);
    cell.Visit(p, message, *this, *obj, dist);
}

void Map::ExecuteMapWorker(std::function<void(Player*)> const& worker)
{
    Map::PlayerList const& pList = GetPlayers();
    for (const auto& itr : pList)
        worker(itr.getSource());
}

void Map::ExecuteMapWorkerZone(uint32 zoneId, std::function<void(Player*)> const& worker)
{
    Map::PlayerList const& pList = GetPlayers();
    for (const auto& itr : pList)
        if (itr.getSource()->GetZoneId() == zoneId)
            worker(itr.getSource());
}

void Map::ExecuteMapWorkerArea(uint32 areaId, std::function<void(Player*)> const& worker)
{
    Map::PlayerList const& pList = GetPlayers();
    for (const auto& itr : pList)
        if (itr.getSource()->GetAreaId() == areaId)
            worker(itr.getSource());
}

bool Map::loaded(const GridPair& p) const
{
    return (getNGrid(p.x_coord, p.y_coord) && isGridObjectDataLoaded(p.x_coord, p.y_coord));
}

#define MAP_METRICS

void Map::VisitNearbyCellsOf(WorldObject* obj, TypeContainerVisitor<MaNGOS::ObjectUpdater, GridTypeMapContainer> &gridVisitor, TypeContainerVisitor<MaNGOS::ObjectUpdater, WorldTypeMapContainer> &worldVisitor)
{
    // lets update mobs/objects in ALL visible cells around player!
    CellArea area = Cell::CalculateCellArea(obj->GetPositionX(), obj->GetPositionY(), obj->IsInWorld() ? obj->GetVisibilityData().GetVisibilityDistance() : GetVisibilityDistance());

    for (uint32 x = area.low_bound.x_coord; x <= area.high_bound.x_coord; ++x)
    {
        for (uint32 y = area.low_bound.y_coord; y <= area.high_bound.y_coord; ++y)
        {
            // marked cells are those that have been visited
            // don't visit the same cell twice
            uint32 cell_id = (y * TOTAL_NUMBER_OF_CELLS_PER_MAP) + x;
            if (!isCellMarked(cell_id))
            {
                markCell(cell_id);
                CellPair pair(x, y);
                Cell cell(pair);
                cell.SetNoCreate();
                Visit(cell, gridVisitor);
                Visit(cell, worldVisitor);
            }
        }
    }
}

void Map::Update(const uint32& t_diff)
{

#ifdef BUILD_METRICS
    metric::duration<std::chrono::milliseconds> meas("map.update", {
        { "map_id", std::to_string(i_id) },
        { "instance_id", std::to_string(i_InstanceId) }
});
#endif

    m_curTime = time(nullptr);

#ifdef _MSC_VER
    localtime_s(&m_curTimeTm, &m_curTime);
#else
    localtime_r(&m_curTime, &m_curTimeTm);
#endif

    uint64 count = 0;

    m_dyn_tree.update(t_diff);

    GetMessager().Execute(this);
    m_spawnManager.Update();

    /// update active cells around players and active objects
    resetMarkedCells();

    WorldObjectUnSet objToUpdate;
    MaNGOS::ObjectUpdater obj_updater(objToUpdate, t_diff);
    TypeContainerVisitor<MaNGOS::ObjectUpdater, GridTypeMapContainer  > grid_object_update(obj_updater);    // For creature
    TypeContainerVisitor<MaNGOS::ObjectUpdater, WorldTypeMapContainer > world_object_update(obj_updater);   // For pets

    for (m_transportsIterator = m_transports.begin(); m_transportsIterator != m_transports.end();)
    {
        Transport* transport = *m_transportsIterator;
        ++m_transportsIterator;
        transport->Update(t_diff);
    }

    // the player iterator is stored in the map object
    // to make sure calls to Map::Remove don't invalidate it
    {
#ifdef BUILD_METRICS
        uint32 updatedSessions = 0;
        metric::duration<std::chrono::milliseconds> sessions_meas("map.update.session", {
            { "map_id", std::to_string(i_id) },
            { "instance_id", std::to_string(i_InstanceId) },
            });
#endif

        for (m_mapRefIter = m_mapRefManager.begin(); m_mapRefIter != m_mapRefManager.end(); ++m_mapRefIter)
        {
            Player* player = m_mapRefIter->getSource();
            if (!player || !player->IsInWorld())
                continue;

            // Update session first
            WorldSession* pSession = player->GetSession();
            pSession->UpdateMap(t_diff);
#ifdef BUILD_METRICS
            ++updatedSessions;
#endif
        }
#ifdef BUILD_METRICS
        sessions_meas.add_field("count", std::to_string(static_cast<int32>(updatedSessions)));
#endif
    }

#ifdef ENABLE_PLAYERBOTS
    // Calculate the active zones every 10 seconds (An active zone is a zone where one or more real players are)
    constexpr uint32 maxActiveZonesTimer = 10000U;
    if (m_activeZonesTimer < maxActiveZonesTimer)
    {
        m_activeZonesTimer += t_diff;
    }
    else
    {
        m_activeZonesTimer = 0U;
        m_activeZones.clear();

        // Recalculate active zones
        if (IsContinent() && HasRealPlayers())
        {
            for (m_mapRefIter = m_mapRefManager.begin(); m_mapRefIter != m_mapRefManager.end(); ++m_mapRefIter)
            {
                Player* plr = m_mapRefIter->getSource();
                if (plr && plr->IsInWorld())
                {
                    // Only consider real players
                    if (plr->GetPlayerbotAI() && !plr->GetPlayerbotAI()->IsRealPlayer())
                        continue;

                    // Ignore afk players
                    if (plr->isAFK())
                        continue;

                    // Ignore gm players
                    if (!plr->isGMVisible())
                        continue;

                    // Register an active zone when a real player is on the zone
                    if (find(m_activeZones.begin(), m_activeZones.end(), plr->GetZoneId()) == m_activeZones.end())
                    {
                        m_activeZones.push_back(plr->GetZoneId());
                    }
                }
            }
        }
    }

    // Reset the has real players flag and check for it again
    const bool hadRealPlayers = hasRealPlayers;
    hasRealPlayers = false;

    uint32 activePlayers = 0;
    uint32 avgDiff = sWorld.GetAverageDiff();

    // Calculate the chance that the bots in this map should update based on server load and real players online
    // (default is a 10% on a avg diff of 100)
    float botUpdateChance = avgDiff * 0.1f;
    if (!hadRealPlayers)
    {
        // If no real players are on the map then lower the chances of updating by 300%
        botUpdateChance *= 3.0f;
    }

    bool shouldUpdateBots = urand(0, (uint32)(botUpdateChance * 100)) < 100;
#endif

    /// update players at tick
    for (m_mapRefIter = m_mapRefManager.begin(); m_mapRefIter != m_mapRefManager.end(); ++m_mapRefIter)
    {
        Player* plr = m_mapRefIter->getSource();
        if (plr && plr->IsInWorld())
        {
#ifdef ENABLE_PLAYERBOTS
            // Determine if the individual bot should update
            bool shouldUpdateBot = shouldUpdateBots;

            // Real players should update always (it will update alt bots)
            if (!plr->GetPlayerbotAI() || plr->GetPlayerbotAI()->IsRealPlayer())
            {
                shouldUpdateBot = true;
                hasRealPlayers = true;
            }
            else
            {
                // If there are real players in the map, check if the bot is on a zone with players
                if (hadRealPlayers)
                {
                    // Check if the bot is in an active zone (or instance)
                    shouldUpdateBot = IsContinent() ? HasActiveZone(plr->GetZoneId()) : true;
                }

                // Check for edge case reasons to force update the bot
                if (!shouldUpdateBot)
                {
                    // Force bots to be active if:
                    // - The bot is playing with a real player
                    // - The bot is in a battleground
                    // - The bot is in combat
                    if ((plr->GetPlayerbotAI() && plr->GetPlayerbotAI()->HasRealPlayerMaster()) ||
                        plr->InBattleGroundQueue() || plr->InBattleGround() ||
                        plr->IsInCombat())
                    {
                        shouldUpdateBot = true;
                    }
                }
            }

            // Save the active characters for later logs
            if (shouldUpdateBot)
            {
                activePlayers++;
            }
#endif

            plr->Update(t_diff);

#ifdef ENABLE_PLAYERBOTS
            if (sPlayerbotAIConfig.disableBotOptimizations)
            {
                plr->UpdateAI(t_diff, false);
            }
            else
            {
                plr->UpdateAI(t_diff, !shouldUpdateBot);
            }
#endif
        }
    }

#ifdef ENABLE_PLAYERBOTS
    // Log the active zones and characters
    if (IsContinent() && HasRealPlayers() && HasActiveZones() && m_activeZonesTimer == 0U)
    {
        sLog.outBasic("Map %u: Active Zones - %u", GetId(), m_activeZones.size());
        sLog.outBasic("Map %u: Active Zone Players - %u of %u", GetId(), activePlayers, m_mapRefManager.getSize());
    }
#endif

    for (m_mapRefIter = m_mapRefManager.begin(); m_mapRefIter != m_mapRefManager.end(); ++m_mapRefIter)
    {
        Player* player = m_mapRefIter->getSource();
        if (!player || !player->IsInWorld() || !player->IsPositionValid())
            continue;

#ifdef ENABLE_PLAYERBOTS
        // For non-players only load the grid
        if (!sPlayerbotAIConfig.disableBotOptimizations && !player->isRealPlayer())
        {
            CellPair center = MaNGOS::ComputeCellPair(player->GetPositionX(), player->GetPositionY()).normalize();
            uint32 cell_id = (center.y_coord * TOTAL_NUMBER_OF_CELLS_PER_MAP) + center.x_coord;

            if (!isCellMarked(cell_id))
            {
                Cell cell(center);
                const uint32 x = cell.GridX();
                const uint32 y = cell.GridY();
                if (!cell.NoCreate() || loaded(GridPair(x, y)))
                {
                    EnsureGridLoaded(player->GetCurrentCell());
                }
            }

            continue;
        }
#endif

        VisitNearbyCellsOf(player, grid_object_update, world_object_update);

        // If player is using far sight, visit that object too
        if (WorldObject* viewPoint = GetWorldObject(player->GetFarSightGuid()))
            VisitNearbyCellsOf(viewPoint, grid_object_update, world_object_update);
    }

#ifdef ENABLE_PLAYERBOTS
    // Calculate the chance that the objects (non players) should update based on server load and real players online
    // (default is a 10% on a avg diff of 100)
    float objectUpdateChance = avgDiff * 0.1f;
    if (!HasRealPlayers())
    {
        // If no real players are on the map then lower the chances of updating by 300%
        objectUpdateChance *= 3.0f;
    }

    const bool shouldUpdateObjects = urand(0, (uint32)(objectUpdateChance * 100)) < 100;
#endif

    // non-player active objects
    if (!m_activeNonPlayers.empty())
    {
        for (m_activeNonPlayersIter = m_activeNonPlayers.begin(); m_activeNonPlayersIter != m_activeNonPlayers.end();)
        {
            // skip not in world
            WorldObject* obj = *m_activeNonPlayersIter;

            // step before processing, in this case if Map::Remove remove next object we correctly
            // step to next-next, and if we step to end() then newly added objects can wait next update.
            ++m_activeNonPlayersIter;

            if (!obj->IsInWorld() || !obj->IsPositionValid())
                continue;

#ifdef ENABLE_PLAYERBOTS
            // Skip objects on locations away from real players if world is laggy
            if (!sPlayerbotAIConfig.disableBotOptimizations && IsContinent() && avgDiff > 100)
            {
                const bool isInActiveZone = IsContinent() ? HasActiveZone(obj->GetZoneId()) : HasRealPlayers();
                if (!isInActiveZone && !shouldUpdateObjects)
                {
                    continue;
                }
            }
#endif

            objToUpdate.insert(obj);

            // lets update mobs/objects in ALL visible cells around player!
            CellArea area = Cell::CalculateCellArea(obj->GetPositionX(), obj->GetPositionY(), GetVisibilityDistance());

            for (uint32 x = area.low_bound.x_coord; x <= area.high_bound.x_coord; ++x)
            {
                for (uint32 y = area.low_bound.y_coord; y <= area.high_bound.y_coord; ++y)
                {
                    // marked cells are those that have been visited
                    // don't visit the same cell twice
                    uint32 cell_id = (y * TOTAL_NUMBER_OF_CELLS_PER_MAP) + x;
                    if (!isCellMarked(cell_id))
                    {
                        markCell(cell_id);
                        CellPair pair(x, y);
                        Cell cell(pair);
                        cell.SetNoCreate();
                        Visit(cell, grid_object_update);
                        Visit(cell, world_object_update);
                    }
                }
            }
        }
    }

    // update all objects
    for (auto wObj : objToUpdate)
    {
        wObj->Update(t_diff);
        ++count;
    }

#ifdef BUILD_METRICS
    meas.add_field("count", std::to_string(static_cast<int32>(count)));
#endif

    // Send world objects and item update field changes
    SendObjectUpdates();

    // Don't unload grids if it's battleground, since we may have manually added GOs,creatures, those doesn't load from DB at grid re-load !
    // This isn't really bother us, since as soon as we have instanced BG-s, the whole map unloads as the BG gets ended
    if (!IsBattleGround())
    {
        for (GridRefManager<NGridType>::iterator i = GridRefManager<NGridType>::begin(); i != GridRefManager<NGridType>::end();)
        {
            NGridType* grid = i->getSource();
            GridInfo* info = i->getSource()->getGridInfoRef();
            ++i;                                            // The update might delete the map and we need the next map before the iterator gets invalid
            MANGOS_ASSERT(grid->GetGridState() >= 0 && grid->GetGridState() < MAX_GRID_STATE);
            sMapMgr.UpdateGridState(grid->GetGridState(), *this, *grid, *info, grid->getX(), grid->getY(), t_diff);
        }
    }

    ///- Process necessary scripts
    if (!m_scriptSchedule.empty())
        ScriptsProcess();

    if (i_data)
        i_data->Update(t_diff);

    m_weatherSystem->UpdateWeathers(t_diff);
}

void Map::Remove(Player* player, bool remove)
{
    if (i_data)
        i_data->OnPlayerLeave(player);

    if (remove)
        player->CleanupsBeforeDelete();
    else
        player->RemoveFromWorld();

    // this may be called during Map::Update
    // after decrement+unlink, ++m_mapRefIter will continue correctly
    // when the first element of the list is being removed
    // nocheck_prev will return the padding element of the RefManager
    // instead of nullptr in the case of prev
    if (m_mapRefIter == player->GetMapRef())
        m_mapRefIter = m_mapRefIter->nocheck_prev();
    player->GetMapRef().unlink();
    CellPair p = MaNGOS::ComputeCellPair(player->GetPositionX(), player->GetPositionY());
    if (p.x_coord >= TOTAL_NUMBER_OF_CELLS_PER_MAP || p.y_coord >= TOTAL_NUMBER_OF_CELLS_PER_MAP)
    {
        // invalid coordinates
        player->ResetMap();

        if (remove)
            DeleteFromWorld(player);

        return;
    }

    Cell cell(p);

    if (!getNGrid(cell.data.Part.grid_x, cell.data.Part.grid_y))
    {
        sLog.outError("Map::Remove() i_grids was nullptr x:%d, y:%d", cell.data.Part.grid_x, cell.data.Part.grid_y);
        return;
    }

    DEBUG_FILTER_LOG(LOG_FILTER_PLAYER_MOVES, "Remove player %s from grid[%u,%u]", player->GetName(), cell.GridX(), cell.GridY());
    NGridType* grid = getNGrid(cell.GridX(), cell.GridY());
    MANGOS_ASSERT(grid != nullptr);

    RemoveFromGrid(player, grid, cell);

    SendRemoveTransports(player);
    UpdateObjectVisibility(player, cell, p);

    player->ResetMap();

    if (remove)
        DeleteFromWorld(player);
}

template<class T>
void Map::Remove(T* obj, bool remove)
{
    CellPair p = MaNGOS::ComputeCellPair(obj->GetPositionX(), obj->GetPositionY());
    if (p.x_coord >= TOTAL_NUMBER_OF_CELLS_PER_MAP || p.y_coord >= TOTAL_NUMBER_OF_CELLS_PER_MAP)
    {
        sLog.outError("Map::Remove: Object (GUID: %u TypeId:%u) have invalid coordinates X:%f Y:%f grid cell [%u:%u]", obj->GetGUIDLow(), obj->GetTypeId(), obj->GetPositionX(), obj->GetPositionY(), p.x_coord, p.y_coord);
        return;
    }

    Cell cell(p);
    if (!loaded(GridPair(cell.data.Part.grid_x, cell.data.Part.grid_y)))
        return;

    DEBUG_FILTER_LOG(LOG_FILTER_CREATURE_MOVES, "Remove %s from grid[%u,%u]", obj->GetGuidStr().c_str(), cell.data.Part.grid_x, cell.data.Part.grid_y);
    NGridType* grid = getNGrid(cell.GridX(), cell.GridY());
    MANGOS_ASSERT(grid != nullptr);

    if (obj->isActiveObject())
        RemoveFromActive(obj);

    if (remove)
        obj->CleanupsBeforeDelete();
    else
        obj->RemoveFromWorld();

    UpdateObjectVisibility(obj, cell, p);                   // i think will be better to call this function while object still in grid, this changes nothing but logically is better(as for me)
    RemoveFromGrid(obj, grid, cell);

    m_objRemoveList.insert(obj->GetObjectGuid());

    if (remove)
        // if option set then object already saved at this moment
        if (!sWorld.getConfig(CONFIG_BOOL_SAVE_RESPAWN_TIME_IMMEDIATELY))
            obj->SaveRespawnTime(); // requires map not being reset

    obj->ResetMap();

    if (remove) // Note: In case resurrectable corpse and pet its removed from global lists in own destructor
        delete obj;
}

void Map::PlayerRelocation(Player* player, float x, float y, float z, float orientation)
{
    MANGOS_ASSERT(player);

    CellPair old_val = MaNGOS::ComputeCellPair(player->GetPositionX(), player->GetPositionY());
    CellPair new_val = MaNGOS::ComputeCellPair(x, y);

    Cell old_cell(old_val);
    Cell new_cell(new_val);
    bool same_cell = (new_cell == old_cell);

    player->Relocate(x, y, z, orientation);

    if (old_cell.DiffGrid(new_cell) || old_cell.DiffCell(new_cell))
    {
        DEBUG_FILTER_LOG(LOG_FILTER_PLAYER_MOVES, "Player %s relocation grid[%u,%u]cell[%u,%u]->grid[%u,%u]cell[%u,%u]", player->GetName(), old_cell.GridX(), old_cell.GridY(), old_cell.CellX(), old_cell.CellY(), new_cell.GridX(), new_cell.GridY(), new_cell.CellX(), new_cell.CellY());

        NGridType* oldGrid = getNGrid(old_cell.GridX(), old_cell.GridY());
        RemoveFromGrid(player, oldGrid, old_cell);
        if (!old_cell.DiffGrid(new_cell))
            AddToGrid(player, oldGrid, new_cell);
        else
            EnsureGridLoadedAtEnter(new_cell, player);

        NGridType* newGrid = getNGrid(new_cell.GridX(), new_cell.GridY());
        player->GetViewPoint().Event_GridChanged(&(*newGrid)(new_cell.CellX(), new_cell.CellY()));
    }

    player->OnRelocated();

    NGridType* newGrid = getNGrid(new_cell.GridX(), new_cell.GridY());
    if (!same_cell && newGrid->GetGridState() != GRID_STATE_ACTIVE)
    {
        ResetGridExpiry(*newGrid, 0.1f);
        newGrid->SetGridState(GRID_STATE_ACTIVE);
    }
}

void Map::CreatureRelocation(Creature* creature, float x, float y, float z, float ang)
{
    Cell new_cell(MaNGOS::ComputeCellPair(x, y));

    // do move or do move to respawn or remove creature if previous all fail
    if (CreatureCellRelocation(creature, new_cell))
    {
        // update pos
        creature->Relocate(x, y, z, ang);
        creature->OnRelocated();
    }
    // if creature can't be move in new cell/grid (not loaded) move it to repawn cell/grid
    // creature coordinates will be updated and notifiers send
    else if (!CreatureRespawnRelocation(creature))
    {
        // ... or unload (if respawn grid also not loaded)
        DEBUG_FILTER_LOG(LOG_FILTER_CREATURE_MOVES, "Creature (GUID: %u Entry: %u ) can't be move to unloaded respawn grid.", creature->GetGUIDLow(), creature->GetEntry());
    }
}

void Map::GameObjectRelocation(GameObject* go, float x, float y, float z, float orientation, bool respawnRelocationOnFail)
{
    Cell new_cell(MaNGOS::ComputeCellPair(x, y));
    Cell old_cell = go->GetCurrentCell();

    if (!respawnRelocationOnFail && !getNGrid(new_cell.GridX(), new_cell.GridY()))
        return;

    if (old_cell.DiffGrid(new_cell))
    {
        if (!go->isActiveObject() && !loaded(new_cell.gridPair()))
        {
            DEBUG_FILTER_LOG(LOG_FILTER_CREATURE_MOVES, "Creature (GUID: %u Entry: %u) attempt move from grid[%u,%u]cell[%u,%u] to unloaded grid[%u,%u]cell[%u,%u].", go->GetGUIDLow(), go->GetEntry(), old_cell.GridX(), old_cell.GridY(), old_cell.CellX(), old_cell.CellY(), new_cell.GridX(), new_cell.GridY(), new_cell.CellX(), new_cell.CellY());
            return;
        }
        EnsureGridLoadedAtEnter(new_cell);
    }

    // delay creature move for grid/cell to grid/cell moves
    if (old_cell.DiffCell(new_cell) || old_cell.DiffGrid(new_cell))
    {
        NGridType* oldGrid = getNGrid(old_cell.GridX(), old_cell.GridY());
        NGridType* newGrid = getNGrid(new_cell.GridX(), new_cell.GridY());
        RemoveFromGrid(go, oldGrid, old_cell);
        AddToGrid(go, newGrid, new_cell);
        go->GetViewPoint().Event_GridChanged(&(*newGrid)(new_cell.CellX(), new_cell.CellY()));
    }

    go->Relocate(x, y, z, orientation);
    go->UpdateModelPosition();
    go->UpdateObjectVisibility();
}

bool Map::CreatureCellRelocation(Creature* c, const Cell& new_cell)
{
    Cell const& old_cell = c->GetCurrentCell();
    if (old_cell.DiffGrid(new_cell))
    {
        if (!c->isActiveObject() && !loaded(new_cell.gridPair()))
        {
            DEBUG_FILTER_LOG(LOG_FILTER_CREATURE_MOVES, "Creature (GUID: %u Entry: %u) attempt move from grid[%u,%u]cell[%u,%u] to unloaded grid[%u,%u]cell[%u,%u].", c->GetGUIDLow(), c->GetEntry(), old_cell.GridX(), old_cell.GridY(), old_cell.CellX(), old_cell.CellY(), new_cell.GridX(), new_cell.GridY(), new_cell.CellX(), new_cell.CellY());
            return false;
        }
        EnsureGridLoadedAtEnter(new_cell);
    }

    if (old_cell != new_cell)
    {
        DEBUG_FILTER_LOG(LOG_FILTER_CREATURE_MOVES, "Creature (GUID: %u Entry: %u) moved in grid[%u,%u] from cell[%u,%u] to cell[%u,%u].", c->GetGUIDLow(), c->GetEntry(), old_cell.GridX(), old_cell.GridY(), old_cell.CellX(), old_cell.CellY(), new_cell.CellX(), new_cell.CellY());
        NGridType* oldGrid = getNGrid(old_cell.GridX(), old_cell.GridY());
        NGridType* newGrid = getNGrid(new_cell.GridX(), new_cell.GridY());
        RemoveFromGrid(c, oldGrid, old_cell);
        AddToGrid(c, newGrid, new_cell);
        c->GetViewPoint().Event_GridChanged(&(*newGrid)(new_cell.CellX(), new_cell.CellY()));
    }
    return true;
}

bool Map::CreatureRespawnRelocation(Creature* c)
{
    float resp_x, resp_y, resp_z, resp_o;
    c->GetRespawnCoord(resp_x, resp_y, resp_z, &resp_o);

    CellPair resp_val = MaNGOS::ComputeCellPair(resp_x, resp_y);
    Cell resp_cell(resp_val);

    c->CombatStopWithPets();
    c->GetMotionMaster()->Clear();

    DEBUG_FILTER_LOG(LOG_FILTER_CREATURE_MOVES, "Creature (GUID: %u Entry: %u) will moved from grid[%u,%u]cell[%u,%u] to respawn grid[%u,%u]cell[%u,%u].", c->GetGUIDLow(), c->GetEntry(), c->GetCurrentCell().GridX(), c->GetCurrentCell().GridY(), c->GetCurrentCell().CellX(), c->GetCurrentCell().CellY(), resp_cell.GridX(), resp_cell.GridY(), resp_cell.CellX(), resp_cell.CellY());

    // teleport it to respawn point (like normal respawn if player see)
    if (CreatureCellRelocation(c, resp_cell))
    {
        c->Relocate(resp_x, resp_y, resp_z, resp_o);
        c->GetMotionMaster()->Initialize();                 // prevent possible problems with default move generators
        c->OnRelocated();
        return true;
    }
    return false;
}

bool Map::UnloadGrid(const uint32& x, const uint32& y, bool pForce)
{
    NGridType* grid = getNGrid(x, y);
    MANGOS_ASSERT(grid != nullptr);

    {
        if (!pForce && ActiveObjectsNearGrid(x, y))
            return false;

        DEBUG_FILTER_LOG(LOG_FILTER_MAP_LOADING, "Unloading grid[%u,%u] for map %u", x, y, i_id);

        ObjectGridStoper stoper(*grid);
        stoper.StopN();

        ObjectGridUnloader unloader(*grid);

        // Finish remove and delete all creatures with delayed remove before moving to respawn grids
        // Must know real mob position before move
        RemoveAllObjectsInRemoveList();

        // move creatures to respawn grids if this is diff.grid or to remove list
        unloader.MoveToRespawnN();

        // Finish remove and delete all creatures with delayed remove before unload
        RemoveAllObjectsInRemoveList();

        unloader.UnloadN();

        // Unloading a grid can also add creatures to the list of objects to be
        // removed, for example guardian pets. Remove these now to avoid they
        // wouldn't actually be removed because the grid is already unloaded.
        RemoveAllObjectsInRemoveList();

        delete getNGrid(x, y);
        setNGrid(nullptr, x, y);
    }

    int gx = (MAX_NUMBER_OF_GRIDS - 1) - x;
    int gy = (MAX_NUMBER_OF_GRIDS - 1) - y;

    // unload GridMap - it is reference-countable so will be deleted safely when lockCount < 1
    // also simply set Map's pointer to corresponding GridMap object to nullptr
    if (m_bLoadedGrids[gx][gy])
    {
        m_bLoadedGrids[gx][gy] = false;
        m_TerrainData->Unload(gx, gy);
    }

    DEBUG_FILTER_LOG(LOG_FILTER_MAP_LOADING, "Unloading grid[%u,%u] for map %u finished", x, y, i_id);
    return true;
}

void Map::UnloadAll(bool pForce)
{
    for (GridRefManager<NGridType>::iterator i = GridRefManager<NGridType>::begin(); i != GridRefManager<NGridType>::end();)
    {
        NGridType& grid(*i->getSource());
        ++i;
        UnloadGrid(grid.getX(), grid.getY(), pForce);       // deletes the grid and removes it from the GridRefManager
    }
}

const char* Map::GetMapName() const
{
    return i_mapEntry ? i_mapEntry->name[sWorld.GetDefaultDbcLocale()] : "UNNAMEDMAP\x0";
}

void Map::UpdateObjectVisibility(WorldObject* obj, Cell cell, const CellPair& cellpair)
{
    cell.SetNoCreate();
    MaNGOS::VisibleChangesNotifier notifier(*obj);
    TypeContainerVisitor<MaNGOS::VisibleChangesNotifier, WorldTypeMapContainer > player_notifier(notifier);
    cell.Visit(cellpair, player_notifier, *this, *obj, obj->GetVisibilityData().GetVisibilityDistance());
    for (auto guid : notifier.GetUnvisitedGuids())
    {
        if (Player* player = GetPlayer(guid))
        {
#ifdef ENABLE_PLAYERBOTS
            if (sPlayerbotAIConfig.disableBotOptimizations || player->isRealPlayer())
#endif
            player->UpdateVisibilityOf(player->GetCamera().GetBody(), obj);
        }
    }
}

void Map::SendInitSelf(Player* player) const
{
    DETAIL_LOG("Creating player data for himself %u", player->GetGUIDLow());

    UpdateData updateData;

    bool hasTransport = false;

    // attach to player data current transport data
    if (GenericTransport* transport = player->GetTransport())
    {
        hasTransport = true;
        // player->m_clientGUIDs.insert(transport->GetObjectGuid());
        transport->BuildCreateUpdateBlockForPlayer(&updateData, player);
    }

    // build data for self presence in world at own client (one time for map)
    player->BuildCreateUpdateBlockForPlayer(&updateData, player);

    // build other passengers at transport also (they always visible and marked as visible and will not send at visibility update at add to map
    if (GenericTransport* transport = player->GetTransport())
    {
        for (auto passenger : transport->GetPassengers())
        {
            if (player != passenger)
            {
                if (player->HasAtClient(passenger) || passenger->isVisibleForInState(player, player, false))
                {
                    player->AddAtClient(passenger);
                    hasTransport = true;
                    passenger->BuildCreateUpdateBlockForPlayer(&updateData, player);
                }
            }
        }
    }

    for (size_t i = 0; i < updateData.GetPacketCount(); ++i)
    {
        WorldPacket packet = updateData.BuildPacket(i, hasTransport);
        player->GetSession()->SendPacket(packet);
    }
}

void Map::SendInitTransports(Player* player) const
{
    // Hack to send out transports
    // no transports at map
    if (m_transports.size() == 0)
        return;

    UpdateData updateData;

    bool hasTransport = false;

    for (auto i : m_transports)
    {
        // send data for current transport in other place
        if (i != player->GetTransport() && i->GetMapId() == i_id)
        {
            hasTransport = true;
            i->BuildCreateUpdateBlockForPlayer(&updateData, player);
        }
    }

    for (size_t i = 0; i < updateData.GetPacketCount(); ++i)
    {
        WorldPacket packet = updateData.BuildPacket(i, hasTransport);
        player->GetSession()->SendPacket(packet);
    }
}

void Map::SendRemoveTransports(Player* player) const
{
    // Hack to send out transports
    // no transports at map
    if (m_transports.size() == 0)
        return;

    UpdateData updateData;

    // except used transport
    for (auto i : m_transports)
        if (i != player->GetTransport() && i->GetMapId() != i_id)
            i->BuildOutOfRangeUpdateBlock(&updateData);

    for (size_t i = 0; i < updateData.GetPacketCount(); ++i)
    {
        WorldPacket packet = updateData.BuildPacket(i);
        player->GetSession()->SendPacket(packet);
    }
}

void Map::LoadTransports()
{
    uint32 mapId = GetId();
    for (TransportTemplate const* transportTemplate : sMapMgr.m_transportsByMap[mapId])
        Transport::LoadTransport(*transportTemplate, this);
}

inline void Map::setNGrid(NGridType* grid, uint32 x, uint32 y)
{
    if (x >= MAX_NUMBER_OF_GRIDS || y >= MAX_NUMBER_OF_GRIDS)
    {
        sLog.outError("map::setNGrid() Invalid grid coordinates found: %d, %d!", x, y);
        MANGOS_ASSERT(false);
    }
    i_grids[x][y] = grid;
}

void Map::AddObjectToRemoveList(WorldObject* obj)
{
    MANGOS_ASSERT(obj->GetMapId() == GetId() && obj->GetInstanceId() == GetInstanceId());

    obj->CleanupsBeforeDelete();                            // remove or simplify at least cross referenced links

    i_objectsToRemove.insert(obj);
    // DEBUG_LOG("Object (GUID: %u TypeId: %u ) added to removing list.",obj->GetGUIDLow(),obj->GetTypeId());
}

void Map::RemoveAllObjectsInRemoveList()
{
    if (i_objectsToRemove.empty())
        return;

    // DEBUG_LOG("Object remover 1 check.");
    while (!i_objectsToRemove.empty())
    {
        WorldObject* obj = *i_objectsToRemove.begin();
        i_objectsToRemove.erase(i_objectsToRemove.begin());

        switch (obj->GetTypeId())
        {
            case TYPEID_CORPSE:
            {
                // ??? WTF
                Corpse* corpse = GetCorpse(obj->GetObjectGuid());
                if (!corpse)
                    sLog.outError("Try delete corpse/bones %u that not in map", obj->GetGUIDLow());
                else
                    Remove(corpse, true);
                break;
            }
            case TYPEID_DYNAMICOBJECT:
                Remove((DynamicObject*)obj, true);
                break;
            case TYPEID_GAMEOBJECT:
                Remove((GameObject*)obj, true);
                break;
            case TYPEID_UNIT:
                Remove((Creature*)obj, true);
                break;
            default:
                sLog.outError("Non-grid object (TypeId: %u) in grid object removing list, ignored.", obj->GetTypeId());
                break;
        }
    }
    // DEBUG_LOG("Object remover 2 check.");
}

uint32 Map::GetPlayersCountExceptGMs() const
{
    uint32 count = 0;
    for (const auto& itr : m_mapRefManager)
        if (!itr.getSource()->IsGameMaster())
            ++count;
    return count;
}

void Map::SendToPlayers(WorldPacket const& data) const
{
    for (const auto& itr : m_mapRefManager)
        itr.getSource()->GetSession()->SendPacket(data);
}

bool Map::SendToPlayersInZone(WorldPacket const& data, uint32 zoneId) const
{
    bool foundPlayer = false;
    for (const auto& itr : m_mapRefManager)
    {
        if (itr.getSource()->GetZoneId() == zoneId)
        {
            itr.getSource()->GetSession()->SendPacket(data);
            foundPlayer = true;
        }
    }
    return foundPlayer;
}

bool Map::ActiveObjectsNearGrid(uint32 x, uint32 y) const
{
    MANGOS_ASSERT(x < MAX_NUMBER_OF_GRIDS);
    MANGOS_ASSERT(y < MAX_NUMBER_OF_GRIDS);

    CellPair cell_min(x * MAX_NUMBER_OF_CELLS, y * MAX_NUMBER_OF_CELLS);
    CellPair cell_max(cell_min.x_coord + MAX_NUMBER_OF_CELLS, cell_min.y_coord + MAX_NUMBER_OF_CELLS);

    // we must find visible range in cells so we unload only non-visible cells...
    float viewDist = GetVisibilityDistance();
    int cell_range = (int)ceilf(viewDist / SIZE_OF_GRID_CELL) + 1;

    cell_min << cell_range;
    cell_min -= cell_range;
    cell_max >> cell_range;
    cell_max += cell_range;

    for (const auto& iter : m_mapRefManager)
    {
        Player* plr = iter.getSource();

        CellPair p = MaNGOS::ComputeCellPair(plr->GetPositionX(), plr->GetPositionY());
        if ((cell_min.x_coord <= p.x_coord && p.x_coord <= cell_max.x_coord) &&
                (cell_min.y_coord <= p.y_coord && p.y_coord <= cell_max.y_coord))
            return true;
    }

    for (auto obj : m_activeNonPlayers)
    {
        CellPair p = MaNGOS::ComputeCellPair(obj->GetPositionX(), obj->GetPositionY());
        if ((cell_min.x_coord <= p.x_coord && p.x_coord <= cell_max.x_coord) &&
                (cell_min.y_coord <= p.y_coord && p.y_coord <= cell_max.y_coord))
            return true;
    }

    return false;
}

void Map::AddToActive(WorldObject* obj)
{
    m_activeNonPlayers.insert(obj);
    Cell cell = Cell(MaNGOS::ComputeCellPair(obj->GetPositionX(), obj->GetPositionY()));
    EnsureGridLoaded(cell);

    // also not allow unloading spawn grid to prevent creating creature clone at load
    if (obj->GetTypeId() == TYPEID_UNIT)
    {
        Creature* c = (Creature*)obj;

        if (!c->IsPet() && c->HasStaticDBSpawnData())
        {
            float x, y, z;
            c->GetRespawnCoord(x, y, z);
            GridPair p = MaNGOS::ComputeGridPair(x, y);
            if (getNGrid(p.x_coord, p.y_coord))
                getNGrid(p.x_coord, p.y_coord)->incUnloadActiveLock();
            else
            {
                GridPair p2 = MaNGOS::ComputeGridPair(c->GetPositionX(), c->GetPositionY());
                sLog.outError("Active creature (GUID: %u Entry: %u) added to grid[%u,%u] but spawn grid[%u,%u] not loaded.",
                              c->GetGUIDLow(), c->GetEntry(), p.x_coord, p.y_coord, p2.x_coord, p2.y_coord);
            }
        }
    }
}

void Map::RemoveFromActive(WorldObject* obj)
{
    // Map::Update for active object in proccess
    if (m_activeNonPlayersIter != m_activeNonPlayers.end())
    {
        ActiveNonPlayers::iterator itr = m_activeNonPlayers.find(obj);
        if (itr == m_activeNonPlayersIter)
            ++m_activeNonPlayersIter;
        m_activeNonPlayers.erase(itr);
    }
    else
        m_activeNonPlayers.erase(obj);

    // also allow unloading spawn grid
    if (obj->GetTypeId() == TYPEID_UNIT)
    {
        Creature* c = (Creature*)obj;

        if (!c->IsPet() && c->HasStaticDBSpawnData())
        {
            float x, y, z;
            c->GetRespawnCoord(x, y, z);
            GridPair p = MaNGOS::ComputeGridPair(x, y);
            if (getNGrid(p.x_coord, p.y_coord))
                getNGrid(p.x_coord, p.y_coord)->decUnloadActiveLock();
            else
            {
                GridPair p2 = MaNGOS::ComputeGridPair(c->GetPositionX(), c->GetPositionY());
                sLog.outError("Active creature (GUID: %u Entry: %u) removed from grid[%u,%u] but spawn grid[%u,%u] not loaded.",
                              c->GetGUIDLow(), c->GetEntry(), p.x_coord, p.y_coord, p2.x_coord, p2.y_coord);
            }
        }
    }
}

void Map::AddToOnEventNotified(WorldObject* obj)
{
    m_onEventNotifiedObjects.insert(obj);
}

void Map::RemoveFromOnEventNotified(WorldObject* obj)
{
    if (m_onEventNotifiedIter != m_onEventNotifiedObjects.end())
    {
        auto itr = m_onEventNotifiedObjects.find(obj);
        if (itr == m_onEventNotifiedIter)
            ++m_onEventNotifiedIter;
        m_onEventNotifiedObjects.erase(obj);
    }
    else
        m_onEventNotifiedObjects.erase(obj);
}

void Map::CreateInstanceData(bool load)
{
    if (i_data != nullptr)
        return;

    if (Instanceable())
    {
        if (InstanceTemplate const* mInstance = ObjectMgr::GetInstanceTemplate(GetId()))
            i_script_id = mInstance->script_id;
    }
    else
    {
        if (WorldTemplate const* mInstance = ObjectMgr::GetWorldTemplate(GetId()))
            i_script_id = mInstance->script_id;
    }

    if (!i_script_id)
        return;

    i_data = sScriptDevAIMgr.CreateInstanceData(this);
    if (!i_data)
        return;

    if (load)
    {
        std::unique_ptr<QueryResult> queryResult;

        if (Instanceable())
            queryResult = CharacterDatabase.PQuery("SELECT data FROM instance WHERE id = '%u'", i_InstanceId);
        else
            queryResult = CharacterDatabase.PQuery("SELECT data FROM world WHERE map = '%u'", GetId());

        if (queryResult)
        {
            Field* fields = queryResult->Fetch();
            const char* data = fields[0].GetString();
            if (data)
            {
                DEBUG_LOG("Loading instance data for `%s` (Map: %u Instance: %u)", sScriptDevAIMgr.GetScriptName(i_script_id), GetId(), i_InstanceId);
                i_data->Load(data);
            }
        }
        else
        {
            // for non-instanceable map always add data to table if not found, later code expected that for map in `word` exist always after load
            if (!Instanceable())
                CharacterDatabase.PExecute("INSERT INTO world VALUES ('%u', '')", GetId());
        }
    }
    else
    {
        DEBUG_LOG("New instance data, \"%s\" ,initialized!", sScriptDevAIMgr.GetScriptName(i_script_id));
        i_data->Initialize();
    }
}

void Map::TeleportAllPlayersTo(TeleportLocation loc)
{
    while (HavePlayers())
    {
        if (Player* plr = m_mapRefManager.getFirst()->getSource())
        {
            // Teleport to specified location and removes the player from this map (if the map exists).
            // Todo : we can add some specific location if needed (ex: map exit location for dungeon)
            switch (loc)
            {
                case TELEPORT_LOCATION_HOMEBIND:
                    plr->TeleportToHomebind();
                    break;
                case TELEPORT_LOCATION_BG_ENTRY_POINT:
                    plr->TeleportToBGEntryPoint();
                    break;
                default:
                    break;
            }
            // just in case, remove the player from the list explicitly here as well to prevent a possible infinite loop
            // note that this remove is not needed if the code works well in other places
            plr->GetMapRef().unlink();
        }
    }
}

void Map::SetWeather(uint32 zoneId, WeatherType type, float grade, bool permanently)
{
    Weather* wth = m_weatherSystem->FindOrCreateWeather(zoneId);
    wth->SetWeather(WeatherType(type), grade, this, permanently);
}

template void Map::Add(Corpse*);
template void Map::Add(Creature*);
template void Map::Add(GameObject*);
template void Map::Add(DynamicObject*);

template void Map::Remove(Corpse*, bool);
template void Map::Remove(Creature*, bool);
template void Map::Remove(GameObject*, bool);
template void Map::Remove(DynamicObject*, bool);

/* ******* World Maps ******* */

WorldPersistentState* WorldMap::GetPersistanceState() const
{
    return (WorldPersistentState*)Map::GetPersistentState();
}

/* ******* Dungeon Instance Maps ******* */

DungeonMap::DungeonMap(uint32 id, time_t expiry, uint32 InstanceId)
    : Map(id, expiry, InstanceId),
      m_resetAfterUnload(false), m_unloadWhenEmpty(false), m_team(TEAM_NONE)
{
    MANGOS_ASSERT(i_mapEntry->IsDungeon());

    // the timer is started by default, and stopped when the first player joins
    // this make sure it gets unloaded if for some reason no player joins
    m_unloadTimer = std::max(sWorld.getConfig(CONFIG_UINT32_INSTANCE_UNLOAD_DELAY), (uint32)MIN_UNLOAD_DELAY);
}

DungeonMap::~DungeonMap()
{
}

void DungeonMap::InitVisibilityDistance()
{
    // init visibility distance for instances
    m_VisibleDistance = World::GetMaxVisibleDistanceInInstances();
}

/*
    Do map specific checks and add the player to the map if successful.
*/
bool DungeonMap::Add(Player* player)
{
    // TODO: Not sure about checking player level: already done in HandleAreaTriggerOpcode
    // GMs still can teleport player in instance.
    // Is it needed?

    if (!CanEnter(player))
        return false;

    // check for existing instance binds
    InstancePlayerBind* playerBind = player->GetBoundInstance(GetId());
    if (playerBind && playerBind->perm)
    {
        // cannot enter other instances if bound permanently
        if (playerBind->state != GetPersistanceState())
        {
            sLog.outError("InstanceMap::Add: player %s(%d) is permanently bound to instance %d,%d,%d,%d,%d but he is being put in instance %d,%d,%d,%d,%d",
                          player->GetName(), player->GetGUIDLow(), playerBind->state->GetMapId(),
                          playerBind->state->GetInstanceId(),
                          playerBind->state->GetPlayerCount(), playerBind->state->GetGroupCount(),
                          playerBind->state->CanReset(),
                          GetPersistanceState()->GetMapId(), GetPersistanceState()->GetInstanceId(),
                          GetPersistanceState()->GetPlayerCount(),
                          GetPersistanceState()->GetGroupCount(), GetPersistanceState()->CanReset());
            MANGOS_ASSERT(false);
        }
    }
    else
    {
        Group* pGroup = player->GetGroup();
        if (pGroup)
        {
            // solo saves should be reset when entering a group
            InstanceGroupBind* groupBind = pGroup->GetBoundInstance(GetId());
            if (playerBind)
            {
                sLog.outError("InstanceMap::Add: %s is being put in instance %d,%d,%d,%d,%d but he is in group (Id: %d) and is bound to instance %d,%d,%d,%d,%d!",
                              player->GetGuidStr().c_str(), playerBind->state->GetMapId(), playerBind->state->GetInstanceId(),
                              playerBind->state->GetPlayerCount(), playerBind->state->GetGroupCount(),
                              playerBind->state->CanReset(), pGroup->GetId(),
                              playerBind->state->GetMapId(), playerBind->state->GetInstanceId(),
                              playerBind->state->GetPlayerCount(), playerBind->state->GetGroupCount(), playerBind->state->CanReset());

                if (groupBind)
                    sLog.outError("InstanceMap::Add: the group (Id: %d) is bound to instance %d,%d,%d,%d,%d",
                                  pGroup->GetId(),
                                  groupBind->state->GetMapId(), groupBind->state->GetInstanceId(),
                                  groupBind->state->GetPlayerCount(), groupBind->state->GetGroupCount(), groupBind->state->CanReset());

                // no reason crash if we can fix state
                player->UnbindInstance(GetId());
            }

            // bind to the group or keep using the group save
            if (!groupBind)
                pGroup->BindToInstance(GetPersistanceState(), false);
            else
            {
                // cannot jump to a different instance without resetting it
                if (groupBind->state != GetPersistentState())
                {
                    sLog.outError("InstanceMap::Add: %s is being put in instance %d,%d but he is in group (Id: %d) which is bound to instance %d,%d!",
                                  player->GetGuidStr().c_str(), GetPersistentState()->GetMapId(),
                                  GetPersistentState()->GetInstanceId(),
                                  pGroup->GetId(), groupBind->state->GetMapId(),
                                  groupBind->state->GetInstanceId());

                    sLog.outError("MapSave players: %d, group count: %d",
                                  GetPersistanceState()->GetPlayerCount(), GetPersistanceState()->GetGroupCount());

                    if (groupBind->state)
                        sLog.outError("GroupBind save players: %d, group count: %d", groupBind->state->GetPlayerCount(), groupBind->state->GetGroupCount());
                    else
                        sLog.outError("GroupBind save nullptr");
                    MANGOS_ASSERT(false);
                }
                // if the group/leader is permanently bound to the instance
                // players also become permanently bound when they enter
                if (groupBind->perm)
                {
                    WorldPacket data(SMSG_INSTANCE_SAVE_CREATED, 4);
                    data << uint32(0);
                    player->GetSession()->SendPacket(data);
                    player->BindToInstance(GetPersistanceState(), true);
                }
            }
        }
        else
        {
            // set up a solo bind or continue using it
            if (!playerBind)
                player->BindToInstance(GetPersistanceState(), false);
            else
                // cannot jump to a different instance without resetting it
                MANGOS_ASSERT(playerBind->state == GetPersistentState());
        }
    }

    // for normal instances cancel the reset schedule when the
    // first player enters (no players yet)
    SetResetSchedule(false);

    DETAIL_LOG("MAP: Player '%s' is entering instance '%u' of map '%s'", player->GetName(), GetInstanceId(), GetMapName());
    // initialize unload state
    m_unloadTimer = 0;
    m_resetAfterUnload = false;
    m_unloadWhenEmpty = false;

    if (!m_team)
        m_team = player->GetTeam();

    if (i_mapEntry->IsNonRaidDungeon())
        player->AddNewInstanceId(GetInstanceId());

    // this will acquire the same mutex so it cannot be in the previous block
    Map::Add(player);

    return true;
}

void DungeonMap::Update(const uint32& t_diff)
{
    Map::Update(t_diff);
}

void DungeonMap::Remove(Player* player, bool remove)
{
    DETAIL_LOG("MAP: Removing player '%s' from instance '%u' of map '%s' before relocating to other map", player->GetName(), GetInstanceId(), GetMapName());

    // if last player set unload timer
    if (!m_unloadTimer && m_mapRefManager.getSize() == 1)
        m_unloadTimer = m_unloadWhenEmpty ? MIN_UNLOAD_DELAY : std::max(sWorld.getConfig(CONFIG_UINT32_INSTANCE_UNLOAD_DELAY), (uint32)MIN_UNLOAD_DELAY);

    Map::Remove(player, remove);

    // for normal instances schedule the reset after all players have left
    SetResetSchedule(true);
}

/*
    Returns true if there are no players in the instance
*/
bool DungeonMap::Reset(InstanceResetMethod method)
{
    // note: since the map may not be loaded when the instance needs to be reset
    // the instance must be deleted from the DB by InstanceSaveManager

    if (HavePlayers())
    {
        if (method == INSTANCE_RESET_ALL)
        {

        }
        else
        {
            if (method == INSTANCE_RESET_GLOBAL)
            {
                // set the homebind timer for players inside (1 minute)
                for (auto& itr : m_mapRefManager)
                    itr.getSource()->m_InstanceValid = false;
            }

            // the unload timer is not started
            // instead the map will unload immediately after the players have left
            m_unloadWhenEmpty = true;
            m_resetAfterUnload = true;
        }
    }
    else
    {
        // unloaded at next update
        m_unloadTimer = MIN_UNLOAD_DELAY;
        m_resetAfterUnload = true;
    }

    return m_mapRefManager.isEmpty();
}

void DungeonMap::PermBindAllPlayers(Player* player)
{
    Group* group = nullptr;
    if (player)
        group = player->GetGroup();
    else
    {
        for (auto& itr : m_mapRefManager)
        {
            Player* plr = itr.getSource();
            if (plr->GetGroup() && !plr->IsGameMaster())
            {
                group = plr->GetGroup();
                break;
            }
        }
    }
    // group members outside the instance group don't get bound
    for (auto& itr : m_mapRefManager)
    {
        Player* plr = itr.getSource();
        // players inside an instance cannot be bound to other instances
        // some players may already be permanently bound, in this case nothing happens
        InstancePlayerBind* bind = plr->GetBoundInstance(GetId());
        if (!bind || !bind->perm)
        {
            plr->BindToInstance(GetPersistanceState(), true);
            WorldPacket data(SMSG_INSTANCE_SAVE_CREATED, 4);
            data << uint32(0);
            plr->GetSession()->SendPacket(data);
        }

        // if the leader is not in the instance the group will not get a perm bind
        if (group && group->GetLeaderGuid() == plr->GetObjectGuid())
            group->BindToInstance(GetPersistanceState(), true);
    }
}

void DungeonMap::UnloadAll(bool pForce)
{
    TeleportAllPlayersTo(TELEPORT_LOCATION_HOMEBIND);

    if (m_resetAfterUnload)
        GetPersistanceState()->DeleteRespawnTimes();

    Map::UnloadAll(pForce);
}

void DungeonMap::SendResetWarnings(uint32 timeLeft) const
{
    for (const auto& itr : m_mapRefManager)
        itr.getSource()->SendInstanceResetWarning(GetId(), timeLeft);
}

void DungeonMap::SetResetSchedule(bool on)
{
    // only for normal instances
    // the reset time is only scheduled when there are no payers inside
    // it is assumed that the reset time will rarely (if ever) change while the reset is scheduled
    if (!HavePlayers() && !IsRaid())
    {
        time_t resetTime;
        if (on)
        {
            resetTime = (uint64)(time(nullptr) + NORMAL_INSTANCE_RESET_TIME);
            GetPersistanceState()->SetResetTime(resetTime);
        }
        else
            resetTime = GetPersistanceState()->GetResetTime();

        sMapPersistentStateMgr.GetScheduler().ScheduleReset(on, resetTime, DungeonResetEvent(RESET_EVENT_NORMAL_DUNGEON, GetId(), GetInstanceId()));
    }
}

uint32 DungeonMap::GetMaxPlayers() const
{
    InstanceTemplate const* iTemplate = ObjectMgr::GetInstanceTemplate(GetId());
    if (!iTemplate)
        return 0;
    return iTemplate->maxPlayers;
}

DungeonPersistentState* DungeonMap::GetPersistanceState() const
{
    return (DungeonPersistentState*)Map::GetPersistentState();
}

/* ******* Battleground Instance Maps ******* */

BattleGroundMap::BattleGroundMap(uint32 id, time_t expiry, uint32 InstanceId)
    : Map(id, expiry, InstanceId)
{
}

BattleGroundMap::~BattleGroundMap()
{
}

void BattleGroundMap::Initialize(bool)
{
    Map::Initialize(false);
}

void BattleGroundMap::Update(const uint32& diff)
{
    Map::Update(diff);

    if (!m_bg)
        return;

    if (!m_bg->GetPlayersSize())
    {
        // BG is empty
        // if there are no players invited, delete BG
        // this will delete arena or bg object, where any player entered
        // [[   but if you use battleground object again (more battles possible to be played on 1 instance)
        //      then this condition should be removed and code:
        //      if (!GetInvitedCount(HORDE) && !GetInvitedCount(ALLIANCE))
        //          this->AddToFreeBGObjectsQueue(); // not yet implemented
        //      should be used instead of current
        // ]]
        // BattleGround Template instance cannot be updated, because it would be deleted
        if (!m_bg->GetInvitedCount(HORDE) && !m_bg->GetInvitedCount(ALLIANCE))
        {
            sBattleGroundMgr.GetMessager().AddMessage([instanceId = GetInstanceId(), typeId = m_bg->GetTypeId()](BattleGroundMgr* mgr)
            {
                mgr->RemoveBattleGround(instanceId, typeId);
            });
            m_bg = nullptr;
        }
    }
    else
        m_bg->Update(diff);
}

BattleGroundPersistentState* BattleGroundMap::GetPersistanceState() const
{
    return (BattleGroundPersistentState*)Map::GetPersistentState();
}


void BattleGroundMap::InitVisibilityDistance()
{
    // init visibility distance for BG
    m_VisibleDistance = World::GetMaxVisibleDistanceInBG();
}

bool BattleGroundMap::CanEnter(Player* player)
{
    if (!Map::CanEnter(player))
        return false;

    if (player->GetBattleGroundId() != GetInstanceId())
        return false;

    // player number limit is checked in bgmgr, no need to do it here
    return true;
}

bool BattleGroundMap::Add(Player* player)
{
    if (!CanEnter(player))
        return false;

    // reset instance validity, battleground maps do not homebind
    player->m_InstanceValid = true;

    return Map::Add(player);
}

void BattleGroundMap::Remove(Player* player, bool remove)
{
    DETAIL_FILTER_LOG(LOG_FILTER_PLAYER_MOVES, "MAP: Removing player '%s' from bg '%u' of map '%s' before relocating to other map", player->GetName(), GetInstanceId(), GetMapName());
    Map::Remove(player, remove);
}

void BattleGroundMap::SetUnload()
{
    m_unloadTimer = MIN_UNLOAD_DELAY;
}

void BattleGroundMap::UnloadAll(bool pForce)
{
    TeleportAllPlayersTo(TELEPORT_LOCATION_BG_ENTRY_POINT);

    Map::UnloadAll(pForce);
}

bool Map::CanEnter(Player* player)
{
    if (player->GetMapRef().getTarget() == this)
        return false;

    return true;
}

/// Put scripts in the execution queue
bool Map::ScriptsStart(ScriptMapType scriptType, uint32 id, Object* source, Object* target, ScriptExecutionParam execParams /*=SCRIPT_EXEC_PARAM_UNIQUE_BY_SOURCE_TARGET*/)
{
    MANGOS_ASSERT(source);

    ///- Find the script map
    auto scriptMapMap = GetMapDataContainer().GetScriptMap(scriptType);
    ScriptMapMap::const_iterator scriptInfoMapMapItr = scriptMapMap->second.find(id);
    if (scriptInfoMapMapItr == scriptMapMap->second.end())
        return false;

    // prepare static data
    ObjectGuid sourceGuid = source->GetObjectGuid();
    ObjectGuid targetGuid = target ? target->GetObjectGuid() : ObjectGuid();
    ObjectGuid ownerGuid  = source->isType(TYPEMASK_ITEM) ? ((Item*)source)->GetOwnerGuid() : ObjectGuid();

    if (execParams)                                         // Check if the execution should be uniquely
    {
        for (ScriptScheduleMap::const_iterator searchItr = m_scriptSchedule.begin(); searchItr != m_scriptSchedule.end(); ++searchItr)
        {
            if (searchItr->second.IsSameScript(scriptMapMap->first, id,
                                               execParams & SCRIPT_EXEC_PARAM_UNIQUE_BY_SOURCE ? sourceGuid : ObjectGuid(),
                                               execParams & SCRIPT_EXEC_PARAM_UNIQUE_BY_TARGET ? targetGuid : ObjectGuid(), ownerGuid))
            {
                DETAIL_FILTER_LOG(LOG_FILTER_DB_SCRIPT, "DB-SCRIPTS: Process table `%s` id %u. Skip script as script already started for source %s, target %s - ScriptsStartParams %u", scriptMapMap->first, id, sourceGuid.GetString().c_str(), targetGuid.GetString().c_str(), execParams);
                return true;
            }
        }
    }

    ///- Schedule script execution for all scripts in the script map
    ScriptMap const& scriptMap = scriptInfoMapMapItr->second;

    // first handle all scripts with 0 delay
    auto scriptInfoItr = scriptMap.begin();
    while (scriptInfoItr != scriptMap.end())
    {
        auto const& scriptInfo = scriptInfoItr->second;
        if (scriptInfo->delay > 0)
            break;

        // fire script with 0 delay directly
        ScriptAction sa(scriptType, this, sourceGuid, targetGuid, ownerGuid, scriptInfo);
        if (sa.HandleScriptStep())
            return true;                    // script failed we should not continue further (command 31 or any error occured)

        ++scriptInfoItr;
    }

    // add delayed script to script scheduler
    for (; scriptInfoItr != scriptMap.end(); ++scriptInfoItr)
    {
        auto const& scriptInfo = scriptInfoItr->second;
        ScriptAction sa(scriptType, this, sourceGuid, targetGuid, ownerGuid, scriptInfo);
        m_scriptSchedule.emplace(GetCurrentClockTime() + std::chrono::milliseconds(scriptInfoItr->first), sa);
    }

    return true;
}

void Map::ScriptCommandStart(ScriptInfo const& script, uint32 delay, Object* source, Object* target)
{
    // NOTE: script record _must_ exist until command executed

    // prepare static data
    ObjectGuid sourceGuid = source->GetObjectGuid();
    ObjectGuid targetGuid = target ? target->GetObjectGuid() : ObjectGuid();
    ObjectGuid ownerGuid  = source->isType(TYPEMASK_ITEM) ? ((Item*)source)->GetOwnerGuid() : ObjectGuid();

    ScriptAction sa(SCRIPT_TYPE_INTERNAL, this, sourceGuid, targetGuid, ownerGuid, std::make_shared<ScriptInfo>(script));

    if (delay)
    {
        m_scriptSchedule.emplace(GetCurrentClockTime() + std::chrono::milliseconds(delay), sa);
    }
    else
        sa.HandleScriptStep();
}

/// Process queued scripts
void Map::ScriptsProcess()
{
    if (m_scriptSchedule.empty())
        return;

    ///- Process overdue queued scripts
    ScriptScheduleMap::iterator iter = m_scriptSchedule.begin();
    // ok as multimap is a *sorted* associative container
    while (!m_scriptSchedule.empty() && (iter->first <= GetCurrentClockTime()))
    {
        if (iter->second.HandleScriptStep())
        {
            // Terminate following script steps of this script
            const char* tableName = iter->second.GetTableName();
            uint32 id = iter->second.GetId();
            ObjectGuid sourceGuid = iter->second.GetSourceGuid();
            ObjectGuid targetGuid = iter->second.GetTargetGuid();
            ObjectGuid ownerGuid = iter->second.GetOwnerGuid();

            for (ScriptScheduleMap::iterator rmItr = m_scriptSchedule.begin(); rmItr != m_scriptSchedule.end();)
            {
                if (rmItr->second.IsSameScript(tableName, id, sourceGuid, targetGuid, ownerGuid))
                    m_scriptSchedule.erase(rmItr++);
                else
                    ++rmItr;
            }
        }
        else
            m_scriptSchedule.erase(iter);

        iter = m_scriptSchedule.begin();
    }
}

/**
 * Function return player that in world at CURRENT map
 *
 * Note: This is function preferred if you sure that need player only placed at specific map
 *       This is not true for some spell cast targeting and most packet handlers
 *
 * @param guid must be player guid (HIGHGUID_PLAYER)
 */
Player* Map::GetPlayer(ObjectGuid guid)
{
    Player* plr = ObjectAccessor::FindPlayer(guid);         // return only in world players
    return plr && plr->GetMap() == this ? plr : nullptr;
}

/**
 * Function return creature (non-pet and then most summoned by spell creatures) that in world at CURRENT map
 *
 * @param guid must be creature guid (HIGHGUID_UNIT)
 */
Creature* Map::GetCreature(ObjectGuid guid)
{
    return m_objectsStore.find<Creature>(guid, (Creature*)nullptr);
}

/**
 * Function return pet that in world at CURRENT map
 *
 * @param guid must be pet guid (HIGHGUID_PET)
 */
Pet* Map::GetPet(ObjectGuid guid)
{
    return m_objectsStore.find<Pet>(guid, (Pet*)nullptr);
}

/**
 * Function return corpse that at CURRENT map
 *
 * Note: corpse can be NOT IN WORLD, so can't be used corpse->GetMap() without pre-check corpse->isInWorld()
 *
 * @param guid must be corpse guid (HIGHGUID_CORPSE)
 */
Corpse* Map::GetCorpse(ObjectGuid guid) const
{
    Corpse* ret = ObjectAccessor::GetCorpseInMap(guid, GetId());
    return ret && ret->GetInstanceId() == GetInstanceId() ? ret : nullptr;
}

/**
 * Function return non-player unit object that in world at CURRENT map, so creature, or pet
 *
 * @param guid must be non-player unit guid (HIGHGUID_PET HIGHGUID_UNIT)
 */
Creature* Map::GetAnyTypeCreature(ObjectGuid guid)
{
    switch (guid.GetHigh())
    {
        case HIGHGUID_UNIT:         return GetCreature(guid);
        case HIGHGUID_PET:          return GetPet(guid);
        default:                    break;
    }

    return nullptr;
}

/**
 * Function return gameobject that in world at CURRENT map
 *
 * @param guid must be gameobject guid (HIGHGUID_GAMEOBJECT)
 */
GameObject* Map::GetGameObject(ObjectGuid guid)
{
    return m_objectsStore.find<GameObject>(guid, (GameObject*)nullptr);
}

/**
 * Function return dynamic object that in world at CURRENT map
 *
 * @param guid must be dynamic object guid (HIGHGUID_DYNAMICOBJECT)
 */
DynamicObject* Map::GetDynamicObject(ObjectGuid guid)
{
    return m_objectsStore.find<DynamicObject>(guid, (DynamicObject*)nullptr);
}

/**
 * Function return unit in world at CURRENT map
 *
 * Note: in case player guid not always expected need player at current map only.
 *       For example in spell casting can be expected any in world player targeting in some cases
 *
 * @param guid must be unit guid (HIGHGUID_PLAYER HIGHGUID_PET HIGHGUID_UNIT)
 */
Unit* Map::GetUnit(ObjectGuid guid)
{
    if (guid.IsPlayer())
        return GetPlayer(guid);

    return GetAnyTypeCreature(guid);
}

/**
 * Function return world object in world at CURRENT map, so any except transports
 */
WorldObject* Map::GetWorldObject(ObjectGuid guid)
{
    switch (guid.GetHigh())
    {
        case HIGHGUID_PLAYER:       return GetPlayer(guid);
        case HIGHGUID_TRANSPORT:
        case HIGHGUID_GAMEOBJECT:   return GetGameObject(guid);
        case HIGHGUID_UNIT:         return GetCreature(guid);
        case HIGHGUID_PET:          return GetPet(guid);
        case HIGHGUID_DYNAMICOBJECT: return GetDynamicObject(guid);
        case HIGHGUID_CORPSE:
        {
            // corpse special case, it can be not in world
            Corpse* corpse = GetCorpse(guid);
            return corpse && corpse->IsInWorld() ? corpse : nullptr;
        }
        case HIGHGUID_MO_TRANSPORT:
        default:                    break;
    }

    return nullptr;
}

void Map::SendObjectUpdates()
{
    UpdateDataMapType update_players;

    while (!i_objectsToClientUpdate.empty())
    {
        Object* obj = *i_objectsToClientUpdate.begin();
        i_objectsToClientUpdate.erase(i_objectsToClientUpdate.begin());
        obj->BuildUpdateData(update_players);
    }

    for (auto& update_player : update_players)
    {
        for (size_t i = 0; i < update_player.second.GetPacketCount(); ++i)
        {
            WorldPacket packet = update_player.second.BuildPacket(i);
            update_player.first->GetSession()->SendPacket(packet);
        }
    }
}

Creature* Map::GetCreature(uint32 dbguid) const
{
    auto itr = m_dbGuidObjects.find(std::make_pair(HIGHGUID_UNIT, dbguid));
    if (itr == m_dbGuidObjects.end())
        return nullptr;

    if ((*itr).second.empty())
        return nullptr;

    // prioritize alive one
    for (auto data : (*itr).second)
        if (static_cast<Creature*>(data)->IsAlive())
            return static_cast<Creature*>(data);

    return static_cast<Creature*>((*itr).second.front());
}

GameObject* Map::GetGameObject(uint32 dbguid) const
{
    auto itr = m_dbGuidObjects.find(std::make_pair(HIGHGUID_GAMEOBJECT, dbguid));
    if (itr == m_dbGuidObjects.end())
        return nullptr;

    if ((*itr).second.empty())
        return nullptr;

    return static_cast<GameObject*>((*itr).second.front());
}

std::vector<WorldObject*> const* Map::GetWorldObjects(std::string stringId) const
{
    return GetWorldObjects(GetMapDataContainer().GetStringId(stringId));
}

std::vector<Creature*> const* Map::GetCreatures(std::string stringId) const
{
    return GetCreatures(GetMapDataContainer().GetStringId(stringId));
}

std::vector<GameObject*> const* Map::GetGameObjects(std::string stringId) const
{
    return GetGameObjects(GetMapDataContainer().GetStringId(stringId));
}

std::vector<WorldObject*> const* Map::GetWorldObjects(uint32 stringId) const
{
    auto itr = m_objectsPerStringId.find(stringId);
    if (itr == m_objectsPerStringId.end())
        return nullptr;

    return &(itr->second.worldObjects);
}

std::vector<Creature*> const* Map::GetCreatures(uint32 stringId) const
{
    auto itr = m_objectsPerStringId.find(stringId);
    if (itr == m_objectsPerStringId.end())
        return nullptr;

    return &(itr->second.creatures);
}

std::vector<GameObject*> const* Map::GetGameObjects(uint32 stringId) const
{
    auto itr = m_objectsPerStringId.find(stringId);
    if (itr == m_objectsPerStringId.end())
        return nullptr;

    return &(itr->second.gameobjects);
}

WorldObject* Map::GetWorldObject(std::string stringId) const
{
    auto objects = GetWorldObjects(stringId);
    return objects && !objects->empty() ? objects->front() : nullptr;
}

Creature* Map::GetCreature(std::string stringId) const
{
    auto objects = GetCreatures(stringId);
    return objects && !objects->empty() ? objects->front() : nullptr;
}

GameObject* Map::GetGameObject(std::string stringId) const
{
    auto objects = GetGameObjects(stringId);
    return objects && !objects->empty() ? objects->front() : nullptr;
}

void Map::AddDbGuidObject(WorldObject* obj)
{
    m_dbGuidObjects[std::make_pair(HighGuid(obj->GetParentHigh()), obj->GetDbGuid())].push_back(obj);
}

void Map::RemoveDbGuidObject(WorldObject* obj)
{
    auto& vec = m_dbGuidObjects[std::make_pair(HighGuid(obj->GetParentHigh()), obj->GetDbGuid())];
    vec.erase(std::remove(vec.begin(), vec.end(), obj), vec.end());
}

void Map::AddStringIdObject(uint32 stringId, WorldObject* obj)
{
    auto& data = m_objectsPerStringId[stringId];
    data.worldObjects.push_back(obj);
    if (obj->IsCreature())
        data.creatures.push_back(static_cast<Creature*>(obj));
    else if (obj->IsGameObject())
        data.gameobjects.push_back(static_cast<GameObject*>(obj));
}

void Map::RemoveStringIdObject(uint32 stringId, WorldObject* obj)
{
    auto& data = m_objectsPerStringId[stringId];
    data.worldObjects.erase(std::remove(data.worldObjects.begin(), data.worldObjects.end(), obj), data.worldObjects.end());
    if (obj->IsCreature())
        data.creatures.erase(std::remove(data.creatures.begin(), data.creatures.end(), static_cast<Creature*>(obj)), data.creatures.end());
    else if (obj->IsGameObject())
        data.gameobjects.erase(std::remove(data.gameobjects.begin(), data.gameobjects.end(), static_cast<GameObject*>(obj)), data.gameobjects.end());
}

uint32 Map::GenerateLocalLowGuid(HighGuid guidhigh)
{
    // TODO: for map local guid counters possible force reload map instead shutdown server at guid counter overflow
    switch (guidhigh)
    {
        case HIGHGUID_UNIT:
            return m_CreatureGuids.Generate();
        case HIGHGUID_TRANSPORT:
        case HIGHGUID_GAMEOBJECT:
            return m_GameObjectGuids.Generate();
        case HIGHGUID_DYNAMICOBJECT:
            return m_DynObjectGuids.Generate();
        case HIGHGUID_PET:
            return m_PetGuids.Generate();
        default:
            MANGOS_ASSERT(false);
            return 0;
    }
}

/**
 * Helper structure for building static chat information
 *
 */
class StaticMonsterChatBuilder
{
    public:
        StaticMonsterChatBuilder(CreatureInfo const* cInfo, ChatMsg msgtype, int32 textId, Language language, Unit const* target, uint32 senderLowGuid = 0)
            : i_cInfo(cInfo), i_msgtype(msgtype), i_textId(textId), i_language(language), i_target(target)
        {
            // 0 lowguid not used in core, but accepted fine in this case by client
            i_senderGuid = i_cInfo->GetObjectGuid(senderLowGuid);
        }
        void operator()(WorldPacket& data, int32 loc_idx)
        {
            char const* text = sObjectMgr.GetMangosString(i_textId, loc_idx);

            char const* nameForLocale = i_cInfo->Name;
            sObjectMgr.GetCreatureLocaleStrings(i_cInfo->Entry, loc_idx, &nameForLocale);

            ChatHandler::BuildChatPacket(data, i_msgtype, text, i_language, CHAT_TAG_NONE, i_senderGuid, nameForLocale, i_target ? i_target->GetObjectGuid() : ObjectGuid(),
                                         i_target ? i_target->GetNameForLocaleIdx(loc_idx) : "");
        }

    private:
        ObjectGuid i_senderGuid;
        CreatureInfo const* i_cInfo;
        ChatMsg i_msgtype;
        int32 i_textId;
        Language i_language;
        Unit const* i_target;
};


/**
 * Function simulates yell of creature
 *
 * @param guid must be creature guid of whom to Simulate the yell, non-creature guids not supported at this moment
 * @param textId Id of the simulated text
 * @param language language of the text
 * @param target, can be nullptr
 */
void Map::MonsterYellToMap(ObjectGuid guid, int32 textId, ChatMsg chatMsg, Language language, Unit const* target) const
{
    if (guid.IsAnyTypeCreature())
    {
        CreatureInfo const* cInfo = ObjectMgr::GetCreatureTemplate(guid.GetEntry());
        if (!cInfo)
        {
            sLog.outError("Map::MonsterYellToMap: Called for nonexistent creature entry in guid: %s", guid.GetString().c_str());
            return;
        }

        MonsterYellToMap(cInfo, textId, chatMsg, language, target, guid.GetCounter());
    }
    else
    {
        sLog.outError("Map::MonsterYellToMap: Called for non creature guid: %s", guid.GetString().c_str());
    }
}


/**
 * Function simulates yell of creature
 *
 * @param cinfo must be entry of Creature of whom to Simulate the yell
 * @param textId Id of the simulated text
 * @param language language of the text
 * @param target, can be nullptr
 * @param senderLowGuid provide way proper show yell for near spawned creature with known lowguid,
 *        0 accepted by client else if this not important
 */
void Map::MonsterYellToMap(CreatureInfo const* cinfo, int32 textId, ChatMsg chatMsg, Language language, Unit const* target, uint32 senderLowGuid /*= 0*/) const
{
    StaticMonsterChatBuilder say_build(cinfo, chatMsg, textId, language, target, senderLowGuid);
    MaNGOS::LocalizedPacketDo<StaticMonsterChatBuilder> say_do(say_build);

    Map::PlayerList const& pList = GetPlayers();
    for (const auto& itr : pList)
        say_do(itr.getSource());
}

/**
 * Function to play sound to all players in map
 *
 * @param soundId Played Sound
 * @param zoneId Id of the Zone to which the sound should be restricted
 */
void Map::PlayDirectSoundToMap(uint32 soundId, uint32 zoneId /*=0*/) const
{
    WorldPacket data(SMSG_PLAY_SOUND, 4);
    data << uint32(soundId);

    Map::PlayerList const& pList = GetPlayers();
    for (const auto& itr : pList)
        if (!zoneId || itr.getSource()->GetZoneId() == zoneId)
            itr.getSource()->SendDirectMessage(data);
}

/**
 * Function to check if a point is in line of sight from an other point
 */
bool Map::IsInLineOfSight(float srcX, float srcY, float srcZ, float destX, float destY, float destZ, bool ignoreM2Model) const
{
    return VMAP::VMapFactory::createOrGetVMapManager()->isInLineOfSight(GetId(), srcX, srcY, srcZ, destX, destY, destZ, ignoreM2Model)
           && m_dyn_tree.isInLineOfSight(srcX, srcY, srcZ, destX, destY, destZ, ignoreM2Model);
}

/**
 * get the hit position and return true if we hit something (in this case the dest position will hold the hit-position)
 * otherwise the result pos will be the dest pos
 */
bool Map::GetHitPosition(float srcX, float srcY, float srcZ, float& destX, float& destY, float& destZ, float modifyDist) const
{
    // at first check all static objects
    float tempX, tempY, tempZ = 0.0f;
    bool result0 = VMAP::VMapFactory::createOrGetVMapManager()->getObjectHitPos(GetId(), srcX, srcY, srcZ, destX, destY, destZ, tempX, tempY, tempZ, modifyDist);
    if (result0)
    {
        //DEBUG_LOG("Map::GetHitPosition vmaps corrects gained with static objects! new dest coords are X:%f Y:%f Z:%f", destX, destY, destZ);
        destX = tempX;
        destY = tempY;
        destZ = tempZ;
    }
    // at second all dynamic objects, if static check has an hit, then we can calculate only to this closer point
    bool result1 = m_dyn_tree.getObjectHitPos(srcX, srcY, srcZ, destX, destY, destZ, tempX, tempY, tempZ, modifyDist);
    if (result1)
    {
        //DEBUG_LOG("Map::GetHitPosition vmaps corrects gained with dynamic objects! new dest coords are X:%f Y:%f Z:%f", destX, destY, destZ);
        destX = tempX;
        destY = tempY;
        destZ = tempZ;
    }
    return result0 || result1;
}

// Find an height within a reasonable range of provided Z. This method may fail so we have to handle that case.
bool Map::GetHeightInRange(float x, float y, float& z, float maxSearchDist /*= 4.0f*/) const
{
    float height;
    float mapHeight = INVALID_HEIGHT_VALUE;
    float vmapHeight = VMAP_INVALID_HEIGHT_VALUE;

    VMAP::IVMapManager* vmgr = VMAP::VMapFactory::createOrGetVMapManager();
    if (!vmgr->isLineOfSightCalcEnabled())
        vmgr = nullptr;

    if (vmgr)
    {
        // pure vmap search
        vmapHeight = vmgr->getHeight(i_id, x, y, z + 2.0f, maxSearchDist + 2.0f);
    }

    // find raw height from .map file on X,Y coordinates
    if (GridMap* gmap = const_cast<TerrainInfo*>(m_TerrainData)->GetGrid(x, y)) // TODO:: find a way to remove that const_cast
        mapHeight = gmap->getHeight(x, y);

    float diffMaps = fabs(fabs(z) - fabs(mapHeight));
    float diffVmaps = fabs(fabs(z) - fabs(vmapHeight));
    if (diffVmaps < maxSearchDist)
    {
        if (diffMaps < maxSearchDist)
        {
            // well we simply have to take the highest as normally there we cannot be on top of cavern is maxSearchDist is not too big
            if (vmapHeight > mapHeight || std::fabs(mapHeight - z) > std::fabs(vmapHeight - z))
                height = vmapHeight;
            else
                height = mapHeight;

            //sLog.outString("vmap %5.4f, map %5.4f, height %5.4f", vmapHeight, mapHeight, height);
        }
        else
        {
            //sLog.outString("vmap %5.4f", vmapHeight);
            height = vmapHeight;
        }
    }
    else
    {
        if (diffMaps < maxSearchDist)
        {
            //sLog.outString("map %5.4f", mapHeight);
            height = mapHeight;
        }
        else
            return false;
    }

    z = std::max<float>(height, m_dyn_tree.getHeight(x, y, height + 1.0f, maxSearchDist));
    return true;
}

float Map::GetHeight(float x, float y, float z, bool swim) const
{
    float staticHeight = m_TerrainData->GetHeightStatic(x, y, z, true, (swim ? DEFAULT_WATER_SEARCH : DEFAULT_HEIGHT_SEARCH));

    // Get Dynamic Height around static Height (if valid)
    float dynSearchHeight = 2.0f + (z < staticHeight ? staticHeight : z);
    return std::max<float>(staticHeight, m_dyn_tree.getHeight(x, y, dynSearchHeight, dynSearchHeight - staticHeight));
}

void Map::InsertGameObjectModel(const GameObjectModel& mdl)
{
    m_dyn_tree.insert(mdl);
}

void Map::RemoveGameObjectModel(const GameObjectModel& mdl)
{
    m_dyn_tree.remove(mdl);
}

bool Map::ContainsGameObjectModel(const GameObjectModel& mdl) const
{
    return m_dyn_tree.contains(mdl);
}

// This will generate a random point to all directions in water for the provided point in radius range.
bool Map::GetRandomPointUnderWater(float& x, float& y, float& z, float radius, GridMapLiquidData& liquid_status, bool randomRange/* = true*/) const
{
    const float angle = rand_norm_f() * (M_PI_F * 2.0f);
    const float range = (randomRange ? rand_norm_f() : 1.f) * radius;

    float i_x = x + range * cos(angle);
    float i_y = y + range * sin(angle);

    // get real ground of new point
    // the code consider cylinder instead of sphere for possible z
    float ground = GetHeight(i_x, i_y, z);
    if (ground > INVALID_HEIGHT) // GetHeight can fail
    {
        float min_z = z - 0.7f * radius; // 0.7 to have a bit a "flat" cylinder, TODO which value looks nicest
        if (min_z < ground)
            min_z = ground + 0.5f; // Get some space to prevent under map

        float liquidLevel = liquid_status.level - 2.0f; // just to make the generated point is in water and not on surface or a bit above

        // if not enough space to fit the creature better is to return from here
        if (min_z > liquidLevel)
            return false;

        // Mobs underwater do not move along Z axis
        //float max_z = std::max(z + 0.7f * radius, min_z);
        //max_z = std::min(max_z, liquidLevel);
        x = i_x;
        y = i_y;
        if (min_z > z)
            z = min_z;
        return true;
    }
    return false;
}

// This will generate a random point to all directions in air for the provided point in radius range.
bool Map::GetRandomPointInTheAir(float& x, float& y, float& z, float radius, bool randomRange/* = true*/) const
{
    const float angle = rand_norm_f() * (M_PI_F * 2.0f);
    const float range = (randomRange ? rand_norm_f() : 1.f) * radius;

    float i_x = x + range * cos(angle);
    float i_y = y + range * sin(angle);

    // get real ground of new point
    // the code consider cylinder instead of sphere for possible z
    float ground = GetHeight(i_x, i_y, z);
    if (ground > INVALID_HEIGHT) // GetHeight can fail
    {
        float min_z = z - 0.7f * radius; // 0.7 to have a bit a "flat" cylinder, TODO which value looks nicest
        if (min_z < ground)
            min_z = ground + 2.5f; // Get some space to prevent landing
        float max_z = std::max(z + 0.7f * radius, min_z);
        x = i_x;
        y = i_y;
        if (min_z > z)
            z = min_z;
        return true;
    }
    return false;
}

// supposed to be used for not big radius, usually less than 20.0f
bool Map::GetReachableRandomPointOnGround(float& x, float& y, float& z, float radius, bool randomRange/* = true*/) const
{
    // Generate a random range and direction for the new point
    const float angle = rand_norm_f() * (M_PI_F * 2.0f);
    const float range = (randomRange ? rand_norm_f() : 1.f) * radius;

    float i_x = x + range * cos(angle);
    float i_y = y + range * sin(angle);
    float i_z = z + 1.0f;

    GetHitPosition(x, y, z + 1.0f, i_x, i_y, i_z, -0.5f);
    i_z = z; // reset i_z to z value to avoid too much difference from original point before GetHeightInRange
    if (!GetHeightInRange(i_x, i_y, i_z)) // GetHeight can fail
        return false;

    // here we have a valid position but the point can have a big Z in some case
    // next code will check angle from 2 points
    //        c
    //       /|
    //      / |
    //    b/__|a

    // project vector to get only positive value
    float ab = fabs(x - i_x);
    float ac = fabs(z - i_z);

    // slope represented by c angle (in radian)
    const float MAX_SLOPE_IN_RADIAN = 50.0f / 180.0f * M_PI_F;  // 50(degree) max seem best value for walkable slope

    // check ab vector to avoid divide by 0
    if (ab > 0.0f)
    {
        // compute c angle and convert it from radian to degree
        float slope = atan(ac / ab);
        if (slope < MAX_SLOPE_IN_RADIAN)
        {
            x = i_x;
            y = i_y;
            z = i_z;
            return true;
        }
    }

    return false;
}

// Get random point by handling different situation depending of if the unit is flying/swimming/walking
bool Map::GetReachableRandomPosition(Unit* unit, float& x, float& y, float& z, float radius, bool randomRange/* = true*/) const
{
    float i_x = x;
    float i_y = y;
    float i_z = z;

    bool isFlying;
    bool isSwimming = true;
    switch (unit->GetTypeId())
    {
        case TYPEID_PLAYER:
            isFlying = static_cast<Player*>(unit)->IsFlying();
            break;
        case TYPEID_UNIT:
            isFlying = static_cast<Creature*>(unit)->IsFlying();
            isSwimming = static_cast<Creature*>(unit)->IsSwimming();
            break;
        default:
            sLog.outError("Map::GetReachableRandomPosition> Unsupported unit type is passed!");
            return false;
    }

    // define 1 as minimum radius for random position
    if (radius < 1.0f)
        radius = 1.0f;

    bool newDestAssigned;   // used to check if new random destination is found
    if (isFlying)
    {
        newDestAssigned = GetRandomPointInTheAir(i_x, i_y, i_z, radius, randomRange);
        /*if (newDestAssigned)
        sLog.outString("Generating air random point for %s", GetGuidStr().c_str());*/
    }
    else
    {
        GridMapLiquidData liquid_status;
        GridMapLiquidStatus res = m_TerrainData->getLiquidStatus(i_x, i_y, i_z, MAP_ALL_LIQUIDS, &liquid_status);
        if (isSwimming && (res & (LIQUID_MAP_UNDER_WATER | LIQUID_MAP_IN_WATER)))
        {
            newDestAssigned = GetRandomPointUnderWater(i_x, i_y, i_z, radius, liquid_status, randomRange);
            /*if (newDestAssigned)
            sLog.outString("Generating swim random point for %s", GetGuidStr().c_str());*/
        }
        else
        {
            newDestAssigned = GetReachableRandomPointOnGround(i_x, i_y, i_z, radius, randomRange);
            /*if (newDestAssigned)
            sLog.outString("Generating ground random point for %s", GetGuidStr().c_str());*/
        }
    }

    if (newDestAssigned)
    {
        x = i_x;
        y = i_y;
        z = i_z;
        return true;
    }

    return false;
}

bool Map::IsMountAllowed() const
{
    if (!IsDungeon())
        return true;

    if (InstanceTemplate const* data = ObjectMgr::GetInstanceTemplate(GetId()))
        return data->mountAllowed;

    return true;
}

bool Map::IsDynguidForced() const
{
    switch (GetId())
    {
        case 533: // Naxxramas
        case 543: // Hellfire Ramparts
            return true;
        default: break;
    }
    return false;
}

void Map::OnEventHappened(uint16 event_id, bool activate, bool resume)
{
    if (i_data)
        i_data->OnEventHappened(event_id, activate, resume);

    for (m_onEventNotifiedIter = m_onEventNotifiedObjects.begin(); m_onEventNotifiedIter != m_onEventNotifiedObjects.end(); ++m_onEventNotifiedIter)
        if ((*m_onEventNotifiedIter)->IsInWorld())
            (*m_onEventNotifiedIter)->OnEventHappened(event_id, activate, resume);
}

uint32 Map::SpawnedCountForEntry(uint32 entry)
{
    return m_spawnedCount[entry].size();
}

void Map::AddToSpawnCount(const ObjectGuid& guid)
{
    m_spawnedCount[guid.GetEntry()].insert(guid);
}

void Map::RemoveFromSpawnCount(const ObjectGuid& guid)
{
    m_spawnedCount[guid.GetEntry()].erase(guid);
}
