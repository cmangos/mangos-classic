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

#include "Socket.hpp"
#include "Log.h"

#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/lexical_cast.hpp>

#include <string>
#include <memory>
#include <utility>
#include <vector>
#include <functional>
#include <cstring>

namespace MaNGOS
{
    Socket::Socket(boost::asio::io_service& service, std::function<void (Socket*)> closeHandler)
        : m_writeState(WriteState::Idle), m_readState(ReadState::Idle), m_socket(service),
          m_closeHandler(std::move(closeHandler)), m_outBufferFlushTimer(service), m_address("0.0.0.0") {}

    bool Socket::Open()
    {
        try
        {
            const_cast<std::string&>(m_address) = m_socket.remote_endpoint().address().to_string();
            const_cast<std::string&>(m_remoteEndpoint) = boost::lexical_cast<std::string>(m_socket.remote_endpoint());
        }
        catch (boost::system::system_error& error)
        {
            sLog.outError("Socket::Open() failed to get remote address.  Error: %s", error.what());
            return false;
        }

        m_outBuffer.reset(new PacketBuffer);
        m_secondaryOutBuffer.reset(new PacketBuffer);
        m_inBuffer.reset(new PacketBuffer);

        StartAsyncRead();

        return true;
    }

    void Socket::Close()
    {
        std::lock_guard<std::mutex> guard(m_closeMutex);
        if (IsClosed())
            return;

        boost::system::error_code ec;
        m_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
        m_socket.close();

        if (m_closeHandler)
            m_closeHandler(this);
    }

    void Socket::StartAsyncRead()
    {
        if (IsClosed())
        {
            m_readState = ReadState::Idle;
            return;
        }

        std::shared_ptr<Socket> ptr = shared<Socket>();
        m_readState = ReadState::Reading;
        m_socket.async_read_some(boost::asio::buffer(&m_inBuffer->m_buffer[m_inBuffer->m_writePosition], m_inBuffer->m_buffer.size() - m_inBuffer->m_writePosition),
                                 make_custom_alloc_handler(m_allocator,
        [ptr](const boost::system::error_code & error, size_t length) { ptr->OnRead(error, length); }));
    }

    void Socket::OnRead(const boost::system::error_code& error, size_t length)
    {
        if (error)
        {
            m_readState = ReadState::Idle;
            OnError(error);
            return;
        }

        if (IsClosed())
        {
            m_readState = ReadState::Idle;
            return;
        }

        m_inBuffer->m_writePosition += length;

        const size_t available = m_socket.available();

        // if there is still data to read, increase the buffer size and do so (if necessary)
        if (available > 0 && (length + available) > m_inBuffer->m_buffer.size())
        {
            m_inBuffer->m_buffer.resize(m_inBuffer->m_buffer.size() + available);
            StartAsyncRead();
            return;
        }

        // we must repeat this in case we have read in multiple messages from the client
        while (m_inBuffer->m_readPosition < m_inBuffer->m_writePosition)
        {
            if (!ProcessIncomingData())
            {
                // this errno is set when there is not enough buffer data available to either complete a header, or the packet length
                // specified in the header goes past what we've read.  in this case, we will reset the buffer with the remaining data
                if (errno == EBADMSG)
                {
                    const size_t bytesRemaining = m_inBuffer->m_writePosition - m_inBuffer->m_readPosition;

                    ::memmove(&m_inBuffer->m_buffer[0], &m_inBuffer->m_buffer[m_inBuffer->m_readPosition], bytesRemaining);

                    m_inBuffer->m_readPosition = 0;
                    m_inBuffer->m_writePosition = bytesRemaining;

                    StartAsyncRead();
                }
                else if (!IsClosed())
                    Close();

                return;
            }
        }

        // at this point, the packet has been read and successfully processed.  reset the buffer.
        m_inBuffer->m_writePosition = m_inBuffer->m_readPosition = 0;

        StartAsyncRead();
    }

    void Socket::OnError(const boost::system::error_code& error)
    {
        // skip logging this code because it happens whenever anyone disconnects.  reduces spam.
        if (error != boost::asio::error::eof &&
                error != boost::asio::error::operation_aborted)
            sLog.outBasic("Socket::OnError.  %s.  Connection closed.", error.message().c_str());

        if (!IsClosed())
            Close();
    }

    bool Socket::Read(char* buffer, int length)
    {
        if (ReadLengthRemaining() < length)
            return false;

        m_inBuffer->Read(buffer, length);
        return true;
    }

    void Socket::Write(const char* header, int headerSize, const char* content, int contentSize)
    {
        std::lock_guard<std::mutex> guard(m_mutex);

        // get the correct buffer depending on the current writing state
        PacketBuffer* outBuffer = m_writeState == WriteState::Sending ? m_secondaryOutBuffer.get() : m_outBuffer.get();

        // write the header
        outBuffer->Write(header, headerSize);

        // write the content
        outBuffer->Write(content, contentSize);

        // flush data if need
        if (m_writeState == WriteState::Idle)
            StartWriteFlushTimer();
    }

    void Socket::Write(const char* buffer, int length)
    {
        std::lock_guard<std::mutex> guard(m_mutex);

        // get the correct buffer depending on the current writing state
        PacketBuffer* outBuffer = m_writeState == WriteState::Sending ? m_secondaryOutBuffer.get() : m_outBuffer.get();

        // write the header
        outBuffer->Write(buffer, length);

        // flush data if need
        if (m_writeState == WriteState::Idle)
            StartWriteFlushTimer();
    }

// note that this function assumes that the socket mutex is locked
    void Socket::StartWriteFlushTimer()
    {
        if (m_writeState == WriteState::Buffering)
            return;

        // if the socket is closed, silently fail
        if (IsClosed())
        {
            m_writeState = WriteState::Idle;
            return;
        }

        m_writeState = WriteState::Buffering;

        std::shared_ptr<Socket> ptr = shared<Socket>();
        m_outBufferFlushTimer.expires_from_now(boost::posix_time::milliseconds(int(BufferTimeout)));
        m_outBufferFlushTimer.async_wait([ptr](const boost::system::error_code&) { ptr->FlushOut(); });
    }

    void Socket::FlushOut()
    {
        // if the socket is closed, silently fail
        if (IsClosed())
        {
            m_writeState = WriteState::Idle;
            return;
        }

        std::lock_guard<std::mutex> guard(m_mutex);

        assert(m_writeState == WriteState::Buffering);

        // at this point we are guarunteed that there is data to send in the primary buffer.  send it.
        m_writeState = WriteState::Sending;

        std::shared_ptr<Socket> ptr = shared<Socket>();
        m_socket.async_write_some(boost::asio::buffer(m_outBuffer->m_buffer, m_outBuffer->m_writePosition),
                                  make_custom_alloc_handler(m_allocator,
        [ptr](const boost::system::error_code & error, size_t length) { ptr->OnWriteComplete(error, length); }));
    }

// if the write state is idle, this will do nothing, which is correct
// if the write state is sending, this will do nothing, which is correct
// if the write state is buffering, this will cancel the running timer, which will immediately trigger FlushOut()
    void Socket::ForceFlushOut()
    {
        m_outBufferFlushTimer.cancel();
    }

    void Socket::OnWriteComplete(const boost::system::error_code& error, size_t length)
    {
        // we must check this before locking the mutex because the connection will be closed,
        // which leads to a locked mutex being destroyed.  not good!
        if (error)
        {
            OnError(error);
            return;
        }

        if (IsClosed())
        {
            m_writeState = WriteState::Idle;
            return;
        }

        std::lock_guard<std::mutex> guard(m_mutex);

        assert(m_writeState == WriteState::Sending);
        assert(length <= m_outBuffer->m_writePosition);

        // if there is data left to write, move it to the start of the buffer
        if (length < m_outBuffer->m_writePosition)
        {
            memcpy(&(m_outBuffer->m_buffer[0]), &(m_outBuffer->m_buffer[length]), (m_outBuffer->m_writePosition - length) * sizeof(m_outBuffer->m_buffer[0]));
            m_outBuffer->m_writePosition -= length;
        }
        // if not, reset the write pointer
        else
            m_outBuffer->m_writePosition = 0;

        // if there is data in the secondary buffer, append it to the primary buffer
        if (m_secondaryOutBuffer->m_writePosition > 0)
        {
            // do we have enough space? if not, resize
            if (m_outBuffer->m_buffer.size() < (m_outBuffer->m_writePosition + m_secondaryOutBuffer->m_writePosition))
                m_outBuffer->m_buffer.resize(m_outBuffer->m_writePosition + m_secondaryOutBuffer->m_writePosition);

            memcpy(&(m_outBuffer->m_buffer[m_outBuffer->m_writePosition]), &(m_secondaryOutBuffer->m_buffer[0]), (m_secondaryOutBuffer->m_writePosition) * sizeof(m_secondaryOutBuffer->m_buffer[0]));

            m_outBuffer->m_writePosition += m_secondaryOutBuffer->m_writePosition;
            m_secondaryOutBuffer->m_writePosition = 0;
        }

        std::shared_ptr<Socket> ptr = shared<Socket>();
        // if there is any data to write, do so immediately
        if (m_outBuffer->m_writePosition > 0)
            m_socket.async_write_some(boost::asio::buffer(m_outBuffer->m_buffer, m_outBuffer->m_writePosition),
                                      make_custom_alloc_handler(m_allocator,
            [ptr](const boost::system::error_code & error, size_t length) { ptr->OnWriteComplete(error, length);}));
        else
            m_writeState = WriteState::Idle;
    }
}
