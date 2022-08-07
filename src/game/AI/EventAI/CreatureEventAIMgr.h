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

#ifndef MANGOS_CREATURE_EAI_MGR_H
#define MANGOS_CREATURE_EAI_MGR_H

#include "Common.h"
#include "CreatureEventAI.h"

class CreatureEventAIMgr
{
    public:
        CreatureEventAIMgr() {};
        ~CreatureEventAIMgr() {};

        void LoadCreatureEventAI_Summons(bool check_entry_use);
        void LoadCreatureEventAI_Scripts();

        std::shared_ptr<CreatureEventAI_Event_Map> const& GetCreatureEventEntryAIMap() const { return m_CreatureEventAIEventEntryMap; }
        std::shared_ptr<CreatureEventAI_Event_Map> const& GetCreatureEventGuidAIMap()  const { return m_CreatureEventAIEventGuidMap; }
        CreatureEventAI_Summon_Map const& GetCreatureEventAISummonMap() const { return m_CreatureEventAI_Summon_Map; }
        std::shared_ptr<CreatureEventAI_EventComputedData_Map> const& GetEAIComputedDataMap() const { return m_creatureEventAIComputedDataMap; }

    private:
        void CheckUnusedAISummons();

        std::shared_ptr<CreatureEventAI_Event_Map>  m_CreatureEventAIEventEntryMap;
        std::shared_ptr<CreatureEventAI_Event_Map>  m_CreatureEventAIEventGuidMap;
        CreatureEventAI_Summon_Map m_CreatureEventAI_Summon_Map; // deprecated
        std::shared_ptr<CreatureEventAI_EventComputedData_Map> m_creatureEventAIComputedDataMap;
};

#define sEventAIMgr MaNGOS::Singleton<CreatureEventAIMgr>::Instance()
#endif
