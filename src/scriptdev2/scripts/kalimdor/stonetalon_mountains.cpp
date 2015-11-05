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
SDName: Stonetalon_Mountains
SD%Complete: 100
SDComment: Quest support: 6523.
SDCategory: Stonetalon Mountains
EndScriptData */

#include "precompiled.h"
#include "escort_ai.h"

/*######
## npc_kaya
######*/

enum
{
    NPC_GRIMTOTEM_RUFFIAN       = 11910,
    NPC_GRIMTOTEM_BRUTE         = 11912,
    NPC_GRIMTOTEM_SORCERER      = 11913,

    SAY_START                   = -1000357,
    SAY_AMBUSH                  = -1000358,
    SAY_END                     = -1000359,

    QUEST_PROTECT_KAYA          = 6523
};

struct npc_kayaAI : public npc_escortAI
{
    npc_kayaAI(Creature* pCreature) : npc_escortAI(pCreature) { Reset(); }

    void Reset() override { }

    void JustSummoned(Creature* pSummoned) override
    {
        pSummoned->AI()->AttackStart(m_creature);
    }

    void WaypointReached(uint32 uiPointId) override
    {
        switch (uiPointId)
        {
                // Ambush
            case 16:
                // note about event here:
                // apparently NPC say _after_ the ambush is over, and is most likely a bug at you-know-where.
                // we simplify this, and make say when the ambush actually start.
                DoScriptText(SAY_AMBUSH, m_creature);
                m_creature->SummonCreature(NPC_GRIMTOTEM_RUFFIAN, -50.75f, -500.77f, -46.13f, 0.4f, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 30000);
                m_creature->SummonCreature(NPC_GRIMTOTEM_BRUTE, -40.05f, -510.89f, -46.05f, 1.7f, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 30000);
                m_creature->SummonCreature(NPC_GRIMTOTEM_SORCERER, -32.21f, -499.20f, -45.35f, 2.8f, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 30000);
                break;
                // Award quest credit
            case 18:
                DoScriptText(SAY_END, m_creature);

                if (Player* pPlayer = GetPlayerForEscort())
                    pPlayer->GroupEventHappens(QUEST_PROTECT_KAYA, m_creature);
                break;
        }
    }
};

CreatureAI* GetAI_npc_kaya(Creature* pCreature)
{
    return new npc_kayaAI(pCreature);
}

bool QuestAccept_npc_kaya(Player* pPlayer, Creature* pCreature, Quest const* pQuest)
{
    // Casting Spell and Starting the Escort quest is buggy, so this is a hack. Use the spell when it is possible.

    if (pQuest->GetQuestId() == QUEST_PROTECT_KAYA)
    {
        pCreature->SetFactionTemporary(FACTION_ESCORT_H_PASSIVE, TEMPFACTION_RESTORE_RESPAWN);
        DoScriptText(SAY_START, pCreature);

        if (npc_kayaAI* pEscortAI = dynamic_cast<npc_kayaAI*>(pCreature->AI()))
            pEscortAI->Start(false, pPlayer, pQuest);
    }
    return true;
}

/*######
## AddSC
######*/

void AddSC_stonetalon_mountains()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "npc_kaya";
    pNewScript->GetAI = &GetAI_npc_kaya;
    pNewScript->pQuestAcceptNPC = &QuestAccept_npc_kaya;
    pNewScript->RegisterSelf();
}
