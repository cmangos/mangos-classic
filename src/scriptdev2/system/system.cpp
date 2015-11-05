/* This file is part of the ScriptDev2 Project. See AUTHORS file for Copyright information
* This program is free software licensed under GPL version 2
* Please see the included DOCS/LICENSE.TXT for more information */

#include "precompiled.h"
#include "system.h"
#include "../config.h"
#include "ProgressBar.h"
#include "ObjectMgr.h"
#include "Database/DatabaseEnv.h"

DatabaseType SD2Database;
std::string  strSD2Version;

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
    QueryResult* pResult = SD2Database.PQuery("SELECT version FROM sd2_db_version LIMIT 1");

    if (pResult)
    {
        Field* pFields = pResult->Fetch();

        strSD2Version = pFields[0].GetCppString();

        delete pResult;
    }
    else
        script_error_log("Missing `sd2_db_version` information.");

    // Setup version info and display it
    if (strSD2Version.empty())
        strSD2Version.append("ScriptDev2 ");

   strSD2Version.append(_FULLSD2VERSION);

    outstring_log("Loading %s", strSD2Version.c_str());
    outstring_log("");
}

void SystemMgr::LoadScriptTexts()
{
    outstring_log("SD2: Loading Script Texts...");
    LoadMangosStrings(SD2Database, "script_texts", TEXT_SOURCE_TEXT_START, TEXT_SOURCE_TEXT_END, true);
}

void SystemMgr::LoadScriptTextsCustom()
{
    outstring_log("SD2: Loading Custom Texts...");
    LoadMangosStrings(SD2Database, "custom_texts", TEXT_SOURCE_CUSTOM_START, TEXT_SOURCE_CUSTOM_END, true);
}

void SystemMgr::LoadScriptGossipTexts()
{
    outstring_log("SD2: Loading Gossip Texts...");
    LoadMangosStrings(SD2Database, "gossip_texts", TEXT_SOURCE_GOSSIP_START, TEXT_SOURCE_GOSSIP_END);
}

void SystemMgr::LoadScriptWaypoints()
{
    uint64 uiCreatureCount = 0;

    // Load Waypoints
    QueryResult* pResult = SD2Database.PQuery("SELECT COUNT(entry) FROM script_waypoint GROUP BY entry");
    if (pResult)
    {
        uiCreatureCount = pResult->GetRowCount();
        delete pResult;
    }

    outstring_log("SD2: Loading Script Waypoints for " UI64FMTD " creature(s)...", uiCreatureCount);

    pResult = SD2Database.PQuery("SELECT entry, pointid, location_x, location_y, location_z, waittime FROM script_waypoint ORDER BY entry, pointid");

    if (pResult)
    {
        BarGoLink bar(pResult->GetRowCount());
        uint32 uiNodeCount = 0;

        do
        {
            bar.step();
            Field* pFields = pResult->Fetch();

            uint32 uiEntry  = pFields[0].GetUInt32();
            int32 pathId    = 1; // pFields[X].GetInt32();
            uint32 pointId  = pFields[1].GetUInt32();
            uint32 delay    = pFields[5].GetUInt32();

            CreatureInfo const* pCInfo = GetCreatureTemplateStore(uiEntry);
            if (!pCInfo)
            {
                error_db_log("SD2: DB table script_waypoint has waypoint for nonexistent creature entry %u", uiEntry);
                continue;
            }


            if (AddWaypointFromExternal(uiEntry, pathId, pointId, pFields[2].GetFloat(), pFields[3].GetFloat(), pFields[4].GetFloat(), 100, delay))
                m_pathInfo[uiEntry][pathId].lastWaypoint = pointId;

            ++uiNodeCount;
        }
        while (pResult->NextRow());

        delete pResult;

        outstring_log("");
        outstring_log(">> Loaded %u Script Waypoint nodes.", uiNodeCount);
    }
    else
    {
        BarGoLink bar(1);
        bar.step();
        outstring_log("");
        outstring_log(">> Loaded 0 Script Waypoints. DB table `script_waypoint` is empty.");
    }
}
