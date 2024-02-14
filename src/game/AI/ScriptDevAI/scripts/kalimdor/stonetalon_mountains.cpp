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
SDComment: Quest support: 1079, 1080, 6523.
SDCategory: Stonetalon Mountains
EndScriptData

*/

#include "AI/ScriptDevAI/include/sc_common.h"
#include "AI/ScriptDevAI/base/escort_ai.h"
#include "Grids/GridNotifiers.h"
#include "Grids/CellImpl.h"

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
            case 17:
                // note about event here:
                // apparently NPC say _after_ the ambush is over, and is most likely a bug at you-know-where.
                // we simplify this, and make say when the ambush actually start.
                DoScriptText(SAY_AMBUSH, m_creature);
                m_creature->SummonCreature(NPC_GRIMTOTEM_RUFFIAN, -50.75f, -500.77f, -46.13f, 0.4f, TEMPSPAWN_CORPSE_TIMED_DESPAWN, 30000);
                m_creature->SummonCreature(NPC_GRIMTOTEM_BRUTE, -40.05f, -510.89f, -46.05f, 1.7f, TEMPSPAWN_CORPSE_TIMED_DESPAWN, 30000);
                m_creature->SummonCreature(NPC_GRIMTOTEM_SORCERER, -32.21f, -499.20f, -45.35f, 2.8f, TEMPSPAWN_CORPSE_TIMED_DESPAWN, 30000);
                break;
            // Award quest credit
            case 19:
                DoScriptText(SAY_END, m_creature);

                if (Player* pPlayer = GetPlayerForEscort())
                    pPlayer->RewardPlayerAndGroupAtEventExplored(QUEST_PROTECT_KAYA, m_creature);
                break;
        }
    }
};

UnitAI* GetAI_npc_kaya(Creature* pCreature)
{
    return new npc_kayaAI(pCreature);
}

bool QuestAccept_npc_kaya(Player* pPlayer, Creature* pCreature, Quest const* pQuest)
{
    // Casting Spell and Starting the Escort quest is buggy, so this is a hack. Use the spell when it is possible.

    if (pQuest->GetQuestId() == QUEST_PROTECT_KAYA)
    {
        pCreature->SetFactionTemporary(FACTION_ESCORT_H_PASSIVE, TEMPFACTION_RESTORE_RESPAWN | TEMPFACTION_TOGGLE_IMMUNE_TO_NPC);
        DoScriptText(SAY_START, pCreature);

        if (npc_kayaAI* pEscortAI = dynamic_cast<npc_kayaAI*>(pCreature->AI()))
            pEscortAI->Start(false, pPlayer, pQuest);
    }
    return true;
}

/*######################
# Covert Ops: Alpha/Beta
######################*/

enum CovertOpsAlphaBetaMisc
{
    // Creatures
    NPC_VENTURE_DEFORESTER = 3991,
    NPC_VENTURE_LOGGER = 3989,
    NPC_VENTURE_DIGGER = 3999,
    NPC_VENTURE_OVERLORD = 4004,
    NPC_VENTURE_OPERATOR = 3988,

    // Phase
    PHASE_INACTIVE = 0,
    PHASE_CALLING  = 1,
    PHASE_HOMING   = 2
};

static const uint32 ventureCoList[5] = { NPC_VENTURE_DEFORESTER, NPC_VENTURE_LOGGER, NPC_VENTURE_DIGGER, NPC_VENTURE_OVERLORD, NPC_VENTURE_OPERATOR };

struct go_covertopsAI : public GameObjectAI
{
    go_covertopsAI(GameObject* go) : GameObjectAI(go)
    {
        Reset();
    }
    uint32 m_phase;
    uint32 m_callTimer;		    // call Venture Co. NPCs to wagon
    uint32 m_returnTimer;		// Send Venture Co. NPCs back to spawn points

    CreatureList m_ventureNpcList;

    void Reset()
    {
        m_phase = PHASE_INACTIVE;
        m_callTimer = 2 * IN_MILLISECONDS;
        m_returnTimer = 45 * IN_MILLISECONDS;
    }

    void OnLootStateChange(Unit* /*user*/) override
    {
        if (m_go->GetLootState() == GO_JUST_DEACTIVATED)
        {
            m_phase = PHASE_CALLING;    // Calling Venture Co. NPCs

            // Get all nearby Venture Co. NPCs
            for (auto entry : ventureCoList)
            {
                MaNGOS::AllCreaturesOfEntryInRangeCheck check(((GameObject*) m_go), entry, 100.0f);
                MaNGOS::CreatureListSearcher<MaNGOS::AllCreaturesOfEntryInRangeCheck> searcher(m_ventureNpcList, check);
                Cell::VisitGridObjects(((GameObject*) m_go), searcher, 100.0f);
            }
        }
    }

    void JustSpawned() override
    {
        Reset();
    }

    void UpdateAI(const uint32 diff) override
    {
        switch (m_phase)
        {
            case PHASE_CALLING:
                if (m_callTimer < diff)
                {
                    for (auto ventureNpc:m_ventureNpcList)
                    {
                        if (!ventureNpc || !ventureNpc->IsAlive() || ventureNpc->IsInCombat())
                            continue;
                        float x, y, z;
                        m_go->GetContactPoint(ventureNpc, x, y, z, 10.0f);
                        ventureNpc->SetWalk(false);
                        ventureNpc->GetMotionMaster()->MoveIdle();
                        ventureNpc->GetMotionMaster()->MovePoint(1, x, y, z);
                    }
                    m_callTimer = 0;
                    m_phase = PHASE_HOMING;
                }
                else
                    m_callTimer -= diff;
                break;
            case PHASE_HOMING:
                if (m_returnTimer < diff)
                {
                    for (auto ventureNpc:m_ventureNpcList)
                    {
                        if (!ventureNpc || !ventureNpc->IsAlive() || ventureNpc->IsInCombat())
                            continue;
                        ventureNpc->SetWalk(true);
                        ventureNpc->AI()->EnterEvadeMode();
                    }
                    m_returnTimer = 0;
                    m_phase = PHASE_INACTIVE;
                }
                else
                    m_returnTimer -= diff;
                break;
            default:
                break;
        }
    }
};

GameObjectAI* GetAIgo_covertops(GameObject* pGo)
{
    return new go_covertopsAI(pGo);
}

/*######
## AddSC
######*/

void AddSC_stonetalon_mountains()
{
    Script* pNewScript = new Script;
    pNewScript->Name = "npc_kaya";
    pNewScript->GetAI = &GetAI_npc_kaya;
    pNewScript->pQuestAcceptNPC = &QuestAccept_npc_kaya;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "go_covert_ops";
    pNewScript->GetGameObjectAI = &GetAIgo_covertops;
    pNewScript->RegisterSelf();
}
