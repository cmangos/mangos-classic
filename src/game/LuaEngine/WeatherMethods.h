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

#ifndef WEATHERMETHODS_H
#define WEATHERMETHODS_H

namespace LuaWeather
{
    /*int GetScriptId(lua_State* L, Weather* weather)
    {
        sEluna.Push(L, weather->GetScriptId());
        return 1;
    }*/

    int GetZoneId(lua_State* L, Weather* weather)
    {
        sEluna.Push(L, weather->GetZone());
        return 1;
    }

    int SetWeather(lua_State* L, Weather* weather)
    {
        uint32 weatherType = sEluna.CHECKVAL<uint32>(L, 2);
        float grade = sEluna.CHECKVAL<float>(L, 3);

        weather->SetWeather((WeatherType)weatherType, grade);
        return 0;
    }

    int SendWeatherUpdateToPlayer(lua_State* L, Weather* weather)
    {
        Player* player = sEluna.CHECKOBJ<Player>(L, 2);

        weather->SendWeatherUpdateToPlayer(player);
        return 0;
    }

    int Regenerate(lua_State* L, Weather* weather)
    {
        sEluna.Push(L, weather->ReGenerate());
        return 1;
    }

    int UpdateWeather(lua_State* L, Weather* weather)
    {
        sEluna.Push(L, weather->UpdateWeather());
        return 1;
    }
};

#endif
