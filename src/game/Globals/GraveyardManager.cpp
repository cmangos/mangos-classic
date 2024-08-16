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

#include "Globals/GraveyardManager.h"
#include "Globals/ObjectMgr.h"
#include "Server/DBCStores.h"
#include "Server/SQLStorages.h"
#include "Maps/GridMap.h"
#include "Maps/Map.h"
#include "World/World.h"

WorldSafeLocsEntry const* GraveyardManager::GetClosestGraveyardHelper(GraveYardMapBounds bounds, float x, float y, float z, uint32 mapId, Team team) const
{
    // Simulate std. algorithm:
    //   found some graveyard associated to (ghost_zone,ghost_map)
    //
    //   if mapId == graveyard.mapId (ghost in plain zone or city or battleground) and search graveyard at same map
    //     then check faction
    //   if mapId != graveyard.mapId (ghost in instance) and search any graveyard associated
    //     then check faction

    // at corpse map
    bool foundNear = false;
    float distNear = std::numeric_limits<float>::max();
    WorldSafeLocsEntry const* entryNear = nullptr;

    // at entrance map for corpse map
    bool foundEntr = false;
    float distEntr = std::numeric_limits<float>::max();
    WorldSafeLocsEntry const* entryEntr = nullptr;

    // some where other
    WorldSafeLocsEntry const* entryFar = nullptr;

    MapEntry const* mapEntry = sMapStore.LookupEntry(mapId);

    InstanceTemplate const* tempEntry = sObjectMgr.GetInstanceTemplate(mapId);

    for (GraveYardMap::const_iterator itr = bounds.first; itr != bounds.second; ++itr)
    {
        GraveYardData const& data = itr->second;

        // Checked on load
        WorldSafeLocsEntry const* entry = sWorldSafeLocsStore.LookupEntry<WorldSafeLocsEntry>(data.safeLocId);

        // skip enemy faction graveyard
        // team == TEAM_BOTH_ALLOWED case can be at call from .neargrave
        // TEAM_INVALID != team for all teams
        if (data.team != TEAM_BOTH_ALLOWED && data.team != team && team != TEAM_BOTH_ALLOWED)
            continue;

        // find now nearest graveyard at other (continent) map
        if (mapId != entry->map_id)
        {
            MANGOS_ASSERT(tempEntry); // should always exist for this case - if it doesnt, missing instance template - vanilla only issue
            // if find graveyard at different map from where entrance placed (or no entrance data), use any first
            if (!mapEntry ||
                tempEntry->ghost_entrance_map < 0 ||
                uint32(tempEntry->ghost_entrance_map) != entry->map_id ||
                (tempEntry->ghost_entrance_x == 0 && tempEntry->ghost_entrance_y == 0))
            {
                // not have any coordinates for check distance anyway
                entryFar = entry;
                continue;
            }

            // at entrance map calculate distance (2D);
            float dist2 = (entry->x - tempEntry->ghost_entrance_x) * (entry->x - tempEntry->ghost_entrance_x)
                + (entry->y - tempEntry->ghost_entrance_y) * (entry->y - tempEntry->ghost_entrance_y);
            if (foundEntr)
            {
                if (dist2 < distEntr)
                {
                    distEntr = dist2;
                    entryEntr = entry;
                }
            }
            else
            {
                foundEntr = true;
                distEntr = dist2;
                entryEntr = entry;
            }
        }
        // find now nearest graveyard at same map
        else
        {
            float dist2 = (entry->x - x) * (entry->x - x) + (entry->y - y) * (entry->y - y) + (entry->z - z) * (entry->z - z);
            if (foundNear)
            {
                if (dist2 < distNear)
                {
                    distNear = dist2;
                    entryNear = entry;
                }
            }
            else
            {
                foundNear = true;
                distNear = dist2;
                entryNear = entry;
            }
        }
    }

    if (entryNear)
        return entryNear;

    if (entryEntr)
        return entryEntr;

    return entryFar;
}

void GraveyardManager::Init(Map* map)
{
    // TODO: Only load relevant ones for specific map - warning: for example TK needs to have netherstorm
    // For now its likely not that harmful, its not that big
    m_graveyardMap = sWorld.GetGraveyardManager().GetGraveyardMap();
}

WorldSafeLocsEntry const* GraveyardManager::GetClosestGraveYard(float x, float y, float z, uint32 mapId, Team team) const
{
    // Search for the closest linked graveyard by decreasing priority:
    //  - First try linked to the current area id (if we have one)
    //  - Then try linked to the current zone id (if we have one)
    //  - Then try linked to the current map id
    const uint32 zoneId = sTerrainMgr.GetZoneId(mapId, x, y, z);
    const uint32 areaId = sTerrainMgr.GetAreaId(mapId, x, y, z);

    constexpr uint32 defaultGraveyardA = 4;
    constexpr uint32 defaultGraveyardH = 10;

    WorldSafeLocsEntry const* graveyard = nullptr;
    if (areaId != 0)
    {
        auto bounds = m_graveyardMap.equal_range(GraveyardLinkKey(areaId, GRAVEYARD_AREALINK));
        graveyard = GetClosestGraveyardHelper(bounds, x, y, z, mapId, team);
    }

    if (zoneId != 0 && graveyard == nullptr)
    {
        auto bounds = m_graveyardMap.equal_range(GraveyardLinkKey(zoneId, GRAVEYARD_AREALINK));
        graveyard = GetClosestGraveyardHelper(bounds, x, y, z, mapId, team);
    }

    if (graveyard == nullptr)
    {
        auto bounds = m_graveyardMap.equal_range(GraveyardLinkKey(mapId, GRAVEYARD_MAPLINK));
        graveyard = GetClosestGraveyardHelper(bounds, x, y, z, mapId, team);
    }

    if (graveyard == nullptr)
    {
        sLog.outErrorDb("Table `game_graveyard_zone` incomplete: Map %u Zone "
            "%u Area %u Team %u does not have a linked graveyard.",
            mapId, zoneId, areaId, uint32(team));
        graveyard = sWorldSafeLocsStore.LookupEntry<WorldSafeLocsEntry>(team == Team::HORDE ? defaultGraveyardH : defaultGraveyardA);
    }
    return graveyard;
}

/*!
 * Return the Graveyard link data for a given graveyard and location
 * combination.
 *
 * \param id        The id of a graveyard. This is an index to the graveyard in
 *                  sWorldSafeLocsStore which holds data from WorldSafeLocs.dbc
 * \param locKey    A location key as returned by ObjectMgr::GraveyardLinkKey
 */
GraveYardData const* GraveyardManager::FindGraveYardData(GraveYardMap const& map, uint32 id, uint32 locKey)
{
    GraveYardMapBounds bounds = map.equal_range(locKey);

    for (GraveYardMap::const_iterator itr = bounds.first; itr != bounds.second; ++itr)
    {
        if (itr->second.safeLocId == id)
            return &itr->second;
    }

    return nullptr;
}

/*!
 * Turn a location id and a link kind into a location key that indexes the
 * graveyard link map.
 *
 * \param locationId    The id of an area or a map.
 * \param linkKind      The kind of id, either GRAVEYARD_MAPLINK or
 *                      GRAVEYARD_AREALINK
 * \return The location key composed of the given data.
 */
uint32 GraveyardManager::GraveyardLinkKey(uint32 locationId, uint32 linkKind)
{
    return locationId | (linkKind << 31);
}

/*!
 * Adds a given graveyard link to the currently active graveyard links.
 * Optionally add it into the database too.
 *
 * \param id        The id of a graveyard. This is an index to the graveyard in
 *                  sWorldSafeLocsStore which holds data from WorldSafeLocs.dbc
 *
 * \param locId     The id of a location. This is either an area id or a map id
 *                  based on the value of linkKind
 *
 * \param linkKind  The kind of the graveyard link to be added. Kind is either
 *                  GRAVEYARD_MAPLINK or GRAVEYARD_AREALINK
 *
 * \param team      The team that is allowed to use this graveyard link. Can be
 *                  TEAM_BOTH_ALLOWED, HORDE or ALLIANCE.
 *
 * \param inDB      True if the given graveyard link needs to be added to the
 *                  database. False otherwise.
 *
 * \return          Whether or not the link was added (False only if the link
 *                  was already added added).
 */
bool GraveyardManager::AddGraveYardLink(uint32 id, uint32 locId, uint32 linkKind, Team team, bool inDB)
{
    uint32 locKey = GraveyardLinkKey(locId, linkKind);
    if (FindGraveYardData(m_graveyardMap, id, locKey))
        return false;

    GraveYardData data;
    data.safeLocId = id;
    data.team = team;
    m_graveyardMap.insert(GraveYardMap::value_type(locKey, data));

    if (inDB)
        WorldDatabase.PExecuteLog("INSERT INTO game_graveyard_zone "
            "(id, ghost_loc, link_kind, faction) VALUES "
            "('%u', '%u','%u', '%u')", id, locId, linkKind, uint32(team));

    return true;
}

void GraveyardManager::SetGraveYardLinkTeam(uint32 id, uint32 locKey, Team team)
{
    auto bounds = m_graveyardMap.equal_range(locKey);

    for (GraveYardMap::iterator itr = bounds.first; itr != bounds.second; ++itr)
    {
        GraveYardData& data = itr->second;

        // skip not matching safezone id
        if (data.safeLocId != id)
            continue;

        data.team = team;                                   // Validate link
        return;
    }

    if (team == TEAM_INVALID)
        return;

    // No graveyard link found but one was expected. Log it and add one to
    // prevent further errors.
    uint32 locId = locKey & 0x7FFFFFFF;
    uint32 linkKind = locKey & 0x80000000;
    sLog.outErrorDb("ObjectMgr::SetGraveYardLinkTeam called for safeLoc %u, "
        "locKey %u, but no graveyard link for this found in database.", id, locKey);
    AddGraveYardLink(id, locId, linkKind, team);
}

GraveYardData const* GraveyardManager::FindGraveYardData(uint32 id, uint32 zoneId)
{
    return FindGraveYardData(m_graveyardMap, id, zoneId);
}
