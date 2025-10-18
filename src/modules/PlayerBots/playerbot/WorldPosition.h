#pragma once
#define DT_POLYREF64 1

#include "Globals/ObjectMgr.h"
#include "Spells/SpellMgr.h"
#include "World/World.h"
#include "MotionGenerators/PathFinder.h"

class ByteBuffer;

namespace G3D
{
    class Vector2;
    class Vector3;
    class Vector4;
}

namespace ai
{
    //Constructor types for WorldPosition
    enum WorldPositionConst
    {
        WP_RANDOM = 0,
        WP_CENTROID = 1,
        WP_MEAN_CENTROID = 2,
        WP_CLOSEST = 3
    };

    template <class D, class W, class URBG>
    inline void WeightedShuffle
    (D first, D last
        , W first_weight, W last_weight
        , URBG&& g)
    {
        while (first != last && first_weight != last_weight)
        {
            std::discrete_distribution<int> dd(first_weight, last_weight);
            auto i = dd(g);

            if (i)
            {
                std::swap(*first, *std::next(first, i));
                std::swap(*first_weight, *std::next(first_weight, i));
            }
            ++first;
            ++first_weight;
        }
    }

    class GuidPosition;

    typedef std::pair<int, int> mGridPair;

    //Extension of WorldLocation with distance functions.
    class WorldPosition : public WorldLocation
    {
    public:
        //Constructors
        void add();
        void rem();
        WorldPosition() : WorldLocation(0,0,0,0,0) { add(); };
        WorldPosition(const WorldLocation& loc) : WorldLocation(loc) { add(); }
        WorldPosition(const WorldPosition& pos) : WorldLocation(pos) { add(); }
        WorldPosition(const std::string str) {char p; std::stringstream  out(str); out >> mapid >> p >> coord_x >> p >> coord_y >> p >> coord_z >> p >> orientation; add();}
        WorldPosition(const uint32 mapid, const float x, const float y, const float z = 0, float orientation = 0) : WorldLocation(mapid, x, y, z, orientation) { add(); }
        WorldPosition(const uint32 mapId, const Position& pos) : WorldLocation(mapId, pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ(), pos.GetPositionO()) { add(); }
        WorldPosition(const WorldObject* wo) { if (wo) { set(WorldLocation(wo->GetMapId(), wo->GetPositionX(), wo->GetPositionY(), wo->GetPositionZ(), wo->GetOrientation())); } add();}
        WorldPosition(const CreatureDataPair* cdPair) { if (cdPair) { set(WorldLocation(cdPair->second.mapid, cdPair->second.posX, cdPair->second.posY, cdPair->second.posZ, cdPair->second.orientation)); } add();}
        WorldPosition(const GameObjectDataPair* cdPair) { if (cdPair) { set(WorldLocation(cdPair->second.mapid, cdPair->second.posX, cdPair->second.posY, cdPair->second.posZ, cdPair->second.orientation)); } add();}
        WorldPosition(const uint32 mapId, const GuidPosition& guidP, uint32 instanceId);
        WorldPosition(const std::vector<WorldPosition*>& list, const WorldPositionConst conType);
        WorldPosition(const std::vector<WorldPosition>& list, const WorldPositionConst conType);
        WorldPosition(const uint32 mapid, const GridPair grid) : WorldLocation(mapid, (int32(grid.x_coord) - CENTER_GRID_ID - 0.5)* SIZE_OF_GRIDS + CENTER_GRID_OFFSET, (int32(grid.y_coord) - CENTER_GRID_ID - 0.5)* SIZE_OF_GRIDS + CENTER_GRID_OFFSET, 0, 0) { add(); }
        WorldPosition(const uint32 mapid, const CellPair cell) : WorldLocation(mapid, (int32(cell.x_coord) - CENTER_GRID_CELL_ID - 0.5)* SIZE_OF_GRID_CELL + CENTER_GRID_CELL_OFFSET, (int32(cell.y_coord) - CENTER_GRID_CELL_ID - 0.5)* SIZE_OF_GRID_CELL + CENTER_GRID_CELL_OFFSET, 0, 0) { add(); }
        WorldPosition(const uint32 mapid, const mGridPair grid) : WorldLocation(mapid, (32 - grid.first)* SIZE_OF_GRIDS, (32 - grid.second)* SIZE_OF_GRIDS, 0, 0) { add(); }
        WorldPosition(const SpellTargetPosition* pos) : WorldLocation(pos->target_mapId, pos->target_X, pos->target_Y, pos->target_Z) { add(); }
        WorldPosition(const TaxiNodesEntry* pos) : WorldLocation(pos->map_id, pos->x, pos->y, pos->z) { add(); }
        WorldPosition(const WorldSafeLocsEntry* pos) : WorldLocation(pos->map_id, pos->x, pos->y, pos->z, pos->o) { add(); }
        WorldPosition(const PlayerInfo* pos) : WorldLocation(pos->mapId,pos->positionX, pos->positionY, pos->positionZ, pos->orientation) { add(); }
        
        virtual ~WorldPosition()
        {
            rem();
        }

        //Setters
        void set(const WorldLocation& pos) { mapid = pos.mapid; coord_x = pos.coord_x; coord_y = pos.coord_y; coord_z = pos.coord_z; orientation = pos.orientation; }
        void set(const WorldPosition& pos) { mapid = pos.mapid; coord_x = pos.coord_x; coord_y = pos.coord_y; coord_z = pos.coord_z; orientation = pos.orientation; }
        void set(const WorldObject* wo) { set(WorldLocation(wo->GetMapId(), wo->GetPositionX(), wo->GetPositionY(), wo->GetPositionZ(), wo->GetOrientation())); }
        void set(const ObjectGuid& guid, const uint32 mapId, const uint32 instanceId);
        void setMapId(const uint32 id) { mapid = id; }
        void setX(const float x) { coord_x = x; }
        void setY(const float y) { coord_y = y; }
        void setZ(const float z) { coord_z = z; }
        void setO(const float o) {orientation = o;}

        //Operators
        operator bool() const { return  coord_x != 0 || coord_y != 0 || coord_z != 0; }
        bool operator==(const WorldPosition& p1) const { return mapid == p1.mapid && coord_x == p1.coord_x && coord_y == p1.coord_y && coord_z == p1.coord_z && orientation == p1.orientation; }
        bool operator!=(const WorldPosition& p1) const { return mapid != p1.mapid || coord_x != p1.coord_x || coord_y != p1.coord_y || coord_z != p1.coord_z || orientation != p1.orientation; }
        
        WorldPosition& operator+=(const WorldPosition& p1) { coord_x += p1.coord_x; coord_y += p1.coord_y; coord_z += p1.coord_z; return *this; }
        WorldPosition& operator-=(const WorldPosition& p1) { coord_x -= p1.coord_x; coord_y -= p1.coord_y; coord_z -= p1.coord_z; return *this; }

        WorldPosition& operator*=(const float s) { coord_x *= s; coord_y *= s; coord_z *= s; return *this; }
        WorldPosition& operator/=(const float s) { coord_x /= s; coord_y /= s; coord_z /= s; return *this; }

        WorldPosition operator+(const WorldPosition& p1) const { WorldPosition p(*this); p += p1; return p; }
        WorldPosition operator-(const WorldPosition& p1) const { WorldPosition p(*this); p -= p1; return p; }

        WorldPosition operator*(const float s) const { WorldPosition p(*this); p *= s; return p; }
        WorldPosition operator/(const float s) const { WorldPosition p(*this); p /= s; return p; }

        float operator*(const WorldPosition& p1) const { return (coord_x * coord_x) + (coord_y * coord_y) + (coord_z * coord_z); }

        //Getters
        uint32 getMapId() const { return mapid; }
        float getX() const { return coord_x; }
        float getY() const { return coord_y; }
        float getZ() const { return coord_z; }
        float getO() const { return orientation; }
        G3D::Vector3 getVector3() const;
        virtual std::string print() const;
        virtual std::string to_string() const { char p = '|'; std::stringstream out; out << mapid << p << coord_x << p << coord_y << p << coord_z << p << orientation; return out.str(); };

        static void printWKT(const std::vector<WorldPosition>& points, std::ostringstream& out, const uint32 dim = 0, const bool loop = false);
        void printWKT(std::ostringstream& out) const { printWKT({ *this }, out); }

        bool isOverworld() const { return mapid == 0 || mapid == 1 || mapid == 530 || mapid == 571 || mapid == 609; }
        bool isBg() const { return mapid == 30 || mapid == 489 || mapid == 529 || mapid == 566 || mapid == 607 || mapid == 628; }
        bool isArena() const { return mapid == 559 || mapid == 572 || mapid == 562 || mapid == 617 || mapid == 618; }
        bool isInstance() const { return !isOverworld() || mapid == 609;}
        bool isInWater() const { return getTerrain() ? getTerrain()->IsInWater(coord_x, coord_y, coord_z) : false; };
        bool isUnderWater() const { return getTerrain() ? getTerrain()->IsUnderWater(coord_x, coord_y, coord_z) : false; };

        WorldPosition relPoint(const WorldPosition& center) const { return WorldPosition(mapid, coord_x - center.coord_x, coord_y - center.coord_y, coord_z - center.coord_z, orientation); }
        WorldPosition offset(const WorldPosition& center) const { return WorldPosition(mapid, coord_x + center.coord_x, coord_y + center.coord_y, coord_z + center.coord_z, orientation); }
        float size() const { return sqrt(pow(coord_x, 2.0) + pow(coord_y, 2.0) + pow(coord_z, 2.0)); }

        //Slow distance function using possible map transfers.
        float distance(const WorldPosition& to) const;

        float fDist(const WorldPosition& to) const;

        //Returns the closest point from the list.
        WorldPosition* closest(const std::vector<WorldPosition*>& list) const { return *std::min_element(list.begin(), list.end(), [this](WorldPosition* i, WorldPosition* j) {return this->distance(*i) < this->distance(*j); }); }
        WorldPosition closest(const std::vector<WorldPosition>& list) const { return *std::min_element(list.begin(), list.end(), [this](WorldPosition i, WorldPosition j) {return this->distance(i) < this->distance(j); }); }

        WorldPosition* furtest(const std::vector<WorldPosition*>& list) const { return *std::max_element(list.begin(), list.end(), [this](WorldPosition* i, WorldPosition* j) {return this->distance(*i) < this->distance(*j); }); }
        WorldPosition furtest(const std::vector<WorldPosition>& list) const { return *std::max_element(list.begin(), list.end(), [this](WorldPosition i, WorldPosition j) {return this->distance(i) < this->distance(j); }); }

        template<class T>
        std::pair<T, WorldPosition>  closest(const std::list<std::pair<T, WorldPosition>>& list) const { return *std::min_element(list.begin(), list.end(), [this](std::pair<T, WorldPosition> i, std::pair<T, WorldPosition> j) {return this->distance(i.second) < this->distance(j.second); }); }
        template<class T>
        std::pair<T, WorldPosition> closest(const std::list<T>& list) const { return closest(GetPosList(list)); }

        template<class T>
        std::pair<T, WorldPosition>  closest(const std::vector<std::pair<T, WorldPosition>>& list) const { return *std::min_element(list.begin(), list.end(), [this](std::pair<T, WorldPosition> i, std::pair<T, WorldPosition> j) {return this->distance(i.second) < this->distance(j.second); }); }
        template<class T>
        std::pair<T, WorldPosition> closest(const std::vector<T>& list) const { return closest(GetPosVector(list)); }

        bool IsWithinDist(const WorldPosition& other, float dist2compare) const { return sqDistance(other) < dist2compare * dist2compare; }

        //Quick square distance in 2d plane.
        float sqDistance2d(const WorldPosition& to) const { return (coord_x - to.coord_x) * (coord_x - to.coord_x) + (coord_y - to.coord_y) * (coord_y - to.coord_y); };

        //Quick square distance calculation without map check. Used for getting the minimum distant points.
        float sqDistance(const WorldPosition& to) const { return (coord_x - to.coord_x) * (coord_x - to.coord_x) + (coord_y - to.coord_y) * (coord_y - to.coord_y) + (coord_z - to.coord_z) * (coord_z - to.coord_z); };

        //Returns the closest point of the list. Fast but only works for the same map.
        WorldPosition* closestSq(const std::vector<WorldPosition*>& list) const { return *std::min_element(list.begin(), list.end(), [this](WorldPosition* i, WorldPosition* j) {return sqDistance(*i) < sqDistance(*j); }); }
        WorldPosition closestSq(const std::vector<WorldPosition>& list) const { return *std::min_element(list.begin(), list.end(), [this](WorldPosition i, WorldPosition j) {return sqDistance(i) < sqDistance(j); }); }

        float getAngleTo(const WorldPosition& endPos) const { float ang = atan2(endPos.coord_y - coord_y, endPos.coord_x - coord_x); return (ang >= 0) ? ang : 2 * M_PI_F + ang; };
        float getAngleBetween(const WorldPosition& dir1, const WorldPosition& dir2) const { return abs(getAngleTo(dir1) - getAngleTo(dir2)); };

        void rotateXY(const float angle) { float nx = cos(angle) * coord_x - sin(angle) * coord_y, ny = sin(angle) * coord_x + cos(angle) * coord_y; coord_x = nx; coord_y = ny; }

        WorldPosition limit(const WorldPosition& center, const float maxDistance) { WorldPosition pos(*this); pos -= center; float size = pos.size(); if (size > maxDistance) { pos /= pos.size(); pos *= maxDistance; pos += center; } return pos; }

        WorldPosition lastInRange(const std::vector<WorldPosition>& list, const float minDist = -1, const float maxDist = -1) const;
        WorldPosition firstOutRange(const std::vector<WorldPosition>& list, const float minDist = -1, const float maxDist = -1) const;

        float mSign(const WorldPosition* p1, const WorldPosition* p2) const { return(coord_x - p2->coord_x) * (p1->coord_y - p2->coord_y) - (p1->coord_x - p2->coord_x) * (coord_y - p2->coord_y); }
        bool isInside(const WorldPosition* p1, const WorldPosition* p2, const WorldPosition* p3) const;

        void distancePartition(const std::vector<float>& distanceLimits, WorldPosition* to, std::vector<std::vector<WorldPosition*>>& partition) const;
        std::vector<std::vector<WorldPosition*>> distancePartition(const std::vector<float>& distanceLimits, std::vector<WorldPosition*> points) const;

        std::vector <WorldPosition*> GetNextPoint(std::vector<WorldPosition*> points, uint32 amount = 1) const;
        std::vector <WorldPosition> GetNextPoint(std::vector<WorldPosition> points, uint32 amount = 1) const;
        
        template<class T>
        void GetNextPoint(std::vector <std::pair<T, WorldPosition*>>& data) const
        {
            std::vector<uint32> weights;

            std::transform(data.begin(), data.end(), std::back_inserter(weights), [this](std::pair<T, WorldPosition*> point) { return 200000 / (1 + this->distance(*point.second)); });

            //If any weight is 0 add 1 to all weights.
            for (auto& w : weights)
            {
                if (w > 0)
                    continue;

                std::for_each(weights.begin(), weights.end(), [](uint32& d) { d += 1; });
                break;
            }

            std::mt19937 gen(time(0));

            WeightedShuffle(data.begin(), data.end(), weights.begin(), weights.end(), gen);
        }

        //Map functions. Player independent.
        const MapEntry* getMapEntry() const { return sMapStore.LookupEntry(mapid); }
        uint32 getFirstInstanceId() const { for (auto& map : sMapMgr.Maps()) { if (map.second->GetId() == getMapId()) return map.second->GetInstanceId(); }; return 0; }
        Map* getMap(uint32 instanceId) const { if (!*this) return nullptr; loadMapAndVMap(instanceId); return sMapMgr.FindMap(mapid, instanceId ? instanceId : (getMapEntry()->Instanceable() ? getFirstInstanceId() : 0)); }
        const TerrainInfo* getTerrain() const { return getMap(getFirstInstanceId()) ? getMap(getFirstInstanceId())->GetTerrain() : sTerrainMgr.LoadTerrain(getMapId()); }
        bool isDungeon() { return getMapEntry()->IsDungeon(); }
        float getVisibilityDistance() { return getMap(0) ? getMap(0)->GetVisibilityDistance() : (isOverworld() ? World::GetMaxVisibleDistanceOnContinents() : World::GetMaxVisibleDistanceInInstances()); }

        bool IsInStaticLineOfSight(WorldPosition pos, float heightMod = 0.5f) const;
#if defined(MANGOSBOT_TWO) || MAX_EXPANSION == 2
        bool IsInLineOfSight(WorldPosition pos, float heightMod = 0.5f) const { return mapid == pos.mapid && getMap(getFirstInstanceId()) && getMap(getFirstInstanceId())->IsInLineOfSight(coord_x, coord_y, coord_z + heightMod, pos.coord_x, pos.coord_y, pos.coord_z + heightMod, 0, true); }
        bool GetHitPosition(WorldPosition& pos) const { return getMap(getFirstInstanceId())->GetHitPosition(coord_x, coord_y, coord_z, pos.coord_x, pos.coord_y, pos.coord_z,0, 0.0f);};
#else
        bool IsInLineOfSight(WorldPosition pos, float heightMod = 0.5f) const { return mapid == pos.mapid && getMap(getFirstInstanceId()) && getMap(getFirstInstanceId())->IsInLineOfSight(coord_x, coord_y, coord_z + heightMod, pos.coord_x, pos.coord_y, pos.coord_z + heightMod, true); }
        bool GetHitPosition(WorldPosition& pos) { return getMap(getFirstInstanceId())->GetHitPosition(coord_x, coord_y, coord_z, pos.coord_x, pos.coord_y, pos.coord_z, 0.0f);};
#endif


        bool isOutside() const { WorldPosition high(*this); high.setZ(coord_z + 500.0f); return IsInLineOfSight(high); }
        bool canFly() const;

#if defined(MANGOSBOT_TWO) || MAX_EXPANSION == 2
        const float getHeight(bool swim = false) const { if(getMap(getFirstInstanceId())) return getMap(getFirstInstanceId())->GetHeight(0, coord_x, coord_y, coord_z, swim); return 0.0;}
        float GetHeightInRange(float maxSearchDist = 4.0f) const { float z = coord_z;  return getMap(getFirstInstanceId()) ? (getMap(getFirstInstanceId())->GetHeightInRange(0, coord_x, coord_y, z, maxSearchDist) ? z : coord_z) : coord_z; }
#else
        float getHeight(bool swim = false) const { return getMap(getFirstInstanceId()) ? getMap(getFirstInstanceId())->GetHeight(coord_x, coord_y, coord_z, swim) : coord_z; }
        float GetHeightInRange(float maxSearchDist = 4.0f) const { float z = coord_z;  return getMap(getFirstInstanceId()) ? (getMap(getFirstInstanceId())->GetHeightInRange(coord_x, coord_y, z, maxSearchDist) ? z : coord_z) : coord_z; }
#endif

        float currentHeight() const { return coord_z - getHeight(); }

        std::set<GenericTransport*> getTransports(uint32 entry = 0);
        void CalculatePassengerPosition(GenericTransport* transport);
        void CalculatePassengerOffset(GenericTransport* transport);

        bool isOnTransport(GenericTransport* transport);

        GridPair getGridPair() const { return MaNGOS::ComputeGridPair(coord_x, coord_y); };
        std::vector<GridPair> getGridPairs(const WorldPosition& secondPos) const;
        static std::vector<WorldPosition> fromGridPair(const GridPair& gridPair, uint32 mapId);

        CellPair getCellPair() const { return MaNGOS::ComputeCellPair(coord_x, coord_y); }
        std::vector<WorldPosition> fromCellPair(const CellPair& cellPair) const;
        std::vector<WorldPosition> gridFromCellPair(const CellPair& cellPair) const;

        mGridPair getmGridPair() const {
            return std::make_pair((int)(32 - coord_x / SIZE_OF_GRIDS), (int)(32 - coord_y / SIZE_OF_GRIDS)); }

        std::vector<mGridPair> getmGridPairs(const WorldPosition& secondPos) const;
        static std::vector<WorldPosition> frommGridPair(const mGridPair& gridPair, uint32 mapId);

        static bool isVmapLoaded(uint32 mapId, int x, int y);

        bool isVmapLoaded() const { return isVmapLoaded(getMapId(), getmGridPair().first, getmGridPair().second); }

        static bool isMmapLoaded(uint32 mapId, uint32 instanceId, int x, int y);

        bool isMmapLoaded(uint32 instanceId) const { return isMmapLoaded(getMapId(), instanceId, getmGridPair().first, getmGridPair().second); }

        static bool loadMapAndVMap(uint32 mapId, uint32 instanceId, int x, int y);
        bool loadMapAndVMap(uint32 instanceId) const {return loadMapAndVMap(getMapId(), instanceId, getmGridPair().first, getmGridPair().second); }
        void loadMapAndVMaps(const WorldPosition& secondPos, uint32 instanceId) const;
        static void unloadMapAndVMaps(uint32 mapId);

        static bool loadVMap(uint32 mapId, int x, int y);
        bool loadVMap() const { return loadVMap(getMapId(), getmGridPair().first, getmGridPair().second); }

        //Display functions
        WorldPosition getDisplayLocation() const;
        float getDisplayX() const { return getDisplayLocation().coord_y * -1.0; }
        float getDisplayY() const { return getDisplayLocation().coord_x; }

        bool isValid() const { return MaNGOS::IsValidMapCoord(coord_x, coord_y, coord_z, orientation); };
        virtual uint16 getAreaFlag() const {
            loadVMap();
            return isValid() && isVmapLoaded() ? sTerrainMgr.GetAreaFlag(getMapId(), coord_x, coord_y, coord_z) : 0; };
        AreaTableEntry const* GetArea() const;
        std::string getAreaName(const bool fullName = true, const bool zoneName = false) const;
        std::string getAreaOverride() const { if (!getTerrain()) return "";  AreaNameInfo nameInfo = getTerrain()->GetAreaName(coord_x, coord_y, coord_z, 0); return nameInfo.wmoNameOverride ? nameInfo.wmoNameOverride : ""; }
        int32 getAreaLevel() const;

        bool HasAreaFlag(const AreaFlags flag = AREA_FLAG_CAPITAL) const;
        bool HasFaction(const Team team) const;

        std::vector<WorldPosition> fromPointsArray(const std::vector<G3D::Vector3>& path) const;

        //Pathfinding
        std::vector<WorldPosition> getPathStepFrom(const WorldPosition& startPos, std::unique_ptr<PathFinder>& pathfinder, const Unit* bot, bool forceNormalPath = false) const;
        std::vector<WorldPosition> getPathStepFrom(const WorldPosition& startPos, const Unit* bot, bool forceNormalPath = false) const;
        std::vector<WorldPosition> getPathFromPath(const std::vector<WorldPosition>& startPath, const Unit* bot, const uint8 maxAttempt = 40) const;
        std::vector<WorldPosition> getPathFrom(const WorldPosition& startPos, const Unit* bot) { return getPathFromPath({ startPos }, bot); };
        std::vector<WorldPosition> getPathTo(WorldPosition endPos, const Unit* bot) const { return endPos.getPathFrom(*this, bot); }
        bool isPathTo(const std::vector<WorldPosition>& path, float const maxDistance = 0) const;
        bool cropPathTo(std::vector<WorldPosition>& path, const float maxDistance = 0) const;
        bool canPathTo(const WorldPosition& endPos, const Unit* bot) const { return endPos.isPathTo(getPathTo(endPos, bot)); }

        float getPathLength(const std::vector<WorldPosition>& points) const { float dist = 0.0f; for (auto& p : points) if (&p == &points.front()) dist = 0; else dist += std::prev(&p, 1)->distance(p); return dist; }

        bool ClosestCorrectPoint(float maxRange, float maxHeight = 5.0f, uint32 instanceId = 0);
        bool GetReachableRandomPointOnGround(const Player* bot, const float radius, const bool randomRange = true); //Generic terrain.
        std::vector<WorldPosition> ComputePathToRandomPoint(const Player* bot, const float radius, const bool randomRange = true); //For use with transports.

        uint32 getUnitsAggro(const std::list<ObjectGuid>& units, const Player* bot) const;

        //Creatures
        std::vector<CreatureDataPair const*> getCreaturesNear(const float radius = 0, const uint32 entry = 0) const;
        //GameObjects
        std::vector<GameObjectDataPair const*> getGameObjectsNear(const float radius = 0, const uint32 entry = 0) const;
    };

    inline ByteBuffer& operator<<(ByteBuffer& b, WorldPosition& guidP)
    {
        b << guidP.getMapId();
        b << guidP.coord_x;
        b << guidP.coord_y;
        b << guidP.coord_z;
        b << guidP.orientation;
        return b;
    }

    inline ByteBuffer& operator>>(ByteBuffer& b, WorldPosition& g)
    {
        uint32 mapid;
        float coord_x;
        float coord_y;
        float coord_z;
        float orientation;
        b >> mapid;
        b >> coord_x;
        b >> coord_y;
        b >> coord_z;
        b >> orientation;

        return b;
    }

    //Generic creature finder
    class FindPointCreatureData
    {
    public:
        FindPointCreatureData(WorldPosition point1 = WorldPosition(), float radius1 = 0, uint32 entry1 = 0) { point = point1; radius = radius1; entry = entry1; }

        bool operator()(CreatureDataPair const& dataPair);
        std::vector<CreatureDataPair const*> GetResult() const { return data; };
    private:
        WorldPosition point;
        float radius;
        uint32 entry;

        std::vector<CreatureDataPair const*> data;
    };

    //Generic gameObject finder
    class FindPointGameObjectData
    {
    public:
        FindPointGameObjectData(WorldPosition point1 = WorldPosition(), float radius1 = 0, uint32 entry1 = 0) { point = point1; radius = radius1; entry = entry1; }

        bool operator()(GameObjectDataPair const& dataPair);
        std::vector<GameObjectDataPair const*> GetResult() const { return data; };
    private:
        WorldPosition point;
        float radius;
        uint32 entry;

        std::vector<GameObjectDataPair const*> data;
    };    
}
