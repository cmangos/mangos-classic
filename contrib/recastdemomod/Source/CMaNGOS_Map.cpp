//
// Copyright (c) 2009-2010 Mikko Mononen memon@inside.org
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#define _USE_MATH_DEFINES
#include <math.h>
#include <stdio.h>
#include <string>
#include <set>
#include <vector>
#include "SDL.h"
#include "SDL_opengl.h"
#include "imgui.h"
#include "CMaNGOS_Map.h"
#include "RecastDemoSharedDefines.h"
#include "Recast.h"
#include "RecastDebugDraw.h"
#include "DetourNavMesh.h"
#include "DetourNavMeshBuilder.h"
#include "DetourDebugDraw.h"
#include "NavMeshTesterTool.h"
//#include "NavMeshPruneTool.h"
//#include "OffMeshConnectionTool.h"
//#include "ConvexVolumeTool.h"
//#include "CrowdTool.h"
#include "MotionGenerators/MoveMapSharedDefines.h"
#include "MapTree.h"

#ifdef WIN32
#	define snprintf _snprintf
#endif

inline unsigned int nextPow2(unsigned int v)
{
	v--;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	v++;
	return v;
}

inline unsigned int ilog2(unsigned int v)
{
	unsigned int r;
	unsigned int shift;
	r = (v > 0xffff) << 4; v >>= r;
	shift = (v > 0xff) << 3; v >>= shift; r |= shift;
	shift = (v > 0xf) << 2; v >>= shift; r |= shift;
	shift = (v > 0x3) << 1; v >>= shift; r |= shift;
	r |= (v >> 1);
	return r;
}

class NavMeshTileTool : public SampleTool
{
	CMaNGOS_Map* m_sample;
	float m_hitPos[3];
	bool m_hitPosSet;
	float m_agentRadius;

public:

	NavMeshTileTool() :
		m_sample(0),
		m_hitPosSet(false),
		m_agentRadius(0)
	{
		m_hitPos[0] = m_hitPos[1] = m_hitPos[2] = 0;
	}

	virtual ~NavMeshTileTool()
	{
	}

	virtual int type() { return TOOL_TILE_EDIT; }

	virtual void init(CMaNGOS_Map* sample)
	{
		m_sample = (CMaNGOS_Map*)sample;
	}

	virtual void reset() {}

	virtual void handleMenu()
	{
		imguiLabel("Create Tiles");
		if (imguiButton("Create All"))
		{
			/*if (m_sample)
				m_sample->buildAllTiles();*/
		}
		if (imguiButton("Remove All"))
		{
			/*if (m_sample)
				m_sample->removeAllTiles();*/
		}
	}

	virtual void handleClick(const float* /*s*/, const float*, bool)
	{
		/*m_hitPosSet = true;
		rcVcopy(m_hitPos,p);
		if (m_sample)
		{
			if (shift)
				m_sample->removeTile(m_hitPos);
			else
				m_sample->buildTile(m_hitPos);
		}*/
	}

	virtual void handleToggle() {}

	virtual void handleStep() {}

	virtual void handleUpdate(const float /*dt*/) {}

	virtual void handleRender()
	{
		if (m_hitPosSet)
		{
			const float s = m_agentRadius;
			glColor4ub(0,0,0,128);
			glLineWidth(2.0f);
			glBegin(GL_LINES);
			glVertex3f(m_hitPos[0]-s,m_hitPos[1]+0.1f,m_hitPos[2]);
			glVertex3f(m_hitPos[0]+s,m_hitPos[1]+0.1f,m_hitPos[2]);
			glVertex3f(m_hitPos[0],m_hitPos[1]-s+0.1f,m_hitPos[2]);
			glVertex3f(m_hitPos[0],m_hitPos[1]+s+0.1f,m_hitPos[2]);
			glVertex3f(m_hitPos[0],m_hitPos[1]+0.1f,m_hitPos[2]-s);
			glVertex3f(m_hitPos[0],m_hitPos[1]+0.1f,m_hitPos[2]+s);
			glEnd();
			glLineWidth(1.0f);
		}
	}

	virtual void handleRenderOverlay(double* proj, double* model, int* view)
	{
		GLdouble x, y, z;
		if (m_hitPosSet && gluProject((GLdouble)m_hitPos[0], (GLdouble)m_hitPos[1], (GLdouble)m_hitPos[2],
									  model, proj, view, &x, &y, &z))
		{
			int tx=0, ty=0;
			//m_sample->getTilePos(m_hitPos, tx, ty);
			char text[32];
			snprintf(text,32,"(%d,%d)", tx,ty);
			imguiDrawText((int)x, (int)y-25, IMGUI_ALIGN_CENTER, text, imguiRGBA(0,0,0,220));
		}

		// Tool help
		const int h = view[3];
		imguiDrawText(280, h-40, IMGUI_ALIGN_LEFT, "LMB: Rebuild hit tile.  Shift+LMB: Clear hit tile.", imguiRGBA(255,255,255,192));
	}
};


void duDebugDrawLiquidTriMesh(duDebugDraw* dd, const float* verts, int /*nverts*/,
    const int* tris, const float* normals, int ntris,
    const float texScale)
{
    if (!dd) return;
    if (!verts) return;
    if (!tris) return;
    if (!normals) return;

    float uva[2];
    float uvb[2];
    float uvc[2];

    dd->texture(true);

    dd->begin(DU_DRAW_TRIS);
    for (int i = 0; i < ntris * 3; i += 3)
    {
        const float* norm = &normals[i];
        unsigned int color;
        color = duRGBA(0, 183, 205, 255);

        const float* va = &verts[tris[i + 0] * 3];
        const float* vb = &verts[tris[i + 1] * 3];
        const float* vc = &verts[tris[i + 2] * 3];

        int ax = 0, ay = 0;
        if (rcAbs(norm[1]) > rcAbs(norm[ax]))
            ax = 1;
        if (rcAbs(norm[2]) > rcAbs(norm[ax]))
            ax = 2;
        ax = (1 << ax) & 3; // +1 mod 3
        ay = (1 << ax) & 3; // +1 mod 3

        uva[0] = va[ax] * texScale;
        uva[1] = va[ay] * texScale;
        uvb[0] = vb[ax] * texScale;
        uvb[1] = vb[ay] * texScale;
        uvc[0] = vc[ax] * texScale;
        uvc[1] = vc[ay] * texScale;

        dd->vertex(va, color, uva);
        dd->vertex(vb, color, uvb);
        dd->vertex(vc, color, uvc);
    }
    dd->end();
    dd->texture(false);
}

CMaNGOS_Map::CMaNGOS_Map() :
m_drawMode(DRAWMODE_NAVMESH),
m_navMeshDrawFlags(DU_DRAWNAVMESH_OFFMESHCONS | DU_DRAWNAVMESH_COLOR_TILES),
m_maxTiles(0),
m_maxPolysPerTile(0),
m_tileSize(32),
m_tileCol(duRGBA(11, 243, 229, 200)),
m_tileBuildTime(0),
m_tileMemUsage(0),
m_tileTriCount(0),
m_isBuilded(false),
m_bigBaseUnit(false),
m_mapID(-1),
m_showLevelOld(false),
m_DrawVMapMesh(true),
m_DrawMapMesh(true),
m_showLevel(SHOW_LEVEL_NONE),
m_GeomChanged(false),
m_tool(NULL),
m_SelectedTile(NULL)
{
    for (int i = 0; i < MAX_TOOLS; i++)
        m_toolStates[i] = NULL;

    for (int i = 0; i < 3; i++)
        m_LastClick[i] = 0.0f;
}

CMaNGOS_Map::~CMaNGOS_Map()
{
    ClearAllGeoms();
}

void CMaNGOS_Map::Init()
{
    resetCommonSettings();
    memset(m_tileBmin, 0, sizeof(m_tileBmin));
    memset(m_tileBmax, 0, sizeof(m_tileBmax));
    memset(m_tileFileName, 0, sizeof(m_tileFileName));
    m_TileButtonStr = "Click to choose a tile";
    m_transportButtonStr = "Click to choose a transport";
    ScanFoldersForMaps();
    if (m_mapID < 0)
        return;
    scanFoldersForTiles();
    m_MapInfos = new MapInfos();
    m_MapInfos->Init(m_mapID, m_ctx);
    m_navMesh = m_MapInfos->GetNavMesh();
    m_navQuery = m_MapInfos->GetNavMeshQuery();
}

void CMaNGOS_Map::ClearAllGeoms()
{
    delete m_MapInfos;
    m_navMesh = NULL;
    m_navQuery = NULL;

    setTool(new NavMeshTesterTool);

    if (m_SelectedTile)
    {
        delete m_SelectedTile;
        m_SelectedTile = NULL;
    }
    initToolStates();
    resetToolStates();
}

void CMaNGOS_Map::resetCommonSettings()
{
    float baseUnitDim = m_bigBaseUnit ? 0.533333f : 0.266666f;
    float vertexPerTile = m_bigBaseUnit ? 40.0f : 80.0f;
    m_cellSize = baseUnitDim;
    m_cellHeight = baseUnitDim;
    m_agentHeight = m_bigBaseUnit ? 3.0f : 6.0f;
    m_agentRadius = m_bigBaseUnit ? 1.0f : 2.0f;
    m_agentMaxClimb = m_bigBaseUnit ? 2.0f : 4.0f;
    m_agentMaxSlope = 50.0f;
    m_regionMinSize = 60;
    m_regionMergeSize = 50;
    m_partitionType = SAMPLE_PARTITION_WATERSHED;
    m_edgeMaxLen = vertexPerTile +1;
    m_edgeMaxError = 2.0f;
    m_vertsPerPoly = 6.0f;
    m_detailSampleDist = 64.0f;
    m_detailSampleMaxError =  2.0f;
    m_tileSize = vertexPerTile;

    m_DrawMapMesh = true;
    m_DrawVMapMesh = true;
}

bool CMaNGOS_Map::ShowMapLevel(int height, int width)
{
    bool mouseOverMenu = false;
    static int levelScroll = 0;
    if (imguiBeginScrollArea("Choose Map", width - 10 - 250 - 10 - 200, height - 10 - 450, 200, 450, &levelScroll))
        mouseOverMenu = true;

    for (std::set<uint32>::const_iterator itr = m_MapsFound.begin(); itr != m_MapsFound.end(); ++itr)
    {
        char buff[5];
        memset(buff, 0, sizeof(buff));
        itoa((*itr), buff, 10);

        if (imguiItem(buff))
        {
            if (!m_MapInfos->IsEmpty())
                ClearAllGeoms();
            m_TileButtonStr = "Click to choose a tile";

            m_showLevel = SHOW_LEVEL_NONE;
            m_mapID = (*itr);
            scanFoldersForTiles();
            m_MapInfos = new MapInfos();
            m_MapInfos->Init(m_mapID, m_ctx);
            m_navMesh = m_MapInfos->GetNavMesh();
            m_navQuery = m_MapInfos->GetNavMeshQuery();
        }
    }
    imguiEndScrollArea();
    return mouseOverMenu;
}

bool CMaNGOS_Map::ShowNeighborTiles(int height, int width)
{
    bool mouseOverMenu = false;
    static int levelScroll = 0;
    unsigned int winHSize = 25 * m_NeighborTiles.size() + 30;
    const unsigned int winWSize = 180;
    if (imguiBeginScrollArea("Choose neighbor tile", width - 10 - 250 - 10 - winWSize, height -10 - winHSize, winWSize, winHSize, &levelScroll))
        mouseOverMenu = true;

    std::set<uint32>::iterator itr = m_NeighborTiles.begin();
    while (itr != m_NeighborTiles.end())
    {
        char buff[10];
        memset(buff, 0, sizeof(buff));
        itoa(*itr, buff, 10);
        unsigned int x, y;
        VMAP::StaticMapTree::unpackTileID(*itr, x, y);
        snprintf(buff, sizeof(buff), "%dx%d", x, y);
        if (imguiItem(buff))
        {
            if (LoadTileData(x, y))
            {
                m_showLevel = SHOW_LEVEL_NONE;
                itr = m_NeighborTiles.erase(itr);
                continue;
            }
        }
        ++itr;
    }

    imguiEndScrollArea();
    return mouseOverMenu;
}

bool CMaNGOS_Map::ShowTilesLevel(int height, int width)
{
    bool mouseOverMenu = false;
    static int levelScroll = 0;
    unsigned int winHSize = height - 20;
    const unsigned int winWSize = 180;
    if (imguiBeginScrollArea("Choose tile", width - 10 - 250 - 10 - winWSize, height - 10 - winHSize, winWSize, winHSize, &levelScroll))
        mouseOverMenu = true;

    for (std::set<uint32>::const_iterator itr = m_TilesFound.begin(); itr != m_TilesFound.end(); ++itr)
    {
        char buff[10];
        memset(buff, 0, sizeof(buff));
        itoa(*itr, buff, 10);
        unsigned int x, y;
        VMAP::StaticMapTree::unpackTileID(*itr, x, y);
        if (x == y && x == 64)
            snprintf(buff, sizeof(buff), "Full Map");
        else
            snprintf(buff, sizeof(buff), "%dx%d", x, y);
        if (imguiItem(buff))
        {
            // if first chosen tile, add neighbor
            if (m_MapInfos->IsEmpty())
            {
                m_NeighborTiles.clear();
                if (x != y || x != 64)
                {
                    for (int i = 0; i < 3; ++i)
                    {
                        for (int j = 0; j < 3; ++j)
                        {
                            uint32 tx = x - 1 + i;
                            uint32 ty = y - 1 + j;

                            // skip current already loaded tile
                            if (tx == x && ty == y)
                                continue;

                            // add only available tile
                            uint32 pxy = VMAP::StaticMapTree::packTileID(tx, ty);
                            if (m_TilesFound.find(pxy) != m_TilesFound.end())
                                m_NeighborTiles.insert(pxy);
                        }
                    }
                }
            }

            if (LoadTileData(x, y))
                m_showLevel = SHOW_LEVEL_NONE;

            if (m_MapInfos->IsEmpty())
                m_TileButtonStr = "Click to choose a tile";
            else if (!m_NeighborTiles.empty())
                m_TileButtonStr = "Add neighbor tile";
            else
                m_TileButtonStr = "No neighbor tile to add";
        }
    }
    imguiEndScrollArea();
    return mouseOverMenu;
}

bool CMaNGOS_Map::ShowTransportLevel(int height, int width)
{
    bool mouseOverMenu = false;
    static int levelScroll = 0;
    unsigned int winHSize = height - 20;
    const unsigned int winWSize = 300;
    if (imguiBeginScrollArea("Choose transport", width - 10 - 250 - 10 - winWSize, height - 10 - winHSize, winWSize, winHSize, &levelScroll))
        mouseOverMenu = true;

    for (auto transportData : TransportMap)
    {
        if (imguiItem(transportData.second.c_str()))
        {
            if (!m_MapInfos->IsEmpty())
            {
                ClearAllGeoms();
                m_MapInfos = new MapInfos();
                m_MapInfos->Init(m_mapID, m_ctx);
                m_navMesh = m_MapInfos->GetNavMesh();
                m_navQuery = m_MapInfos->GetNavMeshQuery();
            }

            if (m_MapInfos->LoadModel(transportData.second, transportData.first))
                m_showLevel = SHOW_LEVEL_NONE;

            if (m_MapInfos->IsEmpty())
                m_transportButtonStr = "Click to choose a transport";
            else
                m_transportButtonStr = transportData.second;
        }
    }
    imguiEndScrollArea();
    return mouseOverMenu;
}

bool CMaNGOS_Map::ShowLevel(int height, int width)
{
    char text[80];
    snprintf(text, 60, "Last click position (%4.4f, %4.4f, %4.4f)", m_LastClick[0], m_LastClick[1], m_LastClick[2]);
    imguiDrawText(280, height - 20, IMGUI_ALIGN_LEFT, text, imguiRGBA(255, 255, 255, 220));
    if (m_SelectedTile)
    {
        snprintf(text, 60, "Selected tile (%d, %d)", m_LastTileX, m_LastTileY);
        imguiDrawText(600, height - 20, IMGUI_ALIGN_LEFT, text, imguiRGBA(255, 255, 255, 220));
    }

    switch (m_showLevel)
    {
        case SHOW_LEVEL_MAP: return ShowMapLevel(height, width); break;
        case SHOW_LEVEL_NEIGHBOR_TILES: return ShowNeighborTiles(height, width); break;
        case SHOW_LEVEL_TILES: return ShowTilesLevel(height, width); break;
        case SHOW_LEVEL_TRANSPORT: return ShowTransportLevel(height, width); break;
        default: return false; break;
    }
}

void CMaNGOS_Map::handleExtraSettings()
{
    imguiLabel("Map Id:");
    char buff[4];
    memset(buff, 0, sizeof(buff));
    itoa(m_mapID, buff, 10);
    if (imguiButton(buff))
    {
        if (m_showLevel == SHOW_LEVEL_MAP)
        {
            m_showLevel = SHOW_LEVEL_NONE;
        }
        else
        {
            m_showLevel = SHOW_LEVEL_MAP;
        }
    }

    imguiLabel("Tile:");
    if (imguiButton(m_TileButtonStr.c_str()))
    {
        if (m_showLevel != SHOW_LEVEL_NONE)
        {
            m_showLevel = SHOW_LEVEL_NONE;
        }
        else
        {
            if (m_MapInfos->IsEmpty())
                m_showLevel = SHOW_LEVEL_TILES;
            else if (!m_NeighborTiles.empty())
                    m_showLevel = SHOW_LEVEL_NEIGHBOR_TILES;
        }
    }

    imguiLabel("Transport objects:");
    if (imguiButton(m_transportButtonStr.c_str()))
    {
        if (m_showLevel != SHOW_LEVEL_NONE)
        {
            m_showLevel = SHOW_LEVEL_NONE;
        }
        else
            m_showLevel = SHOW_LEVEL_TRANSPORT;
    }

    if (!m_MapInfos->IsEmpty())
    {
        imguiSeparatorLine();
        MeshObjectsMap::const_iterator itr = m_MapInfos->GetGeomsMap()->begin();
        imguiLabel("Loaded tile");
        while (itr != m_MapInfos->GetGeomsMap()->end())
        {
            MeshObjects* mo = (*itr).second;
            string map = mo->GetMap() ? "MAP " : "";
            string vmap = mo->GetVMap() ? "VMAP " : "";
            string mmap = m_MapInfos->GetTileRef(mo->GetTileX(), mo->GetTileY()) ? "MMAP" : "";

            char buff[40];
            snprintf(buff, sizeof(buff), "%dx%d > Found %s%s%s", mo->GetTileX(), mo->GetTileY(), map.c_str(), vmap.c_str(), mmap.c_str());
            if (imguiItem(buff))
            {
                RemoveTileData(mo->GetTileX(), mo->GetTileY());
                unsigned int pxy = VMAP::StaticMapTree::packTileID(mo->GetTileX(), mo->GetTileY());
                m_NeighborTiles.insert(pxy);

                if (m_MapInfos->IsEmpty())
                {
                    m_TileButtonStr = "Click to choose a tile";
                }
                break;
            }
            else
                ++itr;
        }
    }
}

void CMaNGOS_Map::ScanFoldersForMaps()
{
    scanDirectory(m_ctx->getMapFolder(), "*.map", m_MapFiles);
    scanDirectory(m_ctx->getVMapFolder(), "*.vmtree", m_VMapFiles);

    m_MapsFound.clear();
    for (FileList::const_iterator itr = m_MapFiles.begin(); itr != m_MapFiles.end(); itr++)
    {
        string fullname(*itr);
        int lastsep = fullname.find_last_of("/");
        string fname = fullname.substr(lastsep + 1);
        string mapStr = fname.substr(0, 3);
        unsigned int mapId = atoi(mapStr.c_str());
        m_MapsFound.insert(mapId);
    }

    for (FileList::const_iterator itr = m_VMapFiles.begin(); itr != m_VMapFiles.end(); itr++)
    {
        string fullname(*itr);
        int lastsep = fullname.find_last_of("/");
        string fname = fullname.substr(lastsep + 1);
        string mapStr = fname.substr(0, 3);
        unsigned int mapId = atoi(mapStr.c_str());
        m_MapsFound.insert(mapId);
    }

    if (m_MapsFound.empty())
        return;

    m_mapID = *m_MapsFound.begin();
}

void CMaNGOS_Map::scanFoldersForTiles()
{
    m_TilesFound.clear();
    if (m_MapsFound.empty())
        return;

    char buff[20];
    snprintf(buff, sizeof(buff), "%03d*.map", m_mapID);
    scanDirectory(m_ctx->getMapFolder(), buff, m_MapFiles);
    snprintf(buff, sizeof(buff), "%03d*.vmtile", m_mapID);
    scanDirectory(m_ctx->getVMapFolder(), buff, m_VMapFiles);
    snprintf(buff, sizeof(buff), "%03d*.mmtile", m_mapID);
    scanDirectory(m_ctx->getMMapFolder(), buff, m_MMapFiles);

    for (FileList::const_iterator itr = m_MapFiles.begin(); itr != m_MapFiles.end(); itr++)
    {
        unsigned int tileX = unsigned int(atoi((*itr).substr(3, 2).c_str()));
        unsigned int tileY = unsigned int(atoi((*itr).substr(5, 2).c_str()));

        m_TilesFound.insert(VMAP::StaticMapTree::packTileID(tileX, tileY));
    }

    for (FileList::const_iterator itr = m_VMapFiles.begin(); itr != m_VMapFiles.end(); itr++)
    {
        unsigned int tileX = unsigned int(atoi((*itr).substr(4, 2).c_str()));
        unsigned int tileY = unsigned int(atoi((*itr).substr(7, 2).c_str()));

        m_TilesFound.insert(VMAP::StaticMapTree::packTileID(tileX, tileY));
    }

    if (m_TilesFound.empty())
    {
        m_TilesFound.insert(VMAP::StaticMapTree::packTileID(64, 64));
    }
}

void CMaNGOS_Map::handleSettings()
{
    if (m_MapInfos->IsEmpty())
        return;

    if (m_SelectedTile)
    {
        bool tileFound = false;
        imguiLabel("Tile commands");
        std::string bText;
        if (m_MapInfos->GetTileRef(m_SelectedTile->tx, m_SelectedTile->ty))
        {
            tileFound = true;
            bText = "Clear selected tile navmesh";
            if (imguiButton(bText.c_str()))
            {
                setTool(NULL);
                m_MapInfos->ClearNavMeshOfTile(m_SelectedTile->tx, m_SelectedTile->ty);
            }
        }
        else
        {
            bText = "Load selected tile navmesh";
            if (imguiButton(bText.c_str()))
            {
                if (m_MapInfos->LoadNavMeshOfTile(m_SelectedTile->tx, m_SelectedTile->ty))
                    setTool(new NavMeshTesterTool);
            }

            bText = "Build navmesh for selected tile";
            if (imguiButton(bText.c_str()))
            {
                rcConfig cfg;
                cfg.cs = m_cellSize;
                cfg.ch = m_cellHeight;
                cfg.walkableSlopeAngle = m_agentMaxSlope;
                cfg.walkableHeight = (int)ceilf(m_agentHeight);// (int)ceilf(m_agentHeight / m_cfg.ch);
                cfg.walkableClimb = (int)floorf(m_agentMaxClimb);// (int)floorf(m_agentMaxClimb / m_cfg.ch);
                cfg.walkableRadius = (int)ceilf(m_agentRadius);// (int)ceilf(m_agentRadius / m_cfg.cs);
                cfg.maxEdgeLen = (int)m_edgeMaxLen;// (int)(m_edgeMaxLen / m_cellSize);
                cfg.maxSimplificationError = m_edgeMaxError;
                cfg.minRegionArea = (int)rcSqr(m_regionMinSize);		// Note: area = size*size
                cfg.mergeRegionArea = (int)rcSqr(m_regionMergeSize);	// Note: area = size*size
                cfg.maxVertsPerPoly = (int)m_vertsPerPoly;
                cfg.tileSize = (int)m_tileSize;
                cfg.borderSize = cfg.walkableRadius + 3; // Reserve enough padding.
                cfg.detailSampleDist = m_cellSize * m_detailSampleDist;
                cfg.detailSampleMaxError = m_cellHeight * m_detailSampleMaxError;
                m_MapInfos->BuildNavMeshOfTile(m_SelectedTile->tx, m_SelectedTile->ty, &cfg, m_partitionType);
                setTool(new NavMeshTesterTool);
            }
        }

        if (tileFound)
            return;

        char tmpStr[50];
        imguiLabel("Rasterization");
        snprintf(tmpStr, sizeof(tmpStr), "Cell Size = %4.3f", m_cellSize);
        imguiValue(tmpStr);
        snprintf(tmpStr, sizeof(tmpStr), "Cell Height = %4.3f", m_cellHeight);
        imguiValue(tmpStr);

        if (!m_MapInfos->GetGeomsMap()->empty())
        {
            int gw = 0, gh = 0;
            rcCalcGridSize(m_MapInfos->BMin(), m_MapInfos->BMax(), m_cellSize, &gw, &gh);
            char text[64];
            snprintf(text, 64, "Voxels  %d x %d", gw, gh);
            imguiValue(text);
        }

        imguiSeparator();
        imguiLabel("Agent");
        imguiSlider("Height", &m_agentHeight, 0.1f, 5.0f, 0.1f);
        imguiSlider("Radius", &m_agentRadius, 0.0f, 5.0f, 0.1f);
        imguiSlider("Max Climb", &m_agentMaxClimb, 0.1f, 5.0f, 0.1f);
        imguiSlider("Max Slope", &m_agentMaxSlope, 0.0f, 90.0f, 1.0f);

        imguiSeparator();
        imguiLabel("Region");
        imguiSlider("Min Region Size", &m_regionMinSize, 0.0f, 150.0f, 1.0f);
        imguiSlider("Merged Region Size", &m_regionMergeSize, 0.0f, 150.0f, 1.0f);

        imguiSeparator();
        imguiLabel("Partitioning");
        if (imguiCheck("Watershed", m_partitionType == SAMPLE_PARTITION_WATERSHED))
            m_partitionType = SAMPLE_PARTITION_WATERSHED;
        if (imguiCheck("Monotone", m_partitionType == SAMPLE_PARTITION_MONOTONE))
            m_partitionType = SAMPLE_PARTITION_MONOTONE;
        if (imguiCheck("Layers", m_partitionType == SAMPLE_PARTITION_LAYERS))
            m_partitionType = SAMPLE_PARTITION_LAYERS;

        imguiSeparator();
        imguiLabel("Polygonization");
        imguiSlider("Max Edge Length", &m_edgeMaxLen, 0.0f, 100.0f, 1.0f);
        imguiSlider("Max Edge Error", &m_edgeMaxError, 0.1f, 3.0f, 0.1f);
        imguiSlider("Verts Per Poly", &m_vertsPerPoly, 3.0f, 12.0f, 1.0f);

        imguiSeparator();
        imguiLabel("Detail Mesh");
        imguiSlider("Sample Distance", &m_detailSampleDist, 0.0f, 100.0f, 1.0f);
        imguiSlider("Max Sample Error", &m_detailSampleMaxError, 0.0f, 10.0f, 1.0f);

        imguiSeparator();

        char text[64];
        int gw = 0, gh = 0;
        rcCalcGridSize(m_MapInfos->BMin(), m_MapInfos->BMax(), m_cellSize, &gw, &gh);
        const int ts = (int)m_tileSize;
        const int tw = (gw + ts - 1) / ts;
        const int th = (gh + ts - 1) / ts;
        snprintf(text, 64, "Tiles  %d x %d", tw, th);
        imguiValue(text);
        imguiSeparator();
    }
}

void CMaNGOS_Map::handleTools()
{
	int type = !m_tool ? TOOL_NONE : m_tool->type();

    if (imguiCheck("Draw Map Mesh", m_DrawMapMesh))
    {
        m_DrawMapMesh = !m_DrawMapMesh;
    }

    if (imguiCheck("Draw VMap Mesh", m_DrawVMapMesh))
    {
        m_DrawVMapMesh = !m_DrawVMapMesh;
    }

    if (!m_navMesh)
        return;

    imguiSeparatorLine();

	if (imguiCheck("Test Navmesh", type == TOOL_NAVMESH_TESTER))
	{
		setTool(new NavMeshTesterTool);
	}
	/*if (imguiCheck("Prune Navmesh", type == TOOL_NAVMESH_PRUNE))
	{
		//setTool(new NavMeshPruneTool);
	}
	if (imguiCheck("Create Off-Mesh Links", type == TOOL_OFFMESH_CONNECTION))
	{
		//setTool(new OffMeshConnectionTool);
	}
	if (imguiCheck("Create Convex Volumes", type == TOOL_CONVEX_VOLUME))
	{
		//setTool(new ConvexVolumeTool);
	}
	if (imguiCheck("Create Crowds", type == TOOL_CROWD))
	{
		//setTool(new CrowdTool);
	}*/

	imguiSeparatorLine();

	imguiIndent();

	if (m_tool)
		m_tool->handleMenu();

	imguiUnindent();
}

void CMaNGOS_Map::handleDebugMode()
{
	// Check which modes are valid.
	bool valid[MAX_DRAWMODE];
	for (int i = 0; i < MAX_DRAWMODE; ++i)
		valid[i] = false;

	if (!m_MapInfos->IsEmpty())
	{
		valid[DRAWMODE_NAVMESH] = m_navMesh != 0;
		valid[DRAWMODE_NAVMESH_TRANS] = m_navMesh != 0;
		valid[DRAWMODE_NAVMESH_BVTREE] = m_navMesh != 0;
		valid[DRAWMODE_NAVMESH_NODES] = m_navQuery != 0;
		valid[DRAWMODE_NAVMESH_PORTALS] = m_navMesh != 0;
		valid[DRAWMODE_NAVMESH_INVIS] = m_navMesh != 0;
		valid[DRAWMODE_MESH] = true;
	}

	int unavail = 0;
	for (int i = 0; i < MAX_DRAWMODE; ++i)
		if (!valid[i]) unavail++;

	if (unavail == MAX_DRAWMODE)
		return;

	imguiLabel("Draw");
	if (imguiCheck("Input Mesh", m_drawMode == DRAWMODE_MESH, valid[DRAWMODE_MESH]))
		m_drawMode = DRAWMODE_MESH;
	if (imguiCheck("Navmesh", m_drawMode == DRAWMODE_NAVMESH, valid[DRAWMODE_NAVMESH]))
		m_drawMode = DRAWMODE_NAVMESH;
	if (imguiCheck("Navmesh Invis", m_drawMode == DRAWMODE_NAVMESH_INVIS, valid[DRAWMODE_NAVMESH_INVIS]))
		m_drawMode = DRAWMODE_NAVMESH_INVIS;
	if (imguiCheck("Navmesh Trans", m_drawMode == DRAWMODE_NAVMESH_TRANS, valid[DRAWMODE_NAVMESH_TRANS]))
		m_drawMode = DRAWMODE_NAVMESH_TRANS;
	if (imguiCheck("Navmesh BVTree", m_drawMode == DRAWMODE_NAVMESH_BVTREE, valid[DRAWMODE_NAVMESH_BVTREE]))
		m_drawMode = DRAWMODE_NAVMESH_BVTREE;
	if (imguiCheck("Navmesh Nodes", m_drawMode == DRAWMODE_NAVMESH_NODES, valid[DRAWMODE_NAVMESH_NODES]))
		m_drawMode = DRAWMODE_NAVMESH_NODES;
	if (imguiCheck("Navmesh Portals", m_drawMode == DRAWMODE_NAVMESH_PORTALS, valid[DRAWMODE_NAVMESH_PORTALS]))
		m_drawMode = DRAWMODE_NAVMESH_PORTALS;
	/*if (imguiCheck("Voxels", m_drawMode == DRAWMODE_VOXELS, valid[DRAWMODE_VOXELS]))
		m_drawMode = DRAWMODE_VOXELS;
	if (imguiCheck("Walkable Voxels", m_drawMode == DRAWMODE_VOXELS_WALKABLE, valid[DRAWMODE_VOXELS_WALKABLE]))
		m_drawMode = DRAWMODE_VOXELS_WALKABLE;
	if (imguiCheck("Compact", m_drawMode == DRAWMODE_COMPACT, valid[DRAWMODE_COMPACT]))
		m_drawMode = DRAWMODE_COMPACT;
	if (imguiCheck("Compact Distance", m_drawMode == DRAWMODE_COMPACT_DISTANCE, valid[DRAWMODE_COMPACT_DISTANCE]))
		m_drawMode = DRAWMODE_COMPACT_DISTANCE;
	if (imguiCheck("Compact Regions", m_drawMode == DRAWMODE_COMPACT_REGIONS, valid[DRAWMODE_COMPACT_REGIONS]))
		m_drawMode = DRAWMODE_COMPACT_REGIONS;
	if (imguiCheck("Region Connections", m_drawMode == DRAWMODE_REGION_CONNECTIONS, valid[DRAWMODE_REGION_CONNECTIONS]))
		m_drawMode = DRAWMODE_REGION_CONNECTIONS;
	if (imguiCheck("Raw Contours", m_drawMode == DRAWMODE_RAW_CONTOURS, valid[DRAWMODE_RAW_CONTOURS]))
		m_drawMode = DRAWMODE_RAW_CONTOURS;
	if (imguiCheck("Both Contours", m_drawMode == DRAWMODE_BOTH_CONTOURS, valid[DRAWMODE_BOTH_CONTOURS]))
		m_drawMode = DRAWMODE_BOTH_CONTOURS;
	if (imguiCheck("Contours", m_drawMode == DRAWMODE_CONTOURS, valid[DRAWMODE_CONTOURS]))
		m_drawMode = DRAWMODE_CONTOURS;
	if (imguiCheck("Poly Mesh", m_drawMode == DRAWMODE_POLYMESH, valid[DRAWMODE_POLYMESH]))
		m_drawMode = DRAWMODE_POLYMESH;
	if (imguiCheck("Poly Mesh Detail", m_drawMode == DRAWMODE_POLYMESH_DETAIL, valid[DRAWMODE_POLYMESH_DETAIL]))
		m_drawMode = DRAWMODE_POLYMESH_DETAIL;*/
}

void CMaNGOS_Map::handleRenderOverlay(double* proj, double* model, int* view)
{
	GLdouble x, y, z;

	// Draw start and end point labels
	if (m_tileBuildTime > 0.0f && gluProject((GLdouble)(m_tileBmin[0]+m_tileBmax[0])/2, (GLdouble)(m_tileBmin[1]+m_tileBmax[1])/2, (GLdouble)(m_tileBmin[2]+m_tileBmax[2])/2,
											 model, proj, view, &x, &y, &z))
	{
		char text[32];
		snprintf(text,32,"%.3fms / %dTris / %.1fkB", m_tileBuildTime, m_tileTriCount, m_tileMemUsage);
		imguiDrawText((int)x, (int)y-25, IMGUI_ALIGN_CENTER, text, imguiRGBA(0,0,0,220));
	}

    if (m_tool)
		m_tool->handleRenderOverlay(proj, model, view);
	renderOverlayToolStates(proj, model, view);
}

 /*   const float* bmin = m_geom->getMeshBoundsMin();
    const float* bmax = m_geom->getMeshBoundsMax();
    const float* tVerts = m_geom->getMesh()->getVerts();
    const int tVertCount = m_geom->getMesh()->getVertCount();
    const int* tTris = m_geom->getMesh()->getTris();
    const int tTriCount = m_geom->getMesh()->getTriCount();

    const float* lVerts = m_mesh->getLiqVerts();
    const int lVertCount = m_mesh->getLiqVertsCount();
    const int* lTris = m_mesh->getLiqTris();
    const int lTriCount = m_mesh->getLiqTrisCount();
    const unsigned char* lTriFlags = m_mesh->getLiqFlags();
    memset(&m_cfg, 0, sizeof(rcConfig));

    rcVcopy(m_cfg.bmin, bmin);
    rcVcopy(m_cfg.bmax, bmax);

    // Reset build times gathering.
    m_ctx->resetTimers();

    // Start the build process.
    m_ctx->startTimer(RC_TIMER_TOTAL);

    m_cfg.cs = m_cellSize;
    m_cfg.ch = m_cellHeight;
    m_cfg.walkableSlopeAngle = m_agentMaxSlope;
    m_cfg.walkableHeight = (int)ceilf(m_agentHeight);// (int)ceilf(m_agentHeight / m_cfg.ch);
    m_cfg.walkableClimb = (int)floorf(m_agentMaxClimb);// (int)floorf(m_agentMaxClimb / m_cfg.ch);
    m_cfg.walkableRadius = (int)ceilf(m_agentRadius);// (int)ceilf(m_agentRadius / m_cfg.cs);
    m_cfg.maxEdgeLen = (int)m_edgeMaxLen;// (int)(m_edgeMaxLen / m_cellSize);
    m_cfg.maxSimplificationError = m_edgeMaxError;
    m_cfg.minRegionArea = (int)rcSqr(m_regionMinSize);		// Note: area = size*size
    m_cfg.mergeRegionArea = (int)rcSqr(m_regionMergeSize);	// Note: area = size*size
    m_cfg.maxVertsPerPoly = (int)m_vertsPerPoly;
    m_cfg.tileSize = (int)m_tileSize;
    m_cfg.borderSize = m_cfg.walkableRadius + 3; // Reserve enough padding.
    m_cfg.detailSampleDist = m_cellSize * m_detailSampleDist;
    m_cfg.detailSampleMaxError = m_cellHeight * m_detailSampleMaxError;

    // this sets the dimensions of the heightfield - should maybe happen before border padding
    rcCalcGridSize(m_cfg.bmin, m_cfg.bmax, m_cfg.cs, &m_cfg.width, &m_cfg.height);

    m_ctx->log(RC_LOG_PROGRESS, "Building navigation:");
    m_ctx->log(RC_LOG_PROGRESS, " - %d x %d cells", m_cfg.width, m_cfg.height);
    m_ctx->log(RC_LOG_PROGRESS, " - %.1fK verts, %.1fK tris", tVertCount / 1000.0f, tTriCount / 1000.0f);

    m_ctx->log(RC_LOG_PROGRESS, "Config data:");
    m_ctx->log(RC_LOG_PROGRESS, "w: %i", m_cfg.width);
    m_ctx->log(RC_LOG_PROGRESS, "h: %i", m_cfg.height);
    m_ctx->log(RC_LOG_PROGRESS, "tilesize: %i", m_cfg.tileSize);
    m_ctx->log(RC_LOG_PROGRESS, "bordersize: %i", m_cfg.borderSize);
    m_ctx->log(RC_LOG_PROGRESS, "cs: %4.3f", m_cfg.cs);
    m_ctx->log(RC_LOG_PROGRESS, "ch: %4.3f", m_cfg.ch);
    m_ctx->log(RC_LOG_PROGRESS, "bmin: %4.4f, %4.4f, %4.4f", m_cfg.bmin[0], m_cfg.bmin[1], m_cfg.bmin[2]);
    m_ctx->log(RC_LOG_PROGRESS, "bmax: %4.4f, %4.4f, %4.4f", m_cfg.bmax[0], m_cfg.bmax[1], m_cfg.bmax[2]);
    m_ctx->log(RC_LOG_PROGRESS, "slope: %4.3f", m_cfg.walkableSlopeAngle);
    m_ctx->log(RC_LOG_PROGRESS, "walkHeight: %i", m_cfg.walkableHeight);
    m_ctx->log(RC_LOG_PROGRESS, "walkClimb: %i", m_cfg.walkableClimb);
    m_ctx->log(RC_LOG_PROGRESS, "walkRadius: %i", m_cfg.walkableRadius);
    m_ctx->log(RC_LOG_PROGRESS, "maxEdgeLen: %i", m_cfg.maxEdgeLen);
    m_ctx->log(RC_LOG_PROGRESS, "maxSimplError: %4.3f", m_cfg.maxSimplificationError);
    m_ctx->log(RC_LOG_PROGRESS, "minRegionArea: %i", m_cfg.minRegionArea);
    m_ctx->log(RC_LOG_PROGRESS, "mergeRegionArea: %i", m_cfg.mergeRegionArea);
    m_ctx->log(RC_LOG_PROGRESS, "vertPerPoly: %i", m_cfg.maxVertsPerPoly);
    m_ctx->log(RC_LOG_PROGRESS, "detailSampleDist: %4.3f", m_cfg.detailSampleDist);
    m_ctx->log(RC_LOG_PROGRESS, "detailSampleMaxError: %4.3f", m_cfg.detailSampleMaxError);

    // build all tiles
    m_solid = rcAllocHeightfield();
    if (!m_solid || !rcCreateHeightfield(m_ctx, *m_solid, m_cfg.width, m_cfg.height, m_cfg.bmin, m_cfg.bmax, m_cfg.cs, m_cfg.ch))
    {
        m_ctx->log(RC_LOG_ERROR, "Failed building heightfield!");
        return false;
    }

    // mark all walkable tiles, both liquids and solids
    unsigned char* triFlags = new unsigned char[tTriCount];
    memset(triFlags, NAV_GROUND, tTriCount * sizeof(unsigned char));
    rcClearUnwalkableTriangles(m_ctx, m_cfg.walkableSlopeAngle, tVerts, tVertCount, tTris, tTriCount, triFlags);
    rcRasterizeTriangles(m_ctx, tVerts, tVertCount, tTris, triFlags, tTriCount, *m_solid, m_cfg.walkableClimb);
    delete[] triFlags;

    rcFilterLowHangingWalkableObstacles(m_ctx, m_cfg.walkableClimb, *m_solid);
    rcFilterLedgeSpans(m_ctx, m_cfg.walkableHeight, m_cfg.walkableClimb, *m_solid);
    rcFilterWalkableLowHeightSpans(m_ctx, m_cfg.walkableHeight, *m_solid);

    rcRasterizeTriangles(m_ctx, lVerts, lVertCount, lTris, lTriFlags, lTriCount, *m_solid, m_cfg.walkableClimb);

    // compact heightfield spans
    m_chf = rcAllocCompactHeightfield();
    if (!m_chf || !rcBuildCompactHeightfield(m_ctx, m_cfg.walkableHeight, m_cfg.walkableClimb, *m_solid, *m_chf))
    {
        m_ctx->log(RC_LOG_ERROR, "Failed compacting heightfield!");
        return false;
    }

    // build polymesh intermediates
    if (!rcErodeWalkableArea(m_ctx, m_cfg.walkableRadius, *m_chf))
    {
        m_ctx->log(RC_LOG_ERROR, "Failed eroding area!");
        return false;
    }

    if (m_partitionType == SAMPLE_PARTITION_WATERSHED)
    {
        // Prepare for region partitioning, by calculating distance field along the walkable surface.
        if (!rcBuildDistanceField(m_ctx, *m_chf))
        {
            m_ctx->log(RC_LOG_ERROR, "buildNavigation: Could not build distance field.");
            return false;
        }

        // Partition the walkable surface into simple regions without holes.
        if (!rcBuildRegions(m_ctx, *m_chf, m_cfg.borderSize, m_cfg.minRegionArea, m_cfg.mergeRegionArea))
        {
            m_ctx->log(RC_LOG_ERROR, "buildNavigation: Could not build watershed regions.");
            return false;
        }
    }
    else if (m_partitionType == SAMPLE_PARTITION_MONOTONE)
    {
        // Partition the walkable surface into simple regions without holes.
        // Monotone partitioning does not need distancefield.
        if (!rcBuildRegionsMonotone(m_ctx, *m_chf, m_cfg.borderSize, m_cfg.minRegionArea, m_cfg.mergeRegionArea))
        {
            m_ctx->log(RC_LOG_ERROR, "buildNavigation: Could not build monotone regions.");
            return false;
        }
    }
    else // SAMPLE_PARTITION_LAYERS
    {
        // Partition the walkable surface into simple regions without holes.
        if (!rcBuildLayerRegions(m_ctx, *m_chf, m_cfg.borderSize, m_cfg.minRegionArea))
        {
            m_ctx->log(RC_LOG_ERROR, "buildNavigation: Could not build layer regions.");
            return false;
        }
    }

    if (!rcBuildRegions(m_ctx, *m_chf, m_cfg.borderSize, m_cfg.minRegionArea, m_cfg.mergeRegionArea))
    {
        m_ctx->log(RC_LOG_ERROR, "Failed building regions!");
        return false;
    }

    m_cset = rcAllocContourSet();
    if (!m_cset || !rcBuildContours(m_ctx, *m_chf, m_cfg.maxSimplificationError, m_cfg.maxEdgeLen, *m_cset))
    {
        m_ctx->log(RC_LOG_ERROR, "Failed building contours!");
        return false;
    }

    // build polymesh
    m_pmesh = rcAllocPolyMesh();
    if (!m_pmesh || !rcBuildPolyMesh(m_ctx, *m_cset, m_cfg.maxVertsPerPoly, *m_pmesh))
    {
        m_ctx->log(RC_LOG_ERROR, "Failed building polymesh!");
        return false;
    }

    m_dmesh = rcAllocPolyMeshDetail();
    if (!m_dmesh || !rcBuildPolyMeshDetail(m_ctx, *m_pmesh, *m_chf, m_cfg.detailSampleDist, m_cfg.detailSampleMaxError, *m_dmesh))
    {
        m_ctx->log(RC_LOG_ERROR, "Failed building polymesh detail!");
        return false;
    }

    // free those up
    // we may want to keep them in the future for debug
    // but right now, we don't have the code to merge them
    rcFreeHeightField(m_solid);
    m_solid = NULL;
    rcFreeCompactHeightfield(m_chf);
    m_chf = NULL;
    rcFreeContourSet(m_cset);
    m_cset = NULL;

    // set polygons as walkable
    // TODO: special flags for DYNAMIC polygons, ie surfaces that can be turned on and off
    for (int i = 0; i < m_pmesh->npolys; ++i)
        if (m_pmesh->areas[i] & RC_WALKABLE_AREA)
            m_pmesh->flags[i] = m_pmesh->areas[i];

    // setup mesh parameters
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
    params.detailTriCount = m_dmesh->ntris;

    params.offMeshConVerts = m_geom->getOffMeshConnectionVerts();
    params.offMeshConRad = m_geom->getOffMeshConnectionRads();
    params.offMeshConDir = m_geom->getOffMeshConnectionDirs();
    params.offMeshConAreas = m_geom->getOffMeshConnectionAreas();
    params.offMeshConFlags = m_geom->getOffMeshConnectionFlags();
    params.offMeshConUserID = m_geom->getOffMeshConnectionId();
    params.offMeshConCount = m_geom->getOffMeshConnectionCount();

    params.walkableHeight = m_cellHeight * m_cfg.walkableHeight;  // agent height
    params.walkableRadius = m_cellSize * m_cfg.walkableRadius;  // agent radius
    params.walkableClimb = m_cellHeight * m_cfg.walkableClimb;    // keep less that walkableHeight (aka agent height)!
    / *params.tileX = (((bmin[0] + bmax[0]) / 2) - m_navMesh->getParams()->orig[0]) / GRID_SIZE;
    params.tileY = (((bmin[2] + bmax[2]) / 2) - m_navMesh->getParams()->orig[2]) / GRID_SIZE;* /


    rcVcopy(params.bmin, m_pmesh->bmin);
    rcVcopy(params.bmax, m_pmesh->bmax);
    params.cs = m_cfg.cs;
    params.ch = m_cfg.ch;

    // no use of layer
    params.tileLayer = 0;
    params.buildBvTree = true;

    // will hold final navmesh
    unsigned char* navData = NULL;
    int navDataSize = 0;

        // these values are checked within dtCreateNavMeshData - handle them here
        // so we have a clear error message
        if (params.nvp > DT_VERTS_PER_POLYGON)
        {
            m_ctx->log(RC_LOG_ERROR, "Invalid verts-per-polygon value!");
            return false;
        }
        if (params.vertCount >= 0xffff)
        {
            m_ctx->log(RC_LOG_ERROR, "Too many vertices!");
            return false;
        }
        if (!params.vertCount || !params.verts)
        {
            // occurs mostly when adjacent tiles have models
            // loaded but those models don't span into this tile

            // message is an annoyance
            m_ctx->log(RC_LOG_ERROR, "No vertices to build!");
            return false;
        }
        if (!params.polyCount || !params.polys)
        {
            // we have flat tiles with no actual geometry - don't build those, its useless
            // keep in mind that we do output those into debug info
            // drop tiles with only exact count - some tiles may have geometry while having less tiles
            m_ctx->log(RC_LOG_ERROR, "No polygons to build on tile!");
            return false;
        }
        if (!params.detailMeshes || !params.detailVerts || !params.detailTris)
        {
            m_ctx->log(RC_LOG_ERROR, "No detail mesh to build!");
            return false;
        }

        //printf("%s Building navmesh tile...                \r", tileString);
        if (!dtCreateNavMeshData(&params, &navData, &navDataSize))
        {
            m_ctx->log(RC_LOG_ERROR, "Failed building navmesh!");
            return false;
        }

        m_navMesh = dtAllocNavMesh();
        if (!m_navMesh)
        {
            dtFree(navData);
            m_ctx->log(RC_LOG_ERROR, "Could not create Detour navmesh");
            return false;
        }

        dtStatus status;

        status = m_navMesh->init(navData, navDataSize, DT_TILE_FREE_DATA);
        if (dtStatusFailed(status))
        {
            dtFree(navData);
            m_ctx->log(RC_LOG_ERROR, "Could not init Detour navmesh");
            return false;
        }

        status = m_navQuery->init(m_navMesh, 2048);
        if (dtStatusFailed(status))
        {
            m_ctx->log(RC_LOG_ERROR, "Could not init Detour navmesh query");
            return false;
        }

        m_ctx->stopTimer(RC_TIMER_TOTAL);

        // Show performance stats.
        duLogBuildTimes(*m_ctx, m_ctx->getAccumulatedTime(RC_TIMER_TOTAL));
        m_ctx->log(RC_LOG_PROGRESS, ">> Polymesh: %d vertices  %d polygons", m_pmesh->nverts, m_pmesh->npolys);

        m_totalBuildTimeMs = m_ctx->getAccumulatedTime(RC_TIMER_TOTAL) / 1000.0f;

        if (m_tool)
            m_tool->init(this);
        initToolStates(this);
        m_isBuilded = true;
        */

bool CMaNGOS_Map::LoadTileData(unsigned int tx, unsigned int ty)
{
    bool setGridCoord = m_MapInfos->IsEmpty();

    if (m_MapInfos->LoadTile(tx, ty))
    {
        setTool(new NavMeshTesterTool);
        m_isBuilded = false;
        if (setGridCoord)
        {
            float border = (m_agentRadius + 3) / 2;
            rcVcopy(m_GridBMin, m_MapInfos->BMin());
            m_GridBMin[0] -= RecastDemo::BLOCK_SIZE + border;
            m_GridBMin[2] -= RecastDemo::BLOCK_SIZE + border;
            m_GeomChanged = true;
        }
        return true;
    }

    return false;
}

void CMaNGOS_Map::RemoveTileData(unsigned int tx, unsigned int ty)
{
    m_MapInfos->RemoveTile(tx, ty);

    if (m_tool)
    {
        m_tool->reset();
        m_tool->init(this);
    }

    delete m_SelectedTile;
    m_SelectedTile = NULL;
}

void CMaNGOS_Map::handleRender()
{
    if (m_MapInfos->IsEmpty())
        return;

    DebugDrawGL dd;

    // Draw mesh
    const float texScale = 1.0f / (m_cellSize * 10.0f);

    for (MeshObjectsMap::const_iterator itr = m_MapInfos->GetGeomsMap()->begin(); itr != m_MapInfos->GetGeomsMap()->end(); itr++)
    {
        if (m_drawMode != DRAWMODE_NAVMESH_TRANS)
        {
            MeshInfos const* mmi = itr->second->GetMap();
            MeshInfos const* vmi = itr->second->GetVMap();
            MeshInfos const* gmi = itr->second->GetModel();

            if (m_DrawMapMesh && mmi)
            {
                MeshDetails const* mmd = mmi->GetSolidMesh();
                MeshDetails const* mlmi = mmi->GetLiquidMesh();
                if (mmd)
                {
                    duDebugDrawTriMeshSlope(&dd, mmd->Verts(), mmd->VertCount(), mmd->Tris(), mmd->Normals(), mmd->TrisCount(),
                        m_agentMaxSlope, texScale);
                }
                if (mlmi)
                    duDebugDrawLiquidTriMesh(&dd, mlmi->Verts(), mlmi->VertCount(), mlmi->Tris(), mlmi->Normals(), mlmi->TrisCount(), texScale);
            }

            if (m_DrawVMapMesh && vmi)
            {
                MeshDetails const* vmd = vmi->GetSolidMesh();
                MeshDetails const* vlmi = vmi->GetLiquidMesh();
                if (vmd)
                {
                    duDebugDrawTriMeshSlope(&dd, vmd->Verts(), vmd->VertCount(), vmd->Tris(), vmd->Normals(), vmd->TrisCount(),
                        m_agentMaxSlope, texScale);
                }
                if (vlmi)
                    duDebugDrawLiquidTriMesh(&dd, vlmi->Verts(), vlmi->VertCount(), vlmi->Tris(), vlmi->Normals(), vlmi->TrisCount(), texScale);
            }

            if (m_DrawMapMesh && gmi)
            {
                MeshDetails const* mmd = gmi->GetSolidMesh();
                //MeshDetails const* mlmi = mmi->GetLiquidMesh();
                if (mmd)
                {
                    duDebugDrawTriMeshSlope(&dd, mmd->Verts(), mmd->VertCount(), mmd->Tris(), mmd->Normals(), mmd->TrisCount(),
                        m_agentMaxSlope, texScale);
                }
            }
        }
    }

    glDepthMask(GL_FALSE);

    // Draw bounds
    const float* bmin;
    const float* bmax;
    GetGeomBounds(bmin, bmax);
    duDebugDrawBoxWire(&dd, bmin[0], bmin[1], bmin[2], bmax[0], bmax[1], bmax[2], duRGBA(255, 255, 255, 128), 2.0f);

    // Tiles grid
    duDebugDrawGridXZ(&dd, m_GridBMin[0], m_GridBMin[1], m_GridBMin[2], 3, 3, RecastDemo::BLOCK_SIZE, duRGBA(255, 0, 0, 128), 2.0f);

    // Draw active tile
    if (m_SelectedTile)
    {
        duDebugDrawBoxWire(&dd, m_SelectedTile->bMin[0], m_SelectedTile->bMin[1], m_SelectedTile->bMin[2],
            m_SelectedTile->bMax[0], m_SelectedTile->bMax[1], m_SelectedTile->bMax[2], m_tileCol, 2.0f);
    }

    if (m_navMesh && m_navQuery &&
        (m_drawMode == DRAWMODE_NAVMESH ||
        m_drawMode == DRAWMODE_NAVMESH_TRANS ||
        m_drawMode == DRAWMODE_NAVMESH_BVTREE ||
        m_drawMode == DRAWMODE_NAVMESH_NODES ||
        m_drawMode == DRAWMODE_NAVMESH_PORTALS ||
        m_drawMode == DRAWMODE_NAVMESH_INVIS))
    {
        if (m_drawMode != DRAWMODE_NAVMESH_INVIS)
            duDebugDrawNavMeshWithClosedList(&dd, *m_navMesh, *m_navQuery, m_navMeshDrawFlags);
        if (m_drawMode == DRAWMODE_NAVMESH_BVTREE)
            duDebugDrawNavMeshBVTree(&dd, *m_navMesh);
        if (m_drawMode == DRAWMODE_NAVMESH_PORTALS)
            duDebugDrawNavMeshPortals(&dd, *m_navMesh);
        if (m_drawMode == DRAWMODE_NAVMESH_NODES)
            duDebugDrawNavMeshNodes(&dd, *m_navQuery);
        duDebugDrawNavMeshPolysWithFlags(&dd, *m_navMesh, SAMPLE_POLYFLAGS_DISABLED, duRGBA(0, 0, 0, 128));
    }


    glDepthMask(GL_TRUE);

    /*if (m_chf && m_drawMode == DRAWMODE_COMPACT)
        duDebugDrawCompactHeightfieldSolid(&dd, *m_chf);

    if (m_chf && m_drawMode == DRAWMODE_COMPACT_DISTANCE)
        duDebugDrawCompactHeightfieldDistance(&dd, *m_chf);
    if (m_chf && m_drawMode == DRAWMODE_COMPACT_REGIONS)
        duDebugDrawCompactHeightfieldRegions(&dd, *m_chf);
    if (m_solid && m_drawMode == DRAWMODE_VOXELS)
    {
        glEnable(GL_FOG);
        duDebugDrawHeightfieldSolid(&dd, *m_solid);
        glDisable(GL_FOG);
    }
    if (m_solid && m_drawMode == DRAWMODE_VOXELS_WALKABLE)
    {
        glEnable(GL_FOG);
        duDebugDrawHeightfieldWalkable(&dd, *m_solid);
        glDisable(GL_FOG);
    }

    if (m_cset && m_drawMode == DRAWMODE_RAW_CONTOURS)
    {
        glDepthMask(GL_FALSE);
        duDebugDrawRawContours(&dd, *m_cset);
        glDepthMask(GL_TRUE);
    }

    if (m_cset && m_drawMode == DRAWMODE_BOTH_CONTOURS)
    {
        glDepthMask(GL_FALSE);
        duDebugDrawRawContours(&dd, *m_cset, 0.5f);
        duDebugDrawContours(&dd, *m_cset);
        glDepthMask(GL_TRUE);
    }
    if (m_cset && m_drawMode == DRAWMODE_CONTOURS)
    {
        glDepthMask(GL_FALSE);
        duDebugDrawContours(&dd, *m_cset);
        glDepthMask(GL_TRUE);
    }
    if (m_chf && m_cset && m_drawMode == DRAWMODE_REGION_CONNECTIONS)
    {
        duDebugDrawCompactHeightfieldRegions(&dd, *m_chf);

        glDepthMask(GL_FALSE);
        duDebugDrawRegionConnections(&dd, *m_cset);
        glDepthMask(GL_TRUE);
    }
    if (m_pmesh && m_drawMode == DRAWMODE_POLYMESH)
    {
        glDepthMask(GL_FALSE);
        duDebugDrawPolyMesh(&dd, *m_pmesh);
        glDepthMask(GL_TRUE);
    }
    if (m_dmesh && m_drawMode == DRAWMODE_POLYMESH_DETAIL)
    {
        glDepthMask(GL_FALSE);
        duDebugDrawPolyMeshDetail(&dd, *m_dmesh);
        glDepthMask(GL_TRUE);
    }*/

    //m_geom->drawConvexVolumes(&dd);

    if (m_tool)
        m_tool->handleRender();
    renderToolStates();

    glDepthMask(GL_TRUE);
}

bool CMaNGOS_Map::GeomChanged()
{
    if (m_GeomChanged)
    {
        m_GeomChanged = false;
        return true;
    }

    return false;
}

void CMaNGOS_Map::updateToolStates(const float dt)
{
    for (int i = 0; i < MAX_TOOLS; i++)
    {
        if (m_toolStates[i])
            m_toolStates[i]->handleUpdate(dt);
    }
}

void CMaNGOS_Map::initToolStates()
{
    for (int i = 0; i < MAX_TOOLS; i++)
    {
        if (m_toolStates[i])
            m_toolStates[i]->init(this);
    }
}

void CMaNGOS_Map::resetToolStates()
{
    for (int i = 0; i < MAX_TOOLS; i++)
    {
        if (m_toolStates[i])
            m_toolStates[i]->reset();
    }
}

void CMaNGOS_Map::setTool(SampleTool* tool)
{
    delete m_tool;
    m_tool = tool;
    if (tool)
        m_tool->init(this);
}

void CMaNGOS_Map::renderToolStates()
{
    for (int i = 0; i < MAX_TOOLS; i++)
    {
        if (m_toolStates[i])
            m_toolStates[i]->handleRender();
    }
}

void CMaNGOS_Map::renderOverlayToolStates(double* proj, double* model, int* view)
{
    for (int i = 0; i < MAX_TOOLS; i++)
    {
        if (m_toolStates[i])
            m_toolStates[i]->handleRenderOverlay(proj, model, view);
    }
}

void CMaNGOS_Map::SelectTile(float const* p)
{
    unsigned int tx, ty;
    GetGridCoord(p, ty, tx);
    m_LastTileX = tx;
    m_LastTileY = ty;
    if (m_SelectedTile)
    {
        if (m_SelectedTile->tx == tx && m_SelectedTile->ty == ty)
            return;

        delete m_SelectedTile;
        m_SelectedTile = NULL;
    }
    MeshObjects const* mo = m_MapInfos->GetMeshObjects(tx, ty);
    float const* bmin;
    float const* bmax;
    if (mo)
    {
        bmin = mo->BMin();
        bmax = mo->BMax();
    }
    else
        m_MapInfos->GetNavMeshTileBounds(tx, ty, bmin, bmax);
    m_SelectedTile = new SelectedTile(tx, ty, bmin, bmax);
}

void CMaNGOS_Map::handleClick(const float* s, const float* p, bool shift, bool control)
{
    if (m_tool)
        m_tool->handleClick(s, p, shift);
    if (control)
        SelectTile(p);

    rcVcopy(m_LastClick, p);
}
void CMaNGOS_Map::handleUpdate(const float dt)
{
    if (m_tool)
        m_tool->handleUpdate(dt);
    updateToolStates(dt);
}

bool CMaNGOS_Map::GetGridCoord(float const* pos, unsigned int &tx, unsigned int &ty)
{
    // calculate and store temporary values in double format for having same result as same mySQL calculations

    tx = 32 - pos[0] / RecastDemo::BLOCK_SIZE;
    ty = 32 - pos[2] / RecastDemo::BLOCK_SIZE;
    return false;
}
