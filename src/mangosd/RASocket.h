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

/// \addtogroup mangosd
/// @{
/// \file

#ifndef _RASOCKET_H
#define _RASOCKET_H

#include "Common.h"

#include "Network/Socket.hpp"

#include <functional>
#include <vector>
#include <string>

/// Remote Administration socket
class RASocket : public MaNGOS::Socket
{
    private:
        enum AuthLevel
        {
            None,
            HaveUsername,
            Authenticated
        };

        const bool m_secure;
        bool m_restricted;

        std::string m_input;

        AuthLevel m_authLevel;
        AccountTypes m_accountLevel;
        uint32 m_accountId;

        virtual bool ProcessIncomingData() override;
        bool HandleInput();
        void Send(const std::string& message);

    public:
        RASocket(boost::asio::io_service& service, std::function<void (Socket*)> closeHandler);
        virtual ~RASocket();

        virtual bool Open() override;
};
#endif
/// @}
