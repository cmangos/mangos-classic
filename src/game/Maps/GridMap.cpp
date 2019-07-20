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

#include "Log.h"
#include "Grids/CellImpl.h"
#include "Maps/Map.h"
#include "Server/DBCEnums.h"
#include "Server/DBCStores.h"
#include "Maps/GridMap.h"
#include "VMapFactory.h"
#include "MotionGenerators/MoveMap.h"
#include "World/World.h"
#include "Policies/Singleton.h"
#include "Util.h"

#include <mutex>

char const* MAP_MAGIC         = "MAPS";
char const* MAP_VERSION_MAGIC = "z1.4";
char const* MAP_AREA_MAGIC    = "AREA";
char const* MAP_HEIGHT_MAGIC  = "MHGT";
char const* MAP_LIQUID_MAGIC  = "MLIQ";

static uint16 holetab_h[4] = { 0x1111, 0x2222, 0x4444, 0x8888 };
static uint16 holetab_v[4] = { 0x000F, 0x00F0, 0x0F00, 0xF000 };

GridMap::GridMap(): m_gridIntHeightMultiplier(0)
{
    m_flags = 0;

    // Area data
    m_gridArea = 0;
    m_area_map = nullptr;

    // Height level data
    m_gridHeight = INVALID_HEIGHT_VALUE;
    m_gridGetHeight = &GridMap::getHeightFromFlat;
    m_V9 = nullptr;
    m_V8 = nullptr;
    memset(m_holes, 0, sizeof(m_holes));

    // Liquid data
    m_liquidGlobalEntry = 0;
    m_liquidGlobalFlags = 0;
    m_liquid_offX   = 0;
    m_liquid_offY   = 0;
    m_liquid_width  = 0;
    m_liquid_height = 0;
    m_liquidLevel = INVALID_HEIGHT_VALUE;
    m_liquidFlags = nullptr;
    m_liquidEntry = nullptr;
    m_liquid_map  = nullptr;
    m_fullyLoaded = false;
}

GridMap::~GridMap()
{
    unloadData();
}

bool GridMap::loadData(char const* filename)
{
    // Unload old data if exist
    unloadData();

    GridMapFileHeader header;
    // Not return error if file not found
    FILE* in = fopen(filename, "rb");
    if (!in)
    {
        DEBUG_FILTER_LOG(LOG_FILTER_MAP_LOADING, "Failled to found %s", filename);
        // its a valid error only in case of no vmap files are available too
        return true;
    }

    fread(&header, sizeof(header), 1, in);
    if (header.mapMagic     == *((uint32 const*)(MAP_MAGIC)) &&
            header.versionMagic == *((uint32 const*)(MAP_VERSION_MAGIC)))
    {
        // loadup area data
        if (header.areaMapOffset && !loadAreaData(in, header.areaMapOffset, header.areaMapSize))
        {
            sLog.outError("Error loading map area data\n");
            fclose(in);
            return false;
        }

        // loadup holes data
        if (header.holesOffset && !loadHolesData(in, header.holesOffset, header.holesSize))
        {
            sLog.outError("Error loading map holes data\n");
            fclose(in);
            return false;
        }

        // loadup height data
        if (header.heightMapOffset && !loadHeightData(in, header.heightMapOffset, header.heightMapSize))
        {
            sLog.outError("Error loading map height data\n");
            fclose(in);
            return false;
        }

        // loadup liquid data
        if (header.liquidMapOffset && !loadGridMapLiquidData(in, header.liquidMapOffset, header.liquidMapSize))
        {
            sLog.outError("Error loading map liquids data\n");
            fclose(in);
            return false;
        }

        fclose(in);
        return true;
    }

    sLog.outError("Map file '%s' is non-compatible version (outdated?). Please, create new using ad.exe program.", filename);
    fclose(in);
    return false;
}

void GridMap::unloadData()
{
    delete[] m_area_map;
    delete[] m_V9;
    delete[] m_V8;
    delete[] m_liquidEntry;
    delete[] m_liquidFlags;
    delete[] m_liquid_map;

    m_area_map = nullptr;
    m_V9 = nullptr;
    m_V8 = nullptr;
    m_liquidEntry = nullptr;
    m_liquidFlags = nullptr;
    m_liquid_map  = nullptr;
    m_gridGetHeight = &GridMap::getHeightFromFlat;
}

bool GridMap::loadAreaData(FILE* in, uint32 offset, uint32 /*size*/)
{
    GridMapAreaHeader header;
    fseek(in, offset, SEEK_SET);
    fread(&header, sizeof(header), 1, in);
    if (header.fourcc != *((uint32 const*)(MAP_AREA_MAGIC)))
        return false;

    m_gridArea = header.gridArea;
    if (!(header.flags & MAP_AREA_NO_AREA))
    {
        m_area_map = new uint16 [16 * 16];
        fread(m_area_map, sizeof(uint16), 16 * 16, in);
    }

    return true;
}

bool GridMap::loadHeightData(FILE* in, uint32 offset, uint32 /*size*/)
{
    GridMapHeightHeader header;
    fseek(in, offset, SEEK_SET);
    fread(&header, sizeof(header), 1, in);
    if (header.fourcc != *((uint32 const*)(MAP_HEIGHT_MAGIC)))
        return false;

    m_gridHeight = header.gridHeight;
    if (!(header.flags & MAP_HEIGHT_NO_HEIGHT))
    {
        if ((header.flags & MAP_HEIGHT_AS_INT16))
        {
            m_uint16_V9 = new uint16 [129 * 129];
            m_uint16_V8 = new uint16 [128 * 128];
            fread(m_uint16_V9, sizeof(uint16), 129 * 129, in);
            fread(m_uint16_V8, sizeof(uint16), 128 * 128, in);
            m_gridIntHeightMultiplier = (header.gridMaxHeight - header.gridHeight) / 65535;
            m_gridGetHeight = &GridMap::getHeightFromUint16;
        }
        else if ((header.flags & MAP_HEIGHT_AS_INT8))
        {
            m_uint8_V9 = new uint8 [129 * 129];
            m_uint8_V8 = new uint8 [128 * 128];
            fread(m_uint8_V9, sizeof(uint8), 129 * 129, in);
            fread(m_uint8_V8, sizeof(uint8), 128 * 128, in);
            m_gridIntHeightMultiplier = (header.gridMaxHeight - header.gridHeight) / 255;
            m_gridGetHeight = &GridMap::getHeightFromUint8;
        }
        else
        {
            m_V9 = new float [129 * 129];
            m_V8 = new float [128 * 128];
            fread(m_V9, sizeof(float), 129 * 129, in);
            fread(m_V8, sizeof(float), 128 * 128, in);
            m_gridGetHeight = &GridMap::getHeightFromFloat;
        }
    }
    else
        m_gridGetHeight = &GridMap::getHeightFromFlat;

    return true;
}

bool GridMap::loadHolesData(FILE* in, uint32 offset, uint32 /*size*/)
{
    if (fseek(in, offset, SEEK_SET) != 0)
        return false;

    if (fread(&m_holes, sizeof(m_holes), 1, in) != 1)
        return false;
    return true;
}

bool GridMap::loadGridMapLiquidData(FILE* in, uint32 offset, uint32 /*size*/)
{
    GridMapLiquidHeader header;
    fseek(in, offset, SEEK_SET);
    fread(&header, sizeof(header), 1, in);
    if (header.fourcc != *((uint32 const*)(MAP_LIQUID_MAGIC)))
        return false;

    m_liquidGlobalEntry = header.liquidType;
    m_liquidGlobalFlags = header.liquidFlags;
    m_liquid_offX   = header.offsetX;
    m_liquid_offY   = header.offsetY;
    m_liquid_width  = header.width;
    m_liquid_height = header.height;
    m_liquidLevel   = header.liquidLevel;

    if (!(header.flags & MAP_LIQUID_NO_TYPE))
    {
        m_liquidEntry = new uint16[16 * 16];
        fread(m_liquidEntry, sizeof(uint16), 16 * 16, in);

        m_liquidFlags = new uint8[16 * 16];
        fread(m_liquidFlags, sizeof(uint8), 16 * 16, in);
    }

    if (!(header.flags & MAP_LIQUID_NO_HEIGHT))
    {
        m_liquid_map = new float [m_liquid_width * m_liquid_height];
        fread(m_liquid_map, sizeof(float), m_liquid_width * m_liquid_height, in);
    }

    return true;
}

uint16 GridMap::getArea(float x, float y) const
{
    if (!m_area_map)
        return m_gridArea;

    x = 16 * (32 - x / SIZE_OF_GRIDS);
    y = 16 * (32 - y / SIZE_OF_GRIDS);
    int lx = (int)x & 15;
    int ly = (int)y & 15;
    return m_area_map[lx * 16 + ly];
}

float GridMap::getHeightFromFlat(float /*x*/, float /*y*/) const
{
    return m_gridHeight;
}

bool GridMap::isHole(int row, int col) const
{
    int cellRow = row / 8;     // 8 squares per cell
    int cellCol = col / 8;
    int holeRow = row % 8 / 2;
    int holeCol = (col - (cellCol * 8)) / 2;

    uint16 hole = m_holes[cellRow][cellCol];

    return (hole & holetab_h[holeCol] & holetab_v[holeRow]) != 0;
}

float GridMap::getHeightFromFloat(float x, float y) const
{
    if (!m_V8 || !m_V9)
        return INVALID_HEIGHT_VALUE;

    x = MAP_RESOLUTION * (32 - x / SIZE_OF_GRIDS);
    y = MAP_RESOLUTION * (32 - y / SIZE_OF_GRIDS);

    int x_int = (int)x;
    int y_int = (int)y;
    x -= x_int;
    y -= y_int;
    x_int &= (MAP_RESOLUTION - 1);
    y_int &= (MAP_RESOLUTION - 1);

    if (isHole(x_int, y_int))
        return INVALID_HEIGHT_VALUE;

    // Height stored as: h5 - its v8 grid, h1-h4 - its v9 grid
    // +--------------> X
    // | h1-------h2     Coordinates is:
    // | | \  1  / |     h1 0,0
    // | |  \   /  |     h2 0,1
    // | | 2  h5 3 |     h3 1,0
    // | |  /   \  |     h4 1,1
    // | | /  4  \ |     h5 1/2,1/2
    // | h3-------h4
    // V Y
    // For find height need
    // 1 - detect triangle
    // 2 - solve linear equation from triangle points
    // Calculate coefficients for solve h = a*x + b*y + c

    float a, b, c;
    // Select triangle:
    if (x + y < 1)
    {
        if (x > y)
        {
            // 1 triangle (h1, h2, h5 points)
            float h1 = m_V9[(x_int) * 129 + y_int];
            float h2 = m_V9[(x_int + 1) * 129 + y_int];
            float h5 = 2 * m_V8[x_int * 128 + y_int];
            a = h2 - h1;
            b = h5 - h1 - h2;
            c = h1;
        }
        else
        {
            // 2 triangle (h1, h3, h5 points)
            float h1 = m_V9[x_int * 129 + y_int  ];
            float h3 = m_V9[x_int * 129 + y_int + 1];
            float h5 = 2 * m_V8[x_int * 128 + y_int];
            a = h5 - h1 - h3;
            b = h3 - h1;
            c = h1;
        }
    }
    else
    {
        if (x > y)
        {
            // 3 triangle (h2, h4, h5 points)
            float h2 = m_V9[(x_int + 1) * 129 + y_int  ];
            float h4 = m_V9[(x_int + 1) * 129 + y_int + 1];
            float h5 = 2 * m_V8[x_int * 128 + y_int];
            a = h2 + h4 - h5;
            b = h4 - h2;
            c = h5 - h4;
        }
        else
        {
            // 4 triangle (h3, h4, h5 points)
            float h3 = m_V9[(x_int) * 129 + y_int + 1];
            float h4 = m_V9[(x_int + 1) * 129 + y_int + 1];
            float h5 = 2 * m_V8[x_int * 128 + y_int];
            a = h4 - h3;
            b = h3 + h4 - h5;
            c = h5 - h4;
        }
    }
    // Calculate height
    return a * x + b * y + c;
}

float GridMap::getHeightFromUint8(float x, float y) const
{
    if (!m_uint8_V8 || !m_uint8_V9)
        return m_gridHeight;

    x = MAP_RESOLUTION * (32 - x / SIZE_OF_GRIDS);
    y = MAP_RESOLUTION * (32 - y / SIZE_OF_GRIDS);

    int x_int = (int)x;
    int y_int = (int)y;
    x -= x_int;
    y -= y_int;
    x_int &= (MAP_RESOLUTION - 1);
    y_int &= (MAP_RESOLUTION - 1);

    int32 a, b, c;
    uint8* V9_h1_ptr = &m_uint8_V9[x_int * 128 + x_int + y_int];
    if (x + y < 1)
    {
        if (x > y)
        {
            // 1 triangle (h1, h2, h5 points)
            int32 h1 = V9_h1_ptr[  0];
            int32 h2 = V9_h1_ptr[129];
            int32 h5 = 2 * m_uint8_V8[x_int * 128 + y_int];
            a = h2 - h1;
            b = h5 - h1 - h2;
            c = h1;
        }
        else
        {
            // 2 triangle (h1, h3, h5 points)
            int32 h1 = V9_h1_ptr[0];
            int32 h3 = V9_h1_ptr[1];
            int32 h5 = 2 * m_uint8_V8[x_int * 128 + y_int];
            a = h5 - h1 - h3;
            b = h3 - h1;
            c = h1;
        }
    }
    else
    {
        if (x > y)
        {
            // 3 triangle (h2, h4, h5 points)
            int32 h2 = V9_h1_ptr[129];
            int32 h4 = V9_h1_ptr[130];
            int32 h5 = 2 * m_uint8_V8[x_int * 128 + y_int];
            a = h2 + h4 - h5;
            b = h4 - h2;
            c = h5 - h4;
        }
        else
        {
            // 4 triangle (h3, h4, h5 points)
            int32 h3 = V9_h1_ptr[  1];
            int32 h4 = V9_h1_ptr[130];
            int32 h5 = 2 * m_uint8_V8[x_int * 128 + y_int];
            a = h4 - h3;
            b = h3 + h4 - h5;
            c = h5 - h4;
        }
    }

    // Calculate height
    return (float)((a * x) + (b * y) + c) * m_gridIntHeightMultiplier + m_gridHeight;
}

float GridMap::getHeightFromUint16(float x, float y) const
{
    if (!m_uint16_V8 || !m_uint16_V9)
        return m_gridHeight;

    x = MAP_RESOLUTION * (32 - x / SIZE_OF_GRIDS);
    y = MAP_RESOLUTION * (32 - y / SIZE_OF_GRIDS);

    int x_int = (int)x;
    int y_int = (int)y;
    x -= x_int;
    y -= y_int;
    x_int &= (MAP_RESOLUTION - 1);
    y_int &= (MAP_RESOLUTION - 1);

    int32 a, b, c;
    uint16* V9_h1_ptr = &m_uint16_V9[x_int * 128 + x_int + y_int];
    if (x + y < 1)
    {
        if (x > y)
        {
            // 1 triangle (h1, h2, h5 points)
            int32 h1 = V9_h1_ptr[  0];
            int32 h2 = V9_h1_ptr[129];
            int32 h5 = 2 * m_uint16_V8[x_int * 128 + y_int];
            a = h2 - h1;
            b = h5 - h1 - h2;
            c = h1;
        }
        else
        {
            // 2 triangle (h1, h3, h5 points)
            int32 h1 = V9_h1_ptr[0];
            int32 h3 = V9_h1_ptr[1];
            int32 h5 = 2 * m_uint16_V8[x_int * 128 + y_int];
            a = h5 - h1 - h3;
            b = h3 - h1;
            c = h1;
        }
    }
    else
    {
        if (x > y)
        {
            // 3 triangle (h2, h4, h5 points)
            int32 h2 = V9_h1_ptr[129];
            int32 h4 = V9_h1_ptr[130];
            int32 h5 = 2 * m_uint16_V8[x_int * 128 + y_int];
            a = h2 + h4 - h5;
            b = h4 - h2;
            c = h5 - h4;
        }
        else
        {
            // 4 triangle (h3, h4, h5 points)
            int32 h3 = V9_h1_ptr[  1];
            int32 h4 = V9_h1_ptr[130];
            int32 h5 = 2 * m_uint16_V8[x_int * 128 + y_int];
            a = h4 - h3;
            b = h3 + h4 - h5;
            c = h5 - h4;
        }
    }

    // Calculate height
    return (float)((a * x) + (b * y) + c) * m_gridIntHeightMultiplier + m_gridHeight;
}

float GridMap::getLiquidLevel(float x, float y) const
{
    if (!m_liquid_map)
        return m_liquidLevel;

    x = MAP_RESOLUTION * (32 - x / SIZE_OF_GRIDS);
    y = MAP_RESOLUTION * (32 - y / SIZE_OF_GRIDS);

    int cx_int = ((int)x & (MAP_RESOLUTION - 1)) - m_liquid_offY;
    int cy_int = ((int)y & (MAP_RESOLUTION - 1)) - m_liquid_offX;

    if (cx_int < 0 || cx_int >= m_liquid_height)
        return INVALID_HEIGHT_VALUE;

    if (cy_int < 0 || cy_int >= m_liquid_width)
        return INVALID_HEIGHT_VALUE;

    return m_liquid_map[cx_int * m_liquid_width + cy_int];
}

uint8 GridMap::getTerrainType(float x, float y) const
{
    if (!m_liquidFlags)
        return (uint8)m_liquidGlobalFlags;

    x = 16 * (32 - x / SIZE_OF_GRIDS);
    y = 16 * (32 - y / SIZE_OF_GRIDS);
    int lx = (int)x & 15;
    int ly = (int)y & 15;
    return m_liquidFlags[lx * 16 + ly];
}

// Get water state on map
GridMapLiquidStatus GridMap::getLiquidStatus(float x, float y, float z, uint8 ReqLiquidType, GridMapLiquidData* data)
{
    // Check water type (if no water return)
    if (!m_liquidFlags && !m_liquidGlobalFlags)
        return LIQUID_MAP_NO_WATER;

    // Get cell
    float cx = MAP_RESOLUTION * (32 - x / SIZE_OF_GRIDS);
    float cy = MAP_RESOLUTION * (32 - y / SIZE_OF_GRIDS);

    int x_int = (int)cx & (MAP_RESOLUTION - 1);
    int y_int = (int)cy & (MAP_RESOLUTION - 1);

    // Check water type in cell
    int idx = (x_int >> 3) * 16 + (y_int >> 3);
    uint8 type = m_liquidFlags ? m_liquidFlags[idx] : m_liquidGlobalFlags;
    uint32 entry = m_liquidEntry ? m_liquidEntry[idx] : m_liquidGlobalEntry;
    if (LiquidTypeEntry const* liquidEntry = sLiquidTypeStore.LookupEntry(entry))
    {
        entry = liquidEntry->Id;
        type &= MAP_LIQUID_TYPE_DEEP_WATER;
        uint32 liqTypeIdx = liquidEntry->Type;
        if (entry < 21)
        {
            if (AreaTableEntry const* area = sAreaStore.LookupEntry(getArea(x, y)))
            {
                uint32 overrideLiquid = area->LiquidTypeOverride;
                if (!overrideLiquid && area->zone)
                {
                    area = GetAreaEntryByAreaID(area->zone);
                    if (area)
                        overrideLiquid = area->LiquidTypeOverride;
                }

                if (LiquidTypeEntry const* liq = sLiquidTypeStore.LookupEntry(overrideLiquid))
                {
                    entry = overrideLiquid;
                    liqTypeIdx = liq->Type;
                }
            }
        }

        type |= (1 << liqTypeIdx) | (type & MAP_LIQUID_TYPE_DEEP_WATER);
    }

    if (type == 0)
        return LIQUID_MAP_NO_WATER;

    // Check req liquid type mask
    if (ReqLiquidType && !(ReqLiquidType & type))
        return LIQUID_MAP_NO_WATER;

    // Check water level:
    // Check water height map
    int lx_int = x_int - m_liquid_offY;
    if (lx_int < 0 || lx_int >= m_liquid_height)
        return LIQUID_MAP_NO_WATER;

    int ly_int = y_int - m_liquid_offX;
    if (ly_int < 0 || ly_int >= m_liquid_width)
        return LIQUID_MAP_NO_WATER;

    // Get water level
    float liquid_level = m_liquid_map ? m_liquid_map[lx_int * m_liquid_width + ly_int] : m_liquidLevel;

    // Get ground level (sub 0.2 for fix some errors)
    float ground_level = getHeight(x, y);

    // Check water level and ground level
    if (liquid_level < ground_level || z < ground_level - 2)
        return LIQUID_MAP_NO_WATER;

    // All ok in water -> store data
    if (data)
    {
        data->entry = entry;
        data->type_flags = type;
        data->level = liquid_level;
        data->depth_level = ground_level;
    }

    // For speed check as int values
    int delta = int((liquid_level - z) * 10);

    // Get position delta
    if (delta > 20)                                         // Under water
        return LIQUID_MAP_UNDER_WATER;

    if (delta > 0)                                          // In water
        return LIQUID_MAP_IN_WATER;

    if (delta > -1)                                         // Walk on water
        return LIQUID_MAP_WATER_WALK;
    // Above water
    return LIQUID_MAP_ABOVE_WATER;
}

bool GridMap::ExistMap(uint32 mapid, int gx, int gy)
{
    int len = sWorld.GetDataPath().length() + strlen("maps/%03u%02u%02u.map") + 1;
    char* tmp = new char[len];
    snprintf(tmp, len, (char*)(sWorld.GetDataPath() + "maps/%03u%02u%02u.map").c_str(), mapid, gx, gy);

    FILE* pf = fopen(tmp, "rb");

    if (!pf)
    {
        sLog.outError("Check existing of map file '%s': not exist!", tmp);
        delete[] tmp;
        return false;
    }

    GridMapFileHeader header;
    fread(&header, sizeof(header), 1, pf);
    if (header.mapMagic     != *((uint32 const*)(MAP_MAGIC)) ||
            header.versionMagic != *((uint32 const*)(MAP_VERSION_MAGIC)))
    {
        sLog.outError("Map file '%s' is non-compatible version (outdated?). Please, create new using ad.exe program.", tmp);
        delete[] tmp;
        fclose(pf);                                         // close file before return
        return false;
    }

    delete[] tmp;
    fclose(pf);
    return true;
}

bool GridMap::ExistVMap(uint32 mapid, int gx, int gy)
{
    if (VMAP::IVMapManager* vmgr = VMAP::VMapFactory::createOrGetVMapManager())
    {
        if (vmgr->isMapLoadingEnabled())
        {
            // x and y are swapped !! => fixed now
            bool exists = vmgr->existsMap((sWorld.GetDataPath() + "vmaps").c_str(),  mapid, gx, gy);
            if (!exists)
            {
                std::string name = vmgr->getDirFileName(mapid, gx, gy);
                sLog.outError("VMap file '%s' is missing or point to wrong version vmap file, redo vmaps with latest vmap_assembler.exe program", (sWorld.GetDataPath() + "vmaps/" + name).c_str());
                return false;
            }
        }
    }

    return true;
}

//////////////////////////////////////////////////////////////////////////
TerrainInfo::TerrainInfo(uint32 mapid) : m_mapId(mapid)
{
    for (int k = 0; k < MAX_NUMBER_OF_GRIDS; ++k)
    {
        for (int i = 0; i < MAX_NUMBER_OF_GRIDS; ++i)
        {
            m_GridMaps[i][k] = nullptr;
            m_GridRef[i][k] = 0;
        }
    }

    // clean up GridMap objects every minute
    const uint32 iCleanUpInterval = 60;
    // schedule start randlomly
    const uint32 iRandomStart = urand(20, 40);

    i_timer.SetInterval(iCleanUpInterval * 1000);
    i_timer.SetCurrent(iRandomStart * 1000);
}

TerrainInfo::~TerrainInfo()
{
    for (int k = 0; k < MAX_NUMBER_OF_GRIDS; ++k)
        for (auto& m_GridMap : m_GridMaps)
            delete m_GridMap[k];

    VMAP::VMapFactory::createOrGetVMapManager()->unloadMap(m_mapId);
    MMAP::MMapFactory::createOrGetMMapManager()->unloadMap(m_mapId);
}

GridMap* TerrainInfo::Load(const uint32 x, const uint32 y, bool mapOnly /*= false*/)
{
    MANGOS_ASSERT(x < MAX_NUMBER_OF_GRIDS);
    MANGOS_ASSERT(y < MAX_NUMBER_OF_GRIDS);

    // reference grid as a first step
    RefGrid(x, y);

    // quick check if GridMap already loaded
    GridMap* pMap = m_GridMaps[x][y];
    if (!pMap)
        pMap = LoadMapAndVMap(x, y, mapOnly);

    return pMap;
}

// schedule lazy GridMap object cleanup
void TerrainInfo::Unload(const uint32 x, const uint32 y)
{
    MANGOS_ASSERT(x < MAX_NUMBER_OF_GRIDS);
    MANGOS_ASSERT(y < MAX_NUMBER_OF_GRIDS);

    if (m_GridMaps[x][y])
    {
        // decrease grid reference count...
        if (UnrefGrid(x, y) == 0)
        {
            // TODO: add your additional logic here
        }
    }
}

// call this method only
void TerrainInfo::CleanUpGrids(const uint32 diff)
{
    i_timer.Update(diff);
    if (!i_timer.Passed())
        return;

    for (int y = 0; y < MAX_NUMBER_OF_GRIDS; ++y)
    {
        for (int x = 0; x < MAX_NUMBER_OF_GRIDS; ++x)
        {
            const int16& iRef = m_GridRef[x][y];
            GridMap* pMap = m_GridMaps[x][y];

            // delete those GridMap objects which have refcount = 0
            if (pMap && iRef == 0)
            {
                m_GridMaps[x][y] = nullptr;
                // delete grid data if reference count == 0
                pMap->unloadData();
                delete pMap;

                // unload VMAPS...
                VMAP::VMapFactory::createOrGetVMapManager()->unloadMap(m_mapId, x, y);

                // unload mmap...
                MMAP::MMapFactory::createOrGetMMapManager()->unloadMap(m_mapId, x, y);
            }
        }
    }

    i_timer.Reset();
}

int TerrainInfo::RefGrid(const uint32& x, const uint32& y)
{
    MANGOS_ASSERT(x < MAX_NUMBER_OF_GRIDS);
    MANGOS_ASSERT(y < MAX_NUMBER_OF_GRIDS);

    LOCK_GUARD _lock(m_refMutex);
    return (m_GridRef[x][y] += 1);
}

int TerrainInfo::UnrefGrid(const uint32& x, const uint32& y)
{
    MANGOS_ASSERT(x < MAX_NUMBER_OF_GRIDS);
    MANGOS_ASSERT(y < MAX_NUMBER_OF_GRIDS);

    int16& iRef = m_GridRef[x][y];

    LOCK_GUARD _lock(m_refMutex);
    if (iRef > 0)
        return (iRef -= 1);

    return 0;
}

float TerrainInfo::GetHeightStatic(float x, float y, float z, bool useVmaps/*=true*/, float maxSearchDist/*=DEFAULT_HEIGHT_SEARCH*/) const
{
    float mapHeight = VMAP_INVALID_HEIGHT_VALUE;            // Store Height obtained by maps
    float vmapHeight = VMAP_INVALID_HEIGHT_VALUE;           // Store Height obtained by vmaps (in "corridor" of z (or slightly above z)

    float z2 = z + 2.f;

    // find raw .map surface under Z coordinates (or well-defined above)
    if (GridMap* gmap = const_cast<TerrainInfo*>(this)->GetGrid(x, y))
        mapHeight = gmap->getHeight(x, y);

    if (useVmaps)
    {
        VMAP::IVMapManager* vmgr = VMAP::VMapFactory::createOrGetVMapManager();
        if (vmgr->isHeightCalcEnabled())
        {
            // if mapHeight has been found search vmap height at least until mapHeight point
            // this prevent case when original Z "too high above ground and vmap height search fail"
            // this will not affect most normal cases (no map in instance, or stay at ground at continent)
            if (mapHeight > INVALID_HEIGHT && z2 - mapHeight > maxSearchDist)
                maxSearchDist = z2 - mapHeight + 1.0f;      // 1.0 make sure that we not fail for case when map height near but above for vamp height

            // look from a bit higher pos to find the floor
            vmapHeight = vmgr->getHeight(GetMapId(), x, y, z2, maxSearchDist);

            // if not found in expected range, look for infinity range (case of far above floor, but below terrain-height)
            if (vmapHeight <= INVALID_HEIGHT)
                vmapHeight = vmgr->getHeight(GetMapId(), x, y, z2, 10000.0f);

            // still not found, look near terrain height
            if (vmapHeight <= INVALID_HEIGHT && mapHeight > INVALID_HEIGHT && z2 < mapHeight)
                vmapHeight = vmgr->getHeight(GetMapId(), x, y, mapHeight + 2.0f, DEFAULT_HEIGHT_SEARCH);
        }
    }

    // mapHeight set for any above raw ground Z or <= INVALID_HEIGHT
    // vmapheight set for any under Z value or <= INVALID_HEIGHT
    if (vmapHeight > INVALID_HEIGHT)
    {
        if (mapHeight > INVALID_HEIGHT)
        {
            // we have mapheight and vmapheight and must select more appropriate

            // we are already under the surface or vmap height above map heigt
            if (z < mapHeight || vmapHeight > mapHeight)
                return vmapHeight;
            return mapHeight;
            // better use .map surface height
        }
        else
            return vmapHeight;                              // we have only vmapHeight (if have)
    }

    return mapHeight;
}

inline bool IsOutdoorWMO(uint32 mogpFlags)
{
    return (mogpFlags & 0x8000) != 0;
}

bool TerrainInfo::IsOutdoors(float x, float y, float z) const
{
    uint32 mogpFlags;
    int32 adtId, rootId, groupId;

    // no wmo found? -> outside by default
    if (!GetAreaInfo(x, y, z, mogpFlags, adtId, rootId, groupId))
        return true;

    return IsOutdoorWMO(mogpFlags);
}

bool TerrainInfo::GetAreaInfo(float x, float y, float z, uint32& flags, int32& adtId, int32& rootId, int32& groupId) const
{
    float vmap_z = z;
    VMAP::IVMapManager* vmgr = VMAP::VMapFactory::createOrGetVMapManager();
    if (vmgr->getAreaInfo(GetMapId(), x, y, vmap_z, flags, adtId, rootId, groupId))
    {
        // check if there's terrain between player height and object height
        if (GridMap* gmap = const_cast<TerrainInfo*>(this)->GetGrid(x, y))
        {
            float _mapheight = gmap->getHeight(x, y);
            // z + 2.0f condition taken from GetHeightStatic(), not sure if it's such a great choice...
            if (z + 2.0f > _mapheight &&  _mapheight > vmap_z)
                return false;
        }
        return true;
    }
    return false;
}

uint16 TerrainInfo::GetAreaFlag(float x, float y, float z, bool* isOutdoors) const
{
    uint32 mogpFlags;
    int32 adtId, rootId, groupId;
    WMOAreaTableEntry const* wmoEntry = nullptr;
    AreaTableEntry const* atEntry = nullptr;
    bool haveAreaInfo = false;

    if (GetAreaInfo(x, y, z, mogpFlags, adtId, rootId, groupId))
    {
        haveAreaInfo = true;
        wmoEntry = GetWMOAreaTableEntryByTripple(rootId, adtId, groupId);
        if (wmoEntry)
            atEntry = GetAreaEntryByAreaID(wmoEntry->areaId);
    }

    uint16 areaflag;
    if (atEntry)
        areaflag = atEntry->exploreFlag;
    else
    {
        if (GridMap* gmap = const_cast<TerrainInfo*>(this)->GetGrid(x, y, true))
            areaflag = gmap->getArea(x, y);
        // this used while not all *.map files generated (instances)
        else
            areaflag = GetAreaFlagByMapId(GetMapId());
    }

    if (isOutdoors)
    {
        if (haveAreaInfo)
            *isOutdoors = IsOutdoorWMO(mogpFlags);
        else
            *isOutdoors = true;
    }
    return areaflag;
}

uint8 TerrainInfo::GetTerrainType(float x, float y) const
{
    if (GridMap* gmap = const_cast<TerrainInfo*>(this)->GetGrid(x, y))
        return gmap->getTerrainType(x, y);
    return 0;
}

uint32 TerrainInfo::GetAreaId(float x, float y, float z) const
{
    return TerrainManager::GetAreaIdByAreaFlag(GetAreaFlag(x, y, z), m_mapId);
}

uint32 TerrainInfo::GetZoneId(float x, float y, float z) const
{
    return TerrainManager::GetZoneIdByAreaFlag(GetAreaFlag(x, y, z), m_mapId);
}

void TerrainInfo::GetZoneAndAreaId(uint32& zoneid, uint32& areaid, float x, float y, float z) const
{
    TerrainManager::GetZoneAndAreaIdByAreaFlag(zoneid, areaid, GetAreaFlag(x, y, z), m_mapId);
}

GridMapLiquidStatus TerrainInfo::getLiquidStatus(float x, float y, float z, uint8 ReqLiquidType, GridMapLiquidData* data) const
{
    GridMapLiquidStatus result = LIQUID_MAP_NO_WATER;
    VMAP::IVMapManager* vmgr = VMAP::VMapFactory::createOrGetVMapManager();
    uint32 liquid_type = 0;
    float liquid_level = INVALID_HEIGHT_VALUE;
    float ground_level = GetHeightStatic(x, y, z, true, DEFAULT_WATER_SEARCH);

    if (vmgr->GetLiquidLevel(GetMapId(), x, y, z, ReqLiquidType, liquid_level, ground_level, liquid_type))
    {
        //DEBUG_LOG("getLiquidStatus(): vmap liquid level: %f ground: %f type: %u", liquid_level, ground_level, liquid_type);
        // Check water level and ground level
        if (liquid_level > ground_level && z > ground_level - 2)
        {
            // All ok in water -> store data
            if (data)
            {
                uint32 liquidFlagType = 0;
                if (LiquidTypeEntry const* liq = sLiquidTypeStore.LookupEntry(liquid_type))
                    liquidFlagType = liq->Type;

                if (liquid_type && liquid_type < 21)
                {
                    if (AreaTableEntry const* area = GetAreaEntryByAreaFlagAndMap(GetAreaFlag(x, y, z), GetMapId()))
                    {
                        uint32 overrideLiquid = area->LiquidTypeOverride;
                        if (!overrideLiquid && area->zone)
                        {
                            area = GetAreaEntryByAreaID(area->zone);
                            if (area)
                                overrideLiquid = area->LiquidTypeOverride;
                        }

                        if (LiquidTypeEntry const* liq = sLiquidTypeStore.LookupEntry(overrideLiquid))
                        {
                            liquid_type = overrideLiquid;
                            liquidFlagType = liq->Type;
                        }
                    }
                }

                data->level = liquid_level;
                data->depth_level = ground_level;

                data->entry = liquid_type;
                data->type_flags = 1 << liquidFlagType;
            }

            // For speed check as int values
            int delta = int((liquid_level - z) * 10);

            // Get position delta
            if (delta > 20)                   // Under water
                return LIQUID_MAP_UNDER_WATER;
            if (delta > 0)                    // In water
                return LIQUID_MAP_IN_WATER;
            if (delta > -1)                   // Walk on water
                return LIQUID_MAP_WATER_WALK;
            result = LIQUID_MAP_ABOVE_WATER;
        }
    }
    else if (GridMap* gmap = const_cast<TerrainInfo*>(this)->GetGrid(x, y))
    {
        GridMapLiquidData map_data;
        GridMapLiquidStatus map_result = gmap->getLiquidStatus(x, y, z, ReqLiquidType, &map_data);
        // Not override LIQUID_MAP_ABOVE_WATER with LIQUID_MAP_NO_WATER:
        if (map_result != LIQUID_MAP_NO_WATER && (map_data.level > ground_level))
        {
            if (data)
                *data = map_data;

            return map_result;
        }
    }
    return result;
}

bool TerrainInfo::IsInWater(float x, float y, float z, GridMapLiquidData* data) const
{
    // Check surface in x, y point for liquid
    if (const_cast<TerrainInfo*>(this)->GetGrid(x, y))
    {
        GridMapLiquidData liquid_status;
        GridMapLiquidData* liquid_ptr = data ? data : &liquid_status;
        if (getLiquidStatus(x, y, z, MAP_ALL_LIQUIDS, liquid_ptr))
        {
            // if (liquid_prt->level - liquid_prt->depth_level > 2) //???
            return true;
        }
    }
    return false;
}

// check if creature is in water and have enough space to swim
bool TerrainInfo::IsSwimmable(float x, float y, float z, float radius /*= 1.5f*/, GridMapLiquidData* data /*= 0*/) const
{
    // Check surface in x, y point for liquid
    if (const_cast<TerrainInfo*>(this)->GetGrid(x, y))
    {
        GridMapLiquidData liquid_status;
        GridMapLiquidData* liquid_ptr = data ? data : &liquid_status;
        if (getLiquidStatus(x, y, z, MAP_ALL_LIQUIDS, liquid_ptr))
        {
            if (liquid_ptr->level - liquid_ptr->depth_level > radius) // is unit have enough space to swim
                return true;
        }
    }
    return false;
}

bool TerrainInfo::IsUnderWater(float x, float y, float z) const
{
    if (const_cast<TerrainInfo*>(this)->GetGrid(x, y))
    {
        if (getLiquidStatus(x, y, z, MAP_LIQUID_TYPE_WATER | MAP_LIQUID_TYPE_OCEAN)&LIQUID_MAP_UNDER_WATER)
            return true;
    }
    return false;
}

/**
 * Function find higher form water or ground height for current floor
 *
 * @param x, y, z    Coordinates original point at floor level
 *
 * @param pGround    optional arg for retrun calculated by function work ground height, it let avoid in caller code recalculate height for point if it need
 *
 * @param swim       z coordinate can be calculated for select above/at or under z coordinate (for fly or swim/walking by bottom)
 *                   in last cases for in water returned under water height for avoid client set swimming unit as saty at water.
 *
 * @return           calculated z coordinate
 */
float TerrainInfo::GetWaterOrGroundLevel(float x, float y, float z, float* pGround /*= nullptr*/, bool swim /*= false*/) const
{
    if (const_cast<TerrainInfo*>(this)->GetGrid(x, y))
    {
        // we need ground level (including grid height version) for proper return water level in point
        float ground_z = GetHeightStatic(x, y, z, true, DEFAULT_WATER_SEARCH);
        if (pGround)
            *pGround = ground_z;

        GridMapLiquidData liquid_status;

        GridMapLiquidStatus res = getLiquidStatus(x, y, ground_z, MAP_ALL_LIQUIDS, &liquid_status);
        return res ? (swim ? liquid_status.level - 2.0f : liquid_status.level) : ground_z;
    }

    return VMAP_INVALID_HEIGHT_VALUE;
}

GridMap* TerrainInfo::GetGrid(const float x, const float y, bool loadOnlyMap /*= false*/)
{
    // half opt method
    int gx = (int)(32 - x / SIZE_OF_GRIDS);                 // grid x
    int gy = (int)(32 - y / SIZE_OF_GRIDS);                 // grid y

    // quick check if GridMap already loaded
    GridMap* pMap = m_GridMaps[gx][gy];
    if (!pMap || (!pMap->IsFullyLoaded() && !loadOnlyMap))
        pMap = LoadMapAndVMap(gx, gy, loadOnlyMap);

    return pMap;
}

GridMap* TerrainInfo::LoadMapAndVMap(const uint32 x, const uint32 y, bool mapOnly /*= false*/)
{
    if ((m_GridMaps[x][y] && mapOnly)
        || (VMAP::VMapFactory::createOrGetVMapManager()->IsTileLoaded(m_mapId, x, y) && MMAP::MMapFactory::createOrGetMMapManager()->IsMMapIsLoaded(m_mapId, x, y)))
    {
        // nothing to load here
        return m_GridMaps[x][y];
    }

    {
        LOCK_GUARD lock(m_mutex);
        // double checked lock pattern
        if (!m_GridMaps[x][y])
        {
            GridMap* map = new GridMap();

            // map file name
            int len = sWorld.GetDataPath().length() + strlen("maps/%03u%02u%02u.map") + 1;
            char* tmp = new char[len];
            snprintf(tmp, len, (char*)(sWorld.GetDataPath() + "maps/%03u%02u%02u.map").c_str(), m_mapId, x, y);
            DEBUG_FILTER_LOG(LOG_FILTER_MAP_LOADING, "Loading map %s", tmp);

            if (!map->loadData(tmp))
            {
                sLog.outError("Error load map file: %s", tmp);
                //assert(false);
            }

            delete[] tmp;
            m_GridMaps[x][y] = map;
        }
    }

    // we'll load the rest later
    if (mapOnly)
        return m_GridMaps[x][y];

    if (!VMAP::VMapFactory::createOrGetVMapManager()->IsTileLoaded(m_mapId, x, y))
    {
        // load VMAPs for current map/grid...
        const MapEntry* i_mapEntry = sMapStore.LookupEntry(m_mapId);
        const char* mapName = i_mapEntry ? i_mapEntry->name[sWorld.GetDefaultDbcLocale()] : "UNNAMEDMAP\x0";

        int vmapLoadResult = VMAP::VMapFactory::createOrGetVMapManager()->loadMap((sWorld.GetDataPath() + "vmaps").c_str(), m_mapId, x, y);
        switch (vmapLoadResult)
        {
            case VMAP::VMAP_LOAD_RESULT_OK:
                DEBUG_FILTER_LOG(LOG_FILTER_MAP_LOADING, "VMAP loaded name:%s, id:%d, x:%d, y:%d (vmap rep.: x:%d, y:%d)", mapName, m_mapId, x, y, x, y);
                break;
            case VMAP::VMAP_LOAD_RESULT_ERROR:
                DEBUG_FILTER_LOG(LOG_FILTER_MAP_LOADING, "Could not load VMAP name:%s, id:%d, x:%d, y:%d (vmap rep.: x:%d, y:%d)", mapName, m_mapId, x, y, x, y);
                break;
            case VMAP::VMAP_LOAD_RESULT_IGNORED:
                DEBUG_FILTER_LOG(LOG_FILTER_MAP_LOADING, "Ignored VMAP name:%s, id:%d, x:%d, y:%d (vmap rep.: x:%d, y:%d)", mapName, m_mapId, x, y, x, y);
                break;
        }
    }

    if (!MMAP::MMapFactory::createOrGetMMapManager()->IsMMapIsLoaded(m_mapId, x, y))
    {
        // load navmesh
        MMAP::MMapFactory::createOrGetMMapManager()->loadMap(m_mapId, x, y);
    }

    if (m_GridMaps[x][y])
        m_GridMaps[x][y]->SetFullyLoaded();

    return  m_GridMaps[x][y];
}

float TerrainInfo::GetWaterLevel(float x, float y, float z, float* pGround /*= nullptr*/) const
{
    if (const_cast<TerrainInfo*>(this)->GetGrid(x, y))
    {
        // we need ground level (including grid height version) for proper return water level in point
        float ground_z = GetHeightStatic(x, y, z, true, DEFAULT_WATER_SEARCH);
        if (pGround)
            *pGround = ground_z;

        GridMapLiquidData liquid_status;

        GridMapLiquidStatus res = getLiquidStatus(x, y, ground_z, MAP_ALL_LIQUIDS, &liquid_status);
        if (!res)
            return VMAP_INVALID_HEIGHT_VALUE;

        return liquid_status.level;
    }

    return VMAP_INVALID_HEIGHT_VALUE;
}

//////////////////////////////////////////////////////////////////////////

#define CLASS_LOCK MaNGOS::ClassLevelLockable<TerrainManager, std::mutex>
INSTANTIATE_SINGLETON_2(TerrainManager, CLASS_LOCK);
INSTANTIATE_CLASS_MUTEX(TerrainManager, std::mutex);

TerrainManager::TerrainManager()
{
}

TerrainManager::~TerrainManager()
{
    for (auto& it : i_TerrainMap)
        delete it.second;
}

TerrainInfo* TerrainManager::LoadTerrain(const uint32 mapId)
{
    Guard _guard(*this);

    TerrainDataMap::const_iterator iter = i_TerrainMap.find(mapId);
    if (iter == i_TerrainMap.end())
    {
        TerrainInfo* info = new TerrainInfo(mapId);
        i_TerrainMap[mapId] = info;
        return info;
    }
    return (*iter).second;
}

void TerrainManager::UnloadTerrain(const uint32 mapId)
{
    if (sWorld.getConfig(CONFIG_BOOL_GRID_UNLOAD) == 0)
        return;

    Guard _guard(*this);

    TerrainDataMap::iterator iter = i_TerrainMap.find(mapId);
    if (iter != i_TerrainMap.end())
    {
        TerrainInfo* ptr = (*iter).second;
        // lets check if this object can be actually freed
        if (!ptr->IsReferenced())
        {
            i_TerrainMap.erase(iter);
            delete ptr;
        }
    }
}

void TerrainManager::Update(const uint32 diff)
{
    // global garbage collection for GridMap objects and VMaps
    for (auto& iter : i_TerrainMap)
        iter.second->CleanUpGrids(diff);
}

void TerrainManager::UnloadAll()
{
    for (auto& it : i_TerrainMap)
        delete it.second;

    i_TerrainMap.clear();
}

uint32 TerrainManager::GetAreaIdByAreaFlag(uint16 areaflag, uint32 map_id)
{
    AreaTableEntry const* entry = GetAreaEntryByAreaFlagAndMap(areaflag, map_id);

    if (entry)
        return entry->ID;
    return 0;
}

uint32 TerrainManager::GetZoneIdByAreaFlag(uint16 areaflag, uint32 map_id)
{
    AreaTableEntry const* entry = GetAreaEntryByAreaFlagAndMap(areaflag, map_id);

    if (entry)
        return (entry->zone != 0) ? entry->zone : entry->ID;
    return 0;
}

void TerrainManager::GetZoneAndAreaIdByAreaFlag(uint32& zoneid, uint32& areaid, uint16 areaflag, uint32 map_id)
{
    AreaTableEntry const* entry = GetAreaEntryByAreaFlagAndMap(areaflag, map_id);

    areaid = entry ? entry->ID : 0;
    zoneid = entry ? ((entry->zone != 0) ? entry->zone : entry->ID) : 0;
}
