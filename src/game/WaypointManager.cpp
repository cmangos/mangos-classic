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
#include "GridDefines.h"
#include "Policies/Singleton.h"
#include "ProgressBar.h"
#include "MapManager.h"
#include "ObjectMgr.h"
#include "ScriptMgr.h"

INSTANTIATE_SINGLETON_1(WaypointManager);

bool WaypointBehavior::isEmpty()
{
    if (emote || spell || model1 || model2)
        return false;

    for (int i = 0; i < MAX_WAYPOINT_TEXT; ++i)
        if (textid[i])
            return false;

    return true;
}

WaypointBehavior::WaypointBehavior(const WaypointBehavior& b)
{
    emote = b.emote;
    spell = b.spell;
    model1 = b.model1;
    model2 = b.model2;
    for (int i = 0; i < MAX_WAYPOINT_TEXT; ++i)
        textid[i] = b.textid[i];
}

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

        //                                   0   1      2           3           4           5         6
        result = WorldDatabase.Query("SELECT id, point, position_x, position_y, position_z, waittime, script_id,"
                                     //   7        8        9        10       11       12     13     14           15      16
                                     "textid1, textid2, textid3, textid4, textid5, emote, spell, orientation, model1, model2 FROM creature_movement");

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
            node.orientation    = fields[14].GetFloat();
            node.delay          = fields[5].GetUInt32();
            node.script_id      = fields[6].GetUInt32();

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

            // WaypointBehavior can be dropped in time. Script_id added may 2010 and can handle all the below behavior.

            WaypointBehavior be;
            be.model1           = fields[15].GetUInt32();
            be.model2           = fields[16].GetUInt32();
            be.emote            = fields[12].GetUInt32();
            be.spell            = fields[13].GetUInt32();

            for (int i = 0; i < MAX_WAYPOINT_TEXT; ++i)
            {
                be.textid[i]    = fields[7 + i].GetInt32();

                if (be.textid[i])
                {
                    if (be.textid[i] < MIN_DB_SCRIPT_STRING_ID || be.textid[i] >= MAX_DB_SCRIPT_STRING_ID)
                    {
                        sLog.outErrorDb("Table `creature_movement` Id %u, point %u has textid%u has value %d out of range. Must be in %u-%u", id, point, i + 1, be.textid[i], MIN_DB_SCRIPT_STRING_ID, MAX_DB_SCRIPT_STRING_ID - 1);
                        be.textid[i] = 0;
                    }
                }
            }

            if (be.spell && ! sSpellStore.LookupEntry(be.spell))
            {
                sLog.outErrorDb("Table creature_movement references unknown spellid %u. Skipping id %u with point %u.", be.spell, id, point);
                be.spell = 0;
            }

            if (be.emote)
            {
                if (!sEmotesStore.LookupEntry(be.emote))
                    sLog.outErrorDb("Waypoint path %u (Point %u) are using emote %u, but emote does not exist.", id, point, be.emote);
            }

            // save memory by not storing empty behaviors
            if (!be.isEmpty())
            {
                node.behavior = new WaypointBehavior(be);
                ++total_behaviors;
            }
            else
                node.behavior = nullptr;
        }
        while (result->NextRow());

        if (!creatureNoMoveType.empty())
        {
            for (std::set<uint32>::const_iterator itr = creatureNoMoveType.begin(); itr != creatureNoMoveType.end(); ++itr)
            {
                const CreatureData* cData = sObjectMgr.GetCreatureData(*itr);
                const CreatureInfo* cInfo = ObjectMgr::GetCreatureTemplate(cData->id);

                ERROR_DB_STRICT_LOG("Table creature_movement has waypoint for creature guid %u (entry %u), but MovementType is not WAYPOINT_MOTION_TYPE(2). Make sure that this is actually used in a script!", *itr, cData->id);

                if (cInfo->MovementType == WAYPOINT_MOTION_TYPE)
                    sLog.outErrorDb("    creature_template for this entry has MovementType WAYPOINT_MOTION_TYPE(2), did you intend to use creature_movement_template ?");
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

        //                                   0      1      2           3           4           5         6
        result = WorldDatabase.Query("SELECT entry, point, position_x, position_y, position_z, waittime, script_id,"
                                     //   7        8        9        10       11       12     13     14           15      16
                                     "textid1, textid2, textid3, textid4, textid5, emote, spell, orientation, model1, model2 FROM creature_movement_template");

        BarGoLink bar(result->GetRowCount());

        do
        {
            bar.step();
            Field* fields = result->Fetch();

            uint32 entry        = fields[0].GetUInt32();
            uint32 point        = fields[1].GetUInt32();

            const CreatureInfo* cInfo = ObjectMgr::GetCreatureTemplate(entry);

            if (!cInfo)
            {
                sLog.outErrorDb("Table creature_movement_template references unknown creature template %u. Skipping.", entry);
                continue;
            }

            WaypointPath& path  = m_pathTemplateMap[entry << 8];
            WaypointNode& node  = path[point];

            node.x              = fields[2].GetFloat();
            node.y              = fields[3].GetFloat();
            node.z              = fields[4].GetFloat();
            node.orientation    = fields[14].GetFloat();
            node.delay          = fields[5].GetUInt32();
            node.script_id      = fields[6].GetUInt32();

            // prevent using invalid coordinates
            if (!MaNGOS::IsValidMapCoord(node.x, node.y, node.z, node.orientation))
            {
                sLog.outErrorDb("Table creature_movement_template for entry %u (point %u) are using invalid coordinates position_x: %f, position_y: %f)",
                                entry, point, node.x, node.y);

                MaNGOS::NormalizeMapCoord(node.x);
                MaNGOS::NormalizeMapCoord(node.y);

                sLog.outErrorDb("Table creature_movement_template for entry %u (point %u) are auto corrected to normalized position_x=%f, position_y=%f",
                                entry, point, node.x, node.y);

                WorldDatabase.PExecute("UPDATE creature_movement_template SET position_x = '%f', position_y = '%f' WHERE entry = %u AND point = %u", node.x, node.y, entry, point);
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

            WaypointBehavior be;
            be.model1           = fields[15].GetUInt32();
            be.model2           = fields[16].GetUInt32();
            be.emote            = fields[12].GetUInt32();
            be.spell            = fields[13].GetUInt32();

            for (int i = 0; i < MAX_WAYPOINT_TEXT; ++i)
            {
                be.textid[i]    = fields[7 + i].GetUInt32();

                if (be.textid[i])
                {
                    if (be.textid[i] < MIN_DB_SCRIPT_STRING_ID || be.textid[i] >= MAX_DB_SCRIPT_STRING_ID)
                    {
                        sLog.outErrorDb("Table `creature_movement_template` Entry %u, point %u has textid%u has value %d out of range. Must be in %u-%u", entry, point, i + 1, be.textid[i], MIN_DB_SCRIPT_STRING_ID, MAX_DB_SCRIPT_STRING_ID - 1);
                        be.textid[i] = 0;
                    }
                }
            }

            if (be.spell && ! sSpellStore.LookupEntry(be.spell))
            {
                sLog.outErrorDb("Table creature_movement_template references unknown spellid %u. Skipping id %u with point %u.", be.spell, entry, point);
                be.spell = 0;
            }

            if (be.emote)
            {
                if (!sEmotesStore.LookupEntry(be.emote))
                    sLog.outErrorDb("Waypoint template path %u (point %u) are using emote %u, but emote does not exist.", entry, point, be.emote);
            }

            // save memory by not storing empty behaviors
            if (!be.isEmpty())
            {
                node.behavior   = new WaypointBehavior(be);
                ++total_behaviors;
            }
            else
                node.behavior   = nullptr;
        }
        while (result->NextRow());

        delete result;

        sLog.outString(">> Loaded %u path templates with %u nodes and %u behaviors from waypoint templates", total_paths, total_nodes, total_behaviors);
        sLog.outString();
    }

    if (!movementScriptSet.empty())
    {
        for (std::set<uint32>::const_iterator itr = movementScriptSet.begin(); itr != movementScriptSet.end(); ++itr)
            sLog.outErrorDb("Table `dbscripts_on_creature_movement` contain unused script, id %u.", *itr);
        sLog.outString();
    }
}

void WaypointManager::Unload()
{
    for (WaypointPathMap::iterator itr = m_pathMap.begin(); itr != m_pathMap.end(); ++itr)
        _clearPath(itr->second);
    m_pathMap.clear();

    for (WaypointPathMap::iterator itr = m_pathTemplateMap.begin(); itr != m_pathTemplateMap.end(); ++itr)
        _clearPath(itr->second);
    m_pathTemplateMap.clear();

    for (WaypointPathMap::iterator itr = m_externalPathTemplateMap.begin(); itr != m_externalPathTemplateMap.end(); ++itr)
        _clearPath(itr->second);
    m_externalPathTemplateMap.clear();

}

void WaypointManager::_clearPath(WaypointPath& path)
{
    for (WaypointPath::const_iterator itr = path.begin(); itr != path.end(); ++itr)
        delete itr->second.behavior;
    path.clear();
}

/// Insert a node into the storage for external access
bool WaypointManager::AddExternalNode(uint32 entry, int32 pathId, uint32 pointId, float x, float y, float z, float o, uint32 waittime)
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

    m_externalPathTemplateMap[(entry << 8) + pathId][pointId] = WaypointNode(x, y, z, o, waittime, 0, nullptr);
    return true;
}

/// - Insert at a certain point, if pointId == 0 insert last. In this case pointId will be changed to the id to which the node was added
WaypointNode const* WaypointManager::AddNode(uint32 entry, uint32 dbGuid, uint32& pointId, WaypointPathOrigin wpDest, float x, float y, float z)
{
    // Support only normal movement tables
    if (wpDest != PATH_FROM_GUID && wpDest != PATH_FROM_ENTRY)
        return nullptr;

    // Prepare information
    char const* const table     = wpDest == PATH_FROM_GUID ? "creature_movement" : "creature_movement_template";
    char const* const key_field = wpDest == PATH_FROM_GUID ? "id" : "entry";
    uint32 const key            = wpDest == PATH_FROM_GUID ? dbGuid : ((entry << 8) /*+ pathId*/);
    WaypointPathMap* wpMap      = wpDest == PATH_FROM_GUID ? &m_pathMap : &m_pathTemplateMap;

    WaypointPath& path = (*wpMap)[key];

    if (pointId == 0 && !path.empty())                      // Start with highest waypoint
        pointId = path.rbegin()->first + 1;
    else if (pointId == 0)
        pointId = 1;

    uint32 nextPoint = pointId;
    WaypointNode temp = WaypointNode(x, y, z, 100, 0, 0, nullptr);
    WaypointPath::iterator find = path.find(nextPoint);
    if (find != path.end())                                 // Point already exists
    {
        do                                                  // Move points along until a free spot is found
        {
            std::swap(temp, find->second);
            ++find;
            ++nextPoint;
        } while (find != path.end() && find->first == nextPoint);
        // After this, we have:
        // pointId, pointId+1, ..., nextPoint [ Can be == path.end ]]
    }

    // Insert new or remaining
    path[nextPoint] = temp;

    // Update original waypoints
    for (WaypointPath::reverse_iterator rItr = path.rbegin(); rItr != path.rend() && rItr->first > pointId; ++rItr)
    {
        if (rItr->first <= nextPoint)
            WorldDatabase.PExecuteLog("UPDATE %s SET point=point+1 WHERE %s=%u AND point=%u", table, key_field, key, rItr->first - 1);
    }
    // Insert new Point to database
    WorldDatabase.PExecuteLog("INSERT INTO %s (%s,point,position_x,position_y,position_z,orientation) VALUES (%u,%u, %f,%f,%f, 100)", table, key_field, key, pointId, x, y, z);

    return &path[pointId];
}

void WaypointManager::DeleteNode(uint32 entry, uint32 dbGuid, uint32 point, int32 pathId, WaypointPathOrigin wpOrigin)
{
    // Support only normal movement tables
    if (wpOrigin != PATH_FROM_GUID && wpOrigin != PATH_FROM_ENTRY)
        return;

    WaypointPath* path = GetPathFromOrigin(entry, dbGuid, pathId, wpOrigin);
    if (!path)
        return;

    char const* const table     = wpOrigin == PATH_FROM_GUID ? "creature_movement" : "creature_movement_template";
    char const* const key_field = wpOrigin == PATH_FROM_GUID ? "id" : "entry";
    uint32 const key            = wpOrigin == PATH_FROM_GUID ? dbGuid : entry;
    WorldDatabase.PExecuteLog("DELETE FROM %s WHERE %s=%u AND point=%u", table, key_field, key, point);

    path->erase(point);
}

void WaypointManager::DeletePath(uint32 id)
{
    WorldDatabase.PExecuteLog("DELETE FROM creature_movement WHERE id=%u", id);
    WaypointPathMap::iterator itr = m_pathMap.find(id);
    if (itr != m_pathMap.end())
        _clearPath(itr->second);
    // the path is not removed from the map, just cleared
    // WMGs have pointers to the path, so deleting them would crash
    // this wastes some memory, but these functions are
    // only meant to be called by GM commands
}

void WaypointManager::SetNodePosition(uint32 entry, uint32 dbGuid, uint32 point, int32 pathId, WaypointPathOrigin wpOrigin, float x, float y, float z)
{
    // Support only normal movement tables
    if (wpOrigin != PATH_FROM_GUID && wpOrigin != PATH_FROM_ENTRY)
        return;

    WaypointPath* path = GetPathFromOrigin(entry, dbGuid, pathId, wpOrigin);
    if (!path)
        return;

    char const* const table     = wpOrigin == PATH_FROM_GUID ? "creature_movement" : "creature_movement_template";
    char const* const key_field = wpOrigin == PATH_FROM_GUID ? "id" : "entry";
    uint32 const key            = wpOrigin == PATH_FROM_GUID ? dbGuid : entry;
    WorldDatabase.PExecuteLog("UPDATE %s SET position_x=%f, position_y=%f, position_z=%f WHERE %s=%u AND point=%u", table, x, y, z, key_field, key, point);

    WaypointPath::iterator find = path->find(point);
    if (find != path->end())
    {
        find->second.x = x;
        find->second.y = y;
        find->second.z = z;
    }
}

void WaypointManager::SetNodeWaittime(uint32 entry, uint32 dbGuid, uint32 point, int32 pathId, WaypointPathOrigin wpOrigin, uint32 waittime)
{
    // Support only normal movement tables
    if (wpOrigin != PATH_FROM_GUID && wpOrigin != PATH_FROM_ENTRY)
        return;

    WaypointPath* path = GetPathFromOrigin(entry, dbGuid, pathId, wpOrigin);
    if (!path)
        return;

    char const* const table     = wpOrigin == PATH_FROM_GUID ? "creature_movement" : "creature_movement_template";
    char const* const key_field = wpOrigin == PATH_FROM_GUID ? "id" : "entry";
    uint32 const key            = wpOrigin == PATH_FROM_GUID ? dbGuid : entry;
    WorldDatabase.PExecuteLog("UPDATE %s SET waittime=%u WHERE %s=%u AND point=%u", table, waittime, key_field, key, point);

    WaypointPath::iterator find = path->find(point);
    if (find != path->end())
        find->second.delay = waittime;
}

void WaypointManager::SetNodeOrientation(uint32 entry, uint32 dbGuid, uint32 point, int32 pathId, WaypointPathOrigin wpOrigin, float orientation)
{
    // Support only normal movement tables
    if (wpOrigin != PATH_FROM_GUID && wpOrigin != PATH_FROM_ENTRY)
        return;

    WaypointPath* path = GetPathFromOrigin(entry, dbGuid, pathId, wpOrigin);
    if (!path)
        return;

    char const* const table     = wpOrigin == PATH_FROM_GUID ? "creature_movement" : "creature_movement_template";
    char const* const key_field = wpOrigin == PATH_FROM_GUID ? "id" : "entry";
    uint32 const key            = wpOrigin == PATH_FROM_GUID ? dbGuid : entry;
    WorldDatabase.PExecuteLog("UPDATE %s SET orientation=%f WHERE %s=%u AND point=%u", table, orientation, key_field, key, point);

    WaypointPath::iterator find = path->find(point);
    if (find != path->end())
        find->second.orientation = orientation;
}

/// return true if a valid scriptId is provided
bool WaypointManager::SetNodeScriptId(uint32 entry, uint32 dbGuid, uint32 point, int32 pathId, WaypointPathOrigin wpOrigin, uint32 scriptId)
{
    // Support only normal movement tables
    if (wpOrigin != PATH_FROM_GUID && wpOrigin != PATH_FROM_ENTRY)
        return false;

    WaypointPath* path = GetPathFromOrigin(entry, dbGuid, pathId, wpOrigin);
    if (!path)
        return false;

    char const* const table     = wpOrigin == PATH_FROM_GUID ? "creature_movement" : "creature_movement_template";
    char const* const key_field = wpOrigin == PATH_FROM_GUID ? "id" : "entry";
    uint32 const key            = wpOrigin == PATH_FROM_GUID ? dbGuid : entry;
    WorldDatabase.PExecuteLog("UPDATE %s SET script_id=%u WHERE %s=%u AND point=%u", table, scriptId, key_field, key, point);

    WaypointPath::iterator find = path->find(point);
    if (find != path->end())
        find->second.script_id = scriptId;

    return sCreatureMovementScripts.second.find(scriptId) != sCreatureMovementScripts.second.end();
}

inline void CheckWPText(bool isTemplate, uint32 entryOrGuid, uint32 point, WaypointBehavior* be, std::set<int32>& ids)
{
    int zeroCount = 0;                                      // Counting leading zeros for futher textid shift
    for (int j = 0; j < MAX_WAYPOINT_TEXT; ++j)
    {
        if (!be->textid[j])
        {
            ++zeroCount;
            continue;
        }
        if (!sObjectMgr.GetMangosStringLocale(be->textid[j]))
        {
            sLog.outErrorDb("Table `creature_movement%s %u, PointId %u has textid%u with non existing textid %i.",
                            isTemplate ? "_template` Entry:" : "` Id:", entryOrGuid, point, j, be->textid[j]);
            be->textid[j] = 0;
            ++zeroCount;
            continue;
        }
        ids.erase(uint32(be->textid[j]));

        // Shifting check
        if (zeroCount)
        {
            // Correct textid but some zeros leading, so move it forward.
            be->textid[j - zeroCount] = be->textid[j];
            be->textid[j] = 0;
        }
    }
}

void WaypointManager::CheckTextsExistance(std::set<int32>& ids)
{
    for (WaypointPathMap::const_iterator pmItr = m_pathMap.begin(); pmItr != m_pathMap.end(); ++pmItr)
    {
        for (WaypointPath::const_iterator pItr = pmItr->second.begin(); pItr != pmItr->second.end(); ++pItr)
            if (pItr->second.behavior)
                CheckWPText(false, pmItr->first, pItr->first, pItr->second.behavior, ids);
    }

    for (WaypointPathMap::const_iterator pmItr = m_pathTemplateMap.begin(); pmItr != m_pathTemplateMap.end(); ++pmItr)
    {
        for (WaypointPath::const_iterator pItr = pmItr->second.begin(); pItr != pmItr->second.end(); ++pItr)
            if (pItr->second.behavior)
                CheckWPText(true, pmItr->first, pItr->first, pItr->second.behavior, ids);
    }
}
