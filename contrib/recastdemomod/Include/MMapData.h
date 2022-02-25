#ifndef MMAPDATA_H
#define MMAPDATA_H

#include <map>
#include "DetourNavMesh.h"
#include "SampleInterfaces.h"
#include "DetourNavMeshQuery.h"
#include "..\..\..\..\contrib\mmap\src\TerrainBuilder.h"
#include "GeomData.h"
#include "RecastDemoSharedDefines.h"

struct TileInfos
{
    dtTileRef tileRef;
    float     bMin[3];
    float     bMax[3];
};

class MMapData
{
    typedef std::map<unsigned int, TileInfos> TilesRefMap;
public:
    MMapData();
    ~MMapData();

    dtNavMesh* Init(unsigned int mapId, BuildContext* ctx);
    bool       LoadNavmesh(const char* fileName, TileInfos& tInfos);
    bool       LoadObjectNavMesh(const std::string fileName);
    bool       LoadTile(unsigned int tx, unsigned int ty);
    bool       LoadTile(float const* bmin, float const* bmax);
    bool       RemoveTile(unsigned int tx, unsigned int ty);
    void       RemoveAllTiles();
    dtTileRef  GetTileRef(unsigned int tx, unsigned int ty) const;
    bool       GetTileBounds(unsigned int tx, unsigned int ty, float const* &bmin, float const* &bmax) const;
    void       BuildNavMeshOfTile(unsigned int tx, unsigned int ty, MeshObjects const* meshObjects, rcConfig const* cfg, float agentHeight, float agentRadius, float agentClimb, SamplePartitionType partitionType);
    void       BuildMoveMapTile(unsigned int tileX, unsigned int tileY, MeshObjects const* meshObjects, rcConfig* cfg, SamplePartitionType partitionType);

    inline dtNavMesh*            GetNavMesh() { return m_NavMesh; }
    inline dtNavMeshQuery*       GetNavMeshQuery() { return m_NavQuery; }
    inline unsigned int          GetTileCount() const { return m_TilesInfos.size(); }
    inline bool                  IsEmpty() const { return m_TilesInfos.empty(); }
    inline bool                  Initialized() const { return m_Initialized; }
    inline float const*          BMin() const { return m_BMin; }
    inline float const*          BMax() const { return m_BMax; }

private:
    void Cleanup();
    unsigned int        m_MapId;
    dtNavMesh*          m_NavMesh;
    dtNavMeshQuery*     m_NavQuery;
    BuildContext*       m_Ctx;
    bool                m_Initialized;
    bool                m_MultipleTile;
    TilesRefMap         m_TilesInfos;
    float               m_BMin[3];
    float               m_BMax[3];

    unsigned char*      m_triareas;
    rcHeightfield*      m_solid;
    rcCompactHeightfield* m_chf;
    rcContourSet*       m_cset;
    rcPolyMesh*         m_pmesh;
    rcPolyMeshDetail*   m_dmesh;
};

#endif
