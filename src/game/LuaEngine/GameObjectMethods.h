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

#ifndef GAMEOBJECTMETHODS_H
#define GAMEOBJECTMETHODS_H

namespace LuaGameObject
{
    int SummonCreature(lua_State* L, GameObject* go)
    {
        if (!go || !go->IsInWorld())
            return 0;

        uint32 entry = sEluna.CHECKVAL<uint32>(L, 2);
        float x = sEluna.CHECKVAL<float>(L, 3);
        float y = sEluna.CHECKVAL<float>(L, 4);
        float z = sEluna.CHECKVAL<float>(L, 5);
        float o = sEluna.CHECKVAL<float>(L, 6);
        uint32 spawnType = sEluna.CHECKVAL<uint32>(L, 7, 0);
        uint32 despawnTimer = sEluna.CHECKVAL<uint32>(L, 8, 0);

        TempSummonType type;
        switch (spawnType)
        {
            case 1:
                type = TEMPSUMMON_TIMED_OR_DEAD_DESPAWN;
                break;
            case 2:
                type = TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN;
                break;
            case 3:
                type = TEMPSUMMON_TIMED_DESPAWN;
                break;
            // case 4:
            //     type = TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT;
            //    break;
            case 5:
                type = TEMPSUMMON_CORPSE_DESPAWN;
                break;
            case 6:
                type = TEMPSUMMON_CORPSE_TIMED_DESPAWN;
                break;
            case 7:
                type = TEMPSUMMON_DEAD_DESPAWN;
                break;
            case 8:
                type = TEMPSUMMON_MANUAL_DESPAWN;
                break;
            default:
                return 0;
        }
        sEluna.Push(L, go->SummonCreature(entry, x, y, z, o, type, despawnTimer));
        return 1;
    }

    int SummonGameObject(lua_State* L, GameObject* go)
    {
        if (!go || !go->IsInWorld())
            return 0;

        uint32 entry = sEluna.CHECKVAL<uint32>(L, 2);
        float x = sEluna.CHECKVAL<float>(L, 3);
        float y = sEluna.CHECKVAL<float>(L, 4);
        float z = sEluna.CHECKVAL<float>(L, 5);
        float o = sEluna.CHECKVAL<float>(L, 6);
        uint32 respawnDelay = sEluna.CHECKVAL<uint32>(L, 7, 30);
        sEluna.Push(L, go->SummonGameObject(entry, x, y, z, o, respawnDelay));
        return 1;
    }

    int GetDisplayId(lua_State* L, GameObject* go)
    {
        if (!go || !go->IsInWorld())
            return 0;

        sEluna.Push(L, go->GetDisplayId());
        return 1;
    }

    int HasQuest(lua_State* L, GameObject* go)
    {
        uint32 questId = sEluna.CHECKVAL<uint32>(L, 2);

        sEluna.Push(L, go->HasQuest(questId));
        return 1;
    }

    int IsSpawned(lua_State* L, GameObject* go)
    {
        if (!go || !go->IsInWorld())
            sEluna.Push(L, false);
        else
            sEluna.Push(L, go->isSpawned());
        return 1;
    }

    int IsTransport(lua_State* L, GameObject* go)
    {
        if (!go || !go->IsInWorld())
            sEluna.Push(L, false);
        else
            sEluna.Push(L, go->IsTransport());
        return 1;
    }

    /*int IsDestructible(lua_State* L, GameObject* go) // TODO: Implementation core side
    {
        if (!go || !go->IsInWorld())
            sEluna.Push(L, false);
        else
            sEluna.Push(L, go->IsDestructibleBuilding());
        return 1;
    }*/

    int IsActive(lua_State* L, GameObject* go)
    {
        if (!go || !go->IsInWorld())
            sEluna.Push(L, false);
        else
            sEluna.Push(L, go->isActiveObject());
        return 1;
    }

    int Move(lua_State* L, GameObject* go)
    {
        if (!go || !go->IsInWorld())
            return 0;

        float x = sEluna.CHECKVAL<float>(L, 2);
        float y = sEluna.CHECKVAL<float>(L, 3);
        float z = sEluna.CHECKVAL<float>(L, 4);
        float o = sEluna.CHECKVAL<float>(L, 5);
        go->Relocate(x, y, z, o);
        return 0;
    }

    int SaveToDB(lua_State* L, GameObject* go)
    {
        go->SaveToDB();
        return 0;
    }

    int RemoveFromWorld(lua_State* L, GameObject* go)
    {
        if (!go || !go->IsInWorld())
            return 0;

        bool del = sEluna.CHECKVAL<bool>(L, 2, false);
        if (del)
            go->DeleteFromDB();
        go->RemoveFromWorld();
        return 0;
    }

    int RegisterEvent(lua_State* L, GameObject* go)
    {
        luaL_checktype(L, 2, LUA_TFUNCTION);
        uint32 delay = sEluna.CHECKVAL<uint32>(L, 3);
        uint32 repeats = sEluna.CHECKVAL<uint32>(L, 4);

        lua_settop(L, 2);
        int functionRef = lua_ref(L, true);
        functionRef = sEluna.m_EventMgr.AddEvent(&go->m_Events, functionRef, delay, repeats, go);
        if (functionRef)
            sEluna.Push(L, functionRef);
        else
            sEluna.Push(L);
        return 1;
    }

    int RemoveEventById(lua_State* L, GameObject* go)
    {
        int eventId = sEluna.CHECKVAL<int>(L, 2);
        sEluna.m_EventMgr.RemoveEvent(&go->m_Events, eventId);
        return 0;
    }

    int RemoveEvents(lua_State* L, GameObject* go)
    {
        sEluna.m_EventMgr.RemoveEvents(&go->m_Events);
        return 0;
    }

    int UseDoorOrButton(lua_State* L, GameObject* go)
    {
        if (!go || !go->IsInWorld())
            return 0;

        uint32 delay = sEluna.CHECKVAL<uint32>(L, 2, 0);

        go->UseDoorOrButton(delay);
        return 0;
    }

    int SetGoState(lua_State* L, GameObject* go)
    {
        if (!go || !go->IsInWorld())
            return 0;

        uint32 state = sEluna.CHECKVAL<uint32>(L, 2, 0);

        if (state == 0)
            go->SetGoState(GO_STATE_ACTIVE);
        else if (state == 1)
            go->SetGoState(GO_STATE_READY);
        else if (state == 2)
            go->SetGoState(GO_STATE_ACTIVE_ALTERNATIVE);

        return 0;
    }

    int GetGoState(lua_State* L, GameObject* go)
    {
        if (!go || !go->IsInWorld())
            return 0;

        sEluna.Push(L, go->GetGoState());
        return 1;
    }

    int SetLootState(lua_State* L, GameObject* go)
    {
        if (!go || !go->IsInWorld())
            return 0;

        uint32 state = sEluna.CHECKVAL<uint32>(L, 2, 0);

        if (state == 0)
            go->SetLootState(GO_NOT_READY);
        else if (state == 1)
            go->SetLootState(GO_READY);
        else if (state == 2)
            go->SetLootState(GO_ACTIVATED);
        else if (state == 3)
            go->SetLootState(GO_JUST_DEACTIVATED);

        return 0;
    }

    int GetLootState(lua_State* L, GameObject* go)
    {
        if (!go || !go->IsInWorld())
            return 0;

        sEluna.Push(L, go->getLootState());
        return 1;
    }

    int Despawn(lua_State* L, GameObject* go)
    {
        uint32 delay = sEluna.CHECKVAL<uint32>(L, 2, 1);
        if (!delay)
            delay = 1;

        go->SetSpawnedByDefault(false);
        go->SetRespawnTime(delay);
        return 0;
    }

    int Respawn(lua_State* L, GameObject* go)
    {
        uint32 delay = sEluna.CHECKVAL<uint32>(L, 2, 1);
        if (!delay)
            delay = 1;

        go->SetSpawnedByDefault(true);
        go->SetRespawnTime(delay);
        return 0;
    }
};
#endif
