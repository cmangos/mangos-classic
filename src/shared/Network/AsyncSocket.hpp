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

#ifndef MANGOSSERVER_ASYNC_SOCKET
#define MANGOSSERVER_ASYNC_SOCKET

#include "Platform/Define.h"
#include <boost/asio.hpp>
#include <boost/enable_shared_from_this.hpp>
#include "boost/lexical_cast.hpp"
#include "Log/Log.h"

namespace MaNGOS
{
    // this socket is different in that it does not block on reads
    template <typename SocketType>
    class AsyncSocket : public std::enable_shared_from_this<SocketType>
    {
        public:
            AsyncSocket(boost::asio::io_context& io_context);
            virtual ~AsyncSocket();

            void Read(char* buffer, size_t length, std::function<void(const boost::system::error_code&, std::size_t)>&& callback);
            void ReadUntil(std::string& buffer, char delimiter, std::function<void(const boost::system::error_code&, std::size_t)>&& callback);
            void ReadSkip(size_t skipSize, std::function<void(const boost::system::error_code&, std::size_t)>&& callback);
            void Write(const char* buffer, size_t length, std::function<void(const boost::system::error_code&, std::size_t)>&& callback);

            bool Start();
            void Close()
            {
                std::lock_guard<std::mutex> guard(m_closeMutex);
                if (IsClosed())
                    return;

                boost::system::error_code ec;
                m_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
                m_socket.close();
            }
            boost::asio::ip::tcp::socket& GetAsioSocket() { return m_socket; }
            virtual bool Deletable() const { return IsClosed(); }
            bool IsClosed() const { return !m_socket.is_open(); }

            boost::asio::ip::address GetRemoteIpAddress() const { return m_remoteAddress; }
            uint16 GetRemotePort() const { return m_remotePort; }

            std::string const& GetRemoteEndpoint() const { return m_remoteEndpoint; }
            std::string const& GetRemoteAddress() const { return m_address; }
        private:
            virtual bool ProcessIncomingData() = 0;
            virtual bool OnOpen() = 0;

            boost::asio::ip::tcp::socket m_socket;

            std::mutex m_closeMutex;
            std::string m_address;
            std::string m_remoteEndpoint;
            boost::asio::ip::address m_remoteAddress;
            uint16 m_remotePort;
    };

    template <typename SocketType>
    MaNGOS::AsyncSocket<SocketType>::AsyncSocket(boost::asio::io_context& io_context) : m_socket(io_context), m_address("0.0.0.0"),
        m_remoteAddress(boost::asio::ip::address()), m_remotePort(0)
    {

    }

    template <typename SocketType>
    MaNGOS::AsyncSocket<SocketType>::~AsyncSocket()
    {
        m_socket.close();
    }

    template <typename SocketType>
    void MaNGOS::AsyncSocket<SocketType>::Read(char* buffer, size_t length, std::function<void(const boost::system::error_code&, std::size_t)>&& callback)
    {
        boost::asio::async_read(m_socket, boost::asio::buffer(buffer, length), callback);
    }

    template <typename SocketType>
    void MaNGOS::AsyncSocket<SocketType>::ReadUntil(std::string& buffer, char delimiter, std::function<void(const boost::system::error_code&, std::size_t)>&& callback)
    {
        boost::asio::async_read_until(m_socket, boost::asio::dynamic_buffer(buffer, 1024), delimiter, callback);
    }

    template<typename SocketType>
    void AsyncSocket<SocketType>::ReadSkip(size_t skipSize, std::function<void(const boost::system::error_code&, std::size_t)>&& callback)
    {
        std::shared_ptr<std::vector<uint8>> buffer = std::make_shared<std::vector<uint8>>(skipSize);
        Read(reinterpret_cast<char*>(buffer->data()), skipSize, [callback, buffer](const boost::system::error_code& error, std::size_t read)
        {
            callback(error, read);
        });
    }

    template <typename SocketType>
    void MaNGOS::AsyncSocket<SocketType>::Write(const char* buffer, size_t length, std::function<void(const boost::system::error_code&, std::size_t)>&& callback)
    {
        boost::asio::async_write(m_socket, boost::asio::buffer(buffer, length), callback);
    }

    template <typename SocketType>
    bool MaNGOS::AsyncSocket<SocketType>::AsyncSocket::Start()
    {
        try
        {
            m_address = m_socket.remote_endpoint().address().to_string();
            m_remoteEndpoint = boost::lexical_cast<std::string>(m_socket.remote_endpoint());
            m_remoteAddress = m_socket.remote_endpoint().address();
            m_remotePort = m_socket.remote_endpoint().port();
        }
        catch (boost::system::system_error& error)
        {
            sLog.outError("Socket::Open() failed to get remote address.  Error: %s", error.what());
            return false;
        }
        OnOpen();
        ProcessIncomingData();
        return true;
    }

}

#endif