/* This file is part of the ScriptDev2 Project. See AUTHORS file for Copyright information
* This program is free software licensed under GPL version 2
* Please see the included DOCS/LICENSE.TXT for more information */

#ifndef SC_SYSTEM_H
#define SC_SYSTEM_H

extern DatabaseType SD2Database;
extern std::string  strSD2Version;                          // version info: database entry and revision

#define TEXT_SOURCE_RANGE -1000000                          // the amount of entries each text source has available

#define TEXT_SOURCE_TEXT_START      TEXT_SOURCE_RANGE
#define TEXT_SOURCE_TEXT_END        TEXT_SOURCE_RANGE*2 + 1

#define TEXT_SOURCE_CUSTOM_START    TEXT_SOURCE_RANGE*2
#define TEXT_SOURCE_CUSTOM_END      TEXT_SOURCE_RANGE*3 + 1

#define TEXT_SOURCE_GOSSIP_START    TEXT_SOURCE_RANGE*3
#define TEXT_SOURCE_GOSSIP_END      TEXT_SOURCE_RANGE*4 + 1

#define pSystemMgr SystemMgr::Instance()

struct PathInformation
{
    uint32 lastWaypoint;
};

class SystemMgr
{
    public:
        SystemMgr();
        ~SystemMgr() {}

        static SystemMgr& Instance();

        typedef std::map < uint32 /*entry*/, std::map < int32 /*pathId*/, PathInformation > > EntryPathInfo;

        // Database
        void LoadVersion();
        void LoadScriptTexts();
        void LoadScriptTextsCustom();
        void LoadScriptGossipTexts();
        void LoadScriptWaypoints();

        PathInformation const* GetPathInfo(uint32 entry, int32 pathId) const
        {
            EntryPathInfo::const_iterator findEntry = m_pathInfo.find(entry);
            if (findEntry == m_pathInfo.end())
                return nullptr;
            std::map<int32, PathInformation>::const_iterator findPath = findEntry->second.find(pathId);
            if (findPath == findEntry->second.end())
                return nullptr;

            return &(findPath->second);
        }

    private:
        EntryPathInfo m_pathInfo;
};

#endif
