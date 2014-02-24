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

#ifndef GROUPMETHODS_H
#define GROUPMETHODS_H

namespace LuaGroup
{
    int GetMembers(lua_State* L, Group* group)
    {
        lua_newtable(L);
        int tbl = lua_gettop(L);
        uint32 i = 0;

        for (GroupReference* itr = group->GetFirstMember(); itr; itr = itr->next())
        {
            Player* member = itr->getSource();

            if (!member || !member->GetSession())
                continue;

            ++i;
            sEluna.Push(L, i);
            sEluna.Push(L, member);
            lua_settable(L, tbl);
        }

        lua_settop(L, tbl); // push table to top of stack
        return 1;
    }

    int GetLeaderGUID(lua_State* L, Group* group)
    {
        sEluna.Push(L, group->GetLeaderGuid());
        return 1;
    }

    int GetLeader(lua_State* L, Group* group)
    {
        sEluna.Push(L, sObjectAccessor.FindPlayer(group->GetLeaderGuid()));
        return 1;
    }

    /*int GetGUID(lua_State* L, Group* group)
    {
        sEluna.Push(L, group->GetObjectGuid());
        return 1;
    }*/

    int ChangeLeader(lua_State* L, Group* group)
    {
        Player* leader = sEluna.CHECKOBJ<Player>(L, 2);

        group->ChangeLeader(leader->GetObjectGuid());
        return 0;
    }

    int IsLeader(lua_State* L, Group* group)
    {
        uint64 guid = sEluna.CHECKVAL<uint64>(L, 2);
        sEluna.Push(L, group->IsLeader(ObjectGuid(guid)));
        return 1;
    }

    // SendPacket(packet, sendToPlayersInBattleground[, ignoreguid])
    int SendPacket(lua_State* L, Group* group)
    {
        WorldPacket* data = sEluna.CHECKOBJ<WorldPacket>(L, 2);
        bool ignorePlayersInBg = sEluna.CHECKVAL<bool>(L, 3);
        uint64 ignore = sEluna.CHECKVAL<uint64>(L, 4);

        group->BroadcastPacket(data, ignorePlayersInBg, -1, (ObjectGuid)ignore);
        return 0;
    }

    int AddInvite(lua_State* L, Group* group)
    {
        Player* player = sEluna.CHECKOBJ<Player>(L, 2);

        sEluna.Push(L, group->AddInvite(player));
        return 1;
    }

    int RemoveMember(lua_State* L, Group* group)
    {
        Player* player = sEluna.CHECKOBJ<Player>(L, 2);
        int method = sEluna.CHECKVAL<int>(L, 3, 0);

        sEluna.Push(L, group->RemoveMember(player->GetObjectGuid(), method));
        return 1;
    }

    int Disband(lua_State* L, Group* group)
    {
        group->Disband();
        return 0;
    }

    int IsFull(lua_State* L, Group* group)
    {
        sEluna.Push(L, group->IsFull());
        return 1;
    }

    /*int isLFGGroup(lua_State* L, Group* group) // TODO: Implementation
    {
        sEluna.Push(L, group->isLFGGroup());
        return 1;
    }*/

    int isRaidGroup(lua_State* L, Group* group)
    {
        sEluna.Push(L, group->isRaidGroup());
        return 1;
    }

    int isBGGroup(lua_State* L, Group* group)
    {
        sEluna.Push(L, group->isBGGroup());
        return 1;
    }

    /*int isBFGroup(lua_State* L, Group* group) // TODO: Implementation
    {
        sEluna.Push(L, group->isBFGroup());
        return 1;
    }*/

    int IsMember(lua_State* L, Group* group)
    {
        Player* player = sEluna.CHECKOBJ<Player>(L, 2);
        sEluna.Push(L, group->IsMember(player->GetObjectGuid()));
        return 1;
    }

    int IsAssistant(lua_State* L, Group* group)
    {
        Player* player = sEluna.CHECKOBJ<Player>(L, 2);

        sEluna.Push(L, group->IsAssistant(player->GetObjectGuid()));
        return 1;
    }

    int SameSubGroup(lua_State* L, Group* group)
    {
        Player* player1 = sEluna.CHECKOBJ<Player>(L, 2);
        Player* player2 = sEluna.CHECKOBJ<Player>(L, 3);
        sEluna.Push(L, group->SameSubGroup(player1, player2));
        return 1;
    }

    int HasFreeSlotSubGroup(lua_State* L, Group* group)
    {
        uint8 subGroup = sEluna.CHECKVAL<uint8>(L, 2);
        sEluna.Push(L, group->HasFreeSlotSubGroup(subGroup));
        return 1;
    }

    int GetMemberGUID(lua_State* L, Group* group)
    {
        const char* name = sEluna.CHECKVAL<const char*>(L, 2);
        sEluna.Push(L, group->GetMemberGuid(name));
        return 1;
    }

    int GetMembersCount(lua_State* L, Group* group)
    {
        sEluna.Push(L, group->GetMembersCount());
        return 1;
    }

    /*int ConvertToLFG(lua_State* L, Group* group) // TODO: Implementation
    {
        group->ConvertToLFG();
        return 0;
    }*/

    int ConvertToRaid(lua_State* L, Group* group)
    {
        group->ConvertToRaid();
        return 0;
    }

    int ChangeMembersGroup(lua_State* L, Group* group)
    {
        Player* player = sEluna.CHECKOBJ<Player>(L, 2);
        uint8 groupID = sEluna.CHECKVAL<uint8>(L, 3);

        group->ChangeMembersGroup(player->GetObjectGuid(), groupID);
        return 0;
    }

    int GetMemberGroup(lua_State* L, Group* group)
    {
        Player* player = sEluna.CHECKOBJ<Player>(L, 2);

        sEluna.Push(L, group->GetMemberGroup(player->GetObjectGuid()));
        return 1;
    }
};
#endif
