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

/// \addtogroup world
/// @{
/// \file

#ifndef __WEATHER_H
#define __WEATHER_H

#include "Common.h"
#include "SharedDefines.h"
#include "Timer.h"

class Player;
class Map;

// ---------------------------------------------------------
//            Actual Weather in one zone
// ---------------------------------------------------------

enum WeatherState
{
    WEATHER_STATE_FINE              = 0,
    WEATHER_STATE_LIGHT_RAIN        = 3,
    WEATHER_STATE_MEDIUM_RAIN       = 4,
    WEATHER_STATE_HEAVY_RAIN        = 5,
    WEATHER_STATE_LIGHT_SNOW        = 6,
    WEATHER_STATE_MEDIUM_SNOW       = 7,
    WEATHER_STATE_HEAVY_SNOW        = 8,
    WEATHER_STATE_LIGHT_SANDSTORM   = 22,
    WEATHER_STATE_MEDIUM_SANDSTORM  = 41,
    WEATHER_STATE_HEAVY_SANDSTORM   = 42
};

struct WeatherZoneChances;

/// Weather for one zone
class Weather
{
    public:
        Weather(uint32 zone, WeatherZoneChances const* weatherChances);
        ~Weather() {};

        /// Send Weather to one player
        void SendWeatherUpdateToPlayer(Player* player);
        /// Set the weather
        void SetWeather(WeatherType type, float grade, Map const* _map, bool isPermanent);
        /// Update the weather in this zone, when the timer is expired the weather will be rolled again
        bool Update(uint32 diff, Map const* _map);
        /// Check if a type is valid
        static bool IsValidWeatherType(uint32 type)
        {
            switch (type)
            {
                case WEATHER_TYPE_FINE:
                case WEATHER_TYPE_RAIN:
                case WEATHER_TYPE_SNOW:
                case WEATHER_TYPE_STORM:
                    return true;
                default:
                    return false;
            }
        }

    private:
        uint32 GetSound();
        /// Send SMSG_WEATHER to all players in the zone
        bool SendWeatherForPlayersInZone(Map const* _map);
        /// Calculate new weather
        bool ReGenerate();
        /// Calculate state based on type and grade
        WeatherState GetWeatherState() const;
        // Helper to get the grade between 0..1
        void NormalizeGrade();
        // Helper to log recent state
        void LogWeatherState(WeatherState state) const;

        uint32 m_zone;
        WeatherType m_type;
        float m_grade;
        ShortIntervalTimer m_timer;
        WeatherZoneChances const* m_weatherChances;
        bool m_isPermanentWeather;
};

// ---------------------------------------------------------
//         Weather information hold on one map
// ---------------------------------------------------------

/// Weathers for one map
class WeatherSystem
{
    public:
        WeatherSystem(Map const* _map);
        ~WeatherSystem();

        Weather* FindOrCreateWeather(uint32 zoneId);
        void UpdateWeathers(uint32 diff);

    private:
        Map const* const m_map;

        typedef std::unordered_map<uint32 /*zoneId*/, Weather*> WeatherMap;
        WeatherMap m_weathers;
};

// ---------------------------------------------------------
//              Global Weather Information
// ---------------------------------------------------------

#define WEATHER_SEASONS 4
struct WeatherSeasonChances
{
    uint32 rainChance;
    uint32 snowChance;
    uint32 stormChance;
};

struct WeatherZoneChances
{
    WeatherSeasonChances data[WEATHER_SEASONS];
};

class WeatherMgr
{
    public:
        WeatherMgr() {};
        ~WeatherMgr() {};

        void LoadWeatherZoneChances();

        WeatherZoneChances const* GetWeatherChances(uint32 zone_id) const
        {
            WeatherZoneMap::const_iterator itr = mWeatherZoneMap.find(zone_id);
            if (itr != mWeatherZoneMap.end())
                return &itr->second;
            else
                return nullptr;
        }

    private:
        typedef std::unordered_map<uint32 /*zoneId*/, WeatherZoneChances> WeatherZoneMap;
        WeatherZoneMap      mWeatherZoneMap;
};

#define sWeatherMgr MaNGOS::Singleton<WeatherMgr>::Instance()

#endif
