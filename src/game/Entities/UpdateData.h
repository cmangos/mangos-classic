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

#ifndef __UPDATEDATA_H
#define __UPDATEDATA_H

#include "ByteBuffer.h"
#include "Entities/ObjectGuid.h"

class WorldPacket;

enum ObjectUpdateType
{
    UPDATETYPE_VALUES               = 0,
    UPDATETYPE_MOVEMENT             = 1,
    UPDATETYPE_CREATE_OBJECT        = 2,
    UPDATETYPE_CREATE_OBJECT2       = 3,
    UPDATETYPE_OUT_OF_RANGE_OBJECTS = 4,
    UPDATETYPE_NEAR_OBJECTS         = 5
};

// checked for 1.12.1
enum ObjectUpdateFlags
{
    UPDATEFLAG_NONE         = 0x0000,
    UPDATEFLAG_SELF         = 0x0001,
    UPDATEFLAG_TRANSPORT    = 0x0002,
    UPDATEFLAG_FULLGUID     = 0x0004,
    UPDATEFLAG_HIGHGUID     = 0x0008,
    UPDATEFLAG_ALL          = 0x0010,
    UPDATEFLAG_LIVING       = 0x0020,
    UPDATEFLAG_HAS_POSITION = 0x0040
};

struct BufferPair
{
    ByteBuffer m_buffer;
    uint32 m_blockCount;
};

class UpdateData
{
    public:
        UpdateData();

        void AddOutOfRangeGUID(GuidSet& guids);
        void AddOutOfRangeGUID(ObjectGuid const& guid);
        void AddUpdateBlock(const ByteBuffer& block);
        WorldPacket BuildPacket(size_t index, bool hasTransport = false); // Copy Elision is a thing
        bool HasData() const { return m_data[0].m_buffer.size() > 0 || !m_outOfRangeGUIDs.empty(); }
        size_t GetPacketCount() const { return m_data.size(); }
        void Clear();

        GuidSet const& GetOutOfRangeGUIDs() const { return m_outOfRangeGUIDs; }

    protected:
        GuidSet m_outOfRangeGUIDs;
        std::vector<BufferPair> m_data;
        uint32 m_currentIndex;

        static void Compress(void* dst, uint32* dst_size, void* src, int src_size);
};
#endif
