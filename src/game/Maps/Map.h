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

#ifndef MANGOS_MAP_H
#define MANGOS_MAP_H

#include "Common.h"
#include "Platform/Define.h"
#include "Policies/ThreadingModel.h"

#include "Server/DBCStructure.h"
#include "Maps/GridDefines.h"
#include "Grids/Cell.h"
#include "Entities/Object.h"
#include "Globals/SharedDefines.h"
#include "Maps/GridMap.h"
#include "GameSystem/GridRefManager.h"
#include "MapRefManager.h"
#include "DBScripts/ScriptMgr.h"
#include "Entities/CreatureLinkingMgr.h"
#include "vmap/DynamicTree.h"

#include <bitset>
#include <functional>
#include <list>

struct CreatureInfo;
class Creature;
class Unit;
class WorldPacket;
class InstanceData;
class Group;
class MapPersistentState;
class WorldPersistentState;
class DungeonPersistentState;
class BattleGroundPersistentState;
struct ScriptInfo;
class BattleGround;
class GridMap;
class GameObjectModel;
class WeatherSystem;
namespace MaNGOS { struct ObjectUpdater; }

// GCC have alternative #pragma pack(N) syntax and old gcc version not support pack(push,N), also any gcc version not support it at some platform
#if defined( __GNUC__ )
#pragma pack(1)
#else
#pragma pack(push,1)
#endif

struct InstanceTemplate
{
    uint32 map;                                             // instance map
    uint32 parent;                                          // non-continent parent instance (for instance with entrance in another instances)
    // or 0 (not related to continent 0 map id)
    uint32 levelMin;
    uint32 levelMax;
    uint32 maxPlayers;
    uint32 reset_delay;                                     // in days
    int32 ghostEntranceMap;                                 // < 0 if not entrance coordinates
    float ghostEntranceX;
    float ghostEntranceY;
    uint32 script_id;
    bool   mountAllowed;
};

struct WorldTemplate
{
    uint32 map;                                             // non-instance map
    uint32 script_id;
};

#if defined( __GNUC__ )
#pragma pack()
#else
#pragma pack(pop)
#endif

#define MIN_UNLOAD_DELAY      1                             // immediate unload

class Map : public GridRefManager<NGridType>
{
        friend class MapReference;
        friend class ObjectGridLoader;
        friend class ObjectWorldLoader;

    protected:
        Map(uint32 id, time_t, uint32 InstanceId);
    public:
        virtual ~Map();

        // currently unused for normal maps
        bool CanUnload(uint32 diff)
        {
            if (!m_unloadTimer) return false;
            if (m_unloadTimer <= diff) return true;
            m_unloadTimer -= diff;
            return false;
        }

        virtual void Initialize(bool loadInstanceData = true);

        virtual bool Add(Player*);
        virtual void Remove(Player*, bool);
        template<class T> void Add(T*);
        template<class T> void Remove(T*, bool);

        static void DeleteFromWorld(Player* pl);        // player object will deleted at call

        void VisitNearbyCellsOf(WorldObject* obj, TypeContainerVisitor<MaNGOS::ObjectUpdater, GridTypeMapContainer> &gridVisitor, TypeContainerVisitor<MaNGOS::ObjectUpdater, WorldTypeMapContainer> &worldVisitor);
        virtual void Update(const uint32&);

        void MessageBroadcast(Player const*, WorldPacket const&, bool to_self);
        void MessageBroadcast(WorldObject const*, WorldPacket const&);
        void MessageDistBroadcast(Player const*, WorldPacket const&, float dist, bool to_self, bool own_team_only = false);
        void MessageDistBroadcast(WorldObject const*, WorldPacket const&, float dist);
        void MessageMapBroadcast(WorldObject const* obj, WorldPacket const& msg);
        void MessageMapBroadcastZone(WorldObject const* obj, WorldPacket const& msg, uint32 zoneId);
        void MessageMapBroadcastArea(WorldObject const* obj, WorldPacket const& msg, uint32 areaId);

        void ExecuteDistWorker(WorldObject const* obj, float dist, std::function<void(Player*)> const& worker);
        void ExecuteMapWorker(std::function<void(Player*)> const& worker);
        void ExecuteMapWorkerZone(uint32 zoneId, std::function<void(Player*)> const& worker);
        void ExecuteMapWorkerArea(uint32 areaId, std::function<void(Player*)> const& worker);

        float GetVisibilityDistance() const { return m_VisibleDistance; }
        // function for setting up visibility distance for maps on per-type/per-Id basis
        virtual void InitVisibilityDistance();

        void PlayerRelocation(Player*, float x, float y, float z, float orientation);
        void CreatureRelocation(Creature* creature, float x, float y, float z, float ang);

        template<class T, class CONTAINER> void Visit(const Cell& cell, TypeContainerVisitor<T, CONTAINER>& visitor);

        bool IsRemovalGrid(float x, float y) const
        {
            GridPair p = MaNGOS::ComputeGridPair(x, y);
            return (!getNGrid(p.x_coord, p.y_coord) || getNGrid(p.x_coord, p.y_coord)->GetGridState() == GRID_STATE_REMOVAL);
        }

        bool IsLoaded(float x, float y) const
        {
            GridPair p = MaNGOS::ComputeGridPair(x, y);
            return loaded(p);
        }

        bool GetUnloadLock(const GridPair& p) const { return getNGrid(p.x_coord, p.y_coord)->getUnloadLock(); }
        void SetUnloadLock(const GridPair& p, bool on) { getNGrid(p.x_coord, p.y_coord)->setUnloadExplicitLock(on); }
        void ForceLoadGrid(float x, float y);
        bool UnloadGrid(const uint32& x, const uint32& y, bool pForce);
        virtual void UnloadAll(bool pForce);

        void ResetGridExpiry(NGridType& grid, float factor = 1) const
        {
            grid.ResetTimeTracker((time_t)((float)i_gridExpiry * factor));
        }

        time_t GetGridExpiry(void) const { return i_gridExpiry; }
        uint32 GetId(void) const { return i_id; }

        // some calls like isInWater should not use vmaps due to processor power
        // can return INVALID_HEIGHT if under z+2 z coord not found height

        virtual void RemoveAllObjectsInRemoveList();

        bool CreatureRespawnRelocation(Creature* c);        // used only in CreatureRelocation and ObjectGridUnloader

        uint32 GetInstanceId() const { return i_InstanceId; }
        virtual bool CanEnter(Player* player);
        const char* GetMapName() const;

        bool Instanceable() const { return i_mapEntry && i_mapEntry->Instanceable(); }
        bool IsDungeon() const { return i_mapEntry && i_mapEntry->IsDungeon(); }
        bool IsRaid() const { return i_mapEntry && i_mapEntry->IsRaid(); }
        bool IsNoRaid() const { return i_mapEntry && i_mapEntry->IsNonRaidDungeon(); }
        bool IsBattleGround() const { return i_mapEntry && i_mapEntry->IsBattleGround(); }
        bool IsContinent() const { return i_mapEntry && i_mapEntry->IsContinent(); }
        bool IsMountAllowed() const;

        // can't be nullptr for loaded map
        MapPersistentState* GetPersistentState() const { return m_persistentState; }

        void AddObjectToRemoveList(WorldObject* obj);

        void UpdateObjectVisibility(WorldObject* obj, Cell cell, const CellPair& cellpair);

        void resetMarkedCells() { marked_cells.reset(); }
        bool isCellMarked(uint32 pCellId) const { return marked_cells.test(pCellId); }
        void markCell(uint32 pCellId) { marked_cells.set(pCellId); }

        bool HavePlayers() const { return !m_mapRefManager.isEmpty(); }
        uint32 GetPlayersCountExceptGMs() const;
        bool ActiveObjectsNearGrid(uint32 x, uint32 y) const;

        /// Send a Packet to all players on a map
        void SendToPlayers(WorldPacket const& data) const;
        /// Send a Packet to all players in a zone. Return false if no player found
        bool SendToPlayersInZone(WorldPacket const& data, uint32 zoneId) const;

        typedef MapRefManager PlayerList;
        PlayerList const& GetPlayers() const { return m_mapRefManager; }

        // per-map script storage
        enum ScriptExecutionParam
        {
            SCRIPT_EXEC_PARAM_NONE                    = 0x00,   // Start regardless if already started
            SCRIPT_EXEC_PARAM_UNIQUE_BY_SOURCE        = 0x01,   // Start Script only if not yet started (uniqueness identified by id and source)
            SCRIPT_EXEC_PARAM_UNIQUE_BY_TARGET        = 0x02,   // Start Script only if not yet started (uniqueness identified by id and target)
            SCRIPT_EXEC_PARAM_UNIQUE_BY_SOURCE_TARGET = 0x03,   // Start Script only if not yet started (uniqueness identified by id, source and target)
        };
        bool ScriptsStart(ScriptMapMapName const& scripts, uint32 id, Object* source, Object* target, ScriptExecutionParam execParams = SCRIPT_EXEC_PARAM_NONE);
        void ScriptCommandStart(ScriptInfo const& script, uint32 delay, Object* source, Object* target);

        // must called with AddToWorld
        void AddToActive(WorldObject* obj);
        // must called with RemoveFromWorld
        void RemoveFromActive(WorldObject* obj);

        // Game Event notification system
        void AddToOnEventNotified(WorldObject* obj);
        void RemoveFromOnEventNotified(WorldObject* obj);
        void OnEventHappened(uint16 event_id, bool activate, bool resume);

        Player* GetPlayer(ObjectGuid guid);
        Creature* GetCreature(ObjectGuid guid);
        Creature* GetCreatureByEntry(uint32 entry);
        Pet* GetPet(ObjectGuid guid);
        Creature* GetAnyTypeCreature(ObjectGuid guid);      // normal creature or pet
        GameObject* GetGameObject(ObjectGuid guid);
        DynamicObject* GetDynamicObject(ObjectGuid guid);
        Corpse* GetCorpse(ObjectGuid guid) const;                 // !!! find corpse can be not in world
        Unit* GetUnit(ObjectGuid guid);                     // only use if sure that need objects at current map, specially for player case
        WorldObject* GetWorldObject(ObjectGuid guid);       // only use if sure that need objects at current map, specially for player case

        typedef TypeUnorderedMapContainer<AllMapStoredObjectTypes, ObjectGuid> MapStoredObjectTypesContainer;
        MapStoredObjectTypesContainer& GetObjectsStore() { return m_objectsStore; }
        std::map<uint32, uint32>& GetTempCreatures() { return m_tempCreatures; }
        std::map<uint32, uint32>& GetTempPets() { return m_tempPets; }

        void AddUpdateObject(Object* obj)
        {
            i_objectsToClientUpdate.insert(obj);
        }

        void RemoveUpdateObject(Object* obj)
        {
            i_objectsToClientUpdate.erase(obj);
        }

        // DynObjects currently
        uint32 GenerateLocalLowGuid(HighGuid guidhigh);

        // get corresponding TerrainData object for this particular map
        const TerrainInfo* GetTerrain() const { return m_TerrainData; }

        void CreateInstanceData(bool load);
        InstanceData* GetInstanceData() const { return i_data; }
        uint32 GetScriptId() const { return i_script_id; }

        void MonsterYellToMap(ObjectGuid guid, int32 textId, ChatMsg chatMsg, Language language, Unit const* target) const;
        void MonsterYellToMap(CreatureInfo const* cinfo, int32 textId, ChatMsg chatMsg, Language language, Unit const* target, uint32 senderLowGuid = 0) const;
        void PlayDirectSoundToMap(uint32 soundId, uint32 zoneId = 0) const;

        // Dynamic VMaps
        float GetHeight(float x, float y, float z) const;
        bool GetHeightInRange(float x, float y, float& z, float maxSearchDist = 4.0f) const;
        bool IsInLineOfSight(float x1, float y1, float z1, float x2, float y2, float z2, bool ignoreM2Model) const;
        bool GetHitPosition(float srcX, float srcY, float srcZ, float& destX, float& destY, float& destZ, float modifyDist) const;

        // Object Model insertion/remove/test for dynamic vmaps use
        void InsertGameObjectModel(const GameObjectModel& mdl);
        void RemoveGameObjectModel(const GameObjectModel& mdl);
        bool ContainsGameObjectModel(const GameObjectModel& mdl) const;

        // Get Holder for Creature Linking
        CreatureLinkingHolder* GetCreatureLinkingHolder() { return &m_creatureLinkingHolder; }

        // Teleport all players in that map to choosed location
        void TeleportAllPlayersTo(TeleportLocation loc);

        // WeatherSystem
        WeatherSystem* GetWeatherSystem() const { return m_weatherSystem; }
        /** Set the weather in a zone on this map
         * @param zoneId set the weather for which zone
         * @param type What weather to set
         * @param grade how strong the weather should be
         * @param permanently set the weather permanently?
         */
        void SetWeather(uint32 zoneId, WeatherType type, float grade, bool permanently);

        // Random on map generation
        bool GetReachableRandomPosition(Unit* unit, float& x, float& y, float& z, float radius) const;
        bool GetReachableRandomPointOnGround(float& x, float& y, float& z, float radius) const;
        bool GetRandomPointInTheAir(float& x, float& y, float& z, float radius) const;
        bool GetRandomPointUnderWater(float& x, float& y, float& z, float radius, GridMapLiquidData& liquid_status) const;

        void AddMessage(const std::function<void(Map*)>& message);

        uint32 SpawnedCountForEntry(uint32 entry);
        void AddToSpawnCount(const ObjectGuid& guid);
        void RemoveFromSpawnCount(const ObjectGuid& guid);

        uint32 GetUpdateTimeMin() { return m_updateTimeMin; }
        uint32 GetUpdateTimeMax() { return m_updateTimeMax; }
        uint32 GetUpdateTimeAvg() { return uint32(m_updateTimeTotal / m_cycleCounter); }

        uint32 GetCurrentMSTime() const;
        TimePoint GetCurrentClockTime() const;
        uint32 GetCurrentDiff() const;

        void CreatePlayerOnClient(Player* player);

        uint32 GetLoadedGridsCount();

    private:
        void LoadMapAndVMap(int gx, int gy);

        void SetTimer(uint32 t) { i_gridExpiry = t < MIN_GRID_DELAY ? MIN_GRID_DELAY : t; }

        void SendInitSelf(Player* player) const;

        void SendInitTransports(Player* player) const;
        void SendRemoveTransports(Player* player) const;

        bool CreatureCellRelocation(Creature* c, const Cell& new_cell);

        bool loaded(const GridPair&) const;
        void EnsureGridCreated(const GridPair&);
        bool EnsureGridLoaded(Cell const&);
        void EnsureGridLoadedAtEnter(Cell const&, Player* player = nullptr);

        void buildNGridLinkage(NGridType* pNGridType) { pNGridType->link(this); }

        NGridType* getNGrid(uint32 x, uint32 y) const
        {
            MANGOS_ASSERT(x < MAX_NUMBER_OF_GRIDS);
            MANGOS_ASSERT(y < MAX_NUMBER_OF_GRIDS);
            return i_grids[x][y];
        }

        bool isGridObjectDataLoaded(uint32 x, uint32 y) const { return getNGrid(x, y)->isGridObjectDataLoaded(); }
        void setGridObjectDataLoaded(bool pLoaded, uint32 x, uint32 y) { getNGrid(x, y)->setGridObjectDataLoaded(pLoaded); }

        void setNGrid(NGridType* grid, uint32 x, uint32 y);
        void ScriptsProcess();

        void SendObjectUpdates();
        std::set<Object*> i_objectsToClientUpdate;

    protected:
        MapEntry const* i_mapEntry;
        uint32 i_id;
        uint32 i_InstanceId;
        uint32 m_unloadTimer;
        float m_VisibleDistance;
        MapPersistentState* m_persistentState;

        MapRefManager m_mapRefManager;
        MapRefManager::iterator m_mapRefIter;

        typedef WorldObjectSet ActiveNonPlayers;
        ActiveNonPlayers m_activeNonPlayers;
        ActiveNonPlayers::iterator m_activeNonPlayersIter;
        MapStoredObjectTypesContainer m_objectsStore;
        std::map<uint32, uint32> m_tempCreatures;
        std::map<uint32, uint32> m_tempPets;

        std::vector<std::function<void(Map*)>> m_messageVector;
        std::mutex m_messageMutex;

        WorldObjectSet m_onEventNotifiedObjects;
        WorldObjectSet::iterator m_onEventNotifiedIter;

    private:
        time_t i_gridExpiry;

        NGridType* i_grids[MAX_NUMBER_OF_GRIDS][MAX_NUMBER_OF_GRIDS];

        // Shared geodata object with map coord info...
        TerrainInfo* const m_TerrainData;
        bool m_bLoadedGrids[MAX_NUMBER_OF_GRIDS][MAX_NUMBER_OF_GRIDS];

        std::bitset<TOTAL_NUMBER_OF_CELLS_PER_MAP* TOTAL_NUMBER_OF_CELLS_PER_MAP> marked_cells;

        WorldObjectSet i_objectsToRemove;

        typedef std::multimap<time_t, ScriptAction> ScriptScheduleMap;
        ScriptScheduleMap m_scriptSchedule;

        InstanceData* i_data;
        uint32 i_script_id;

        // Map local low guid counters
        ObjectGuidGenerator<HIGHGUID_UNIT> m_CreatureGuids;
        ObjectGuidGenerator<HIGHGUID_GAMEOBJECT> m_GameObjectGuids;
        ObjectGuidGenerator<HIGHGUID_DYNAMICOBJECT> m_DynObjectGuids;
        ObjectGuidGenerator<HIGHGUID_PET> m_PetGuids;

        // Type specific code for add/remove to/from grid
        template<class T>
        void AddToGrid(T*, NGridType*, Cell const&);

        template<class T>
        void RemoveFromGrid(T*, NGridType*, Cell const&);
        // Holder for information about linked mobs
        CreatureLinkingHolder m_creatureLinkingHolder;

        // Dynamic Map tree object
        DynamicMapTree m_dyn_tree;

        // WeatherSystem
        WeatherSystem* m_weatherSystem;

        std::unordered_map<uint32, std::set<ObjectGuid>> m_spawnedCount;

        // Map update performance logging
        std::atomic<uint32> m_cycleCounter;
        std::atomic<uint32> m_updateTimeMin;
        std::atomic<uint32> m_updateTimeMax;
        std::atomic<uint64> m_updateTimeTotal;
};

class WorldMap : public Map
{
    private:
        using Map::GetPersistentState;                      // hide in subclass for overwrite
    public:
        WorldMap(uint32 id, time_t expiry) : Map(id, expiry, 0) {}
        ~WorldMap() {}

        // can't be nullptr for loaded map
        WorldPersistentState* GetPersistanceState() const;
};

class DungeonMap : public Map
{
    private:
        using Map::GetPersistentState;                      // hide in subclass for overwrite
    public:
        DungeonMap(uint32 id, time_t, uint32 InstanceId);
        ~DungeonMap();
        bool Add(Player*) override;
        void Remove(Player*, bool) override;
        void Update(const uint32&) override;
        bool Reset(InstanceResetMethod method);
        void PermBindAllPlayers(Player* player);
        void UnloadAll(bool pForce) override;
        void SendResetWarnings(uint32 timeLeft) const;
        void SetResetSchedule(bool on);
        uint32 GetMaxPlayers() const;

        Team GetInstanceTeam() { return m_team; };

        // can't be nullptr for loaded map
        DungeonPersistentState* GetPersistanceState() const;

        virtual void InitVisibilityDistance() override;
    private:
        bool m_resetAfterUnload;
        bool m_unloadWhenEmpty;
        Team m_team;
};

class BattleGroundMap : public Map
{
    private:
        using Map::GetPersistentState;                      // hide in subclass for overwrite
    public:
        BattleGroundMap(uint32 id, time_t, uint32 InstanceId);
        ~BattleGroundMap();

        virtual void Initialize(bool) override;
        void Update(const uint32&) override;
        bool Add(Player*) override;
        void Remove(Player*, bool) override;
        bool CanEnter(Player* player) override;
        void SetUnload();
        void UnloadAll(bool pForce) override;

        virtual void InitVisibilityDistance() override;
        BattleGround* GetBG() const { return m_bg; }
        void SetBG(BattleGround* bg) { m_bg = bg; }

        // can't be nullptr for loaded map
        BattleGroundPersistentState* GetPersistanceState() const;

    private:
        BattleGround* m_bg;
};

template<class T, class CONTAINER>
inline void
Map::Visit(const Cell& cell, TypeContainerVisitor<T, CONTAINER>& visitor)
{
    const uint32 x = cell.GridX();
    const uint32 y = cell.GridY();
    const uint32 cell_x = cell.CellX();
    const uint32 cell_y = cell.CellY();

    if (!cell.NoCreate() || loaded(GridPair(x, y)))
    {
        EnsureGridLoaded(cell);
        getNGrid(x, y)->Visit(cell_x, cell_y, visitor);
    }
}
#endif
