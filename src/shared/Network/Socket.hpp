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

#ifndef __SOCKET_HPP_
#define __SOCKET_HPP_

#include <memory>
#include <string>
#include <mutex>
#include <functional>

#include <boost/asio.hpp>

#include "Platform/Define.h"

#include "PacketBuffer.hpp"

namespace MaNGOS
{
    class MANGOS_DLL_SPEC Socket
    {
        private:
            // buffer timeout period, in milliseconds.  higher values decrease responsiveness
            // ingame but increase bandwidth efficiency by reducing tcp overhead.
            static const int BufferTimeout = 50;

            enum class WriteState
            {
                Idle,       // no write operation is currently underway
                Buffering,  // a write operation has been performed, and we are currently awaiting others before sending
                Sending,    // a send operation is underway
            };

            enum class ReadState
            {
                Idle,
                Reading
            };

            WriteState m_writeState;
            ReadState m_readState;

            boost::asio::ip::tcp::socket m_socket;

            std::function<void(Socket *)> m_closeHandler;

            std::unique_ptr<PacketBuffer> m_inBuffer;
            std::unique_ptr<PacketBuffer> m_outBuffer;
            std::unique_ptr<PacketBuffer> m_secondaryOutBuffer;

            std::mutex m_mutex;
            boost::asio::deadline_timer m_outBufferFlushTimer;

            void StartAsyncRead();
            void OnRead(const boost::system::error_code &error, size_t length);

            void StartWriteFlushTimer();
            void OnWriteComplete(const boost::system::error_code &error, size_t length);
            void FlushOut();

            void OnError(const boost::system::error_code &error);

        protected:
            const std::string m_address;

            virtual bool ProcessIncomingData() = 0;

            const uint8 *InPeak() const { return &m_inBuffer->m_buffer[m_inBuffer->m_readPosition]; }

            int ReadLengthRemaining() const { return m_inBuffer->ReadLengthRemaining(); }

            void ForceFlushOut();

        public:
            Socket(boost::asio::io_service &service, std::function<void (Socket *)> closeHandler);
            ~Socket() { assert(Deletable()); }

            virtual bool Open();
            void Close();

            bool IsClosed() const { return !m_socket.is_open(); }
            virtual bool Deletable() const { return IsClosed() && m_writeState == WriteState::Idle && m_readState == ReadState::Idle; }

            bool Read(char *buffer, int length);
            void ReadSkip(int length) { m_inBuffer->Read(nullptr, length); }

            void Write(const char *buffer, int length);

            boost::asio::ip::tcp::socket &GetAsioSocket() { return m_socket; }

            const std::string &GetRemoteAddress() const { return m_address; }
    };
}

#endif /* !__SOCKET_HPP_ */