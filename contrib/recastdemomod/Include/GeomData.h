#ifndef GEOMDATA_H
#define GEOMDATA_H

#include <list>
#include <map>
#include "DetourNavMesh.h"
#include "DetourNavMeshQuery.h"
#include "SampleInterfaces.h"
#include "ChunkyTriMesh.h"
#include "..\..\..\..\contrib\mmap\src\TerrainBuilder.h"

class MeshDetails
{
public:
    MeshDetails(float const* verts, int const* tris, unsigned int vCount, unsigned int tCount, unsigned char* flags = NULL);

    ~MeshDetails();

    inline float const*           Verts() const { return m_Verts; }
    inline int const*             Tris() const { return m_Tris; }
    inline float const*           Normals() const { return m_Normals; }
    inline unsigned int           VertCount() const { return m_VertCount; }
    inline unsigned int           TrisCount() const { return m_TrisCount; }
    inline rcChunkyTriMesh const* GetChunckyTM() const { return m_chunkyMesh; }
    inline unsigned char*         GetFlags() const { return m_Flags; }

private:
    MeshDetails() {};
    void ComputeNormals();
    void CreateChunckyTriMesh();
    float const*          m_Verts;
    int const*            m_Tris;
    float*                m_Normals;
    unsigned int          m_VertCount;
    unsigned int          m_TrisCount;
    rcChunkyTriMesh*      m_chunkyMesh;
    unsigned char*        m_Flags;
    bool                  m_LocalFlags;
};

class MeshInfos
{
public:
    MeshInfos(MeshDetails const* sMesh, MeshDetails const* lMesh, float const* bmin, float const* bmax);

    inline MeshDetails const* GetSolidMesh() const { return m_SolidMesh; }
    inline MeshDetails const* GetLiquidMesh() const { return m_LiquidMesh; }
    inline float const*       BMin() const { return m_BMin; }
    inline float const*       BMax() const { return m_BMax; }

private:
    MeshDetails const* m_SolidMesh;
    MeshDetails const* m_LiquidMesh;
    float              m_BMin[3];
    float              m_BMax[3];

};

enum MeshObjectType
{
    MESH_OBJECT_TYPE_TILE,
    MESH_OBJECT_TYPE_OBJECT
};

class MeshObjects
{
public:
    MeshObjects(unsigned int mapId, unsigned int tx, unsigned int ty, BuildContext* ctx);
    MeshObjects(const std::string modelName, BuildContext* ctx);
    ~MeshObjects();

    inline MeshInfos const* GetMap() const { return m_MapInfos; }
    inline MeshInfos const* GetVMap() const { return m_VMapInfos; }
    inline MeshInfos const* GetModel() const { return m_ModelInfos; }

    inline unsigned int     GetTileX() const { return m_TileX; }
    inline unsigned int     GetTileY() const { return m_TileY; }
    inline float const*     BMin() const { return m_BMin; }
    inline float const*     BMax() const { return m_BMax; }
    bool                    GetMeshData(G3D::Array<float> &sVerts, G3D::Array<int> &sTris, G3D::Array<float> &lVerts, G3D::Array<int> &lTris, G3D::Array<unsigned char> &liquidFlags) const;
    static void             GetMeshBounds(float const* verts, int vertCount, float* bmin, float* bmax);
    static void             GetTileBounds(unsigned int tileX, unsigned int tileY, float const* verts, int vertCount, float* bmin, float* bmax);
    MeshObjectType          GetMeshObjectType() const { return m_meshType; }

private:
    void LoadMap();
    void LoadVMap();
    void LoadObject();
    void MergeMeshArrays(G3D::Array<float>& dstVerts, G3D::Array<int>& dstTris, G3D::Array<float> const& srcVerts, G3D::Array<int> const& srcTris) const;
    void MergeIndices(G3D::Array<int> &dst, G3D::Array<int> const* src, unsigned int offset) const;
    MeshInfos*     m_MapInfos;
    MeshInfos*     m_VMapInfos;
    MeshInfos*     m_ModelInfos;
    MMAP::MeshData m_MapMesh;
    MMAP::MeshData m_VMapMesh;
    MMAP::MeshData m_ModelMesh;
    BuildContext*  m_Ctx;
    std::string m_modelName;
    MeshObjectType m_meshType;
    float          m_BMin[3];
    float          m_BMax[3];
    unsigned int   m_MapId;
    unsigned int   m_TileX;
    unsigned int   m_TileY;
};

typedef std::map<unsigned int, MeshObjects*> MeshObjectsMap;

class GeomData
{
public:
    GeomData();
    ~GeomData();

    void               Init(unsigned int mapId, BuildContext* ctx);
    void               Init(const std::string modelName, BuildContext* ctx);
    MeshObjects const* LoadTile(unsigned int tx, unsigned int ty);
    MeshObjects const* LoadModel(const std::string modelName);
    bool               RemoveTile(unsigned int tx, unsigned int ty);
    void               RemoveAllTiles();
    MeshObjects const* GetMeshObjects(unsigned int tx, unsigned int ty) const;
    bool               RaycastMesh(float* src, float* dst, float& tmin) const;
    bool               RaycastMesh(MeshDetails const* mesh, float* src, float* dst, float& tmin) const;

    inline MeshObjectsMap const* GetGeomsMap() const { return &m_MeshObjectsMap; }
    inline bool                  IsEmpty() const { return m_MeshObjectsMap.empty(); }
    inline float const*          BMin() const { return m_BMin; }
    inline float const*          BMax() const { return m_BMax; }
    inline bool                  NoMapFile() const { return m_NoMapFile; }

private:
    void           ComputeBounds();
    MeshObjectsMap m_MeshObjectsMap;
    BuildContext*  m_Ctx;
    unsigned int   m_MapId;
    std::string m_modelName;
    bool           m_NoMapFile;
    float          m_BMin[3];
    float          m_BMax[3];
};

#endif
