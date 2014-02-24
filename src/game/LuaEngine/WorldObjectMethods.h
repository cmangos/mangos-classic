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

#ifndef WORLDOBJECTMETHODS_H
#define WORLDOBJECTMETHODS_H

namespace LuaWorldObject
{
    int GetName(lua_State* L, WorldObject* obj)
    {
        sEluna.Push(L, obj->GetName());
        return 1;
    }
    int GetMap(lua_State* L, WorldObject* obj)
    {
        sEluna.Push(L, obj->GetMap());
        return 1;
    }
    /*int GetPhaseMask(lua_State* L, WorldObject* obj)
    {
        sEluna.Push(L, obj->GetPhaseMask());
        return 1;
    }*/
    int GetInstanceId(lua_State* L, WorldObject* obj)
    {
        sEluna.Push(L, obj->GetInstanceId());
        return 1;
    }
    int GetAreaId(lua_State* L, WorldObject* obj)
    {
        sEluna.Push(L, obj->GetAreaId());
        return 1;
    }
    int GetZoneId(lua_State* L, WorldObject* obj)
    {
        sEluna.Push(L, obj->GetZoneId());
        return 1;
    }
    int GetMapId(lua_State* L, WorldObject* obj)
    {
        sEluna.Push(L, obj->GetMapId());
        return 1;
    }
    int GetX(lua_State* L, WorldObject* obj)
    {
        sEluna.Push(L, obj->GetPositionX());
        return 1;
    }
    int GetY(lua_State* L, WorldObject* obj)
    {
        sEluna.Push(L, obj->GetPositionY());
        return 1;
    }
    int GetZ(lua_State* L, WorldObject* obj)
    {
        sEluna.Push(L, obj->GetPositionZ());
        return 1;
    }
    int GetO(lua_State* L, WorldObject* obj)
    {
        sEluna.Push(L, obj->GetOrientation());
        return 1;
    }
    int GetLocation(lua_State* L, WorldObject* obj)
    {
        sEluna.Push(L, obj->GetPositionX());
        sEluna.Push(L, obj->GetPositionY());
        sEluna.Push(L, obj->GetPositionZ());
        sEluna.Push(L, obj->GetOrientation());
        return 4;
    }
    int GetNearestPlayer(lua_State* L, WorldObject* obj)
    {
        float range = sEluna.CHECKVAL<float>(L, 2, SIZE_OF_GRIDS);

        Unit* target = NULL;
        Eluna::WorldObjectInRangeCheck checker(true, obj, range, TYPEMASK_PLAYER);
        MaNGOS::UnitLastSearcher<Eluna::WorldObjectInRangeCheck> searcher(target, checker);
        Cell::VisitWorldObjects(obj, searcher, range);

        sEluna.Push(L, target);
        return 1;
    }
    int GetNearestGameObject(lua_State* L, WorldObject* obj)
    {
        float range = sEluna.CHECKVAL<float>(L, 2, SIZE_OF_GRIDS);
        uint32 entry = sEluna.CHECKVAL<uint32>(L, 3, 0);

        GameObject* target = NULL;
        Eluna::WorldObjectInRangeCheck checker(true, obj, range, TYPEMASK_GAMEOBJECT, entry);
        MaNGOS::GameObjectLastSearcher<Eluna::WorldObjectInRangeCheck> searcher(target, checker);
        Cell::VisitGridObjects(obj, searcher, range);

        sEluna.Push(L, target);
        return 1;
    }
    int GetNearestCreature(lua_State* L, WorldObject* obj)
    {
        float range = sEluna.CHECKVAL<float>(L, 2, SIZE_OF_GRIDS);
        uint32 entry = sEluna.CHECKVAL<uint32>(L, 3, 0);

        Creature* target = NULL;
        Eluna::WorldObjectInRangeCheck checker(true, obj, range, TYPEMASK_UNIT, entry);
        MaNGOS::CreatureLastSearcher<Eluna::WorldObjectInRangeCheck> searcher(target, checker);
        Cell::VisitGridObjects(obj, searcher, range);

        sEluna.Push(L, target);
        return 1;
    }
    int GetPlayersInRange(lua_State* L, WorldObject* obj)
    {
        float range = sEluna.CHECKVAL<float>(L, 2, SIZE_OF_GRIDS);

        std::list<Player*> list;
        Eluna::WorldObjectInRangeCheck checker(false, obj, range, TYPEMASK_PLAYER);
        MaNGOS::PlayerListSearcher<Eluna::WorldObjectInRangeCheck> searcher(list, checker);
        Cell::VisitWorldObjects(obj, searcher, range);

        lua_newtable(L);
        int tbl = lua_gettop(L);
        uint32 i = 0;

        for (std::list<Player*>::const_iterator it = list.begin(); it != list.end(); ++it)
        {
            sEluna.Push(L, ++i);
            sEluna.Push(L, *it);
            lua_settable(L, tbl);
        }

        lua_settop(L, tbl);
        return 1;
    }
    int GetCreaturesInRange(lua_State* L, WorldObject* obj)
    {
        float range = sEluna.CHECKVAL<float>(L, 2, SIZE_OF_GRIDS);
        uint32 entry = sEluna.CHECKVAL<uint32>(L, 3, 0);

        std::list<Creature*> list;
        Eluna::WorldObjectInRangeCheck checker(false, obj, range, TYPEMASK_UNIT, entry);
        MaNGOS::CreatureListSearcher<Eluna::WorldObjectInRangeCheck> searcher(list, checker);
        Cell::VisitGridObjects(obj, searcher, range);

        lua_newtable(L);
        int tbl = lua_gettop(L);
        uint32 i = 0;

        for (std::list<Creature*>::const_iterator it = list.begin(); it != list.end(); ++it)
        {
            sEluna.Push(L, ++i);
            sEluna.Push(L, *it);
            lua_settable(L, tbl);
        }

        lua_settop(L, tbl);
        return 1;
    }
    int GetGameObjectsInRange(lua_State* L, WorldObject* obj)
    {
        float range = sEluna.CHECKVAL<float>(L, 2, SIZE_OF_GRIDS);
        uint32 entry = sEluna.CHECKVAL<uint32>(L, 3, 0);

        std::list<GameObject*> list;
        Eluna::WorldObjectInRangeCheck checker(false, obj, range, TYPEMASK_GAMEOBJECT, entry);
        MaNGOS::GameObjectListSearcher<Eluna::WorldObjectInRangeCheck> searcher(list, checker);
        Cell::VisitGridObjects(obj, searcher, range);

        lua_newtable(L);
        int tbl = lua_gettop(L);
        uint32 i = 0;

        for (std::list<GameObject*>::const_iterator it = list.begin(); it != list.end(); ++it)
        {
            sEluna.Push(L, ++i);
            sEluna.Push(L, *it);
            lua_settable(L, tbl);
        }

        lua_settop(L, tbl);
        return 1;
    }
    int GetNearObject(lua_State* L, WorldObject* obj)
    {
        bool nearest = sEluna.CHECKVAL<bool>(L, 2, true);
        float range = sEluna.CHECKVAL<float>(L, 3, SIZE_OF_GRIDS);
        uint16 type = sEluna.CHECKVAL<uint16>(L, 4, 0); // TypeMask
        uint32 entry = sEluna.CHECKVAL<uint32>(L, 5, 0);
        uint32 hostile = sEluna.CHECKVAL<uint32>(L, 6, 0); // 0 none, 1 hostile, 2 friendly

        float x, y, z;
        obj->GetPosition(x, y, z);
        Eluna::WorldObjectInRangeCheck checker(nearest, obj, range, type, entry, hostile);
        if (nearest)
        {
            WorldObject* target = NULL;
            MaNGOS::WorldObjectLastSearcher<Eluna::WorldObjectInRangeCheck> searcher(target, checker);
            Cell::VisitAllObjects(obj, searcher, range);

            sEluna.Push(L, target);
            return 1;
        }
        else
        {
            std::list<WorldObject*> list;
            MaNGOS::WorldObjectListSearcher<Eluna::WorldObjectInRangeCheck> searcher(list, checker);
            Cell::VisitAllObjects(obj, searcher, range);

            lua_newtable(L);
            int tbl = lua_gettop(L);
            uint32 i = 0;

            for (std::list<WorldObject*>::const_iterator it = list.begin(); it != list.end(); ++it)
            {
                sEluna.Push(L, ++i);
                sEluna.Push(L, *it);
                lua_settable(L, tbl);
            }

            lua_settop(L, tbl);
            return 1;
        }

        return 0;
    }
    int GetWorldObject(lua_State* L, WorldObject* obj)
    {
        ObjectGuid guid = ObjectGuid(sEluna.CHECKVAL<uint64>(L, 2));

        switch (guid.GetHigh())
        {
            case HIGHGUID_PLAYER:        sEluna.Push(L, obj->GetMap()->GetPlayer(guid)); break;
            case HIGHGUID_TRANSPORT:
            case HIGHGUID_MO_TRANSPORT:
            case HIGHGUID_GAMEOBJECT:    sEluna.Push(L, obj->GetMap()->GetGameObject(guid)); break;
            case HIGHGUID_UNIT:
            case HIGHGUID_PET:           sEluna.Push(L, obj->GetMap()->GetAnyTypeCreature(guid)); break;
            default:                     return 0;
        }
        return 1;
    }

    int GetDistance(lua_State* L, WorldObject* obj)
    {
        WorldObject* target = sEluna.CHECKOBJ<WorldObject>(L, 2, false);
        if (target && target->IsInWorld())
            sEluna.Push(L, obj->GetDistance(target));
        else
        {
            float X = sEluna.CHECKVAL<float>(L, 2);
            float Y = sEluna.CHECKVAL<float>(L, 3);
            float Z = sEluna.CHECKVAL<float>(L, 4);
            sEluna.Push(L, obj->GetDistance(X, Y, Z));
        }
        return 1;
    }

    int GetRelativePoint(lua_State* L, WorldObject* obj)
    {
        float dist = sEluna.CHECKVAL<float>(L, 2);
        float rad = sEluna.CHECKVAL<float>(L, 3);

        float x, y, z;
        obj->GetClosePoint(x, y, z, 0.0f, dist, rad);

        sEluna.Push(L, x);
        sEluna.Push(L, y);
        sEluna.Push(L, z);
        return 3;
    }

    int GetAngle(lua_State* L, WorldObject* obj)
    {
        WorldObject* target = sEluna.CHECKOBJ<WorldObject>(L, 2, false);

        if (target && target->IsInWorld())
            sEluna.Push(L, obj->GetAngle(target));
        else
        {
            float x = sEluna.CHECKVAL<float>(L, 2);
            float y = sEluna.CHECKVAL<float>(L, 3);
            sEluna.Push(L, obj->GetAngle(x, y));
        }
        return 1;
    }
};
#endif
