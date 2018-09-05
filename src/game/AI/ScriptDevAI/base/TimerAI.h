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
    Timer(uint32 id, uint32 timer, std::function<void()> functor, bool combat, bool disabled = false) : id(id), timer(timer), disabled(disabled), combat(combat), functor(functor) {}
    uint32 id;
    uint32 timer;
    bool disabled;
    bool combat;
    std::function<void()> functor;

    bool UpdateTimer(const uint32 diff, bool combat);
};

/*
Not an AI in itself
Used for adding unified timer support to any AI
*/
class TimerAI
{
    public:
        TimerAI(uint32 maxCombatActions) : m_actionReadyStatus(maxCombatActions) {}

        void AddCombatAction(uint32 id, uint32 timer);
        void AddCustomAction(uint32 id, uint32 timer, std::function<void()> functor, bool disabled = false);

        void UpdateTimers(const uint32 diff, bool combat);
        virtual void ExecuteActions() = 0;

        inline void SetActionReadyStatus(uint32 index, bool state) { m_actionReadyStatus[index] = state; }
        inline bool GetActionReadyStatus(uint32 index) { return m_actionReadyStatus[index]; }
        inline void ResetTimer(uint32 index, uint32 timer)
        {
            auto data = m_timers.find(index);
            (*data).second.timer = timer; (*data).second.disabled = false;
        }
        inline void DisableTimer(uint32 index)
        {
            auto data = m_timers.find(index);
            (*data).second.timer = 0; (*data).second.disabled = true;
        }

        void GetAIInformation(ChatHandler& reader);

    private:
        std::map<uint32, Timer> m_timers;
        std::vector<bool> m_actionReadyStatus;
};
