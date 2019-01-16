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

#ifndef RECASTCMANGOSMAP_H
#define RECASTCMANGOSMAP_H

#include <set>
#include <vector>
#include "Tools.h"
#include "MapData.h"
#include "DetourNavMesh.h"
#include "Recast.h"
#include "ChunkyTriMesh.h"
#include "Filelist.h"
#include "G3D\Set.h"
#include "RecastDemoSharedDefines.h"

#ifdef WIN32
#	define snprintf _snprintf
#	define putenv _putenv
#endif

#define MAX_TILE_TO_LOAD 9

enum ShowLevels
{
    SHOW_LEVEL_NONE,
    SHOW_LEVEL_MAP,
    SHOW_LEVEL_NEIGHBOR_TILES,
    SHOW_LEVEL_TILES
};

struct SelectedTile
{
    SelectedTile(unsigned int x, unsigned int y, float const* bmin, float const* bmax) :
        tx(x), ty(y)
    {
        rcVcopy(bMin, bmin);
        rcVcopy(bMax, bmax);
    }

    unsigned int tx;
    unsigned int ty;
    float bMin[3];
    float bMax[3];
};

class CMaNGOS_Map
{
protected:
	enum DrawMode
	{
		DRAWMODE_NAVMESH,
		DRAWMODE_NAVMESH_TRANS,
		DRAWMODE_NAVMESH_BVTREE,
		DRAWMODE_NAVMESH_NODES,
		DRAWMODE_NAVMESH_PORTALS,
		DRAWMODE_NAVMESH_INVIS,
		DRAWMODE_MESH,
		DRAWMODE_VOXELS,
		DRAWMODE_VOXELS_WALKABLE,
		DRAWMODE_COMPACT,
		DRAWMODE_COMPACT_DISTANCE,
		DRAWMODE_COMPACT_REGIONS,
		DRAWMODE_REGION_CONNECTIONS,
		DRAWMODE_RAW_CONTOURS,
		DRAWMODE_BOTH_CONTOURS,
		DRAWMODE_CONTOURS,
		DRAWMODE_POLYMESH,
		DRAWMODE_POLYMESH_DETAIL,
		MAX_DRAWMODE
	};

	DrawMode m_drawMode;
    bool m_DrawMapMesh;
    bool m_DrawVMapMesh;

	int m_maxTiles;
	int m_maxPolysPerTile;
	float m_tileSize;

	unsigned int m_tileCol;
	float m_tileBmin[3];
	float m_tileBmax[3];
	float m_tileBuildTime;
	float m_tileMemUsage;
	int m_tileTriCount;

    char m_tileFileName[256];
    MapInfos* m_MapInfos;

    char m_searchMask[8];
    bool m_isBuilded;
    bool m_bigBaseUnit;

    int m_mapID;
    unsigned int m_tileX, m_tileY;
    bool m_showLevelOld;
    ShowLevels m_showLevel;
    float m_LastClick[3];
    unsigned int m_LastTileX;
    unsigned int m_LastTileY;
    SelectedTile* m_SelectedTile;
    float m_GridBMin[3];

    FileList m_MapFiles;
    FileList m_VMapFiles;
    FileList m_MMapFiles;
    std::set <uint32> m_TilesFound;
    std::set <uint32> m_MapsFound;
    std::set <uint32> m_NeighborTiles;
    string m_TileButtonStr;
    bool m_GeomChanged;

    BuildContext* m_ctx;
    class dtNavMesh* m_navMesh;
    class dtNavMeshQuery* m_navQuery;
    unsigned char m_navMeshDrawFlags;
    float m_cellSize;
    float m_cellHeight;
    float m_agentHeight;
    float m_agentRadius;
    float m_agentMaxClimb;
    float m_agentMaxSlope;
    float m_regionMinSize;
    float m_regionMergeSize;
    float m_edgeMaxLen;
    float m_edgeMaxError;
    float m_vertsPerPoly;
    float m_detailSampleDist;
    float m_detailSampleMaxError;
    SamplePartitionType m_partitionType;
    SampleTool* m_tool;
    SampleToolState* m_toolStates[MAX_TOOLS];

    void initToolStates();
    void resetToolStates();
    void renderToolStates();
    void setTool(SampleTool* tool);
    void renderOverlayToolStates(double* proj, double* model, int* view);
public:
	CMaNGOS_Map();
	virtual ~CMaNGOS_Map();

	void handleSettings();
	void handleTools();
	void handleDebugMode();
	void handleRender();
	void handleRenderOverlay(double* proj, double* model, int* view);
    bool ShowLevel(int height, int width);


    void setContext(BuildContext* ctx) { m_ctx = ctx; }
    void handleClick(const float* s, const float* p, bool shift, bool control);
    void handleUpdate(const float dt);
    void updateToolStates(const float dt);
    class dtNavMesh* getNavMesh() { return m_navMesh; }
    class dtNavMeshQuery* getNavMeshQuery() { return m_navQuery; }
    //class dtCrowd* getCrowd() { return m_crowd; }
    float getAgentRadius() { return m_agentRadius; }
    float getAgentHeight() { return m_agentHeight; }
    float getAgentClimb() { return m_agentMaxClimb; }

    void resetCommonSettings();
    bool buildMoveMapTile();
    void handleExtraSettings();
    void ClearAllGeoms();
    void scanFoldersForTiles();
    bool ShowMapLevel(int height, int width);
    bool ShowNeighborTiles(int height, int width);
    bool ShowTilesLevel(int height, int width);
    bool LoadTileData(unsigned int tx, unsigned int ty);
    bool GeomChanged();
    inline void GetGeomBounds(float const* &bmin, float const* &bmax) { bmin = m_MapInfos->BMin(); bmax = m_MapInfos->BMax(); }
    void ScanFoldersForMaps();
    void Init();
    int GetMapId() const { return m_mapID; };

    inline MapInfos const* GetMapInfos() const { return m_MapInfos; }
    void RemoveTileData(unsigned int tx, unsigned int ty);
    bool GetGridCoord(float const* pos, unsigned int &tx, unsigned int &ty);
    void SelectTile(float const* p);
};


#endif // RECASTCMANGOSMAP_H
