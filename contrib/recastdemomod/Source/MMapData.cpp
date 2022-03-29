#include <stdio.h>
#include "memory.h"
#include "Vmap/MapTree.h"
#include "MMapData.h"
#include "../../../../contrib/mmap/src/MMapCommon.h"
#include "MotionGenerators/MoveMapSharedDefines.h"
#include "GeomData.h"
#include "DetourNavMeshBuilder.h"
#include "../../../../contrib/mmap/src/IntermediateValues.h"
#include "Recast.h"
#include "../../../../contrib/mmap/src/MapBuilder.h"

#ifdef WIN32
#   define snprintf _snprintf
#endif

using namespace MMAP;
using namespace VMAP;

MMapData::MMapData() :
    m_NavMesh(NULL),
    m_NavQuery(NULL),
    m_Initialized(false),
    m_triareas(NULL),
    m_solid(NULL),
    m_chf(NULL),
    m_cset(NULL),
    m_pmesh(NULL),
    m_dmesh(NULL),
    m_MultipleTile(false)
{
}

MMapData::~MMapData()
{
    if (m_Initialized)
    {
        if (m_NavQuery)
            dtFreeNavMeshQuery(m_NavQuery);
        if (m_NavMesh)
            dtFreeNavMesh(m_NavMesh);
    }
}

dtNavMesh* MMapData::Init(unsigned int mapId, BuildContext* ctx)
{
    char mmapFilename[256];
    snprintf(mmapFilename, sizeof(mmapFilename), "%s/mmaps/%03d.mmap", ctx->getDataDir(), mapId);

    FILE* fp = fopen(mmapFilename, "rb");
    if (!fp)
    {
        ctx->log(RC_LOG_ERROR, "LoadMMapTile: '%s' not found!", mmapFilename);
        return NULL;
    }

    dtNavMeshParams params;
    fread(&params, sizeof(dtNavMeshParams), 1, fp);
    fclose(fp);
    m_NavMesh = dtAllocNavMesh();
    dtStatus dtResult = m_NavMesh->init(&params);
    if (dtStatusFailed(dtResult))
    {
        ctx->log(RC_LOG_ERROR, "LoadMMapTile: NavMesh init failed!");
        return NULL;
    }

    m_NavQuery = dtAllocNavMeshQuery();
    dtStatus status = m_NavQuery->init(m_NavMesh, 2048);
    if (dtStatusFailed(status))
    {
        ctx->log(RC_LOG_ERROR, "Could not init Detour navmesh query");
        return NULL;
    }

    m_MapId = mapId;
    m_Ctx = ctx;
    m_Initialized = true;
    return m_NavMesh;
}

bool MMapData::LoadNavmesh(const char* fileName, TileInfos& tInfos)
{
    FILE* fp = fopen(fileName, "rb");
    if (!fp)
    {
        m_Ctx->log(RC_LOG_ERROR, "LoadMMapTile: '%s' not found!", fileName);
        return false;
    }

    // Read header.
    MmapTileHeader header;
    fread(&header, sizeof(MmapTileHeader), 1, fp);
    if (header.mmapVersion != MMAP_VERSION)
    {
        fclose(fp);
        m_Ctx->log(RC_LOG_ERROR, "LoadMMapTile: Mmap version is not correct! Found(%u), expected(%u)!", header.mmapVersion, unsigned int(MMAP_VERSION));
        return false;
    }

    unsigned char* data = (unsigned char*)dtAlloc(header.size, DT_ALLOC_PERM);
    if (!data)
    {
        m_Ctx->log(RC_LOG_ERROR, "LoadMMapTile: Memory allocation failed!");
        fclose(fp);
        return false;
    }

    size_t result = fread(data, header.size, 1, fp);
    if (!result)
    {
        m_Ctx->log(RC_LOG_ERROR, "LoadMMapTile: Problem to read '%s'!", fileName);
        return false;
    }
    fclose(fp);

    dtMeshHeader* meshHeader = (dtMeshHeader*)data;
    rcVcopy(tInfos.bMin, meshHeader->bmin);
    rcVcopy(tInfos.bMax, meshHeader->bmax);
    rcVmin(m_BMin, tInfos.bMin);
    rcVmax(m_BMax, tInfos.bMax);
    dtStatus dtRes = m_NavMesh->addTile(data, header.size, DT_TILE_FREE_DATA, dtTileRef(0), &tInfos.tileRef);
    if (dtStatusFailed(dtRes))
    {
        m_Ctx->log(RC_LOG_ERROR, "LoadMMapTile: add tile failed!");
        return false;
    }
    return true;
}

bool MMapData::LoadObjectNavMesh(const std::string fileName)
{
    if (!m_Initialized)
        return false;

    char tileFilename[256];
    snprintf(tileFilename, sizeof(tileFilename), "%s/mmaps/%s.mmtile", m_Ctx->getDataDir(), fileName.c_str()); // Todo: check X,Y swap reason and get ride of it

    TileInfos tInfos;
    if (!LoadNavmesh(tileFilename, tInfos))
        return false;
    m_TilesInfos.insert(std::make_pair(0, tInfos));
    return true;
}

bool MMapData::LoadTile(unsigned int tx, unsigned int ty)
{
    if (!m_Initialized)
        return false;

    char tileFilename[256];
    snprintf(tileFilename, sizeof(tileFilename), "%s/mmaps/%03d%02d%02d.mmtile", m_Ctx->getDataDir(), m_MapId, tx, ty); // Todo: check X,Y swap reason and get ride of it

    TileInfos tInfos;
    if (!LoadNavmesh(tileFilename, tInfos))
        return false;

    unsigned int pxy = StaticMapTree::packTileID(tx, ty);
    m_TilesInfos.insert(std::make_pair(pxy, tInfos));
    return true;
}

// load multiple tile for instances withouts map (only vmap data found)
bool MMapData::LoadTile(float const* bmin, float const* bmax)
{
    if (!m_Initialized)
        return false;

    m_MultipleTile = true;

    // convert coord bounds to grid bounds
    unsigned int minX, minY, maxX, maxY;
    maxX = 32 - bmin[0] / RecastDemo::BLOCK_SIZE;
    maxY = 32 - bmin[2] / RecastDemo::BLOCK_SIZE;
    minX = 32 - bmax[0] / RecastDemo::BLOCK_SIZE;
    minY = 32 - bmax[2] / RecastDemo::BLOCK_SIZE;

    // add all tiles within bounds to tile list.
    for (unsigned int i = minX; i <= maxX; ++i)
        for (unsigned int j = minY; j <= maxY; ++j)
            LoadTile(j, i);

    if (m_TilesInfos.size() != 0)
        return true;

    return false;
}

void MMapData::RemoveAllTiles()
{
    for (TilesRefMap::iterator itr = m_TilesInfos.begin(); itr != m_TilesInfos.end(); ++itr)
        m_NavMesh->removeTile(itr->second.tileRef, 0, 0);
    m_TilesInfos.clear();
}

bool MMapData::RemoveTile(unsigned int tx, unsigned int ty)
{
    if (m_MultipleTile)
    {
        RemoveAllTiles();
        return true;
    }

    unsigned int pxy = StaticMapTree::packTileID(tx, ty);
    TilesRefMap::iterator itr = m_TilesInfos.find(pxy);
    if (itr != m_TilesInfos.end())
    {
        dtStatus res = m_NavMesh->removeTile(itr->second.tileRef, 0, 0);
        m_TilesInfos.erase(itr);

        itr = m_TilesInfos.begin();
        if (itr != m_TilesInfos.end())
        {
            rcVcopy(m_BMin, itr->second.bMin);
            rcVcopy(m_BMax, itr->second.bMax);
            ++itr;
            while (itr != m_TilesInfos.end())
            {
                rcVmin(m_BMin, itr->second.bMin);
                rcVmax(m_BMax, itr->second.bMax);
                ++itr;
            }
        }

        return dtStatusSucceed(res);
    }
    return false;
}

dtTileRef MMapData::GetTileRef(unsigned int tx, unsigned int ty) const
{
    unsigned int pxy = StaticMapTree::packTileID(tx, ty);
    TilesRefMap::const_iterator itr = m_TilesInfos.find(pxy);
    if (itr != m_TilesInfos.end())
        return itr->second.tileRef;
    return 0;
}

bool MMapData::GetTileBounds(unsigned int tx, unsigned int ty, float const* &bmin, float const* &bmax) const
{
    unsigned int pxy = StaticMapTree::packTileID(tx, ty);
    TilesRefMap::const_iterator itr = m_TilesInfos.find(pxy);
    if (itr != m_TilesInfos.end())
    {
        bmin = itr->second.bMin;
        bmax = itr->second.bMax;
        return true;
    }
    return false;
}

void MMapData::Cleanup()
{
    if (m_triareas)
        delete[] m_triareas;
    m_triareas = NULL;
    rcFreeHeightField(m_solid);
    m_solid = NULL;
    rcFreeCompactHeightfield(m_chf);
    m_chf = NULL;
    rcFreeContourSet(m_cset);
    m_cset = NULL;
    rcFreePolyMesh(m_pmesh);
    m_pmesh = NULL;
    rcFreePolyMeshDetail(m_dmesh);
    m_dmesh = NULL;
}


void MMapData::BuildNavMeshOfTile(unsigned int tx, unsigned int ty, MeshObjects const* meshObjects, rcConfig const* cfg, float agentHeight, float agentRadius, float agentClimb, SamplePartitionType partitionType)
{
    IntermediateValues iv;

    G3D::Array<float> solidVerts;
    G3D::Array<int>   solidTris;
    G3D::Array<float> liquidVerts;
    G3D::Array<int>   liquidTris;
    G3D::Array<unsigned char> liquidFlags;

    if (!meshObjects || !meshObjects->GetMeshData(solidVerts, solidTris, liquidVerts, liquidTris, liquidFlags))
    {
        m_Ctx->log(RC_LOG_ERROR, "No mesh data to process!!!");
        return;
    }

    float* tVerts = solidVerts.getCArray();
    int tVertCount = solidVerts.size() / 3;
    int* tTris = solidTris.getCArray();
    int tTriCount = solidTris.size() / 3;

    float* lVerts = liquidVerts.getCArray();
    int lVertCount = liquidVerts.size() / 3;
    int* lTris = liquidTris.getCArray();
    int lTriCount = liquidTris.size() / 3;
    uint8* lTriFlags = liquidFlags.getCArray();

    float m_tileMemUsage = 0;
    float m_tileBuildTime = 0;

    Cleanup();

    /*const float* verts = m_geom->getMesh()->getVerts();
    const int nverts = m_geom->getMesh()->getVertCount();
    const int ntris = m_geom->getMesh()->getTriCount();
    const rcChunkyTriMesh* chunkyMesh = m_geom->getChunkyMesh();*/

    // Reset build times gathering.
    m_Ctx->resetTimers();

    // Start the build process.
    m_Ctx->startTimer(RC_TIMER_TOTAL);

    m_Ctx->log(RC_LOG_PROGRESS, "Building navigation:");
    m_Ctx->log(RC_LOG_PROGRESS, " - %d x %d cells", cfg->width, cfg->height);
    m_Ctx->log(RC_LOG_PROGRESS, " - %.1fK verts, %.1fK tris", tVertCount / 1000.0f, tTriCount / 1000.0f);
    m_Ctx->log(RC_LOG_PROGRESS, " - %.1fK lverts, %.1fK ltris", lVertCount / 1000.0f, lTriCount / 1000.0f);

    // Allocate voxel heightfield where we rasterize our input data to.
    m_solid = rcAllocHeightfield();
    if (!m_solid)
    {
        m_Ctx->log(RC_LOG_ERROR, "buildNavigation: Out of memory 'solid'.");
        return;
    }
    if (!rcCreateHeightfield(m_Ctx, *m_solid, cfg->width, cfg->height, cfg->bmin, cfg->bmax, cfg->cs, cfg->ch))
    {
        m_Ctx->log(RC_LOG_ERROR, "buildNavigation: Could not create solid heightfield.");
        return;
    }
    // mark all walkable tiles, both liquids and solids
    unsigned char* triFlags = new unsigned char[tTriCount];
    memset(triFlags, NAV_GROUND, tTriCount * sizeof(unsigned char));
    rcClearUnwalkableTriangles(m_Ctx, cfg->walkableSlopeAngle, tVerts, tVertCount, tTris, tTriCount, triFlags);
    rcRasterizeTriangles(m_Ctx, tVerts, tVertCount, tTris, triFlags, tTriCount, *m_solid, cfg->walkableClimb);
    delete[] triFlags;

    // Once all geometry is rasterized, we do initial pass of filtering to
    // remove unwanted overhangs caused by the conservative rasterization
    // as well as filter spans where the character cannot possibly stand.
    rcFilterLowHangingWalkableObstacles(m_Ctx, cfg->walkableClimb, *m_solid);
    rcFilterLedgeSpans(m_Ctx, cfg->walkableHeight, cfg->walkableClimb, *m_solid);
    rcFilterWalkableLowHeightSpans(m_Ctx, cfg->walkableHeight, *m_solid);
    delete[] triFlags;
    rcRasterizeTriangles(m_Ctx, lVerts, lVertCount, lTris, lTriFlags, lTriCount, *m_solid, cfg->walkableClimb);

    // Compact the heightfield so that it is faster to handle from now on.
    // This will result more cache coherent data as well as the neighbours
    // between walkable cells will be calculated.
    m_chf = rcAllocCompactHeightfield();
    if (!m_chf)
    {
        m_Ctx->log(RC_LOG_ERROR, "buildNavigation: Out of memory 'chf'.");
        return;
    }
    if (!rcBuildCompactHeightfield(m_Ctx, cfg->walkableHeight, cfg->walkableClimb, *m_solid, *m_chf))
    {
        m_Ctx->log(RC_LOG_ERROR, "buildNavigation: Could not build compact data.");
        return;
    }

    // Erode the walkable area by agent radius.
    if (!rcErodeWalkableArea(m_Ctx, cfg->walkableRadius, *m_chf))
    {
        m_Ctx->log(RC_LOG_ERROR, "buildNavigation: Could not erode.");
        return;
    }

    // (Optional) Mark areas.
    /*const ConvexVolume* vols = m_geom->getConvexVolumes();
    for (int i = 0; i < m_geom->getConvexVolumeCount(); ++i)
        rcMarkConvexPolyArea(m_Ctx, vols[i].verts, vols[i].nverts, vols[i].hmin, vols[i].hmax, (unsigned char)vols[i].area, *m_chf);*/


    // Partition the heightfield so that we can use simple algorithm later to triangulate the walkable areas.
    // There are 3 martitioning methods, each with some pros and cons:
    // 1) Watershed partitioning
    //   - the classic Recast partitioning
    //   - creates the nicest tessellation
    //   - usually slowest
    //   - partitions the heightfield into nice regions without holes or overlaps
    //   - the are some corner cases where this method creates produces holes and overlaps
    //      - holes may appear when a small obstacles is close to large open area (triangulation can handle this)
    //      - overlaps may occur if you have narrow spiral corridors (i.e stairs), this make triangulation to fail
    //   * generally the best choice if you precompute the nacmesh, use this if you have large open areas
    // 2) Monotone partioning
    //   - fastest
    //   - partitions the heightfield into regions without holes and overlaps (guaranteed)
    //   - creates long thin polygons, which sometimes causes paths with detours
    //   * use this if you want fast navmesh generation
    // 3) Layer partitoining
    //   - quite fast
    //   - partitions the heighfield into non-overlapping regions
    //   - relies on the triangulation code to cope with holes (thus slower than monotone partitioning)
    //   - produces better triangles than monotone partitioning
    //   - does not have the corner cases of watershed partitioning
    //   - can be slow and create a bit ugly tessellation (still better than monotone)
    //     if you have large open areas with small obstacles (not a problem if you use tiles)
    //   * good choice to use for tiled navmesh with medium and small sized tiles

    if (partitionType == SAMPLE_PARTITION_WATERSHED)
    {
        // Prepare for region partitioning, by calculating distance field along the walkable surface.
        if (!rcBuildDistanceField(m_Ctx, *m_chf))
        {
            m_Ctx->log(RC_LOG_ERROR, "buildNavigation: Could not build distance field.");
            return;
        }

        // Partition the walkable surface into simple regions without holes.
        if (!rcBuildRegions(m_Ctx, *m_chf, cfg->borderSize, cfg->minRegionArea, cfg->mergeRegionArea))
        {
            m_Ctx->log(RC_LOG_ERROR, "buildNavigation: Could not build watershed regions.");
            return;
        }
    }
    else if (partitionType == SAMPLE_PARTITION_MONOTONE)
    {
        // Partition the walkable surface into simple regions without holes.
        // Monotone partitioning does not need distancefield.
        if (!rcBuildRegionsMonotone(m_Ctx, *m_chf, cfg->borderSize, cfg->minRegionArea, cfg->mergeRegionArea))
        {
            m_Ctx->log(RC_LOG_ERROR, "buildNavigation: Could not build monotone regions.");
            return;
        }
    }
    else // SAMPLE_PARTITION_LAYERS
    {
        // Partition the walkable surface into simple regions without holes.
        if (!rcBuildLayerRegions(m_Ctx, *m_chf, cfg->borderSize, cfg->minRegionArea))
        {
            m_Ctx->log(RC_LOG_ERROR, "buildNavigation: Could not build layer regions.");
            return;
        }
    }

    // Create contours.
    m_cset = rcAllocContourSet();
    if (!m_cset)
    {
        m_Ctx->log(RC_LOG_ERROR, "buildNavigation: Out of memory 'cset'.");
        return;
    }
    if (!rcBuildContours(m_Ctx, *m_chf, cfg->maxSimplificationError, cfg->maxEdgeLen, *m_cset))
    {
        m_Ctx->log(RC_LOG_ERROR, "buildNavigation: Could not create contours.");
        return;
    }

    if (m_cset->nconts == 0)
    {
        return;
    }

    // Build polygon navmesh from the contours.
    m_pmesh = rcAllocPolyMesh();
    if (!m_pmesh)
    {
        m_Ctx->log(RC_LOG_ERROR, "buildNavigation: Out of memory 'pmesh'.");
        return;
    }
    if (!rcBuildPolyMesh(m_Ctx, *m_cset, cfg->maxVertsPerPoly, *m_pmesh))
    {
        m_Ctx->log(RC_LOG_ERROR, "buildNavigation: Could not triangulate contours.");
        return;
    }

    // Build detail mesh.
    m_dmesh = rcAllocPolyMeshDetail();
    if (!m_dmesh)
    {
        m_Ctx->log(RC_LOG_ERROR, "buildNavigation: Out of memory 'dmesh'.");
        return;
    }

    if (!rcBuildPolyMeshDetail(m_Ctx, *m_pmesh, *m_chf,
        cfg->detailSampleDist, cfg->detailSampleMaxError,
        *m_dmesh))
    {
        m_Ctx->log(RC_LOG_ERROR, "buildNavigation: Could build polymesh detail.");
        return;
    }

    unsigned char* navData = 0;
    int navDataSize = 0;
    if (cfg->maxVertsPerPoly <= DT_VERTS_PER_POLYGON)
    {
        if (m_pmesh->nverts >= 0xffff)
        {
            // The vertex indices are ushorts, and cannot point to more than 0xffff vertices.
            m_Ctx->log(RC_LOG_ERROR, "Too many vertices per tile %d (max: %d).", m_pmesh->nverts, 0xffff);
            return;
        }

        // Update poly flags from areas.
        for (int i = 0; i < m_pmesh->npolys; ++i)
        {
            if (m_pmesh->areas[i] == RC_WALKABLE_AREA)
                m_pmesh->areas[i] = SAMPLE_POLYAREA_GROUND;

            if (m_pmesh->areas[i] == SAMPLE_POLYAREA_GROUND ||
                m_pmesh->areas[i] == SAMPLE_POLYAREA_GRASS ||
                m_pmesh->areas[i] == SAMPLE_POLYAREA_ROAD)
            {
                m_pmesh->flags[i] = SAMPLE_POLYFLAGS_WALK;
            }
            else if (m_pmesh->areas[i] == SAMPLE_POLYAREA_WATER)
            {
                m_pmesh->flags[i] = SAMPLE_POLYFLAGS_SWIM;
            }
            else if (m_pmesh->areas[i] == SAMPLE_POLYAREA_DOOR)
            {
                m_pmesh->flags[i] = SAMPLE_POLYFLAGS_WALK | SAMPLE_POLYFLAGS_DOOR;
            }
        }

        dtNavMeshCreateParams params;
        memset(&params, 0, sizeof(params));
        params.verts = m_pmesh->verts;
        params.vertCount = m_pmesh->nverts;
        params.polys = m_pmesh->polys;
        params.polyAreas = m_pmesh->areas;
        params.polyFlags = m_pmesh->flags;
        params.polyCount = m_pmesh->npolys;
        params.nvp = m_pmesh->nvp;
        params.detailMeshes = m_dmesh->meshes;
        params.detailVerts = m_dmesh->verts;
        params.detailVertsCount = m_dmesh->nverts;
        params.detailTris = m_dmesh->tris;
        params.detailTriCount = m_dmesh->ntris;/*
        params.offMeshConVerts = m_geom->getOffMeshConnectionVerts();
        params.offMeshConRad = m_geom->getOffMeshConnectionRads();
        params.offMeshConDir = m_geom->getOffMeshConnectionDirs();
        params.offMeshConAreas = m_geom->getOffMeshConnectionAreas();
        params.offMeshConFlags = m_geom->getOffMeshConnectionFlags();
        params.offMeshConUserID = m_geom->getOffMeshConnectionId();
        params.offMeshConCount = m_geom->getOffMeshConnectionCount();*/
        params.walkableHeight = agentHeight;
        params.walkableRadius = agentRadius;
        params.walkableClimb = agentClimb;
        params.tileX = tx;
        params.tileY = ty;
        params.tileLayer = 0;
        rcVcopy(params.bmin, m_pmesh->bmin);
        rcVcopy(params.bmax, m_pmesh->bmax);
        params.cs = cfg->cs;
        params.ch = cfg->ch;
        params.buildBvTree = true;

        if (!dtCreateNavMeshData(&params, &navData, &navDataSize))
        {
            m_Ctx->log(RC_LOG_ERROR, "Could not build Detour navmesh.");
            return;
        }
        TileInfos tinfos;
        rcVcopy(tinfos.bMin, params.bmin);
        rcVcopy(tinfos.bMax, params.bmax);
        rcVmin(m_BMin, tinfos.bMin);
        rcVmax(m_BMax, tinfos.bMax);
        dtStatus dtRes = m_NavMesh->addTile(navData, navDataSize, DT_TILE_FREE_DATA, 0, &tinfos.tileRef);
        if (dtStatusFailed(dtRes))
        {
            m_Ctx->log(RC_LOG_ERROR, "LoadMMapTile: add tile failed!");
            return;
        }

        unsigned int pxy = StaticMapTree::packTileID(tx, ty);
        m_TilesInfos.insert(std::make_pair(pxy, tinfos));
    }


    m_tileMemUsage = navDataSize / 1024.0f;

    m_Ctx->stopTimer(RC_TIMER_TOTAL);

    // Show performance stats.
    duLogBuildTimes(*m_Ctx, m_Ctx->getAccumulatedTime(RC_TIMER_TOTAL));
    m_Ctx->log(RC_LOG_PROGRESS, ">> Polymesh: %d vertices  %d polygons", m_pmesh->nverts, m_pmesh->npolys);

    m_tileBuildTime = m_Ctx->getAccumulatedTime(RC_TIMER_TOTAL) / 1000.0f;

    m_Ctx->log(RC_LOG_PROGRESS, ">> Memory usage: %f", m_tileMemUsage);
    m_Ctx->log(RC_LOG_PROGRESS, ">> Build time: %f", m_tileBuildTime);
    return;
}

void MMapData::BuildMoveMapTile(unsigned int tileX, unsigned int tileY, MeshObjects const* meshObjects, rcConfig* cfg, SamplePartitionType partitionType)
{
    IntermediateValues iv;

    G3D::Array<float> solidVerts;
    G3D::Array<int>   solidTris;
    G3D::Array<float> liquidVerts;
    G3D::Array<int>   liquidTris;
    G3D::Array<unsigned char> liquidFlags;

    if (!meshObjects || !meshObjects->GetMeshData(solidVerts, solidTris, liquidVerts, liquidTris, liquidFlags))
    {
        m_Ctx->log(RC_LOG_ERROR, "No mesh data to process!!!");
        return;
    }

    float* tVerts    = solidVerts.getCArray();
    int tVertCount   = solidVerts.size() / 3;
    int* tTris       = solidTris.getCArray();
    int tTriCount    = solidTris.size() / 3;

    float* lVerts    = liquidVerts.getCArray();
    int lVertCount   = liquidVerts.size() / 3;
    int* lTris       = liquidTris.getCArray();
    int lTriCount    = liquidTris.size() / 3;
    uint8* lTriFlags = liquidFlags.getCArray();

    /*// these are WORLD UNIT based metrics
    // this are basic unit dimentions
    // value have to divide GRID_SIZE(533.33333f) ( aka: 0.5333, 0.2666, 0.3333, 0.1333, etc )
    const static float BASE_UNIT_DIM = m_bigBaseUnit ? 0.533333f : 0.266666f;

    // All are in UNIT metrics!
    const static int VERTEX_PER_MAP = int(GRID_SIZE / BASE_UNIT_DIM + 0.5f);
    const static int VERTEX_PER_TILE = m_bigBaseUnit ? 40 : 80; // must divide VERTEX_PER_MAP
    const static int TILES_PER_MAP = VERTEX_PER_MAP / VERTEX_PER_TILE;*/

    // allocate subregions : tiles
    Tile* tiles = new Tile[RecastDemo::TILES_PER_MAP * RecastDemo::TILES_PER_MAP];

    // this sets the dimensions of the heightfield - should maybe happen before border padding
    MeshObjects::GetTileBounds(tileY, tileX, tVerts, tVertCount, cfg->bmin, cfg->bmax);
    rcCalcGridSize(cfg->bmin, cfg->bmax, cfg->cs, &cfg->width, &cfg->height);

    // Initialize per tile config.
    rcConfig tileCfg;
    memcpy(&tileCfg, cfg, sizeof(rcConfig));
    tileCfg.width = cfg->tileSize + cfg->borderSize * 2;
    tileCfg.height = cfg->tileSize + cfg->borderSize * 2;

    // build all tiles
    for (int y = 0; y < RecastDemo::TILES_PER_MAP; ++y)
    {
        for (int x = 0; x < RecastDemo::TILES_PER_MAP; ++x)
        {
            Tile& tile = tiles[x + y * RecastDemo::TILES_PER_MAP];

            // Calculate the per tile bounding box.
            tileCfg.bmin[0] = cfg->bmin[0] + (x * cfg->tileSize - cfg->borderSize) * cfg->cs;
            tileCfg.bmin[2] = cfg->bmin[2] + (y * cfg->tileSize - cfg->borderSize) * cfg->cs;
            tileCfg.bmax[0] = cfg->bmin[0] + ((x + 1) * cfg->tileSize + cfg->borderSize) * cfg->cs;
            tileCfg.bmax[2] = cfg->bmin[2] + ((y + 1) * cfg->tileSize + cfg->borderSize) * cfg->cs;

            float tbmin[2], tbmax[2];
            tbmin[0] = tileCfg.bmin[0];
            tbmin[1] = tileCfg.bmin[2];
            tbmax[0] = tileCfg.bmax[0];
            tbmax[1] = tileCfg.bmax[2];

            // build heightfield
            tile.solid = rcAllocHeightfield();
            if (!tile.solid || !rcCreateHeightfield(m_Ctx, *tile.solid, tileCfg.width, tileCfg.height, tileCfg.bmin, tileCfg.bmax, tileCfg.cs, tileCfg.ch))
            {
                m_Ctx->log(RC_LOG_ERROR, "Failed building heightfield!");
                continue;
            }

            // mark all walkable tiles, both liquids and solids
            unsigned char* triFlags = new unsigned char[tTriCount];
            memset(triFlags, NAV_GROUND, tTriCount * sizeof(unsigned char));
            rcClearUnwalkableTriangles(m_Ctx, tileCfg.walkableSlopeAngle, tVerts, tVertCount, tTris, tTriCount, triFlags);
            rcRasterizeTriangles(m_Ctx, tVerts, tVertCount, tTris, triFlags, tTriCount, *tile.solid, cfg->walkableClimb);
            delete[] triFlags;

            rcFilterLowHangingWalkableObstacles(m_Ctx, cfg->walkableClimb, *tile.solid);
            rcFilterLedgeSpans(m_Ctx, tileCfg.walkableHeight, tileCfg.walkableClimb, *tile.solid);
            rcFilterWalkableLowHeightSpans(m_Ctx, tileCfg.walkableHeight, *tile.solid);

            rcRasterizeTriangles(m_Ctx, lVerts, lVertCount, lTris, lTriFlags, lTriCount, *tile.solid, cfg->walkableClimb);

            // compact heightfield spans
            tile.chf = rcAllocCompactHeightfield();
            if (!tile.chf || !rcBuildCompactHeightfield(m_Ctx, tileCfg.walkableHeight, tileCfg.walkableClimb, *tile.solid, *tile.chf))
            {
                m_Ctx->log(RC_LOG_ERROR, "Failed compacting heightfield!");
                continue;
            }

            // build polymesh intermediates
            if (!rcErodeWalkableArea(m_Ctx, cfg->walkableRadius, *tile.chf))
            {
                m_Ctx->log(RC_LOG_ERROR, "Failed eroding area!");
                continue;
            }

            if (!rcBuildDistanceField(m_Ctx, *tile.chf))
            {
                m_Ctx->log(RC_LOG_ERROR, "Failed building distance field!");
                continue;
            }

            if (!rcBuildRegions(m_Ctx, *tile.chf, tileCfg.borderSize, tileCfg.minRegionArea, tileCfg.mergeRegionArea))
            {
                m_Ctx->log(RC_LOG_ERROR, "Failed building regions!");
                continue;
            }

            tile.cset = rcAllocContourSet();
            if (!tile.cset || !rcBuildContours(m_Ctx, *tile.chf, tileCfg.maxSimplificationError, tileCfg.maxEdgeLen, *tile.cset))
            {
                m_Ctx->log(RC_LOG_ERROR, "Failed building contours!");
                continue;
            }

            // build polymesh
            tile.pmesh = rcAllocPolyMesh();
            if (!tile.pmesh || !rcBuildPolyMesh(m_Ctx, *tile.cset, tileCfg.maxVertsPerPoly, *tile.pmesh))
            {
                m_Ctx->log(RC_LOG_ERROR, "Failed building polymesh!");
                continue;
            }

            tile.dmesh = rcAllocPolyMeshDetail();
            if (!tile.dmesh || !rcBuildPolyMeshDetail(m_Ctx, *tile.pmesh, *tile.chf, tileCfg.detailSampleDist, tileCfg.detailSampleMaxError, *tile.dmesh))
            {
                m_Ctx->log(RC_LOG_ERROR, "Failed building polymesh detail!");
                continue;
            }

            // free those up
            // we may want to keep them in the future for debug
            // but right now, we don't have the code to merge them
            rcFreeHeightField(tile.solid);
            tile.solid = NULL;
            rcFreeCompactHeightfield(tile.chf);
            tile.chf = NULL;
            rcFreeContourSet(tile.cset);
            tile.cset = NULL;
        }
    }

    // merge per tile poly and detail meshes
    rcPolyMesh** pmmerge = new rcPolyMesh*[RecastDemo::TILES_PER_MAP * RecastDemo::TILES_PER_MAP];
    if (!pmmerge)
    {
        m_Ctx->log(RC_LOG_ERROR, "Alloc pmmerge FIALED!");
        return;
    }

    rcPolyMeshDetail** dmmerge = new rcPolyMeshDetail*[RecastDemo::TILES_PER_MAP * RecastDemo::TILES_PER_MAP];
    if (!dmmerge)
    {
        m_Ctx->log(RC_LOG_ERROR, "Alloc dmmerge FIALED!");
        return;
    }

    int nmerge = 0;
    for (int y = 0; y < RecastDemo::TILES_PER_MAP; ++y)
    {
        for (int x = 0; x < RecastDemo::TILES_PER_MAP; ++x)
        {
            Tile& tile = tiles[x + y * RecastDemo::TILES_PER_MAP];
            if (tile.pmesh)
            {
                pmmerge[nmerge] = tile.pmesh;
                dmmerge[nmerge] = tile.dmesh;
                nmerge++;
            }
        }
    }

    iv.polyMesh = rcAllocPolyMesh();
    if (!iv.polyMesh)
    {
        m_Ctx->log(RC_LOG_ERROR, "Alloc iv.polyMesh FIALED!");
        return;
    }
    rcMergePolyMeshes(m_Ctx, pmmerge, nmerge, *iv.polyMesh);

    iv.polyMeshDetail = rcAllocPolyMeshDetail();
    if (!iv.polyMeshDetail)
    {
        m_Ctx->log(RC_LOG_ERROR, "Alloc m_dmesh FIALED!");
        return;
    }
    rcMergePolyMeshDetails(m_Ctx, dmmerge, nmerge, *iv.polyMeshDetail);

    // free things up
    delete[] pmmerge;
    delete[] dmmerge;

    delete[] tiles;

    // set polygons as walkable
    // TODO: special flags for DYNAMIC polygons, ie surfaces that can be turned on and off
    for (int i = 0; i < iv.polyMesh->npolys; ++i)
        if (iv.polyMesh->areas[i] & RC_WALKABLE_AREA)
            iv.polyMesh->flags[i] = iv.polyMesh->areas[i];

    // setup mesh parameters
    dtNavMeshCreateParams params;
    memset(&params, 0, sizeof(params));
    params.verts = iv.polyMesh->verts;
    params.vertCount = iv.polyMesh->nverts;
    params.polys = iv.polyMesh->polys;
    params.polyAreas = iv.polyMesh->areas;
    params.polyFlags = iv.polyMesh->flags;
    params.polyCount = iv.polyMesh->npolys;
    params.nvp = iv.polyMesh->nvp;
    params.detailMeshes = iv.polyMeshDetail->meshes;
    params.detailVerts = iv.polyMeshDetail->verts;
    params.detailVertsCount = iv.polyMeshDetail->nverts;
    params.detailTris = iv.polyMeshDetail->tris;
    params.detailTriCount = iv.polyMeshDetail->ntris;

    /*params.offMeshConVerts = meshData.offMeshConnections.getCArray();
    params.offMeshConCount = meshData.offMeshConnections.size() / 6;
    params.offMeshConRad = meshData.offMeshConnectionRads.getCArray();
    params.offMeshConDir = meshData.offMeshConnectionDirs.getCArray();
    params.offMeshConAreas = meshData.offMeshConnectionsAreas.getCArray();
    params.offMeshConFlags = meshData.offMeshConnectionsFlags.getCArray();*/

    params.walkableHeight = RecastDemo::BASE_UNIT_DIM * cfg->walkableHeight;  // agent height
    params.walkableRadius = RecastDemo::BASE_UNIT_DIM * cfg->walkableRadius;  // agent radius
    params.walkableClimb = RecastDemo::BASE_UNIT_DIM * cfg->walkableClimb;    // keep less that walkableHeight (aka agent height)!
    params.tileX = (((cfg->bmin[0] + cfg->bmax[0]) / 2) - m_NavMesh->getParams()->orig[0]) / GRID_SIZE;
    params.tileY = (((cfg->bmin[2] + cfg->bmax[2]) / 2) - m_NavMesh->getParams()->orig[2]) / GRID_SIZE;
    rcVcopy(params.bmin, cfg->bmin);
    rcVcopy(params.bmax, cfg->bmax);
    params.cs = cfg->cs;
    params.ch = cfg->ch;
    params.tileLayer = 0;
    params.buildBvTree = true;

    // will hold final navmesh
    unsigned char* navData = NULL;
    int navDataSize = 0;

    do
    {
        // these values are checked within dtCreateNavMeshData - handle them here
        // so we have a clear error message
        if (params.nvp > DT_VERTS_PER_POLYGON)
        {
            m_Ctx->log(RC_LOG_ERROR, " Invalid verts-per-polygon value!");
            continue;
        }
        if (params.vertCount >= 0xffff)
        {
            m_Ctx->log(RC_LOG_ERROR, " Too many vertices!");
            continue;
        }
        if (!params.vertCount || !params.verts)
        {
            // occurs mostly when adjacent tiles have models
            // loaded but those models don't span into this tile

            // message is an annoyance
            //m_Ctx->log(RC_LOG_ERROR, "No vertices to build tile!              ");
            continue;
        }
        if (!params.polyCount || !params.polys ||
            RecastDemo::TILES_PER_MAP * RecastDemo::TILES_PER_MAP == params.polyCount)
        {
            // we have flat tiles with no actual geometry - don't build those, its useless
            // keep in mind that we do output those into debug info
            // drop tiles with only exact count - some tiles may have geometry while having less tiles
            m_Ctx->log(RC_LOG_ERROR, " No polygons to build on tile!");
            continue;
        }
        if (!params.detailMeshes || !params.detailVerts || !params.detailTris)
        {
            m_Ctx->log(RC_LOG_ERROR, " No detail mesh to build tile!");
            continue;
        }

        m_Ctx->log(RC_LOG_ERROR, " Building navmesh tile...");
        if (!dtCreateNavMeshData(&params, &navData, &navDataSize))
        {
            m_Ctx->log(RC_LOG_ERROR, " Failed building navmesh tile!");
            continue;
        }

        TileInfos tinfos;
        rcVcopy(tinfos.bMin, cfg->bmin);
        rcVcopy(tinfos.bMax, cfg->bmax);
        dtStatus dtRes = m_NavMesh->addTile(navData, navDataSize, DT_TILE_FREE_DATA, 0, &tinfos.tileRef);
        if (dtStatusFailed(dtRes))
        {
            m_Ctx->log(RC_LOG_ERROR, "Adding tile to navmesh failed!");
            continue;
        }

        unsigned int pxy = StaticMapTree::packTileID(tileX, tileY);
        m_TilesInfos.insert(std::make_pair(pxy, tinfos));
    } while (0);

    m_Ctx->stopTimer(RC_TIMER_TOTAL);

    // Show performance stats.
    duLogBuildTimes(*m_Ctx, m_Ctx->getAccumulatedTime(RC_TIMER_TOTAL));
    //m_Ctx->log(RC_LOG_PROGRESS, ">> Polymesh: %d vertices  %d polygons", m_pmesh->nverts, m_pmesh->npolys);

    float tileBuildTime = m_Ctx->getAccumulatedTime(RC_TIMER_TOTAL) / 1000.0f;
    float tileMemUsage = navDataSize / 1024.0f;

    m_Ctx->log(RC_LOG_PROGRESS, ">> Memory usage: %f", tileMemUsage);
    m_Ctx->log(RC_LOG_PROGRESS, ">> Build time: %f", tileBuildTime);
}

