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
SDName: Boss Jammal'an The Prophet and Ogom the Wretched
SD%Complete: 90
SDComment: Jammal'an, Ogom
SDCategory: Sunken Temple
EndScriptData */

#include "precompiled.h"
#include "sunken_temple.h"

enum Spells
{
	// Jammal'an 5710
	SPELL_EARTHGRAB_TOTEM = 8376,
	SPELL_FLAMESTRIKE = 12468,
	SPELL_HEALING_WAVE = 12492,
	SPELL_HEX_OF_JAMMALAN_TRANSFORM = 12480,
	SPELL_HEX_OF_JAMMALAN_CHARM = 12483,

	// Ogom 5711
	SPELL_CURSE_OF_WEAKNESS = 12493,
	SPELL_SHADOW_BOLT = 12471,
	SPELL_SHADOW_WORD_PAIN = 11639
};

enum Yells
{
	SAY_HEX = -1109005
};

struct boss_jammalan_the_prophetAI : public ScriptedAI
{
	boss_jammalan_the_prophetAI(Creature* pCreature) : ScriptedAI(pCreature)
	{
		m_pInstance = (instance_sunken_temple*)pCreature->GetInstanceData();
		Reset();
	}

	instance_sunken_temple* m_pInstance;

	uint32 m_uiEarthgrabTotemTimer;
	uint32 m_uiFlamestrikeTimer;
	uint32 m_uiHealingWaveTimer;
	uint32 m_uiHexOfJammalanTimer;

	void Reset()
	{
		m_uiEarthgrabTotemTimer = urand(3000, 4000);
		m_uiFlamestrikeTimer = urand(4000, 6000);
		m_uiHealingWaveTimer = urand(6000, 8000);
		m_uiHexOfJammalanTimer = 10000;
		if (m_pInstance)
		{
			Creature* pOgom = m_pInstance->GetSingleCreatureFromStorage(NPC_OGOM_THE_WRETCHED);
			if (pOgom && pOgom->isDead())
				pOgom->Respawn();
		}
	}

	void JustReachedHome()
	{
		if (m_pInstance)
			m_pInstance->SetData(TYPE_JAMMALAN, NOT_STARTED);
	}

	void Aggro(Unit* /*pWho*/)
	{
		if (m_pInstance)
			m_pInstance->SetData(TYPE_JAMMALAN, IN_PROGRESS);
	}

	void JustDied(Unit* /*pKiller*/)
	{
		if (m_pInstance)
			m_pInstance->SetData(TYPE_JAMMALAN, DONE);
	}

	void UpdateAI(const uint32 uiDiff)
	{
		if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
			return;

		// Earthgrab Totem
		if (m_uiEarthgrabTotemTimer < uiDiff)
		{
			DoCastSpellIfCan(m_creature, SPELL_EARTHGRAB_TOTEM);
			m_uiEarthgrabTotemTimer = 20000;
		}
		else
			m_uiEarthgrabTotemTimer -= uiDiff;

		// Flamestrike
		if (m_uiFlamestrikeTimer < uiDiff)
		{
			DoCastSpellIfCan(m_creature->getVictim(), SPELL_FLAMESTRIKE);
			m_uiFlamestrikeTimer = urand(4000, 6000);
		}
		else
			m_uiFlamestrikeTimer -= uiDiff;

		// Healing Wave
		if (m_uiHealingWaveTimer < uiDiff)
		{
			DoCastSpellIfCan(m_creature, SPELL_HEALING_WAVE);
			m_uiHealingWaveTimer = urand(6000, 8000);
		}
		else
			m_uiHealingWaveTimer -= uiDiff;

		// Hex of Jammal'an
		if (m_uiHexOfJammalanTimer < uiDiff)
		{
			if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
			{
				if (DoCastSpellIfCan(pTarget, SPELL_HEX_OF_JAMMALAN_TRANSFORM, CAST_TRIGGERED) == CAST_OK &&
					DoCastSpellIfCan(pTarget, SPELL_HEX_OF_JAMMALAN_CHARM, CAST_TRIGGERED) == CAST_OK)
				{
					DoScriptText(SAY_HEX, m_creature);
					m_uiHexOfJammalanTimer = urand(15000, 20000);
				}
			}
		}
		else
			m_uiHexOfJammalanTimer -= uiDiff;

		DoMeleeAttackIfReady();
	}
};

CreatureAI* GetAI_boss_jammalan_the_prophet(Creature* pCreature)
{
	return new boss_jammalan_the_prophetAI(pCreature);
}

struct boss_ogom_the_wretchedAI : public ScriptedAI
{
	boss_ogom_the_wretchedAI(Creature* pCreature) : ScriptedAI(pCreature)
	{
		Reset();
	}

	uint32 m_uiCurseOfWeaknessTimer;
	uint32 m_uiShadowBoltTimer;
	uint32 m_uiShadowWordPainTimer;

	void Reset()
	{
		m_uiCurseOfWeaknessTimer = urand(2000, 3000);
		m_uiShadowBoltTimer = urand(4000, 6000);
		m_uiShadowWordPainTimer = urand(6000, 8000);
	}

	void Aggro(Unit* /*pWho*/)
	{
	}

	void UpdateAI(const uint32 uiDiff)
	{
		if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
			return;

		// Curse of Weakness
		if (m_uiCurseOfWeaknessTimer < uiDiff)
		{
			if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
				DoCastSpellIfCan(pTarget, SPELL_CURSE_OF_WEAKNESS);
			m_uiCurseOfWeaknessTimer = 15000;
		}
		else
			m_uiCurseOfWeaknessTimer -= uiDiff;

		// Shadow Bolt
		if (m_uiShadowBoltTimer < uiDiff)
		{
			DoCastSpellIfCan(m_creature->getVictim(), SPELL_SHADOW_BOLT);
			m_uiShadowBoltTimer = urand(5000, 7000);
		}
		else
			m_uiShadowBoltTimer -= uiDiff;

		// Shadow Word: Pain
		if (m_uiShadowWordPainTimer < uiDiff)
		{
			if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
				DoCastSpellIfCan(pTarget, SPELL_SHADOW_WORD_PAIN);
			m_uiShadowWordPainTimer = urand(7000, 9000);
		}
		else
			m_uiShadowWordPainTimer -= uiDiff;

		DoMeleeAttackIfReady();
	}
};
CreatureAI* GetAI_boss_ogom_the_wretched(Creature* pCreature)
{
	return new boss_ogom_the_wretchedAI(pCreature);
}

void AddSC_boss_jammalan_ogom()
{
	Script* pNewscript;

	pNewscript = new Script;
	pNewscript->Name = "boss_jammalan_the_prophet";
	pNewscript->GetAI = &GetAI_boss_jammalan_the_prophet;
	pNewscript->RegisterSelf();

	pNewscript = new Script;
	pNewscript->Name = "boss_ogom_the_wretched";
	pNewscript->GetAI = &GetAI_boss_ogom_the_wretched;
	pNewscript->RegisterSelf();
}
