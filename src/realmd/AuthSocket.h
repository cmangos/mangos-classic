/*
 * This file is part of the CMaNGOS Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/// \addtogroup realmd
/// @{
/// \file

#ifndef _AUTHSOCKET_H
#define _AUTHSOCKET_H

#include "Common.h"
#include "Auth/BigNumber.h"
#include "Auth/Sha1.h"
#include "ByteBuffer.h"

#include "Network/Socket.hpp"

#include <boost/asio.hpp>

#include <functional>

#define HMAC_RES_SIZE 20

class AuthSocket : public MaNGOS::Socket
{
    public:
        const static int s_BYTE_SIZE = 32;

        AuthSocket(boost::asio::io_service& service, std::function<void (Socket*)> closeHandler);

        void SendProof(Sha1Hash sha);
        void LoadRealmlist(ByteBuffer& pkt, uint32 acctid);
        int32 generateToken(char const* b32key);

        bool VerifyVersion(uint8 const* a, int32 aLength, uint8 const* versionProof, bool isReconnect);
        bool _HandleLogonChallenge();
        bool _HandleLogonProof();
        bool _HandleReconnectChallenge();
        bool _HandleReconnectProof();
        bool _HandleRealmList();
        // data transfer handle for patch

        bool _HandleXferResume();
        bool _HandleXferCancel();
        bool _HandleXferAccept();

        void _SetVSFields(const std::string& rI);

    private:
        enum eStatus
        {
            STATUS_CHALLENGE,
            STATUS_LOGON_PROOF,
            STATUS_RECON_PROOF,
            STATUS_PATCH,      // unused in CMaNGOS
            STATUS_AUTHED,
            STATUS_CLOSED
        };

        BigNumber N, s, g, v;
        BigNumber b, B;
        BigNumber K;
        BigNumber _reconnectProof;

        eStatus _status;

        std::string _login;
        std::string _safelogin;
        std::string _token;
        std::string m_os;

        // Since GetLocaleByName() is _NOT_ bijective, we have to store the locale as a string. Otherwise we can't differ
        // between enUS and enGB, which is important for the patch system
        std::string _localizationName;
        uint16 _build;
        AccountTypes _accountSecurityLevel;

        virtual bool ProcessIncomingData() override;
};
#endif
/// @}
