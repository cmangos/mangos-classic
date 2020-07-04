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

#include "TransportMgr.h"

#include "Server/DBCStores.h"

void TransportMgr::LoadTransportAnimationAndRotation()
{
    for (uint32 i = 0; i < sTransportAnimationStore.GetNumRows(); ++i)
        if (TransportAnimationEntry const* anim = sTransportAnimationStore.LookupEntry(i))
            AddPathNodeToTransport(anim->TransportEntry, anim->TimeSeg, anim);
}

void TransportMgr::AddPathNodeToTransport(uint32 transportEntry, uint32 timeSeg, TransportAnimationEntry const* node)
{
    TransportAnimation& animNode = m_transportAnimations[transportEntry];
    if (animNode.TotalTime < timeSeg)
        animNode.TotalTime = timeSeg;

    animNode.Path[timeSeg] = node;
}

TransportAnimationEntry const* TransportAnimation::GetPrevAnimNode(uint32 time) const
{
    auto itr = Path.lower_bound(time);
    if (itr != Path.end() && itr != Path.begin())
    {
        --itr;
        return itr->second;
    }

    return nullptr;
}

TransportAnimationEntry const* TransportAnimation::GetNextAnimNode(uint32 time) const
{
    auto itr = Path.lower_bound(time);
    if (itr != Path.end())
        return itr->second;

    return nullptr;
}
