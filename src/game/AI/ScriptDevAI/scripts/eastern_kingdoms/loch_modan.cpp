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
SDName: Loch_Modan
SD%Complete: 100
SDComment: Quest support: 3181 (only to argue with pebblebitty to get to searing gorge, before quest rewarded), 309
SDCategory: Loch Modan
EndScriptData */

#include "AI/ScriptDevAI/include/sc_common.h"

/* ContentData
npc_miran
EndContentData */


#include "AI/ScriptDevAI/base/escort_ai.h"

/*######
## npc_miran
######*/

enum
{
    QUEST_PROTECTING_THE_SHIPMENT = 309,

    SAY_MIRAN_1           = 510,
    SAY_MIRAN_2           = 511,
    SAY_MIRAN_3           = 498,

    SAY_DARK_IRON_RAIDER  = 512,
    NPC_DARK_IRON_RAIDER  = 2149,

    MIRAN_ESCORT_PATH     = 1379
};

struct Location
{
    float m_fX, m_fY, m_fZ, m_fO;
};

static const Location m_afAmbushSpawn[] =
{
    { -5705.012f, -3736.6575f, 318.56738f, 0.57595f},
    { -5696.1943f, -3736.78f, 318.58145f, 2.40855f}
};

struct npc_miranAI: public npc_escortAI
{
    npc_miranAI(Creature* creature): npc_escortAI(creature)
    {
        Reset();
    }

    uint8 m_uiDwarves;

    void Reset() override
    {
        if (!HasEscortState(STATE_ESCORT_ESCORTING))
            m_uiDwarves = 0;
    }

    void WaypointReached(uint32 uiPointId) override
    {
        switch (uiPointId)
        {
            case 16:
                DoBroadcastText(SAY_MIRAN_1, m_creature);
                break;
            case 17:
                SetEscortPaused(true);
                m_creature->SummonCreature(NPC_DARK_IRON_RAIDER, m_afAmbushSpawn[0].m_fX, m_afAmbushSpawn[0].m_fY, m_afAmbushSpawn[0].m_fZ, m_afAmbushSpawn[0].m_fO, TEMPSPAWN_CORPSE_TIMED_DESPAWN, 25000);
                m_creature->SummonCreature(NPC_DARK_IRON_RAIDER, m_afAmbushSpawn[1].m_fX, m_afAmbushSpawn[1].m_fY, m_afAmbushSpawn[1].m_fZ, m_afAmbushSpawn[1].m_fO, TEMPSPAWN_CORPSE_TIMED_DESPAWN, 25000);
                break;
            case 21:
                DoBroadcastText(SAY_MIRAN_3, m_creature);
                if (Player* player = GetPlayerForEscort())
                    player->RewardPlayerAndGroupAtEventExplored(QUEST_PROTECTING_THE_SHIPMENT, m_creature);
                SetEscortPaused(true); 
                m_creature->ForcedDespawn(15000);
                break;
        }
    }

    void SummonedCreatureJustDied(Creature* summoned) override
    {
        if (summoned->GetEntry() == NPC_DARK_IRON_RAIDER)
        {
            --m_uiDwarves;
            if (!m_uiDwarves)
            {
                DoBroadcastText(SAY_MIRAN_2, m_creature);
                SetEscortPaused(false);
            }
        }
    }

    void JustSummoned(Creature* summoned) override
    {
        if (summoned->GetEntry() == NPC_DARK_IRON_RAIDER)
        {
            ++m_uiDwarves;
            summoned->AI()->AttackStart(m_creature);
            DoBroadcastText(SAY_DARK_IRON_RAIDER, summoned);
        }
    }
};

bool QuestAccept_npc_miran(Player* player, Creature* creature, const Quest* quest)
{
    if (quest->GetQuestId() == QUEST_PROTECTING_THE_SHIPMENT)
    {
        if (npc_miranAI* pEscortAI = dynamic_cast<npc_miranAI*>(creature->AI()))
            pEscortAI->Start(false, player, quest, false, false, MIRAN_ESCORT_PATH);
    }
    return true;
}

UnitAI* GetAI_npc_miran(Creature* creature)
{
    return new npc_miranAI(creature);
}

void AddSC_loch_modan()
{
    Script* pNewScript = new Script;
    pNewScript->Name = "npc_miran";
    pNewScript->GetAI = &GetAI_npc_miran;
    pNewScript->pQuestAcceptNPC = &QuestAccept_npc_miran;
    pNewScript->RegisterSelf();
}
