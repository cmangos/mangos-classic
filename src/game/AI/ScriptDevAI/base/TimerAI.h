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

#ifndef TIMER_AI_H
#define TIMER_AI_H

#include "Util.h"
#include "Platform/Define.h"

#include <functional>
#include <map>
#include <vector>

class ChatHandler;

/*
Timer data class used for execution of TimerAI events
*/
struct Timer
{
    Timer(uint32 id, std::function<void()> functor, uint32 timerMin, uint32 timerMax, bool disabled = false);
    uint32 id;
    uint32 timer;
    bool disabled;
    std::function<void()> functor;

    // initial settings
    uint32 initialMin, initialMax;
    bool initialDisabled;

    virtual bool UpdateTimer(const uint32 diff);
    void ResetTimer();
};

struct CombatTimer : public Timer
{
    CombatTimer(uint32 id, std::function<void()> functor, bool combat, uint32 timerMin, uint32 timerMax, bool disabled = false) : Timer(id, functor, timerMin, timerMax, disabled), combat(combat) {}

    bool combat;

    virtual bool UpdateTimer(const uint32 diff, bool combat);
};

/*
Not an AI in itself
Used for adding unified timer support to any AI
*/
class TimerManager
{
    public:
        TimerManager() {}

        // TODO: remove first function
        void AddCustomAction(uint32 id, bool disabled, std::function<void()> functor);
        void AddCustomAction(uint32 id, uint32 timer, std::function<void()> functor);
        void AddCustomAction(uint32 id, uint32 timerMin, uint32 timerMax, std::function<void()> functor);

        virtual void ResetTimer(uint32 index, uint32 timer)
        {
            auto data = m_timers.find(index);
            (*data).second.timer = timer; (*data).second.disabled = false;
        }
        virtual void DisableTimer(uint32 index)
        {
            auto data = m_timers.find(index);
            (*data).second.timer = 0; (*data).second.disabled = true;
        }

        virtual void UpdateTimers(const uint32 diff);
        virtual void ResetAllTimers();

        virtual void GetAIInformation(ChatHandler& reader);

    protected:
        void AddTimer(uint32 id, Timer&& timer);
    private:
        std::map<uint32, Timer> m_timers;
};

class CombatActions : public TimerManager
{
    public:
        CombatActions(uint32 maxCombatActions) : m_actionReadyStatus(maxCombatActions) {}

        // Family of methods which add combat action with reset settings saved
        // Adds a combat action which is always disabled at start
        void AddCombatAction(uint32 id, bool disabled);
        // Adds a combat action which is always reset to static timer value
        void AddCombatAction(uint32 id, uint32 timer);
        // Adds a combat action which is reset to a random number between min and max (inclusive)
        void AddCombatAction(uint32 id, uint32 timerMin, uint32 timerMax);
        // Adds a combat action which has no timer. It is reset to default value at start. Useful for one-off actions like phase transition at HP level.
        void AddTimerlessCombatAction(uint32 id, bool byDefault);

        virtual void ResetTimer(uint32 index, uint32 timer) override;
        virtual void DisableTimer(uint32 index) override;

        void DisableCombatAction(uint32 index)
        {
            DisableTimer(index);
            SetActionReadyStatus(index, false);
        }

        void ResetCombatAction(uint32 index, uint32 timer)
        {
            ResetTimer(index, timer);
            SetActionReadyStatus(index, false);
        }

        inline void SetActionReadyStatus(uint32 index, bool state) { m_actionReadyStatus[index] = state; }
        inline bool GetActionReadyStatus(uint32 index) { return m_actionReadyStatus[index]; }

        virtual void UpdateTimers(const uint32 diff, bool combat);
        virtual void ExecuteActions() = 0;
        virtual void ResetAllTimers() override;

        virtual void GetAIInformation(ChatHandler& reader) override;

        size_t GetCombatActionCount() { return m_actionReadyStatus.size(); }

    private:
        std::map<uint32, CombatTimer> m_CombatActions;
        std::vector<bool> m_actionReadyStatus;
        std::map<uint32, bool> m_timerlessActionSettings;
        std::map<uint32, uint32> m_spellAction;
};

#endif
