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

#ifndef MANGOS_CODEBENCH_H
#define MANGOS_CODEBENCH_H

struct ChronoTimeTracker
{
    public:
        ChronoTimeTracker() : m_startTime(std::chrono::steady_clock::now()), m_name("Unnamed Timer") {}
        ChronoTimeTracker(std::string name) : m_startTime(std::chrono::steady_clock::now()), m_name(name) {}
        ~ChronoTimeTracker();
        std::chrono::seconds elapsedSeconds()
        {
            return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - m_startTime);
        }
        std::chrono::milliseconds elapsedMillis()
        {
            return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - m_startTime);
        }
        std::chrono::microseconds elapsedMicros()
        {
            return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - m_startTime);
        }
        std::chrono::nanoseconds elapsedNanos()
        {
            return std::chrono::steady_clock::now() - m_startTime;
        }
    private:
        std::chrono::time_point<std::chrono::steady_clock, std::chrono::nanoseconds> m_startTime;
        std::string m_name;
        inline constexpr std::chrono::nanoseconds nanos(std::chrono::nanoseconds nanos) { return nanos; }
        inline constexpr std::chrono::microseconds micros(std::chrono::nanoseconds nanos) { return std::chrono::duration_cast<std::chrono::microseconds>(nanos); }
        inline constexpr std::chrono::milliseconds millis(std::chrono::nanoseconds nanos) { return std::chrono::duration_cast<std::chrono::milliseconds>(nanos); }
        inline constexpr std::chrono::seconds secs(std::chrono::nanoseconds nanos) { return std::chrono::duration_cast<std::chrono::seconds>(nanos); }
};

#endif

