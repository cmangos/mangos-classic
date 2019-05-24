/* This file is part of the ScriptDev2 Project. See AUTHORS file for Copyright information
* This program is free software licensed under GPL version 2
* Please see the included DOCS/LICENSE.TXT for more information */

#include "AI/ScriptDevAI/include/precompiled.h"
#include "system.h"
#include "ProgressBar.h"
#include "Globals/ObjectMgr.h"
#include "Database/DatabaseEnv.h"
#include "MotionGenerators/WaypointManager.h"

std::string strSD2Version;

SystemMgr::SystemMgr()
{
}

SystemMgr& SystemMgr::Instance()
{
    static SystemMgr pSysMgr;
    return pSysMgr;
}

void SystemMgr::LoadVersion()
{
    // Get Version information
    QueryResult* result = WorldDatabase.PQuery("SELECT version FROM sd2_db_version LIMIT 1");

    if (result)
    {
        Field* fields = result->Fetch();

        strSD2Version = fields[0].GetCppString();
        delete result;
    }
    else
        script_error_log("Missing `sd2_db_version` information.");

    // Setup version info and display it
    if (strSD2Version.empty())
        strSD2Version.append("ScriptDev2 ");

    outstring_log("Loading %s\n", strSD2Version.c_str());
}

void SystemMgr::LoadScriptTexts()
{
    outstring_log("SD2: Loading Script Texts...");
    LoadMangosStrings(WorldDatabase, "script_texts", TEXT_SOURCE_TEXT_START, TEXT_SOURCE_TEXT_END, true);
}

void SystemMgr::LoadScriptTextsCustom()
{
    outstring_log("SD2: Loading Custom Texts...");
    LoadMangosStrings(WorldDatabase, "custom_texts", TEXT_SOURCE_CUSTOM_START, TEXT_SOURCE_CUSTOM_END, true);
}

void SystemMgr::LoadScriptGossipTexts()
{
    outstring_log("SD2: Loading Gossip Texts...");
    LoadMangosStrings(WorldDatabase, "gossip_texts", TEXT_SOURCE_GOSSIP_START, TEXT_SOURCE_GOSSIP_END);
}

void SystemMgr::LoadScriptWaypoints()
{
    uint64 creatureCount = 0;

    // Load Waypoints
    QueryResult* result = WorldDatabase.PQuery("SELECT COUNT(entry) FROM script_waypoint GROUP BY entry");
    if (result)
    {
        creatureCount = result->GetRowCount();
        delete result;
    }

    outstring_log("SD2: Loading Script Waypoints for " UI64FMTD " creature(s)...", creatureCount);

    result = WorldDatabase.PQuery("SELECT entry, pathId, pointid, position_x, position_y, position_z, orientation, waittime, script_id FROM script_waypoint ORDER BY entry, pathId, pointid");

    if (result)
    {
        BarGoLink bar(result->GetRowCount());
        uint32 nodeCount = 0;

        do
        {
            bar.step();
            Field* fields = result->Fetch();

            uint32 entry  = fields[0].GetUInt32();

            CreatureInfo const* info = GetCreatureTemplateStore(entry);
            if (!info)
            {
                error_db_log("SD2: DB table script_waypoint has waypoint for nonexistent creature entry %u", entry);
                continue;
            }

            uint32 pathId       = fields[1].GetUInt32();
            uint32 pointId      = fields[2].GetUInt32();
            float position_x    = fields[3].GetFloat();
            float position_y    = fields[4].GetFloat();
            float position_z    = fields[5].GetFloat();
            float orientation   = fields[6].GetFloat();
            uint32 waitTime     = fields[7].GetUInt32();
            uint32 scriptId     = fields[8].GetUInt32();

            sWaypointMgr.AddExternalNode(entry, pathId, pointId, position_x, position_y, position_z, orientation, waitTime, scriptId);

            ++nodeCount;
        }
        while (result->NextRow());

        delete result;

        outstring_log("\n>> Loaded %u Script Waypoint nodes.", nodeCount);
    }
    else
    {
        BarGoLink bar(1);
        bar.step();
        outstring_log("\n>> Loaded 0 Script Waypoints. DB table `script_waypoint` is empty.");
    }
}
