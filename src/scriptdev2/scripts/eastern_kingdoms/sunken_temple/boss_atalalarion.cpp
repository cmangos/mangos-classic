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
SDName: Boss Atal'Alarion
SD%Complete: 90
SDComment: Atal'Alarion, statues, altar of hakkar
SDCategory: Sunken Temple
EndScriptData */

#include "precompiled.h"
#include "sunken_temple.h"

enum Spells
{
	SPELL_GROUND_TREMOR = 6524,
	SPELL_SWEPING_SLAM = 12887,
	SPELL_TRASH = 3391
};

struct boss_atalalarionAI : public ScriptedAI
{
	boss_atalalarionAI(Creature* pCreature) : ScriptedAI(pCreature)
	{
		m_pInstance = (instance_sunken_temple*)pCreature->GetInstanceData();
		Reset();
	}

	instance_sunken_temple* m_pInstance;

	uint32 m_uiGroundTremorTimer;
	uint32 m_uiSweepingSlamTimer;
	uint32 m_uiTrashTimer;

	void Reset()
	{
		m_uiGroundTremorTimer = urand(3000, 4000);
		m_uiSweepingSlamTimer = urand(4000, 6000);
		m_uiTrashTimer = urand(6000, 8000);
	}

	void Aggro(Unit* pWho/*pWho*/)
	{
	}

	void JustDied(Unit* pKiller/*pKiller*/)
	{
		if (m_pInstance)
			m_pInstance->SetData(TYPE_ATALARION, DONE);
	}

	void UpdateAI(const uint32 uiDiff)
	{
		if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
			return;

		// Grounding Tremor
		if (m_uiGroundTremorTimer < uiDiff)
		{
			DoCastSpellIfCan(m_creature->getVictim(), SPELL_GROUND_TREMOR);
			m_uiGroundTremorTimer = urand(7000, 9000);
		}
		else
			m_uiGroundTremorTimer -= uiDiff;

		// Sweeping Slam
		if (m_uiSweepingSlamTimer < uiDiff)
		{
			DoCastSpellIfCan(m_creature->getVictim(), SPELL_SWEPING_SLAM);
			m_uiSweepingSlamTimer = urand(5000, 7000);
		}
		else
			m_uiSweepingSlamTimer -= uiDiff;

		// Trash
		if (m_uiTrashTimer < uiDiff)
		{
			DoCastSpellIfCan(m_creature, SPELL_TRASH);
			m_uiTrashTimer = urand(6000, 8000);
		}
		else
			m_uiTrashTimer -= uiDiff;

		DoMeleeAttackIfReady();
	}
};

CreatureAI* GetAI_boss_atalalarion(Creature* pCreature)
{
	return new boss_atalalarionAI(pCreature);
}

bool GOUse_go_atalai_statue(Player* pPlayer, GameObject* pGo)
{
	if (instance_sunken_temple* m_pInstance = (instance_sunken_temple*)pGo->GetInstanceData())
	{
		//m_pInstance->HandleAtalaiStatue(pGo->GetEntry());
		pGo->SetGoState(GO_STATE_ACTIVE);
		pGo->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_INTERACT_COND + GO_FLAG_NO_INTERACT);
	}
	return true;
}

bool GossipHello_go_altar_of_hakkar(Player* pPlayer, GameObject* pGo)
{
	instance_sunken_temple* m_pInstance = (instance_sunken_temple*)pGo->GetInstanceData();

	if (pGo->GetGoType() == GAMEOBJECT_TYPE_QUESTGIVER)
	{
		pPlayer->PrepareQuestMenu(pGo->GetObjectGuid());
		pPlayer->SendPreparedQuest(pGo->GetObjectGuid());
	}

	if (m_pInstance && m_pInstance->GetData(TYPE_ATALARION) == IN_PROGRESS)
		pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Reset statues", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

	pPlayer->SEND_GOSSIP_MENU(pPlayer->GetGossipTextId(pGo), pGo->GetObjectGuid());
	return true;
}

bool GossipSelect_go_altar_of_hakkar(Player* pPlayer, GameObject* pGo, uint32 uiSender, uint32 uiAction)
{
	instance_sunken_temple* m_pInstance = (instance_sunken_temple*)pGo->GetInstanceData();

	if (m_pInstance && uiAction == GOSSIP_ACTION_INFO_DEF + 1)
	{
		m_pInstance->SetData(TYPE_ATALARION, NOT_STARTED);
		pPlayer->CLOSE_GOSSIP_MENU();
	}
	return true;
}

void AddSC_boss_atalalarion()
{
	Script* pNewScript;

	pNewScript = new Script;
	pNewScript->Name = "boss_atalalarion";
	pNewScript->GetAI = &GetAI_boss_atalalarion;
	pNewScript->RegisterSelf();

	pNewScript = new Script;
	pNewScript->Name = "go_atalai_statue";
	pNewScript->pGOUse = &GOUse_go_atalai_statue;
	pNewScript->RegisterSelf();

	pNewScript = new Script;
	pNewScript->Name = "go_altar_of_hakkar";
	pNewScript->pGossipHelloGO = &GossipHello_go_altar_of_hakkar;
	pNewScript->pGossipSelectGO = &GossipSelect_go_altar_of_hakkar;
	pNewScript->RegisterSelf();
}
