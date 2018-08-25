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

#include "WaypointManager.h"
#include "Database/DatabaseEnv.h"
#include "Maps/GridDefines.h"
#include "Policies/Singleton.h"
#include "ProgressBar.h"
#include "Maps/MapManager.h"
#include "Globals/ObjectMgr.h"

INSTANTIATE_SINGLETON_1(WaypointManager);

char const* waypointOriginTables[] =
{
    "",
    "creature_movement",
    "creature_movement_template",
    "DND",
    "creature_movement_reference",
};

char const* waypointKeyColumn[] =
{
    "",
    "id",
    "entry",
    "DND",
    "entry",
};

void WaypointManager::Load()
{
    uint32 total_paths = 0;
    uint32 total_nodes = 0;
    uint32 total_behaviors = 0;

    std::set<uint32> movementScriptSet;

    for (ScriptMapMap::const_iterator itr = sCreatureMovementScripts.second.begin(); itr != sCreatureMovementScripts.second.end(); ++itr)
        movementScriptSet.insert(itr->first);

    // /////////////////////////////////////////////////////
    // creature_movement
    // /////////////////////////////////////////////////////

    QueryResult* result = WorldDatabase.Query("SELECT id, COUNT(point) FROM creature_movement GROUP BY id");

    if (!result)
    {
        BarGoLink bar(1);
        bar.step();
        sLog.outString(">> Loaded 0 paths. DB table `creature_movement` is empty.");
        sLog.outString();
    }
    else
    {
        total_paths = (uint32)result->GetRowCount();

        do                                                  // Count expected amount of nodes
        {
            Field* fields   = result->Fetch();

            // uint32 id    = fields[0].GetUInt32();
            uint32 count    = fields[1].GetUInt32();

            total_nodes += count;
        }
        while (result->NextRow());
        delete result;

        //                                   0   1      2           3           4           5            6         7
        result = WorldDatabase.Query("SELECT id, point, position_x, position_y, position_z, orientation, waittime, script_id FROM creature_movement");

        BarGoLink bar(result->GetRowCount());

        // error after load, we check if creature guid corresponding to the path id has proper MovementType
        std::set<uint32> creatureNoMoveType;

        do
        {
            bar.step();
            Field* fields = result->Fetch();

            uint32 id           = fields[0].GetUInt32();
            uint32 point        = fields[1].GetUInt32();

            const CreatureData* cData = sObjectMgr.GetCreatureData(id);

            if (!cData)
            {
                sLog.outErrorDb("Table creature_movement contain path for creature guid %u, but this creature guid does not exist. Skipping.", id);
                continue;
            }

            if (cData->movementType != WAYPOINT_MOTION_TYPE)
                creatureNoMoveType.insert(id);

            WaypointPath& path  = m_pathMap[id];
            WaypointNode& node  = path[point];

            node.x              = fields[2].GetFloat();
            node.y              = fields[3].GetFloat();
            node.z              = fields[4].GetFloat();
            node.orientation    = fields[5].GetFloat();
            node.delay          = fields[6].GetUInt32();
            node.script_id      = fields[7].GetUInt32();

            // prevent using invalid coordinates
            if (!MaNGOS::IsValidMapCoord(node.x, node.y, node.z, node.orientation))
            {
                QueryResult* result1 = WorldDatabase.PQuery("SELECT id, map FROM creature WHERE guid = '%u'", id);
                if (result1)
                    sLog.outErrorDb("Creature (guidlow %d, entry %d) have invalid coordinates in his waypoint %d (X: %f, Y: %f).",
                                    id, result1->Fetch()[0].GetUInt32(), point, node.x, node.y);
                else
                    sLog.outErrorDb("Waypoint path %d, have invalid coordinates in his waypoint %d (X: %f, Y: %f).",
                                    id, point, node.x, node.y);

                MaNGOS::NormalizeMapCoord(node.x);
                MaNGOS::NormalizeMapCoord(node.y);

                if (result1)
                {
                    node.z = sTerrainMgr.LoadTerrain(result1->Fetch()[1].GetUInt32())->GetHeightStatic(node.x, node.y, node.z);
                    delete result1;
                }

                WorldDatabase.PExecute("UPDATE creature_movement SET position_x = '%f', position_y = '%f', position_z = '%f' WHERE id = '%u' AND point = '%u'", node.x, node.y, node.z, id, point);
            }

            if (node.script_id)
            {
                if (sCreatureMovementScripts.second.find(node.script_id) == sCreatureMovementScripts.second.end())
                {
                    sLog.outErrorDb("Table creature_movement for id %u, point %u have script_id %u that does not exist in `dbscripts_on_creature_movement`, ignoring", id, point, node.script_id);
                    continue;
                }

                movementScriptSet.erase(node.script_id);
            }
        }
        while (result->NextRow());

        if (!creatureNoMoveType.empty())
        {
            for (uint32 itr : creatureNoMoveType)
            {
                const CreatureData* cData = sObjectMgr.GetCreatureData(itr);
                const CreatureInfo* cInfo = ObjectMgr::GetCreatureTemplate(cData->id);

                ERROR_DB_STRICT_LOG("Table creature_movement has waypoint for creature guid %u (entry %u), but MovementType is not WAYPOINT_MOTION_TYPE(2). Make sure that this is actually used in a script!", itr, cData->id);

                if (cInfo->MovementType == WAYPOINT_MOTION_TYPE)
                    sLog.outErrorDb("Table creature_template for this entry(%u) guid(%u) has MovementType WAYPOINT_MOTION_TYPE(2), did you intend to use creature_movement_template ?", cData->id, itr);
            }
        }

        sLog.outString(">> Loaded %u paths, %u nodes and %u behaviors from waypoints", total_paths, total_nodes, total_behaviors);
        sLog.outString();

        delete result;
    }

    // /////////////////////////////////////////////////////
    // creature_movement_template
    // /////////////////////////////////////////////////////

    result = WorldDatabase.Query("SELECT entry, COUNT(point) FROM creature_movement_template GROUP BY entry");

    if (!result)
    {
        BarGoLink bar(1);
        bar.step();
        sLog.outString(">> Loaded 0 path templates. DB table `creature_movement_template` is empty.");
        sLog.outString();
    }
    else
    {
        total_nodes = 0;
        total_behaviors = 0;
        total_paths = (uint32)result->GetRowCount();

        do                                                  // Count expected amount of nodes
        {
            Field* fields = result->Fetch();

            // uint32 entry = fields[0].GetUInt32();
            uint32 count    = fields[1].GetUInt32();

            total_nodes += count;
        }
        while (result->NextRow());
        delete result;

        //                                   0      1       2      3           4           5           6            7
        result = WorldDatabase.Query("SELECT entry, pathId, point, position_x, position_y, position_z, orientation, waittime, script_id FROM creature_movement_template");

        BarGoLink bar(result->GetRowCount());

        do
        {
            bar.step();
            Field* fields = result->Fetch();

            uint32 entry        = fields[0].GetUInt32();
            uint32 pathId       = fields[1].GetUInt32();
            uint32 point        = fields[2].GetUInt32();

            const CreatureInfo* cInfo = ObjectMgr::GetCreatureTemplate(entry);

            if (!cInfo)
            {
                sLog.outErrorDb("Table creature_movement_template references unknown creature template %u. Skipping.", entry);
                continue;
            }

            WaypointPath& path  = m_pathTemplateMap[(entry << 8) + pathId];
            WaypointNode& node  = path[point];

            node.x              = fields[3].GetFloat();
            node.y              = fields[4].GetFloat();
            node.z              = fields[5].GetFloat();
            node.orientation    = fields[6].GetFloat();
            node.delay          = fields[7].GetUInt32();
            node.script_id      = fields[8].GetUInt32();

            // prevent using invalid coordinates
            if (!MaNGOS::IsValidMapCoord(node.x, node.y, node.z, node.orientation))
            {
                sLog.outErrorDb("Table creature_movement_template for entry %u (point %u) are using invalid coordinates position_x: %f, position_y: %f)",
                                entry, point, node.x, node.y);

                MaNGOS::NormalizeMapCoord(node.x);
                MaNGOS::NormalizeMapCoord(node.y);

                sLog.outErrorDb("Table creature_movement_template for entry %u (point %u) are auto corrected to normalized position_x=%f, position_y=%f",
                                entry, point, node.x, node.y);

                WorldDatabase.PExecute("UPDATE creature_movement_template SET position_x = '%f', position_y = '%f' WHERE entry = %u AND point = %u AND pathId = %u", node.x, node.y, entry, point, pathId);
            }

            if (node.script_id)
            {
                if (sCreatureMovementScripts.second.find(node.script_id) == sCreatureMovementScripts.second.end())
                {
                    sLog.outErrorDb("Table creature_movement_template for entry %u, point %u have script_id %u that does not exist in `dbscripts_on_creature_movement`, ignoring", entry, point, node.script_id);
                    continue;
                }

                movementScriptSet.erase(node.script_id);
            }
        }
        while (result->NextRow());

        delete result;

        sLog.outString(">> Loaded %u path templates with %u nodes and %u behaviors from waypoint templates", total_paths, total_nodes, total_behaviors);
        sLog.outString();
    }

    if (!movementScriptSet.empty())
    {
        for (uint32 itr : movementScriptSet)
        sLog.outErrorDb("Table `dbscripts_on_creature_movement` contain unused script, id %u.", itr);
        sLog.outString();
    }
}

/// Insert a node into the storage for external access
bool WaypointManager::AddExternalNode(uint32 entry, int32 pathId, uint32 pointId, float x, float y, float z, float o, uint32 waittime, uint32 scriptId)
{
    if (pathId < 0 || pathId >= 0xFF)
    {
        sLog.outErrorScriptLib("WaypointManager::AddExternalNode: (Npc-Entry %u, PathId %i) Invalid pathId", entry, pathId);
        return false;
    }

    if (!MaNGOS::IsValidMapCoord(x, y, z, o))
    {
        sLog.outErrorScriptLib("WaypointManager::AddExternalNode: (Npc-Entry %u, PathId %i) Invalid coordinates", entry, pathId);
        return false;
    }

    m_externalPathTemplateMap[(entry << 8) + pathId][pointId] = WaypointNode(x, y, z, o, waittime, scriptId);
    return true;
}

/// - Insert at a certain point, if pointId == 0 insert last. In this case pointId will be changed to the id to which the node was added
WaypointNode const* WaypointManager::AddNode(uint32 entry, uint32 dbGuid, uint32 pathId, uint32& pointId, WaypointPathOrigin wpDest, float x, float y, float z, float orientation)
{
    // Support only normal movement tables
    if (wpDest != PATH_FROM_GUID && wpDest != PATH_FROM_ENTRY)
        return nullptr;

    // Prepare information
    char const* const table     = waypointOriginTables[wpDest];
    char const* const key_field = waypointKeyColumn[wpDest];
    WaypointPathMap* wpMap      = getMapForPathType(wpDest);

    WaypointPath& path = (*wpMap)[wpDest == PATH_FROM_GUID ? dbGuid : ((entry << 8) + pathId)];

    if (pointId == 0 && !path.empty())                      // Start with highest waypoint
        pointId = path.rbegin()->first + 1;
    else if (pointId == 0)
        pointId = 1;

    uint32 nextPoint = pointId;
    WaypointNode temp = WaypointNode(x, y, z, 100, 0, 0);
    WaypointPath::iterator find = path.find(nextPoint);
    if (find != path.end())                                 // Point already exists
    {
        do                                                  // Move points along until a free spot is found
        {
            std::swap(temp, find->second);
            ++find;
            ++nextPoint;
        }
        while (find != path.end() && find->first == nextPoint);
        // After this, we have:
        // pointId, pointId+1, ..., nextPoint [ Can be == path.end ]]
    }

    // Insert new or remaining
    path[nextPoint] = temp;

    uint32 key = wpDest == PATH_FROM_GUID ? dbGuid : entry;

    // Update original waypoints
    for (WaypointPath::reverse_iterator rItr = path.rbegin(); rItr != path.rend() && rItr->first > pointId; ++rItr)
    {
        if (rItr->first <= nextPoint)
        {
            if (wpDest == PATH_FROM_ENTRY)
                WorldDatabase.PExecuteLog("UPDATE %s SET point=point+1 WHERE %s=%u AND point=%u AND pathId=%u", table, key_field, key, rItr->first - 1, pathId);
            else
                WorldDatabase.PExecuteLog("UPDATE %s SET point=point+1 WHERE %s=%u AND point=%u", table, key_field, key, rItr->first - 1);
        }
    }
    // Insert new Point to database
    if (wpDest == PATH_FROM_ENTRY)
        WorldDatabase.PExecuteLog("INSERT INTO %s (%s,pathId,point,position_x,position_y,position_z,orientation) VALUES (%u,%u,%u, %f,%f,%f, %f)", table, key_field, pathId, key, pointId, x, y, z, orientation);
    else
        WorldDatabase.PExecuteLog("INSERT INTO %s (%s,point,position_x,position_y,position_z,orientation) VALUES (%u,%u, %f,%f,%f, %f)", table, key_field, key, pointId, x, y, z, orientation);

    return &path[pointId];
}

void WaypointManager::DeleteNode(uint32 entry, uint32 dbGuid, uint32 point, uint32 pathId, WaypointPathOrigin wpOrigin)
{
    // Support only normal movement tables
    if (wpOrigin == PATH_FROM_EXTERNAL)
        return;

    WaypointPath* path = GetPathFromOrigin(entry, dbGuid, pathId, wpOrigin);
    if (!path)
        return;

    char const* const table     = waypointOriginTables[wpOrigin];
    char const* const key_field = waypointKeyColumn[wpOrigin];
    uint32 const key            = wpOrigin == PATH_FROM_GUID ? dbGuid : entry;
    if (wpOrigin == PATH_FROM_ENTRY)
        WorldDatabase.PExecuteLog("DELETE FROM %s WHERE %s=%u AND point=%u AND pathId=%u", table, key_field, key, point, pathId);
    else
        WorldDatabase.PExecuteLog("DELETE FROM %s WHERE %s=%u AND point=%u", table, key_field, key, point);

    path->erase(point);
}

void WaypointManager::DeletePath(uint32 id)
{
    WorldDatabase.PExecuteLog("DELETE FROM creature_movement WHERE id=%u", id);
    WaypointPathMap::iterator itr = m_pathMap.find(id);
    if (itr != m_pathMap.end())
        itr->second.clear();
    // the path is not removed from the map, just cleared
    // WMGs have pointers to the path, so deleting them would crash
    // this wastes some memory, but these functions are
    // only meant to be called by GM commands
}

void WaypointManager::SetNodePosition(uint32 entry, uint32 dbGuid, uint32 point, uint32 pathId, WaypointPathOrigin wpOrigin, float x, float y, float z)
{
    // Support only normal movement tables
    if (wpOrigin == PATH_FROM_EXTERNAL)
        return;

    WaypointPath* path = GetPathFromOrigin(entry, dbGuid, pathId, wpOrigin);
    if (!path)
        return;

    char const* const table     = waypointOriginTables[wpOrigin];
    char const* const key_field = waypointKeyColumn[wpOrigin];
    uint32 const key            = wpOrigin == PATH_FROM_GUID ? dbGuid : entry;
    if (wpOrigin == PATH_FROM_ENTRY)
        WorldDatabase.PExecuteLog("UPDATE %s SET position_x=%f, position_y=%f, position_z=%f WHERE %s=%u AND point=%u AND pathId=%u", table, x, y, z, key_field, key, point, pathId);
    else
        WorldDatabase.PExecuteLog("UPDATE %s SET position_x=%f, position_y=%f, position_z=%f WHERE %s=%u AND point=%u", table, x, y, z, key_field, key, point);

    WaypointPath::iterator find = path->find(point);
    if (find != path->end())
    {
        find->second.x = x;
        find->second.y = y;
        find->second.z = z;
    }
}

void WaypointManager::SetNodeWaittime(uint32 entry, uint32 dbGuid, uint32 point, uint32 pathId, WaypointPathOrigin wpOrigin, uint32 waittime)
{
    // Support only normal movement tables
    if (wpOrigin == PATH_FROM_EXTERNAL)
        return;

    WaypointPath* path = GetPathFromOrigin(entry, dbGuid, pathId, wpOrigin);
    if (!path)
        return;

    char const* const table     = waypointOriginTables[wpOrigin];
    char const* const key_field = waypointKeyColumn[wpOrigin];
    uint32 const key            = wpOrigin == PATH_FROM_GUID ? dbGuid : entry;
    if (wpOrigin == PATH_FROM_ENTRY)
        WorldDatabase.PExecuteLog("UPDATE %s SET waittime=%u WHERE %s=%u AND point=%u AND pathId=%u", table, waittime, key_field, key, point, pathId);
    else
        WorldDatabase.PExecuteLog("UPDATE %s SET waittime=%u WHERE %s=%u AND point=%u", table, waittime, key_field, key, point);

    WaypointPath::iterator find = path->find(point);
    if (find != path->end())
        find->second.delay = waittime;
}

void WaypointManager::SetNodeOrientation(uint32 entry, uint32 dbGuid, uint32 point, uint32 pathId, WaypointPathOrigin wpOrigin, float orientation)
{
    // Support only normal movement tables
    if (wpOrigin == PATH_FROM_EXTERNAL)
        return;

    WaypointPath* path = GetPathFromOrigin(entry, dbGuid, pathId, wpOrigin);
    if (!path)
        return;

    char const* const table     = waypointOriginTables[wpOrigin];
    char const* const key_field = waypointKeyColumn[wpOrigin];
    uint32 const key            = wpOrigin == PATH_FROM_GUID ? dbGuid : entry;
    if (wpOrigin == PATH_FROM_ENTRY)
        WorldDatabase.PExecuteLog("UPDATE %s SET orientation=%f WHERE %s=%u AND point=%u AND pathId=%u", table, orientation, key_field, key, point, pathId);
    else
        WorldDatabase.PExecuteLog("UPDATE %s SET orientation=%f WHERE %s=%u AND point=%u", table, orientation, key_field, key, point);

    WaypointPath::iterator find = path->find(point);
    if (find != path->end())
        find->second.orientation = orientation;
}

/// return true if a valid scriptId is provided
bool WaypointManager::SetNodeScriptId(uint32 entry, uint32 dbGuid, uint32 point, uint32 pathId, WaypointPathOrigin wpOrigin, uint32 scriptId)
{
    // Support only normal movement tables
    if (wpOrigin == PATH_FROM_EXTERNAL)
        return false;

    WaypointPath* path = GetPathFromOrigin(entry, dbGuid, pathId, wpOrigin);
    if (!path)
        return false;

    char const* const table     = waypointOriginTables[wpOrigin];
    char const* const key_field = waypointKeyColumn[wpOrigin];
    uint32 const key            = wpOrigin == PATH_FROM_GUID ? dbGuid : ((entry << 8) + pathId);
    if (wpOrigin == PATH_FROM_ENTRY)
        WorldDatabase.PExecuteLog("UPDATE %s SET script_id=%u WHERE %s=%u AND point=%u AND pathId=%u", table, scriptId, key_field, key, point, pathId);
    else
        WorldDatabase.PExecuteLog("UPDATE %s SET script_id=%u WHERE %s=%u AND point=%u", table, scriptId, key_field, key, point);

    WaypointPath::iterator find = path->find(point);
    if (find != path->end())
        find->second.script_id = scriptId;

    return sCreatureMovementScripts.second.find(scriptId) != sCreatureMovementScripts.second.end();
}
