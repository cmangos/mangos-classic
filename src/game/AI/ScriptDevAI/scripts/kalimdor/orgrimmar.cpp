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
SDName: Orgrimmar
SD%Complete: 100
SDComment: Quest support: 2460
SDCategory: Orgrimmar
EndScriptData

*/

#include "AI/ScriptDevAI/include/precompiled.h"/* ContentData
npc_shenthul
EndContentData */



/*######
## npc_shenthul
######*/

enum
{
    QUEST_SHATTERED_SALUTE  = 2460
};

struct npc_shenthulAI : public ScriptedAI
{
    npc_shenthulAI(Creature* pCreature) : ScriptedAI(pCreature) { Reset(); }

    uint32 m_uiSaluteTimer;
    uint32 m_uiResetTimer;

    ObjectGuid m_playerGuid;

    void Reset() override
    {
        m_uiSaluteTimer = 0;
        m_uiResetTimer = 0;

        m_playerGuid.Clear();
    }

    void ReceiveEmote(Player* pPlayer, uint32 uiTextEmote) override
    {
        if (m_uiResetTimer && uiTextEmote == TEXTEMOTE_SALUTE && pPlayer->GetQuestStatus(QUEST_SHATTERED_SALUTE) == QUEST_STATUS_INCOMPLETE)
        {
            pPlayer->AreaExploredOrEventHappens(QUEST_SHATTERED_SALUTE);
            EnterEvadeMode();
        }
    }

    void DoStartQuestEvent(Player* pPlayer)
    {
        m_playerGuid = pPlayer->GetObjectGuid();
        m_uiSaluteTimer = 6000;
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        if (m_uiResetTimer)
        {
            if (m_uiResetTimer <= uiDiff)
            {
                if (Player* pPlayer = m_creature->GetMap()->GetPlayer(m_playerGuid))
                {
                    if (pPlayer->GetTypeId() == TYPEID_PLAYER && pPlayer->GetQuestStatus(QUEST_SHATTERED_SALUTE) == QUEST_STATUS_INCOMPLETE)
                        pPlayer->FailQuest(QUEST_SHATTERED_SALUTE);
                }

                m_uiResetTimer = 0;
                EnterEvadeMode();
            }
            else
                m_uiResetTimer -= uiDiff;
        }

        if (m_uiSaluteTimer)
        {
            if (m_uiSaluteTimer <= uiDiff)
            {
                m_creature->HandleEmote(EMOTE_ONESHOT_SALUTE);
                m_uiResetTimer = 60000;
                m_uiSaluteTimer = 0;
            }
            else
                m_uiSaluteTimer -= uiDiff;
        }

        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        DoMeleeAttackIfReady();
    }
};

UnitAI* GetAI_npc_shenthul(Creature* pCreature)
{
    return new npc_shenthulAI(pCreature);
}

bool QuestAccept_npc_shenthul(Player* pPlayer, Creature* pCreature, const Quest* pQuest)
{
    if (pQuest->GetQuestId() == QUEST_SHATTERED_SALUTE)
    {
        if (npc_shenthulAI* pShenAI = dynamic_cast<npc_shenthulAI*>(pCreature->AI()))
            pShenAI->DoStartQuestEvent(pPlayer);
    }

    return true;
}

void AddSC_orgrimmar()
{
    Script* pNewScript = new Script;
    pNewScript->Name = "npc_shenthul";
    pNewScript->GetAI = &GetAI_npc_shenthul;
    pNewScript->pQuestAcceptNPC =  &QuestAccept_npc_shenthul;
    pNewScript->RegisterSelf();
}
