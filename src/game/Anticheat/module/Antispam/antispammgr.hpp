/*
 * Copyright (C) 2017-2020 namreeb (legal@namreeb.org)
 *
 * This is private software and may not be shared under any circumstances,
 * absent permission of namreeb.
 */

#ifndef __ANTISPAMMGR_HPP_
#define __ANTISPAMMGR_HPP_

#include "Policies/Singleton.h"

#include <string>
#include <vector>
#include <mutex>
#include <unordered_set>
#include <unordered_map>
#include <thread>
#include <memory>
#include <array>

class WorldSession;

namespace NamreebAnticheat
{
class Antispam;

class AntispamMgr
{
    private:
        // note that once we begin using c++17, std::shared_mutex will be much more performant
        // here as the guarded values are seldom changed but frequently read
        mutable std::mutex _mutex;

        bool _shutdownRequested;

        // this collection contains a pair of strings, the original entry and the normalized version based on current settings
        std::vector<std::pair<std::string, std::string> > _blacklist;

        // NOTE: _asciiReplace and _unicodeReplace are not protected by _mutex, because it would make the code much more complicated
        // and they should never be changing once the world server has started.

        std::vector<std::pair<std::string, std::string> > _asciiReplace;        // replacements for ascii strings (for things like @ -> A or \/\/ -> W etc.)
        std::vector<std::pair<std::wstring, std::wstring> > _unicodeReplace;    // replacements for individual unicode characters

        // set of sessions to analyze in the next tick of the antispam worker thread
        std::unordered_set<std::shared_ptr<Antispam> > _workQueue;

        // temporarily cache antispam session information in case they reconnect and resume spamming
        std::unordered_map<uint32, std::pair<uint32, std::shared_ptr<Antispam> > > _temporaryCache;

        // the thread is declared after all other members to guarantee that it is initialized last
        std::thread _worker;

        // this function performs the actual normalization, but assumes that the mutex is already locked
        std::string NormalizeStringInternal(const std::string &string, uint32 mask) const;

        void WorkerLoop();

    public:
        AntispamMgr();
        ~AntispamMgr();

        void LoadFromDB();

        // assumes ownership of the mutex and normalizes a string
        std::string NormalizeString(const std::string &string, uint32 mask) const;

        void BlacklistAdd(const std::string &string);

        // returns how many blacklist entries appear in the given string, including multiple occurrences of the same entry
        // will also log blacklist violations in 'log', if there are any
        uint32 CheckBlacklist(const std::string &string, std::string &log) const;

        void ScheduleAnalysis(std::shared_ptr<Antispam> session);

        // temporarily cache antispam session data for the configured amount of time in case the
        // account reconnects to a new session
        void CacheSession(std::shared_ptr<Antispam> session);

        // creates new antispam session data, or restores old data from the cache if it is available and
        // remove it from the cache
        std::shared_ptr<Antispam> GetSession(uint32 accountId);

        // readonly check on the cache.  returns null if nothing is found
        std::shared_ptr<Antispam> CheckCache(uint32 accountId);

        bool IsSilenced(uint32 accountId) const;
        bool IsSilenced(const WorldSession *session) const;
        void Silence(uint32 accountId) const;
        void Unsilence(uint32 accountId) const;
};
}

#define sAntispamMgr MaNGOS::Singleton<NamreebAnticheat::AntispamMgr>::Instance()

#endif /* !__ANTISPAMMGR_HPP_ */