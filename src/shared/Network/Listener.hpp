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

#ifndef __LISTENER_HPP_
#define __LISTENER_HPP_

#include "NetworkThread.hpp"

#include <boost/asio.hpp>

#include <memory>
#include <thread>
#include <vector>

namespace MaNGOS
{
    template <typename SocketType>
    class Listener
    {
        private:
            boost::asio::io_service m_service;
            boost::asio::ip::tcp::acceptor m_acceptor;

            std::thread m_acceptorThread;
            std::vector<std::unique_ptr<NetworkThread<SocketType>>> m_workerThreads;

            // the time in milliseconds to sleep a worker thread at the end of each tick
            const int SleepInterval = 100;

            NetworkThread<SocketType> *SelectWorker() const
            {
                int minIndex = 0;
                size_t minSize = m_workerThreads[minIndex]->Size();

                for (size_t i = 1; i < m_workerThreads.size(); ++i)
                {
                    const size_t size = m_workerThreads[i]->Size();

                    if (size < minSize)
                    {
                        minSize = size;
                        minIndex = i;
                    }
                }

                return m_workerThreads[minIndex].get();
            }

            void BeginAccept();
            void OnAccept(NetworkThread<SocketType> *worker, std::shared_ptr<SocketType> const& socket, const boost::system::error_code &ec);

        public:
            Listener(std::string const& address, int port, int workerThreads);
            ~Listener();
    };

    template <typename SocketType>
    Listener<SocketType>::Listener(std::string const& address, int port, int workerThreads)
    : m_service(), m_acceptor(m_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string(address), port))
    {
        m_workerThreads.reserve(workerThreads);
        for (auto i = 0; i < workerThreads; ++i)
            m_workerThreads.push_back(std::unique_ptr<NetworkThread<SocketType>>(new NetworkThread<SocketType>));

        BeginAccept();

        m_acceptorThread = std::thread([this]() { m_service.run(); });
    }

    template <typename SocketType>
    Listener<SocketType>::~Listener()
    {
        // Close the acceptor. This will cancel any asynchronous accept
        // operation and should stop the acceptor thread. Note that closing
        // the acceptor needs to be done in the acceptor thread, because
        // using the m_acceptor object from multiple threads is unsafe!
        m_service.post( [this]() { m_acceptor.close(); } );
        m_acceptorThread.join();
    }

    template <typename SocketType>
    void Listener<SocketType>::BeginAccept()
    {
        auto worker = SelectWorker();
        auto socket = worker->CreateSocket();

        m_acceptor.async_accept(socket->GetAsioSocket(),
            [this, worker, socket] (const boost::system::error_code &ec)
        {
            this->OnAccept(worker, socket, ec);
        });
    }

    template <typename SocketType>
    void Listener<SocketType>::OnAccept(NetworkThread<SocketType> *worker, std::shared_ptr<SocketType> const& socket, const boost::system::error_code &ec)
    {
        // an error has occurred
        if (ec)
            worker->RemoveSocket(socket.get());
        else
            socket->Open();

        if (m_acceptor.is_open())
            BeginAccept();
    }
}

#endif /* !__LISTENER_HPP_ */
