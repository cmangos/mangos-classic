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

#ifndef _MANGOSSOAP_H
#define _MANGOSSOAP_H

#include "Common.h"
#include "World/World.h"
#include "Accounts/AccountMgr.h"
#include "Log.h"

#include "soapH.h"
#include "soapStub.h"

#include <string>
#include <thread>

class SOAPThread
{
    private:
        static const int WorkerThreads = 5;
        static const int AcceptTimeout = 3;
        static const int DataTimeout = 5;
        static const int BackLogSize = 100;

        const std::string m_host;
        const int m_port;

        std::thread m_workerThread;

        void Work();

    public:
        static const AccountTypes MinLevel = AccountTypes::SEC_ADMINISTRATOR;
        static const int CommandOutputBufferSize = 256;

        SOAPThread(const std::string& host, int port);
        ~SOAPThread();
};

#endif
