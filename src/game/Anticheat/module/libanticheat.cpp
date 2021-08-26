/*
 * Copyright (C) 2017-2020 namreeb (legal@namreeb.org)
 *
 * This is private software and may not be shared under any circumstances,
 * absent permission of namreeb.
 */

#include "libanticheat.hpp"
#include "config.hpp"

#include "Policies/Singleton.h"
#include "World/World.h"
#include "Entities/Player.h"
#include "Server/WorldSession.h"
#include "Accounts/AccountMgr.h"
#include "Globals/ObjectAccessor.h"
#include "Chat/Channel.h"
#include "Chat/ChannelMgr.h"
#include "Database/DatabaseImpl.h"
#include "Database/DatabaseEnv.h"
#include "Chat/Chat.h"
#include "Tools/Language.h"
#include "Globals/ObjectMgr.h"
#include "Log.h"

#include "Antispam/antispammgr.hpp"
#include "Antispam/antispam.hpp"
#include "Movement/movement.hpp"
#include "Warden/warden.hpp"
#include "Warden/WardenWin.hpp"
#include "Warden/WardenMac.hpp"
#include "Warden/WardenScanMgr.hpp"

#include <memory>
#include <sstream>
#include <iostream>
#include <string>
#include <cstdio>
#include <functional>
#include <algorithm>
#include <exception>
#include <mutex>

namespace
{
Warden *CreateWarden(WorldSession *session, const BigNumber &K, NamreebAnticheat::SessionAnticheat *anticheat)
{
    Warden* warden;
    ClientOSType os = session->GetOS();

    if (os == CLIENT_OS_MAC)
        warden = new WardenMac(session, K, anticheat);
    else if (os == CLIENT_OS_WIN)
        warden = new WardenWin(session, K, anticheat);
    else
    {
        anticheat->RecordCheatInternal(NamreebAnticheat::CheatType::CHEAT_TYPE_WARDEN, "Unknown client operating system");

        // kick here regardless of what the config says because we cannot support this scenario!
        session->KickPlayer();
        return nullptr;
    }

    return warden;
}

std::string SplitWord(std::string &in)
{
    auto space = in.find(' ');

    if (space == std::string::npos)
        return std::move(in);

    auto const ret = in.substr(0, space);
    in = in.substr(space + 1);
    return ret;
}

// taken from https://graphics.stanford.edu/~seander/bithacks.html#CountBitsSetParallel
int CountBits(uint32 value)
{
    value = value - ((value >> 1) & 0x55555555);
    value = (value & 0x33333333) + ((value >> 2) & 0x33333333);
    return (((value + (value >> 4)) & 0x0F0F0F0F) * 0x01010101) >> 24;
}

std::string ActionMaskToString(uint32 actionMask)
{
    if (actionMask == CHEAT_ACTION_NONE)
        return "None";

    std::stringstream r;

    if (!!(actionMask & CHEAT_ACTION_PROMPT_LOG))
        r << "ActionPrompt, ";
    else if (!!(actionMask & CHEAT_ACTION_INFO_LOG))
        r << "Inform, ";

    if (!!(actionMask & CHEAT_ACTION_KICK))
        r << "Kick, ";
    else
    {
        if (!!(actionMask & CHEAT_ACTION_BAN_ACCOUNT))
            r << "BanAccount, ";
        if (!!(actionMask & CHEAT_ACTION_BAN_IP))
            r << "BanIP, ";
    }

    if (!!(actionMask & CHEAT_ACTION_SILENCE))
        r << "Silence, ";

    auto ret = r.str();
    if (ret[ret.length() - 2] == ',')
        ret = ret.substr(0, ret.length() - 2);

    return ret;
}

// log cheat event into database, and notify GMs.  the comment is not required to have player,
// account, ip, etc. information.  it IS expected to have information about the cheat itself
void LogCheat(WorldSession *session, uint32 actionMask, const std::string &info)
{
    auto const name = session->GetPlayerName();
    auto const action = ActionMaskToString(actionMask);

    std::stringstream message;

    message << "Player: " << name
            << " IP: " << session->GetRemoteAddress()
            << " Account ID: " << session->GetAccountId()
            << " Action: " << action
            << " " << info;

    // GM NOTIFICATION
    sWorld.SendGMTextFlags(ACCOUNT_FLAG_SHOW_ANTICHEAT, LANG_GM_ANNOUNCE_COLOR, "AntiCheat", message.str().c_str());

    // LOGS DATABASE
    static SqlStatementID insLog;

    LogsDatabase.BeginTransaction();

    auto stmt = LogsDatabase.CreateStatement(insLog,
        "INSERT INTO logs_anticheat (realm, account, ip, fingerprint, actionMask, player, info) VALUES(?, ?, ?, ?, ?, ?, ?)");

    stmt.addUInt32(realmID);
    stmt.addUInt32(session->GetAccountId());
    stmt.addString(session->GetRemoteAddress());

    if (auto const anticheat = dynamic_cast<NamreebAnticheat::SessionAnticheat *>(session->GetAnticheat()))
        stmt.addUInt32(anticheat->GetFingerprint());
    else
        stmt.addUInt32(0);

    stmt.addUInt8(actionMask);
    stmt.addString(name);
    stmt.addString(info);

    stmt.Execute();

    LogsDatabase.CommitTransaction();

    sLog.outBasic("Cheat detected: %s", message.str().c_str());
}

void CleanupFingerprintHistoryCallback(QueryResult *result, uint32 fingerprint)
{
    if (!result)
        return;

    auto const history = sAnticheatConfig.GetFingerprintHistory();
    auto const pruneCount = result->Fetch()[0].GetUInt32();

    if (pruneCount > history)
    {
        static SqlStatementID pruneLog;

        LoginDatabase.BeginTransaction();

        auto prune = LoginDatabase.CreateStatement(pruneLog, "DELETE FROM system_fingerprint_usage WHERE fingerprint = ? ORDER BY `time` ASC LIMIT ?");

        prune.addUInt32(fingerprint);
        prune.addUInt32(pruneCount - history);
        prune.Execute();
        LoginDatabase.CommitTransaction();
    }

    delete result;
}

const struct
{
    uint32 spell;
    uint16 flag;

} sUnitTracking[] =
{
    { 1494,     0x01 }, // beasts
    { 19879,    0x02 }, // dragonkin
    { 19878,    0x04 }, // demons
    { 19880,    0x08 }, // elementals
    { 19882,    0x10 }, // giants
    { 19884,    0x20 }, // undead
    { 19883,    0x40 }, // humanoids
    { 5225,     0x40 }, // humanoids
},
sResourceTracking[] =
{
    { 2383,     0x02 }, // herbs
    { 2580,     0x04 }, // minerals
    { 2481,     0x20 }, // treasure
};
}

namespace NamreebAnticheat
{
void AnticheatLib::Reload()
{
    if (!sAnticheatConfig.SetSource(_LIB_ANTICHEAT_CONFIG))
        sLog.outError("[Anticheat] Could not find configuration file %s.", _LIB_ANTICHEAT_CONFIG);

    // the configuration setting must be loaded before the database data because the current config settings
    // will affect how the database data is interpreted (i.e. normalization of antispam blacklist entries)
    sAnticheatConfig.loadConfigSettings();

    sLog.outString("Loading antispam system...");
    sAntispamMgr.LoadFromDB();

    sLog.outString("Loading Warden scan database...");
    sWardenScanMgr.loadFromDB();
}

void AnticheatLib::Initialize()
{
    Reload();

    // these should be loaded only on startup since they wont change without a recompile anyway
    sLog.outString("Loading scripted Warden scans...");
    Warden::LoadScriptedScans();
}

std::unique_ptr<SessionAnticheatInterface> AnticheatLib::NewSession(WorldSession *session, const BigNumber &K)
{
    if (sAnticheatConfig.EnableAnticheat())
        return std::make_unique<SessionAnticheat>(session, K);

    return std::make_unique<NullSessionAnticheat>(session);
}

bool AnticheatLib::ValidateGuildName(const std::string &name) const
{
    std::string log;
    return !!sAntispamMgr.CheckBlacklist(name, log);
}

std::string AnticheatLib::NormalizeString(const std::string &msg, uint32 mask)
{
    return sAntispamMgr.NormalizeString(msg, mask);
}

void AnticheatLib::EnableExtrapolationDebug(uint32 seconds)
{
    _extrapDebugActive = true;
    _extrapDebugTimer = WorldTimer::getMSTime() + seconds * IN_MILLISECONDS;
}

void AnticheatLib::OfferExtrapolationData(
    const MovementInfo &start, float speed1, float speed2,
    const MovementInfo &theirEnd, const Position &extrapEnd, float errorDistance)
{
    if (!_extrapDebugActive)
        return;

    std::lock_guard<std::mutex> guard(_extrapMutex);

    _extrapPoints.insert(errorDistance, { start, theirEnd, extrapEnd, speed1, speed2, errorDistance });

    if (_extrapDebugTimer < WorldTimer::getMSTime())
    {
        _extrapDebugActive = false;
        _extrapDebugTimer = 0;

        std::stringstream str;

        for (auto const &i : _extrapPoints)
        {
            str << "Start: (" << i.start.pos.x << ", " << i.start.pos.y << ", " << i.start.pos.z
                << ") o: " << i.start.pos.o << " pitch: " << i.start.s_pitch << " flags: 0x"
                << std::hex << i.start.moveFlags << std::dec << " Speed 1: " << i.speed1 << " Speed 2: " << i.speed2
                << " Client time: " << i.start.ctime << " Server time: " << i.start.stime << "\n"
                << "Their end: (" << i.theirEnd.pos.x << ", " << i.theirEnd.pos.y << ", " << i.theirEnd.pos.z
                << ") o: " << i.theirEnd.pos.o << " pitch: " << i.theirEnd.s_pitch << " flags: 0x"
                << std::hex << i.theirEnd.moveFlags << std::dec
                << " Client time: " << i.theirEnd.ctime << " Server time: " << i.theirEnd.stime << "\n"
                << "Extrapolated end position: (" << i.extrapEnd.x << ", " << i.extrapEnd.y << ", " << i.extrapEnd.z
                << ") o: " << i.extrapEnd.o << "\n"
                << "Error distance: " << i.errorDistance << "\n";
        }

        sLog.outBasic("Extrapolation debug window has ended.  Highest errors:\n%s", str.str().c_str());

        _extrapPoints.clear();
    }
}

#if 0
bool AnticheatLib::ChatCommand(ChatHandler *handler, const std::string &origArgs)
{
    std::string args;
    std::transform(origArgs.begin(), origArgs.end(), std::back_inserter(args), ::tolower);

    auto const command = SplitWord(args);

    if (command == "info")
    {
        const Player *target = nullptr;

        if (!args.empty())
            target = sObjectMgr.GetPlayer(args.c_str());

        if (!target)
        {
            auto const session = handler->GetSession();

            if (session)
            {
                if (auto const player = session->GetPlayer())
                {
                    auto const targetGuid = player->GetTargetGuid();

                    if (targetGuid.IsPlayer())
                        target = sObjectMgr.GetPlayer(targetGuid);

                    if (!target)
                        target = player;
                }
            }
        }

        if (!target)
        {
            handler->SendSysMessage("No target specified");
            return true;
        }

        if (auto const anticheat = dynamic_cast<const SessionAnticheat *>(target->GetSession()->GetAnticheat()))
        {
            handler->PSendSysMessage("Anticheat info for %s", target->GetGuidStr().c_str());
            anticheat->SendCheatInfo(handler);
        }
        else
        {
            handler->PSendSysMessage("No anticheat session for %s, they may be a bot or anticheat is disabled", target->GetGuidStr().c_str());
        }
        return true;
    }
    else if (command == "spaminfo")
    {
        const Player *target = nullptr;

        if (!args.empty())
            target = sObjectMgr.GetPlayer(args.c_str());

        if (!target)
        {
            auto const session = handler->GetSession();

            if (session)
            {
                if (auto const player = session->GetPlayer())
                {
                    auto const targetGuid = player->GetTargetGuid();

                    if (targetGuid.IsPlayer())
                        target = sObjectMgr.GetPlayer(targetGuid);

                    if (!target)
                        target = player;
                }
            }
        }

        if (!target)
        {
            handler->SendSysMessage("No target specified");
            return true;
        }

        if (auto const anticheat = dynamic_cast<const SessionAnticheat *>(target->GetSession()->GetAnticheat()))
        {
            if (auto const antispam = anticheat->GetAntispam())
                antispam->SendInfo(handler);
            else
                handler->PSendSysMessage("No antispam info available for %s", target->GetName());
        }
        else
        {
            handler->PSendSysMessage("No anticheat session for %s, they may be a bot or anticheat is disabled", target->GetGuidStr().c_str());
        }

        return true;
    }
    else if (command == "fingerprint")
    {
        // does the account have a high enough level for this command?
        if (auto const session = handler->GetSession())
            if (static_cast<uint32>(handler->GetSession()->GetSecurity()) < sAnticheatConfig.GetFingerprintLevel())
            {
                handler->PSendSysMessage("You do not have access to this command.  Rank %u is required.",
                    sAnticheatConfig.GetFingerprintLevel());
                return false;
            }

        auto const subcommand = SplitWord(args);

        if (subcommand == "list")
        {
            auto const fingerprint = SplitWord(args);
            uint32 f;

            if (fingerprint.length() > 2 && fingerprint.substr(0, 2) == "0x")
                f = static_cast<uint32>(std::stoul(fingerprint, nullptr, 16));
            else
                f = 0;

            if (!f)
            {
                handler->PSendSysMessage("No fingerprint given.  Usage: .anticheat fingerprint list <fingerprint>");
                return false;
            }

            int count = 0;

            auto const &sessions = sWorld.GetAllSessions();
            for (auto const &s : sessions)
            {
                auto const sess = s.second;
                auto const anticheat = dynamic_cast<const SessionAnticheat *>(sess->GetAnticheat());

                if (!anticheat)
                    continue;

                if (anticheat->GetFingerprint() == f)
                {
                    std::stringstream str;

                    str << "Account: " << sess->GetUsername() << " ID: " << sess->GetAccountId() << " IP: " << sess->GetRemoteAddress();

                    if (auto const player = sess->GetPlayer())
                        str << " Player name: " << player->GetName();

                    handler->SendSysMessage(str.str().c_str());

                    ++count;
                }
            }

            handler->PSendSysMessage("End of listing for fingerprint 0x%lx.  Found %d matches.", f, count);
            return true;
        }
        else if (subcommand == "history")
        {
            auto const fingerprint = SplitWord(args);
            uint32 f;

            if (fingerprint.length() > 2 && fingerprint.substr(0, 2) == "0x")
                f = static_cast<uint32>(std::stoul(fingerprint, nullptr, 16));
            else
                f = 0;

            if (!f)
            {
                handler->PSendSysMessage("No fingerprint given.  Usage: .anticheat fingerprint history <fingerprint>");
                return false;
            }

            handler->PSendSysMessage("Listing history for fingerprint 0x%lx.  Maximum history length from config: %u", f, sAnticheatConfig.GetFingerprintHistory());

            std::unique_ptr<QueryResult> result(LoginDatabase.PQuery("SELECT account, ip, realm, time FROM system_fingerprint_usage WHERE fingerprint = %u ORDER BY `time` DESC", f));

            int count = 0;
            if (result)
            {
                do
                {
                    auto const fields = result->Fetch();

                    auto const accountId = fields[0].GetUInt32();
                    auto const ip = fields[1].GetCppString();
                    auto const realm = fields[2].GetUInt32();
                    auto const time = fields[3].GetCppString();

                    handler->PSendSysMessage("Account ID: %u IP: %s Realm: %u Time: %s", accountId, ip.c_str(), realm, time.c_str());

                    ++count;
                } while (result->NextRow());
            }

            handler->PSendSysMessage("End of history for fingerprint 0x%lx.  Found %d matches", f, count);

            return true;
        }
        else if (subcommand == "ahistory")
        {
            try
            {
                auto const account = std::stoi(SplitWord(args));

                handler->PSendSysMessage("Listing history for account %u.  Maximum length: %u", account, sAnticheatConfig.GetFingerprintHistory());

                std::unique_ptr<QueryResult> result(LoginDatabase.PQuery(
                    "SELECT fingerprint, ip, realm, time FROM system_fingerprint_usage WHERE account = %u ORDER BY `time` DESC LIMIT %u",
                    account, sAnticheatConfig.GetFingerprintHistory()));

                int count = 0;
                if (result)
                {
                    do
                    {
                        auto const fields = result->Fetch();

                        auto const fingerprint = fields[0].GetUInt32();
                        auto const ip = fields[1].GetCppString();
                        auto const realm = fields[2].GetUInt32();
                        auto const time = fields[3].GetCppString();

                        handler->PSendSysMessage("Fingerprint: 0x%lx IP: %s Realm: %u Time: %s", fingerprint, ip.c_str(), realm, time.c_str());

                        ++count;
                    } while (result->NextRow());
                }

                handler->PSendSysMessage("End of history for account %u.  Found %d matches", account, count);
            }
            catch (std::logic_error const &)
            {
                handler->SendSysMessage("Failed to parse arguments");
                return false;
            }

            return true;
        }
        // theoretically we can add more commands here later
        else
        {
            handler->SendSysMessage("Unrecognized subcommand.  Valid subcommands are: list, history, ahistory");
            return false;
        }
    }
    else if (command == "cheatinform")
    {
        auto session = handler->GetSession();

        if (!session)
        {
            handler->SendSysMessage("This command is not available for the console");
            return false;
        }

        auto const flags = session->GetAccountFlags();

        if (!(flags & ACCOUNT_FLAG_SHOW_ANTICHEAT))
        {
            session->SetAccountFlags(flags | ACCOUNT_FLAG_SHOW_ANTICHEAT);
            LoginDatabase.PExecute("UPDATE account SET flags = flags | 0x%x WHERE id = %u",
                session->GetAccountId(), ACCOUNT_FLAG_SHOW_ANTICHEAT);
            handler->SendSysMessage("Anticheat messages will be shown");
        }
        else
        {
            session->SetAccountFlags(flags & ~ACCOUNT_FLAG_SHOW_ANTICHEAT);
            LoginDatabase.PExecute("UPDATE account SET flags = flags & ~0x%x WHERE id = %u",
                session->GetAccountId(), ACCOUNT_FLAG_SHOW_ANTICHEAT);
            handler->SendSysMessage("Anticheat messages will be hidden");
        }

        return true;
    }
    else if (command == "spaminform")
    {
        auto session = handler->GetSession();

        if (!session)
        {
            handler->SendSysMessage("This command is not available for the console");
            return false;
        }

        auto const flags = session->GetAccountFlags();

        if (!(flags & ACCOUNT_FLAG_SHOW_ANTISPAM))
        {
            session->SetAccountFlags(flags | ACCOUNT_FLAG_SHOW_ANTISPAM);
            LoginDatabase.PExecute("UPDATE account SET flags = flags | 0x%x WHERE id = %u",
                session->GetAccountId(), ACCOUNT_FLAG_SHOW_ANTISPAM);
            handler->SendSysMessage("Antispam messages will be shown");
        }
        else
        {
            session->SetAccountFlags(flags & ~ACCOUNT_FLAG_SHOW_ANTISPAM);
            LoginDatabase.PExecute("UPDATE account SET flags = flags & ~0x%x WHERE id = %u",
                session->GetAccountId(), ACCOUNT_FLAG_SHOW_ANTISPAM);
            handler->SendSysMessage("Antispam messages will be hidden");
        }

        return true;
    }
    else if (command == "blacklist")
    {
        // does the account have a high enough level for this command?
        if (auto const session = handler->GetSession())
            if (static_cast<uint32>(handler->GetSession()->GetSecurity()) < sAnticheatConfig.GetAntispamBlacklistLevel())
            {
                handler->PSendSysMessage("You do not have access to this command.  Rank %u is required.",
                    sAnticheatConfig.GetAntispamBlacklistLevel());
                return false;
            }

        sAntispamMgr.BlacklistAdd(args);

        handler->SendSysMessage("Blacklist add submitted");
    }
    else if (command == "silence")
    {
        try
        {
            auto const account = static_cast<uint32>(std::stoi(SplitWord(args)));

            sAntispamMgr.Silence(account);

            handler->PSendSysMessage("Silenced account %u", account);
        }
        catch (std::logic_error const &)
        {
            handler->SendSysMessage("Command parse failed.  Usage: .anticheat silence <account id>");
            return false;
        }

        return true;
    }
    else if (command == "unsilence")
    {
        try
        {
            auto const account = static_cast<uint32>(std::stoi(SplitWord(args)));

            sAntispamMgr.Unsilence(account);

            handler->PSendSysMessage("Unsilenced account %u", account);
        }
        catch (std::logic_error const &)
        {
            handler->SendSysMessage("Command parse failed.  Usage: .anticheat unsilence <account id>");
            return false;
        }

        return true;
    }
    else
    {
        handler->SendSysMessage("Unrecognized command.  Valid .anticheat commands are: info, spaminfo, fingerprint, cheatinform, spaminform, blacklist, silence, unsilence");
        return false;
    }

    return false;
}
#endif

SessionAnticheat::SessionAnticheat(WorldSession *session, const BigNumber &K) :
    _session(session), _warden(CreateWarden(session, K, this)), _inWorld(false),_fingerprint(0), _tickTimer(0),
    _cheatsReported(0), _kickTimer(0), _banTimer(0), _banAccount(false), _banIP(false), _worldEnterTime(0),
    _antispam(sAntispamMgr.GetSession(session->GetAccountId()))
{
    memset(_cheatOccuranceTick, 0, sizeof(_cheatOccuranceTick));
    memset(_cheatOccuranceTotal, 0, sizeof(_cheatOccuranceTotal));
}

SessionAnticheat::~SessionAnticheat()
{
    if (World::IsStopped())
        return;

    // when the session is destroyed we want to move the antispam information for the session into a temporary cache.
    // this way, if the session reconnects within the configured amount of time, we can restore their statistics rather than reset them.
    if (_antispam)
    {
        sAntispamMgr.CacheSession(_antispam);
        _antispam.reset();
    }

    // if the kick timer is running, kick them right away.  this is probably redundant but
    // may prove useful if they are disconnecting from this realm and connecting to another
    // with the same session id with the auth server.
    if (!!_kickTimer)
    {
        _kickTimer = 0;
        _session->KickPlayer();
    }

    // if the ban timer is running, ban them right away
    if (!!_banTimer)
    {
        _banTimer = 0;

        if (_banAccount)
            sWorld.BanAccount(BAN_ACCOUNT, _session->GetAccountName(), 0, "Cheat detected", "Anticheat");

        if (_banIP)
            sWorld.BanAccount(BAN_IP, _session->GetRemoteAddress(), 0, "Cheat detected", "Anticheat");
    }
}

void SessionAnticheat::EnterWorld()
{
    _inWorld = true;
    _worldEnterTime = WorldTimer::getMSTime();
    _movementData->HandleEnterWorld();
}

void SessionAnticheat::BeginKickTimer()
{
    // if the kick or ban timers are already running, don't restart this, otherwise repeated hacks means they'll be online forever!
    if (!_kickTimer && !_banTimer)
        _kickTimer = sAnticheatConfig.GetKickDelay();
}

void SessionAnticheat::BeginBanTimer(bool account, bool ip)
{
    // if the timer is already running, don't restart it, otherwise repeated hacks means they'll be online forever!
    if (!!_banTimer)
        return;

    _banAccount = account;
    _banIP = ip;
    _banTimer = sAnticheatConfig.GetBanDelay();
}

void SessionAnticheat::Update(uint32 diff)
{
    if (!!_kickTimer)
    {
        if (_kickTimer > diff)
            _kickTimer -= diff;
        else
        {
            _kickTimer = 0;
            _session->KickPlayer();
        }
    }

    if (!!_banTimer)
    {
        if (_banTimer > diff)
            _banTimer -= diff;
        else
        {
            _banTimer = 0;

            if (_banAccount)
                sWorld.BanAccount(BAN_ACCOUNT, _session->GetAccountName(), 0, "Cheat detected", "Anticheat");

            if (_banIP)
                sWorld.BanAccount(BAN_IP, _session->GetRemoteAddress(), 0, "Cheat detected", "Anticheat");
        }
    }

    // if the anticheat is disabled, do nothing (except enforcement of previously scheduled actions, above)
    if (!sAnticheatConfig.EnableAnticheat())
        return;

    _warden->Update(diff);

    if (_tickTimer > diff)
        _tickTimer -= diff;
    else
    {
        // reset per-tick values
        if (_movementData)
        {
            _movementData->CheckExpiredOrders(_session->GetLatency());
            _movementData->overSpeedDistanceTick = 0.f;
        }

        _tickTimer = AnticheatUpdateInterval;
        memset(_cheatOccuranceTick, 0, sizeof(_cheatOccuranceTick));
        _cheatsReported = 0;
    }
}

void SessionAnticheat::SendCharEnum(WorldPacket &&packet)
{
    _warden->SetCharEnumPacket(std::move(packet));
}

bool SessionAnticheat::IsSilenced() const
{
    return sAntispamMgr.IsSilenced(_session);
}

void SessionAnticheat::NewPlayer()
{
    auto const player = _session->GetPlayer();

    if (!player)
        _movementData.reset();
    else
        _movementData = std::make_unique<NamreebAnticheat::Movement>(player);
}

void SessionAnticheat::LeaveWorld()
{
    if (_inWorld) // disconnect case
    {
        _inWorld = false;
        _movementData->overSpeedDistanceTick = _movementData->overSpeedDistanceTotal = 0.f;
    }
}

void SessionAnticheat::Disconnect()
{
    // when the session is destroyed we want to move the antispam information for the session into a temporary cache.
    // this way, if the session reconnects within the configured amount of time, we can restore their statistics rather than reset them.

    // NOTE: we deliberately do nothing here besides caching antispam results.  this is because it is possible either
    // now or in the future that some session teardown will trigger the anticheat and we will want to still have it up and running.
    if (_antispam)
    {
        sAntispamMgr.CacheSession(_antispam);
        _antispam.reset();
    }
}

void SessionAnticheat::SendPlayerInfo(ChatHandler *handler) const
{
    auto const includeFingerprint = !handler->GetSession() ||
        static_cast<uint32>(handler->GetSession()->GetSecurity()) >= sAnticheatConfig.GetFingerprintLevel();

    if (includeFingerprint)
        handler->PSendSysMessage("OS: %s Build: %u Fingerprint: 0x%x Local IP: %s",
            _session->GetOS() == CLIENT_OS_WIN ? "Win" : "Mac", _session->GetGameBuild(), _fingerprint, _session->GetLocalAddress().c_str());

    _warden->SendPlayerInfo(handler, includeFingerprint);
}

void SessionAnticheat::SendCheatInfo(ChatHandler *handler) const
{
    // session can be reconnecting and not attached to player yet
    if (!_inWorld)
        return;

    handler->SendSysMessage("----- ANTICHEAT v3 -----");

    _movementData->SendOrderInfo(handler);

    handler->SendSysMessage("_____ Cheats detected");

    for (auto i = 0; i < CHEATS_COUNT; ++i)
        if (_cheatOccuranceTotal[i])
            handler->PSendSysMessage("%2u x %s (cheat %u)", _cheatOccuranceTotal[i], sAnticheatConfig.GetDetectorName(static_cast<CheatType>(i)), i);

    handler->SendSysMessage("_____ Extrapolation");
    handler->PSendSysMessage("Over speed distance tick = %f", _movementData->overSpeedDistanceTick);
    handler->PSendSysMessage("Over speed distance total = %f", _movementData->overSpeedDistanceTotal);

    SendPlayerInfo(handler);
}

void SessionAnticheat::SendSpamInfo(ChatHandler *handler) const
{
    if (_antispam)
        handler->SendSysMessage(_antispam->GetInfo().c_str());
}

void SessionAnticheat::RecordCheat(uint32 actionMask, const char *detector, const char *format, ...)
{
    if (actionMask == CHEAT_ACTION_NONE)
        return;

    std::string reason;

    {
        std::stringstream msg;

        msg << "Detector: " << detector;

        if (!!format && !!strlen(format))
        {
            char buff[1024];

            va_list ap;
            va_start(ap, format);
            vsnprintf(buff, sizeof(buff), format, ap);
            va_end(ap);

            msg << " Message: " << buff;
        }

        reason = msg.str();
    }

    // all cheat logging should be done here!
    if (actionMask & (CHEAT_ACTION_INFO_LOG | CHEAT_ACTION_PROMPT_LOG))
    {
        LogCheat(_session, actionMask, reason);

        // if the prompt-log flag is set, any other flags are interpreted as acceptable manual actions when prompting for user intervention
        if (actionMask & CHEAT_ACTION_PROMPT_LOG)
            return;
    }

    // permanently silence
    if (actionMask & CHEAT_ACTION_SILENCE)
    {
        if (_session->GetSecurity() == SEC_PLAYER)
        {
            // ACCOUNT FLAGS
            LoginDatabase.PExecute("UPDATE account SET flags = flags | 0x%x WHERE id = %u",
                _session->GetAccountId(), ACCOUNT_FLAG_SILENCED);

            _session->AddAccountFlag(ACCOUNT_FLAG_SILENCED);
        }
    }

    // either kick, or some combination of account and/or ip ban
    if (actionMask & CHEAT_ACTION_KICK)
        BeginKickTimer();
    else if (actionMask & (CHEAT_ACTION_BAN_ACCOUNT | CHEAT_ACTION_BAN_IP))
        BeginBanTimer(!!(actionMask & CHEAT_ACTION_BAN_ACCOUNT), !!(actionMask & CHEAT_ACTION_BAN_IP));
}

bool SessionAnticheat::Movement(MovementInfo &mi, const WorldPacket &packet)
{
    // we use this as a signal that the client has finished its world entry process.  don't bother
    // checking for any cheats yet (much of our data structure will not yet be populated).
    if (!_inWorld && packet.GetOpcode() == CMSG_SET_ACTIVE_MOVER)
    {
        EnterWorld();
        return false;
    }

    if (!_movementData)
        return false;

    if (_movementData->HandleAnticheatTests(mi, _session, packet))
        return true;

    if (_movementData->CheckTeleport(packet.GetOpcode(), mi))
        return true;

    // if the above checks do not object to the message, update some other status...
    if (mi.HasMovementFlag(MOVEFLAG_ONTRANSPORT))
    {
        auto const mover = _session->GetPlayer()->GetMover();

        if (mover->GetTypeId() == TYPEID_PLAYER)
            _movementData->OnTransport(reinterpret_cast<Player *>(mover), mi.GetTransportGuid());
    }

    return false;
}

void SessionAnticheat::TimeSkipped(const ObjectGuid &mover, uint32 ms)
{
    if (_movementData)
        _movementData->TimeSkipped(mover, ms);
}

bool SessionAnticheat::ExtrapolateMovement(MovementInfo const& mi, uint32 diffMs, Position &pos)
{
    return _movementData && _movementData->ExtrapolateMovement(mi, diffMs, pos);
}

bool SessionAnticheat::SpeedChangeAck(MovementInfo &mi, const WorldPacket &packet, float newSpeed)
{
    return !_movementData || _movementData->HandleSpeedChangeAck(mi, _session, packet, newSpeed);
}

bool SessionAnticheat::IsInKnockBack() const
{
    return _movementData && _movementData->IsInKnockBack();
}

void SessionAnticheat::KnockBack(float speedxy, float speedz, float cos, float sin)
{
    if (_movementData)
        _movementData->KnockBack(speedxy, speedz, cos, sin);
}

void SessionAnticheat::OnExplore(const AreaTableEntry *p)
{
    if (_movementData)
        _movementData->OnExplore(p);
}

void SessionAnticheat::Teleport(const Position &pos)
{
    if (_movementData)
        _movementData->HandleTeleport(pos);
}

void SessionAnticheat::OrderSent(uint16 opcode, uint32 counter)
{
    if (_movementData)
        _movementData->OrderSent(opcode, counter);
}

void SessionAnticheat::OrderAck(uint16 opcode, uint32 counter)
{
    if (_movementData)
        _movementData->OrderAck(opcode, counter);
}

void SessionAnticheat::WardenPacket(WorldPacket &packet)
{
    _warden->HandlePacket(packet);
}

void SessionAnticheat::AutoReply(const std::string &msg)
{
    if (!!_antispam && !sAntispamMgr.IsSilenced(_session))
        _antispam->AutoReply(msg);
}

void SessionAnticheat::Whisper(const std::string &msg, const ObjectGuid &to)
{
    if (!!_antispam && !sAntispamMgr.IsSilenced(_session))
        _antispam->Whisper(msg, to);
}

void SessionAnticheat::Say(const std::string &msg)
{
    if (!!_antispam && !sAntispamMgr.IsSilenced(_session))
        _antispam->Say(msg);
}

void SessionAnticheat::Yell(const std::string &msg)
{
    if (!!_antispam && !sAntispamMgr.IsSilenced(_session))
        _antispam->Yell(msg);
}

void SessionAnticheat::Channel(const std::string &msg)
{
    if (!!_antispam && !sAntispamMgr.IsSilenced(_session))
        _antispam->Channel(msg);
}

void SessionAnticheat::Mail(const std::string &subject, const std::string &body, const ObjectGuid &to)
{
    if (!!_antispam && !sAntispamMgr.IsSilenced(_session))
        _antispam->Mail(subject, body, to);
}

void SessionAnticheat::ChannelInvite(const std::string& channelName, const ObjectGuid& to)
{
    if (!!_antispam && !sAntispamMgr.IsSilenced(_session))
        _antispam->ChannelInvite(channelName, to);
}

void SessionAnticheat::PartyInvite(const ObjectGuid& to)
{
    if (!!_antispam && !sAntispamMgr.IsSilenced(_session))
        _antispam->PartyInvite(to);
}

void SessionAnticheat::RecordCheatInternal(CheatType cheat, const char *format, ...)
{
    MANGOS_ASSERT(cheat < CHEATS_COUNT);

    if (_session != nullptr)
    {
        // ACCOUNT FLAGS
        if (_session->GetAccountFlags() & ACCOUNT_FLAG_HIDDEN)
            return;
    }

    ++_cheatOccuranceTotal[cheat];
    ++_cheatOccuranceTick[cheat];

    uint32 actionMask;

    // when false, take no action
    if (!sAnticheatConfig.CheckResponse(cheat, _cheatOccuranceTick[cheat], _cheatOccuranceTotal[cheat], actionMask))
        return;

    auto constexpr activeActionMask = CHEAT_ACTION_KICK | CHEAT_ACTION_BAN_ACCOUNT | CHEAT_ACTION_BAN_IP | CHEAT_ACTION_SILENCE;

    // special handling for cheats detected by Warden to make it harder for hack and bot authors to analyze
    // if we are prompting GMs to suggest actions, then none of the other flags matter right now
    // too low level for warden non-logging ("active") actions?  remove them
    if (cheat == CHEAT_TYPE_WARDEN &&
        !(actionMask & CHEAT_ACTION_PROMPT_LOG) &&
        _session->GetAccountMaxLevel() < sAnticheatConfig.GetWardenMinimumLevel())
            actionMask &= ~activeActionMask;

    std::string comment("");

    // can be null
    if (!!format)
    {
        char buff[1024];

        va_list ap;
        va_start(ap, format);
        vsnprintf(buff, sizeof(buff), format, ap);
        va_end(ap);

        comment = buff;
    }

    // if a cheat of this type has recently been reported, do not do so again
    if (_cheatsReported & (1 << cheat))
        actionMask &= ~(CHEAT_ACTION_INFO_LOG | CHEAT_ACTION_PROMPT_LOG);
    else
        _cheatsReported |= (1 << cheat);

    // if we are performing some active action, we always should log it, since the log is ommitted from the ban message
    if (!(actionMask & CHEAT_ACTION_PROMPT_LOG) && actionMask & activeActionMask)
        actionMask |= CHEAT_ACTION_INFO_LOG;

    // TODO: delay notification until end of current anticheat tick so tick occurance can be included in the notification message
    RecordCheat(actionMask, sAnticheatConfig.GetDetectorName(cheat), comment.c_str());
}

void SessionAnticheat::CleanupFingerprintHistory() const
{
    LoginDatabase.AsyncPQuery(&CleanupFingerprintHistoryCallback, _fingerprint,
        "SELECT COUNT(*) FROM system_fingerprint_usage WHERE fingerprint = %u", _fingerprint);
}

size_t SessionAnticheat::PendingOrderCount() const
{
    return _movementData ? _movementData->PendingOrderCount() : 0;
}

float SessionAnticheat::GetDistanceTraveled() const
{
    return _movementData ? _movementData->TotalDistanceTraveled() : 0.f;
}

bool SessionAnticheat::GetMovementSpeeds(float *speeds) const
{
    if (!_movementData)
        return false;

    memcpy(speeds, _movementData->clientSpeeds, sizeof(_movementData->clientSpeeds));
    return true;
}

bool SessionAnticheat::VerifyTracking(uint16 unitTracking, uint16 resourceTracking) const
{
    // if no tracking at all is enabled, no hack
    if (!unitTracking && !resourceTracking)
        return false;

    auto const player = _session->GetPlayer();

    if (!player)
        return false;

    // ensure that only one tracking is active across both categories
    auto const unitBits = CountBits(unitTracking);
    auto const resourceBits = CountBits(resourceTracking);

    if (unitBits + resourceBits > 1)
    {
        sLog.outBasic(
            "AC Tracking hack unit tracking: 0x%x (%u) resource tracking: 0x%x (%u) player %s account %u ip %s",
            unitTracking, unitBits, resourceTracking, resourceBits, _session->GetPlayerName(), _session->GetAccountId(),
            _session->GetRemoteAddress().c_str());

        return true;
    }

    bool legit = false;

    // some single tracking is enabled.  make sure it is something the player can use
    // note that needn't bother checking if the aura is currently present.  this is a workaround
    // latency causing desync.  technically it means that someone can use a hack to track resources
    // that they could track anyway, but we don't care about that.  if the player has the spell
    // corresponding to the tracking that they have, the check is considered legitimate.  this
    // allows for multiple spell entries in sUnitTracking and sResourceTracking for the same flag.

    if (!!unitTracking)
    {
        auto constexpr unitTrackingCount = sizeof(sUnitTracking) / sizeof(sUnitTracking[0]);

        for (auto i = 0u; i < unitTrackingCount; ++i)
        {
            // if they don't have the flag, who cares
            if (!(unitTracking & sUnitTracking[i].flag))
                continue;

            // if they have the spell, it is considered legit, and stop looking
            if (player->HasSpell(sUnitTracking[i].spell))
            {
                legit = true;
                break;
            }
        }
    }
    else
    {
        auto constexpr resourceTrackingCount = sizeof(sResourceTracking) / sizeof(sResourceTracking[0]);

        for (auto i = 0u; i < resourceTrackingCount; ++i)
        {
            // if they don't have the flag, who cares
            if (!(resourceTracking & sResourceTracking[i].flag))
                continue;

            // if they have the spell, it is considered legit, and stop looking
            if (player->HasSpell(sResourceTracking[i].spell))
            {
                legit = true;
                break;
            }
        }
    }

    if (!legit)
        sLog.outBasic(
            "AC Tracking not legit. unit: 0x%x resource: 0x%x player %s account %u ip %s",
            unitTracking, resourceTracking, _session->GetPlayerName(), _session->GetAccountId(),
            _session->GetRemoteAddress().c_str());

    // if it is not legit, consider it a hack
    return !legit;
}

void SessionAnticheat::VerifyMovementFlags(uint32 flags, uint32 &removeFlags, bool strict) const
{
    removeFlags = 0;

    if (_movementData)
        _movementData->VerifyMovementFlags(flags, removeFlags, strict);
}

void SessionAnticheat::GetMovementDebugString(std::string &out) const
{
    std::stringstream ret;

    if (!_movementData)
        return;

    std::string tmp;

    _movementData->DumpOrders(tmp);
    ret << "Orders:\n" << tmp;

    _movementData->DumpAcks(tmp);
    ret << "Acks:\n" << tmp;

    _movementData->DumpTimeSkips(tmp);
    ret << "Time Skips:\n" << tmp;

    _movementData->DumpWorldChanges(tmp);
    ret << "World changes:\n" << tmp;

    _movementData->DumpMovement(tmp);
    ret << "Movement:\n" << tmp;

    out = ret.str();
}
}

AnticheatLibInterface* GetAnticheatLib()
{
    static NamreebAnticheat::AnticheatLib l;
    return &l;
}

