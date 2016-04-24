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
SDName: Desolace
SD%Complete: 100
SDComment: Quest support: 1440, 5561, 5821, 5943, 6132
SDCategory: Desolace
EndScriptData */

/* ContentData
npc_aged_dying_ancient_kodo
npc_dalinda_malem
npc_melizza_brimbuzzle
EndContentData */

#include "precompiled.h"
#include "escort_ai.h"

/*######
## npc_aged_dying_ancient_kodo
######*/

enum
{
    SAY_SMEED_HOME_1                = -1000348,
    SAY_SMEED_HOME_2                = -1000349,
    SAY_SMEED_HOME_3                = -1000350,

    QUEST_KODO                      = 5561,

    NPC_SMEED                       = 11596,
    NPC_AGED_KODO                   = 4700,
    NPC_DYING_KODO                  = 4701,
    NPC_ANCIENT_KODO                = 4702,
    NPC_TAMED_KODO                  = 11627,

    SPELL_KODO_KOMBO_ITEM           = 18153,
    SPELL_KODO_KOMBO_PLAYER_BUFF    = 18172,                // spells here have unclear function, but using them at least for visual parts and checks
    SPELL_KODO_KOMBO_DESPAWN_BUFF   = 18377,
    SPELL_KODO_KOMBO_GOSSIP         = 18362
};

struct npc_aged_dying_ancient_kodoAI : public ScriptedAI
{
    npc_aged_dying_ancient_kodoAI(Creature* pCreature) : ScriptedAI(pCreature) { Reset(); }

    uint32 m_uiDespawnTimer;

    void Reset() override
    {
        m_uiDespawnTimer = 0;
    }

    void MoveInLineOfSight(Unit* pWho) override
    {
        if (pWho->GetEntry() == NPC_SMEED)
        {
            if (m_creature->HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP))
                return;

            if (m_creature->IsWithinDistInMap(pWho, 10.0f))
            {
                switch (urand(0, 2))
                {
                    case 0: DoScriptText(SAY_SMEED_HOME_1, pWho); break;
                    case 1: DoScriptText(SAY_SMEED_HOME_2, pWho); break;
                    case 2: DoScriptText(SAY_SMEED_HOME_3, pWho); break;
                }

                // spell have no implemented effect (dummy), so useful to notify spellHit
                m_creature->CastSpell(m_creature, SPELL_KODO_KOMBO_GOSSIP, true);
            }
        }
    }

    void SpellHit(Unit* /*pCaster*/, SpellEntry const* pSpell) override
    {
        if (pSpell->Id == SPELL_KODO_KOMBO_GOSSIP)
        {
            m_creature->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
            m_uiDespawnTimer = 60000;
        }
    }

    void UpdateAI(const uint32 diff) override
    {
        // timer should always be == 0 unless we already updated entry of creature. Then not expect this updated to ever be in combat.
        if (m_uiDespawnTimer && m_uiDespawnTimer <= diff)
        {
            if (!m_creature->getVictim() && m_creature->isAlive())
            {
                Reset();
                m_creature->SetDeathState(JUST_DIED);
                m_creature->Respawn();
                return;
            }
        }
        else m_uiDespawnTimer -= diff;

        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_aged_dying_ancient_kodo(Creature* pCreature)
{
    return new npc_aged_dying_ancient_kodoAI(pCreature);
}

bool EffectDummyCreature_npc_aged_dying_ancient_kodo(Unit* pCaster, uint32 spellId, SpellEffectIndex effIndex, Creature* pCreatureTarget, ObjectGuid /*originalCasterGuid*/)
{
    // always check spellid and effectindex
    if (spellId == SPELL_KODO_KOMBO_ITEM && effIndex == EFFECT_INDEX_0)
    {
        // no effect if player/creature already have aura from spells
        if (pCaster->HasAura(SPELL_KODO_KOMBO_PLAYER_BUFF) || pCreatureTarget->HasAura(SPELL_KODO_KOMBO_DESPAWN_BUFF))
            return true;

        if (pCreatureTarget->GetEntry() == NPC_AGED_KODO ||
                pCreatureTarget->GetEntry() == NPC_DYING_KODO ||
                pCreatureTarget->GetEntry() == NPC_ANCIENT_KODO)
        {
            pCaster->CastSpell(pCaster, SPELL_KODO_KOMBO_PLAYER_BUFF, true);

            pCreatureTarget->UpdateEntry(NPC_TAMED_KODO);
            pCreatureTarget->CastSpell(pCreatureTarget, SPELL_KODO_KOMBO_DESPAWN_BUFF, false);

            if (pCreatureTarget->GetMotionMaster()->GetCurrentMovementGeneratorType() == WAYPOINT_MOTION_TYPE)
                pCreatureTarget->GetMotionMaster()->MoveIdle();

            pCreatureTarget->GetMotionMaster()->MoveFollow(pCaster, PET_FOLLOW_DIST, PET_FOLLOW_ANGLE);
        }

        // always return true when we are handling this spell and effect
        return true;
    }
    return false;
}

bool GossipHello_npc_aged_dying_ancient_kodo(Player* pPlayer, Creature* pCreature)
{
    if (pPlayer->HasAura(SPELL_KODO_KOMBO_PLAYER_BUFF) && pCreature->HasAura(SPELL_KODO_KOMBO_DESPAWN_BUFF))
    {
        // the expected quest objective
        pPlayer->TalkedToCreature(pCreature->GetEntry(), pCreature->GetObjectGuid());

        pPlayer->RemoveAurasDueToSpell(SPELL_KODO_KOMBO_PLAYER_BUFF);
        pCreature->GetMotionMaster()->MoveIdle();
    }

    pPlayer->SEND_GOSSIP_MENU(pPlayer->GetGossipTextId(pCreature), pCreature->GetObjectGuid());
    return true;
}

/*######
## npc_dalinda_malem
######*/

enum
{
    QUEST_RETURN_TO_VAHLARRIEL  = 1440,
};

struct npc_dalinda_malemAI : public npc_escortAI
{
    npc_dalinda_malemAI(Creature* m_creature) : npc_escortAI(m_creature) { Reset(); }

    void Reset() override {}

    void JustStartedEscort() override
    {
        m_creature->SetStandState(UNIT_STAND_STATE_STAND);
    }

    void WaypointReached(uint32 uiPointId) override
    {
        if (uiPointId == 18)
        {
            if (Player* pPlayer = GetPlayerForEscort())
                pPlayer->GroupEventHappens(QUEST_RETURN_TO_VAHLARRIEL, m_creature);
        }
    }
};

CreatureAI* GetAI_npc_dalinda_malem(Creature* pCreature)
{
    return new npc_dalinda_malemAI(pCreature);
}

bool QuestAccept_npc_dalinda_malem(Player* pPlayer, Creature* pCreature, const Quest* pQuest)
{
    if (pQuest->GetQuestId() == QUEST_RETURN_TO_VAHLARRIEL)
    {
        if (npc_dalinda_malemAI* pEscortAI = dynamic_cast<npc_dalinda_malemAI*>(pCreature->AI()))
        {
            // TODO This faction change needs confirmation, also possible that we need to drop her PASSIVE flag
            pCreature->SetFactionTemporary(FACTION_ESCORT_A_NEUTRAL_PASSIVE, TEMPFACTION_RESTORE_RESPAWN | TEMPFACTION_TOGGLE_PASSIVE);
            pEscortAI->Start(false, pPlayer, pQuest);
        }
    }
    return true;
}

/*######
## npc_melizza_brimbuzzle
######*/

enum
{
    QUEST_GET_ME_OUT_OF_HERE    = 6132,

    GO_MELIZZAS_CAGE            = 177706,

    SAY_MELIZZA_START           = -1000784,
    SAY_MELIZZA_FINISH          = -1000785,
    SAY_MELIZZA_1               = -1000786,
    SAY_MELIZZA_2               = -1000787,
    SAY_MELIZZA_3               = -1000788,

    NPC_MARAUDINE_MARAUDER      = 4659,
    NPC_MARAUDINE_BONEPAW       = 4660,
    NPC_MARAUDINE_WRANGLER      = 4655,

    NPC_MELIZZA                 = 12277,

    POINT_ID_QUEST_COMPLETE     = 1,
    POINT_ID_EVENT_COMPLETE     = 2,

    MAX_MARAUDERS               = 2,
    MAX_WRANGLERS               = 3,
};

static const DialogueEntry aIntroDialogue[] =
{
    {POINT_ID_QUEST_COMPLETE,   0,              1000},
    {QUEST_GET_ME_OUT_OF_HERE,  NPC_MELIZZA,    0},
    {POINT_ID_EVENT_COMPLETE,   0,              2000},
    {SAY_MELIZZA_1,             NPC_MELIZZA,    4000},
    {SAY_MELIZZA_2,             NPC_MELIZZA,    5000},
    {SAY_MELIZZA_3,             NPC_MELIZZA,    4000},
    {NPC_MELIZZA,               0,              0},
    {0, 0, 0},
};

struct SummonLocation
{
    float m_fX, m_fY, m_fZ;
};

static const SummonLocation aMarauderSpawn[] =
{
    { -1291.492f, 2644.650f, 111.556f},
    { -1306.730f, 2675.163f, 111.561f},
};

static const SummonLocation wranglerSpawn = { -1393.194f, 2429.465f, 88.689f };

struct npc_melizza_brimbuzzleAI : public npc_escortAI, private DialogueHelper
{
    npc_melizza_brimbuzzleAI(Creature* m_creature) : npc_escortAI(m_creature),
        DialogueHelper(aIntroDialogue)
    {
        Reset();
    }

    void Reset() override {}

    void JustStartedEscort() override
    {
        if (GameObject* pCage = GetClosestGameObjectWithEntry(m_creature, GO_MELIZZAS_CAGE, INTERACTION_DISTANCE))
            pCage->UseDoorOrButton();
    }

    Creature* GetSpeakerByEntry(uint32 uiEntry) override
    {
        if (uiEntry == NPC_MELIZZA)
            return m_creature;

        return nullptr;
    }

    void WaypointReached(uint32 uiPointId) override
    {
        switch (uiPointId)
        {
            case 1:
                if (Player* pPlayer = GetPlayerForEscort())
                    DoScriptText(SAY_MELIZZA_START, m_creature, pPlayer);

                m_creature->SetFactionTemporary(FACTION_ESCORT_N_NEUTRAL_PASSIVE, TEMPFACTION_RESTORE_RESPAWN);
                break;
            case 4:
                for (uint8 i = 0; i < MAX_MARAUDERS; ++i)
                {
                    for (uint8 j = 0; j < MAX_MARAUDERS; ++j)
                    {
                        // Summon 2 Marauders on each point
                        float fX, fY, fZ;
                        m_creature->GetRandomPoint(aMarauderSpawn[i].m_fX, aMarauderSpawn[i].m_fY, aMarauderSpawn[i].m_fZ, 7.0f, fX, fY, fZ);
                        m_creature->SummonCreature(NPC_MARAUDINE_MARAUDER, fX, fY, fZ, 0.0f, TEMPSUMMON_DEAD_DESPAWN, 0);
                    }
                }
                break;
            case 9:
                for (uint8 i = 0; i < MAX_WRANGLERS; ++i)
                {
                    float fX, fY, fZ;
                    m_creature->GetRandomPoint(wranglerSpawn.m_fX, wranglerSpawn.m_fY, wranglerSpawn.m_fZ, 10.0f, fX, fY, fZ);
                    m_creature->SummonCreature(NPC_MARAUDINE_BONEPAW, fX, fY, fZ, 0.0f, TEMPSUMMON_DEAD_DESPAWN, 0);

                    m_creature->GetRandomPoint(wranglerSpawn.m_fX, wranglerSpawn.m_fY, wranglerSpawn.m_fZ, 10.0f, fX, fY, fZ);
                    m_creature->SummonCreature(NPC_MARAUDINE_WRANGLER, fX, fY, fZ, 0.0f, TEMPSUMMON_DEAD_DESPAWN, 0);
                }
                break;
            case 12:
                StartNextDialogueText(POINT_ID_QUEST_COMPLETE);
                break;
            case 19:
                StartNextDialogueText(POINT_ID_EVENT_COMPLETE);
                break;
        }
    }

    void JustDidDialogueStep(int32 iEntry) override
    {
        switch (iEntry)
        {
            case POINT_ID_QUEST_COMPLETE:
                SetEscortPaused(true);
                break;
            case QUEST_GET_ME_OUT_OF_HERE:
                if (Player* pPlayer = GetPlayerForEscort())
                {
                    DoScriptText(SAY_MELIZZA_FINISH, m_creature, pPlayer);
                    pPlayer->GroupEventHappens(QUEST_GET_ME_OUT_OF_HERE, m_creature);
                }

                m_creature->ClearTemporaryFaction();
                SetRun(true);
                SetEscortPaused(false);
                break;
            case POINT_ID_EVENT_COMPLETE:
                SetEscortPaused(true);
                m_creature->SetFacingTo(4.71f);
                break;
            case NPC_MELIZZA:
                SetEscortPaused(false);
                break;
        }
    }

    void UpdateEscortAI(const uint32 uiDiff) override
    {
        DialogueUpdate(uiDiff);

        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_melizza_brimbuzzle(Creature* pCreature)
{
    return new npc_melizza_brimbuzzleAI(pCreature);
}

bool QuestAccept_npc_melizza_brimbuzzle(Player* pPlayer, Creature* pCreature, const Quest* pQuest)
{
    if (pQuest->GetQuestId() == QUEST_GET_ME_OUT_OF_HERE)
    {
        if (npc_melizza_brimbuzzleAI* pEscortAI = dynamic_cast<npc_melizza_brimbuzzleAI*>(pCreature->AI()))
            pEscortAI->Start(false, pPlayer, pQuest);
    }

    return true;
}

/*######
## npc_cork_gizelton
######*/

enum
{
    SAY_CORK_AMBUSH1        = -1001191,
    SAY_CORK_AMBUSH2        = -1001192,
    SAY_CORK_AMBUSH3        = -1001193,
    SAY_CORK_END            = -1001194,
    SAY_RIGGER_AMBUSH1      = -1001195,
    SAY_RIGGER_AMBUSH2      = -1001196,
    SAY_RIGGER_END          = -1001197,

    NPC_CORK_GIZELTON       = 11625,
    NPC_RIGGER_GIZELTON     = 11626,
    NPC_VENDOR_TRON         = 12245,
    NPC_SUPER_SELLER        = 12246,

    NPC_KOLKAR_WAYLAYER     = 12976,
    NPC_KOLKAR_AMBUSHER     = 12977,

    NPC_DOOMWARDER          = 4677,
    NPC_LESSER_INFERNAL     = 4676,
    NPC_NETHER_SORCERESS    = 4684,

    QUEST_BODYGUARD_TO_HIRE = 5821,
    QUEST_GIZELTON_CARAVAN  = 5943,
};

static const SummonLocation aAmbushLocsBodyGuard[12] =
{
    // Quest QUEST_BODYGUARD_TO_HIRE
    {-969.05f, 1174.91f, 90.39f},       // First ambush
    {-985.71f, 1173.95f, 91.02f},
    {-983.01f, 1192.88f, 90.01f},
    {-965.51f, 1193.58f, 92.15f},
    {-1147.83f, 1180.87f, 91.38f},      // Second ambush
    {-1163.96f, 1183.72f, 93.79f},
    {-1160.97f, 1201.36f, 93.15f},
    {-1146.20f, 1199.75f, 91.37f},
    {-1277.78f, 1218.56f, 109.30f},     // Third ambush
    {-1292.65f, 1221.28f, 109.99f},
    {-1289.25f, 1239.20f, 108.79f},
    {-1272.91f, 1234.39f, 108.14f},
};

static const SummonLocation aAmbushLocsGizelton[9] =
{
    // Quest QUEST_GIZELTON_CARAVAN
    {-1823.7f, 2060.88f, 62.0925f},     // First ambush
    {-1814.46f, 2060.13f, 62.4916f},
    {-1814.87f, 2080.6f, 63.6323f},
    {-1782.92f, 1942.55f, 60.2205f},    // Second ambush
    {-1786.5f, 1926.05f, 59.7502f},
    {-1805.74f, 1942.77f, 60.791f},
    {-1677.56f, 1835.67f, 58.9269f},    // Third ambush
    {-1675.66f, 1863.0f, 59.0008f},
    {-1692.31f, 1862.69f, 58.9553f},
};

static const uint32 AmbushersBodyguard[4] = { NPC_KOLKAR_WAYLAYER, NPC_KOLKAR_AMBUSHER, NPC_KOLKAR_WAYLAYER, NPC_KOLKAR_AMBUSHER };
static const uint32 AmbushersGizleton[3] = { NPC_NETHER_SORCERESS, NPC_LESSER_INFERNAL, NPC_DOOMWARDER };

struct npc_cork_gizeltonAI : public ScriptedAI
{
    npc_cork_gizeltonAI(Creature* pCreature) : ScriptedAI(pCreature) { Reset(); }

    ObjectGuid m_playerGuid;
    uint8 uiQuestStatus;

    void Reset() override
    {
        uiQuestStatus = 0;
    }

    void ReceiveAIEvent(AIEventType eventType, Creature* /*pSender*/, Unit* pInvoker, uint32 uiMiscValue) override
    {
        if (eventType == AI_EVENT_START_ESCORT && pInvoker->GetTypeId() == TYPEID_PLAYER)
            m_playerGuid = pInvoker->GetObjectGuid();
    }

    // Custom function to handle event ambushes
    void DoAmbush(uint32 uiQuestId, uint8 uiAmbushPoint)
    {
        uiAmbushPoint--;
        switch (uiQuestId)
        {
            case QUEST_BODYGUARD_TO_HIRE:
                // Summon 2 NPC_KOLKAR_WAYLAYER and 2 NPC_KOLKAR_AMBUSHER
                for (uint8 i = 0; i < 4; ++i)
                {
                    float fX, fY, fZ;
                    m_creature->GetRandomPoint(aAmbushLocsBodyGuard[i + 4 * uiAmbushPoint].m_fX, aAmbushLocsBodyGuard[i + 4 * uiAmbushPoint].m_fY, aAmbushLocsBodyGuard[i + 4 * uiAmbushPoint].m_fZ, 7.0f, fX, fY, fZ);
                    m_creature->SummonCreature(AmbushersBodyguard[i], fX, fY, fZ, 0.0f, TEMPSUMMON_DEAD_DESPAWN, 0);
                }
                break;
            case QUEST_GIZELTON_CARAVAN:
                // Summon 1 NPC_NETHER_SORCERESS, 1 NPC_LESSER_INFERNAL and 1 NPC_DOOMWARDER
                for (uint8 i = 0; i < 3; ++i)
                {
                    float fX, fY, fZ;
                    m_creature->GetRandomPoint(aAmbushLocsGizelton[i + 3 * uiAmbushPoint].m_fX, aAmbushLocsGizelton[i + 3 * uiAmbushPoint].m_fY, aAmbushLocsGizelton[i + 3 * uiAmbushPoint].m_fZ, 7.0f, fX, fY, fZ);
                    m_creature->SummonCreature(AmbushersGizleton[i], fX, fY, fZ, 0.0f, TEMPSUMMON_DEAD_DESPAWN, 0);
                }
                break;
        }

        return;
    }

    void MovementInform(uint32 uiType, uint32 uiPointId) override
    {
        if (uiType != WAYPOINT_MOTION_TYPE)
            return;

        // No player assigned as quest taker: abort to avoid summoning adds
        Player* pPlayer = m_creature->GetMap()->GetPlayer(m_playerGuid);
        if (!pPlayer)
            return;

        if (pPlayer->GetQuestStatus(QUEST_BODYGUARD_TO_HIRE) == QUEST_STATUS_INCOMPLETE)
        {
            switch (uiPointId)
            {
                case 77:
                    uiQuestStatus = 1;
                // First Ambush
                case 96:
                    DoScriptText(SAY_CORK_AMBUSH1, m_creature);
                    DoAmbush(QUEST_BODYGUARD_TO_HIRE, 1);
                    break;
                // Second Ambush
                case 103:
                    DoScriptText(SAY_CORK_AMBUSH2, m_creature);
                    DoAmbush(QUEST_BODYGUARD_TO_HIRE, 2);
                    break;
                // Third Ambush
                case 111:
                    DoScriptText(SAY_CORK_AMBUSH3, m_creature);
                    DoAmbush(QUEST_BODYGUARD_TO_HIRE, 3);
                    break;
                case 116:
                    DoScriptText(SAY_CORK_END, m_creature);
                    // Award quest credit
                    if (pPlayer)
                        pPlayer->GroupEventHappens(QUEST_BODYGUARD_TO_HIRE, m_creature);
                    // Remove player to avoid adds being spawned again next turn
                    m_playerGuid.Clear();
                    uiQuestStatus = 0;
                    break;
            }
        }
        // The second escort quest is also handled by NPC Cork though it is given by NPC Rigger
        else if (pPlayer->GetQuestStatus(QUEST_GIZELTON_CARAVAN) == QUEST_STATUS_INCOMPLETE)
        {
            switch (uiPointId)
            {
                case 209:
                    uiQuestStatus = 2;
                    // First Ambush
                case 218:
                    if (Creature* pRigger = GetClosestCreatureWithEntry(m_creature, NPC_RIGGER_GIZELTON, 100.0f))
                        DoScriptText(SAY_RIGGER_AMBUSH1, pRigger);
                    DoAmbush(QUEST_GIZELTON_CARAVAN, 1);
                    break;
                    // Second Ambush
                case 225:
                    if (Creature* pRigger = GetClosestCreatureWithEntry(m_creature, NPC_RIGGER_GIZELTON, 100.0f))
                        DoScriptText(SAY_RIGGER_AMBUSH2, pRigger);
                    DoAmbush(QUEST_GIZELTON_CARAVAN, 2);
                    break;
                    // Third Ambush
                case 235:
                    if (Creature* pRigger = GetClosestCreatureWithEntry(m_creature, NPC_RIGGER_GIZELTON, 100.0f))
                        DoScriptText(SAY_RIGGER_AMBUSH1, pRigger);
                    DoAmbush(QUEST_GIZELTON_CARAVAN, 3);
                    break;
                case 241:
                    if (Creature* pRigger = GetClosestCreatureWithEntry(m_creature, NPC_RIGGER_GIZELTON, 100.0f))
                        DoScriptText(SAY_RIGGER_END, pRigger);
                    // Award quest credit
                    if (pPlayer)
                        pPlayer->GroupEventHappens(QUEST_GIZELTON_CARAVAN, m_creature);
                    // Remove player to avoid adds being spawned again next turn
                    m_playerGuid.Clear();
                    uiQuestStatus = 0;
                    break;
            }
        }
    }

    void JustSummoned(Creature* pSummoned) override
    {
        // By default: summoned for the two escort quests will attack
        // So we want to add a special case to avoid the two summoned NPC vendors to also attack
        if (pSummoned->GetEntry() != NPC_VENDOR_TRON && pSummoned->GetEntry() != NPC_SUPER_SELLER)
            pSummoned->AI()->AttackStart(m_creature);
    }

    void JustDied(Unit* /*pKiller*/) override
    {
        Player* pPlayer = m_creature->GetMap()->GetPlayer(m_playerGuid);
        if (!pPlayer)
            return;

        // Handle all players in group (if they took quest)
        if (Group* pGroup = pPlayer->GetGroup())
        {
            for (GroupReference* pRef = pGroup->GetFirstMember(); pRef != nullptr; pRef = pRef->next())
            {
                if (Player* pMember = pRef->getSource())
                {
                    if (pMember->GetQuestStatus(QUEST_BODYGUARD_TO_HIRE) == QUEST_STATUS_INCOMPLETE)
                        pMember->FailQuest(QUEST_BODYGUARD_TO_HIRE);
                    if (pMember->GetQuestStatus(QUEST_GIZELTON_CARAVAN) == QUEST_STATUS_INCOMPLETE)
                        pMember->FailQuest(QUEST_GIZELTON_CARAVAN);
                }
            }
        }
        else
        {
            if (pPlayer->GetQuestStatus(QUEST_BODYGUARD_TO_HIRE) == QUEST_STATUS_INCOMPLETE)
                pPlayer->FailQuest(QUEST_BODYGUARD_TO_HIRE);
            if (pPlayer->GetQuestStatus(QUEST_GIZELTON_CARAVAN) == QUEST_STATUS_INCOMPLETE)
                pPlayer->FailQuest(QUEST_GIZELTON_CARAVAN);
        }
    }
};

CreatureAI* GetAI_npc_cork_gizelton(Creature* pCreature)
{
    return new npc_cork_gizeltonAI(pCreature);
}

bool QuestAccept_npc_cork_gizelton(Player* pPlayer, Creature* pCreature, const Quest* pQuest)
{
    if (pQuest->GetQuestId() == QUEST_BODYGUARD_TO_HIRE)
    {
        if (pPlayer->GetTeam() == ALLIANCE)
            pCreature->SetFactionTemporary(FACTION_ESCORT_A_PASSIVE, TEMPFACTION_RESTORE_RESPAWN);

        if (pPlayer->GetTeam() == HORDE)
            pCreature->SetFactionTemporary(FACTION_ESCORT_H_PASSIVE, TEMPFACTION_RESTORE_RESPAWN);

        if (npc_cork_gizeltonAI* pCork = dynamic_cast<npc_cork_gizeltonAI*>(pCreature->AI()))
            pCreature->AI()->SendAIEvent(AI_EVENT_START_ESCORT, pPlayer, pCreature, pQuest->GetQuestId());
    }
    return true;
}

/*######
## npc_rigger_gizelton
######*/

struct npc_rigger_gizeltonAI : public ScriptedAI
{
    npc_rigger_gizeltonAI(Creature* pCreature) : ScriptedAI(pCreature) { Reset(); }

    void Reset() override {}
};

CreatureAI* GetAI_npc_rigger_gizelton(Creature* pCreature)
{
    return new npc_rigger_gizeltonAI(pCreature);
}

bool QuestAccept_npc_rigger_gizelton(Player* pPlayer, Creature* pCreature, const Quest* pQuest)
{
    if (pQuest->GetQuestId() == QUEST_GIZELTON_CARAVAN)
    {
        if (pPlayer->GetTeam() == ALLIANCE)
            pCreature->SetFactionTemporary(FACTION_ESCORT_A_PASSIVE, TEMPFACTION_RESTORE_RESPAWN);

        if (pPlayer->GetTeam() == HORDE)
            pCreature->SetFactionTemporary(FACTION_ESCORT_H_PASSIVE, TEMPFACTION_RESTORE_RESPAWN);

        // Now the quest is accepted, tell NPC Cork what player took it so it can handle quest credit/failure
        // because NPC Cork will handle both escort quests
        if (Creature* pCork = GetClosestCreatureWithEntry(pCreature, NPC_CORK_GIZELTON, 100.0f))
            pCreature->AI()->SendAIEvent(AI_EVENT_START_ESCORT, pPlayer, pCork, pQuest->GetQuestId());
    }

    return true;
}

void AddSC_desolace()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "npc_aged_dying_ancient_kodo";
    pNewScript->GetAI = &GetAI_npc_aged_dying_ancient_kodo;
    pNewScript->pEffectDummyNPC = &EffectDummyCreature_npc_aged_dying_ancient_kodo;
    pNewScript->pGossipHello = &GossipHello_npc_aged_dying_ancient_kodo;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_dalinda_malem";
    pNewScript->GetAI = &GetAI_npc_dalinda_malem;
    pNewScript->pQuestAcceptNPC = &QuestAccept_npc_dalinda_malem;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_melizza_brimbuzzle";
    pNewScript->GetAI = &GetAI_npc_melizza_brimbuzzle;
    pNewScript->pQuestAcceptNPC = &QuestAccept_npc_melizza_brimbuzzle;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_cork_gizelton";
    pNewScript->GetAI = &GetAI_npc_cork_gizelton;
    pNewScript->pQuestAcceptNPC = &QuestAccept_npc_cork_gizelton;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_rigger_gizelton";
    pNewScript->GetAI = &GetAI_npc_rigger_gizelton;
    pNewScript->pQuestAcceptNPC = &QuestAccept_npc_rigger_gizelton;
    pNewScript->RegisterSelf();
}
