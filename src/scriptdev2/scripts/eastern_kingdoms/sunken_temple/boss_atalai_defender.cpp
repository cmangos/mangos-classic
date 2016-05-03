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
SDName: Atal'ai Defenders
SD%Complete: 90
SDComment: Gasher, Hukku, Loro, Mijan, Zolo, Zul'Lor
SDCategory: Sunken Temple
EndScriptData */

#include "precompiled.h"
#include "sunken_temple.h"

enum Spells
{
	// Gasher
	SPELL_TRASH = 3391,
	// Hukku
	SPELL_CURSE_OF_BLOOD = 12279,
	SPELL_HUKKUS_GUARDIANS = 12790,
	SPELL_SHADOW_BOLT = 12471,
	SPELL_SHADOW_BOLT_VOLLEY = 9081,
	// Loro
	SPELL_IMPROVED_BLOCKING = 3639,
	SPELL_SHIELD_SLAM = 8242,
	// Mijan
	SPELL_HEALING_WARD = 11899,
	SPELL_HEALING_WAVE = 12492,
	SPELL_RENEW = 8362,
	SPELL_THORNS_AURA = 8148,
	// Zolo
	SPELL_ATALAI_SKELETON_TOTEM = 12506,
	SPELL_CHAIN_LIGHTNING = 12058,
	// Zul'Lor
	SPELL_CLEAVE = 15496,
	SPELL_FRAILTY = 12530
};

struct boss_atalai_defenderAI : public ScriptedAI
{
	boss_atalai_defenderAI(Creature* pCreature) : ScriptedAI(pCreature)
	{
		m_pInstance = (instance_sunken_temple*)pCreature->GetInstanceData();
		Reset();
	}

	instance_sunken_temple* m_pInstance;

	uint32 m_uiSpell1Timer;
	uint32 m_uiSpell2Timer;
	uint32 m_uiSpell3Timer;
	uint32 m_uiSpell4Timer;

	void Reset()
	{
		m_uiSpell1Timer = urand(1000, 3000);
		m_uiSpell2Timer = urand(4000, 6000);
		m_uiSpell3Timer = urand(8000, 10000);
		m_uiSpell4Timer = urand(12000, 14000);
	}

	void Aggro(Unit* /*pWho*/)
	{
	}

	void JustDied(Unit* /*pKiller*/)
	{
		if (m_pInstance)
			m_pInstance->SetData(TYPE_ATALAI_DEFENDERS, m_pInstance->GetData(TYPE_ATALAI_DEFENDERS) + 1);
	}

	void UpdateAI(const uint32 uiDiff)
	{
		if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
			return;

		switch (m_creature->GetEntry())
		{
		case NPC_GASHER:
			// Trash
			if (m_uiSpell1Timer < uiDiff)
			{
				DoCastSpellIfCan(m_creature, SPELL_TRASH);
				m_uiSpell1Timer = urand(3000, 4000);
			}
			else
				m_uiSpell1Timer -= uiDiff;
			break;
		case NPC_HUKKU:
			// Shadow Bolt
			if (m_uiSpell1Timer < uiDiff)
			{
				DoCastSpellIfCan(m_creature->getVictim(), SPELL_SHADOW_BOLT);
				m_uiSpell1Timer = urand(5000, 7000);
			}
			else
				m_uiSpell1Timer -= uiDiff;

			// Guardians
			if (m_uiSpell2Timer < uiDiff)
			{
				DoCastSpellIfCan(m_creature->getVictim(), SPELL_HUKKUS_GUARDIANS);
				m_uiSpell2Timer = 30000;
			}
			else
				m_uiSpell2Timer -= uiDiff;

			// Curse Of Blood
			if (m_uiSpell3Timer < uiDiff)
			{
				if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
					DoCastSpellIfCan(pTarget, SPELL_CURSE_OF_BLOOD);
				m_uiSpell3Timer = urand(10000, 15000);
			}
			else
				m_uiSpell3Timer -= uiDiff;

			// Shadow Bolt Volley
			if (m_uiSpell4Timer < uiDiff)
			{
				DoCastSpellIfCan(m_creature->getVictim(), SPELL_SHADOW_BOLT_VOLLEY);
				m_uiSpell4Timer = urand(7000, 9000);
			}
			else
				m_uiSpell4Timer -= uiDiff;
			break;
		case NPC_LORO:
			// Improved Blocking
			if (m_uiSpell1Timer < uiDiff)
			{
				DoCastSpellIfCan(m_creature, SPELL_IMPROVED_BLOCKING);
				m_uiSpell1Timer = urand(10000, 15000);
			}
			else
				m_uiSpell1Timer -= uiDiff;

			// Shield Slam
			if (m_uiSpell2Timer < uiDiff)
			{
				DoCastSpellIfCan(m_creature->getVictim(), SPELL_SHIELD_SLAM);
				m_uiSpell2Timer = urand(5000, 7000);
			}
			else
				m_uiSpell2Timer -= uiDiff;
			break;
		case NPC_MIJAN:
			// Healing Ward
			if (m_uiSpell1Timer < uiDiff)
			{
				DoCastSpellIfCan(m_creature, SPELL_HEALING_WARD);
				m_uiSpell1Timer = 25000;
			}
			else
				m_uiSpell1Timer -= uiDiff;

			// Healing Wave
			if (m_uiSpell2Timer < uiDiff)
			{
				DoCastSpellIfCan(m_creature, SPELL_HEALING_WAVE);
				m_uiSpell2Timer = urand(7000, 9000);
			}
			else
				m_uiSpell2Timer -= uiDiff;

			// Renew
			if (m_uiSpell3Timer < uiDiff)
			{
				DoCastSpellIfCan(m_creature, SPELL_RENEW);
				m_uiSpell3Timer = urand(5000, 7000);
			}
			else
				m_uiSpell3Timer -= uiDiff;

			// Thorns Aura
			if (m_uiSpell4Timer < uiDiff)
			{
				DoCastSpellIfCan(m_creature, SPELL_THORNS_AURA);
				m_uiSpell4Timer = urand(10000, 15000);
			}
			else
				m_uiSpell4Timer -= uiDiff;
			break;
		case NPC_ZOLO:
			// Atalai Skeleton Totem
			if (m_uiSpell1Timer < uiDiff)
			{
				DoCastSpellIfCan(m_creature, SPELL_ATALAI_SKELETON_TOTEM);
				m_uiSpell1Timer = urand(10000, 15000);
			}
			else
				m_uiSpell1Timer -= uiDiff;

			// Chain Lightning
			if (m_uiSpell2Timer < uiDiff)
			{
				DoCastSpellIfCan(m_creature->getVictim(), SPELL_CHAIN_LIGHTNING);
				m_uiSpell2Timer = urand(5000, 7000);
			}
			else
				m_uiSpell2Timer -= uiDiff;
			break;
		case NPC_ZULLOR:
			// Cleave
			if (m_uiSpell1Timer < uiDiff)
			{
				DoCastSpellIfCan(m_creature->getVictim(), SPELL_CLEAVE);
				m_uiSpell1Timer = urand(4000, 6000);
			}
			else
				m_uiSpell1Timer -= uiDiff;

			// Frailty
			if (m_uiSpell2Timer < uiDiff)
			{
				DoCastSpellIfCan(m_creature->getVictim(), SPELL_FRAILTY);
				m_uiSpell2Timer = urand(15000, 20000);
			}
			else
				m_uiSpell2Timer -= uiDiff;
			break;
		}

		DoMeleeAttackIfReady();
	}
};
CreatureAI* GetAI_boss_atalai_defender(Creature* pCreature)
{
	return new boss_atalai_defenderAI(pCreature);
}

void AddSC_boss_atalai_defender()
{
	Script* pNewscript;
	pNewscript = new Script;
	pNewscript->Name = "boss_atalai_defender";
	pNewscript->GetAI = &GetAI_boss_atalai_defender;
	pNewscript->RegisterSelf();
}
