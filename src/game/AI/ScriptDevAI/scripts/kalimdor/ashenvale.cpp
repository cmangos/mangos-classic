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
SDName: Ashenvale
SD%Complete: 70
SDComment: Quest support: 976, 6482, 6544, 6641
SDCategory: Ashenvale Forest
EndScriptData

*/

#include "AI/ScriptDevAI/include/sc_common.h"/* ContentData
npc_muglash
npc_ruul_snowhoof
npc_torek
npc_feero_ironhand
EndContentData */


#include "AI/ScriptDevAI/base/escort_ai.h"

/*####
# npc_muglash
####*/

enum
{
    SAY_MUG_START1          = -1000501,
    SAY_MUG_START2          = -1000502,
    SAY_MUG_BRAZIER         = -1000503,
    SAY_MUG_BRAZIER_WAIT    = -1000504,
    SAY_MUG_ON_GUARD        = -1000505,
    SAY_MUG_REST            = -1000506,
    SAY_MUG_DONE            = -1000507,
    SAY_MUG_GRATITUDE       = -1000508,
    SAY_MUG_PATROL          = -1000509,
    SAY_MUG_RETURN          = -1000510,

    QUEST_VORSHA            = 6641,

    GO_NAGA_BRAZIER         = 178247,
    NPC_MUGLASH             = 12717,

    NPC_WRATH_RIDER         = 3713,
    NPC_WRATH_SORCERESS     = 3717,
    NPC_WRATH_RAZORTAIL     = 3712,

    NPC_WRATH_PRIESTESS     = 3944,
    NPC_WRATH_MYRMIDON      = 3711,
    NPC_WRATH_SEAWITCH      = 3715,

    NPC_VORSHA              = 12940
};

static float m_afFirstNagaCoord[3][3] =
{
    {3603.504150f, 1122.631104f, 1.635f},                   // rider
    {3589.293945f, 1148.664063f, 5.565f},                   // sorceress
    {3609.925537f, 1168.759521f, -1.168f}                   // razortail
};

static float m_afSecondNagaCoord[3][3] =
{
    {3609.925537f, 1168.759521f, -1.168f},                  // witch
    {3645.652100f, 1139.425415f, 1.322f},                   // priest
    {3583.602051f, 1128.405762f, 2.347f}                    // myrmidon
};

static float m_fVorshaCoord[] = {3633.056885f, 1172.924072f, -5.388f};

struct npc_muglashAI : public npc_escortAI
{
    npc_muglashAI(Creature* pCreature) : npc_escortAI(pCreature)
    {
        m_uiWaveId = 0;
        m_bIsBrazierExtinguished = false;
        Reset();
    }

    bool m_bIsBrazierExtinguished;

    uint32 m_uiWaveId;
    uint32 m_uiEventTimer;

    void Reset() override
    {
        m_uiEventTimer = 10000;

        if (!HasEscortState(STATE_ESCORT_ESCORTING))
        {
            m_uiWaveId = 0;
            m_bIsBrazierExtinguished = false;
        }
    }

    void Aggro(Unit* /*pWho*/) override
    {
        if (HasEscortState(STATE_ESCORT_PAUSED))
        {
            if (urand(0, 1))
                return;

            if (Player* pPlayer = GetPlayerForEscort())
                DoScriptText(SAY_MUG_ON_GUARD, m_creature, pPlayer);
        }
    }

    void WaypointReached(uint32 uiPointId) override
    {
        switch (uiPointId)
        {
            case 1:
                if (Player* pPlayer = GetPlayerForEscort())
                    DoScriptText(SAY_MUG_START2, m_creature, pPlayer);
                break;
            case 25:
                if (Player* pPlayer = GetPlayerForEscort())
                    DoScriptText(SAY_MUG_BRAZIER, m_creature, pPlayer);

                if (GameObject* pGo = GetClosestGameObjectWithEntry(m_creature, GO_NAGA_BRAZIER, INTERACTION_DISTANCE * 2))
                {
                    // some kind of event flag? Update to player/group only?
                    pGo->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_NO_INTERACT);
                    SetEscortPaused(true);
                }
                break;
            case 26:
                DoScriptText(SAY_MUG_GRATITUDE, m_creature);

                if (Player* pPlayer = GetPlayerForEscort())
                    pPlayer->RewardPlayerAndGroupAtEventExplored(QUEST_VORSHA, m_creature);
                break;
            case 27:
                DoScriptText(SAY_MUG_PATROL, m_creature);
                break;
            case 28:
                DoScriptText(SAY_MUG_RETURN, m_creature);
                break;
        }
    }

    void DoWaveSummon()
    {
        switch (m_uiWaveId)
        {
            case 1:
                m_creature->SummonCreature(NPC_WRATH_RIDER,     m_afFirstNagaCoord[0][0], m_afFirstNagaCoord[0][1], m_afFirstNagaCoord[0][2], 0.0f, TEMPSPAWN_TIMED_OOC_DESPAWN, 60000);
                m_creature->SummonCreature(NPC_WRATH_SORCERESS, m_afFirstNagaCoord[1][0], m_afFirstNagaCoord[1][1], m_afFirstNagaCoord[1][2], 0.0f, TEMPSPAWN_TIMED_OOC_DESPAWN, 60000);
                m_creature->SummonCreature(NPC_WRATH_RAZORTAIL, m_afFirstNagaCoord[2][0], m_afFirstNagaCoord[2][1], m_afFirstNagaCoord[2][2], 0.0f, TEMPSPAWN_TIMED_OOC_DESPAWN, 60000);
                break;
            case 2:
                m_creature->SummonCreature(NPC_WRATH_PRIESTESS, m_afSecondNagaCoord[0][0], m_afSecondNagaCoord[0][1], m_afSecondNagaCoord[0][2], 0.0f, TEMPSPAWN_TIMED_OOC_DESPAWN, 60000);
                m_creature->SummonCreature(NPC_WRATH_MYRMIDON,  m_afSecondNagaCoord[1][0], m_afSecondNagaCoord[1][1], m_afSecondNagaCoord[1][2], 0.0f, TEMPSPAWN_TIMED_OOC_DESPAWN, 60000);
                m_creature->SummonCreature(NPC_WRATH_SEAWITCH,  m_afSecondNagaCoord[2][0], m_afSecondNagaCoord[2][1], m_afSecondNagaCoord[2][2], 0.0f, TEMPSPAWN_TIMED_OOC_DESPAWN, 60000);
                break;
            case 3:
                m_creature->SummonCreature(NPC_VORSHA, m_fVorshaCoord[0], m_fVorshaCoord[1], m_fVorshaCoord[2], 0.0f, TEMPSPAWN_TIMED_OOC_DESPAWN, 60000);
                break;
            case 4:
                SetEscortPaused(false);
                DoScriptText(SAY_MUG_DONE, m_creature);
                break;
        }
    }

    void JustSummoned(Creature* pSummoned) override
    {
        pSummoned->AI()->AttackStart(m_creature);
    }

    void UpdateEscortAI(const uint32 uiDiff) override
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->GetVictim())
        {
            if (HasEscortState(STATE_ESCORT_PAUSED) && m_bIsBrazierExtinguished)
            {
                if (m_uiEventTimer < uiDiff)
                {
                    ++m_uiWaveId;
                    DoWaveSummon();
                    m_uiEventTimer = 10000;
                }
                else
                    m_uiEventTimer -= uiDiff;
            }

            return;
        }

        DoMeleeAttackIfReady();
    }
};

bool QuestAccept_npc_muglash(Player* pPlayer, Creature* pCreature, const Quest* pQuest)
{
    if (pQuest->GetQuestId() == QUEST_VORSHA)
    {
        if (npc_muglashAI* pEscortAI = dynamic_cast<npc_muglashAI*>(pCreature->AI()))
        {
            DoScriptText(SAY_MUG_START1, pCreature);
            pCreature->SetFactionTemporary(FACTION_ESCORT_H_PASSIVE, TEMPFACTION_RESTORE_RESPAWN | TEMPFACTION_TOGGLE_IMMUNE_TO_NPC);

            pEscortAI->Start(false, pPlayer, pQuest);
        }
    }

    return true;
}

UnitAI* GetAI_npc_muglash(Creature* pCreature)
{
    return new npc_muglashAI(pCreature);
}

bool GOUse_go_naga_brazier(Player* /*pPlayer*/, GameObject* pGo)
{
    if (Creature* pCreature = GetClosestCreatureWithEntry(pGo, NPC_MUGLASH, INTERACTION_DISTANCE * 2))
    {
        if (npc_muglashAI* pEscortAI = dynamic_cast<npc_muglashAI*>(pCreature->AI()))
        {
            DoScriptText(SAY_MUG_BRAZIER_WAIT, pCreature);

            pEscortAI->m_bIsBrazierExtinguished = true;
            return false;
        }
    }

    return true;
}

/*####
# npc_ruul_snowhoof
####*/

enum
{
    QUEST_FREEDOM_TO_RUUL   = 6482,
    NPC_T_AVENGER           = 3925,
    NPC_T_SHAMAN            = 3924,
    NPC_T_PATHFINDER        = 3926,
    SPELL_RUUL_SHAPECHANGE  = 20514,
    SAY_RUUL_COMPLETE       = -1010022
};

static uint32 m_ruulAmbushers[3] = { NPC_T_AVENGER, NPC_T_SHAMAN, NPC_T_PATHFINDER};

static float m_ruulAmbushCoords[2][3] =
        {
                {3425.33f, -595.93f, 178.31f},    // First ambush
                {3245.34f, -506.66f, 150.05f},    // Second ambush
        };

struct npc_ruul_snowhoofAI : public npc_escortAI
{
    npc_ruul_snowhoofAI(Creature* creature) : npc_escortAI(creature) { Reset(); }

    void Reset() override
    {
        DoCastSpellIfCan(m_creature, SPELL_RUUL_SHAPECHANGE, TRIGGERED_OLD_TRIGGERED);
    }

    void DoSpawnAmbush(uint8 index)
    {
        for (auto ambusherEntry : m_ruulAmbushers)
        {
            float fx, fy, fz;
            m_creature->GetRandomPoint(m_ruulAmbushCoords[index][0], m_ruulAmbushCoords[index][1], m_ruulAmbushCoords[index][2], 7.0f, fx, fy, fz);
            if (Creature* ambusher = m_creature->SummonCreature(ambusherEntry, fx, fy, fz, 0, TEMPSPAWN_DEAD_DESPAWN, 60 * IN_MILLISECONDS))
            {
                ambusher->SetWalk(false);
                ambusher->GetMotionMaster()->MovePoint(0, m_creature->GetPositionX(), m_creature->GetPositionY(), m_creature->GetPositionZ());
            }
        }
    }

    void WaypointReached(uint32 pointId) override
    {
        switch (pointId)
        {
            case 14:
                DoSpawnAmbush(0);
                break;
            case 31:
                DoSpawnAmbush(1);
                break;
            case 32:
                m_creature->SetImmuneToNPC(true);
                m_creature->RemoveAurasDueToSpell(SPELL_RUUL_SHAPECHANGE);
                if (Player* player = GetPlayerForEscort())
                {
                    m_creature->SetFacingToObject(player);
                    player->RewardPlayerAndGroupAtEventExplored(QUEST_FREEDOM_TO_RUUL, m_creature);
                }
                break;
            case 33:
                if (Player* player = GetPlayerForEscort())
                {
                    DoScriptText(SAY_RUUL_COMPLETE, m_creature, player);
                    m_creature->SetFacingToObject(player);
                }
                m_creature->HandleEmote(EMOTE_ONESHOT_BOW);
                m_creature->SetWalk(false);
                break;
            case 34:
                DoCastSpellIfCan(m_creature, SPELL_RUUL_SHAPECHANGE);
                break;
            case 36:
                m_creature->SetImmuneToNPC(false);
                m_creature->SetWalk(true);
                m_creature->ForcedDespawn();
        }
    }
};

bool QuestAccept_npc_ruul_snowhoof(Player* player, Creature* creature, const Quest* quest)
{
    if (quest->GetQuestId() == QUEST_FREEDOM_TO_RUUL)
    {
        creature->SetFactionTemporary(FACTION_ESCORT_H_NEUTRAL_PASSIVE, TEMPFACTION_RESTORE_RESPAWN | TEMPFACTION_TOGGLE_IMMUNE_TO_PLAYER);
        creature->SetStandState(UNIT_STAND_STATE_STAND);

        if (npc_ruul_snowhoofAI* escortAI = dynamic_cast<npc_ruul_snowhoofAI*>(creature->AI()))
            escortAI->Start(false, player, quest);
    }
    return true;
}

UnitAI* GetAI_npc_ruul_snowhoofAI(Creature* pCreature)
{
    return new npc_ruul_snowhoofAI(pCreature);
}

/*####
# npc_torek
####*/

enum
{
    SAY_READY                   = -1000106,
    SAY_MOVE                    = -1000107,
    SAY_PREPARE                 = -1000108,
    SAY_WIN                     = -1000109,
    SAY_END                     = -1000110,

    SPELL_REND                  = 11977,
    SPELL_THUNDERCLAP           = 8078,

    QUEST_TOREK_ASSULT          = 6544,

    NPC_SPLINTERTREE_RAIDER     = 12859,
    NPC_DURIEL                  = 12860,
    NPC_SILVERWING_SENTINEL     = 12896,
    NPC_SILVERWING_WARRIOR      = 12897
};

struct npc_torekAI : public npc_escortAI
{
    npc_torekAI(Creature* pCreature) : npc_escortAI(pCreature) {Reset();}

    uint32 m_uiRend_Timer;
    uint32 m_uiThunderclap_Timer;

    void Reset() override
    {
        m_uiRend_Timer = 5000;
        m_uiThunderclap_Timer = 8000;
    }

    void WaypointReached(uint32 uiPointId) override
    {
        Player* pPlayer = GetPlayerForEscort();

        if (!pPlayer)
            return;

        switch (uiPointId)
        {
            case 2:
                DoScriptText(SAY_MOVE, m_creature, pPlayer);
                break;
            case 9:
                DoScriptText(SAY_PREPARE, m_creature, pPlayer);
                break;
            case 20:
                // TODO: verify location and creatures amount.
                m_creature->SummonCreature(NPC_DURIEL, 1776.73f, -2049.06f, 109.83f, 1.54f, TEMPSPAWN_TIMED_OOC_DESPAWN, 25000);
                m_creature->SummonCreature(NPC_SILVERWING_SENTINEL, 1774.64f, -2049.41f, 109.83f, 1.40f, TEMPSPAWN_TIMED_OOC_DESPAWN, 25000);
                m_creature->SummonCreature(NPC_SILVERWING_WARRIOR, 1778.73f, -2049.50f, 109.83f, 1.67f, TEMPSPAWN_TIMED_OOC_DESPAWN, 25000);
                break;
            case 21:
                DoScriptText(SAY_WIN, m_creature, pPlayer);
                pPlayer->RewardPlayerAndGroupAtEventExplored(QUEST_TOREK_ASSULT, m_creature);
                break;
            case 22:
                DoScriptText(SAY_END, m_creature, pPlayer);
                break;
        }
    }

    void JustSummoned(Creature* pSummoned) override
    {
        pSummoned->AI()->AttackStart(m_creature);
    }

    void UpdateEscortAI(const uint32 uiDiff) override
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->GetVictim())
            return;

        if (m_uiRend_Timer < uiDiff)
        {
            DoCastSpellIfCan(m_creature->GetVictim(), SPELL_REND);
            m_uiRend_Timer = 20000;
        }
        else
            m_uiRend_Timer -= uiDiff;

        if (m_uiThunderclap_Timer < uiDiff)
        {
            DoCastSpellIfCan(m_creature, SPELL_THUNDERCLAP);
            m_uiThunderclap_Timer = 30000;
        }
        else
            m_uiThunderclap_Timer -= uiDiff;

        DoMeleeAttackIfReady();
    }
};

bool QuestAccept_npc_torek(Player* pPlayer, Creature* pCreature, const Quest* pQuest)
{
    if (pQuest->GetQuestId() == QUEST_TOREK_ASSULT)
    {
        // TODO: find companions, make them follow Torek, at any time (possibly done by mangos/database in future?)
        DoScriptText(SAY_READY, pCreature, pPlayer);

        if (npc_torekAI* pEscortAI = dynamic_cast<npc_torekAI*>(pCreature->AI()))
            pEscortAI->Start(true, pPlayer, pQuest);
    }

    return true;
}

UnitAI* GetAI_npc_torek(Creature* pCreature)
{
    return new npc_torekAI(pCreature);
}

/*####
# npc_feero_ironhand
####*/

enum
{
    SAY_QUEST_START             = -1000771,
    SAY_FIRST_AMBUSH_START      = -1000772,
    SAY_FIRST_AMBUSH_END        = -1000773,
    SAY_SECOND_AMBUSH_START     = -1000774,
    SAY_SCOUT_SECOND_AMBUSH     = -1000775,
    SAY_SECOND_AMBUSH_END       = -1000776,
    SAY_FINAL_AMBUSH_START      = -1000777,
    SAY_BALIZAR_FINAL_AMBUSH    = -1000778,
    SAY_FINAL_AMBUSH_ATTACK     = -1000779,
    SAY_QUEST_END               = -1000780,

    QUEST_SUPPLIES_TO_AUBERDINE = 976,

    NPC_DARK_STRAND_ASSASSIN    = 3879,
    NPC_FORSAKEN_SCOUT          = 3893,

    NPC_ALIGAR_THE_TORMENTOR    = 3898,
    NPC_BALIZAR_THE_UMBRAGE     = 3899,
    NPC_CAEDAKAR_THE_VICIOUS    = 3900,
};

/*
 * Notes about the event:
 * The summon coords and event sequence are guesswork based on the comments from wowhead and wowwiki
 */

// Distance, Angle or Offset
static const float aSummonPositions[2][2] =
{
    {30.0f, 1.25f},
    {30.0f, 0.95f}
};

// Hardcoded positions for the last 3 mobs
static const float aEliteSummonPositions[3][4] =
{
    {4243.12f, 108.22f, 38.12f, 3.62f},
    {4240.95f, 114.04f, 38.35f, 3.56f},
    {4235.78f, 118.09f, 38.08f, 4.12f}
};

struct npc_feero_ironhandAI : public npc_escortAI
{
    npc_feero_ironhandAI(Creature* pCreature) : npc_escortAI(pCreature)
    {
        Reset();
    }

    uint8 m_uiCreaturesCount;
    bool m_bIsAttacked;

    void Reset() override
    {
        if (!HasEscortState(STATE_ESCORT_ESCORTING))
        {
            m_uiCreaturesCount  = 0;
            m_bIsAttacked       = false;
        }
    }

    void WaypointReached(uint32 uiPointId) override
    {
        switch (uiPointId)
        {
            case 15:
                // Prepare the first ambush
                DoScriptText(SAY_FIRST_AMBUSH_START, m_creature);
                for (uint8 i = 0; i < 4; ++i)
                    DoSpawnMob(NPC_DARK_STRAND_ASSASSIN, aSummonPositions[0][0], aSummonPositions[0][1] - M_PI_F / 4 * i);
                break;
            case 21:
                // Prepare the second ambush
                DoScriptText(SAY_SECOND_AMBUSH_START, m_creature);
                for (uint8 i = 0; i < 3; ++i)
                    DoSpawnMob(NPC_FORSAKEN_SCOUT, aSummonPositions[1][0], aSummonPositions[1][1] - M_PI_F / 3 * i);
                break;
            case 30:
                // Final ambush
                DoScriptText(SAY_FINAL_AMBUSH_START, m_creature);
                m_creature->SummonCreature(NPC_BALIZAR_THE_UMBRAGE, aEliteSummonPositions[0][0], aEliteSummonPositions[0][1], aEliteSummonPositions[0][2], aEliteSummonPositions[0][3], TEMPSPAWN_TIMED_OOC_DESPAWN, 20000);
                m_creature->SummonCreature(NPC_ALIGAR_THE_TORMENTOR, aEliteSummonPositions[1][0], aEliteSummonPositions[1][1], aEliteSummonPositions[1][2], aEliteSummonPositions[1][3], TEMPSPAWN_TIMED_OOC_DESPAWN, 20000);
                m_creature->SummonCreature(NPC_CAEDAKAR_THE_VICIOUS, aEliteSummonPositions[2][0], aEliteSummonPositions[2][1], aEliteSummonPositions[2][2], aEliteSummonPositions[2][3], TEMPSPAWN_TIMED_OOC_DESPAWN, 20000);
                break;
            case 31:
                // Complete the quest
                if (Player* pPlayer = GetPlayerForEscort())
                    pPlayer->RewardPlayerAndGroupAtEventExplored(QUEST_SUPPLIES_TO_AUBERDINE, m_creature);
                break;
        }
    }

    void AttackedBy(Unit* pWho) override
    {
        // Yell only at the first attack
        if (!m_bIsAttacked)
        {
            if (((Creature*)pWho)->GetEntry() == NPC_BALIZAR_THE_UMBRAGE)
            {
                DoScriptText(SAY_FINAL_AMBUSH_ATTACK, m_creature);
                m_bIsAttacked = true;
            }
        }
    }

    // Summon mobs at calculated points
    void DoSpawnMob(uint32 uiEntry, float fDistance, float fAngle)
    {
        float fX, fY, fZ;
        m_creature->GetNearPoint(m_creature, fX, fY, fZ, 0, fDistance, fAngle);

        m_creature->SummonCreature(uiEntry, fX, fY, fZ, 0, TEMPSPAWN_TIMED_OOC_DESPAWN, 20000);
    }

    void SummonedCreatureJustDied(Creature* pSummoned) override
    {
        --m_uiCreaturesCount;

        if (!m_uiCreaturesCount)
        {
            switch (pSummoned->GetEntry())
            {
                case NPC_DARK_STRAND_ASSASSIN:
                    DoScriptText(SAY_FIRST_AMBUSH_END, m_creature);
                    break;
                case NPC_FORSAKEN_SCOUT:
                    DoScriptText(SAY_SECOND_AMBUSH_END, m_creature);
                    break;
                case NPC_ALIGAR_THE_TORMENTOR:
                case NPC_BALIZAR_THE_UMBRAGE:
                case NPC_CAEDAKAR_THE_VICIOUS:
                    DoScriptText(SAY_QUEST_END, m_creature);
                    break;
            }
        }
    }

    void JustSummoned(Creature* pSummoned) override
    {
        if (pSummoned->GetEntry() == NPC_FORSAKEN_SCOUT)
        {
            // Only one of the scouts yells
            if (m_uiCreaturesCount == 1)
                DoScriptText(SAY_SCOUT_SECOND_AMBUSH, pSummoned, m_creature);
        }
        else if (pSummoned->GetEntry() == NPC_BALIZAR_THE_UMBRAGE)
            DoScriptText(SAY_BALIZAR_FINAL_AMBUSH, pSummoned);

        ++m_uiCreaturesCount;
        pSummoned->AI()->AttackStart(m_creature);
    }
};

UnitAI* GetAI_npc_feero_ironhand(Creature* pCreature)
{
    return new npc_feero_ironhandAI(pCreature);
}

bool QuestAccept_npc_feero_ironhand(Player* pPlayer, Creature* pCreature, const Quest* pQuest)
{
    if (pQuest->GetQuestId() == QUEST_SUPPLIES_TO_AUBERDINE)
    {
        DoScriptText(SAY_QUEST_START, pCreature, pPlayer);
        pCreature->SetFactionTemporary(FACTION_ESCORT_A_NEUTRAL_PASSIVE, TEMPFACTION_RESTORE_RESPAWN | TEMPFACTION_TOGGLE_IMMUNE_TO_NPC);

        if (npc_feero_ironhandAI* pEscortAI = dynamic_cast<npc_feero_ironhandAI*>(pCreature->AI()))
            pEscortAI->Start(true, pPlayer, pQuest, true);
    }

    return true;
}

void AddSC_ashenvale()
{
    Script* pNewScript = new Script;
    pNewScript->Name = "npc_muglash";
    pNewScript->GetAI = &GetAI_npc_muglash;
    pNewScript->pQuestAcceptNPC = &QuestAccept_npc_muglash;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "go_naga_brazier";
    pNewScript->pGOUse = &GOUse_go_naga_brazier;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_ruul_snowhoof";
    pNewScript->GetAI = &GetAI_npc_ruul_snowhoofAI;
    pNewScript->pQuestAcceptNPC = &QuestAccept_npc_ruul_snowhoof;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_torek";
    pNewScript->GetAI = &GetAI_npc_torek;
    pNewScript->pQuestAcceptNPC = &QuestAccept_npc_torek;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_feero_ironhand";
    pNewScript->GetAI = &GetAI_npc_feero_ironhand;
    pNewScript->pQuestAcceptNPC = &QuestAccept_npc_feero_ironhand;
    pNewScript->RegisterSelf();
}
