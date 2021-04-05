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

#include "MoveMap.h"
#include "Maps/GridMap.h"
#include "Entities/Creature.h"
#include "PathFinder.h"
#include "Log.h"
#include "World/World.h"
#include "Entities/Transports.h"
#include <Detour/Include/DetourCommon.h>
#include <Detour/Include/DetourMath.h>

#ifdef BUILD_METRICS
 #include "Metric/Metric.h"
#endif

#include <limits>
////////////////// PathFinder //////////////////
PathFinder::PathFinder(const Unit* owner) :
    m_polyLength(0), m_type(PATHFIND_BLANK),
    m_useStraightPath(false), m_forceDestination(false), m_straightLine(false), m_pointPathLimit(MAX_POINT_PATH_LENGTH), // TODO: Fix legitimate long paths
    m_sourceUnit(owner), m_navMesh(nullptr), m_navMeshQuery(nullptr), m_cachedPoints(m_pointPathLimit * VERTEX_SIZE), m_pathPolyRefs(m_pointPathLimit), m_smoothPathPolyRefs(m_pointPathLimit), m_defaultMapId(m_sourceUnit->GetMapId())
{
    DEBUG_FILTER_LOG(LOG_FILTER_PATHFINDING, "++ PathFinder::PathInfo for %u \n", m_sourceUnit->GetGUIDLow());

    if (MMAP::MMapFactory::IsPathfindingEnabled(m_sourceUnit->GetMapId(), m_sourceUnit))
    {
        MMAP::MMapManager* mmap = MMAP::MMapFactory::createOrGetMMapManager();
        m_defaultNavMeshQuery = mmap->GetNavMeshQuery(m_sourceUnit->GetMapId(), m_sourceUnit->GetInstanceId());
    }

    createFilter();
}

PathFinder::~PathFinder()
{
    DEBUG_FILTER_LOG(LOG_FILTER_PATHFINDING, "++ PathFinder::~PathInfo() for %u \n", m_sourceUnit->GetGUIDLow());
}

void PathFinder::SetCurrentNavMesh()
{
    if (MMAP::MMapFactory::IsPathfindingEnabled(m_sourceUnit->GetMapId(), m_sourceUnit))
    {
        MMAP::MMapManager* mmap = MMAP::MMapFactory::createOrGetMMapManager();
        if (GenericTransport* transport = m_sourceUnit->GetTransport())
            m_navMeshQuery = mmap->GetModelNavMeshQuery(transport->GetDisplayId());
        else
        {
            if (m_defaultMapId != m_sourceUnit->GetMapId())
                m_defaultNavMeshQuery = mmap->GetNavMeshQuery(m_sourceUnit->GetMapId(), m_sourceUnit->GetInstanceId());

            m_navMeshQuery = m_defaultNavMeshQuery;
        }

        if (m_navMeshQuery)
            m_navMesh = m_navMeshQuery->getAttachedNavMesh();
    }
}

bool PathFinder::calculate(float destX, float destY, float destZ, bool forceDest/* = false*/, bool straightLine/* = false*/)
{
    float x, y, z;
    m_sourceUnit->GetPosition(x, y, z, m_sourceUnit->GetTransport());
    Vector3 dest(destX, destY, destZ);
    if (GenericTransport* transport = m_sourceUnit->GetTransport())
        transport->CalculatePassengerOffset(dest.x, dest.y, dest.z);
    return calculate(Vector3(x, y, z), dest, forceDest, straightLine);
}

bool PathFinder::calculate(Vector3 const& start, Vector3 const& dest, bool forceDest/* = false*/, bool straightLine/* = false*/)
{
    if (!MaNGOS::IsValidMapCoord(dest.x, dest.y, dest.z))
        return false;

    if (!MaNGOS::IsValidMapCoord(start.x, start.y, start.z))
        return false;

#ifdef BUILD_METRICS
    metric::duration<std::chrono::microseconds> meas("pathfinder.calculate", {
        { "entry", std::to_string(m_sourceUnit->GetEntry()) },
        { "guid", std::to_string(m_sourceUnit->GetGUIDLow()) },
        { "unit_type", std::to_string(m_sourceUnit->GetGUIDHigh()) },
        { "map_id", std::to_string(m_sourceUnit->GetMapId()) },
        { "instance_id", std::to_string(m_sourceUnit->GetInstanceId()) }
    }, 1000);
#endif

    //if (GenericTransport* transport = m_sourceUnit->GetTransport())
    //    transport->CalculatePassengerOffset(dest.x, dest.y, dest.z, nullptr);

    setStartPosition(start);

    setEndPosition(dest);

    m_forceDestination = forceDest;
    m_straightLine = straightLine;

    SetCurrentNavMesh();

    DEBUG_FILTER_LOG(LOG_FILTER_PATHFINDING, "++ PathFinder::calculate() for %u \n", m_sourceUnit->GetGUIDLow());

    // make sure navMesh works - we can run on map w/o mmap
    // check if the start and end point have a .mmtile loaded (can we pass via not loaded tile on the way?)
    if (!m_navMesh || !m_navMeshQuery || m_sourceUnit->hasUnitState(UNIT_STAT_IGNORE_PATHFINDING) ||
        !HaveTile(start) || !HaveTile(dest))
    {
        BuildShortcut();
        m_type = PathType(PATHFIND_NORMAL | PATHFIND_NOT_USING_PATH);
        return true;
    }

    updateFilter();

    BuildPolyPath(start, dest);
    return true;
}

dtPolyRef PathFinder::getPathPolyByPosition(const dtPolyRef* polyPath, uint32 polyPathSize, const float* point, float* distance) const
{
    if (!polyPath || !polyPathSize)
        return INVALID_POLYREF;

    dtPolyRef nearestPoly = INVALID_POLYREF;
    float minDist3d = std::numeric_limits<float>::max();

    for (uint32 i = 0; i < polyPathSize; ++i)
    {
        float closestPoint[VERTEX_SIZE];
        if (dtStatusFailed(m_navMeshQuery->closestPointOnPoly(polyPath[i], point, closestPoint, nullptr)))
            continue;

        float d = dtVdistSqr(point, closestPoint);
        if (d < minDist3d)
        {
            minDist3d = d;
            nearestPoly = polyPath[i];
        }

        if (minDist3d < 1.0f) // shortcut out - close enough for us
            break;
    }

    if (distance)
        *distance = dtMathSqrtf(minDist3d);

    return (minDist3d < 3.0f) ? nearestPoly : INVALID_POLYREF;
}

dtPolyRef PathFinder::getPolyByLocation(const float* point, float* distance) const
{
    // first we check the current path
    // if the current path doesn't contain the current poly,
    // we need to use the expensive navMesh.findNearestPoly
    dtPolyRef polyRef = getPathPolyByPosition(m_pathPolyRefs.data(), m_polyLength, point, distance);
    if (polyRef != INVALID_POLYREF)
        return polyRef;

    // we don't have it in our old path
    // try to get it by findNearestPoly()
    // first try with low search box
    float extents[VERTEX_SIZE] = {5.0f, 5.0f, 5.0f};    // bounds of poly search area
    float closestPoint[VERTEX_SIZE] = {0.0f, 0.0f, 0.0f};
    dtStatus dtResult = m_navMeshQuery->findNearestPoly(point, extents, &m_filter, &polyRef, closestPoint);
    if (dtStatusSucceed(dtResult) && polyRef != INVALID_POLYREF)
    {
        *distance = dtVdist(closestPoint, point);
        return polyRef;
    }

    // still nothing ..
    // try with bigger search box

    // From dtNavMeshQuery::findNearestPoly: "If the search extents overlaps more than
    // 128 polygons it may return an invalid result". So use about 45 yards on Y.
    extents[1] = DEFAULT_VISIBILITY_DISTANCE / 2.0f;

    dtResult = m_navMeshQuery->findNearestPoly(point, extents, &m_filter, &polyRef, closestPoint);
    if (dtStatusSucceed(dtResult) && polyRef != INVALID_POLYREF)
    {
        *distance = dtVdist(closestPoint, point);
        return polyRef;
    }

    return INVALID_POLYREF;
}

void PathFinder::BuildPolyPath(const Vector3& startPos, const Vector3& endPos)
{
    // *** getting start/end poly logic ***
    if (m_sourceUnit->GetMap()->IsDungeon())
    {
        float distance = sqrt((endPos.x - startPos.x) * (endPos.x - startPos.x) + (endPos.y - startPos.y) * (endPos.y - startPos.y) + (endPos.z - startPos.z) * (endPos.z - startPos.z));
        if (distance > 300.f)
            m_pointPathLimit = MAX_POINT_PATH_LENGTH * 4;
        else if (distance > 200.f)
            m_pointPathLimit = MAX_POINT_PATH_LENGTH * 3;
        else if (distance > 100.f)
            m_pointPathLimit = MAX_POINT_PATH_LENGTH * 2;
        if (m_pointPathLimit > m_pathPolyRefs.size())
            m_pathPolyRefs.resize(m_pointPathLimit);
    }
    float distToStartPoly, distToEndPoly;
    float startPoint[VERTEX_SIZE] = {startPos.y, startPos.z, startPos.x};
    float endPoint[VERTEX_SIZE] = {endPos.y, endPos.z, endPos.x};

    dtPolyRef startPoly = getPolyByLocation(startPoint, &distToStartPoly);
    dtPolyRef endPoly = getPolyByLocation(endPoint, &distToEndPoly);

    dtStatus dtResult;

    // we have a hole in our mesh
    // make shortcut path and mark it as NOPATH ( with flying exception )
    // its up to caller how he will use this info
    if (startPoly == INVALID_POLYREF || endPoly == INVALID_POLYREF)
    {
        DEBUG_FILTER_LOG(LOG_FILTER_PATHFINDING, "++ BuildPolyPath :: (startPoly == 0 || endPoly == 0)\n");
        BuildShortcut();

        // Check for swimming or flying shortcut
        if ((startPoly == INVALID_POLYREF && m_sourceUnit->GetTerrain()->IsSwimmable(startPos.x, startPos.y, startPos.z)) ||
            (endPoly == INVALID_POLYREF && m_sourceUnit->GetTerrain()->IsSwimmable(endPos.x, endPos.y, endPos.z)))
            m_type = m_sourceUnit->CanSwim() ? PathType(PATHFIND_NORMAL | PATHFIND_NOT_USING_PATH) : PATHFIND_NOPATH;
        else
        {
            if (m_sourceUnit->GetTypeId() != TYPEID_PLAYER)
                m_type = m_sourceUnit->CanFly() ? PathType(PATHFIND_NORMAL | PATHFIND_NOT_USING_PATH) : PATHFIND_NOPATH;
            else
                m_type = PATHFIND_NOPATH;
        }

        return;
    }

    // we may need a better number here
    bool farFromPoly = (distToStartPoly > 7.0f || distToEndPoly > 7.0f);
    if (farFromPoly)
    {
        DEBUG_FILTER_LOG(LOG_FILTER_PATHFINDING, "++ BuildPolyPath :: farFromPoly distToStartPoly=%.3f distToEndPoly=%.3f\n", distToStartPoly, distToEndPoly);

        bool buildShotrcut = false;
        Vector3 p = (distToStartPoly > 7.0f) ? startPos : endPos;
        if (m_sourceUnit->GetTerrain()->IsUnderWater(p.x, p.y, p.z))
        {
            DEBUG_FILTER_LOG(LOG_FILTER_PATHFINDING, "++ BuildPolyPath :: underWater case\n");
            if (m_sourceUnit->CanSwim())
                buildShotrcut = true;
        }
        else
        {
            DEBUG_FILTER_LOG(LOG_FILTER_PATHFINDING, "++ BuildPolyPath :: flying case\n");
            if (m_sourceUnit->CanFly())
                buildShotrcut = true;
        }

        if (buildShotrcut)
        {
            BuildShortcut();
            m_type = PathType(PATHFIND_NORMAL | PATHFIND_NOT_USING_PATH);
            return;
        }
        float closestPoint[VERTEX_SIZE];
        // we may want to use closestPointOnPolyBoundary instead
        if (dtStatusSucceed(m_navMeshQuery->closestPointOnPoly(endPoly, endPoint, closestPoint, nullptr)))
        {
            dtVcopy(endPoint, closestPoint);
            setActualEndPosition(Vector3(endPoint[2], endPoint[0], endPoint[1]));
        }

        m_type = PATHFIND_INCOMPLETE;
    }

    // *** poly path generating logic ***

    // start and end are on same polygon
    // just need to move in straight line
    if (startPoly == endPoly)
    {
        DEBUG_FILTER_LOG(LOG_FILTER_PATHFINDING, "++ BuildPolyPath :: (startPoly == endPoly)\n");

        BuildShortcut();

        m_pathPolyRefs[0] = startPoly;
        m_polyLength = 1;

        m_type = farFromPoly ? PATHFIND_INCOMPLETE : PATHFIND_NORMAL;
        DEBUG_FILTER_LOG(LOG_FILTER_PATHFINDING, "++ BuildPolyPath :: path type %d\n", m_type);
        return;
    }

    // look for startPoly/endPoly in current path
    // TODO: we can merge it with getPathPolyByPosition() loop
    bool startPolyFound = false;
    bool endPolyFound = false;
    uint32 pathStartIndex = 0;
    uint32 pathEndIndex = 0;

    if (m_polyLength)
    {
        for (pathStartIndex = 0; pathStartIndex < m_polyLength; ++pathStartIndex)
        {
            // here to catch few bugs
            if (m_pathPolyRefs[pathStartIndex] == INVALID_POLYREF)
            {
                sLog.outError("Invalid poly ref in BuildPolyPath. polyLength: %u, pathStartIndex: %u,"
                              " startPos: %s, endPos: %s, mapId: %u",
                              m_polyLength, pathStartIndex, startPos.toString().c_str(), endPos.toString().c_str(),
                              m_sourceUnit->GetMapId());
                break;
            }

            if (m_pathPolyRefs[pathStartIndex] == startPoly)
            {
                startPolyFound = true;
                break;
            }
        }

        for (pathEndIndex = m_polyLength - 1; pathEndIndex > pathStartIndex; --pathEndIndex)
        {
            if (m_pathPolyRefs[pathEndIndex] == endPoly)
            {
                endPolyFound = true;
                break;
            }
        }
    }

    if (startPolyFound && endPolyFound)
    {
        DEBUG_FILTER_LOG(LOG_FILTER_PATHFINDING, "++ BuildPolyPath :: (startPolyFound && endPolyFound)\n");

        // we moved along the path and the target did not move out of our old poly-path
        // our path is a simple subpath case, we have all the data we need
        // just "cut" it out

        m_polyLength = pathEndIndex - pathStartIndex + 1;
        memmove(m_pathPolyRefs.data(), m_pathPolyRefs.data() + pathStartIndex, m_polyLength * sizeof(dtPolyRef));
    }
    //else if (startPolyFound && !endPolyFound)
    //{
    //    DEBUG_FILTER_LOG(LOG_FILTER_PATHFINDING, "++ BuildPolyPath :: (startPolyFound && !endPolyFound)\n");

    //    // we are moving on the old path but target moved out
    //    // so we have atleast part of poly-path ready

    //    m_polyLength -= pathStartIndex;

    //    // try to adjust the suffix of the path instead of recalculating entire length
    //    // at given interval the target cannot get too far from its last location
    //    // thus we have less poly to cover
    //    // sub-path of optimal path is optimal

    //    // take ~80% of the original length
    //    // TODO : play with the values here
    //    uint32 prefixPolyLength = uint32(m_polyLength * 0.8f + 0.5f);
    //    memmove(m_pathPolyRefs.data(), m_pathPolyRefs.data() + pathStartIndex, prefixPolyLength * sizeof(dtPolyRef));

    //    dtPolyRef suffixStartPoly = m_pathPolyRefs[prefixPolyLength - 1];

    //    // we need any point on our suffix start poly to generate poly-path, so we need last poly in prefix data
    //    float suffixEndPoint[VERTEX_SIZE];
    //    if (dtStatusFailed(m_navMeshQuery->closestPointOnPoly(suffixStartPoly, endPoint, suffixEndPoint, nullptr)))
    //    {
    //        // we can hit offmesh connection as last poly - closestPointOnPoly() don't like that
    //        // try to recover by using prev polyref
    //        --prefixPolyLength;
    //        suffixStartPoly = m_pathPolyRefs[prefixPolyLength - 1];
    //        if (dtStatusFailed(m_navMeshQuery->closestPointOnPoly(suffixStartPoly, endPoint, suffixEndPoint, nullptr)))
    //        {
    //            // suffixStartPoly is still invalid, error state
    //            BuildShortcut();
    //            m_type = PATHFIND_NOPATH;
    //            return;
    //        }
    //    }

    //    // generate suffix
    //    uint32 suffixPolyLength = 0;

    //    if (!m_straightLine)
    //    {
    //        dtResult = m_navMeshQuery->findPath(
    //                                suffixStartPoly,    // start polygon
    //                                endPoly,            // end polygon
    //                                suffixEndPoint,     // start position
    //                                endPoint,           // end position
    //                                &m_filter,            // polygon search filter
    //                                m_pathPolyRefs.data() + prefixPolyLength - 1,    // [out] path
    //                                (int*)&suffixPolyLength,
    //                                m_pointPathLimit - prefixPolyLength);   // max number of polygons in output path
    //    }
    //    else
    //    {
    //        float hit = 0.0f;
    //        float hitNormal[3] = {0.0f, 0.0f, 0.0f};

    //        dtResult = m_navMeshQuery->raycast(
    //                        suffixStartPoly,
    //                        suffixEndPoint,
    //                        endPoint,
    //                        &m_filter,
    //                        &hit,
    //                        hitNormal,
    //                        m_pathPolyRefs.data() + prefixPolyLength - 1,
    //                        (int*)&suffixPolyLength,
    //                        m_pointPathLimit - prefixPolyLength);

    //        // raycast() sets hit to FLT_MAX if there is a ray between start and end
    //        if (hit != FLT_MAX)
    //        {
    //            // the ray hit something, return no path instead of the incomplete one
    //            m_type = PATHFIND_NOPATH;
    //            return;
    //        }
    //    }

    //    if (!suffixPolyLength || dtStatusFailed(dtResult))
    //    {
    //        // this is probably an error state, but we'll leave it
    //        // and hopefully recover on the next Update
    //        // we still need to copy our preffix
    //        sLog.outError("%u's Path Build failed: 0 length path", m_sourceUnit->GetGUIDLow());
    //    }

    //    DEBUG_FILTER_LOG(LOG_FILTER_PATHFINDING, "++  m_polyLength=%u prefixPolyLength=%u suffixPolyLength=%u \n", m_polyLength, prefixPolyLength, suffixPolyLength);

    //    // new path = prefix + suffix - overlap
    //    m_polyLength = prefixPolyLength + suffixPolyLength - 1;
    //}
    else
    {
        DEBUG_FILTER_LOG(LOG_FILTER_PATHFINDING, "++ BuildPolyPath :: (!startPolyFound && !endPolyFound)\n");

        // either we have no path at all -> first run
        // or something went really wrong -> we aren't moving along the path to the target
        // just generate new path

        // free and invalidate old path data
        clear();

        if (!m_straightLine)
        {
            dtResult = m_navMeshQuery->findPath(
                    startPoly,          // start polygon
                    endPoly,            // end polygon
                    startPoint,         // start position
                    endPoint,           // end position
                    &m_filter,          // polygon search filter
                    m_pathPolyRefs.data(), // [out] path
                    (int*)&m_polyLength,
                    m_pointPathLimit);   // max number of polygons in output path
        }
        else
        {
            float hit = 0.0f;
            float hitNormal[3] = {0.0f, 0.0f, 0.0f};

            dtResult = m_navMeshQuery->raycast(
                            startPoly,
                            startPoint,
                            endPoint,
                            &m_filter,
                            &hit,
                            hitNormal,
                            m_pathPolyRefs.data(),
                            (int*)&m_polyLength,
                            m_pointPathLimit);

            // raycast() sets hit to FLT_MAX if there is a ray between start and end
            if (hit != FLT_MAX)
            {
                // the ray hit something, return no path instead of the incomplete one
                clear();
                m_polyLength = 2;
                m_pathPoints.resize(2);
                m_pathPoints[0] = getStartPosition();
                float hitPos[3];
                dtVlerp(hitPos, startPoint, endPoint, hit);
                m_pathPoints[1] = G3D::Vector3(hitPos[2], hitPos[0], hitPos[1]);

                m_type = PATHFIND_INCOMPLETE;
                return;
            }
        }

        if (!m_polyLength || dtStatusFailed(dtResult))
        {
            // only happens if we passed bad data to findPath(), or navmesh is messed up
            sLog.outError("%u's Path Build failed: 0 length path", m_sourceUnit->GetGUIDLow());
            BuildShortcut();
            m_type = PATHFIND_NOPATH;
            return;
        }
    }

    // by now we know what type of path we can get
    if (m_pathPolyRefs[m_polyLength - 1] == endPoly && !(m_type & PATHFIND_INCOMPLETE))
        m_type = PATHFIND_NORMAL;
    else
        m_type = PATHFIND_INCOMPLETE;

    // generate the point-path out of our up-to-date poly-path
    BuildPointPath(startPoint, endPoint);
}

void PathFinder::BuildPointPath(const float* startPoint, const float* endPoint)
{
    if (m_pointPathLimit * VERTEX_SIZE > m_cachedPoints.size())
        m_cachedPoints.resize(m_pointPathLimit * VERTEX_SIZE);
    float* pathPoints = m_cachedPoints.data();
    uint32 pointCount = 0;
    dtStatus dtResult = DT_FAILURE;

    if (m_straightLine)
    {
        dtResult = DT_SUCCESS;
        pointCount = 1;
        memcpy(&pathPoints[VERTEX_SIZE * 0], startPoint, sizeof(float) * 3); // first point

        // path has to be split into polygons with dist SMOOTH_PATH_STEP_SIZE between them
        G3D::Vector3 startVec = G3D::Vector3(startPoint[0], startPoint[1], startPoint[2]);
        G3D::Vector3 endVec = G3D::Vector3(endPoint[0], endPoint[1], endPoint[2]);
        G3D::Vector3 diffVec = (endVec - startVec);
        G3D::Vector3 prevVec = startVec;
        float len = diffVec.length();
        diffVec *= SMOOTH_PATH_STEP_SIZE / len;
        while (len > SMOOTH_PATH_STEP_SIZE)
        {
            len -= SMOOTH_PATH_STEP_SIZE;
            prevVec += diffVec;
            pathPoints[VERTEX_SIZE * pointCount + 0] = prevVec.x;
            pathPoints[VERTEX_SIZE * pointCount + 1] = prevVec.y;
            pathPoints[VERTEX_SIZE * pointCount + 2] = prevVec.z;
            ++pointCount;
        }

        memcpy(&pathPoints[VERTEX_SIZE * pointCount], endPoint, sizeof(float) * 3); // last point
        ++pointCount;
    }
    else if (m_useStraightPath)
    {
        dtResult = m_navMeshQuery->findStraightPath(
                       startPoint,         // start position
                       endPoint,           // end position
                       m_pathPolyRefs.data(), // current path
                       m_polyLength,       // lenth of current path
                       pathPoints,         // [out] path corner points
                       nullptr,               // [out] flags
                       nullptr,               // [out] shortened path
                       (int*)&pointCount,
                       m_pointPathLimit);   // maximum number of points/polygons to use
    }
    else
    {
        dtResult = findSmoothPath(
                       startPoint,         // start position
                       endPoint,           // end position
                       m_pathPolyRefs.data(), // current path
                       m_polyLength,       // length of current path
                       pathPoints,         // [out] path corner points
                       (int*)&pointCount,
                       m_pointPathLimit);    // maximum number of points
    }

    if (pointCount < 2 || dtStatusFailed(dtResult))
    {
        // only happens if pass bad data to findStraightPath or navmesh is broken
        // single point paths can be generated here
        // TODO : check the exact cases
        DEBUG_FILTER_LOG(LOG_FILTER_PATHFINDING, "++ PathFinder::BuildPointPath FAILED! path sized %d returned\n", pointCount);
        BuildShortcut();
        m_type = PATHFIND_NOPATH;
        return;
    }
	else if (pointCount == m_pointPathLimit)
	{
		DEBUG_FILTER_LOG(LOG_FILTER_PATHFINDING, "BuildPointPath FAILED! path sized %d returned, lower than limit set to %d\n", pointCount, m_pointPathLimit);
		BuildShortcut();
		m_type = PATHFIND_SHORT;
		return;
	}

    if (pointCount == m_pointPathLimit)
    {
        DEBUG_FILTER_LOG(LOG_FILTER_PATHFINDING, "++ PathFinder::BuildPointPath FAILED! path sized %d returned, lower than limit set to %d\n", pointCount, m_pointPathLimit);
        BuildShortcut();
        m_type = PATHFIND_SHORT;
        return;
    }

    if (pointCount > 2 && sWorld.getConfig(CONFIG_BOOL_PATH_FIND_OPTIMIZE))
    {
        uint32 tempPointCounter = 2;

        PointsArray tempPathPoints;
        tempPathPoints.resize(pointCount);

        for (uint32 i = 0; i < pointCount; ++i)      // y, z, x  expected here
        {
            uint32 pointPos = i * VERTEX_SIZE;
            tempPathPoints[i] = Vector3(pathPoints[pointPos + 2], pathPoints[pointPos], pathPoints[pointPos + 1]);
        }

        // Optimize points
        Vector3 emptyVec = { 0.0f, 0.0f, 0.0f };

        uint8 cutLimit = 0;
        for (uint32 i = 1; i < pointCount - 1; ++i)
        {
            G3D::Vector3 p  = tempPathPoints[i];     // Point
            G3D::Vector3 p1 = tempPathPoints[i - 1]; // PrevPoint
            G3D::Vector3 p2 = tempPathPoints[i + 1]; // NextPoint

            float lineLen = (p1.y - p2.y) * p.x + (p2.x - p1.x) * p.y + (p1.x * p2.y - p2.x * p1.y);

            if (fabs(lineLen) < LINE_FAULT && cutLimit < SKIP_POINT_LIMIT)
            {
                tempPathPoints[i] = emptyVec;
                cutLimit++;
            }
            else
            {
                tempPointCounter++;
                cutLimit = 0;
            }
        }

        m_pathPoints.resize(tempPointCounter);

        uint32 pointPos = 0;
        for (uint32 i = 0; i < pointCount; ++i)
        {
            if (tempPathPoints[i] != emptyVec)
            {
                m_pathPoints[pointPos] = tempPathPoints[i];
                pointPos++;
            }
        }

        pointCount = tempPointCounter;
    }
    else
    {
        m_pathPoints.resize(pointCount);
        for (uint32 i = 0; i < pointCount; ++i)
        {
            uint32 pointPos = i * VERTEX_SIZE;
            m_pathPoints[i] = { pathPoints[pointPos + 2], pathPoints[pointPos], pathPoints[pointPos + 1] };
        }
    }

    // first point is always our current location - we need the next one
    setActualEndPosition(m_pathPoints[pointCount - 1]);

    // force the given destination, if needed
    if (m_forceDestination &&
            (!(m_type & PATHFIND_NORMAL) || !inRange(getEndPosition(), getActualEndPosition(), 1.0f, 1.0f)))
    {
        // we may want to keep partial subpath
        if (dist3DSqr(getActualEndPosition(), getEndPosition()) < 0.3f * dist3DSqr(getStartPosition(), getEndPosition()))
        {
            setActualEndPosition(getEndPosition());
            m_pathPoints[m_pathPoints.size() - 1] = getEndPosition();
        }
        else
        {
            setActualEndPosition(getEndPosition());
            BuildShortcut();
        }

        m_type = PathType(PATHFIND_NORMAL | PATHFIND_NOT_USING_PATH);
    }

    NormalizePath();

    DEBUG_FILTER_LOG(LOG_FILTER_PATHFINDING, "++ PathFinder::BuildPointPath path type %d size %d poly-size %d\n", m_type, pointCount, m_polyLength);
}

void PathFinder::NormalizePath()
{
    if (!sWorld.getConfig(CONFIG_BOOL_PATH_FIND_NORMALIZE_Z))
        return;

    GenericTransport* transport = m_sourceUnit->GetTransport();

    for (auto& m_pathPoint : m_pathPoints)
    {
        if (transport)
            transport->CalculatePassengerPosition(m_pathPoint.x, m_pathPoint.y, m_pathPoint.z);
        m_sourceUnit->UpdateAllowedPositionZ(m_pathPoint.x, m_pathPoint.y, m_pathPoint.z);
        if (transport)
            transport->CalculatePassengerOffset(m_pathPoint.x, m_pathPoint.y, m_pathPoint.z);
    }
}

void PathFinder::BuildShortcut()
{
    DEBUG_FILTER_LOG(LOG_FILTER_PATHFINDING, "++ PathFinder::BuildShortcut :: making shortcut\n");

    clear();

    // make two point path, our curr pos is the start, and dest is the end
    m_pathPoints.resize(2);

    // set start and a default next position
    m_pathPoints[0] = getStartPosition();
    m_pathPoints[1] = getActualEndPosition();

    NormalizePath();

    m_type = PATHFIND_SHORTCUT;
}

void PathFinder::createFilter()
{
    uint16 includeFlags = 0;
    uint16 excludeFlags = 0;

    if (m_sourceUnit->GetTypeId() == TYPEID_UNIT)
    {
        Creature* creature = (Creature*)m_sourceUnit;
        if (creature->CanWalk())
            includeFlags |= NAV_GROUND;          // walk

        // creatures don't take environmental damage
        if (creature->CanSwim())
            includeFlags |= (NAV_WATER | NAV_MAGMA | NAV_SLIME);           // swim
    }
    else if (m_sourceUnit->GetTypeId() == TYPEID_PLAYER)
    {
        // perfect support not possible, just stay 'safe'
        includeFlags |= (NAV_GROUND | NAV_WATER);
    }

    m_filter.setIncludeFlags(includeFlags);
    m_filter.setExcludeFlags(excludeFlags);

    updateFilter();
}

void PathFinder::updateFilter()
{
    // allow creatures to cheat and use different movement types if they are moved
    // forcefully into terrain they can't normally move in
    if (m_sourceUnit->IsInWater() || m_sourceUnit->IsUnderwater())
    {
        uint16 includedFlags = m_filter.getIncludeFlags();
        includedFlags |= getNavTerrain(m_sourceUnit->GetPositionX(),
                                       m_sourceUnit->GetPositionY(),
                                       m_sourceUnit->GetPositionZ());

        m_filter.setIncludeFlags(includedFlags);
    }
}

NavTerrain PathFinder::getNavTerrain(float x, float y, float z) const
{
    GridMapLiquidData data;
    if (m_sourceUnit->GetTerrain()->getLiquidStatus(x, y, z, MAP_ALL_LIQUIDS, &data) == LIQUID_MAP_NO_WATER)
        return NAV_GROUND;

    switch (data.type_flags)
    {
        case MAP_LIQUID_TYPE_WATER:
        case MAP_LIQUID_TYPE_OCEAN:
            return NAV_WATER;
        case MAP_LIQUID_TYPE_MAGMA:
            return NAV_MAGMA;
        case MAP_LIQUID_TYPE_SLIME:
            return NAV_SLIME;
        default:
            return NAV_GROUND;
    }
}

bool PathFinder::HaveTile(const Vector3& p) const
{
    if (m_sourceUnit->GetTransport())
        return true;

    int tx = -1, ty = -1;
    float point[VERTEX_SIZE] = {p.y, p.z, p.x};

    m_navMesh->calcTileLoc(point, &tx, &ty);

    /// Workaround
    /// For some reason, often the tx and ty variables wont get a valid value
    /// Use this check to prevent getting negative tile coords and crashing on getTileAt
    if (tx == -1 || ty == -1)
        return false;

    return (m_navMesh->getTileAt(tx, ty, 0) != nullptr); // Don't use layer so always set to 0
}

uint32 PathFinder::fixupCorridor(dtPolyRef* path, uint32 npath, uint32 maxPath, dtPolyRef const* visited, uint32 nvisited)
{
    int32 furthestPath = -1;
    int32 furthestVisited = -1;

    // Find furthest common polygon.
    for (int32 i = npath - 1; i >= 0; --i)
    {
        bool found = false;
        for (int32 j = nvisited - 1; j >= 0; --j)
        {
            if (path[i] == visited[j])
            {
                furthestPath = i;
                furthestVisited = j;
                found = true;
            }
        }
        if (found)
            break;
    }

    // If no intersection found just return current path.
    if (furthestPath == -1 || furthestVisited == -1)
        return npath;

    // Concatenate paths.

    // Adjust beginning of the buffer to include the visited.
    uint32 req = nvisited - furthestVisited;
    uint32 orig = uint32(furthestPath + 1) < npath ? furthestPath + 1 : npath;
    uint32 size = npath > orig ? npath - orig : 0;
    if (req + size > maxPath)
        size = maxPath - req;

    if (req >= maxPath)
        return npath;

    if (size)
        memmove(path + req, path + orig, size * sizeof(dtPolyRef));

    // Store visited
    for (uint32 i = 0; i < req; ++i)
        path[i] = visited[(nvisited - 1) - i];

    return req + size;
}

bool PathFinder::getSteerTarget(const float* startPos, const float* endPos,
                                float minTargetDist, const dtPolyRef* path, uint32 pathSize,
                                float* steerPos, unsigned char& steerPosFlag, dtPolyRef& steerPosRef) const
{
    // Find steer target.
    static const uint32 MAX_STEER_POINTS = 3;
    float steerPath[MAX_STEER_POINTS * VERTEX_SIZE];
    unsigned char steerPathFlags[MAX_STEER_POINTS];
    dtPolyRef steerPathPolys[MAX_STEER_POINTS];
    uint32 nsteerPath = 0;
    dtStatus dtResult = m_navMeshQuery->findStraightPath(startPos, endPos, path, pathSize,
                        steerPath, steerPathFlags, steerPathPolys, (int*)&nsteerPath, MAX_STEER_POINTS);
    if (!nsteerPath || dtStatusFailed(dtResult))
        return false;

    // Find vertex far enough to steer to.
    uint32 ns = 0;
    while (ns < nsteerPath)
    {
        // Stop at Off-Mesh link or when point is further than slop away.
        if ((steerPathFlags[ns] & DT_STRAIGHTPATH_OFFMESH_CONNECTION) ||
                !inRangeYZX(&steerPath[ns * VERTEX_SIZE], startPos, minTargetDist, 1000.0f))
            break;
        ++ns;
    }
    // Failed to find good point to steer to.
    if (ns >= nsteerPath)
        return false;

    dtVcopy(steerPos, &steerPath[ns * VERTEX_SIZE]);
    steerPos[1] = startPos[1];  // keep Z value
    steerPosFlag = steerPathFlags[ns];
    steerPosRef = steerPathPolys[ns];

    return true;
}

dtStatus PathFinder::findSmoothPath(const float* startPos, const float* endPos,
                                    const dtPolyRef* polyPath, uint32 polyPathSize,
                                    float* smoothPath, int* smoothPathSize, uint32 maxSmoothPathSize)
{
    *smoothPathSize = 0;
    uint32 nsmoothPath = 0;

    if (m_pointPathLimit > m_smoothPathPolyRefs.size())
        m_smoothPathPolyRefs.resize(m_pointPathLimit);
    memcpy(m_smoothPathPolyRefs.data(), polyPath, polyPathSize * sizeof(dtPolyRef));
    uint32 npolys = polyPathSize;

    float iterPos[VERTEX_SIZE];
    if (dtStatusFailed(m_navMeshQuery->closestPointOnPolyBoundary(m_smoothPathPolyRefs[0], startPos, iterPos)))
        return DT_FAILURE;

    float targetPos[VERTEX_SIZE];
    if (dtStatusFailed(m_navMeshQuery->closestPointOnPolyBoundary(m_smoothPathPolyRefs[npolys - 1], endPos, targetPos)))
        return DT_FAILURE;

    dtVcopy(&smoothPath[nsmoothPath * VERTEX_SIZE], iterPos);
    ++nsmoothPath;

    // Move towards target a small advancement at a time until target reached or
    // when ran out of memory to store the path.
    while (npolys && nsmoothPath < maxSmoothPathSize)
    {
        // Find location to steer towards.
        float steerPos[VERTEX_SIZE];
        unsigned char steerPosFlag;
        dtPolyRef steerPosRef = INVALID_POLYREF;

        if (!getSteerTarget(iterPos, targetPos, SMOOTH_PATH_SLOP, m_smoothPathPolyRefs.data(), npolys, steerPos, steerPosFlag, steerPosRef))
            break;

        const bool endOfPath = (steerPosFlag & DT_STRAIGHTPATH_END) != 0;
        const bool offMeshConnection = (steerPosFlag & DT_STRAIGHTPATH_OFFMESH_CONNECTION) != 0;

        // Find movement delta.
        float delta[VERTEX_SIZE];
        dtVsub(delta, steerPos, iterPos);
        float len = dtMathSqrtf(dtVdot(delta, delta));
        // If the steer target is end of path or off-mesh link, do not move past the location.
        if ((endOfPath || offMeshConnection) && len < SMOOTH_PATH_STEP_SIZE)
            len = 1.0f;
        else
            len = SMOOTH_PATH_STEP_SIZE / len;

        float moveTgt[VERTEX_SIZE];
        dtVmad(moveTgt, iterPos, delta, len);

        // Move
        float result[VERTEX_SIZE];
        const static uint32 MAX_VISIT_POLY = 16;
        dtPolyRef visited[MAX_VISIT_POLY];

        uint32 nvisited = 0;
        m_navMeshQuery->moveAlongSurface(m_smoothPathPolyRefs[0], iterPos, moveTgt, &m_filter, result, visited, (int*)&nvisited, MAX_VISIT_POLY);
        npolys = fixupCorridor(m_smoothPathPolyRefs.data(), npolys, m_pointPathLimit, visited, nvisited);

        m_navMeshQuery->getPolyHeight(m_smoothPathPolyRefs[0], result, &result[1]);
        result[1] += 0.5f;
        dtVcopy(iterPos, result);

        // Handle end of path and off-mesh links when close enough.
        if (endOfPath && inRangeYZX(iterPos, steerPos, SMOOTH_PATH_SLOP, 1.0f))
        {
            // Reached end of path.
            dtVcopy(iterPos, targetPos);
            if (nsmoothPath < maxSmoothPathSize)
            {
                dtVcopy(&smoothPath[nsmoothPath * VERTEX_SIZE], iterPos);
                ++nsmoothPath;
            }
            break;
        }
        if (offMeshConnection && inRangeYZX(iterPos, steerPos, SMOOTH_PATH_SLOP, 1.0f))
        {
            // Advance the path up to and over the off-mesh connection.
            dtPolyRef prevRef = INVALID_POLYREF;
            dtPolyRef polyRef = m_smoothPathPolyRefs[0];
            uint32 npos = 0;
            while (npos < npolys && polyRef != steerPosRef)
            {
                prevRef = polyRef;
                polyRef = m_smoothPathPolyRefs[npos];
                ++npos;
            }

            for (uint32 i = npos; i < npolys; ++i)
                m_smoothPathPolyRefs[i - npos] = m_smoothPathPolyRefs[i];

            npolys -= npos;

            // Handle the connection.
            float newStartPos[VERTEX_SIZE], newEndPos[VERTEX_SIZE];
            if (dtStatusSucceed(m_navMesh->getOffMeshConnectionPolyEndPoints(prevRef, polyRef, newStartPos, newEndPos)))
            {
                if (nsmoothPath < maxSmoothPathSize)
                {
                    dtVcopy(&smoothPath[nsmoothPath * VERTEX_SIZE], startPos);
                    ++nsmoothPath;
                }
                // Move position at the other side of the off-mesh link.
                dtVcopy(iterPos, endPos);

                m_navMeshQuery->getPolyHeight(m_smoothPathPolyRefs[0], iterPos, &iterPos[1]);
                iterPos[1] += 0.5f;
            }
        }

        // Store results.
        if (nsmoothPath < maxSmoothPathSize)
        {
            dtVcopy(&smoothPath[nsmoothPath * VERTEX_SIZE], iterPos);
            ++nsmoothPath;
        }
    }

    *smoothPathSize = nsmoothPath;

    // this is most likely a loop
    return nsmoothPath < m_pointPathLimit ? DT_SUCCESS : DT_FAILURE;
}

bool PathFinder::inRangeYZX(const float* v1, const float* v2, float r, float h) const
{
    const float dx = v2[0] - v1[0];
    const float dy = v2[1] - v1[1]; // elevation
    const float dz = v2[2] - v1[2];
    return (dx * dx + dz * dz) < r * r && fabsf(dy) < h;
}

bool PathFinder::inRange(const Vector3& p1, const Vector3& p2, float r, float h) const
{
    Vector3 d = p1 - p2;
    return (d.x * d.x + d.y * d.y) < r * r && fabsf(d.z) < h;
}

float PathFinder::dist3DSqr(const Vector3& p1, const Vector3& p2) const
{
    return (p1 - p2).squaredLength();
}
