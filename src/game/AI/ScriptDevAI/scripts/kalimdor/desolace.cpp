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
EndScriptData

*/

#include "AI/ScriptDevAI/include/sc_common.h"/* ContentData
npc_aged_dying_ancient_kodo
npc_dalinda_malem
npc_melizza_brimbuzzle
EndContentData */


#include "AI/ScriptDevAI/base/escort_ai.h"
#include "AI/ScriptDevAI/scripts/kalimdor/world_kalimdor.h"

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
    SPELL_KODO_KOMBO_GOSSIP         = 18362,
    SPELL_KODO_KOMBOBULATOR         = 18793,                // spell only exists in classic dbc, used by kodo on player after gossip menu is used
    SPELL_KODO_DESPAWN              = 22970,                // spell only exists in classic dbc, used by kodo during following?
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
                m_creature->CastSpell(m_creature, SPELL_KODO_KOMBO_GOSSIP, TRIGGERED_OLD_TRIGGERED);
            }
        }
    }

    void SpellHit(Unit* /*pCaster*/, SpellEntry const* pSpell) override
    {
        if (pSpell->Id == SPELL_KODO_KOMBO_GOSSIP)
        {
            m_creature->GetMotionMaster()->MoveIdle();
            m_creature->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
            m_uiDespawnTimer = 60000;
        }
    }

    void UpdateAI(const uint32 diff) override
    {
        // timer should always be == 0 unless we already updated entry of creature. Then not expect this updated to ever be in combat.
        if (m_uiDespawnTimer && m_uiDespawnTimer <= diff)
        {
            if (!m_creature->GetVictim() && m_creature->IsAlive())
            {
                Reset();
                m_creature->SetDeathState(JUST_DIED);
                m_creature->Respawn();
                return;
            }
        }
        else m_uiDespawnTimer -= diff;

        if (!m_creature->SelectHostileTarget() || !m_creature->GetVictim())
            return;

        DoMeleeAttackIfReady();
    }
};

UnitAI* GetAI_npc_aged_dying_ancient_kodo(Creature* pCreature)
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
            pCaster->CastSpell(pCaster, SPELL_KODO_KOMBO_PLAYER_BUFF, TRIGGERED_OLD_TRIGGERED);

            pCreatureTarget->UpdateEntry(NPC_TAMED_KODO);
            pCreatureTarget->CastSpell(pCreatureTarget, SPELL_KODO_KOMBO_DESPAWN_BUFF, TRIGGERED_NONE);

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
        pCreature->ForcedDespawn(10000);
    }

    pPlayer->PrepareGossipMenu(pCreature, pPlayer->GetDefaultGossipMenuForSource(pCreature));
    pPlayer->SendPreparedGossip(pCreature);
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
    npc_dalinda_malemAI(Creature* m_creature) : npc_escortAI(m_creature)
    {
        m_creature->SetStandState(UNIT_STAND_STATE_KNEEL, true);
        Reset();
    }

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
                pPlayer->RewardPlayerAndGroupAtEventExplored(QUEST_RETURN_TO_VAHLARRIEL, m_creature);
        }
    }
};

UnitAI* GetAI_npc_dalinda_malem(Creature* pCreature)
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
            pCreature->SetFactionTemporary(FACTION_ESCORT_A_NEUTRAL_PASSIVE, TEMPFACTION_RESTORE_RESPAWN | TEMPFACTION_TOGGLE_IMMUNE_TO_NPC);
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

    SAY_HORNIZZ_1               = -1010030,
    SAY_HORNIZZ_2               = -1010031,

    NPC_MARAUDINE_MARAUDER      = 4659,
    NPC_MARAUDINE_BONEPAW       = 4660,
    NPC_MARAUDINE_WRANGLER      = 4655,

    NPC_MELIZZA                 = 12277,
    // NPC_HORNIZZ                 = 6019,

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
    {NPC_MELIZZA,               0,              6000},
    {SAY_HORNIZZ_1,             NPC_HORNIZZ,    10000},
    {SAY_HORNIZZ_2,             NPC_HORNIZZ,    0},
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

        if (uiEntry == NPC_HORNIZZ)
            if (ScriptedInstance* instance = static_cast<ScriptedInstance*>(m_creature->GetInstanceData()))
                return instance->GetSingleCreatureFromStorage(NPC_HORNIZZ);

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
                for (auto i : aMarauderSpawn)
                {
                    for (uint8 j = 0; j < MAX_MARAUDERS; ++j)
                    {
                        // Summon 2 Marauders on each point
                        float fX, fY, fZ;
                        m_creature->GetRandomPoint(i.m_fX, i.m_fY, i.m_fZ, 7.0f, fX, fY, fZ);
                        m_creature->SummonCreature(NPC_MARAUDINE_MARAUDER, fX, fY, fZ, 0.0f, TEMPSPAWN_DEAD_DESPAWN, 0);
                    }
                }
                break;
            case 9:
                for (uint8 i = 0; i < MAX_WRANGLERS; ++i)
                {
                    float fX, fY, fZ;
                    m_creature->GetRandomPoint(wranglerSpawn.m_fX, wranglerSpawn.m_fY, wranglerSpawn.m_fZ, 10.0f, fX, fY, fZ);
                    m_creature->SummonCreature(NPC_MARAUDINE_BONEPAW, fX, fY, fZ, 0.0f, TEMPSPAWN_DEAD_DESPAWN, 0);

                    m_creature->GetRandomPoint(wranglerSpawn.m_fX, wranglerSpawn.m_fY, wranglerSpawn.m_fZ, 10.0f, fX, fY, fZ);
                    m_creature->SummonCreature(NPC_MARAUDINE_WRANGLER, fX, fY, fZ, 0.0f, TEMPSPAWN_DEAD_DESPAWN, 0);
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
                    pPlayer->RewardPlayerAndGroupAtEventExplored(QUEST_GET_ME_OUT_OF_HERE, m_creature);
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

        if (!m_creature->SelectHostileTarget() || !m_creature->GetVictim())
            return;

        DoMeleeAttackIfReady();
    }
};

UnitAI* GetAI_npc_melizza_brimbuzzle(Creature* pCreature)
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
    NPC_CORK_GIZELTON = 11625,
    NPC_RIGGER_GIZELTON = 11626,
    NPC_VENDOR_TRON = 12245,
    NPC_SUPER_SELLER = 12246,

    QUEST_BODYGUARD_TO_HIRE = 5821,
    QUEST_GIZELTON_CARAVAN = 5943,

    WAYPOINT_PATH_ID = 19019,
    PATH_ID_CORK_GIZELTON = 11625,
    PATH_ID_RIGGER_GIZELTON = 11626,
    WAYPOINT_PATH_POINT_AFTER_ESCORT_1 = 33,
    WAYPOINT_PATH_POINT_AFTER_ESCORT_2 = 100,

    GIZELTON_TIMER_AFTER_FAIL = 60 * IN_MILLISECONDS, // forced despawn when one of the member die even if still in combat
};

struct npc_cork_gizeltonAI : public npc_escortAI
{
    npc_cork_gizeltonAI(Creature* pCreature) : npc_escortAI(pCreature) { Reset(); }

    int32 FailTimer = int32(GIZELTON_TIMER_AFTER_FAIL);
    bool StatusFailed = false;
    bool ScriptDone = false;
    bool SouthQuest = false;

    void Reset() override {   }

    void MakeQuestFailAndClean()
    {
        Player* player = GetPlayerForEscort();
        if (!player)
            return;

        // Handle all players in group (if they took quest)
        if (Group* pGroup = player->GetGroup())
        {
            for (GroupReference* pRef = pGroup->GetFirstMember(); pRef != nullptr; pRef = pRef->next())
            {
                if (Player* member = pRef->getSource())
                {
                    if (member->GetQuestStatus(QUEST_BODYGUARD_TO_HIRE) == QUEST_STATUS_INCOMPLETE)
                        member->FailQuest(QUEST_BODYGUARD_TO_HIRE);
                    if (member->GetQuestStatus(QUEST_GIZELTON_CARAVAN) == QUEST_STATUS_INCOMPLETE)
                        member->FailQuest(QUEST_GIZELTON_CARAVAN);
                }
            }
        }
        else
        {
            if (player->GetQuestStatus(QUEST_BODYGUARD_TO_HIRE) == QUEST_STATUS_INCOMPLETE)
                player->FailQuest(QUEST_BODYGUARD_TO_HIRE);
            if (player->GetQuestStatus(QUEST_GIZELTON_CARAVAN) == QUEST_STATUS_INCOMPLETE)
                player->FailQuest(QUEST_GIZELTON_CARAVAN);
        }

        m_creature->GetCreatureGroup()->Despawn();
    }

    void UpdateEscortAI(const uint32 diff) override
    {
        npc_escortAI::UpdateEscortAI(diff);

        if (ScriptDone)
            return;

        if (!StatusFailed)
            return;

        FailTimer -= diff;
        if (FailTimer > 0 && !m_creature->GetCreatureGroup()->IsOutOfCombat())
            return;

        MakeQuestFailAndClean();
        ScriptDone = true;
    }

    void WaypointReached(uint32 pointId) override
    {
        if (!HasEscortState(STATE_ESCORT_ESCORTING))
            return;

        Player* player = GetPlayerForEscort();

        switch (pointId)
        {
            case 19:
                if (!SouthQuest)
                {
                    if (player && player->GetQuestStatus(QUEST_BODYGUARD_TO_HIRE) == QUEST_STATUS_INCOMPLETE)
                    {
                        // Award quest credit
                        player->RewardPlayerAndGroupAtEventExplored(QUEST_BODYGUARD_TO_HIRE, m_creature);
                    }
                    m_creature->GetMotionMaster()->Clear(false, true);
                    m_creature->GetMotionMaster()->MoveWaypoint(WAYPOINT_PATH_ID, PATH_FROM_WAYPOINT_PATH);
                    m_creature->GetMotionMaster()->SetNextWaypoint(WAYPOINT_PATH_POINT_AFTER_ESCORT_1);
                    m_creature->SetWalk(false);
                    End(); // normal movement resumes - caravan doesnt despawn - it loops through zone                    
                   
                }
                break;
                // The second escort quest is also handled by NPC Cork though it is given by NPC Rigger
            case 21:
                if (SouthQuest)
                {
                    if (player && player->GetQuestStatus(QUEST_GIZELTON_CARAVAN) == QUEST_STATUS_INCOMPLETE)
                    {
                        // Award quest credit
                        player->RewardPlayerAndGroupAtEventExplored(QUEST_GIZELTON_CARAVAN, m_creature);
                    }
                    m_creature->GetMotionMaster()->Clear(false, true);
                    m_creature->GetMotionMaster()->MoveWaypoint(WAYPOINT_PATH_ID, PATH_FROM_WAYPOINT_PATH);
                    m_creature->GetMotionMaster()->SetNextWaypoint(WAYPOINT_PATH_POINT_AFTER_ESCORT_2);
                    m_creature->SetWalk(false);
                    End(); // normal movement resumes - caravan doesnt despawn - it loops through zone
                    SouthQuest = false;                                    
                }
                break;
        }
    }

    void StartRiggerEscort(Player* player)
    {
        SouthQuest = true;
        Start(false, player, m_questForEscort, false, false, PATH_ID_RIGGER_GIZELTON);
    }


    void CreatureGroupMemberDied(Unit* /*killed*/) override
    {
        StatusFailed = true;
    }
};

bool QuestAccept_npc_cork_gizelton(Player* player, Creature* creature, const Quest* quest)
{
    if (quest->GetQuestId() == QUEST_BODYGUARD_TO_HIRE)
    {
        if (npc_cork_gizeltonAI* escortAI = dynamic_cast<npc_cork_gizeltonAI*>(creature->AI()))
            escortAI->Start(false, player, quest, false, false, PATH_ID_CORK_GIZELTON); // Note: Escort path use other coords then normal path
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

bool QuestAccept_npc_rigger_gizelton(Player* player, Creature* creature, const Quest* quest)
{
    if (quest->GetQuestId() == QUEST_GIZELTON_CARAVAN)
    {
        // Now the quest is accepted, tell NPC Cork what player took it so it can handle quest credit/failure
        // because NPC Cork will handle both escort quests
        if (Creature* cork = GetClosestCreatureWithEntry(creature, NPC_CORK_GIZELTON, 100.0f))
            if (npc_cork_gizeltonAI* corkAI = dynamic_cast<npc_cork_gizeltonAI*>(cork->AI()))
                corkAI->StartRiggerEscort(player);               // Note: Escort path use other coords then normal path
    }
    return true;
}

enum
{
    SPELL_CURSE_OF_MAGRAMI = 18159,
    SPELL_GHOST_SPAWN_IN   = 17321,
    SPELL_BLUE_AURA        = 17327,
    SPELL_GREEN_AURA       = 18951,
    SPELL_CHILLING_TOUCH   = 18146,

    FACTION_HOSTILE        = 16,

    SAY_EMOTE_1            = -1001226,
    SAY_EMOTE_2            = -1001227,
};

struct npc_magrami_spectre : public ScriptedAI
{
    npc_magrami_spectre(Creature* pCreature) : ScriptedAI(pCreature) { Reset(); }

    uint32 m_uiCurseTimer;

    void Reset() override
    {
        m_uiCurseTimer = 4000;
    }

    void JustRespawned() override
    {
        m_creature->CastSpell(nullptr, SPELL_GHOST_SPAWN_IN, TRIGGERED_NONE);
        m_creature->CastSpell(nullptr, SPELL_BLUE_AURA, TRIGGERED_NONE);
        m_creature->CastSpell(nullptr, SPELL_CHILLING_TOUCH, TRIGGERED_NONE);

        switch (urand(0, 1))
        {
            case 0:
                DoScriptText(SAY_EMOTE_1, m_creature);
                break;
            case 1:
                DoScriptText(SAY_EMOTE_2, m_creature);
                break;
        }
    }

    void MovementInform(uint32 uiMovementType, uint32 uiData) override
    {
        if (uiMovementType == POINT_MOTION_TYPE)
        {
            if (uiData == 1)
            {
                float x, y, z;
                m_creature->GetPosition(x, y, z);
                m_creature->GetMotionMaster()->MoveRandomAroundPoint(x, y, z, 5.f);
                m_creature->RemoveAurasDueToSpell(SPELL_BLUE_AURA);
                m_creature->CastSpell(m_creature, SPELL_GREEN_AURA, TRIGGERED_NONE);
                m_creature->SetFactionTemporary(FACTION_HOSTILE, TEMPFACTION_NONE);
            }
        }
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->GetVictim())
            return;

        if (m_uiCurseTimer)
        {
            if (m_uiCurseTimer <= uiDiff)
            {
                m_uiCurseTimer = 0;
                DoCastSpellIfCan(m_creature->GetVictim(), SPELL_CURSE_OF_MAGRAMI);
            }
            else
                m_uiCurseTimer -= uiDiff;
        }

        DoMeleeAttackIfReady();
    }
};

UnitAI* GetAI_npc_magrami_spectre(Creature* pCreature)
{
    return new npc_magrami_spectre(pCreature);
}

void AddSC_desolace()
{
    Script* pNewScript = new Script;
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
    pNewScript->GetAI = &GetNewAIInstance<npc_cork_gizeltonAI>;
    pNewScript->pQuestAcceptNPC = &QuestAccept_npc_cork_gizelton;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_rigger_gizelton";
    pNewScript->GetAI = &GetNewAIInstance<npc_rigger_gizeltonAI>;
    pNewScript->pQuestAcceptNPC = &QuestAccept_npc_rigger_gizelton;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_magrami_spectre";
    pNewScript->GetAI = &GetAI_npc_magrami_spectre;
    pNewScript->RegisterSelf();
}
