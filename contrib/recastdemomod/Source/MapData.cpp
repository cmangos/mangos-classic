#include "MapData.h"

void MapInfos::Init(unsigned int mapId, BuildContext* ctx)
{
    if (m_Initilized)
        return;

    m_MMapData.Init(mapId, ctx);
    m_GeomData.Init(mapId, ctx);
    m_Initilized = true;
}

bool MapInfos::LoadTile(unsigned int tx, unsigned int ty, bool loadGeom /*= true*/, bool loadMmap /*= true */)
{
    if (!m_Initilized)
        return false;

    MeshObjects const* newObj = NULL;
    if (loadGeom)
    {
        newObj = m_GeomData.LoadTile(tx, ty);
        if (newObj)
        {
            rcVcopy(m_BMin, m_GeomData.BMin());
            rcVcopy(m_BMax, m_GeomData.BMax());
        }
    }

    bool mMapTileFound = false;
    if (loadMmap && m_MMapData.Initialized())
    {
        if (m_GeomData.NoMapFile())
        {
            if (newObj && newObj->GetVMap())
                mMapTileFound = m_MMapData.LoadTile(newObj->GetVMap()->BMin(), newObj->GetVMap()->BMax());
        }
        else
            mMapTileFound = m_MMapData.LoadTile(tx, ty);

        if (mMapTileFound && !newObj)
        {
            rcVcopy(m_BMin, m_MMapData.BMin());
            rcVcopy(m_BMax, m_MMapData.BMax());
        }
    }
    if (newObj || mMapTileFound)
        return true;

    return false;
}

bool MapInfos::LoadModel(const std::string modelName, const std::string objId)
{
    if (!m_Initilized)
        return false;

    MeshObjects const* newObj = m_GeomData.LoadModel(modelName);

    std::string filename = "go" + objId;


    m_MMapData.LoadObjectNavMesh(filename);

    return true;
}

void MapInfos::RemoveTile(unsigned int tx, unsigned int ty)
{
    m_GeomData.RemoveTile(tx, ty);
    m_MMapData.RemoveTile(tx, ty);
    if (!m_GeomData.IsEmpty())
    {
        rcVcopy(m_BMin, m_GeomData.BMin());
        rcVcopy(m_BMax, m_GeomData.BMax());
    }
    else if (!m_MMapData.IsEmpty())
    {
        rcVcopy(m_BMin, m_MMapData.BMin());
        rcVcopy(m_BMax, m_MMapData.BMax());
    }
}

void MapInfos::RemoveAllTiles()
{
    m_GeomData.RemoveAllTiles();
    m_MMapData.RemoveAllTiles();
}

void MapInfos::BuildNavMeshOfTile(unsigned int tx, unsigned int ty, rcConfig* cfg, SamplePartitionType partitionType)
{
    MeshObjects const* mo = m_GeomData.GetMeshObjects(tx, ty);
    if (!mo)
        return;

    if (m_GeomData.NoMapFile())
    {
        mo = m_GeomData.GetMeshObjects(64, 64);
        if (!mo)
            return;

        float const* bmin = mo->BMin();
        float const* bmax = mo->BMax();
        // convert coord bounds to grid bounds
        unsigned int minX, minY, maxX, maxY;
        maxX = 32 - bmin[0] / RecastDemo::BLOCK_SIZE;
        maxY = 32 - bmin[2] / RecastDemo::BLOCK_SIZE;
        minX = 32 - bmax[0] / RecastDemo::BLOCK_SIZE;
        minY = 32 - bmax[2] / RecastDemo::BLOCK_SIZE;

        // add all tiles within bounds to tile list.
        for (unsigned int i = minX; i <= maxX; ++i)
            for (unsigned int j = minY; j <= maxY; ++j)
                m_MMapData.BuildMoveMapTile(j, i, mo, cfg, partitionType);
    }
    else
        m_MMapData.BuildMoveMapTile(tx, ty, mo, cfg, partitionType);
}

bool MapInfos::LoadNavMeshOfTile(unsigned int tx, unsigned int ty)
{
    if (m_GeomData.NoMapFile())
    {
        MeshObjects const* mo = m_GeomData.GetMeshObjects(tx, ty);
        if (mo && mo->GetVMap())
            return m_MMapData.LoadTile(mo->GetVMap()->BMin(), mo->GetVMap()->BMax());
        return false;
    }
    else
        return m_MMapData.LoadTile(tx, ty);
}
