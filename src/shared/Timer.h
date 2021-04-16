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

#ifndef MANGOS_TIMER_H
#define MANGOS_TIMER_H

#include "Common.h"
#include <chrono>

inline std::chrono::steady_clock::time_point GetApplicationStartTime()
{
    using namespace std::chrono;

    static const steady_clock::time_point ApplicationStartTime = steady_clock::now();

    return ApplicationStartTime;
}

class WorldTimer
{
    public:

        // get current server time
        static uint32 getMSTime();

        static inline uint32 getMSTimeDiff(const uint32& oldMSTime, std::chrono::steady_clock::time_point newTime)
        {
            using namespace std::chrono;

            uint32 newMSTime = uint32(duration_cast<milliseconds>(newTime - GetApplicationStartTime()).count());
            return getMSTimeDiff(oldMSTime, newMSTime);
        }

        // get time difference between two timestamps
        static inline uint32 getMSTimeDiff(const uint32& oldMSTime, const uint32& newMSTime)
        {
            if (oldMSTime > newMSTime)
            {
                const uint32 diff_1 = (uint32(0xFFFFFFFF) - oldMSTime) + newMSTime;
                const uint32 diff_2 = oldMSTime - newMSTime;

                return std::min(diff_1, diff_2);
            }

            return newMSTime - oldMSTime;
        }

        // get last world tick time
        static uint32 tickTime();

        // get previous world tick time
        static uint32 tickPrevTime();

        // tick world timer
        static uint32 tick();

     private:
        static uint32 m_iTime;
        static uint32 m_iPrevTime;
};

class IntervalTimer
{
    public:
        IntervalTimer() : _interval(0), _current(0) {}

        void Update(time_t diff)
        {
            _current += diff;
            if (_current < 0)
                _current = 0;
        }
        bool Passed() const { return _current >= _interval; }
        void Reset()
        {
            if (_current >= _interval)
                _current -= _interval;
        }

        void SetCurrent(time_t current) { _current = current; }
        void SetInterval(time_t interval) { _interval = interval; }
        time_t GetInterval() const { return _interval; }
        time_t GetCurrent() const { return _current; }

    private:
        time_t _interval;
        time_t _current;
};

class ShortIntervalTimer
{
    public:
        ShortIntervalTimer() : _interval(0), _current(0) {}

        void Update(uint32 diff)
        {
            _current += diff;
        }

        bool Passed() const { return _current >= _interval; }
        void Reset()
        {
            if (_current >= _interval)
                _current -= _interval;
        }

        void SetCurrent(uint32 current) { _current = current; }
        void SetInterval(uint32 interval) { _interval = interval; }
        uint32 GetInterval() const { return _interval; }
        uint32 GetCurrent() const { return _current; }

    private:
        uint32 _interval;
        uint32 _current;
};

struct TimeTracker
{
    public:
        TimeTracker(time_t expiry) : i_expiryTime(expiry) {}
        void Update(time_t diff) { i_expiryTime -= diff; }
        bool Passed() const { return (i_expiryTime <= 0); }
        void Reset(time_t interval) { i_expiryTime = interval; }
        time_t GetExpiry() const { return i_expiryTime; }

    private:
        time_t i_expiryTime;
};

struct ShortTimeTracker
{
    public:
        ShortTimeTracker(uint32 expiry = 0) : i_expiryTime(expiry) {}
        void Update(uint32 diff)
        {
            if (i_expiryTime <= diff)
                i_expiryTime = 0;
            else
                i_expiryTime -= diff;
        }
        bool Passed() const { return (i_expiryTime <= 0); }
        void Reset(uint32 interval) { i_expiryTime = interval; }
        uint32 GetExpiry() const { return i_expiryTime; }

    private:
        uint32 i_expiryTime;
};

#endif
