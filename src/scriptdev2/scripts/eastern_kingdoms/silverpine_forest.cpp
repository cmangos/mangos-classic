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
SDName: Silverpine_Forest
SD%Complete: 100
SDComment: Quest support: 435, 452
SDCategory: Silverpine Forest
EndScriptData */

/* ContentData
npc_deathstalker_erland
npc_deathstalker_faerleia
EndContentData */

#include "precompiled.h"
#include "escort_ai.h"

/*#####
## npc_deathstalker_erland
#####*/

enum
{
    SAY_START_1         = -1000306,
    SAY_START_2         = -1000307,
    SAY_AGGRO_1         = -1000308,
    SAY_AGGRO_2         = -1000309,
    SAY_AGGRO_3         = -1000310,
    SAY_PROGRESS        = -1000311,
    SAY_END             = -1000312,
    SAY_RANE            = -1000313,
    SAY_RANE_REPLY      = -1000314,
    SAY_CHECK_NEXT      = -1000315,
    SAY_QUINN           = -1000316,
    SAY_QUINN_REPLY     = -1000317,
    SAY_BYE             = -1000318,

    QUEST_ERLAND        = 435,
    NPC_RANE            = 1950,
    NPC_QUINN           = 1951
};

struct npc_deathstalker_erlandAI : public npc_escortAI
{
    npc_deathstalker_erlandAI(Creature* pCreature) : npc_escortAI(pCreature) { Reset(); }

    void WaypointReached(uint32 i) override
    {
        Player* pPlayer = GetPlayerForEscort();

        if (!pPlayer)
            return;

        switch (i)
        {
            case 0:
                DoScriptText(SAY_START_2, m_creature, pPlayer);
                break;
            case 13:
                DoScriptText(SAY_END, m_creature, pPlayer);
                pPlayer->GroupEventHappens(QUEST_ERLAND, m_creature);
                break;
            case 14:
                if (Creature* pRane = GetClosestCreatureWithEntry(m_creature, NPC_RANE, 45.0f))
                    DoScriptText(SAY_RANE, pRane, m_creature);
                break;
            case 15:
                DoScriptText(SAY_RANE_REPLY, m_creature);
                break;
            case 16:
                DoScriptText(SAY_CHECK_NEXT, m_creature);
                break;
            case 24:
                DoScriptText(SAY_QUINN, m_creature);
                break;
            case 25:
                if (Creature* pQuinn = GetClosestCreatureWithEntry(m_creature, NPC_QUINN, 45.0f))
                    DoScriptText(SAY_QUINN_REPLY, pQuinn, m_creature);
                break;
            case 26:
                DoScriptText(SAY_BYE, m_creature);
                break;
        }
    }

    void Reset() override {}

    void Aggro(Unit* who) override
    {
        switch (urand(0, 2))
        {
            case 0: DoScriptText(SAY_AGGRO_1, m_creature, who); break;
            case 1: DoScriptText(SAY_AGGRO_2, m_creature, who); break;
            case 2: DoScriptText(SAY_AGGRO_3, m_creature, who); break;
        }
    }
};

bool QuestAccept_npc_deathstalker_erland(Player* pPlayer, Creature* pCreature, const Quest* pQuest)
{
    if (pQuest->GetQuestId() == QUEST_ERLAND)
    {
        DoScriptText(SAY_START_1, pCreature);

        if (npc_deathstalker_erlandAI* pEscortAI = dynamic_cast<npc_deathstalker_erlandAI*>(pCreature->AI()))
            pEscortAI->Start(false, pPlayer, pQuest);
    }
    return true;
}

CreatureAI* GetAI_npc_deathstalker_erland(Creature* pCreature)
{
    return new npc_deathstalker_erlandAI(pCreature);
}

/*#####
## npc_deathstalker_faerleia
#####*/

enum
{
    QUEST_PYREWOOD_AMBUSH    = 452,

    // cast it after every wave
    SPELL_DRINK_POTION       = 3359,

    SAY_START                = -1000553,
    SAY_COMPLETED            = -1000554,

    // 1st wave
    NPC_COUNCILMAN_SMITHERS  = 2060,
    // 2nd wave
    NPC_COUNCILMAN_THATHER   = 2061,
    NPC_COUNCILMAN_HENDRICKS = 2062,
    // 3rd wave
    NPC_COUNCILMAN_WILHELM   = 2063,
    NPC_COUNCILMAN_HARTIN    = 2064,
    NPC_COUNCILMAN_HIGARTH   = 2066,
    // final wave
    NPC_COUNCILMAN_COOPER    = 2065,
    NPC_COUNCILMAN_BRUNSWICK = 2067,
    NPC_LORD_MAYOR_MORRISON  = 2068
};

struct SpawnPoint
{
    float fX;
    float fY;
    float fZ;
    float fO;
};

SpawnPoint SpawnPoints[] =
{
    { -397.45f, 1509.56f, 18.87f, 4.73f},
    { -398.35f, 1510.75f, 18.87f, 4.76f},
    { -396.41f, 1511.06f, 18.87f, 4.74f}
};

static float m_afMoveCoords[] = { -410.69f, 1498.04f, 19.77f};

struct npc_deathstalker_faerleiaAI : public ScriptedAI
{
    npc_deathstalker_faerleiaAI(Creature* pCreature) : ScriptedAI(pCreature) { Reset(); }

    void Reset() override {}

    ObjectGuid m_playerGuid;
    uint32 m_uiWaveTimer;
    uint32 m_uiSummonCount;
    uint8  m_uiWaveCount;
    bool   m_bEventStarted;

    void StartEvent(Player* pPlayer)
    {
        m_creature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);

        m_playerGuid  = pPlayer->GetObjectGuid();
        m_bEventStarted = true;
        m_uiWaveTimer   = 10000;
        m_uiSummonCount = 0;
        m_uiWaveCount   = 0;
    }

    void FinishEvent()
    {
        m_playerGuid.Clear();
        m_bEventStarted = false;
        m_creature->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
    }

    void JustDied(Unit* /*pKiller*/) override
    {
        if (Player* pPlayer = m_creature->GetMap()->GetPlayer(m_playerGuid))
            pPlayer->SendQuestFailed(QUEST_PYREWOOD_AMBUSH);

        FinishEvent();
    }

    void JustSummoned(Creature* pSummoned) override
    {
        ++m_uiSummonCount;

        // put them on correct waypoints later on
        float fX, fY, fZ;
        pSummoned->GetRandomPoint(m_afMoveCoords[0], m_afMoveCoords[1], m_afMoveCoords[2], 10.0f, fX, fY, fZ);
        pSummoned->GetMotionMaster()->MovePoint(0, fX, fY, fZ);
    }

    void SummonedCreatureJustDied(Creature* /*pKilled*/) override
    {
        --m_uiSummonCount;

        if (!m_uiSummonCount)
        {
            DoCastSpellIfCan(m_creature, SPELL_DRINK_POTION);

            // final wave
            if (m_uiWaveCount == 4)
            {
                DoScriptText(SAY_COMPLETED, m_creature);

                if (Player* pPlayer = m_creature->GetMap()->GetPlayer(m_playerGuid))
                    pPlayer->GroupEventHappens(QUEST_PYREWOOD_AMBUSH, m_creature);

                FinishEvent();
            }
        }
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        if (m_bEventStarted && !m_uiSummonCount)
        {
            if (m_uiWaveTimer < uiDiff)
            {
                switch (m_uiWaveCount)
                {
                    case 0:
                        m_creature->SummonCreature(NPC_COUNCILMAN_SMITHERS,  SpawnPoints[1].fX, SpawnPoints[1].fY, SpawnPoints[1].fZ, SpawnPoints[1].fO, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 20000);
                        m_uiWaveTimer = 10000;
                        break;
                    case 1:
                        m_creature->SummonCreature(NPC_COUNCILMAN_THATHER,   SpawnPoints[2].fX, SpawnPoints[2].fY, SpawnPoints[2].fZ, SpawnPoints[2].fO, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 20000);
                        m_creature->SummonCreature(NPC_COUNCILMAN_HENDRICKS, SpawnPoints[1].fX, SpawnPoints[1].fY, SpawnPoints[1].fZ, SpawnPoints[1].fO, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 20000);
                        m_uiWaveTimer = 10000;
                        break;
                    case 2:
                        m_creature->SummonCreature(NPC_COUNCILMAN_WILHELM,   SpawnPoints[1].fX, SpawnPoints[1].fY, SpawnPoints[1].fZ, SpawnPoints[1].fO, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 20000);
                        m_creature->SummonCreature(NPC_COUNCILMAN_HARTIN,    SpawnPoints[0].fX, SpawnPoints[0].fY, SpawnPoints[0].fZ, SpawnPoints[0].fO, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 20000);
                        m_creature->SummonCreature(NPC_COUNCILMAN_HIGARTH,   SpawnPoints[2].fX, SpawnPoints[2].fY, SpawnPoints[2].fZ, SpawnPoints[2].fO, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 20000);
                        m_uiWaveTimer  = 8000;
                        break;
                    case 3:
                        m_creature->SummonCreature(NPC_COUNCILMAN_COOPER,    SpawnPoints[1].fX, SpawnPoints[1].fY, SpawnPoints[1].fZ, SpawnPoints[1].fO, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 20000);
                        m_creature->SummonCreature(NPC_COUNCILMAN_BRUNSWICK, SpawnPoints[2].fX, SpawnPoints[2].fY, SpawnPoints[2].fZ, SpawnPoints[2].fO, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 20000);
                        m_creature->SummonCreature(NPC_LORD_MAYOR_MORRISON,  SpawnPoints[0].fX, SpawnPoints[0].fY, SpawnPoints[0].fZ, SpawnPoints[0].fO, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 20000);
                        break;
                }

                ++m_uiWaveCount;
            }
            else
                m_uiWaveTimer -= uiDiff;
        }

        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        DoMeleeAttackIfReady();
    }
};

bool QuestAccept_npc_deathstalker_faerleia(Player* pPlayer, Creature* pCreature, const Quest* pQuest)
{
    if (pQuest->GetQuestId() == QUEST_PYREWOOD_AMBUSH)
    {
        DoScriptText(SAY_START, pCreature, pPlayer);

        if (npc_deathstalker_faerleiaAI* pFaerleiaAI = dynamic_cast<npc_deathstalker_faerleiaAI*>(pCreature->AI()))
            pFaerleiaAI->StartEvent(pPlayer);
    }
    return true;
}

CreatureAI* GetAI_npc_deathstalker_faerleia(Creature* pCreature)
{
    return new npc_deathstalker_faerleiaAI(pCreature);
}

void AddSC_silverpine_forest()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "npc_deathstalker_erland";
    pNewScript->GetAI = &GetAI_npc_deathstalker_erland;
    pNewScript->pQuestAcceptNPC = &QuestAccept_npc_deathstalker_erland;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_deathstalker_faerleia";
    pNewScript->GetAI = &GetAI_npc_deathstalker_faerleia;
    pNewScript->pQuestAcceptNPC = &QuestAccept_npc_deathstalker_faerleia;
    pNewScript->RegisterSelf();
}
