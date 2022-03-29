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

#include "Chat/ChannelMgr.h"
#include "Policies/Singleton.h"
#include "World/World.h"

INSTANTIATE_SINGLETON_1(AllianceChannelMgr);
INSTANTIATE_SINGLETON_1(HordeChannelMgr);

ChannelMgr* channelMgr(Team team)
{
    if (sWorld.getConfig(CONFIG_BOOL_ALLOW_TWO_SIDE_INTERACTION_CHANNEL))
        return &MaNGOS::Singleton<AllianceChannelMgr>::Instance();        // cross-faction

    if (team == ALLIANCE)
        return &MaNGOS::Singleton<AllianceChannelMgr>::Instance();
    if (team == HORDE)
        return &MaNGOS::Singleton<HordeChannelMgr>::Instance();

    return nullptr;
}

ChannelMgr::~ChannelMgr()
{
    for (auto& channel : channels)
        delete channel.second;

    channels.clear();
}

Channel* ChannelMgr::GetJoinChannel(const std::string& name)
{
    std::wstring wname;
    Utf8toWStr(name, wname);
    wstrToLower(wname);

    if (channels.find(wname) == channels.end())
    {
        Channel* nchan = new Channel(name);
        channels[wname] = nchan;
        return nchan;
    }

    return channels[wname];
}

Channel* ChannelMgr::GetChannel(const std::string& name, Player* p, bool pkt)
{
    std::wstring wname;
    Utf8toWStr(name, wname);
    wstrToLower(wname);

    ChannelMap::const_iterator i = channels.find(wname);

    if (i == channels.end())
    {
        if (pkt)
        {
            WorldPacket data;
            Channel::MakeNotMember(data, name);
            p->GetSession()->SendPacket(data);
        }

        return nullptr;
    }
    return i->second;
}

void ChannelMgr::LeftChannel(const std::string& name)
{
    std::wstring wname;
    Utf8toWStr(name, wname);
    wstrToLower(wname);

    ChannelMap::const_iterator i = channels.find(wname);

    if (i == channels.end())
        return;

    Channel* channel = i->second;

    if (channel->GetNumPlayers() == 0 && !channel->IsConstant())
    {
        channels.erase(wname);
        delete channel;
    }
}
