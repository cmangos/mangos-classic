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

#ifndef QUERYMETHODS_H
#define QUERYMETHODS_H

namespace LuaQuery
{
    int NextRow(lua_State* L, QueryResult* result)
    {
        if (!result)
            sEluna.Push(L, false);
        else
            sEluna.Push(L, result->NextRow());
        return 1;
    }

    int GetColumnCount(lua_State* L, QueryResult* result)
    {
        if (!result)
            sEluna.Push(L, 0);
        else
            sEluna.Push(L, result->GetFieldCount());
        return 1;
    }

    int GetRowCount(lua_State* L, QueryResult* result)
    {
        if (!result)
            sEluna.Push(L, 0);
        else
        {
            if (result->GetRowCount() > (uint32) - 1)
                sEluna.Push(L, (uint32) - 1);
            else
                sEluna.Push(L, result->GetRowCount());
        }
        return 1;
    }

    int IsNull(lua_State* L, QueryResult* result)
    {
        uint32 col = sEluna.CHECKVAL<uint32>(L, 2);
        if (!result || col >= result->GetFieldCount())
            sEluna.Push(L, true);
        else
            sEluna.Push(L, result->Fetch()[col].IsNULL());
        return 1;
    }

    int GetBool(lua_State* L, QueryResult* result)
    {
        uint32 col = sEluna.CHECKVAL<uint32>(L, 2);
        if (!result || col >= result->GetFieldCount())
            sEluna.Push(L, false);
        else
            sEluna.Push(L, result->Fetch()[col].GetBool());
        return 1;
    }

    int GetUInt8(lua_State* L, QueryResult* result)
    {
        uint32 col = sEluna.CHECKVAL<uint32>(L, 2);
        if (!result || col >= result->GetFieldCount())
            sEluna.Push(L, 0);
        else
            sEluna.Push(L, result->Fetch()[col].GetUInt8());
        return 1;
    }

    int GetUInt16(lua_State* L, QueryResult* result)
    {
        uint32 col = sEluna.CHECKVAL<uint32>(L, 2);
        if (!result || col >= result->GetFieldCount())
            sEluna.Push(L, 0);
        else
            sEluna.Push(L, result->Fetch()[col].GetUInt16());
        return 1;
    }

    int GetUInt32(lua_State* L, QueryResult* result)
    {
        uint32 col = sEluna.CHECKVAL<uint32>(L, 2);
        if (!result || col >= result->GetFieldCount())
            sEluna.Push(L, 0);
        else
            sEluna.Push(L, result->Fetch()[col].GetUInt32());
        return 1;
    }

    int GetUInt64(lua_State* L, QueryResult* result)
    {
        uint32 col = sEluna.CHECKVAL<uint32>(L, 2);
        if (!result || col >= result->GetFieldCount())
            sEluna.Push(L, 0);
        else
            sEluna.Push(L, result->Fetch()[col].GetUInt64());
        return 1;
    }

    int GetInt8(lua_State* L, QueryResult* result)
    {
        uint32 col = sEluna.CHECKVAL<uint32>(L, 2);
        if (!result || col >= result->GetFieldCount())
            sEluna.Push(L, 0);
        else
            sEluna.Push(L, result->Fetch()[col].GetInt8());
        return 1;
    }

    int GetInt16(lua_State* L, QueryResult* result)
    {
        uint32 col = sEluna.CHECKVAL<uint32>(L, 2);
        if (!result || col >= result->GetFieldCount())
            sEluna.Push(L, 0);
        else
            sEluna.Push(L, result->Fetch()[col].GetInt16());
        return 1;
    }

    int GetInt32(lua_State* L, QueryResult* result)
    {
        uint32 col = sEluna.CHECKVAL<uint32>(L, 2);
        if (!result || col >= result->GetFieldCount())
            sEluna.Push(L, 0);
        else
            sEluna.Push(L, result->Fetch()[col].GetInt32());
        return 1;
    }

    int GetInt64(lua_State* L, QueryResult* result)
    {
        uint32 col = sEluna.CHECKVAL<uint32>(L, 2);
        if (!result || col >= result->GetFieldCount())
            sEluna.Push(L, 0);
        else
            sEluna.Push(L, result->Fetch()[col].GetInt64());
        return 1;
    }

    int GetFloat(lua_State* L, QueryResult* result)
    {
        uint32 col = sEluna.CHECKVAL<uint32>(L, 2);
        if (!result || col >= result->GetFieldCount())
            sEluna.Push(L, 0.0f);
        else
            sEluna.Push(L, result->Fetch()[col].GetFloat());
        return 1;
    }

    int GetDouble(lua_State* L, QueryResult* result)
    {
        uint32 col = sEluna.CHECKVAL<uint32>(L, 2);
        if (!result || col >= result->GetFieldCount())
            sEluna.Push(L, 0.0);
        else
            sEluna.Push(L, result->Fetch()[col].GetDouble());
        return 1;
    }

    int GetString(lua_State* L, QueryResult* result)
    {
        uint32 col = sEluna.CHECKVAL<uint32>(L, 2);
        if (!result || col >= result->GetFieldCount())
            sEluna.Push(L, "");
        else
            sEluna.Push(L, result->Fetch()[col].GetString());
        return 1;
    }
};
#endif
