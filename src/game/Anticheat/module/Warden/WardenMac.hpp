/*
 * Copyright (C) 2017-2020 namreeb (legal@namreeb.org)
 *
 * This is private software and may not be shared under any circumstances,
 * absent permission of namreeb.
 */

#ifndef __WARDENMAC_HPP_
#define __WARDENMAC_HPP_

#include "warden.hpp"
#include "WardenScan.hpp"
#include "WorldPacket.h"

#include <openssl/md5.h>

#include <string>

class WorldSession;
class BigNumber;
class SessionAnticheatInterface;

class WardenMac final : public Warden
{
    private:
        bool _fingerprintSaved;

        std::string _hashString;
        uint8 _hashSHA[SHA_DIGEST_LENGTH];
        uint8 _hashMD5[MD5_DIGEST_LENGTH];

        WorldPacket _charEnum;

        virtual uint32 GetScanFlags() const;

        virtual void InitializeClient();

    public:
        static void LoadScriptedScans();

        WardenMac(WorldSession *session, const BigNumber &K, SessionAnticheatInterface *anticheat);

        void Update(uint32 diff);

        // set pending character enum packet (to be sent once we are satisfied that Warden is loaded)
        virtual void SetCharEnumPacket(WorldPacket &&packet);

        virtual void SendPlayerInfo(ChatHandler *handler, bool includeFingerprint) const {}
};

#endif /*!__WARDENMAC_HPP_*/