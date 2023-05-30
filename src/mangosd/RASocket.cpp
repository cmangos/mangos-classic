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

/** \file
    \ingroup mangosd
*/

#include "Common.h"
#include "Database/DatabaseEnv.h"
#include "Log.h"
#include "RASocket.h"
#include "World/World.h"
#include "Config/Config.h"
#include "Util/Util.h"
#include "Accounts/AccountMgr.h"
#include "Tools/Language.h"
#include "Globals/ObjectMgr.h"
#include "Policies/Lock.h"

#include <utility>
#include <vector>
#include <string>

/// RASocket constructor
RASocket::RASocket(boost::asio::io_service& service, std::function<void(Socket*)> closeHandler) :
    MaNGOS::Socket(service, std::move(closeHandler)), m_secure(sConfig.GetBoolDefault("RA.Secure", true)),
    m_authLevel(AuthLevel::None), m_accountLevel(AccountTypes::SEC_PLAYER), m_accountId(0)
{
    if (sConfig.IsSet("RA.Stricted"))
    {
        sLog.outError("Deprecated config option RA.Stricted being used.  Use RA.Restricted instead.");
        m_restricted = sConfig.GetBoolDefault("RA.Stricted", true);
    }
    else
        m_restricted = sConfig.GetBoolDefault("RA.Restricted", true);
}

/// RASocket destructor
RASocket::~RASocket()
{
    sLog.outRALog("Connection was closed.");
}

/// Accept an incoming connection
bool RASocket::Open()
{
    if (!Socket::Open())
        return false;

    sLog.outRALog("Incoming connection from %s.", m_address.c_str());

    ///- print Motd
    Send(sWorld.GetMotd());
    Send("\r\n");
    Send(sObjectMgr.GetMangosStringForDbcLocale(LANG_RA_USER));

    return true;
}

/// Read data from the network
bool RASocket::ProcessIncomingData()
{
    DEBUG_LOG("RASocket::ProcessIncomingData");

    std::string buffer;
    buffer.resize(ReadLengthRemaining());
    Read(&buffer[0], buffer.size());

    static const std::string NEWLINE = "\n\r";

    auto pos = 0;

    while (pos != std::string::npos)
    {
        auto newLine = buffer.find_first_of(NEWLINE, pos);

        m_input += buffer.substr(pos, newLine - pos);

        pos = buffer.find_first_not_of(NEWLINE, newLine);

        if (newLine == std::string::npos)
            break;

        if (!HandleInput())
            return false;
    }

    return true;
}

bool RASocket::HandleInput()
{
    auto const minLevel = static_cast<AccountTypes>(sConfig.GetIntDefault("RA.MinLevel", AccountTypes::SEC_ADMINISTRATOR));

    switch (m_authLevel)
    {
        /// <ul> <li> If the input is '<username>'
        case AuthLevel::None:
        {
            m_accountId = sAccountMgr.GetId(m_input);

            ///- If the user is not found, deny access
            if (!m_accountId)
            {
                Send("-No such user.\r\n");
                sLog.outRALog("User %s does not exist.", m_input.c_str());

                if (m_secure)
                    return false;

                Send("\r\n");
                Send(sObjectMgr.GetMangosStringForDbcLocale(LANG_RA_USER));
                break;
            }

            m_accountLevel = sAccountMgr.GetSecurity(m_accountId);

            ///- if gmlevel is too low, deny access
            if (m_accountLevel < minLevel)
            {
                Send("-Not enough privileges.\r\n");
                sLog.outRALog("User %s has no privilege.", m_input.c_str());

                if (m_secure)
                    return false;

                Send("\r\n");
                Send(sObjectMgr.GetMangosStringForDbcLocale(LANG_RA_USER));
                break;
            }

            ///- allow by remotely connected admin use console level commands dependent from config setting
            if (m_accountLevel >= SEC_ADMINISTRATOR && !m_restricted)
                m_accountLevel = SEC_CONSOLE;

            m_authLevel = AuthLevel::HaveUsername;
            Send(sObjectMgr.GetMangosStringForDbcLocale(LANG_RA_PASS));
            break;
        }
        ///<li> If the input is '<password>' (and the user already gave his username)
        case AuthLevel::HaveUsername:
        {
            // login+pass ok
            if (sAccountMgr.CheckPassword(m_accountId, m_input))
            {
                m_authLevel = AuthLevel::Authenticated;

                Send("+Logged in.\r\n");
                sLog.outRALog("User account %u has logged in.", m_accountId);
                Send("mangos>");
            }
            else
            {
                ///- Else deny access
                Send("-Wrong pass.\r\n");
                sLog.outRALog("User account %u has failed to log in.", m_accountId);

                if (m_secure)
                    return false;

                Send("\r\n");
                Send(sObjectMgr.GetMangosStringForDbcLocale(LANG_RA_PASS));
            }
            break;
        }
        ///<li> If user is logged, parse and execute the command
        case AuthLevel::Authenticated:
        {
            if (m_input.length())
            {
                sLog.outRALog("Got '%s' cmd.", m_input.c_str());

                if (m_input == "quit")
                    return false;

                sWorld.QueueCliCommand(new CliCommandHolder(m_accountId, m_accountLevel, m_input.c_str(),
                [this](const char* buffer) { this->Send(buffer); },
                [this](bool) { this->Send("mangos>"); }));
            }
            else
                Send("mangos>");

            break;
        }

        default:
            return false;
            ///</ul>
    }

    m_input.clear();

    return true;
}

void RASocket::Send(const std::string& message)
{
    Write(message.c_str(), message.length());
}
