#include "WorldPosition.h"
#include "GuidPosition.h"
#include "playerbot/TravelMgr.h"
#include "TravelNode.h"

#include "Maps/Map.h"

#include "World/World.h"
#include "Grids/CellImpl.h"
#include "Globals/ObjectAccessor.h"
#include "Entities/Transports.h"
#include "MemoryMonitor.h"

#include "MotionGenerators/MoveMap.h"

#ifdef MANGOSBOT_TWO
#include "Vmap/VMapFactory.h"
#else
#include "vmap/VMapFactory.h"
#endif

#include <numeric>
#include <iomanip>

using namespace ai;
using namespace MaNGOS;

void WorldPosition::add()
{
#ifdef MEMORY_MONITOR
    sMemoryMonitor.Add("WorldPosition", (int64)this);
#endif
}

void WorldPosition::rem()
{
#ifdef MEMORY_MONITOR
    sMemoryMonitor.Rem("WorldPosition", (int64)this);
#endif
}

WorldPosition::WorldPosition(const uint32 mapId, const GuidPosition& guidP, uint32 instanceId)
{
    if (guidP.mapid !=0 || guidP.coord_x != 0 || guidP.coord_y != 0 || guidP.coord_z !=0) {
        set(WorldPosition(guidP.mapid, guidP.coord_x, guidP.coord_y, guidP.coord_z, guidP.orientation));
        return;
    }

    set(ObjectGuid(guidP), guidP.mapid, instanceId);

    add();
 }

void WorldPosition::set(const ObjectGuid& guid, const uint32 mapId, const uint32 instanceId)
{
    switch (guid.GetHigh())
    {
    case HIGHGUID_PLAYER:
    {
        Player* player = sObjectAccessor.FindPlayer(guid);
        if (player)
        {
            set(player);
            return;
        }
        break;
    }
    case HIGHGUID_GAMEOBJECT:
    {
        GameObjectDataPair const* gpair = sObjectMgr.GetGODataPair(guid.GetCounter());
        if (gpair)
        {
            set(gpair);
            return;
        }
        break;
    }
    case HIGHGUID_UNIT:
#ifdef MANGOSBOT_TWO
    case HIGHGUID_VEHICLE:
#endif
    {
        setMapId(mapId);
        setX(1); //Pretend to know so map can be loaded.
        if (Map* map = getMap(instanceId))
        {
            Creature* creature = map->GetAnyTypeCreature(guid);
            if (creature)
            {
                set(creature);
                return;
            }
        }

        CreatureDataPair const* cpair = sObjectMgr.GetCreatureDataPair(guid.GetCounter());
        if (cpair)
        {
            set(cpair);
            return;
        }
        break;
    }
    case HIGHGUID_TRANSPORT:
    case HIGHGUID_MO_TRANSPORT:
    case HIGHGUID_ITEM:
    case HIGHGUID_PET:
    case HIGHGUID_DYNAMICOBJECT:
    case HIGHGUID_CORPSE:
        set(WorldPosition());
        return;
    }

    set(WorldPosition());
}

WorldPosition::WorldPosition(const std::vector<WorldPosition*>& list, const WorldPositionConst conType)
{
    uint32 size = list.size();
    if (size == 0)
        return;
    else if (size == 1)
        set(*list.front());
    else if (conType == WP_RANDOM)
        set(*list[urand(0, size - 1)]);
    else if (conType == WP_CENTROID)
        set(std::accumulate(list.begin(), list.end(), WorldLocation(list[0]->getMapId(), 0, 0, 0, 0), [size](WorldLocation i, WorldPosition* j) {i.coord_x += j->getX() / size; i.coord_y += j->getY() / size; i.coord_z += j->getZ() / size; i.orientation += j->getO() / size; return i; }));
    else if (conType == WP_MEAN_CENTROID)
    {
        WorldPosition pos = WorldPosition(list, WP_CENTROID);
        set(*pos.closestSq(list));
    }

    add();
}

WorldPosition::WorldPosition(const std::vector<WorldPosition>& list, const WorldPositionConst conType)
{    
    uint32 size = list.size();
    if (size == 0)
        return;
    else if (size == 1)
        set(list.front());
    else if (conType == WP_RANDOM)
        set(list[urand(0, size - 1)]);
    else if (conType == WP_CENTROID)
        set(std::accumulate(list.begin(), list.end(), WorldLocation(list[0].getMapId(), 0, 0, 0, 0), [size](WorldLocation i, WorldPosition j) {i.coord_x += j.getX() / size; i.coord_y += j.getY() / size; i.coord_z += j.getZ() / size; i.orientation += j.getO() / size; return i; }));
    else if (conType == WP_MEAN_CENTROID)
    {
        WorldPosition pos = WorldPosition(list, WP_CENTROID);
        set(pos.closestSq(list));
    }    

    add();
}

float WorldPosition::distance(const WorldPosition& to) const
{
    if(mapid == to.getMapId())
        return relPoint(to).size();

    //this -> mapTransfer | mapTransfer -> center
    return sTravelMgr.MapTransDistance(*this, to);
};

float WorldPosition::fDist(const WorldPosition& to) const
{
    if (mapid == to.getMapId())
        return sqrt(sqDistance2d(to));

    //this -> mapTransfer | mapTransfer -> center
    return sTravelMgr.MapTransDistance(*this, to);
};

//When moving from this along list return last point that falls within range.
//Distance is move distance along path.
WorldPosition WorldPosition::lastInRange(const std::vector<WorldPosition>& list, const float minDist, const float maxDist) const
{
    WorldPosition rPoint;

    float startDist = 0.0f;

    //Enter the path at the closest point.
    for (auto& p : list)
    {
        float curDist = distance(p);
        if (startDist < curDist || p == list.front())
            startDist = curDist + 0.1f;
    }

    float totalDist = 0.0f;

    //Follow the path from the last nearest point
    //Return last point in range.
    for (auto& p : list)
    {
        float curDist = distance(p);

        if (totalDist > 0) //We have started the path. Keep counting.
            totalDist += p.distance(*std::prev(&p, 1));

        if (curDist == startDist) //Start the path here.
            totalDist = startDist;

        if (minDist > 0 && totalDist < minDist)
            continue;

        if (maxDist > 0 && totalDist > maxDist)
            continue; //We do not break here because the path may loop back and have a second startDist point.

        rPoint = p;
    }

    return rPoint;
};

//Todo: remove or adjust to above standard.
WorldPosition WorldPosition::firstOutRange(const std::vector<WorldPosition>& list, const float minDist, const float maxDist) const
{
    WorldPosition rPoint;

    for (auto& p : list)
    {
        if (minDist > 0 && distance(p) < minDist)
            return p;

        if (maxDist > 0 && distance(p) > maxDist)
            return p;

        rPoint = p;
    }

    return rPoint;
}

//Returns true if (on the x-y plane) the position is inside the three points.
bool WorldPosition::isInside(const WorldPosition* p1, const WorldPosition* p2, const WorldPosition* p3) const
{
    if (getMapId() != p1->getMapId() != p2->getMapId() != p3->getMapId())
        return false;

    float d1, d2, d3;
    bool has_neg, has_pos;

    d1 = mSign(p1, p2);
    d2 = mSign(p2, p3);
    d3 = mSign(p3, p1);

    has_neg = (d1 < 0) || (d2 < 0) || (d3 < 0);
    has_pos = (d1 > 0) || (d2 > 0) || (d3 > 0);

    return !(has_neg && has_pos);
}

void WorldPosition::distancePartition(const std::vector<float>& distanceLimits, WorldPosition* to, std::vector<std::vector<WorldPosition*>>& partitions) const
{
    float dist = distance(*to);

    for (uint8 l = 0; l < distanceLimits.size(); l++)
        if (dist <= distanceLimits[l])
            partitions[l].push_back(to);
}

std::vector<std::vector<WorldPosition*>> WorldPosition::distancePartition(const std::vector<float>& distanceLimits, std::vector<WorldPosition*> points) const
{
    std::vector<std::vector<WorldPosition*>> partitions;

    for (auto lim : distanceLimits)
        partitions.push_back({});

    for (auto& point : points)
    {
        distancePartition(distanceLimits, point, partitions);
    }

    return partitions;
}

std::vector<WorldPosition*> WorldPosition::GetNextPoint(std::vector<WorldPosition*> points, uint32 amount) const {
    std::vector<WorldPosition*> retVec;

    if (points.size() < 2)
    {
        retVec.push_back(points[0]);
        return retVec;
    }

    retVec = points;

    std::vector<uint32> weights;

    std::transform(retVec.begin(), retVec.end(), std::back_inserter(weights), [this](WorldPosition* point) { return 200000 / (1 + this->distance(*point)); });

    //If any weight is 0 add 1 to all weights.
    for (auto& w : weights)
    {
        if (w > 0)
            continue;

        std::for_each(weights.begin(), weights.end(), [](uint32& d) { d += 1; });
        break;

    }

    std::mt19937 gen(time(0));

    WeightedShuffle(retVec.begin(), retVec.end(), weights.begin(), weights.end(), gen);

    return retVec;
}

std::vector<WorldPosition> WorldPosition::GetNextPoint(std::vector<WorldPosition> points, uint32 amount) const {
    std::vector<WorldPosition> retVec;

    if (points.size() < 2)
    {
        if (points.size() == 1)
            retVec.push_back(points[0]);
        return retVec;
    }

    retVec = points;


    std::vector<uint32> weights;

    //List of weights based on distance (Gausian curve that starts at 100 and lower to 1 at 1000 distance)
    //std::transform(retVec.begin(), retVec.end(), std::back_inserter(weights), [center](WorldPosition point) { return 1 + 1000 * exp(-1 * pow(point.distance(center) / 400.0, 2)); });

    //List of weights based on distance (Twice the distance = half the weight). Caps out at 200.0000 range.
    std::transform(retVec.begin(), retVec.end(), std::back_inserter(weights), [this](WorldPosition point) { return 200000 / (1 + this->distance(point)); });

    //If any weight is 0 add 1 to all weights.
    for (auto& w : weights)
    {
        if (w > 0)
            continue;

        std::for_each(weights.begin(), weights.end(), [](uint32& d) { d += 1; });
        break;

    }

    std::mt19937 gen(time(0));

    WeightedShuffle(retVec.begin(), retVec.end(), weights.begin(), weights.end(), gen);

    return retVec;
}

bool WorldPosition::IsInStaticLineOfSight(WorldPosition pos, float heightMod) const
{
    if (mapid != pos.mapid)
    {
        return false;
    }
    
    float srcX = coord_x;
    float srcY = coord_y;
    float srcZ = coord_z + heightMod;
    float dstX = pos.coord_x;
    float dstY = pos.coord_y;
    float dstZ = pos.coord_z + heightMod;

    return VMAP::VMapFactory::createOrGetVMapManager()->isInLineOfSight(mapid, srcX, srcY, srcZ, dstX, dstY, dstZ, true);
}

bool WorldPosition::canFly() const
{
#ifdef MANGOSBOT_ZERO
    return false;
#endif
    if (!getTerrain())
        return false;

    uint32 zoneid, areaid;
    getTerrain()->GetZoneAndAreaId(zoneid, areaid, getX(), getY(), getZ());

#ifdef MANGOSBOT_ONE  
    uint32 v_map = GetVirtualMapForMapAndZone(getMapId(), zoneid);
    MapEntry const* mapEntry = sMapStore.LookupEntry(v_map);
    if (!mapEntry || mapEntry->addon < 1 || !mapEntry->IsContinent())
        return false;
#endif
#ifdef MANGOSBOT_TWO
    // Disallow mounting in wintergrasp when battle is in progress
    if (OutdoorPvP* outdoorPvP = sOutdoorPvPMgr.GetScript(zoneid))
    {
        if (outdoorPvP->IsBattlefield())
            return ((Battlefield*)outdoorPvP)->GetBattlefieldStatus() != BF_STATUS_IN_PROGRESS;
    }

    // don't allow flying in Dalaran restricted areas
    // (no other zones currently has areas with AREA_FLAG_CANNOT_FLY)
    if (AreaTableEntry const* atEntry = GetAreaEntryByAreaID(areaid))
        return (!(atEntry->flags & AREA_FLAG_CANNOT_FLY));
#endif

    return true;
}

G3D::Vector3 WorldPosition::getVector3() const
{
    return G3D::Vector3(coord_x, coord_y, coord_z); 
}

std::string WorldPosition::print() const
{
    std::ostringstream out;
    out << mapid << std::fixed << std::setprecision(2);
    out << ';'<< coord_x;
    out << ';' << coord_y;
    out << ';' << coord_z;
    out << ';' << orientation;

    return out.str();
}

void WorldPosition::printWKT(const std::vector<WorldPosition>& points, std::ostringstream& out, const uint32 dim, const bool loop)
{
    switch (dim) {
    case 0:
        if(points.size() == 1)
            out << "\"POINT(";
        else
            out << "\"MULTIPOINT(";
        break;
    case 1:
        out << "\"LINESTRING(";
        break;
    case 2:
        out << "\"POLYGON((";
    }

    for (auto& p : points)
        out << p.getDisplayX() << " " << p.getDisplayY() << (!loop && &p == &points.back() ? "" : ",");

    if (loop)
        out << points.front().getDisplayX() << " " << points.front().getDisplayY();

    out << (dim == 2 ? "))\"," : ")\",");
}

WorldPosition WorldPosition::getDisplayLocation() const
{ 
    WorldPosition mapOffset = sTravelNodeMap.getMapOffset(getMapId());
    return offset(mapOffset);
};

AreaTableEntry const* WorldPosition::GetArea() const
{
    uint16 areaFlag = getAreaFlag();

    return GetAreaEntryByAreaFlagAndMap(areaFlag, getMapId());
}

std::string WorldPosition::getAreaName(const bool fullName, const bool zoneName) const
{    
    if (!isOverworld())
    {
        MapEntry const* map = sMapStore.LookupEntry(getMapId());
        if (map)
            return map->name[0];
    }

    AreaTableEntry const* area = GetArea();

    if (!area)
        return "";

    std::string areaName = area->area_name[0];

    if (fullName)
    {
        uint16 zoneId = area->zone;

        while (zoneId > 0)
        {
            AreaTableEntry const* parentArea = GetAreaEntryByAreaID(zoneId);

            if (!parentArea)
                break;

            std::string subAreaName = parentArea->area_name[0];

            if (zoneName)
                areaName = subAreaName;
            else
                areaName = subAreaName + " " + areaName;

            zoneId = parentArea->zone;
        }
    }

    return areaName;
}

int32 WorldPosition::getAreaLevel() const
{
    if (mapid == 609)
        return 1;

    if(GetArea())
        return sTravelMgr.GetAreaLevel(GetArea()->ID);

    return 0;
}

bool WorldPosition::HasAreaFlag(const AreaFlags flag) const
{
    AreaTableEntry const* areaEntry = GetArea();
    if (areaEntry)
    {
        if (areaEntry->zone)
            areaEntry = GetAreaEntryByAreaID(areaEntry->zone);

        if (areaEntry && areaEntry->flags & flag)
            return true;
    }

    return false;
}

bool WorldPosition::HasFaction(const Team team) const
{
    AreaTableEntry const* areaEntry = GetArea();
    if (areaEntry)
    {
        if (areaEntry->team == 2 && team == ALLIANCE)
            return true;
        if (areaEntry->team == 4 && team == HORDE)
            return true;
        if (areaEntry->team == 6)
            return true;
    }
    return false;
}

std::set<GenericTransport*> WorldPosition::getTransports(uint32 entry)
{
    std::set<GenericTransport*> transports;
    for (auto transport : getMap(getFirstInstanceId())->GetTransports()) //Boats&Zeppelins.
        if (!entry || transport->GetEntry() == entry)
            transports.insert(transport);

    if (transports.empty() || !entry) //Elevators&rams
    {
        for (auto gopair : getGameObjectsNear(0.0f, entry))
            if (GameObject* go = getMap(getFirstInstanceId())->GetGameObject(gopair->first))
                if (GenericTransport* transport = dynamic_cast<GenericTransport*>(go))
                    transports.insert(transport);
    }

    return transports;
}

void WorldPosition::CalculatePassengerPosition(GenericTransport* transport)
{
    transport->CalculatePassengerPosition(coord_x, coord_y, coord_z, &orientation);
}

void WorldPosition::CalculatePassengerOffset(GenericTransport* transport)
{
    transport->CalculatePassengerOffset(coord_x, coord_y, coord_z, &orientation);
}

bool WorldPosition::isOnTransport(GenericTransport* transport)
{
    if (!transport)
        return false;

    WorldPosition trans(transport);

    if (distance(trans) > 40.0f)
        return false;

    WorldPosition below(*this);

    below.setZ(below.getZ() - 5.0f);

    bool result0 = VMAP::VMapFactory::createOrGetVMapManager()->getObjectHitPos(mapid, coord_x, coord_y, coord_z + 0.5f, below.getX(), below.getY(), below.getZ(), below.coord_x, below.coord_y, below.coord_z, 0.0f);

    if (result0)
        return false;

    return GetHitPosition(below);
}

std::vector<GridPair> WorldPosition::getGridPairs(const WorldPosition& secondPos) const
{
    std::vector<GridPair> retVec;

    int lx = std::min(getGridPair().x_coord, secondPos.getGridPair().x_coord);
    int ly = std::min(getGridPair().y_coord, secondPos.getGridPair().y_coord);
    int ux = std::max(getGridPair().x_coord, secondPos.getGridPair().x_coord);
    int uy = std::max(getGridPair().y_coord, secondPos.getGridPair().y_coord);
    int border = 1;

    lx = std::min(std::max(border, lx), MAX_NUMBER_OF_GRIDS - border);
    ly = std::min(std::max(border, ly), MAX_NUMBER_OF_GRIDS - border);
    ux = std::min(std::max(border, ux), MAX_NUMBER_OF_GRIDS - border);
    uy = std::min(std::max(border, uy), MAX_NUMBER_OF_GRIDS - border);

    for (int x = lx - border; x <= ux + border; x++)
    {
        for (int y = ly - border; y <= uy + border; y++)
        {
            retVec.push_back(GridPair(x, y));
        }
    }

    return retVec;
}

std::vector<WorldPosition> WorldPosition::fromGridPair(const GridPair& gridPair, uint32 mapId)
{
    std::vector<WorldPosition> retVec;
    GridPair g;

    for (uint32 d = 0; d < 4; d++)
    {
        g = gridPair;

        if (d == 1 || d == 2)
            g >> 1;
        if (d == 2 || d == 3)
            g += 1;

        retVec.push_back(WorldPosition(mapId, g));
    }
   
    return retVec;
}

std::vector<WorldPosition> WorldPosition::fromCellPair(const CellPair& cellPair) const
{
    std::vector<WorldPosition> retVec;
    CellPair p;

    for (uint32 d = 0; d < 4; d++)
    {
        p = cellPair;

        if (d == 1 || d == 2)
            p >> 1;
        if (d == 2 || d == 3)
            p += 1;

        retVec.push_back(WorldPosition(getMapId(), p));
    }
    return retVec;
}

std::vector<WorldPosition> WorldPosition::gridFromCellPair(const CellPair& cellPair) const
{    
    Cell c(cellPair);

    return fromGridPair(GridPair(c.GridX(), c.GridY()), getMapId());
}

std::vector<std::pair<int,int>> WorldPosition::getmGridPairs(const WorldPosition& secondPos) const
{
    std::vector<mGridPair> retVec;

    int lx = std::min(getmGridPair().first, secondPos.getmGridPair().first);
    int ly = std::min(getmGridPair().second, secondPos.getmGridPair().second);
    int ux = std::max(getmGridPair().first, secondPos.getmGridPair().first);
    int uy = std::max(getmGridPair().second, secondPos.getmGridPair().second);
    int border = 1;

    lx = std::min(std::max(border, lx), MAX_NUMBER_OF_GRIDS - border);
    ly = std::min(std::max(border, ly), MAX_NUMBER_OF_GRIDS - border);
    ux = std::min(std::max(border, ux), MAX_NUMBER_OF_GRIDS - border);
    uy = std::min(std::max(border, uy), MAX_NUMBER_OF_GRIDS - border);

    for (int x = lx - border; x <= ux + border; x++)
    {
        for (int y = ly - border; y <= uy + border; y++)
        {
            retVec.push_back(std::make_pair(x, y));
        }
    }

    return retVec;
}

std::vector<WorldPosition> WorldPosition::frommGridPair(const mGridPair& gridPair, uint32 mapId)
{
    std::vector<WorldPosition> retVec;
    mGridPair g;

    for (uint32 d = 0; d < 4; d++)
    {
        g = gridPair;

        if (d == 1 || d == 2)
            g.second++;
        if (d == 2 || d == 3)
            g.first++;

        retVec.push_back(WorldPosition(mapId, g));
    }

    return retVec;
}

bool WorldPosition::isVmapLoaded(uint32 mapId, int x, int y) 
{
    return VMAP::VMapFactory::createOrGetVMapManager()->IsTileLoaded(mapId, x, y);
}

bool WorldPosition::isMmapLoaded(uint32 mapId, uint32 instanceId, int x, int y)
{
#ifndef MANGOSBOT_TWO
    return MMAP::MMapFactory::createOrGetMMapManager()->IsMMapIsLoaded(mapId, x, y);
#else
    return MMAP::MMapFactory::createOrGetMMapManager()->IsMMapTileLoaded(mapId, instanceId, x, y);
#endif
}

bool WorldPosition::loadMapAndVMap(uint32 mapId, uint32 instanceId, int x, int y)
{
    std::string logName = "load_map_grid.csv";

    bool hasMmap = false;
    if (mapId == 0 || mapId == 1 || mapId == 530 || mapId == 571)
        hasMmap = isMmapLoaded(mapId, 0, x, y);
    else
        hasMmap = isMmapLoaded(mapId, instanceId, x, y);

    if (hasMmap)
        return true;

    if (sTravelMgr.IsBadMmap(mapId, x, y))
        return false;

    bool isLoaded = false;

    if (!hasMmap)
    {
#ifndef MANGOSBOT_TWO
        if (mapId == 0 || mapId == 1 || mapId == 530 || mapId == 571)
            isLoaded = MMAP::MMapFactory::createOrGetMMapManager()->loadMap(sWorld.GetDataPath(), mapId, x, y);
        else
        {
            MMAP::MMapFactory::createOrGetMMapManager()->loadMapInstance(sWorld.GetDataPath(), mapId, instanceId);
            isLoaded = MMAP::MMapFactory::createOrGetMMapManager()->loadMap(sWorld.GetDataPath(), mapId, x, y);
        }
#else
        if (mapId == 0 || mapId == 1 || mapId == 530 || mapId == 571)
        {
            isLoaded = MMAP::MMapFactory::createOrGetMMapManager()->loadMap(sWorld.GetDataPath(), mapId, 0, x, y, 0);
        }
        else
        {
            bool loadedMap = MMAP::MMapFactory::createOrGetMMapManager()->GetNavMesh(mapId, instanceId);

            if (!loadedMap)
                loadedMap = MMAP::MMapFactory::createOrGetMMapManager()->loadMapInstance(sWorld.GetDataPath(), mapId, instanceId);

            if (loadedMap)
                isLoaded = MMAP::MMapFactory::createOrGetMMapManager()->loadMap(sWorld.GetDataPath(), mapId, instanceId, x, y, 0);
        }
#endif


        //if (!isLoaded)
        //    sTravelMgr.AddBadMmap(mapId, x, y);
    }

    //if (!hasVmap)
    //{
    //    loadVMap(mapId, x, y);
    //}

    if (sPlayerbotAIConfig.hasLog(logName))
    {
        std::ostringstream out;
        out << sPlayerbotAIConfig.GetTimestampStr();
        out << "+00,\"mmap\", " << x << "," << y << "," << (sTravelMgr.IsBadMmap(mapId, x, y) ? "0" : "1") << ",";
        printWKT(frommGridPair(mGridPair(x, y), mapId), out, 1, true);
        sPlayerbotAIConfig.log(logName, out.str().c_str());
    }

    return isLoaded;
}

void WorldPosition::loadMapAndVMaps(const WorldPosition& secondPos, uint32 instanceId) const
{
    for (auto& grid : getmGridPairs(secondPos))
    {
        loadMapAndVMap(getMapId(), instanceId, grid.first, grid.second);
    }
}

void WorldPosition::unloadMapAndVMaps(uint32 mapId)
{
#ifndef MANGOSBOT_TWO
    //TerrainInfoAccess* terrain = reinterpret_cast<TerrainInfoAccess*>(const_cast<TerrainInfo*>(sTerrainMgr.LoadTerrain(mapId)));
    //terrain->UnLoadUnused();
#endif
}

bool WorldPosition::loadVMap(uint32 mapId, int x, int y)
{
    if (isVmapLoaded(mapId, x, y))
        return true;

    return VMAP::VMapFactory::createOrGetVMapManager()->loadMap(sWorld.GetDataPath().c_str(), mapId, x, y);
}

std::vector<WorldPosition> WorldPosition::fromPointsArray(const std::vector<G3D::Vector3>& path) const
{
    std::vector<WorldPosition> retVec;
    for (auto p : path)
        retVec.push_back(WorldPosition(getMapId(), p.x, p.y, p.z, getO()));

    return retVec;
}

//A single pathfinding attempt from one position to another. Returns pathfinding status and path.
std::vector<WorldPosition> WorldPosition::getPathStepFrom(const WorldPosition& startPos, std::unique_ptr<PathFinder>& pathfinder, const Unit* bot, bool forceNormalPath) const
{
    std::hash<std::thread::id> hasher;
    uint32 instanceId;
    if (sTravelNodeMap.gethasToGen())
        instanceId = 0;
    else if (!bot || bot->GetMapId() != startPos.getMapId())
        instanceId = hasher(std::this_thread::get_id());
    else
        instanceId = bot->GetInstanceId();
    //Load mmaps and vmaps between the two points.

    loadMapAndVMaps(startPos, instanceId);

    PointsArray points;
    PathType type;

    WorldPosition start = startPos, end = *this;

    if (bot && bot->GetTransport())
    {
        start.CalculatePassengerOffset(bot->GetTransport());
        end.CalculatePassengerOffset(bot->GetTransport());
    }

    pathfinder->calculate(start.getVector3(), end.getVector3(), false);

    points = pathfinder->getPath();

    if (bot && bot->GetTransport())
    {
        for (auto& p : points)
            bot->GetTransport()->CalculatePassengerPosition(p.x, p.y, p.z);
    }

    type = pathfinder->getPathType();

    if (sPlayerbotAIConfig.hasLog("pathfind_attempt_point.csv"))
    {
        std::ostringstream out;
        out << std::fixed << std::setprecision(1);
        printWKT({ startPos, *this }, out);
        sPlayerbotAIConfig.log("pathfind_attempt_point.csv", out.str().c_str());
    }

    if (sPlayerbotAIConfig.hasLog("pathfind_attempt.csv") && (type == PATHFIND_INCOMPLETE || type == PATHFIND_NORMAL))
    {
        std::ostringstream out;
        out << sPlayerbotAIConfig.GetTimestampStr() << "+00,";
        out << std::fixed << std::setprecision(1) << type << ",";
        printWKT(fromPointsArray(points), out, 1);
        sPlayerbotAIConfig.log("pathfind_attempt.csv", out.str().c_str());
    }

    std::vector<WorldPosition> retvec = fromPointsArray(points);

    if (type == PATHFIND_INCOMPLETE)
    {
        WorldPosition lastPoint = retvec.back();

        float dist = lastPoint.distance(end);

        if (lastPoint.distance(end) < 50.0f && lastPoint.isUnderWater() && end.isUnderWater() && lastPoint.IsInLineOfSight(end))
        {
            if (dist < 5.0f)
                retvec.push_back(end);
            else
            {
                WorldPosition stepPoint = (end - lastPoint) / dist * 5.0f;
                retvec.push_back(stepPoint);
            }

            return retvec;
        }
    }

    if ((!forceNormalPath && type == PATHFIND_INCOMPLETE) || type == PATHFIND_NORMAL)
        return retvec;

    return {};
}

std::vector<WorldPosition> WorldPosition::getPathStepFrom(const WorldPosition& startPos, const Unit* bot, bool forceNormalPath) const
{
    std::unique_ptr<PathFinder> pathfinder = std::make_unique<PathFinder>(bot);
    return getPathStepFrom(startPos, pathfinder, bot, forceNormalPath);
}

bool WorldPosition::isPathTo(const std::vector<WorldPosition>& path, float const maxDistance) const
{
    float realMaxDistance = maxDistance ? maxDistance : sPlayerbotAIConfig.targetPosRecalcDistance;
    return !path.empty() && distance(path.back()) < realMaxDistance;
};


bool WorldPosition::cropPathTo(std::vector<WorldPosition>& path, const float maxDistance) const
{
    float realMaxDistance = maxDistance ? maxDistance : sPlayerbotAIConfig.targetPosRecalcDistance; 

    if (path.empty())
        return false;

   auto bestPos = std::min_element(path.begin(), path.end(), [this](WorldPosition i, WorldPosition j) {return this->sqDistance(i) < this->sqDistance(j); });

   bool insRange = this->sqDistance(*bestPos) <= realMaxDistance * realMaxDistance;

   if (bestPos == path.end())
       return insRange;

   path.erase(std::next(bestPos), path.end());

   return insRange;
}

//A sequential series of pathfinding attempts. Returns the complete path and if the patfinder eventually found a way to the destination.
std::vector<WorldPosition> WorldPosition::getPathFromPath(const std::vector<WorldPosition>& startPath, const Unit* bot, uint8 maxAttempt) const
{
    //We start at the end of the last path.
    WorldPosition currentPos = startPath.back();

    //No pathfinding across maps.
    if (getMapId() != currentPos.getMapId())
        return { };

    std::vector<WorldPosition> subPath, fullPath = startPath;

    std::hash<std::thread::id> hasher;
    uint32 instanceId;
    if (sTravelNodeMap.gethasToGen())
        instanceId = 0;
    else if (!bot || bot->GetMapId() != currentPos.getMapId())
        instanceId = hasher(std::this_thread::get_id());
    else
        instanceId = bot->GetInstanceId();
    //Load mmaps and vmaps between the two points.

    std::unique_ptr<PathFinder> pathfinder = nullptr;

    if (bot && instanceId == bot->GetInstanceId())
        pathfinder = std::make_unique<PathFinder>(bot);
    else
        pathfinder = std::make_unique<PathFinder>(getMapId(), instanceId);

    pathfinder->setAreaCost(NAV_AREA_WATER, 10.0f);
    pathfinder->setAreaCost(12, 5.0f);
    pathfinder->setAreaCost(13, 20.0f);

    //Limit the pathfinding attempts
    for (uint32 i = 0; i < maxAttempt; i++)
    {
        //Try to pathfind to this position.
        subPath = getPathStepFrom(currentPos, pathfinder, bot);

        //If we could not find a path return what we have now.
        if (subPath.empty() || currentPos.distance(subPath.back()) < sPlayerbotAIConfig.targetPosRecalcDistance)
            break;

        //Append the path excluding the start (this should be the same as the end of the startPath)
        fullPath.insert(fullPath.end(), std::next(subPath.begin(), 1), subPath.end());

        //Are we there yet?
        if (isPathTo(subPath))
            break;

        //Continue pathfinding.
        currentPos = subPath.back();
    }

    return fullPath;
}

bool WorldPosition::ClosestCorrectPoint(float maxRange, float maxHeight, uint32 instanceId)
{
    MANGOS_ASSERT(std::isfinite(coord_x) && std::isfinite(coord_y) && std::isfinite(coord_z));

    MMAP::MMapManager* mmap = MMAP::MMapFactory::createOrGetMMapManager();

    MANGOS_ASSERT(mmap);

    dtNavMeshQuery const* query = mmap->GetNavMeshQuery(getMapId(), instanceId);

    MANGOS_ASSERT(query && query->getAttachedNavMesh());

    float curPoint[VERTEX_SIZE] = {coord_y, coord_z, coord_x };
    float extend[VERTEX_SIZE] = { maxRange, maxHeight, maxRange };
    float newPoint[VERTEX_SIZE];

    dtQueryFilter filter;
    dtPolyRef polyRef = INVALID_POLYREF;

    uint16 includeFlags = 0;
    uint16 excludeFlags = 0;

    includeFlags |= (NAV_GROUND );
    excludeFlags |= (NAV_MAGMA_SLIME | NAV_GROUND_STEEP | NAV_WATER);


    filter.setIncludeFlags(includeFlags);
    filter.setExcludeFlags(excludeFlags);

    dtStatus dtResult = query->findNearestPoly(curPoint, extend, &filter, &polyRef, newPoint);

    coord_y = newPoint[0];
    coord_z = newPoint[1];
    coord_x = newPoint[2];

    return dtStatusSucceed(dtResult) && polyRef != INVALID_POLYREF;
}

bool WorldPosition::GetReachableRandomPointOnGround(const Player* bot, const float radius, const bool randomRange) 
{
#ifndef MANGOSBOT_TWO         
    return getMap(bot ? bot->GetInstanceId() : getFirstInstanceId())->GetReachableRandomPointOnGround(coord_x, coord_y, coord_z, radius, randomRange);
#else
    return getMap(bot ? bot->GetInstanceId() : getFirstInstanceId())->GetReachableRandomPointOnGround(bot->GetPhaseMask(), coord_x, coord_y, coord_z, radius, randomRange);
#endif
}

std::vector<WorldPosition> WorldPosition::ComputePathToRandomPoint(const Player* bot, const float radius, const bool randomRange)
{
    WorldPosition start = *this;

    float angle = rand_norm_f() * 2 * M_PI_F;
    float range = radius;

    if (randomRange)
        range *= rand_norm_f();

    coord_x += range * cos(angle);
    coord_y += range * sin(angle);

    std::unique_ptr<PathFinder> pathfinder = std::make_unique<PathFinder>(bot);

    pathfinder->setAreaCost(NAV_AREA_WATER, 10.0f);
    pathfinder->setAreaCost(12, 5.0f);
    pathfinder->setAreaCost(13, 20.0f);

    std::vector<WorldPosition> path = getPathStepFrom(start, pathfinder, bot);
    
    if (path.size())
        set(path.back());
    else
        set(start);

    return path;
}

uint32 WorldPosition::getUnitsAggro(const std::list<ObjectGuid>& units, const Player* bot) const
{
    uint32 count = 0;
    for (auto guid : units)
    {
        Unit* unit = GuidPosition(guid,bot).GetUnit(bot->GetInstanceId()); 
        
        if (!unit) continue; 
        
        if (this->sqDistance(unit) > unit->GetAttackDistance(bot) * unit->GetAttackDistance(bot))
            continue;

        count++;
    }

    return count;
};



bool FindPointCreatureData::operator()(CreatureDataPair const& dataPair)
{
    if (!entry || dataPair.second.id == entry)
        if ((!point || dataPair.second.mapid == point.getMapId()) && (!radius || point.sqDistance(WorldPosition(dataPair.second.mapid, dataPair.second.posX, dataPair.second.posY, dataPair.second.posZ)) < radius * radius))
        {
            data.push_back(&dataPair);
        }

    return false;
}

bool FindPointGameObjectData::operator()(GameObjectDataPair const& dataPair)
{
    if (!entry || dataPair.second.id == entry)
        if ((!point || dataPair.second.mapid == point.getMapId()) && (!radius || point.sqDistance(WorldPosition(dataPair.second.mapid, dataPair.second.posX, dataPair.second.posY, dataPair.second.posZ)) < radius * radius))
        {
            data.push_back(&dataPair);
        }

    return false;
}

std::vector<CreatureDataPair const*> WorldPosition::getCreaturesNear(const float radius, const uint32 entry) const
{
    FindPointCreatureData worker(*this, radius, entry);
    sObjectMgr.DoCreatureData(worker);
    return worker.GetResult();
}

std::vector<GameObjectDataPair const*> WorldPosition::getGameObjectsNear(const float radius, const uint32 entry) const
{
    FindPointGameObjectData worker(*this, radius, entry);
    sObjectMgr.DoGOData(worker);
    return worker.GetResult();
}
