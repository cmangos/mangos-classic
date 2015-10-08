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
SDName: Searing_Gorge
SD%Complete: 80
SDComment: Quest support: 3367.
SDCategory: Searing Gorge
EndScriptData */

/* ContentData
npc_dorius_stonetender
EndContentData */

#include "precompiled.h"
#include "escort_ai.h"

/*######
## npc_dorius_stonetender
######*/

enum
{
    SAY_DORIUS_AGGRO_1              = -1000993,
    SAY_DORIUS_AGGRO_2              = -1000994,

    NPC_DARK_IRON_STEELSHIFTER      = 8337,
    MAX_STEELSHIFTERS               = 4,

    QUEST_ID_SUNTARA_STONES         = 3367,
};

struct npc_dorius_stonetenderAI : public npc_escortAI
{
    npc_dorius_stonetenderAI(Creature* pCreature) : npc_escortAI(pCreature) { Reset(); }

    void Reset() override { }

    void Aggro(Unit* pWho) override
    {
        DoScriptText(urand(0, 1) ? SAY_DORIUS_AGGRO_1 : SAY_DORIUS_AGGRO_2, m_creature, pWho);
    }

    void ReceiveAIEvent(AIEventType eventType, Creature* /*pSender*/, Unit* pInvoker, uint32 uiMiscValue) override
    {
        if (eventType == AI_EVENT_START_ESCORT && pInvoker->GetTypeId() == TYPEID_PLAYER)
        {
            // ToDo: research if there is any text here
            m_creature->SetStandState(UNIT_STAND_STATE_STAND);
            m_creature->SetFactionTemporary(FACTION_ESCORT_A_NEUTRAL_PASSIVE, TEMPFACTION_RESTORE_RESPAWN);
            Start(false, (Player*)pInvoker, GetQuestTemplateStore(uiMiscValue), true);
        }
    }

    void WaypointReached(uint32 uiPointId) override
    {
        switch (uiPointId)
        {
            case 20:
                // ToDo: research if there is any text here!
                float fX, fY, fZ;
                for (uint8 i = 0; i < MAX_STEELSHIFTERS; ++i)
                {
                    m_creature->GetNearPoint(m_creature, fX, fY, fZ, 0, 15.0f, i * M_PI_F / 2);
                    m_creature->SummonCreature(NPC_DARK_IRON_STEELSHIFTER, fX, fY, fZ, 0, TEMPSUMMON_TIMED_OOC_OR_DEAD_DESPAWN, 60000);
                }
                break;
            case 33:
                // ToDo: research if there is any event and text here!
                if (Player* pPlayer = GetPlayerForEscort())
                    pPlayer->GroupEventHappens(QUEST_ID_SUNTARA_STONES, m_creature);
                m_creature->SetStandState(UNIT_STAND_STATE_DEAD);
                break;
        }
    }

    void JustSummoned(Creature* pSummoned) override
    {
        if (pSummoned->GetEntry() == NPC_DARK_IRON_STEELSHIFTER)
            pSummoned->AI()->AttackStart(m_creature);
    }

    void UpdateEscortAI(const uint32 uiDiff) override
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_dorius_stonetender(Creature* pCreature)
{
    return new npc_dorius_stonetenderAI(pCreature);
}

bool QuestAccept_npc_dorius_stonetender(Player* pPlayer, Creature* pCreature, const Quest* pQuest)
{
    if (pQuest->GetQuestId() == QUEST_ID_SUNTARA_STONES)
    {
        pCreature->AI()->SendAIEvent(AI_EVENT_START_ESCORT, pPlayer, pCreature, pQuest->GetQuestId());
        return true;
    }

    return false;
}

void AddSC_searing_gorge()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "npc_dorius_stonetender";
    pNewScript->GetAI = &GetAI_npc_dorius_stonetender;
    pNewScript->pQuestAcceptNPC = &QuestAccept_npc_dorius_stonetender;
    pNewScript->RegisterSelf();
}
