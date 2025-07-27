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

#ifndef BOSS_AI_H
#define BOSS_AI_H

#include "AI/ScriptDevAI/include/sc_instance.h"
#include "Entities/Creature.h"
#include "AI/ScriptDevAI/include/sc_creature.h"
#include "AI/ScriptDevAI/base/CombatAI.h"

enum InstanceActions
{
    INSTANCE_CLOSE_ENTRANCE_DOOR = 250,
};

class BossAI : public CombatAI
{
    public:
        BossAI(Creature* creature, uint32 combatActions) : CombatAI(creature, combatActions)
        {
            AddCustomAction(INSTANCE_CLOSE_ENTRANCE_DOOR, true, [&]()
            {
                OpenEntrances(false);
            });
        }

        void OpenEntrances(bool open = true)
        {
            ScriptedInstance* instance = dynamic_cast<ScriptedInstance*>(m_creature->GetInstanceData());
            if (!instance)
                return;

            for (auto& id : m_entranceObjects)
                instance->DoUseOpenableObject(id, open);
        }
        void OpenExits()
        {
            ScriptedInstance* instance = dynamic_cast<ScriptedInstance*>(m_creature->GetInstanceData());
            if (!instance)
                return;

            for (auto& id : m_exitObjects)
                instance->DoUseOpenableObject(id, true);
        }
        /**
        * Adds one or more Broadcast Texts to possibly emit when Unit dies
        * This function is not called if JustDied is overridden. Add CombatAI::JustDied(); to your overriding function.
        * @param text Broadcast Text ID
        */
        void AddOnDeathText(uint32 text);
        template<typename... Targs>
        void AddOnDeathText(uint32 value, Targs... fargs)
        {
            AddOnDeathText(value);
            AddOnDeathText(fargs...);
        }
        /**
        * Adds one or more Broadcast Texts to possibly emit when Unit enters combat
        * This function is not called if Aggro is overridden. Add CombatAI::Aggro(); to your overriding function.
        * @param text Broadcast Text ID
        */
        void AddOnAggroText(uint32 text);
        template<typename... Targs>
        void AddOnAggroText(uint32 value, Targs... fargs)
        {
            AddOnAggroText(value);
            AddOnAggroText(fargs...);
        }

        void SetDataType(uint32 type) { m_instanceDataType = type; }

        void JustDied(Unit* killer = nullptr) override;
        void JustReachedHome() override;
        void Aggro(Unit* who = nullptr) override;
        void AddEntranceObject(uint32 value);
        template <typename... Targs>
        void AddEntranceObject(uint32 value, Targs... fargs)
        {
            AddEntranceObject(value);
            AddEntranceObject(fargs...);
        }
        void AddExitObject(uint32 value);
        template <typename... Targs>
        void AddExitObject(uint32 value, Targs... fargs)
        {
            AddExitObject(value);
            AddExitObject(fargs...);
        }
        void SetGateDelay(std::chrono::milliseconds delay) { m_gateDelay = delay; }

        std::chrono::seconds TimeSinceEncounterStart()
        {
            return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - m_combatStartTimestamp);
        }

        static std::chrono::milliseconds RandomTimer(std::chrono::milliseconds min, std::chrono::milliseconds max)
        {
            return std::chrono::milliseconds(urand(min.count(), max.count()));
        }
    private:
        std::vector<uint32> m_onKilledTexts;
        std::vector<uint32> m_onAggroTexts;

        std::vector<uint32> m_entranceObjects;
        std::vector<uint32> m_exitObjects;
        std::chrono::milliseconds m_gateDelay = 3s;

        uint32 m_instanceDataType = -1;

        std::chrono::steady_clock::time_point m_combatStartTimestamp;
};

#endif