#ifndef MAPINFOS_H
#define MAPINFOS_H

#include "MMapData.h"
#include "GeomData.h"

class MapInfos
{
public:
    MapInfos() : m_Initilized(false){}
    void Init(unsigned int mapId, BuildContext* ctx);
    bool LoadTile(unsigned int tx, unsigned int ty, bool loadGeom = true, bool loadMmap = true);
    bool LoadModel(const std::string modelName, const std::string objId);
    void RemoveTile(unsigned int tx, unsigned int ty);
    void ClearNavMeshOfTile(unsigned int tx, unsigned int ty) { m_MMapData.RemoveTile(tx, ty); };
    bool LoadNavMeshOfTile(unsigned int tx, unsigned int ty);
    void BuildNavMeshOfTile(unsigned int tx, unsigned int ty, rcConfig* cfg, SamplePartitionType partitionType);
    void RemoveAllTiles();

    inline dtNavMesh*            GetNavMesh() { return m_MMapData.GetNavMesh(); }
    inline dtNavMeshQuery*       GetNavMeshQuery() { return m_MMapData.GetNavMeshQuery(); }
    inline MeshObjectsMap const* GetGeomsMap() const { return m_GeomData.GetGeomsMap(); }
    inline MeshObjects const*    GetMeshObjects(unsigned int tx, unsigned int ty) const { return m_GeomData.GetMeshObjects(tx, ty); };
    inline bool                  GetNavMeshTileBounds(unsigned int tx, unsigned int ty, float const* &bmin, float const* &bmax) { return m_MMapData.GetTileBounds(tx, ty, bmin, bmax); };
    inline bool                  IsEmpty() const { return m_GeomData.IsEmpty() && m_MMapData.IsEmpty(); }
    inline dtTileRef             GetTileRef(unsigned int tx, unsigned int ty) { return m_MMapData.GetTileRef(tx, ty); }
    inline bool                  RaycastMesh(float* src, float* dst, float& tmin) const { return m_GeomData.RaycastMesh(src, dst, tmin); }
    inline float const*          BMin() const { return m_BMin; }
    inline float const*          BMax() const { return m_BMax; }

private:
    MMapData     m_MMapData;
    GeomData     m_GeomData;
    float        m_BMin[3];
    float        m_BMax[3];
    bool         m_Initilized;
};

#endif