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
#include "Util/ByteBuffer.h"
#include "Server/WorldPacket.h"
#include "Log/Log.h"
#include "Server/Opcodes.h"
#include "World/World.h"
#include "Entities/ObjectGuid.h"
#include "Server/WorldSession.h"

UpdateData::UpdateData() : m_blocks(0)
{
}

void UpdateData::AddOutOfRangeGUID(GuidSet& guids)
{
    m_outOfRangeGUIDs.insert(guids.begin(), guids.end());
}

void UpdateData::AddOutOfRangeGUID(ObjectGuid const& guid)
{
    m_outOfRangeGUIDs.insert(guid);
}

void UpdateData::AddUpdateBlock(const ByteBuffer& block, bool transport)
{
    m_blocks.append(block);
    m_blockEnds.push_back(m_blocks.wpos());
    m_transportBlocks.push_back(transport);
}

void UpdateData::AddAfterCreatePacket(const WorldPacket& data)
{
    m_afterCreatePacket.emplace_back(data);
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

// End (one past the last block) of the packet that starts with firstBlock
size_t UpdateData::GetPacketEnd(size_t firstBlock) const
{
    size_t reserved = 128;
    if (firstBlock == 0)
        reserved += 9 * m_outOfRangeGUIDs.size();

    size_t const start = firstBlock == 0 ? 0 : m_blockEnds[firstBlock - 1];
    size_t end = firstBlock;
    while (end < m_blockEnds.size() && (end == firstBlock || reserved + (m_blockEnds[end] - start) < MAX_NETCLIENT_PACKET_SIZE))
        ++end;
    return end;
}

WorldPacket UpdateData::BuildPacket(size_t index) const
{
    size_t firstBlock = 0;
    for (size_t i = 0; i < index; ++i)
        firstBlock = GetPacketEnd(firstBlock);
    return BuildPacketOfBlocks(firstBlock, GetPacketEnd(firstBlock));
}

WorldPacket UpdateData::BuildPacketOfBlocks(size_t firstBlock, size_t endBlock) const
{
    WorldPacket packet;
    MANGOS_ASSERT(packet.empty());                         // shouldn't happen

    bool const hasOutOfRange = firstBlock == 0 && !m_outOfRangeGUIDs.empty();
    size_t const start = firstBlock == 0 ? 0 : m_blockEnds[firstBlock - 1];
    size_t const stop = endBlock == 0 ? 0 : m_blockEnds[endBlock - 1];
    bool hasTransport = false;
    for (size_t i = firstBlock; i < endBlock && !hasTransport; ++i)
        hasTransport = m_transportBlocks[i];

    ByteBuffer buf(4 + 1 + (hasOutOfRange ? 1 + 4 + 9 * m_outOfRangeGUIDs.size() : 0) + (stop - start));

    buf << uint32(endBlock - firstBlock + (hasOutOfRange ? 1 : 0));
    buf << uint8(hasTransport);

    if (hasOutOfRange)
    {
        buf << (uint8) UPDATETYPE_OUT_OF_RANGE_OBJECTS;
        buf << (uint32) m_outOfRangeGUIDs.size();

        for (auto m_outOfRangeGUID : m_outOfRangeGUIDs)
            buf << m_outOfRangeGUID.WriteAsPacked();
    }

    if (stop > start)
        buf.append(m_blocks.contents() + start, stop - start);

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
    // the buffers are kept for the next packets, unless a large update made them grow beyond what the usual ones need
    size_t const keptBlocksSize = 16 * 1024;
    size_t const keptBlockCount = 512;

    m_outOfRangeGUIDs.clear();
    if (m_blocks.wpos() > keptBlocksSize)
        m_blocks = ByteBuffer(0);
    else
        m_blocks.clear();
    if (m_blockEnds.size() > keptBlockCount)
    {
        std::vector<size_t>().swap(m_blockEnds);
        std::vector<bool>().swap(m_transportBlocks);
    }
    else
    {
        m_blockEnds.clear();
        m_transportBlocks.clear();
    }
    if (m_afterCreatePacket.size() > keptBlockCount)
        std::vector<WorldPacket>().swap(m_afterCreatePacket);
    else
        m_afterCreatePacket.clear();
}

void UpdateData::SendData(WorldSession& session, bool forceSend/*= false*/) const
{
    if (!HasData())
        return;

    size_t firstBlock = 0;
    do
    {
        size_t const endBlock = GetPacketEnd(firstBlock);
        WorldPacket packet = BuildPacketOfBlocks(firstBlock, endBlock);
        session.SendPacket(packet);
        firstBlock = endBlock;
    }
    while (firstBlock < m_blockEnds.size());

    for (auto& packet : m_afterCreatePacket)
        session.SendPacket(packet, forceSend);
}
