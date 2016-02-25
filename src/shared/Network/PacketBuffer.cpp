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

#include "Platform/Define.h"
#include "PacketBuffer.hpp"

#include <cassert>
#include <vector>
#include <cstring>

using namespace MaNGOS;

PacketBuffer::PacketBuffer(int initialSize) : m_writePosition(0), m_readPosition(0), m_buffer(initialSize, 0) {}

void PacketBuffer::Read(char *buffer, int length)
{
    assert(ReadLengthRemaining() >= length);

    if (!!buffer)
        memcpy(buffer, &m_buffer[m_readPosition], length);

    m_readPosition += length;
}

void PacketBuffer::Write(const char *buffer, int length)
{
    assert(!!buffer && !!length);

    const size_t newLength = m_writePosition + length;

    if (m_buffer.size() < newLength)
        m_buffer.resize(newLength);

    memcpy(&m_buffer[m_writePosition], buffer, length);

    m_writePosition += length;
}