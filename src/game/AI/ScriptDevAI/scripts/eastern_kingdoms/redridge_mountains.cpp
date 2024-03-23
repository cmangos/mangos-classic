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
SDName: redridge_mountains
SD%Complete: 100%
SDComment: Quest support: 219
SDCategory: Redridge Mountains
EndScriptData

*/

#include "AI/ScriptDevAI/include/sc_common.h"/* ContentData
npc_corporal_keeshan */


#include "AI/ScriptDevAI/base/escort_ai.h"

/*######
## npc_corporal_keehsan
######*/

enum
{
    QUEST_MISSING_IN_ACTION = 219,

    KEESHAN_PATH_ID         = 349,
    SAY_CORPORAL_KEESHAN_1  = 25,
    SAY_CORPORAL_KEESHAN_2  = 26,
    SAY_CORPORAL_KEESHAN_3  = 27,
    SAY_CORPORAL_KEESHAN_4  = 30,
};

struct npc_corporal_keeshan_escortAI : public npc_escortAI
{
    npc_corporal_keeshan_escortAI(Creature* pCreature) : npc_escortAI(pCreature) { Reset(); }


    void ReceiveAIEvent(AIEventType eventType, Unit* /*pSender*/, Unit* pInvoker, uint32 uiMiscValue) override
    {
        if (eventType == AI_EVENT_START_ESCORT && pInvoker->GetTypeId() == TYPEID_PLAYER)
        {
            DoBroadcastText(SAY_CORPORAL_KEESHAN_1, m_creature, pInvoker);
            m_creature->SetFactionTemporary(FACTION_ESCORT_A_NEUTRAL_ACTIVE, TEMPFACTION_RESTORE_RESPAWN);
            Start(false, (Player*)pInvoker, GetQuestTemplateStore(uiMiscValue), true, false, KEESHAN_PATH_ID);
        }
    }

    void WaypointReached(uint32 uiWP) override
    {
        switch (uiWP)
        {
            case 25:
                m_creature->SetStandState(UNIT_STAND_STATE_SIT);
                if (Player* player = GetPlayerForEscort())                
                    DoBroadcastText(SAY_CORPORAL_KEESHAN_2, m_creature, player);
                break;
            case 26:
                m_creature->SetStandState(UNIT_STAND_STATE_STAND);
                if (Player* player = GetPlayerForEscort())
                    DoBroadcastText(SAY_CORPORAL_KEESHAN_3, m_creature, player);
                break;
            case 69:                                        // quest_complete
                if (Player* player = GetPlayerForEscort())
                {
                    player->RewardPlayerAndGroupAtEventExplored(QUEST_MISSING_IN_ACTION, m_creature);
                    DoBroadcastText(SAY_CORPORAL_KEESHAN_4, m_creature, player);
                }
                break;
        }
    }
};

UnitAI* GetAI_npc_corporal_keeshan(Creature* pCreature)
{
    return new npc_corporal_keeshan_escortAI(pCreature);
}

bool QuestAccept_npc_corporal_keeshan(Player* player, Creature* pCreature, const Quest* pQuest)
{
    if (pQuest->GetQuestId() == QUEST_MISSING_IN_ACTION)
        pCreature->AI()->SendAIEvent(AI_EVENT_START_ESCORT, player, pCreature, pQuest->GetQuestId());

    return true;
}

void AddSC_redridge_mountains()
{
    Script* pNewScript = new Script;
    pNewScript->Name = "npc_corporal_keeshan";
    pNewScript->GetAI = &GetAI_npc_corporal_keeshan;
    pNewScript->pQuestAcceptNPC = &QuestAccept_npc_corporal_keeshan;
    pNewScript->RegisterSelf();
}
