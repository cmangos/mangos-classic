/* This file is part of the ScriptDev2 Project. See AUTHORS file for Copyright information
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

#include "TimerAI.h"

bool Timer::UpdateTimer(const uint32 diff)
{
    if (!disabled)
    {
        if (timer <= diff)
        {
            timer = 0;
            disabled = true;
            return true;
        }
        else timer -= diff;
    }
    return false;
}

void TimerAI::AddCombatAction(uint32 id, uint32 timer)
{
    m_timers.emplace(id, Timer(id, timer, [&, id] { m_actionReadyStatus[id] = true; }));
}

void TimerAI::AddCustomAction(uint32 id, uint32 timer, std::function<void()> functor, bool disabled)
{
    m_timers.emplace(id, Timer(id, timer, functor, disabled));
}

void TimerAI::UpdateTimers(const uint32 diff)
{
    for (auto itr = m_timers.begin(); itr != m_timers.end();)
    {
        Timer& timer = (*itr).second;
        if (timer.UpdateTimer(diff))
            timer.functor();
        else ++itr;
    }
}