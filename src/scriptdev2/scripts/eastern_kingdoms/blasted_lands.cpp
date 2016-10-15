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

/* ScriptData
SDName: Blasted_Lands
SD%Complete: 90
SDComment: Quest support: 2784, 2801. Missing some texts for Fallen Hero. Teleporter to Rise of the Defiler missing group support.
SDCategory: Blasted Lands
EndScriptData */

/* ContentData
npc_fallen_hero_of_horde
EndContentData */

#include "precompiled.h"

enum
{
	NPC_RAZELIKH = 7668,
	NPC_SEVINE = 7671,
	NPC_ALLISTARJ = 7670,
	NPC_GROL = 7669,

	GOB_RAZELIKH = 141812,
	GOB_SEVINE = 141859,
	GOB_ALLISTARJ = 141858,
	GOB_GROL = 141857,

	SPELL_SELFSTUN = 9032
};

bool m_uiStoneDestroyed = false;
bool m_uiStoneEnabled = false;
GameObject* m_uiStoneGameObject;
uint32 m_uiStunTimer = 0;

struct npc_stone_servantAI : public ScriptedAI
{
	npc_stone_servantAI(Creature* pCreature) : ScriptedAI(pCreature) { Reset(); }

	void Reset() override
	{
		m_uiStoneDestroyed	= false;
		m_uiStoneEnabled	= false;
		m_uiStunTimer = 0;
	}

	GameObject* FindStoneOfBinding()
	{
		//Find nearest Stone of Binding object with an entry corresponding to the NPC entry.
		switch (m_creature->GetEntry())
		{
		case NPC_RAZELIKH:
			return GetClosestGameObjectWithEntry(m_creature, GOB_RAZELIKH, 40.0f);
			break;
		case NPC_SEVINE:
			return GetClosestGameObjectWithEntry(m_creature, GOB_SEVINE, 40.0f);
			break;
		case NPC_ALLISTARJ:
			return GetClosestGameObjectWithEntry(m_creature, GOB_ALLISTARJ, 40.0f);
			break;
		case NPC_GROL:
			return GetClosestGameObjectWithEntry(m_creature, GOB_GROL, 40.0f);
			break;
		default:
			return nullptr;
			break;
		}
	}

	void DamageTaken(Unit* /*pDoneBy*/, uint32& uiDamage) override
	{
		if (uiDamage < m_creature->GetHealth())
			return;

		if(m_creature->GetHealth() >= 1)
			m_creature->SetHealth((uint32)1);

		if (!m_uiStoneDestroyed)
		{
			uiDamage = 0;
		}
	}

	void UpdateAI(const uint32 uiDiff) override
	{
		if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
			return;

		if ((m_creature->GetHealthPercent() < 5.0f))
		{
			if (m_uiStunTimer <= 0 && DoCastSpellIfCan(m_creature, SPELL_SELFSTUN))
				m_uiStunTimer = 30000;
			else
				m_uiStunTimer -= uiDiff;

			if(!m_uiStoneEnabled)
				if (m_uiStoneGameObject = FindStoneOfBinding())
				{
					m_uiStoneGameObject->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_NO_INTERACT);
					m_uiStoneEnabled = true;
				}
		}
		DoMeleeAttackIfReady();
	}
};

bool GOUse_go_stone_of_binding(Player* pPlayer, GameObject* pGo)
{
	if (!(pGo == m_uiStoneGameObject))
		return false;

	m_uiStoneDestroyed = true;
	m_uiStoneEnabled = false;
	m_uiStoneGameObject->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_NO_INTERACT);

	return true;
}

CreatureAI* GetAI_npc_stone_servant(Creature* pCreature)
{
	return new npc_stone_servantAI(pCreature);
}

/*######
## npc_fallen_hero_of_horde
######*/

#define GOSSIP_ITEM_FALLEN "Continue..."

#define GOSSIP_ITEM_FALLEN1 "What could be worse than death?"
#define GOSSIP_ITEM_FALLEN2 "Subordinates?"
#define GOSSIP_ITEM_FALLEN3 "What are the stones of binding?"
#define GOSSIP_ITEM_FALLEN4 "You can count on me, Hero"
#define GOSSIP_ITEM_FALLEN5 "I shall"

bool GossipHello_npc_fallen_hero_of_horde(Player* pPlayer, Creature* pCreature)
{
    if (pCreature->isQuestGiver())
        pPlayer->PrepareQuestMenu(pCreature->GetObjectGuid());

    if (pPlayer->GetQuestStatus(2784) == QUEST_STATUS_INCOMPLETE)
        pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Why are you here?", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

    if (pPlayer->GetQuestStatus(2801) == QUEST_STATUS_INCOMPLETE && pPlayer->GetTeam() == HORDE)
        pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Continue story...", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);

    if (pPlayer->GetQuestStatus(2801) == QUEST_STATUS_INCOMPLETE && pPlayer->GetTeam() == ALLIANCE)
        pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Why are you here?", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

    pPlayer->SEND_GOSSIP_MENU(pPlayer->GetGossipTextId(pCreature), pCreature->GetObjectGuid());

    return true;
}

bool GossipSelect_npc_fallen_hero_of_horde(Player* pPlayer, Creature* pCreature, uint32 /*uiSender*/, uint32 uiAction)
{
    switch (uiAction)
    {
        case GOSSIP_ACTION_INFO_DEF+1:
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_FALLEN, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 11);
            pPlayer->SEND_GOSSIP_MENU(1392, pCreature->GetObjectGuid());
            break;
        case GOSSIP_ACTION_INFO_DEF+11:
            pPlayer->SEND_GOSSIP_MENU(1411, pCreature->GetObjectGuid());
            if (pPlayer->GetQuestStatus(2784) == QUEST_STATUS_INCOMPLETE)
                pPlayer->AreaExploredOrEventHappens(2784);
            if (pPlayer->GetTeam() == ALLIANCE)
            {
                pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_FALLEN, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
                pPlayer->SEND_GOSSIP_MENU(1411, pCreature->GetObjectGuid());
            }
            break;

        case GOSSIP_ACTION_INFO_DEF+2:
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_FALLEN, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 21);
            pPlayer->SEND_GOSSIP_MENU(1451, pCreature->GetObjectGuid());
            break;
        case GOSSIP_ACTION_INFO_DEF+21:
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_FALLEN1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 22);
            pPlayer->SEND_GOSSIP_MENU(1452, pCreature->GetObjectGuid());
            break;
        case GOSSIP_ACTION_INFO_DEF+22:
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_FALLEN2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 23);
            pPlayer->SEND_GOSSIP_MENU(1453, pCreature->GetObjectGuid());
            break;
        case GOSSIP_ACTION_INFO_DEF+23:
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_FALLEN3, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 24);
            pPlayer->SEND_GOSSIP_MENU(1454, pCreature->GetObjectGuid());
            break;
        case GOSSIP_ACTION_INFO_DEF+24:
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_FALLEN4, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 25);
            pPlayer->SEND_GOSSIP_MENU(1455, pCreature->GetObjectGuid());
            break;
        case GOSSIP_ACTION_INFO_DEF+25:
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_FALLEN5, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 26);
            pPlayer->SEND_GOSSIP_MENU(1456, pCreature->GetObjectGuid());
            break;
        case GOSSIP_ACTION_INFO_DEF+26:
            pPlayer->CLOSE_GOSSIP_MENU();
            pPlayer->AreaExploredOrEventHappens(2801);
            break;
    }
    return true;
}

void AddSC_blasted_lands()
{
    Script* pNewScript;

	pNewScript = new Script;
	pNewScript->Name = "npc_stone_servant";
	pNewScript->GetAI = &GetAI_npc_stone_servant;
	pNewScript->RegisterSelf();

	pNewScript = new Script;
	pNewScript->Name = "go_stone_of_binding";
	pNewScript->pGOUse = &GOUse_go_stone_of_binding;
	pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_fallen_hero_of_horde";
    pNewScript->pGossipHello =  &GossipHello_npc_fallen_hero_of_horde;
    pNewScript->pGossipSelect = &GossipSelect_npc_fallen_hero_of_horde;
    pNewScript->RegisterSelf();
}
