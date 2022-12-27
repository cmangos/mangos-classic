/*
 * Copyright (C) 2017-2020 namreeb (legal@namreeb.org)
 *
 * This is private software and may not be shared under any circumstances,
 * absent permission of namreeb.
 */

#ifndef __WARDEN_HPP_
#define __WARDEN_HPP_

#include "../libanticheat.hpp"
#include "WardenScan.hpp"

#include "Common.h"
#include "Util/ByteBuffer.h"
#include "Auth/SARC4.h"

#include <vector>
#include <memory>

enum WardenOpcodes
{
    // Client->Server
    WARDEN_CMSG_MODULE_MISSING                  = 0,
    WARDEN_CMSG_MODULE_OK                       = 1,
    WARDEN_CMSG_CHEAT_CHECKS_RESULT             = 2,
    WARDEN_CMSG_MEM_CHECKS_RESULT               = 3,        // only sent if MEM_CHECK bytes doesn't match
    WARDEN_CMSG_HASH_RESULT                     = 4,
    WARDEN_CMSG_MODULE_FAILED                   = 5,        // this is sent when client failed to load uploaded module due to cache fail

    // Server->Client
    WARDEN_SMSG_MODULE_USE                      = 0,
    WARDEN_SMSG_MODULE_CACHE                    = 1,
    WARDEN_SMSG_CHEAT_CHECKS_REQUEST            = 2,
    WARDEN_SMSG_MODULE_INITIALIZE               = 3,
    WARDEN_SMSG_MEM_CHECKS_REQUEST              = 4,        // byte len; whole(!EOF) { byte unk(1); byte index(++); string module(can be 0); int offset; byte len; byte[] bytes_to_compare[len]; }
    WARDEN_SMSG_HASH_REQUEST                    = 5
};

class WorldSession;
class WardenModule;
class BigNumber;
class Scan;
class ChatHandler;
struct ChallengeResponseEntry;

class Warden
{
    private:
        const ChallengeResponseEntry *_crk;

        // when non-zero, if the current time passes this value, kick for inactivity
        uint32 _timeoutClock;

        // when non-zero, if the current time passes this value, perform hack scans
        uint32 _scanClock;

        // true when we have sent a module to the client and are waiting for a result
        bool _moduleSendPending;

        SARC4 _inputCrypto;
        SARC4 _outputCrypto;

        void EncryptData(uint8* buffer, size_t size);
        void DecryptData(uint8* buffer, size_t size);

        void BeginTimeoutClock();
        void StopTimeoutClock();

        void StopScanClock();

        void SendModuleToClient();

        virtual void InitializeClient() = 0;

        void RequestChallenge();
        void HandleChallengeResponse(ByteBuffer &buff);

        void ReadScanResults(ByteBuffer &buff);

    protected:
        Warden(WorldSession *session, const WardenModule *module, const BigNumber &K, SessionAnticheatInterface *anticheat);

        // RC4 key length for packet encryption
        static constexpr size_t KeyLength = 16;

        virtual uint32 GetScanFlags() const = 0;
        std::vector<std::shared_ptr<const Scan>> SelectScans(ScanFlags flags) const;

        void BeginScanClock();

        // enqueue, but do not immediately send scans.  this is useful if we anticipate wanting to
        // send multiple scans and can wait to send them all at once.
        void EnqueueScans(std::vector<std::shared_ptr<const Scan>> &&scans);

        // enqueue scans, and send them immediately if possible
        void RequestScans(std::vector<std::shared_ptr<const Scan>> &&scans);

        void SendPacket(const ByteBuffer &buff);

        static uint32 BuildChecksum(const uint8* data, size_t size);

        NamreebAnticheat::SessionAnticheat * const _anticheat;

        WorldSession *const _session;

        const WardenModule *const _module;

        uint8 _xor;

        // true when the client has been confirmed and sent any initialization packet(s)
        bool _initialized;

        std::vector<std::shared_ptr<const Scan>> _pendingScans;
        std::vector<std::shared_ptr<const Scan>> _enqueuedScans;

    public:
        virtual ~Warden() = default;

        // size, in bytes, of client request buffer
        static constexpr size_t MaxRequest = 256;

        // size, in bytes, of client reply buffer
        static constexpr size_t MaxReply = 256;

        static void LoadScriptedScans();

        void HandlePacket(WorldPacket & recvData);

        const WardenModule *GetModule() const { return _module; }
        uint8 GetXor() const { return _xor; }

        virtual void Update(uint32 diff);

        // set pending character enum packet (to be sent once we are satisfied that Warden is loaded)
        virtual void SetCharEnumPacket(WorldPacket &&packet) = 0;

        virtual void SendPlayerInfo(ChatHandler *handler, bool includeFingerprint) const = 0;
};

#endif /*!__WARDEN_HPP_*/
