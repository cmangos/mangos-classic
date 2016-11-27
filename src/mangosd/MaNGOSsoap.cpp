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

#include "MaNGOSsoap.h"

#include <string>

SOAPThread::SOAPThread(const std::string &host, int port) : m_host(host), m_port(port), m_workerThread(&SOAPThread::Work, this) {}

SOAPThread::~SOAPThread()
{
    sLog.outError("SOAP shutting down");
    m_workerThread.join();
}

void SOAPThread::Work()
{
    soap soap;

    soap_init(&soap);
    soap_set_imode(&soap, SOAP_C_UTFSTRING);
    soap_set_omode(&soap, SOAP_C_UTFSTRING);

    soap.accept_timeout = AcceptTimeout;
    soap.recv_timeout = DataTimeout;
    soap.send_timeout = DataTimeout;

    if (soap_bind(&soap, m_host.c_str(), m_port, BackLogSize) < 0)
    {
        sLog.outError("MaNGOSsoap: couldn't bind to %s:%d", m_host.c_str(), m_port);
        exit(-1);
    }

    sLog.outString("MaNGOSsoap: bound to http://%s:%d", m_host.c_str(), m_port);

    while (!World::IsStopped())
    {
        auto s = soap_accept(&soap);

        // timeout?
        if (s == SOAP_INVALID_SOCKET)
            continue;

        DEBUG_LOG("MaNGOSsoap: accepted connection from IP=%d.%d.%d.%d", (int)(soap.ip >> 24) & 0xFF, (int)(soap.ip >> 16) & 0xFF, (int)(soap.ip >> 8) & 0xFF, (int)soap.ip & 0xFF);

        auto copy = soap_copy(&soap);
        soap_serve(copy);
        soap_destroy(copy);
    }

    soap_end(&soap);
    soap_done(&soap);
}

/*
Code used for generating stubs:

int ns1__executeCommand(char* command, char** result);
*/
int ns1__executeCommand(soap* soap, char* command, char** result)
{
    // security check
    if (!soap->userid || !soap->passwd)
    {
        DEBUG_LOG("MaNGOSsoap: Client didn't provide login information");
        return 401;
    }

    auto const accountId = sAccountMgr.GetId(soap->userid);
    if (!accountId)
    {
        DEBUG_LOG("MaNGOSsoap: Client used invalid username '%s'", soap->userid);
        return 401;
    }

    if (!sAccountMgr.CheckPassword(accountId, soap->passwd))
    {
        DEBUG_LOG("MaNGOSsoap: invalid password for account '%s'", soap->userid);
        return 401;
    }

    if (sAccountMgr.GetSecurity(accountId) < SOAPThread::MinLevel)
    {
        DEBUG_LOG("MaNGOSsoap: %s's gmlevel is too low", soap->userid);
        return 403;
    }

    if (!command || !*command)
        return soap_sender_fault(soap, "Command mustn't be empty", "The supplied command was an empty string");

    DEBUG_LOG("MaNGOSsoap: got command '%s'", command);

    bool commandExecuted = false, commandSucceeded = false;
    std::vector<char> buffer;
    buffer.reserve(SOAPThread::CommandOutputBufferSize);

    // commands are executed in the world thread. We have to wait for them to be completed
    sWorld.QueueCliCommand(new CliCommandHolder(accountId, SEC_CONSOLE, command,
        [&buffer] (const char *output)
        {
            assert(output);

            for (auto p = output; *p; ++p)
                buffer.push_back(*p);
        },
        [soap, &commandExecuted, &commandSucceeded] (bool success)
        {
            commandExecuted = true;
            commandSucceeded = success;
        }));

    while (!commandExecuted)
        std::this_thread::sleep_for(std::chrono::milliseconds(50));

    buffer.push_back(0);
    auto const printBuffer = soap_strdup(soap, &buffer[0]);

    if (!commandSucceeded)
    {
        auto ret = soap_sender_fault(soap, printBuffer, printBuffer);

        return ret;
    }

    *result = printBuffer;
    return SOAP_OK;
}

////////////////////////////////////////////////////////////////////////////////
//
//  Namespace Definition Table
//
////////////////////////////////////////////////////////////////////////////////

struct Namespace namespaces[] =
{
    { "SOAP-ENV", "http://schemas.xmlsoap.org/soap/envelope/" }, // must be first
    { "SOAP-ENC", "http://schemas.xmlsoap.org/soap/encoding/" }, // must be second
    { "xsi", "http://www.w3.org/1999/XMLSchema-instance", "http://www.w3.org/*/XMLSchema-instance" },
    { "xsd", "http://www.w3.org/1999/XMLSchema",          "http://www.w3.org/*/XMLSchema" },
    { "ns1", "urn:MaNGOS" },     // "ns1" namespace prefix
    { nullptr, nullptr }
};
