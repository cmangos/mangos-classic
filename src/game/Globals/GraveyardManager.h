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

#ifndef MANGOS_GRAVEYARD_MANAGER_H
#define MANGOS_GRAVEYARD_MANAGER_H

#include "Common.h"
#include "Globals/SharedDefines.h"

struct WorldSafeLocsEntry
{
    uint32  ID;
    uint32  map_id;
    float   x;
    float   y;
    float   z;
    float   o;
    char*   name;
};

struct GraveYardData
{
    uint32 safeLocId;
    Team team;
};
#define GRAVEYARD_AREALINK  0
#define GRAVEYARD_MAPLINK   1
typedef std::multimap < uint32 /*locId*/, GraveYardData > GraveYardMap;
typedef std::pair<GraveYardMap::const_iterator, GraveYardMap::const_iterator> GraveYardMapBounds;

class Map;

class GraveyardManager
{
    public:
        void Init(Map* map);

        WorldSafeLocsEntry const* GetClosestGraveYard(float x, float y, float z, uint32 mapId, Team team) const;
        bool AddGraveYardLink(uint32 id, uint32 locId, uint32 linkKind, Team team, bool inDB = true);
        void SetGraveYardLinkTeam(uint32 id, uint32 linkKey, Team team);
        GraveYardData const* FindGraveYardData(uint32 id, uint32 zoneId);
        static GraveYardData const* FindGraveYardData(GraveYardMap const& map, uint32 id, uint32 zoneId);

        static uint32 GraveyardLinkKey(uint32 locId, uint32 linkKind);

        // Only for use in Map
        GraveYardMap& GetGraveyardMap() { return m_graveyardMap; }
    private:
        WorldSafeLocsEntry const* GetClosestGraveyardHelper(GraveYardMapBounds bounds, float x, float y, float z, uint32 mapId, Team team) const;

        GraveYardMap m_graveyardMap;
};

#endif