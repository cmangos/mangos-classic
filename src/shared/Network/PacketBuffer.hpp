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

#ifndef __PACKET_BUFFER_HPP_
#define __PACKET_BUFFER_HPP_

#include <vector>
#include <functional>

#include "Platform/Define.h"

#define DEFAULT_BUFFER_SIZE     6144

namespace MaNGOS
{
    class PacketBuffer
    {
        friend class Socket;

        private:
            size_t m_writePosition;
            size_t m_readPosition;

            std::vector<uint8> m_buffer;

        public:
            PacketBuffer(int initialSize = DEFAULT_BUFFER_SIZE);

            uint8 Peak() const { return m_buffer[m_readPosition]; }

            void Read(char *buffer, int length);
            int ReadLengthRemaining() const { return m_writePosition - m_readPosition; }

            void Write(const char *buffer, int length);
    };
}

#endif /* !__PACKET_BUFFER_HPP_ */