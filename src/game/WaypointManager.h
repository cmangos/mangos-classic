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

#ifndef MANGOS_WAYPOINTMANAGER_H
#define MANGOS_WAYPOINTMANAGER_H

#include "Common.h"

enum WaypointPathOrigin
{
    PATH_NO_PATH            = 0,
    PATH_FROM_GUID          = 1,
    PATH_FROM_ENTRY         = 2,
    PATH_FROM_EXTERNAL      = 3
};

// Obsolete structure
#define MAX_WAYPOINT_TEXT 5
struct WaypointBehavior
{
    uint32 emote;
    uint32 spell;
    int32  textid[MAX_WAYPOINT_TEXT];
    uint32 model1;
    uint32 model2;

    bool isEmpty();
    WaypointBehavior(): emote(0), spell(0), model1(0), model2(0)
    {}
    WaypointBehavior(const WaypointBehavior& b);
};

struct WaypointNode
{
    float x;
    float y;
    float z;
    float orientation;
    uint32 delay;
    uint32 script_id;                                       // Added may 2010. WaypointBehavior w/DB data should in time be removed.
    WaypointBehavior* behavior;
    WaypointNode() : x(0.0f), y(0.0f), z(0.0f), orientation(0.0f), delay(0), script_id(0), behavior(nullptr) {}
    WaypointNode(float _x, float _y, float _z, float _o, uint32 _delay, uint32 _script_id, WaypointBehavior* _behavior)
        : x(_x), y(_y), z(_z), orientation(_o), delay(_delay), script_id(_script_id), behavior(_behavior) {}
};

typedef std::map<uint32 /*pointId*/, WaypointNode> WaypointPath;

class WaypointManager
{
    public:
        WaypointManager() : m_externalTable("external.waypointTable") {}
        ~WaypointManager() { Unload(); }

        void Load();
        void Unload();

        // We may get a path for several occasions:

        // 1: When creature.MovementType=2
        //    1a) Path is selected by creature.guid == creature_movement.id
        //    1b) Path for 1a) does not exist and then use path from creature.GetEntry() == creature_movement_template.entry

        // 2: When creature_template.MovementType=2
        //    2a) Creature is summoned and has creature_template.MovementType=2
        //        Creators need to be sure that creature_movement_template is always valid for summons.
        //        Mob that can be summoned anywhere should not have creature_movement_template for example.

        WaypointPath* GetDefaultPath(uint32 entry, uint32 lowGuid, WaypointPathOrigin* wpOrigin = nullptr)
        {
            WaypointPath* path = GetPath(lowGuid);
            if (path && wpOrigin)
                *wpOrigin = PATH_FROM_GUID;

            // No movement found for guid
            if (!path)
            {
                path = GetPathTemplate(entry);
                if (path && wpOrigin)
                    *wpOrigin = PATH_FROM_ENTRY;
            }

            return path;
        }

        // Helper function to get a path provided the required information
        WaypointPath* GetPathFromOrigin(uint32 entry, uint32 lowGuid, int32 pathId, WaypointPathOrigin wpOrigin)
        {
            WaypointPathMap* wpMap;
            uint32 key;

            switch (wpOrigin)
            {
                case PATH_FROM_GUID:
                    key = lowGuid;
                    wpMap = &m_pathMap;
                    break;
                case PATH_FROM_ENTRY:
                    if (pathId >= 0xFF || pathId < 0)
                        return nullptr;
                    key = (entry << 8) + pathId;
                    wpMap = &m_pathTemplateMap;
                    break;
                case PATH_FROM_EXTERNAL:
                    if (pathId >= 0xFF || pathId < 0)
                        return nullptr;
                    key = (entry << 8) + pathId;
                    wpMap = &m_externalPathTemplateMap;
                    break;
                case PATH_NO_PATH:
                default:
                    return nullptr;
            }
            WaypointPathMap::iterator find = wpMap->find(key);
            return find != wpMap->end() ? &find->second : nullptr;
        }

        void DeletePath(uint32 id);
        void CheckTextsExistance(std::set<int32>& ids);

        /// Set external source table
        void SetExternalWPTable(char const* tableName) { m_externalTable = std::string(tableName); }
        std::string GetExternalWPTable() const { return m_externalTable; }
        /// Add Nodes from external sources
        bool AddExternalNode(uint32 entry, int32 pathId, uint32 pointId, float x, float y, float z, float o, uint32 waittime);

        // Toolbox for .wp add command
        /// Add a node as position pointId. If pointId == 0 then as last point
        WaypointNode const* AddNode(uint32 entry, uint32 dbGuid, uint32& pointId, WaypointPathOrigin wpDest, float x, float y, float z);

        // Toolbox for .wp modify command
        void DeleteNode(uint32 entry, uint32 dbGuid, uint32 point, int32 pathId, WaypointPathOrigin wpOrigin);
        void SetNodePosition(uint32 entry, uint32 dbGuid, uint32 point, int32 pathId, WaypointPathOrigin wpOrigin, float x, float y, float z);
        void SetNodeWaittime(uint32 entry, uint32 dbGuid, uint32 point, int32 pathId, WaypointPathOrigin wpOrigin, uint32 waittime);
        void SetNodeOrientation(uint32 entry, uint32 dbGuid, uint32 point, int32 pathId, WaypointPathOrigin wpOrigin, float orientation);
        bool SetNodeScriptId(uint32 entry, uint32 dbGuid, uint32 point, int32 pathId, WaypointPathOrigin wpOrigin, uint32 scriptId);

        // Small Helper for nice output
        static std::string GetOriginString(WaypointPathOrigin origin)
        {
            switch (origin)
            {
                case PATH_NO_PATH:          return "<no path>";
                case PATH_FROM_GUID:        return "guid";
                case PATH_FROM_ENTRY:       return "entry";
                case PATH_FROM_EXTERNAL:    return "external";
                default:                    return "invalid origin";
            }
        }

    private:
        WaypointPath* GetPath(uint32 id)
        {
            WaypointPathMap::iterator itr = m_pathMap.find(id);
            return itr != m_pathMap.end() ? &itr->second : nullptr;
        }

        WaypointPath* GetPathTemplate(uint32 entry)
        {
            WaypointPathMap::iterator itr = m_pathTemplateMap.find((entry << 8) /*+ pathId*/);
            return itr != m_pathTemplateMap.end() ? &itr->second : nullptr;
        }

        void _clearPath(WaypointPath& path);

        typedef std::unordered_map<uint32 /*guidOrEntry*/, WaypointPath> WaypointPathMap;
        WaypointPathMap m_pathMap;
        WaypointPathMap m_pathTemplateMap;
        WaypointPathMap m_externalPathTemplateMap;
        std::string m_externalTable;
};

#define sWaypointMgr MaNGOS::Singleton<WaypointManager>::Instance()

/// Accessor for Scripting library
MANGOS_DLL_SPEC bool AddWaypointFromExternal(uint32 entry, int32 pathId, uint32 pointId, float x, float y, float z, float o, uint32 waittime);

#endif
