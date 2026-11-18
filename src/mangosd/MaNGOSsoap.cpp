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

#include <atomic>
#include <memory>
#include <string>

SOAPThread::SOAPThread(const std::string& host, int port) : m_host(host), m_port(port)
{
    soap_init(&m_soap);
    soap_set_imode(&m_soap, SOAP_C_UTFSTRING);
    soap_set_omode(&m_soap, SOAP_C_UTFSTRING);

    m_soap.accept_timeout = AcceptTimeout;
    m_soap.recv_timeout = DataTimeout;
    m_soap.send_timeout = DataTimeout;
}

std::unique_ptr<SOAPThread> SOAPThread::Create(const std::string& host, int port)
{
    std::unique_ptr<SOAPThread> soapThread(new SOAPThread(host, port));
    if (soap_bind(&soapThread->m_soap, host.c_str(), port, BackLogSize) < 0)
    {
        sLog.outError("MaNGOSsoap: couldn't bind to %s:%d", host.c_str(), port);
        return nullptr;
    }

    sLog.outString("MaNGOSsoap: bound to http://%s:%d", host.c_str(), port);
    return soapThread;
}

void SOAPThread::Start()
{
    m_workerThread = std::thread(&SOAPThread::Work, this);
}

SOAPThread::~SOAPThread()
{
    sLog.outError("SOAP shutting down");
    if (m_workerThread.joinable())
        m_workerThread.join();

    soap_end(&m_soap);
    soap_done(&m_soap);
}

void SOAPThread::Work()
{
    while (!World::IsStopped())
    {
        auto s = soap_accept(&m_soap);

        // timeout?
        if (s == SOAP_INVALID_SOCKET)
            continue;

        DEBUG_LOG("MaNGOSsoap: accepted connection from IP=%d.%d.%d.%d", (int)(m_soap.ip >> 24) & 0xFF, (int)(m_soap.ip >> 16) & 0xFF, (int)(m_soap.ip >> 8) & 0xFF, (int)m_soap.ip & 0xFF);

        auto copy = soap_copy(&m_soap);
        soap_serve(copy);
        soap_destroy(copy); // deserialized C++ objects
        soap_end(copy);     // temporary data
        soap_free(copy);    // the copied context itself (allocated by soap_copy)
    }
}

/*
Code used for generating stubs:

int ns1__executeCommand(char* command, char** result);
*/
// 50 ms waits for a queued command once the world has stopped, before the request gives up
static const int StoppedGraceWaits = 20;

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

    // shared with the callbacks: the world thread can still run the command after this request gave up waiting
    struct CommandState
    {
        std::vector<char> buffer;
        std::atomic<bool> executed{false};
        bool succeeded = false;
    };
    auto const state = std::make_shared<CommandState>();
    state->buffer.reserve(SOAPThread::CommandOutputBufferSize);

    // commands are executed in the world thread. We have to wait for them to be completed
    sWorld.QueueCliCommand(new CliCommandHolder(accountId, SEC_CONSOLE, command,
                           [state](const char* output)
    {
        assert(output);

        for (auto p = output; *p; ++p)
            state->buffer.push_back(*p);
    },
    [state](bool success)
    {
        state->succeeded = success;
        state->executed = true;
    }));

    // the world thread runs queued commands only until the world stops. A request accepted around
    // the stop would otherwise wait forever, and the shutdown would hang joining this thread.
    // The grace lets a command that stops the world itself (server exit) still send its reply
    int stoppedWaits = 0;
    while (!state->executed)
    {
        if (World::IsStopped() && ++stoppedWaits > StoppedGraceWaits)
            return soap_sender_fault(soap, "Server is shutting down", "The command was not run, or ran without a reply, because the server is shutting down");

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    state->buffer.push_back(0);
    auto const printBuffer = soap_strdup(soap, &state->buffer[0]);

    if (!state->succeeded)
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
