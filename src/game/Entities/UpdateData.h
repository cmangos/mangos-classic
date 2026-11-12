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

#include "Util/ByteBuffer.h"
#include "Entities/ObjectGuid.h"

class WorldPacket;
class WorldSession;

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
    UPDATEFLAG_HAS_ATTACKING_TARGET     = 0x0004,
    UPDATEFLAG_HIGHGUID     = 0x0008,
    UPDATEFLAG_ALL          = 0x0010,
    UPDATEFLAG_LIVING       = 0x0020,
    UPDATEFLAG_HAS_POSITION = 0x0040
};

// Blocks of SMSG_UPDATE_OBJECT for one client. The packets are cut when they are built: the out of range block goes once,
// at the head of the first packet, the other blocks follow in the order they were added, and a block that does not fit
// starts a new packet.
class UpdateData
{
    public:
        UpdateData();

        void AddOutOfRangeGUID(GuidSet& guids);
        void AddOutOfRangeGUID(ObjectGuid const& guid);
        void AddUpdateBlock(const ByteBuffer& block, bool transport);
        void AddAfterCreatePacket(const WorldPacket& data);
        WorldPacket BuildPacket(size_t index) const; // Copy Elision is a thing
        bool HasData() const { return !m_blockEnds.empty() || !m_outOfRangeGUIDs.empty(); }
        void Clear();

        void SendData(WorldSession& session, bool forceSend = false) const;

    protected:
        size_t GetPacketEnd(size_t firstBlock) const;
        WorldPacket BuildPacketOfBlocks(size_t firstBlock, size_t endBlock) const;

        GuidSet m_outOfRangeGUIDs;
        ByteBuffer m_blocks;
        std::vector<size_t> m_blockEnds;                    // end of each block in m_blocks
        std::vector<bool> m_transportBlocks;                // whether each block is a transport's

        std::vector<WorldPacket> m_afterCreatePacket;

        static void Compress(void* dst, uint32* dst_size, void* src, int src_size);
};
#endif
