#define MMAP_GENERATOR

#include "GeomData.h"
#include "MapTree.h"
#include "ChunkyTriMesh.h"
#include "WorldModel.h"

#ifdef WIN32
#   define snprintf _snprintf
#endif

using namespace MMAP;
using namespace VMAP;
using namespace std;

MeshDetails::MeshDetails(float const* verts, int const* tris, unsigned int vCount, unsigned int tCount, unsigned char* flags) :
m_Verts(verts),
m_Tris(tris),
m_Normals(NULL),
m_VertCount(vCount),
m_TrisCount(tCount),
m_Flags(flags),
m_LocalFlags(false)
{
    ComputeNormals();
    CreateChunckyTriMesh();
    if (!m_Flags)
    {
        m_Flags = new unsigned char[tCount];
        memset(m_Flags, NAV_GROUND, tCount * sizeof(unsigned char));
        m_LocalFlags = true;
    }
}


MeshDetails::~MeshDetails()
{
    delete[] m_Normals;
    if (m_LocalFlags)
        delete[] m_Flags;
}

void MeshDetails::ComputeNormals()//(float* verts, int* tris, int trisSize)
{
    m_Normals = new float[m_TrisCount * 3];
    for (int i = 0; i < m_TrisCount * 3; i += 3)
    {
        const float* v0 = &m_Verts[m_Tris[i] * 3];
        const float* v1 = &m_Verts[m_Tris[i + 1] * 3];
        const float* v2 = &m_Verts[m_Tris[i + 2] * 3];
        float e0[3], e1[3];
        for (int j = 0; j < 3; ++j)
        {
            e0[j] = v1[j] - v0[j];
            e1[j] = v2[j] - v0[j];
        }
        float* n = &m_Normals[i];
        n[0] = e0[1] * e1[2] - e0[2] * e1[1];
        n[1] = e0[2] * e1[0] - e0[0] * e1[2];
        n[2] = e0[0] * e1[1] - e0[1] * e1[0];
        float d = sqrtf(n[0] * n[0] + n[1] * n[1] + n[2] * n[2]);
        if (d > 0)
        {
            d = 1.0f / d;
            n[0] *= d;
            n[1] *= d;
            n[2] *= d;
        }
    }
}

void MeshDetails::CreateChunckyTriMesh()
{
    m_chunkyMesh = new rcChunkyTriMesh;
    if (m_chunkyMesh)
    {
        rcCreateChunkyTriMesh(m_Verts, m_Tris, m_TrisCount, 256, m_chunkyMesh);
    }
}

void MeshObjects::GetMeshBounds(float const* verts, int vertCount, float* bmin, float* bmax)
{
    if (verts && vertCount)
        rcCalcBounds(verts, vertCount, bmin, bmax);
}

void MeshObjects::GetTileBounds(unsigned int tileX, unsigned int tileY, float const* verts, int vertCount, float* bmin, float* bmax)
{
    // this is for elevation
    if (verts && vertCount)
        rcCalcBounds(verts, vertCount, bmin, bmax);
    else
    {
        bmin[1] = FLT_MIN;
        bmax[1] = FLT_MAX;
    }

    if (tileX == tileY && tileX == 64)
        return;

    // this is for width and depth
    bmax[0] = (32 - int(tileX)) * GRID_SIZE;
    bmax[2] = (32 - int(tileY)) * GRID_SIZE;
    bmin[0] = bmax[0] - GRID_SIZE;
    bmin[2] = bmax[2] - GRID_SIZE;
}

MeshInfos::MeshInfos(MeshDetails const* sMesh, MeshDetails const* lMesh, float const* bmin, float const* bmax) :
    m_SolidMesh(sMesh),
    m_LiquidMesh(lMesh)
{
    rcVcopy(m_BMin, bmin);
    rcVcopy(m_BMax, bmax);
}

MeshObjects::MeshObjects(const std::string modelName, BuildContext* ctx) :
    m_MapId(0), m_TileX(0), m_TileY(0), m_Ctx(ctx),
    m_MapInfos(NULL), m_VMapInfos(NULL), m_ModelInfos(nullptr), m_modelName(modelName), m_meshType(MESH_OBJECT_TYPE_OBJECT)
{
    LoadObject();
}

MeshObjects::MeshObjects(unsigned int mapId, unsigned int tileX, unsigned int tileY, BuildContext* ctx) :
    m_MapId(mapId), m_TileX(tileX), m_TileY(tileY), m_Ctx(ctx),
    m_MapInfos(NULL), m_VMapInfos(NULL), m_ModelInfos(nullptr), m_meshType(MESH_OBJECT_TYPE_TILE)
{
    if (mapId < 0 || tileX < 0 || tileX > 64 || tileY < 0 || tileY > 64)
        return;

    LoadMap();
    LoadVMap();

    if (m_MapInfos && m_VMapInfos)
    {
        rcVcopy(m_BMin, m_MapInfos->BMin());
        rcVcopy(m_BMax, m_MapInfos->BMax());
        //rcVmin(m_BMin, m_VMapInfos->BMin());
        //rcVmin(m_BMax, m_VMapInfos->BMax());
    }
    else if (m_MapInfos)
    {
        rcVcopy(m_BMin, m_MapInfos->BMin());
        rcVcopy(m_BMax, m_MapInfos->BMax());
    }
    else if (m_VMapInfos)
    {
        rcVcopy(m_BMin, m_VMapInfos->BMin());
        rcVcopy(m_BMax, m_VMapInfos->BMax());
    }
}

MeshObjects::~MeshObjects()
{
    if (m_MapInfos)
        delete m_MapInfos;
    if (m_VMapInfos)
        delete m_VMapInfos;
    if (m_ModelInfos)
        delete m_ModelInfos;
}

void MeshObjects::LoadMap()
{
    if ((m_TileX == m_TileY) && m_TileX == 64)
        return;

    TerrainBuilder* terrainBuilder = new TerrainBuilder(false, m_Ctx->getDataDir());

    terrainBuilder->loadMap(m_MapId, m_TileY, m_TileX, m_MapMesh);

    // get the coord bounds of the model data
    if (m_MapMesh.solidVerts.size() + m_MapMesh.liquidVerts.size() == 0)
    {
        m_Ctx->log(RC_LOG_ERROR, "Could not load map file for tile(%i, %i)", m_TileX, m_TileY);
        return;
    }

    TerrainBuilder::cleanVertices(m_MapMesh.solidVerts, m_MapMesh.solidTris);
    TerrainBuilder::cleanVertices(m_MapMesh.liquidVerts, m_MapMesh.liquidTris);

    MeshDetails* solid = NULL;
    MeshDetails* liquid = NULL;
    float sBMin[3], sBMax[3], lBMin[3], lBMax[3];

    if ((m_MapMesh.solidVerts.size() != 0) || (m_MapMesh.liquidVerts.size() != 0))
    {
        if (m_MapMesh.solidVerts.size() != 0)
        {
            solid = new MeshDetails(m_MapMesh.solidVerts.getCArray(), m_MapMesh.solidTris.getCArray(),
                m_MapMesh.solidVerts.size() / 3, m_MapMesh.solidTris.size() / 3);
            MeshObjects::GetTileBounds(m_TileY, m_TileX, solid->Verts(), solid->VertCount(), sBMin, sBMax);
        }

        if (m_MapMesh.liquidVerts.size() != 0)
        {
            liquid = new MeshDetails(m_MapMesh.liquidVerts.getCArray(), m_MapMesh.liquidTris.getCArray(),
                m_MapMesh.liquidVerts.size() / 3, m_MapMesh.liquidTris.size() / 3, m_MapMesh.liquidType.getCArray());
            MeshObjects::GetTileBounds(m_TileY, m_TileX, solid->Verts(), solid->VertCount(), lBMin, lBMax);
            if (m_MapMesh.solidVerts.size() != 0)
            {
                rcVmin(sBMin, lBMin);
                rcVmax(sBMax, lBMax);
            }
            else
            {
                rcVcopy(sBMin, lBMin);
                rcVcopy(sBMax, lBMax);
            }
        }

        m_MapInfos = new MeshInfos(solid, liquid, sBMin, sBMax);
    }
    delete terrainBuilder;
}

void MeshObjects::LoadVMap()
{
    TerrainBuilder* terrainBuilder = new TerrainBuilder(false, m_Ctx->getDataDir());

    terrainBuilder->loadVMap(m_MapId, m_TileX, m_TileY, m_VMapMesh);

    // get the coord bounds of the model data
    if (m_VMapMesh.solidVerts.size() + m_VMapMesh.liquidVerts.size() == 0)
    {
        m_Ctx->log(RC_LOG_ERROR, "Could not load vmap file for tile(%i, %i)", m_TileX, m_TileY);
        return;
    }

    TerrainBuilder::cleanVertices(m_VMapMesh.solidVerts, m_VMapMesh.solidTris);
    TerrainBuilder::cleanVertices(m_VMapMesh.liquidVerts, m_VMapMesh.liquidTris);

    MeshDetails* solid = NULL;
    MeshDetails* liquid = NULL;
    float sBMin[3], sBMax[3], lBMin[3], lBMax[3];

    if ((m_VMapMesh.solidVerts.size() != 0) || (m_VMapMesh.liquidVerts.size() != 0))
    {
        if (m_VMapMesh.solidVerts.size() != 0)
        {
            solid = new MeshDetails(m_VMapMesh.solidVerts.getCArray(), m_VMapMesh.solidTris.getCArray(),
                m_VMapMesh.solidVerts.size() / 3, m_VMapMesh.solidTris.size() / 3);

            MeshObjects::GetTileBounds(m_TileY, m_TileX, solid->Verts(), solid->VertCount(), sBMin, sBMax);
        }

        if (m_VMapMesh.liquidVerts.size() != 0)
        {
            liquid = new MeshDetails(m_VMapMesh.liquidVerts.getCArray(), m_VMapMesh.liquidTris.getCArray(),
                m_VMapMesh.liquidVerts.size() / 3, m_VMapMesh.liquidTris.size() / 3, m_VMapMesh.liquidType.getCArray());

            MeshObjects::GetTileBounds(m_TileY, m_TileX, solid->Verts(), solid->VertCount(), lBMin, lBMax);
            if (m_MapMesh.solidVerts.size() != 0)
            {
                rcVmin(sBMin, lBMin);
                rcVmax(sBMax, lBMax);
            }
            else
            {
                rcVcopy(sBMin, lBMin);
                rcVcopy(sBMax, lBMax);
            }
        }

        m_VMapInfos = new MeshInfos(solid, liquid, sBMin, sBMax);
    }
    delete terrainBuilder;
}

void MeshObjects::LoadObject()
{
    TerrainBuilder* terrainBuilder = new TerrainBuilder(false, m_Ctx->getDataDir());

    std::string fullName(m_Ctx->getDataDir());
    fullName += "/vmaps/" + m_modelName;

    WorldModel m;
    if (!m.readFile(fullName))
    {
        printf("* Unable to open file\n");
        return;
    }

    // Load model data into navmesh
    vector<GroupModel> groupModels;
    m.getGroupModels(groupModels);

    // all M2s need to have triangle indices reversed
    bool isM2 = m_modelName.find(".m2") != m_modelName.npos || m_modelName.find(".M2") != m_modelName.npos;
    MeshDetails* solid = NULL;
    float sBMin[3], sBMax[3];
    for (vector<GroupModel>::iterator it = groupModels.begin(); it != groupModels.end(); ++it)
    {
        // transform data
        vector<Vector3> tempVertices;
        vector<MeshTriangle> tempTriangles;
        WmoLiquid* liquid = nullptr;

        (*it).getMeshData(tempVertices, tempTriangles, liquid);

        int offset = m_ModelMesh.solidVerts.size() / 3;

        TerrainBuilder::copyVertices(tempVertices, m_ModelMesh.solidVerts);
        TerrainBuilder::copyIndices(tempTriangles, m_ModelMesh.solidTris, offset, isM2);
    }
    // if there is no data, give up now
    if (!m_ModelMesh.solidVerts.size())
    {
        printf("* no solid vertices found\n");
        return;
    }

    TerrainBuilder::cleanVertices(m_ModelMesh.solidVerts, m_ModelMesh.solidTris);

    solid = new MeshDetails(m_ModelMesh.solidVerts.getCArray(), m_ModelMesh.solidTris.getCArray(),
        m_ModelMesh.solidVerts.size() / 3, m_ModelMesh.solidTris.size() / 3);

    MeshObjects::GetMeshBounds(solid->Verts(), solid->VertCount(), sBMin, sBMax);

    m_ModelInfos = new MeshInfos(solid, nullptr, sBMin, sBMax);
}

void MeshObjects::MergeMeshArrays(G3D::Array<float> &dstVerts, G3D::Array<int> &dstTris, G3D::Array<float> const& srcVerts, G3D::Array<int> const& srcTris) const
{
    int offset = dstVerts.size() / 3;
    dstVerts.append(srcVerts);
    MergeIndices(dstTris, &srcTris, offset);
}

void MeshObjects::MergeIndices(G3D::Array<int> &dst, G3D::Array<int> const* src, unsigned int offset) const
{
    int const* s = src->getCArray();
    for (int32 i = 0; i < src->size(); ++i)
        dst.append(s[i] + offset);
}

bool MeshObjects::GetMeshData(G3D::Array<float> &sVerts, G3D::Array<int> &sTris, G3D::Array<float> &lVerts, G3D::Array<int> &lTris, G3D::Array<unsigned char> &liquidFlags) const
{
    bool res = false;
    if (m_MapMesh.solidVerts.size())
    {
        MergeMeshArrays(sVerts, sTris, m_MapMesh.solidVerts, m_MapMesh.solidTris);
        res = true;
    }

    if (m_VMapMesh.solidVerts.size())
    {
        MergeMeshArrays(sVerts, sTris, m_VMapMesh.solidVerts, m_VMapMesh.solidTris);
        res = true;
    }

    if (m_MapMesh.liquidVerts.size())
    {
        MergeMeshArrays(sVerts, sTris, m_MapMesh.liquidVerts, m_MapMesh.liquidTris);
        liquidFlags.append(m_MapMesh.liquidType);
        res = true;
    }

    if (m_VMapMesh.liquidVerts.size())
    {
        MergeMeshArrays(sVerts, sTris, m_VMapMesh.liquidVerts, m_VMapMesh.liquidTris);
        liquidFlags.append(m_VMapMesh.liquidType);
        res = true;
    }

    return res;
}

GeomData::GeomData() :
m_MapId(0), m_Ctx(NULL), m_NoMapFile(false)
{

}

GeomData::~GeomData()
{
    RemoveAllTiles();
}

void GeomData::Init(unsigned int mapId, BuildContext* ctx)
{
    m_MapId = mapId;
    m_Ctx = ctx;
}

void GeomData::Init(const std::string modelName, BuildContext* ctx)
{
    m_MapId = 0;
    m_Ctx = ctx;
    m_modelName = modelName;
}

MeshObjects const* GeomData::LoadTile(unsigned int tx, unsigned int ty)
{
    unsigned int pxy = StaticMapTree::packTileID(tx, ty);

    if (tx == ty && tx == 64)
        m_NoMapFile = true;

    MeshObjects* newObj = new MeshObjects(m_MapId, tx, ty, m_Ctx);

    if (newObj->GetMap() || newObj->GetVMap())
    {
        if (m_MeshObjectsMap.empty())
        {
            rcVcopy(m_BMin, newObj->BMin());
            rcVcopy(m_BMax, newObj->BMax());
        }
        else
        {
            rcVmin(m_BMin, newObj->BMin());
            rcVmax(m_BMax, newObj->BMax());
        }

        m_MeshObjectsMap.insert(std::make_pair(pxy, newObj));
        return newObj;
    }

    delete newObj;
    return NULL;
}

MeshObjects const* GeomData::LoadModel(const std::string modelName)
{
    MeshObjects* newObj = new MeshObjects(modelName, m_Ctx);
    if (newObj->GetMap() || newObj->GetVMap() || newObj->GetModel())
    {
        rcVcopy(m_BMin, newObj->BMin());
        rcVcopy(m_BMax, newObj->BMax());
        m_MeshObjectsMap.insert(std::make_pair(0, newObj));
        return newObj;
    }

    delete newObj;
    return NULL;
}

bool GeomData::RemoveTile(unsigned int tx, unsigned int ty)
{
    unsigned int pxy = StaticMapTree::packTileID(tx, ty);
    MeshObjectsMap::iterator itr = m_MeshObjectsMap.find(pxy);
    if (itr != m_MeshObjectsMap.end())
    {
        delete itr->second;
        m_MeshObjectsMap.erase(itr);
        ComputeBounds();
        return true;
    }
    return false;
}

MeshObjects const* GeomData::GetMeshObjects(unsigned int tx, unsigned int ty) const
{
    unsigned int pxy;
    if (m_NoMapFile)
        pxy = StaticMapTree::packTileID(64, 64);
    else
        pxy = StaticMapTree::packTileID(tx, ty);

    MeshObjectsMap::const_iterator itr = m_MeshObjectsMap.find(pxy);
    if (itr != m_MeshObjectsMap.end())
        return itr->second;
    return NULL;
}

void GeomData::RemoveAllTiles()
{
    for (MeshObjectsMap::iterator itr = m_MeshObjectsMap.begin(); itr != m_MeshObjectsMap.end(); ++itr)
        delete itr->second;
    m_MeshObjectsMap.clear();
}

void GeomData::ComputeBounds()
{
    MeshObjectsMap::iterator itr = m_MeshObjectsMap.begin();
    if (itr != m_MeshObjectsMap.end())
    {
        rcVcopy(m_BMin, itr->second->BMin());
        rcVcopy(m_BMax, itr->second->BMax());
        ++itr;

        while (itr != m_MeshObjectsMap.end())
        {
            rcVmin(m_BMin, itr->second->BMin());
            rcVmax(m_BMax, itr->second->BMax());
            ++itr;
        }
    }
}

static bool isectSegAABB(const float* sp, const float* sq,
    const float* amin, const float* amax,
    float& tmin, float& tmax)
{
    static const float EPS = 1e-6f;

    float d[3];
    d[0] = sq[0] - sp[0];
    d[1] = sq[1] - sp[1];
    d[2] = sq[2] - sp[2];
    tmin = 0.0;
    tmax = 1.0f;

    for (int i = 0; i < 3; i++)
    {
        if (fabsf(d[i]) < EPS)
        {
            if (sp[i] < amin[i] || sp[i] > amax[i])
                return false;
        }
        else
        {
            const float ood = 1.0f / d[i];
            float t1 = (amin[i] - sp[i]) * ood;
            float t2 = (amax[i] - sp[i]) * ood;
            if (t1 > t2) { float tmp = t1; t1 = t2; t2 = tmp; }
            if (t1 > tmin) tmin = t1;
            if (t2 < tmax) tmax = t2;
            if (tmin > tmax) return false;
        }
    }

    return true;
}

static bool intersectSegmentTriangle(const float* sp, const float* sq,
    const float* a, const float* b, const float* c,
    float &t)
{
    float v, w;
    float ab[3], ac[3], qp[3], ap[3], norm[3], e[3];
    rcVsub(ab, b, a);
    rcVsub(ac, c, a);
    rcVsub(qp, sp, sq);

    // Compute triangle normal. Can be precalculated or cached if
    // intersecting multiple segments against the same triangle
    rcVcross(norm, ab, ac);

    // Compute denominator d. If d <= 0, segment is parallel to or points
    // away from triangle, so exit early
    float d = rcVdot(qp, norm);
    if (d <= 0.0f) return false;

    // Compute intersection t value of pq with plane of triangle. A ray
    // intersects iff 0 <= t. Segment intersects iff 0 <= t <= 1. Delay
    // dividing by d until intersection has been found to pierce triangle
    rcVsub(ap, sp, a);
    t = rcVdot(ap, norm);
    if (t < 0.0f) return false;
    if (t > d) return false; // For segment; exclude this code line for a ray test

    // Compute barycentric coordinate components and test if within bounds
    rcVcross(e, qp, ap);
    v = rcVdot(ac, e);
    if (v < 0.0f || v > d) return false;
    w = -rcVdot(ab, e);
    if (w < 0.0f || v + w > d) return false;

    // Segment/ray intersects triangle. Perform delayed division
    t /= d;

    return true;
}

bool GeomData::RaycastMesh(MeshDetails const* mesh, float* src, float* dst, float& tmin) const
{
    float dir[3];
    rcVsub(dir, dst, src);

    // Prune hit ray.
    float btmin, btmax;
    if (!isectSegAABB(src, dst, m_BMin, m_BMax, btmin, btmax))
        return false;
    float p[2], q[2];
    p[0] = src[0] + (dst[0] - src[0])*btmin;
    p[1] = src[2] + (dst[2] - src[2])*btmin;
    q[0] = src[0] + (dst[0] - src[0])*btmax;
    q[1] = src[2] + (dst[2] - src[2])*btmax;

    int cid[512];
    const int ncid = rcGetChunksOverlappingSegment(mesh->GetChunckyTM(), p, q, cid, 512);
    if (!ncid)
        return false;

    tmin = 1.0f;
    bool hit = false;
    const float* verts = mesh->Verts();

    for (int i = 0; i < ncid; ++i)
    {
        const rcChunkyTriMeshNode& node = mesh->GetChunckyTM()->nodes[cid[i]];
        const int* tris = &mesh->GetChunckyTM()->tris[node.i * 3];
        const int ntris = node.n;

        for (int j = 0; j < ntris * 3; j += 3)
        {
            float t = 1;
            if (intersectSegmentTriangle(src, dst,
                &verts[tris[j] * 3],
                &verts[tris[j + 1] * 3],
                &verts[tris[j + 2] * 3], t))
            {
                if (t < tmin)
                    tmin = t;
                hit = true;
            }
        }
    }

    return hit;
}

bool GeomData::RaycastMesh(float* src, float* dst, float& tmin) const
{
    if (m_MeshObjectsMap.empty())
        return false;

    bool hit = false;
    MeshObjectsMap::const_iterator itr = m_MeshObjectsMap.begin();
    while (itr != m_MeshObjectsMap.end())
    {
        MeshObjects const* mo = itr->second;
        MeshInfos const* mmi = mo->GetMap();
        MeshInfos const* vmmi = mo->GetVMap();

        if (mmi && mmi->GetSolidMesh())
            hit = RaycastMesh(mmi->GetSolidMesh(), src, dst, tmin);

        if (!hit && vmmi && vmmi->GetSolidMesh())
                    hit = RaycastMesh(vmmi->GetSolidMesh(), src, dst, tmin);

        if (hit)
                break;

        ++itr;
    }

    return hit;
}
