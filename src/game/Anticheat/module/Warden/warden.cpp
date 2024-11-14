/*
 * Copyright (C) 2017-2020 namreeb (legal@namreeb.org)
 *
 * This is private software and may not be shared under any circumstances,
 * absent permission of namreeb.
 */

#include "WardenModule.hpp"
#include "WardenKeyGeneration.h"
#include "../config.hpp"

#include "Common.h"
#include "Entities/Player.h"
#include "Server/WorldPacket.h"
#include "Server/WorldSession.h"
#include "World/World.h"
#include "Log/Log.h"
#include "Server/Opcodes.h"
#include "Util/ByteBuffer.h"
#include "Database/DatabaseEnv.h"
#include "Policies/Singleton.h"
#include "Auth/BigNumber.h"
#include "warden.hpp"
#include "WardenModuleMgr.hpp"
#include "Util/Util.h"
#include "Anticheat/module/libanticheat.hpp"
#include "WardenWin.hpp"
#include "WardenMac.hpp"
#include "WardenScanMgr.hpp"

#include <zlib.h>

#include <algorithm>
#include <memory>

using CheatType = NamreebAnticheat::CheatType;
using SessionAnticheat = NamreebAnticheat::SessionAnticheat;

void Warden::LoadScriptedScans()
{
    auto const start = sWardenScanMgr.Count();

    WardenWin::LoadScriptedScans();
    WardenMac::LoadScriptedScans();

    sLog.outBasic(">> %lu scripted Warden scans loaded from anticheat module", uint64(sWardenScanMgr.Count() - start));
}

Warden::Warden(WorldSession *session, const WardenModule *module, const BigNumber &K, SessionAnticheatInterface *anticheat) :
    _crk(nullptr), _timeoutClock(0), _scanClock(0), _moduleSendPending(false), _inputCrypto(KeyLength), _outputCrypto(KeyLength),
    _anticheat(reinterpret_cast<SessionAnticheat *>(anticheat)), _session(session), _module(module), _initialized(false)
{
    MANGOS_ASSERT(!!_module || session->GetPlatform() != CLIENT_PLATFORM_X86);
    MANGOS_ASSERT(!!_anticheat);

    auto const kBytes = K.AsByteArray(0);

    SHA1Randx WK(kBytes.data(), kBytes.size());

    uint8 inputKey[KeyLength];
    WK.Generate(inputKey, sizeof(inputKey));

    uint8 outputKey[KeyLength];
    WK.Generate(outputKey, sizeof(outputKey));

    _inputCrypto.Init(inputKey);
    _outputCrypto.Init(outputKey);

    _xor = inputKey[0];

    sLog.outDebug("WARDEN: Initializing for account %u ip %s", _session->GetAccountId(), _session->GetRemoteAddress().c_str());

    if (_module)
    {
        ByteBuffer pkt(1 + _module->hash.size() + _module->key.size() + 4);

        pkt << static_cast<uint8>(WARDEN_SMSG_MODULE_USE);

        pkt.append(&_module->hash[0], _module->hash.size());
        pkt.append(&_module->key[0], _module->key.size());

        pkt << static_cast<uint32>(_module->binary.size());

        SendPacket(pkt);

        BeginTimeoutClock();
    }
}

void Warden::RequestChallenge()
{
    MANGOS_ASSERT(!!_module && !_module->crk.empty());

    sLog.outDebug("WARDEN: Sending challenge to account %u", _session->GetAccountId());

    StopTimeoutClock();

    // select a random challenge/response/key entry
    _crk = &_module->crk[urand(0, _module->crk.size() - 1)];

    ByteBuffer pkt(1 + sizeof(_crk->seed));

    pkt << static_cast<uint8>(WARDEN_SMSG_HASH_REQUEST);
    pkt.append(_crk->seed, sizeof(_crk->seed));

    SendPacket(pkt);

    BeginTimeoutClock();
}

void Warden::HandleChallengeResponse(ByteBuffer &buff)
{
    buff.rpos(buff.wpos());

    // shouldn't happen under normal conditions
    if (!_crk)
    {
        _anticheat->RecordCheatInternal(CheatType::CHEAT_TYPE_WARDEN, "Challenge response without request being sent");
        _session->KickPlayer();
        return;
    }

    // Verify key
    if (buff.wpos() != 1 + sizeof(_crk->reply) || !!memcmp(buff.contents() + 1, _crk->reply, sizeof(_crk->reply)))
    {
        _anticheat->RecordCheatInternal(CheatType::CHEAT_TYPE_WARDEN, "Challenge response failed");
        _session->KickPlayer();
        return;
    }

    StopTimeoutClock();

    _inputCrypto.Init(_crk->clientKey);
    _outputCrypto.Init(_crk->serverKey);

    _xor = _crk->clientKey[0];

    sLog.outDebug("WARDEN: Challenge response validated.  Warden packet encryption initialized.");

    _crk = nullptr;
}

void Warden::SendModuleToClient()
{
    sLog.outDebug("WARDEN: Sending module to account %u ip %s", _session->GetAccountId(), _session->GetRemoteAddress().c_str());

    StopTimeoutClock();

    for (auto pos = 0u; pos < _module->binary.size(); )
    {
        const std::uint16_t sizeLeft = _module->binary.size() - pos;
        const std::uint16_t len = std::min(sizeLeft, static_cast<decltype(sizeLeft)>(500));

        ByteBuffer pkt(1 + 2 + len);

        pkt << static_cast<uint8>(WARDEN_SMSG_MODULE_CACHE) << len;
        pkt.append(&_module->binary[pos], len);

        SendPacket(pkt);

        pos += len;
    }

    BeginTimeoutClock();

    _moduleSendPending = true;

    sLog.outDebug("WARDEN: Module transfer complete");
}

std::vector<std::shared_ptr<const Scan>> Warden::SelectScans(ScanFlags flags) const
{
    return sWardenScanMgr.GetRandomScans(static_cast<ScanFlags>(flags | GetScanFlags()));
}

void Warden::EnqueueScans(std::vector<std::shared_ptr<const Scan>> &&scans)
{
    // append the requested scans to the queue
    _enqueuedScans.insert(_enqueuedScans.end(), scans.cbegin(), scans.cend());
}

void Warden::RequestScans(std::vector<std::shared_ptr<const Scan>> &&scans)
{
    // start by appending the requested scans to the queue, which will usually be empty when this is called
    _enqueuedScans.insert(_enqueuedScans.end(), scans.cbegin(), scans.cend());

    // if for whatever reason there is nothing to do, stop
    if (_enqueuedScans.empty())
        return;

    // if there are already some scans pending, stop
    if (!_pendingScans.empty())
        return;

    // begin building the scan request, taking as many scans from the queue as we can

    std::vector<std::string> strings;
    ByteBuffer scan;

    size_t request = 0, reply = 0;

    bool queueUpdated = false;

    for (auto i = _enqueuedScans.begin(); i != _enqueuedScans.end(); ++i)
    {
        // too many or too big? keep the current scan, as well as all following it, in the queue, and halt
        if (request + (*i)->requestSize > MaxRequest || reply + (*i)->replySize > MaxReply ||
            _pendingScans.size() >= sAnticheatConfig.GetWardenScanCount())
        {
            _enqueuedScans = std::vector<std::shared_ptr<const Scan>>(i, _enqueuedScans.end());
            queueUpdated = true;
            break;
        }

        // if we can fit this one, do it

        auto const startSize = scan.wpos();
        auto const startStringSize = strings.size();

        (*i)->Build(this, strings, scan);

        // if the scan did not change the buffer size or the string size, consider
        // it a NOP and don't bother marking it as pending
        if (scan.wpos() != startSize || strings.size() != startStringSize)
            _pendingScans.push_back(*i);
    }

    // if there are still no pending scans, it means that there is a single scan which is too big.
    // this should never happen, so if it does, just crash
    MANGOS_ASSERT(!_pendingScans.empty());

    // if the scan queue has not been updated, its because we were able to fit the entire queue into one request.
    // therefore, the queue can be emptied
    if (!queueUpdated)
        _enqueuedScans.clear();

    // this buffer is where we will combine the different portions of the packet
    ByteBuffer buff;

    // warden opcode
    buff << static_cast<uint8>(WARDEN_SMSG_CHEAT_CHECKS_REQUEST);

    if (_session->GetOS() == CLIENT_OS_WIN)
    {
        // string table for this request
        for (auto const& s : strings)
        {
            buff << static_cast<uint8>(s.length());
            buff.append(s.c_str(), s.length());
        }

        // end of string table
        buff << static_cast<uint8>(0);
    }

    // all scan requests
    buff.append(scan);

    if (_session->GetOS() == CLIENT_OS_WIN)
    {
        // indicates to the client that there are no further requests in this packet
        buff << uint8(_module->scanTerminator ^ _xor);
    }

    BeginTimeoutClock();
    SendPacket(buff);
}

void Warden::ReadScanResults(ByteBuffer &buff)
{
    // move the scans to the stack because the check handlers we call may enqueue additional scans
    auto const scans = std::move(_pendingScans);

    auto const inWorld = _session->GetPlayer() && _session->GetPlayer()->IsInWorld();

    for (auto const &s : scans)
    {
        // checks return true when they have discovered a hack 
        if (s->Check(this, buff))
        {
            // if this scan requires being in the world and they are not in the world (meaning they left
            // the world between the request and response), ignore the results
            if (!!(s->flags & InWorld) && !inWorld)
                continue;

            _anticheat->RecordCheatInternal(CheatType::CHEAT_TYPE_WARDEN, "Check \"%s\" failed", s->comment.c_str());
        }
    }

    // if there are enqueued scans at this point, the check handlers must have requested them.
    // when this happens, request them to be sent immediately.
    if (!_enqueuedScans.empty())
        RequestScans({});
}

void Warden::SendPacket(const ByteBuffer &buff)
{
    WorldPacket pkt(SMSG_WARDEN_DATA, buff.wpos());
    pkt.append(buff);

    // we specifically append the packet copy, rather than the input copy, to avoid
    // creating side-effects for this function
    EncryptData(const_cast<uint8 *>(pkt.contents()), pkt.wpos());

    _session->SendPacket(pkt, true);
}

void Warden::DecryptData(uint8* buffer, size_t size)
{
    _inputCrypto.UpdateData(buffer, size);
}

void Warden::EncryptData(uint8* buffer, size_t size)
{
    _outputCrypto.UpdateData(buffer, size);
}

void Warden::BeginTimeoutClock()
{
#ifdef _DEBUG
    _timeoutClock = 0;
#else
    // we will expect a reply eventually
    _timeoutClock = WorldTimer::getMSTime() + 1000 * sAnticheatConfig.GetWardenTimeout();
#endif
}

void Warden::StopTimeoutClock()
{
    _timeoutClock = 0;
}

void Warden::BeginScanClock()
{
    _scanClock = WorldTimer::getMSTime() + 1000 * sAnticheatConfig.GetWardenScanFrequency();
}

void Warden::StopScanClock()
{
    _scanClock = 0;
}

uint32 Warden::BuildChecksum(const uint8* data, size_t size)
{
    uint8 hash[Sha1Hash::GetLength()];
    SHA1(data, size, hash);

    uint32 checkSum = 0;

    for (auto i = 0u; i < sizeof(hash) / sizeof(uint32); ++i)
        checkSum ^= *reinterpret_cast<uint32 *>(&hash[i * 4]);

    return checkSum;
}

void Warden::HandlePacket(WorldPacket& recvData)
{
	// initialize decrypt packet
	DecryptData(const_cast<uint8*>(recvData.contents()), recvData.size());

    uint8 opcode;
    recvData >> opcode;

    // when there is a challenge/response pending, the only packet we expect is the hash result
    if (!!_crk && opcode != WARDEN_CMSG_HASH_RESULT)
    {
        _anticheat->RecordCheatInternal(CheatType::CHEAT_TYPE_WARDEN, "Unexpected opcode (%u) received while awaiting hash challenge response", opcode);
        recvData.rpos(recvData.wpos());
        _session->KickPlayer();
        return;
    }

    switch (opcode)
    {
        case WARDEN_CMSG_MODULE_MISSING:
        {
            if (!_module)
            {
                sLog.outBasic("WARDEN: Account %u IP %s requested module when none was offered.", _session->GetAccountId(), _session->GetRemoteAddress().c_str());
                _session->KickPlayer();
                break;
            }

            if (_moduleSendPending)
            {
                sLog.outBasic("WARDEN: Account %u IP %s failed to load module.  Kicking.", _session->GetAccountId(), _session->GetRemoteAddress().c_str());
                _session->KickPlayer();
                _moduleSendPending = false;
                break;
            }

            sLog.outBasic("WARDEN: Account - %u get opcode 00 - Load module failed or module is missing...", _session->GetAccountId());
            SendModuleToClient();
            break;
        }

        case WARDEN_CMSG_MODULE_OK:
        {
            if (!_module)
            {
                sLog.outBasic("WARDEN: Account %u IP %s loaded module without server request.", _session->GetAccountId(), _session->GetRemoteAddress().c_str());
                _session->KickPlayer();
                break;
            }

            _moduleSendPending = false;
            sLog.outBasic("WARDEN: Account - %u get opcode 01 - Module has loaded. Recv answer....", _session->GetAccountId());
            RequestChallenge();
            break;
        }

        case WARDEN_CMSG_CHEAT_CHECKS_RESULT:
        {
            if (_session->GetOS() == CLIENT_OS_WIN)
            {
                // verify checksum integrity
                uint16 length;
                uint32 checksum;
                recvData >> length >> checksum;

                if (length > (recvData.size() - recvData.rpos()))
                {
                    recvData.rpos(recvData.wpos());
                    _anticheat->RecordCheatInternal(CheatType::CHEAT_TYPE_WARDEN, "Packet checksum length fail");
                    _session->KickPlayer();
                    return;
                }

                if (BuildChecksum(recvData.contents() + recvData.rpos(), length) != checksum)
                {
                    recvData.rpos(recvData.wpos());
                    _anticheat->RecordCheatInternal(CheatType::CHEAT_TYPE_WARDEN, "Packet checksum fail");
                    _session->KickPlayer();
                    return;
                }
            }

            // this function will also act on the results
            ReadScanResults(recvData);

            StopTimeoutClock();
            BeginScanClock();

            break;
        }

        // FIXME: Find when/why/how this actually happens and how to handle it
        case WARDEN_CMSG_MEM_CHECKS_RESULT:
        {
            sLog.outDebug("WARDEN: Account - %u received opcode 03", _session->GetAccountId());
            break;
        }

        case WARDEN_CMSG_HASH_RESULT:
        {
            sLog.outDebug("WARDEN: Account - %u received opcode 04", _session->GetAccountId());

            HandleChallengeResponse(recvData);

            // if the challenge failed, this will still be set.  by now, the session will be closed.  do nothing further.
            if (!!_crk)
                return;

            // at this point the client has our module loaded.  send whatever packets are necessary to initialize Warden
            InitializeClient();

            // send any initial hack scans that the scan manager may have for us
            RequestScans(SelectScans(ScanFlags::InitialLogin));

            // begin the scan clock (note that even if the clock expires before any initial scans are answered, no new
            // checks will be requested until the reply is received).
            BeginScanClock();

            break;
        }

        case WARDEN_CMSG_MODULE_FAILED:
        {
            sLog.outBasic("WARDEN: Account - %u received opcode 05 - Module load failed!", _session->GetAccountId());
            _session->KickPlayer();
            break;
        }

        default:
        {
            _anticheat->RecordCheatInternal(CheatType::CHEAT_TYPE_WARDEN, "Received unknown opcode %02X of size %u.", opcode, recvData.size() - 1);
            _session->KickPlayer();
            break;
        }
    }
}

void Warden::Update(uint32 diff)
{
    if (!!_timeoutClock && WorldTimer::getMSTime() > _timeoutClock)
    {
        sLog.outBasic("WARDEN: Account %u ip %s timeout", _session->GetAccountId(), _session->GetRemoteAddress().c_str());
        _session->KickPlayer();
        return;
    }

    if (_pendingScans.empty())
    {
        // if there are enqueued scans which may now be requested, do so immediately (with no additional scans)
        if (!_enqueuedScans.empty())
            RequestScans({});
        // otherwise, if the scan clock is running and has expired, request randomly selected scans
        else if (!!_scanClock && WorldTimer::getMSTime() > _scanClock)
        {
            auto const inWorld = _session->GetPlayer() ? _session->GetPlayer()->IsInWorld() : false;

            RequestScans(SelectScans(inWorld ? InWorld : None));

            // if no scans were found, just begin the clock again, in case things change
            if (_pendingScans.empty())
                BeginScanClock();
            else
            {
                // let us keep things simple and not request checks while others are pending, but only if scans were found and requested
                StopScanClock();
                BeginTimeoutClock();
            }
        }
    }
}
