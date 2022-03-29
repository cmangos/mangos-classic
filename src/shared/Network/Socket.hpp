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

#include "PacketBuffer.hpp"

#include "Platform/Define.h"

#include <boost/asio.hpp>

#include <memory>
#include <string>
#include <mutex>
#include <functional>

namespace MaNGOS
{
    class Socket : public std::enable_shared_from_this<Socket>
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
            std::mutex m_closeMutex;
            boost::asio::deadline_timer m_outBufferFlushTimer;

            void StartAsyncRead();
            void OnRead(const boost::system::error_code &error, size_t length);

            void StartWriteFlushTimer();
            void OnWriteComplete(const boost::system::error_code &error, size_t length);
            void FlushOut();

            void OnError(const boost::system::error_code &error);

        protected:
            const std::string m_address;
            const std::string m_remoteEndpoint;
            boost::asio::ip::address m_remoteAddress;
            uint16 m_remotePort;

            virtual bool ProcessIncomingData() = 0;

            const uint8 *InPeak() const { return &m_inBuffer->m_buffer[m_inBuffer->m_readPosition]; }

            int ReadLengthRemaining() const { return m_inBuffer->ReadLengthRemaining(); }

            void ForceFlushOut();

        public:
            Socket(boost::asio::io_service &service, std::function<void (Socket *)> closeHandler);
            virtual ~Socket() = default;

            virtual bool Open();
            void Close();

            bool IsClosed() const { return !m_socket.is_open(); }
            virtual bool Deletable() const { return IsClosed(); }

            bool Read(char *buffer, int length);
            void ReadSkip(int length) { m_inBuffer->Read(nullptr, length); }

            void Write(const char *buffer, int length);
            void Write(const char *header, int headerSize, const char* content, int contentSize);

            boost::asio::ip::tcp::socket &GetAsioSocket() { return m_socket; }

            const std::string &GetRemoteEndpoint() const { return m_remoteEndpoint; }
            const std::string &GetRemoteAddress() const { return m_address; }

            template <typename T>
            std::shared_ptr<T> shared() { return std::static_pointer_cast<T>(shared_from_this()); }

            boost::asio::ip::address GetRemoteIpAddress() const { return m_remoteAddress; }
            uint16 GetRemotePort() const { return m_remotePort; }

        private:
            // custom allocator based on example from http://www.boost.org/doc/libs/1_62_0/doc/html/boost_asio/example/cpp11/allocation/server.cpp

            // Class to manage the memory to be used for handler-based custom allocation.
            // It contains a single block of memory which may be returned for allocation
            // requests. If the memory is in use when an allocation request is made, the
            // allocator delegates allocation to the global heap.
            class handler_allocator
            {
                private:
                    static const size_t BufferSize = 512;

                    // Storage space used for handler-based custom memory allocation.
                    std::aligned_storage<BufferSize>::type m_buffer;

                    // Whether the handler-based custom allocation storage has been used.
                    bool m_inUse;

                public:
                    handler_allocator() : m_inUse(false) {}

                    handler_allocator(const handler_allocator&) = delete;
                    handler_allocator& operator=(const handler_allocator&) = delete;

                    void* allocate(std::size_t size)
                    {
                        if (!m_inUse && size <= sizeof(m_buffer))
                        {
                            m_inUse = true;
                            return &m_buffer;
                        }

                        return ::operator new(size);
                    }

                    void deallocate(void* pointer)
                    {
                        if (pointer == &m_buffer)
                            m_inUse = false;
                        else
                            ::operator delete(pointer);
                    }
            };

            // Wrapper class template for handler objects to allow handler memory
            // allocation to be customised. Calls to operator() are forwarded to the
            // encapsulated handler.
            template <typename Handler>
            class custom_alloc_handler
            {
                private:
                    handler_allocator& m_allocator;
                    Handler m_handler;

                public:
                    custom_alloc_handler(handler_allocator& a, Handler h) : m_allocator(a), m_handler(h) {}

                    template <typename ...Args>
                    void operator()(Args&&... args)
                    {
                        m_handler(std::forward<Args>(args)...);
                    }

                    friend void* asio_handler_allocate(std::size_t size, custom_alloc_handler<Handler>* this_handler)
                    {
                        return this_handler->m_allocator.allocate(size);
                    }

                    friend void asio_handler_deallocate(void* pointer, std::size_t /*size*/, custom_alloc_handler<Handler>* this_handler)
                    {
                        this_handler->m_allocator.deallocate(pointer);
                    }
            };

            // Helper function to wrap a handler object to add custom allocation.
            template <typename Handler>
            static custom_alloc_handler<Handler> make_custom_alloc_handler(handler_allocator& a, Handler h)
            {
                return custom_alloc_handler<Handler>(a, h);
            }

            handler_allocator m_allocator;
    };
}

#endif /* !__SOCKET_HPP_ */
