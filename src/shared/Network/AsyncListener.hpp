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

#ifndef MANGOSSERVER_ASYNC_LISTENER
#define MANGOSSERVER_ASYNC_LISTENER

#include "Platform/Define.h"
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include "AsyncSocket.hpp"

namespace MaNGOS
{
    template <typename SocketType>
    class AsyncListener
    {
        public:
            // constructor for accepting connection from client
            AsyncListener(boost::asio::io_context& io_context, std::string const& bindIp, unsigned short port) : m_context(io_context), m_acceptor(io_context, boost::asio::ip::tcp::endpoint(boost::asio::ip::make_address(bindIp), port))
            {
                startAccept();
            }
            void HandleAccept(std::shared_ptr<SocketType> connection, const boost::system::error_code& err)
            {
                if (!err)
                    connection->Start();

                startAccept();
            }
        private:
            boost::asio::io_context& m_context;
            boost::asio::ip::tcp::acceptor m_acceptor;
            void startAccept()
            {
                // socket
                std::shared_ptr<SocketType> connection = std::make_shared<SocketType>(m_context);

                // asynchronous accept operation and wait for a new connection.
                m_acceptor.async_accept(connection->GetAsioSocket(), boost::bind(&AsyncListener::HandleAccept, this, connection, boost::asio::placeholders::error));
            }
    };
}

#endif