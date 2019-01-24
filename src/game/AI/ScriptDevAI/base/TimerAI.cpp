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
#include "Chat/Chat.h"
#include <string>

Timer::Timer(uint32 id, std::function<void()> functor, uint32 timerMin, uint32 timerMax, bool disabled)
    : id(id), timer(urand(timerMin, timerMax)), disabled(disabled), functor(functor), initialMin(timerMin), initialMax(timerMax), initialDisabled(disabled)
    {}

bool Timer::UpdateTimer(const uint32 diff)
{
    if (disabled)
        return false;

    if (timer <= diff)
    {
        timer = 0;
        disabled = true;
        return true;
    }
    else timer -= diff;

    return false;
}

void Timer::ResetTimer()
{
    timer = urand(initialMin, initialMax);
    disabled = initialDisabled;
}

bool CombatTimer::UpdateTimer(const uint32 diff, bool combat)
{
    if (disabled)
        return false;

    if (this->combat && !combat)
        return false;

    if (timer <= diff)
    {
        timer = 0;
        disabled = true;
        return true;
    }
    else timer -= diff;

    return false;
}

void TimerManager::AddTimer(uint32 id, Timer&& timer)
{
    m_timers.emplace(id, timer);
}

void TimerManager::AddCustomAction(uint32 id, bool disabled, std::function<void()> functor)
{
    m_timers.emplace(id, Timer(id, functor, 0, 0, false));
}

void TimerManager::AddCustomAction(uint32 id, uint32 timer, std::function<void()> functor)
{
    m_timers.emplace(id, Timer(id, functor, timer, timer, false));
}

void TimerManager::AddCustomAction(uint32 id, uint32 timerMin, uint32 timerMax, std::function<void()> functor)
{
    m_timers.emplace(id, Timer(id, functor, timerMin, timerMax, false));
}

void TimerManager::UpdateTimers(const uint32 diff)
{
    for (auto& data : m_timers)
    {
        Timer& timer = data.second;
        if (timer.UpdateTimer(diff))
            timer.functor();
    }
}

void TimerManager::ResetAllTimers()
{
    for (auto& data : m_timers)
        data.second.ResetTimer();
}

void TimerManager::GetAIInformation(ChatHandler& reader)
{
    reader.PSendSysMessage("TimerAI: Timers:");
    std::string output = "";
    for (auto itr = m_timers.begin(); itr != m_timers.end(); ++itr)
    {
        Timer& timer = (*itr).second;
        output += "Timer ID: " + std::to_string(timer.id) + " Timer: " + std::to_string(timer.timer), +" Disabled: " + std::to_string(timer.disabled) + "\n";
    }
    reader.PSendSysMessage("%s", output.data());
}

void CombatTimerAI::UpdateTimers(const uint32 diff, bool combat)
{
    TimerManager::UpdateTimers(diff);
    for (auto& data : m_combatTimers)
    {
        CombatTimer& timer = data.second;
        if (timer.UpdateTimer(diff, combat))
            timer.functor();
    }
}

void CombatTimerAI::ResetAllTimers()
{
    for (uint32 i = 0; i < m_actionReadyStatus.size(); ++i)
    {
        auto itr = m_timerlessActionSettings.find(i);
        if (itr == m_timerlessActionSettings.end())
            m_actionReadyStatus[i] = false;
        else
            m_actionReadyStatus[i] = (*itr).second;
    }
    for (auto& data : m_combatTimers)
        data.second.ResetTimer();
    TimerManager::ResetAllTimers();
}

void CombatTimerAI::AddCombatAction(uint32 id, bool disabled)
{
    m_combatTimers.emplace(id, CombatTimer(id, [&, id] { m_actionReadyStatus[id] = true; }, true, 0, 0, disabled));
}

void CombatTimerAI::AddCombatAction(uint32 id, uint32 timer)
{
    m_combatTimers.emplace(id, CombatTimer(id, [&, id] { m_actionReadyStatus[id] = true; }, true, timer, timer, false));
}

void CombatTimerAI::AddCombatAction(uint32 id, uint32 timerMin, uint32 timerMax)
{
    m_combatTimers.emplace(id, CombatTimer(id, [&, id] { m_actionReadyStatus[id] = true; }, true, timerMin, timerMax, false));
}

void CombatTimerAI::AddTimerlessCombatAction(uint32 id, bool byDefault)
{
    m_timerlessActionSettings[id] = byDefault;
}

void CombatTimerAI::GetAIInformation(ChatHandler& reader)
{
    reader.PSendSysMessage("Combat Timers:");
    std::string output = "";
    for (auto itr = m_combatTimers.begin(); itr != m_combatTimers.end(); ++itr)
    {
        Timer& timer = (*itr).second;
        output += "Timer ID: " + std::to_string(timer.id) + " Timer: " + std::to_string(timer.timer), +" Disabled: " + std::to_string(timer.disabled) + "\n";
    }
    reader.PSendSysMessage("%s", output.data());
}