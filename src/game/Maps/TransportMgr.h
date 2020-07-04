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

#ifndef _TRANSPORT_SYSTEM_H
#define _TRANSPORT_SYSTEM_H

#include "Platform/Define.h"
#include "Server/DBCStructure.h"

#include <map>

typedef std::map<uint32, TransportAnimationEntry const*> TransportPathContainer;
// typedef std::map<uint32, TransportRotationEntry const*> TransportPathRotationContainer;

struct TransportAnimation
{
    TransportAnimation() : TotalTime(0) { }

    TransportPathContainer Path;
    // TransportPathRotationContainer Rotations;
    uint32 TotalTime;

    TransportAnimationEntry const* GetPrevAnimNode(uint32 time) const;
    TransportAnimationEntry const* GetNextAnimNode(uint32 time) const;
    // TransportRotationEntry const* GetAnimRotation(uint32 time) const; - wotlk onwards
};

typedef std::map<uint32, TransportAnimation> TransportAnimationContainer;

class TransportMgr
{
    public:

        void LoadTransportAnimationAndRotation();

        TransportAnimation const* GetTransportAnimInfo(uint32 entry) const
        {
            TransportAnimationContainer::const_iterator itr = m_transportAnimations.find(entry);
            if (itr != m_transportAnimations.end())
                return &itr->second;

            return nullptr;
        }

    private:
        void AddPathNodeToTransport(uint32 transportEntry, uint32 timeSeg, TransportAnimationEntry const* node);

        TransportAnimationContainer m_transportAnimations;
};

#define sTransportMgr MaNGOS::Singleton<TransportMgr>::Instance()

#endif