/* Copyright (C) 2006 - 2011 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

/* ScriptData
SDName: Atal'ai Dragons
SD%Complete: 90
SDComment: Dreamscythe, Weaver, Morphaz, Hazzas
SDCategory: Sunken Temple
EndScriptData */

#include "precompiled.h"
#include "sunken_temple.h"

enum Spells
{
	SPELL_ACID_BREATH = 12884,
	SPELL_WING_FLAP = 12882
};

struct boss_atalai_dragonAI : public ScriptedAI
{
	boss_atalai_dragonAI(Creature* pCreature) : ScriptedAI(pCreature)
	{
		Reset();
	}

	uint32 m_uiAcidBreathTimer;
	uint32 m_uiWingFlapTimer;

	void Reset()
	{
		m_uiAcidBreathTimer = urand(2000, 3000);
		m_uiWingFlapTimer = urand(4000, 6000);
	}

	void Aggro(Unit* /*pWho*/)
	{
	}

	void UpdateAI(const uint32 uiDiff)
	{
		if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
			return;

		// Acid Breath
		if (m_uiAcidBreathTimer < uiDiff)
		{
			DoCastSpellIfCan(m_creature->getVictim(), SPELL_ACID_BREATH);
			m_uiAcidBreathTimer = urand(4000, 6000);
		}
		else
			m_uiAcidBreathTimer -= uiDiff;

		// Wing Flap
		if (m_uiWingFlapTimer < uiDiff)
		{
			DoCastSpellIfCan(m_creature->getVictim(), SPELL_WING_FLAP);
			m_uiWingFlapTimer = urand(7000, 9000);
		}
		else
			m_uiWingFlapTimer -= uiDiff;

		DoMeleeAttackIfReady();
	}
};
CreatureAI* GetAI_boss_atalai_dragon(Creature* pCreature)
{
	return new boss_atalai_dragonAI(pCreature);
}

void AddSC_boss_atalai_dragon()
{
	Script* pNewscript;
	pNewscript = new Script;
	pNewscript->Name = "boss_atalai_dragon";
	pNewscript->GetAI = &GetAI_boss_atalai_dragon;
	pNewscript->RegisterSelf();
}
