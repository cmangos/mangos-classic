/*
 * Copyright (C) 2017-2020 namreeb (legal@namreeb.org)
 *
 * This is private software and may not be shared under any circumstances,
 * absent permission of namreeb.
 */

#include "WardenMac.hpp"
#include "WardenModuleMgr.hpp"
#include "WardenScanMgr.hpp"
#include "Server/WorldSession.h"
#include "Auth/BigNumber.h"
#include "Auth/Sha1.h"
#include "World/World.h"
#include "../config.hpp"
#include "Log.h"

#include <openssl/md5.h>

#include <memory>
#include <vector>
#include <string>
#include <sstream>

void WardenMac::LoadScriptedScans()
{
    sWardenScanMgr.AddMacScan(std::make_shared<MacScan>(
    // builder
    [](const Warden *warden, std::vector<std::string> &, ByteBuffer &scan)
    {
        auto const macWarden = reinterpret_cast<const WardenMac *>(warden);

        MANGOS_ASSERT(macWarden->_hashString.size() <= 0xFF);

        scan << static_cast<uint8>(WARDEN_SMSG_CHEAT_CHECKS_REQUEST)
             << static_cast<uint8>(macWarden->_hashString.size());

        // skip null terminator this way
        scan.append(macWarden->_hashString.c_str(), macWarden->_hashString.size());
    },
    // checker
    [](const Warden *warden, ByteBuffer &buff)
    {
        auto const macWarden = reinterpret_cast<const WardenMac *>(warden);

        uint8 sha[SHA_DIGEST_LENGTH];
        uint8 md5[MD5_DIGEST_LENGTH];

        buff.read(sha, sizeof(sha));
        buff.read(md5, sizeof(md5));

        return !!memcmp(sha, macWarden->_hashSHA, sizeof(sha)) || !!memcmp(md5, macWarden->_hashMD5, sizeof(md5));
    }, 128, sizeof(uint8) + SHA_DIGEST_LENGTH + MD5_DIGEST_LENGTH, "Mac string hash check"));
}

WardenMac::WardenMac(WorldSession *session, const BigNumber &K, SessionAnticheatInterface *anticheat)
    : _fingerprintSaved(false), Warden(session, sWardenModuleMgr.GetMacModule(), K, anticheat)
{
    std::stringstream hash;

    // the only known capability of the Mac Warden module is hashing a string and sending back the hashed value
    // so at least we can make the string that we ask it to hash change by account, I guess...
    hash << "namreeb was here.  please be good.  your username is " << _session->GetAccountName();

    _hashString = hash.str();

    static constexpr uint32 magic = 0xFEEDFACE;

    Sha1Hash sha1;
    sha1.UpdateData(_hashString);
    sha1.UpdateData(reinterpret_cast<const uint8 *>(&magic), sizeof(magic));
    sha1.Finalize();

    memcpy(_hashSHA, sha1.GetDigest(), sizeof(_hashSHA));

    MD5_CTX md5;
    MD5_Init(&md5);
    MD5_Update(&md5, _hashString.c_str(), _hashString.size());
    MD5_Final(_hashMD5, &md5);
}

void WardenMac::Update(uint32 diff)
{
    Warden::Update(diff);

    if (!_initialized)
        return;

    if (!_fingerprintSaved)
    {
        // XXX FINGERPRINT LOGGING
        //LoginDatabase.BeginTransaction();

        //static SqlStatementID fingerprintUpdate;

        //auto stmt = LoginDatabase.CreateStatement(fingerprintUpdate,
        //    "INSERT INTO system_fingerprint_usage (fingerprint, account, ip, realm) VALUES(?, ?, ?, ?)");

        //stmt.addUInt32(m_anticheat->GetFingerprint());
        //stmt.addUInt32(_session->GetAccountId());
        //stmt.addString(_session->GetRemoteAddress());
        //stmt.addUInt32(realmID);
        //stmt.Execute();

        //LoginDatabase.CommitTransaction();

        _fingerprintSaved = true;

        // at this point if we have the character enum packet, it is okay to send
        if (!_charEnum.empty())
        {
            _session->SendPacket(_charEnum, true);
            _charEnum.clear();
        }
    }
}

void WardenMac::SetCharEnumPacket(WorldPacket &&packet)
{
    // if we have already recorded system information, send the packet immediately.  otherwise delay
    if (_initialized)
        _session->SendPacket(packet, true);
    else
        _charEnum = std::move(packet);
}

uint32 WardenMac::GetScanFlags() const
{
    uint32 ret = ScanFlags::MacAllBuild;

    if (_session->GetAccountMaxLevel() >= sAnticheatConfig.GetWardenMinimumAdvancedLevel())
        ret |= ScanFlags::AdvancedScan;

    return ret;
}

void WardenMac::InitializeClient()
{
    _initialized = true;
}
