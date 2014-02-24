/*
 * This file is part of the CMaNGOS Project. See AUTHORS file for Copyright information
 * Copyright (C) 2010 - 2014 Eluna Lua Engine <http://emudevs.com/>
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

#ifndef GUILDMETHODS_H
#define GUILDMETHODS_H

namespace LuaGuild
{
    int GetMembers(lua_State* L, Guild* guild)
    {
        lua_newtable(L);
        int tbl = lua_gettop(L);
        uint32 i = 0;

        SessionMap const& sessions = sWorld.GetAllSessions();
        for (SessionMap::const_iterator it = sessions.begin(); it != sessions.end(); ++it)
        {
            if (Player* player = it->second->GetPlayer())
            {
                if (player->GetSession() && (player->GetGuildId() == guild->GetId()))
                {
                    ++i;
                    sEluna.Push(L, i);
                    sEluna.Push(L, player);
                    lua_settable(L, tbl);
                }
            }
        }

        lua_settop(L, tbl); // push table to top of stack
        return 1;
    }

    int GetMemberCount(lua_State* L, Guild* guild)
    {
        sEluna.Push(L, guild->GetMemberSize());
        return 1;
    }

    int GetLeader(lua_State* L, Guild* guild)
    {
        sEluna.Push(L, sObjectMgr.GetPlayer(guild->GetLeaderGuid()));
        return 1;
    }

    int SetLeader(lua_State* L, Guild* guild)
    {
        Player* player = sEluna.CHECKOBJ<Player>(L, 2);

        guild->SetLeader(player->GetObjectGuid());
        return 0;
    }

    int GetLeaderGUID(lua_State* L, Guild* guild)
    {
        sEluna.Push(L, guild->GetLeaderGuid());
        return 1;
    }

    // SendPacketToGuild(packet)
    int SendPacket(lua_State* L, Guild* guild)
    {
        WorldPacket* data = sEluna.CHECKOBJ<WorldPacket>(L, 2);

        guild->BroadcastPacket(data);
        return 0;
    }

    // SendPacketToRankedInGuild(packet, rankId)
    int SendPacketToRanked(lua_State* L, Guild* guild)
    {
        WorldPacket* data = sEluna.CHECKOBJ<WorldPacket>(L, 2);
        uint8 ranked = sEluna.CHECKVAL<uint8>(L, 3);

        guild->BroadcastPacketToRank(data, ranked);
        return 0;
    }

    int Disband(lua_State* L, Guild* guild)
    {
        guild->Disband();
        return 0;
    }

    int GetId(lua_State* L, Guild* guild)
    {
        sEluna.Push(L, guild->GetId());
        return 1;
    }

    int GetName(lua_State* L, Guild* guild)
    {
        sEluna.Push(L, guild->GetName());
        return 1;
    }

    int GetMOTD(lua_State* L, Guild* guild)
    {
        sEluna.Push(L, guild->GetMOTD());
        return 1;
    }

    int GetInfo(lua_State* L, Guild* guild)
    {
        sEluna.Push(L, guild->GetGINFO());
        return 1;
    }

    int AddMember(lua_State* L, Guild* guild)
    {
        Player* player = sEluna.CHECKOBJ<Player>(L, 2);
        uint8 rankId = sEluna.CHECKVAL<uint8>(L, 3, GUILD_RANK_NONE);

        guild->AddMember(player->GetObjectGuid(), rankId);
        return 0;
    }

    int DeleteMember(lua_State* L, Guild* guild)
    {
        Player* player = sEluna.CHECKOBJ<Player>(L, 2);
        bool isDisbanding = sEluna.CHECKVAL<bool>(L, 3, false);

        guild->DelMember(player->GetObjectGuid(), isDisbanding);
        return 0;
    }

    int ChangeMemberRank(lua_State* L, Guild* guild)
    {
        Player* player = sEluna.CHECKOBJ<Player>(L, 2);
        uint8 newRank = sEluna.CHECKVAL<uint8>(L, 3);

        guild->ChangeMemberRank(player->GetObjectGuid(), newRank);
        return 0;
    }

    /*int SetBankTabText(lua_State* L, Guild* guild)
    {
        uint8 tabId = sEluna.CHECKVAL<uint8>(L, 2);
        const char* text = sEluna.CHECKVAL<const char*>(L, 3);
        guild->SetGuildBankTabText(tabId, text);
        return 0;
    }*/

	/*int GetBankMoney(lua_State* L, Guild* guild)
    {
        sEluna.Push(L, guild->GetGuildBankMoney());
        return 1;
    }*/

	/*int WithdrawBankMoney(lua_State* L, Guild* guild)
    {
        Player* player = sEluna.CHECKOBJ<Player>(L, 2);
        uint32 money = sEluna.CHECKVAL<uint32>(L, 3);

        if (guild->GetGuildBankMoney() < money)
            return 0;

        player->SetMoney(player->GetMoney() + money);
        guild->SetBankMoney(guild->GetGuildBankMoney() - money);
        return 0;
    }*/

	/*int DepositBankMoney(lua_State* L, Guild* guild)
    {
        Player* player = sEluna.CHECKOBJ<Player>(L, 2);
        uint32 money = sEluna.CHECKVAL<uint32>(L, 3);

        if (player->GetMoney() < money)
            return 0;

        player->SetMoney(player->GetMoney() - money);
        guild->SetBankMoney(guild->GetGuildBankMoney() + money);
        return 0;
    }*/
};
#endif
