/*
 * Copyright (C) 2017-2020 namreeb (legal@namreeb.org)
 *
 * This is private software and may not be shared under any circumstances,
 * absent permission of namreeb.
 */

#include "antispammgr.hpp"
#include "antispam.hpp"
#include "../config.hpp"
#include "Server/WorldSession.h"
#include "World/World.h"
#include "Accounts/AccountMgr.h"
#include "Log.h"

#include "Database/DatabaseEnv.h"
#include "Policies/Singleton.h"

#include <string>
#include <mutex>
#include <vector>
#include <regex>
#include <algorithm>
#include <memory>
#include <unordered_set>
#include <thread>
#include <chrono>
#include <array>

INSTANTIATE_SINGLETON_1(NamreebAnticheat::AntispamMgr);

namespace
{
void ReplaceAll(std::string &str, const std::string& from, const std::string& to)
{
    size_t startPos = 0;
    while ((startPos = str.find(from, startPos)) != std::string::npos)
    {
        str.replace(startPos, from.length(), to);
        startPos += to.length();
    }
}

void ReplaceAllW(std::wstring &str, const std::wstring& from, const std::wstring& to)
{
    size_t startPos = 0;
    while ((startPos = str.find(from, startPos)) != std::wstring::npos)
    {
        str.replace(startPos, from.length(), to);
        startPos += to.length();
    }
}
}

namespace NamreebAnticheat
{
std::string AntispamMgr::NormalizeString(const std::string &string, uint32 mask) const
{
    std::lock_guard<std::mutex> guard(_mutex);
    return NormalizeStringInternal(string, mask);
}

std::string AntispamMgr::NormalizeStringInternal(const std::string &string, uint32 mask) const
{
    auto newMsg = string;

    if (mask & NF_CUT_COLOR)
    {
        static const std::regex regex1("(\\|c\\w{8})");
        static const std::regex regex2("(\\|H[\\w|\\W]{1,}\\|h)");
        newMsg = std::regex_replace(newMsg, regex1, "");
        ReplaceAll(newMsg, "|h|r", "");
        newMsg = std::regex_replace(newMsg, regex2, "");
    }

    if (mask & NF_REPLACE_WORDS)
    {
        for (auto const& e : _asciiReplace)
            ReplaceAll(newMsg, e.first, e.second);
    }

    if (mask & NF_CUT_CTRL)
    {
        static const std::regex regex4("([[:cntrl:]]+)");
        newMsg = std::regex_replace(newMsg, regex4, "");
    }

    if (mask & NF_CUT_PUNCT)
    {
        static const std::regex regex5("([[:punct:]]+)");
        newMsg = std::regex_replace(newMsg, regex5, "");
    }

    if (mask & NF_CUT_SPACE)
    {
        static const std::regex regex6("(\\s+|_)");
        newMsg = std::regex_replace(newMsg, regex6, "");
    }

    if (mask & NF_CUT_NUMBERS)
    {
        static std::regex regex3("(\\d+)");
        newMsg = std::regex_replace(newMsg, regex3, "");
    }

    if (mask & NF_REPLACE_UNICODE)
    {
        std::wstring w_tempMsg, w_tempMsg2;
        Utf8toWStr(newMsg, w_tempMsg);
        wstrToUpper(w_tempMsg);

        if (!isBasicLatinString(w_tempMsg, true))
        {
            for (auto const& s : _unicodeReplace)
                ReplaceAllW(w_tempMsg, s.first, s.second);

            if (mask & NF_REMOVE_NON_LATIN)
            {
                for (size_t i = 0; i < w_tempMsg.size(); ++i)
                    if (isBasicLatinCharacter(w_tempMsg[i]) || isNumeric(w_tempMsg[i]))
                        w_tempMsg2.push_back(w_tempMsg[i]);
            }
            else
                w_tempMsg2 = w_tempMsg;
        }
        else
            w_tempMsg2 = w_tempMsg;

        newMsg = std::string(w_tempMsg2.begin(), w_tempMsg2.end());
    }
    else
        std::transform(newMsg.begin(), newMsg.end(), newMsg.begin(), ::toupper);

    if (mask & NF_REMOVE_REPEATS)
        newMsg.erase(std::unique(newMsg.begin(), newMsg.end()), newMsg.end());

    return newMsg;
}

AntispamMgr::AntispamMgr() : _shutdownRequested(false), _worker(&AntispamMgr::WorkerLoop, this) {}

AntispamMgr::~AntispamMgr()
{
    _shutdownRequested = true;
    _worker.join();
}

void AntispamMgr::WorkerLoop()
{
    while (!_shutdownRequested)
    {
        auto const duration = sAnticheatConfig.GetAntispamAnalysisTimer() * IN_MILLISECONDS;

        auto const start = std::chrono::high_resolution_clock::now();
        auto const startMS = WorldTimer::getMSTime();
        auto const expireMS = sAnticheatConfig.GetAntispamCacheExpiration() * IN_MILLISECONDS;

        if (sAnticheatConfig.EnableAntispam())
        {
            std::unordered_set<std::shared_ptr<Antispam> > workQueue;

            // lock the mutex only long enough to move the work queue to a local container and expire old blacklist history
            {
                std::lock_guard<std::mutex> guard(_mutex);
                workQueue = std::move(_workQueue);

                for (auto i = _temporaryCache.begin(); i != _temporaryCache.end(); )
                {
                    if (i->second.first + expireMS <= startMS)
                        i = _temporaryCache.erase(i);
                    else
                        ++i;
                }
            }

            for (auto const &s : workQueue)
                s->Analyze();
        }
        else
        {
            std::lock_guard<std::mutex> guard(_mutex);
            _workQueue.clear();
            _temporaryCache.clear();
        }

        auto stop = std::chrono::high_resolution_clock::now();
        auto runTime = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();

        // if we completed sooner than expected, sleep for the remaining time
        while (duration > runTime && !_shutdownRequested)
        {
            stop = std::chrono::high_resolution_clock::now();
            runTime = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }
}

void AntispamMgr::LoadFromDB()
{
    std::lock_guard<std::mutex> guard(_mutex);

    auto const normMask = sAnticheatConfig.GetSpamNormalizationMask();

    _blacklist.clear();

    std::unique_ptr<QueryResult> result(LoginDatabase.Query("SELECT `string` FROM antispam_blacklist"));

    if (result)
        do
        {
            auto fields = result->Fetch();
            auto entry = fields[0].GetCppString();

            if (entry.empty())
            {
                sLog.outError("Refusing to load empty antispam blacklist entry");
                continue;
            }

            std::transform(entry.begin(), entry.end(), entry.begin(), ::toupper);

            for (auto const &b : _blacklist)
                if (b.first == entry)
                {
                    sLog.outError("Duplicate entry \"%s\" in antispam blacklist", entry.c_str());
                    continue;
                }

            auto const normEntry = NormalizeStringInternal(entry, normMask);

            _blacklist.emplace_back(entry, normEntry);
        } while (result->NextRow());

    sLog.outString(">> %lu blacklist entries loaded and normalized", uint64(_blacklist.size()));

    result.reset(LoginDatabase.Query("SELECT `from`, `to` FROM antispam_replacement"));

    _asciiReplace.clear();

    if (result)
        do
        {
            auto fields = result->Fetch();
            _asciiReplace.emplace_back(fields[0].GetCppString(), fields[1].GetCppString());
        } while (result->NextRow());

    sLog.outString(">> %lu ASCII string replacements loaded", uint64(_asciiReplace.size()));

    result.reset(LoginDatabase.Query("SELECT `from`, `to` FROM antispam_unicode_replacement"));

    _unicodeReplace.clear();

    if (result)
        do
        {
            std::wostringstream wss;

            auto fields = result->Fetch();
            
            wss.str(std::wstring());
            wss << wchar_t(fields[0].GetUInt32());
            std::wstring key = wss.str();

            wss.str(std::wstring());
            wss << wchar_t(fields[1].GetUInt32());
            std::wstring value = wss.str();

            _unicodeReplace.emplace_back(key, value);
        } while (result->NextRow());

    sLog.outString(">> %lu unicode character replacements loaded", uint64(_unicodeReplace.size()));
}

void AntispamMgr::BlacklistAdd(const std::string &string_)
{
    std::lock_guard<std::mutex> guard(_mutex);

    // cannot be empty!
    if (string_.empty())
        return;

    // convert to upper case, since all comparisons are done upper case
    std::string entry;
    std::transform(string_.begin(), string_.end(), std::back_inserter(entry), ::toupper);

    auto const normEntry = NormalizeStringInternal(entry, sAnticheatConfig.GetSpamNormalizationMask());

    // if already in the blacklist, do not add again
    for (auto const &b : _blacklist)
        if (b.first == entry)
            return;

    static SqlStatementID insertBlacklist;

    LoginDatabase.BeginTransaction();

    auto ins = LoginDatabase.CreateStatement(insertBlacklist, "INSERT INTO antispam_blacklist (string) VALUES(?)");

    ins.addString(entry);
    ins.Execute();

    LoginDatabase.CommitTransaction();

    _blacklist.emplace_back(entry, normEntry);
}

uint32 AntispamMgr::CheckBlacklist(const std::string &string, std::string &log) const
{
    std::lock_guard<std::mutex> guard(_mutex);

    auto const normalizationMask = sAnticheatConfig.GetSpamNormalizationMask();
    auto const msg = NormalizeStringInternal(string, normalizationMask);

    std::stringstream logstr;
    logstr << "Original message:\n" << string << "\nNormalized message:\n" << msg << "\nBlacklist violations:";

    uint32 result = 0;

    // for each entry, search the given string for multiple occurrences of both the original and normalized form of the entry
    for (auto const& entry : _blacklist)
    {
        // search the original string for the original blacklist entry
        if (!entry.first.empty())
        {
            for (auto pos = string.find(entry.first); pos != std::string::npos; pos = string.find(entry.first, pos + entry.first.length()))
            {
                logstr << "\nOriginal: \"" << entry.first << "\"";
                ++result;
            }
        }

        // search for the normalized blacklist entry
        if (!entry.second.empty())
        {
            for (auto pos = msg.find(entry.second); pos != std::string::npos; pos = msg.find(entry.second, pos + entry.second.length()))
            {
                logstr << "\nNormalized: \"" << entry.second << "\"";
                ++result;
            }
        }
    }

    logstr << "\n";

    // if there were results found, save the log
    if (!!result)
        log = logstr.str();

    return result;
}

void AntispamMgr::ScheduleAnalysis(std::shared_ptr<Antispam> session)
{
    std::lock_guard<std::mutex> guard(_mutex);
    _workQueue.insert(session);
}

void AntispamMgr::CacheSession(std::shared_ptr<Antispam> session)
{
    std::lock_guard<std::mutex> guard(_mutex);
    _temporaryCache[session->GetAccount()] = std::make_pair(WorldTimer::getMSTime(), session);
}

std::shared_ptr<Antispam> AntispamMgr::GetSession(uint32 accountId)
{
    std::lock_guard<std::mutex> guard(_mutex);
    
    auto const i = _temporaryCache.find(accountId);

    if (i == _temporaryCache.end())
        return std::make_shared<Antispam>(accountId);

    auto ret = i->second.second;

    _temporaryCache.erase(i);

    return ret;
}

std::shared_ptr<Antispam> AntispamMgr::CheckCache(uint32 accountId)
{
    std::lock_guard<std::mutex> guard(_mutex);

    auto const i = _temporaryCache.find(accountId);

    if (i == _temporaryCache.end())
        return nullptr;

    return i->second.second;
}

bool AntispamMgr::IsSilenced(uint32 accountId) const
{
    if (auto const session = sWorld.FindSession(accountId))
        return IsSilenced(session);

    // ACCOUNT FLAGS
    return !!(sAccountMgr.GetFlags(accountId) & ACCOUNT_FLAG_SILENCED);
}

bool AntispamMgr::IsSilenced(const WorldSession *session) const
{
    // ACCOUNT FLAGS
    return !!(session->GetAccountFlags() & ACCOUNT_FLAG_SILENCED);
}

void AntispamMgr::Silence(uint32 accountId) const
{
    // ACCOUNT FLAGS
    if (auto session = sWorld.FindSession(accountId))
        session->AddAccountFlag(ACCOUNT_FLAG_SILENCED);

    LoginDatabase.BeginTransaction();

    // remove any permanent mute that may be present
    LoginDatabase.PExecute("UPDATE account SET flags = flags | 0x%x WHERE id = %u", ACCOUNT_FLAG_SILENCED, accountId);

    LoginDatabase.CommitTransaction();
}

void AntispamMgr::Unsilence(uint32 accountId) const
{
    // ACCOUNT FLAGS
    if (auto session = sWorld.FindSession(accountId))
        session->RemoveAccountFlag(ACCOUNT_FLAG_SILENCED);

    LoginDatabase.BeginTransaction();

    // remove any permanent mute that may be present
    LoginDatabase.PExecute("UPDATE account SET flags = flags & ~0x%x WHERE id = %u", ACCOUNT_FLAG_SILENCED, accountId);

    LoginDatabase.CommitTransaction();
}
}