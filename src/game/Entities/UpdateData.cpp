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

#include <zlib.h>

#include "Common.h"
#include "Entities/UpdateData.h"
#include "ByteBuffer.h"
#include "WorldPacket.h"
#include "Log.h"
#include "Server/Opcodes.h"
#include "World/World.h"
#include "Entities/ObjectGuid.h"

UpdateData::UpdateData() : m_data(1), m_currentIndex(0)
{
    m_data[0].m_buffer = 0;
}

void UpdateData::AddOutOfRangeGUID(GuidSet& guids)
{
    m_outOfRangeGUIDs.insert(guids.begin(), guids.end());
}

void UpdateData::AddOutOfRangeGUID(ObjectGuid const& guid)
{
    m_outOfRangeGUIDs.insert(guid);
}

void UpdateData::AddUpdateBlock(const ByteBuffer& block)
{
    if (m_data[m_currentIndex].m_buffer.size() < 700)
    {
        m_data[m_currentIndex].m_buffer.append(block);
        ++m_data[m_currentIndex].m_blockCount;
    }
    else
    {
        ++m_currentIndex;
        m_data.emplace_back();
        m_data[m_currentIndex].m_buffer.append(block);
        m_data[m_currentIndex].m_blockCount = 1;
    }
}

void UpdateData::Compress(void* dst, uint32* dst_size, void* src, int src_size)
{
    z_stream c_stream;

    c_stream.zalloc = (alloc_func)nullptr;
    c_stream.zfree = (free_func)nullptr;
    c_stream.opaque = (voidpf)nullptr;

    // default Z_BEST_SPEED (1)
    int z_res = deflateInit(&c_stream, sWorld.getConfig(CONFIG_UINT32_COMPRESSION));
    if (z_res != Z_OK)
    {
        sLog.outError("Can't compress update packet (zlib: deflateInit) Error code: %i (%s)", z_res, zError(z_res));
        *dst_size = 0;
        return;
    }

    c_stream.next_out = (Bytef*)dst;
    c_stream.avail_out = *dst_size;
    c_stream.next_in = (Bytef*)src;
    c_stream.avail_in = (uInt)src_size;

    z_res = deflate(&c_stream, Z_NO_FLUSH);
    if (z_res != Z_OK)
    {
        sLog.outError("Can't compress update packet (zlib: deflate) Error code: %i (%s)", z_res, zError(z_res));
        *dst_size = 0;
        return;
    }

    if (c_stream.avail_in != 0)
    {
        sLog.outError("Can't compress update packet (zlib: deflate not greedy)");
        *dst_size = 0;
        return;
    }

    z_res = deflate(&c_stream, Z_FINISH);
    if (z_res != Z_STREAM_END)
    {
        sLog.outError("Can't compress update packet (zlib: deflate should report Z_STREAM_END instead %i (%s)", z_res, zError(z_res));
        *dst_size = 0;
        return;
    }

    z_res = deflateEnd(&c_stream);
    if (z_res != Z_OK)
    {
        sLog.outError("Can't compress update packet (zlib: deflateEnd) Error code: %i (%s)", z_res, zError(z_res));
        *dst_size = 0;
        return;
    }

    *dst_size = c_stream.total_out;
}

WorldPacket UpdateData::BuildPacket(size_t index, bool hasTransport)
{
    WorldPacket packet;
    MANGOS_ASSERT(packet.empty());                         // shouldn't happen

    ByteBuffer buf(4 + 1 + (m_outOfRangeGUIDs.empty() ? 0 : 1 + 4 + 9 * m_outOfRangeGUIDs.size()) + m_data[index].m_buffer.wpos());

    buf << (uint32)(!m_outOfRangeGUIDs.empty() ? m_data[index].m_blockCount + 1 : m_data[index].m_blockCount);
    buf << (uint8)(hasTransport ? 1 : 0);

    if (!m_outOfRangeGUIDs.empty())
    {
        buf << (uint8) UPDATETYPE_OUT_OF_RANGE_OBJECTS;
        buf << (uint32) m_outOfRangeGUIDs.size();

        for (auto m_outOfRangeGUID : m_outOfRangeGUIDs)
            buf << m_outOfRangeGUID.WriteAsPacked();
    }

    buf.append(m_data[index].m_buffer);

    size_t pSize = buf.wpos();                              // use real used data size

    if (pSize > 100)                                        // compress large packets
    {
        uint32 destsize = compressBound(pSize);
        packet.resize(destsize + sizeof(uint32));

        packet.put<uint32>(0, pSize);
        Compress(const_cast<uint8*>(packet.contents()) + sizeof(uint32), &destsize, (void*)buf.contents(), pSize);
        if (destsize == 0)
            return packet;

        packet.resize(destsize + sizeof(uint32));
        packet.SetOpcode(SMSG_COMPRESSED_UPDATE_OBJECT);
    }
    else                                                    // send small packets without compression
    {
        packet.append(buf);
        packet.SetOpcode(SMSG_UPDATE_OBJECT);
    }

    return packet;
}

void UpdateData::Clear()
{
    m_data.clear();
    m_outOfRangeGUIDs.clear();
}
