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

#ifndef MANGOS_MAP_GUID_ALLOCATOR_H
#define MANGOS_MAP_GUID_ALLOCATOR_H

#include "Common.h"
#include "Entities/ObjectGuid.h"

#include <chrono>
#include <functional>
#include <mutex>

/**
 * Hands out the map local guid counters of one guid type: creatures, gameobjects or pets.
 *
 * First round: the counters of the range are handed out in order, exactly like ObjectGuidGenerator.
 * At the end of the range, instead of stopping the server, the cursor wraps around to the start of the
 * range and from then on skips every counter still used by an object of the same entry. The entry (the
 * pet number for pets) is part of the guid, so objects of different entries never share a guid, even
 * with the same counter. A counter thus comes back only after a full round of the cursor, which every
 * entry shares.
 *
 * If every counter of the range is in use for the entry, this is itself a sign something else is
 * wrong: Generate() stops the server, as ObjectGuidGenerator does on overflow, instead of
 * silently refusing object creations one by one.
 */
class MapGuidAllocator
{
    public:
        // Tells whether an object with this guid is currently in the map
        typedef std::function<bool(ObjectGuid const&)> GuidInUse;

        MapGuidAllocator(HighGuid high, uint32 mapId, uint32 instanceId, GuidInUse inUse);

        // First counter handed out; the range ends where ObjectGuidGenerator stops the server
        void Set(uint32 first) { SetRange(first, ObjectGuid::GetMaxCounter(m_high) - 1000); }
        // Counters handed out: [first, end)
        void SetRange(uint32 first, uint32 end);

        // entry: the one written in the guid (the pet number for pets). Returns 0 if none is free.
        uint32 Generate(uint32 entry);

    private:
        void StartNewRound();

        HighGuid const m_high;
        uint32 const m_mapId;
        uint32 const m_instanceId;
        GuidInUse const m_inUse;

        std::mutex m_lock;
        uint32 m_first;
        uint32 m_end;
        uint32 m_next;
        uint32 m_wraps;
        std::chrono::steady_clock::time_point m_roundStart;
};

#endif
