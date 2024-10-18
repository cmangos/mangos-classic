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

#ifndef MANGOS_PATH_FINDER_H
#define MANGOS_PATH_FINDER_H

#include "MoveMapSharedDefines.h"

#include <Detour/Include/DetourNavMesh.h>
#include <Detour/Include/DetourNavMeshQuery.h>

#include "Movement/MoveSplineInitArgs.h"

using Movement::Vector3;
using Movement::PointsArray;

class Unit;

// 74*4.0f=296y  number_of_points*interval = max_path_len
// this is way more than actual evade range
// I think we can safely cut those down even more
#ifdef ENABLE_PLAYERBOTS
// This value is doubled from the original and then used only half by findpath. 
// If the same value is used by findpath and findsmooth path no result will be found by the second at max length.
#define MAX_PATH_LENGTH         148 
#define MAX_POINT_PATH_LENGTH   148
#else
#define MAX_PATH_LENGTH         74
#define MAX_POINT_PATH_LENGTH   74
#endif

#define SMOOTH_PATH_STEP_SIZE   4.0f
#define SMOOTH_PATH_SLOP        0.3f

// How many points can be cutted
// May occupt visual bugs when lenght > 20y
#define SKIP_POINT_LIMIT        6
#define LINE_FAULT              0.5f
#define VERTEX_SIZE       3
#define INVALID_POLYREF   0

// bound box of poly search area
static float NearPolySearchBound[VERTEX_SIZE] = { 5.0f, 5.0f, 5.0f };
static float FarPolySearchBound[VERTEX_SIZE] = { 10.0f, 10.0f, 10.0f };

enum PathType
{
    PATHFIND_BLANK          = 0x0000,   // path not built yet
    PATHFIND_NORMAL         = 0x0001,   // normal path
    PATHFIND_SHORTCUT       = 0x0002,   // travel through obstacles, terrain, air, etc (old behavior)
    PATHFIND_INCOMPLETE     = 0x0004,   // we have partial path to follow - getting closer to target
    PATHFIND_NOPATH         = 0x0008,   // no valid path at all or error in generating one
    PATHFIND_NOT_USING_PATH = 0x0010,   // used when we are either flying/swiming or on map w/o mmaps
    PATHFIND_SHORT          = 0x0020,   // path is longer or equal to its limited path length
};

class PathFinder
{
    public:
        PathFinder(Unit const* owner, bool ignoreNormalization = false);
        ~PathFinder();

        // Calculate the path from owner to given destination
        // return: true if new path was calculated, false otherwise (no change needed)
        bool calculate(float destX, float destY, float destZ, bool forceDest = false, bool straightLine = false); // transfers coorddinates from global to local space if on transport - use other func if coords are already in transport space
        bool calculate(Vector3 const& start, Vector3 const& dest, bool forceDest = false, bool straightLine = false);

        // compute a straight path to some random point in max range
        void ComputePathToRandomPoint(Vector3 const& startPoint, float maxRange);

        // option setters - use optional
        void setUseStrightPath(bool useStraightPath) { m_useStraightPath = useStraightPath; };
        void setPathLengthLimit(float distance) { m_pointPathLimit = std::min<uint32>(uint32(distance / SMOOTH_PATH_STEP_SIZE * 1.25f), MAX_POINT_PATH_LENGTH); };

        // result getters
        Vector3 getStartPosition()      const { return m_startPosition; }
        Vector3 getEndPosition()        const { return m_endPosition; }
        Vector3 getActualEndPosition()  const { return m_actualEndPosition; }

        PointsArray& getPath() { return m_pathPoints; }
        PathType getPathType() const { return m_type; }

#ifdef ENABLE_PLAYERBOTS
        PathFinder();
        PathFinder(uint32 mapId, uint32 instanceId = 0);
        void setArea(uint32 mapId, float x, float y, float z, uint32 area = 1, float range = 10.0f);
        uint32 getArea(uint32 mapId, float x, float y, float z);
        unsigned short getFlags(uint32 mapId, float x, float y, float z);
        void setAreaCost(uint32 area = 1, float cost = 0.0f);
#endif

    private:

        PointsArray    m_pathPoints;       // our actual (x,y,z) path to the target
        PathType       m_type;             // tells what kind of path this is

        bool           m_useStraightPath;  // type of path will be generated
        bool           m_forceDestination; // when set, we will always arrive at given point
        bool           m_straightLine;     // use raycast if true for a straight line path
        uint32         m_pointPathLimit;   // limit point path size; min(this, MAX_POINT_PATH_LENGTH)
        std::vector<float> m_cachedPoints; // caching for BuildPointPath - must be after m_pointPathLimit

        std::vector<dtPolyRef> m_pathPolyRefs;       // array of detour polygon references
        uint32         m_polyLength;                 // number of polygons in the path
        std::vector<dtPolyRef> m_smoothPathPolyRefs; // caching for findSmoothPath

        Vector3        m_startPosition;    // {x, y, z} of current location
        Vector3        m_endPosition;      // {x, y, z} of the destination
        Vector3        m_actualEndPosition;// {x, y, z} of the closest possible point to given destination

        const Unit* const       m_sourceUnit;       // the unit that is moving
        const dtNavMesh*        m_navMesh;          // the nav mesh
        const dtNavMeshQuery*   m_navMeshQuery;     // the nav mesh query used to find the path

        const dtNavMeshQuery*   m_defaultNavMeshQuery;     // the nav mesh query used to find the path
        uint32                  m_defaultMapId;
#ifdef ENABLE_PLAYERBOTS
        uint32                  m_defaultInstanceId;
#endif

        bool                    m_ignoreNormalization;

        dtQueryFilter m_filter;                     // use single filter for all movements, update it when needed

        void setStartPosition(const Vector3& point) { m_startPosition = point; }
        void setEndPosition(const Vector3& point) { m_actualEndPosition = point; m_endPosition = point; }
        void setActualEndPosition(const Vector3& point) { m_actualEndPosition = point; }
        void NormalizePath();
        void SetCurrentNavMesh();

        void clear()
        {
            m_polyLength = 0;
            m_pathPoints.clear();
        }

        bool inRange(const Vector3& p1, const Vector3& p2, float r, float h) const;
        float dist3DSqr(const Vector3& p1, const Vector3& p2) const;
        bool inRangeYZX(const float* v1, const float* v2, float r, float h) const;

        dtPolyRef getPathPolyByPosition(const dtPolyRef* polyPath, uint32 polyPathSize, const float* point, float* distance = nullptr, const float maxDist = 3.0f) const;
        dtPolyRef getPolyByLocation(const float* point, float* distance);
        bool HaveTile(const Vector3& p) const;

        void BuildPolyPath(const Vector3& startPos, const Vector3& endPos);
        void BuildPointPath(const float* startPoint, const float* endPoint);
        void BuildShortcut();
#ifdef ENABLE_PLAYERBOTS
        bool IsPointHigherThan(const Vector3& posOne, const Vector3& posTwo);
#endif

        NavTerrainFlag getNavTerrain(float x, float y, float z) const;
        void createFilter();
        void updateFilter();

        // smooth path aux functions
        uint32 fixupCorridor(dtPolyRef* path, uint32 npath, uint32 maxPath,
                             const dtPolyRef* visited, uint32 nvisited);
        bool getSteerTarget(const float* startPos, const float* endPos, float minTargetDist,
                            const dtPolyRef* path, uint32 pathSize, float* steerPos,
                            unsigned char& steerPosFlag, dtPolyRef& steerPosRef) const;
        dtStatus findSmoothPath(const float* startPos, const float* endPos,
                                const dtPolyRef* polyPath, uint32 polyPathSize,
                                float* smoothPath, int* smoothPathSize, uint32 maxSmoothPathSize);
};

#endif
