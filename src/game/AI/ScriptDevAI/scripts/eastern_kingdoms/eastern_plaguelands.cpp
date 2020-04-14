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
SDName: Eastern_Plaguelands
SD%Complete: 100
SDComment: Quest support: 7622.
SDCategory: Eastern Plaguelands
EndScriptData

*/

#include "AI/ScriptDevAI/include/sc_common.h"/* ContentData
npc_eris_havenfire
EndContentData */



/*######
## npc_eris_havenfire
######*/

enum
{
    SAY_PHASE_HEAL                      = -1000815,
    SAY_EVENT_END                       = -1000816,
    SAY_EVENT_FAIL_1                    = -1000817,
    SAY_EVENT_FAIL_2                    = -1000818,
    SAY_PEASANT_APPEAR_1                = -1000819,
    SAY_PEASANT_APPEAR_2                = -1000820,
    SAY_PEASANT_APPEAR_3                = -1000821,

    // SPELL_DEATHS_DOOR                 = 23127,           // damage spells cast on the peasants
    // SPELL_SEETHING_PLAGUE             = 23072,
    SPELL_ENTER_THE_LIGHT_DND           = 23107,
    SPELL_BLESSING_OF_NORDRASSIL        = 23108,

    NPC_INJURED_PEASANT                 = 14484,
    NPC_PLAGUED_PEASANT                 = 14485,
    NPC_SCOURGE_ARCHER                  = 14489,
    NPC_SCOURGE_FOOTSOLDIER             = 14486,
    NPC_THE_CLEANER                     = 14503,            // can be summoned if the priest has more players in the party for help. requires further research

    QUEST_BALANCE_OF_LIGHT_AND_SHADOW   = 7622,

    BASE_PEASANTS_PER_WAVE              = 11,
    MAX_KILLED_PEASANT                  = 15,
    MAX_SAVED_PEASANT                   = 50,
    MAX_ARCHERS                         = 8,
};

static const float aArcherSpawn[8][4] =
{
    {3327.42f, -3021.11f, 170.57f, 6.01f},
    {3335.4f,  -3054.3f,  173.63f, 0.49f},
    {3351.3f,  -3079.08f, 178.67f, 1.15f},
    {3358.93f, -3076.1f,  174.87f, 1.57f},
    {3371.58f, -3069.24f, 175.20f, 1.99f},
    {3369.46f, -3023.11f, 171.83f, 3.69f},
    {3383.25f, -3057.01f, 181.53f, 2.21f},
    {3380.03f, -3062.73f, 181.90f, 2.31f},
};

static const float aPeasantSpawnLoc[3] = {3360.12f, -3047.79f, 165.26f};
static const float aPeasantMoveLoc[3] = {3335.0f, -2994.04f, 161.14f};

static const int32 aPeasantSpawnYells[3] = {SAY_PEASANT_APPEAR_1, SAY_PEASANT_APPEAR_2, SAY_PEASANT_APPEAR_3};

struct npc_eris_havenfireAI : public ScriptedAI
{
    npc_eris_havenfireAI(Creature* pCreature) : ScriptedAI(pCreature) { Reset(); }

    uint32 m_uiEventTimer;
    uint32 m_uiSadEndTimer;
    uint32 m_uiArcherCheckTargetTimer;
    uint8 m_uiPhase;
    uint8 m_uiCurrentWave;
    uint8 m_uiKillCounter;
    uint8 m_uiSaveCounter;
    uint8 m_uiTotalCounter;

    ObjectGuid m_playerGuid;
    GuidList m_lSummonedGuidList;

    void Reset() override
    {
        m_uiEventTimer              = 0;
        m_uiSadEndTimer             = 0;
        m_uiPhase                   = 0;
        m_uiCurrentWave             = 0;
        m_uiKillCounter             = 0;
        m_uiSaveCounter             = 0;
        m_uiArcherCheckTargetTimer  = 0;

        m_playerGuid.Clear();
        m_lSummonedGuidList.clear();
        m_creature->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
    }

    void JustSummoned(Creature* pSummoned) override
    {
        switch (pSummoned->GetEntry())
        {
            case NPC_INJURED_PEASANT:
            case NPC_PLAGUED_PEASANT:
                float fX, fY, fZ;
                pSummoned->GetRandomPoint(aPeasantMoveLoc[0], aPeasantMoveLoc[1], aPeasantMoveLoc[2], 10.0f, fX, fY, fZ);
                pSummoned->GetMotionMaster()->MovePoint(1, fX, fY, fZ);
                m_uiTotalCounter++;
                break;
            case NPC_SCOURGE_FOOTSOLDIER:
            case NPC_THE_CLEANER:
                if (Player* pPlayer = m_creature->GetMap()->GetPlayer(m_playerGuid))
                    pSummoned->AI()->AttackStart(pPlayer);
                break;
            case NPC_SCOURGE_ARCHER:
                break;
        }

        m_lSummonedGuidList.push_back(pSummoned->GetObjectGuid());
    }

    void SummonedMovementInform(Creature* pSummoned, uint32 uiMotionType, uint32 uiPointId) override
    {
        if (uiMotionType != POINT_MOTION_TYPE || !uiPointId)
            return;

        if (uiPointId)
        {
            ++m_uiSaveCounter;
            pSummoned->GetMotionMaster()->Clear();

            pSummoned->RemoveAllAuras();
            pSummoned->CastSpell(pSummoned, SPELL_ENTER_THE_LIGHT_DND, TRIGGERED_NONE);
            pSummoned->ForcedDespawn(10000);

            // Event ended
            if (m_uiSaveCounter >= MAX_SAVED_PEASANT)
                DoBalanceEventEnd();
            // Phase ended
            else if (m_uiSaveCounter + m_uiKillCounter == m_uiTotalCounter)
                DoHandlePhaseEnd();
        }
    }

    void SummonedCreatureJustDied(Creature* pSummoned) override
    {
        if (pSummoned->GetEntry() == NPC_INJURED_PEASANT || pSummoned->GetEntry() == NPC_PLAGUED_PEASANT)
        {
            ++m_uiKillCounter;

            // If more than 15 peasants have died, then fail the quest
            if (m_uiKillCounter == MAX_KILLED_PEASANT)
            {
                if (Player* pPlayer = m_creature->GetMap()->GetPlayer(m_playerGuid))
                    pPlayer->FailQuest(QUEST_BALANCE_OF_LIGHT_AND_SHADOW);

                DoScriptText(SAY_EVENT_FAIL_1, m_creature);
                m_uiSadEndTimer = 4000;
            }
            else if (m_uiSaveCounter + m_uiKillCounter == m_uiTotalCounter)
                DoHandlePhaseEnd();
        }
    }

    void DoSummonWave(uint32 uiSummonId = 0)
    {
        float fX, fY, fZ;

        if (!uiSummonId)
        {
            for (uint8 i = 0; i < BASE_PEASANTS_PER_WAVE + m_uiCurrentWave; ++i)
            {
                uint32 uiSummonEntry = roll_chance_i(70) ? NPC_INJURED_PEASANT : NPC_PLAGUED_PEASANT;
                m_creature->GetRandomPoint(aPeasantSpawnLoc[0], aPeasantSpawnLoc[1], aPeasantSpawnLoc[2], 10.0f, fX, fY, fZ);
                if (Creature* pTemp = m_creature->SummonCreature(uiSummonEntry, fX, fY, fZ, 0, TEMPSPAWN_DEAD_DESPAWN, 0))
                {
                    // Only the first mob needs to yell
                    if (!i)
                        DoScriptText(aPeasantSpawnYells[urand(0, 2)], pTemp);
                }
            }

            ++m_uiCurrentWave;
        }
        else if (uiSummonId == NPC_SCOURGE_FOOTSOLDIER)
        {
            uint8 uiRand = urand(2, 3);
            for (uint8 i = 0; i < uiRand; ++i)
            {
                m_creature->GetRandomPoint(aPeasantSpawnLoc[0], aPeasantSpawnLoc[1], aPeasantSpawnLoc[2], 15.0f, fX, fY, fZ);
                m_creature->SummonCreature(NPC_SCOURGE_FOOTSOLDIER, fX, fY, fZ, 0, TEMPSPAWN_DEAD_DESPAWN, 0);
            }
        }
        else if (uiSummonId == NPC_SCOURGE_ARCHER)
        {
            for (auto i : aArcherSpawn)
                m_creature->SummonCreature(NPC_SCOURGE_ARCHER, i[0], i[1], i[2], i[3], TEMPSPAWN_DEAD_DESPAWN, 0);
        }
    }

    void DoHandlePhaseEnd()
    {
        if (Player* pPlayer = m_creature->GetMap()->GetPlayer(m_playerGuid))
            pPlayer->CastSpell(pPlayer, SPELL_BLESSING_OF_NORDRASSIL, TRIGGERED_OLD_TRIGGERED);

        DoScriptText(SAY_PHASE_HEAL, m_creature);

        // Send next wave
        if (m_uiCurrentWave < 5)
            DoSummonWave();
    }

    void DoStartBalanceEvent(Player* pPlayer)
    {
        m_creature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
        m_playerGuid = pPlayer->GetObjectGuid();
        m_uiEventTimer = 5000;
        m_uiArcherCheckTargetTimer = 2000;
    }

    void DoBalanceEventEnd()
    {
        if (Player* pPlayer = m_creature->GetMap()->GetPlayer(m_playerGuid))
            pPlayer->AreaExploredOrEventHappens(QUEST_BALANCE_OF_LIGHT_AND_SHADOW);

        DoScriptText(SAY_EVENT_END, m_creature);
        m_uiArcherCheckTargetTimer = 0;
        DoDespawnSummons(true);
        EnterEvadeMode();
    }

    void DoDespawnSummons(bool bIsEventEnd = false)
    {
        for (GuidList::const_iterator itr = m_lSummonedGuidList.begin(); itr != m_lSummonedGuidList.end(); ++itr)
        {
            if (Creature* pTemp = m_creature->GetMap()->GetCreature(*itr))
            {
                if (bIsEventEnd && (pTemp->GetEntry() == NPC_INJURED_PEASANT || pTemp->GetEntry() == NPC_PLAGUED_PEASANT))
                    continue;

                pTemp->ForcedDespawn();
            }
        }
    }

    // Loop over all Scourge Archer NPCs to pick a target and attack it
    void DoAttackArchersTarget()
    {
        for (GuidList::const_iterator itr = m_lSummonedGuidList.begin(); itr != m_lSummonedGuidList.end(); ++itr)
        {
            Creature* pTemp = m_creature->GetMap()->GetCreature(*itr);
            if (pTemp && pTemp->GetEntry() == NPC_SCOURGE_ARCHER)
            {
                // Archer has a victim and is in range, keep on attacking it
                if (pTemp->GetVictim() && pTemp->IsWithinDistInMap(pTemp->GetVictim(), 30.0f))
                    continue;
                // Else, find a new target in range
                // First look for an Injured Peasant in range (arbitrary choice), if none look for a Plagued Peasant
                Creature* pTarget = GetClosestCreatureWithEntry(pTemp, NPC_INJURED_PEASANT, 30.0f);
                if (!pTarget)
                    pTarget = GetClosestCreatureWithEntry(pTemp, NPC_PLAGUED_PEASANT, 30.0f);

                if (pTarget)
                    pTemp->AI()->AttackStart(pTarget);
            }
        }
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        if (m_uiEventTimer)
        {
            if (m_uiEventTimer <= uiDiff)
            {
                switch (m_uiPhase)
                {
                    case 0:
                        DoSummonWave(NPC_SCOURGE_ARCHER);
                        m_uiEventTimer = 5000;
                        break;
                    case 1:
                        DoSummonWave();
                        m_uiEventTimer = urand(60000, 80000);
                        break;
                    default:
                        // The summoning timer of the soldiers isn't very clear
                        DoSummonWave(NPC_SCOURGE_FOOTSOLDIER);
                        m_uiEventTimer = urand(5000, 30000);
                        break;
                }
                ++m_uiPhase;
            }
            else
                m_uiEventTimer -= uiDiff;
        }

        // Handle event end in case of fail
        if (m_uiSadEndTimer)
        {
            if (m_uiSadEndTimer <= uiDiff)
            {
                DoScriptText(SAY_EVENT_FAIL_2, m_creature);
                m_creature->ForcedDespawn(5000);
                DoDespawnSummons();
                m_uiSadEndTimer = 0;
            }
            else
                m_uiSadEndTimer -= uiDiff;
        }

        // Timer to force Scourge Archer NPCs to refresh their target
        if (m_uiArcherCheckTargetTimer)
        {
            if (m_uiArcherCheckTargetTimer <= uiDiff)
            {
                // Make Scourge Archer search for a target in range and attack it
                DoAttackArchersTarget();
                m_uiArcherCheckTargetTimer = 2000;
            }
            else
                m_uiArcherCheckTargetTimer -= uiDiff;
        }
    }
};

UnitAI* GetAI_npc_eris_havenfire(Creature* pCreature)
{
    return new npc_eris_havenfireAI(pCreature);
}

bool QuestAccept_npc_eris_havenfire(Player* pPlayer, Creature* pCreature, const Quest* pQuest)
{
    if (pQuest->GetQuestId() == QUEST_BALANCE_OF_LIGHT_AND_SHADOW)
    {
        if (npc_eris_havenfireAI* pErisAI = dynamic_cast<npc_eris_havenfireAI*>(pCreature->AI()))
            pErisAI->DoStartBalanceEvent(pPlayer);
    }

    return true;
}

void AddSC_eastern_plaguelands()
{
    Script* pNewScript = new Script;
    pNewScript->Name = "npc_eris_havenfire";
    pNewScript->GetAI = &GetAI_npc_eris_havenfire;
    pNewScript->pQuestAcceptNPC = &QuestAccept_npc_eris_havenfire;
    pNewScript->RegisterSelf();
}
