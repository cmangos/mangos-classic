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
        CreatureEventAIMgr() : m_usedTextsAmount(0) {};
        ~CreatureEventAIMgr() {};

        void LoadCreatureEventAI_Texts(bool check_entry_use);
        void LoadCreatureEventAI_Summons(bool check_entry_use);
        void LoadCreatureEventAI_Scripts();

        CreatureEventAI_Event_Map  const& GetCreatureEventAIMap()       const { return m_CreatureEventAI_Event_Map; }
        CreatureEventAI_Summon_Map const& GetCreatureEventAISummonMap() const { return m_CreatureEventAI_Summon_Map; }

    private:
        void CheckUnusedAITexts();
        void CheckUnusedAISummons();

        CreatureEventAI_Event_Map  m_CreatureEventAI_Event_Map;
        CreatureEventAI_Summon_Map m_CreatureEventAI_Summon_Map;

        uint32 m_usedTextsAmount;
};

#define sEventAIMgr MaNGOS::Singleton<CreatureEventAIMgr>::Instance()
#endif
