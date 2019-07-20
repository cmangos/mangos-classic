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
SDName: Winterspring
SD%Complete: 100
SDComment: Quest support: 4901.
SDCategory: Winterspring
EndScriptData

*/

#include "AI/ScriptDevAI/include/precompiled.h"/* ContentData
npc_ranshalla
EndContentData */


#include "AI/ScriptDevAI/base/escort_ai.h"

/*######
# npc_ranshalla
####*/

enum
{
    // Escort texts
    SAY_QUEST_START             = -1000739,
    SAY_ENTER_OWL_THICKET       = -1000707,
    SAY_REACH_TORCH_1           = -1000708,
    SAY_REACH_TORCH_2           = -1000709,
    SAY_REACH_TORCH_3           = -1000710,
    SAY_AFTER_TORCH_1           = -1000711,
    SAY_AFTER_TORCH_2           = -1000712,
    SAY_REACH_ALTAR_1           = -1000713,
    SAY_REACH_ALTAR_2           = -1000714,

    // After lighting the altar cinematic
    SAY_RANSHALLA_ALTAR_1       = -1000715,
    SAY_RANSHALLA_ALTAR_2       = -1000716,
    SAY_PRIESTESS_ALTAR_3       = -1000717,
    SAY_PRIESTESS_ALTAR_4       = -1000718,
    SAY_RANSHALLA_ALTAR_5       = -1000719,
    SAY_RANSHALLA_ALTAR_6       = -1000720,
    SAY_PRIESTESS_ALTAR_7       = -1000721,
    SAY_PRIESTESS_ALTAR_8       = -1000722,
    SAY_PRIESTESS_ALTAR_9       = -1000723,
    SAY_PRIESTESS_ALTAR_10      = -1000724,
    SAY_PRIESTESS_ALTAR_11      = -1000725,
    SAY_PRIESTESS_ALTAR_12      = -1000726,
    SAY_PRIESTESS_ALTAR_13      = -1000727,
    SAY_PRIESTESS_ALTAR_14      = -1000728,
    SAY_VOICE_ALTAR_15          = -1000729,
    SAY_PRIESTESS_ALTAR_16      = -1000730,
    SAY_PRIESTESS_ALTAR_17      = -1000731,
    SAY_PRIESTESS_ALTAR_18      = -1000732,
    SAY_PRIESTESS_ALTAR_19      = -1000733,
    SAY_PRIESTESS_ALTAR_20      = -1000734,
    SAY_PRIESTESS_ALTAR_21      = -1000735,
    SAY_QUEST_END_1             = -1000736,
    SAY_QUEST_END_2             = -1000737,

    EMOTE_CHANT_SPELL           = -1000738,

    SPELL_LIGHT_TORCH           = 18953,        // channeled spell by Ranshalla while waiting for the torches / altar
    SPELL_RANSHALLA_DESPAWN     = 18954,
    SPELL_BIND_WILDKIN          = 18994,

    NPC_RANSHALLA               = 10300,
    NPC_PRIESTESS_ELUNE         = 12116,
    NPC_VOICE_ELUNE             = 12152,
    NPC_GUARDIAN_ELUNE          = 12140,

    NPC_PRIESTESS_DATA_1        = 1,            // dummy member for the first priestess (right)
    NPC_PRIESTESS_DATA_2        = 2,            // dummy member for the second priestess (left)
    DATA_MOVE_PRIESTESS         = 3,            // dummy member to check the priestess movement
    DATA_EVENT_END              = 4,            // dummy member to indicate the event end

    GO_ELUNE_ALTAR              = 177404,
    GO_ELUNE_FIRE               = 177417,
    GO_ELUNE_GEM                = 177414,       // is respawned in script
    GO_ELUNE_LIGHT              = 177415,       // are respawned in script
    GO_ELUNE_AURA               = 177416,       // is respawned in script

    QUEST_GUARDIANS_ALTAR       = 4901,
};

static const DialogueEntry aIntroDialogue[] =
{
    {SAY_REACH_ALTAR_1,      NPC_RANSHALLA,        2000},
    {SAY_REACH_ALTAR_2,      NPC_RANSHALLA,        3000},
    {NPC_RANSHALLA,          0,                    0},      // start the altar channeling
    {SAY_PRIESTESS_ALTAR_3,  NPC_PRIESTESS_DATA_2, 1000},
    {SAY_PRIESTESS_ALTAR_4,  NPC_PRIESTESS_DATA_1, 4000},
    {SAY_RANSHALLA_ALTAR_5,  NPC_RANSHALLA,        4000},
    {SAY_RANSHALLA_ALTAR_6,  NPC_RANSHALLA,        4000},   // start the escort here
    {SAY_PRIESTESS_ALTAR_7,  NPC_PRIESTESS_DATA_2, 4000},
    {SAY_PRIESTESS_ALTAR_8,  NPC_PRIESTESS_DATA_2, 5000},   // show the gem
    {GO_ELUNE_GEM,           0,                    5000},
    {SAY_PRIESTESS_ALTAR_9,  NPC_PRIESTESS_DATA_1, 4000},   // move priestess 1 near m_creature
    {NPC_PRIESTESS_DATA_1,   0,                    3000},
    {SAY_PRIESTESS_ALTAR_10, NPC_PRIESTESS_DATA_1, 5000},
    {SAY_PRIESTESS_ALTAR_11, NPC_PRIESTESS_DATA_1, 4000},
    {SAY_PRIESTESS_ALTAR_12, NPC_PRIESTESS_DATA_1, 5000},
    {SAY_PRIESTESS_ALTAR_13, NPC_PRIESTESS_DATA_1, 8000},   // summon voice and guard of elune
    {NPC_VOICE_ELUNE,        0,                    12000},
    {SAY_VOICE_ALTAR_15,     NPC_VOICE_ELUNE,      5000},   // move priestess 2 near m_creature
    {NPC_PRIESTESS_DATA_2,   0,                    3000},
    {SAY_PRIESTESS_ALTAR_16, NPC_PRIESTESS_DATA_2, 4000},
    {SAY_PRIESTESS_ALTAR_17, NPC_PRIESTESS_DATA_2, 6000},
    {SAY_PRIESTESS_ALTAR_18, NPC_PRIESTESS_DATA_1, 5000},
    {SAY_PRIESTESS_ALTAR_19, NPC_PRIESTESS_DATA_1, 3000},   // move the owlbeast
    {NPC_GUARDIAN_ELUNE,     0,                    2000},
    {SAY_PRIESTESS_ALTAR_20, NPC_PRIESTESS_DATA_1, 4000},   // move the first priestess up
    {SAY_PRIESTESS_ALTAR_21, NPC_PRIESTESS_DATA_2, 10000},  // move second priestess up
    {DATA_MOVE_PRIESTESS,    0,                    6000},   // despawn the gem
    {DATA_EVENT_END,         0,                    2000},   // turn towards the player
    {SAY_QUEST_END_2,        NPC_RANSHALLA,        0},
    {0, 0, 0},
};

struct EventLocations
{
    float m_fX, m_fY, m_fZ, m_fO;
};

static EventLocations aWingThicketLocations[] =
{
    {5515.98f, -4903.43f, 846.30f, 4.58f},      // 0 right priestess summon loc
    {5501.94f, -4920.20f, 848.69f, 6.15f},      // 1 left priestess summon loc
    {5497.35f, -4906.49f, 850.83f, 2.76f},      // 2 guard of elune summon loc
    {5518.38f, -4913.47f, 845.57f},             // 3 right priestess move loc
    {5510.36f, -4921.17f, 846.33f},             // 4 left priestess move loc
    {5511.31f, -4913.82f, 847.17f},             // 5 guard of elune move loc
    {5518.51f, -4917.56f, 845.23f},             // 6 right priestess second move loc
    {5514.40f, -4921.16f, 845.49f}              // 7 left priestess second move loc
};

struct npc_ranshallaAI : public npc_escortAI, private DialogueHelper
{
    npc_ranshallaAI(Creature* pCreature) : npc_escortAI(pCreature),
        DialogueHelper(aIntroDialogue)
    {
        Reset();
    }

    uint32 m_uiDelayTimer;
    uint32 m_uiCurrentWaypoint;

    ObjectGuid m_firstPriestessGuid;
    ObjectGuid m_secondPriestessGuid;
    ObjectGuid m_guardEluneGuid;
    ObjectGuid m_voiceEluneGuid;
    ObjectGuid m_altarGuid;

    void Reset() override
    {
        m_uiDelayTimer = 0;
        m_uiCurrentWaypoint = 0;
    }

    // Called when the player activates the torch / altar
    void DoContinueEscort(bool bIsAltarWaypoint = false)
    {
        if (bIsAltarWaypoint)
            DoScriptText(SAY_RANSHALLA_ALTAR_1, m_creature);
        else
        {
            switch (urand(0, 1))
            {
                case 0: DoScriptText(SAY_AFTER_TORCH_1, m_creature); break;
                case 1: DoScriptText(SAY_AFTER_TORCH_2, m_creature); break;
            }
        }

        m_uiDelayTimer = 2000;
    }

    void SpellHit(Unit* /*pCaster*/, const SpellEntry* pSpell) override
    {
        // If Ranshalla cast the torch lightening spell for too long she will trigger SPELL_RANSHALLA_DESPAWN (quest failed and despawn)
        if (pSpell->Id == SPELL_RANSHALLA_DESPAWN)
        {
            FailQuestForPlayerAndGroup();
            m_creature->ForcedDespawn();
        }
    }

    // Called when Ranshalla starts to channel on a torch / altar
    void DoChannelTorchSpell(bool bIsAltarWaypoint = false)
    {
        // Check if we are using the fire or the altar and remove the no_interact flag
        if (bIsAltarWaypoint)
        {
            if (GameObject* pGo = GetClosestGameObjectWithEntry(m_creature, GO_ELUNE_ALTAR, 10.0f))
            {
                pGo->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_NO_INTERACT);
                m_creature->SetFacingToObject(pGo);
                m_altarGuid = pGo->GetObjectGuid();
            }
        }
        else
        {
            if (GameObject* pGo = GetClosestGameObjectWithEntry(m_creature, GO_ELUNE_FIRE, 10.0f))
                pGo->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_NO_INTERACT);
        }

        // Yell and set escort to pause
        switch (urand(0, 2))
        {
            case 0: DoScriptText(SAY_REACH_TORCH_1, m_creature); break;
            case 1: DoScriptText(SAY_REACH_TORCH_2, m_creature); break;
            case 2: DoScriptText(SAY_REACH_TORCH_3, m_creature); break;
        }

        DoScriptText(EMOTE_CHANT_SPELL, m_creature);
        DoCastSpellIfCan(m_creature, SPELL_LIGHT_TORCH);
        SetEscortPaused(true);
    }

    void DoSummonPriestess()
    {
        // Summon 2 Elune priestess and make each of them move to a different spot
        if (Creature* pPriestess = m_creature->SummonCreature(NPC_PRIESTESS_ELUNE, aWingThicketLocations[0].m_fX, aWingThicketLocations[0].m_fY, aWingThicketLocations[0].m_fZ, aWingThicketLocations[0].m_fO, TEMPSPAWN_CORPSE_DESPAWN, 0))
        {
            pPriestess->GetMotionMaster()->MovePoint(0, aWingThicketLocations[3].m_fX, aWingThicketLocations[3].m_fY, aWingThicketLocations[3].m_fZ);
            m_firstPriestessGuid = pPriestess->GetObjectGuid();
        }
        if (Creature* pPriestess = m_creature->SummonCreature(NPC_PRIESTESS_ELUNE, aWingThicketLocations[1].m_fX, aWingThicketLocations[1].m_fY, aWingThicketLocations[1].m_fZ, aWingThicketLocations[1].m_fO, TEMPSPAWN_CORPSE_DESPAWN, 0))
        {
            // Left priestess should have a distinct move point because she is the one who starts the dialogue at point reach
            pPriestess->GetMotionMaster()->MovePoint(1, aWingThicketLocations[4].m_fX, aWingThicketLocations[4].m_fY, aWingThicketLocations[4].m_fZ);
            m_secondPriestessGuid = pPriestess->GetObjectGuid();
        }
    }

    void SummonedMovementInform(Creature* pSummoned, uint32 uiType, uint32 uiPointId) override
    {
        if (uiType != POINT_MOTION_TYPE || pSummoned->GetEntry() != NPC_PRIESTESS_ELUNE || uiPointId != 1)
            return;

        // Start the dialogue when the priestess reach the altar (they should both reach the point in the same time)
        StartNextDialogueText(SAY_PRIESTESS_ALTAR_3);
    }

    void WaypointReached(uint32 uiPointId) override
    {
        switch (uiPointId)
        {
            case 3:
                DoScriptText(SAY_ENTER_OWL_THICKET, m_creature);
                break;
            case 10: // Cavern 1
            case 15: // Cavern 2
            case 20: // Cavern 3
            case 25: // Cavern 4
            case 36: // Cavern 5
                m_uiCurrentWaypoint = uiPointId;
                DoChannelTorchSpell();
                break;
            case 39:
                m_uiCurrentWaypoint = uiPointId;
                StartNextDialogueText(SAY_REACH_ALTAR_1);
                SetEscortPaused(true);
                break;
            case 41:
            {
                // Search for all nearest lights and respawn them
                GameObjectList m_lEluneLights;
                GetGameObjectListWithEntryInGrid(m_lEluneLights, m_creature, GO_ELUNE_LIGHT, 20.0f);
                for (GameObjectList::const_iterator itr = m_lEluneLights.begin(); itr != m_lEluneLights.end(); ++itr)
                {
                    if ((*itr)->IsSpawned())
                        continue;

                    (*itr)->SetRespawnTime(115);
                    (*itr)->Refresh();
                }

                if (GameObject* pAltar = m_creature->GetMap()->GetGameObject(m_altarGuid))
                    m_creature->SetFacingToObject(pAltar);
                break;
            }
            case 42:
                // Summon the 2 priestess
                SetEscortPaused(true);
                DoSummonPriestess();
                DoScriptText(SAY_RANSHALLA_ALTAR_2, m_creature);
                break;
            case 44:
                // Stop the escort and turn towards the altar
                SetEscortPaused(true);
                if (GameObject* pAltar = m_creature->GetMap()->GetGameObject(m_altarGuid))
                    m_creature->SetFacingToObject(pAltar);
                break;
        }
    }

    void JustDidDialogueStep(int32 iEntry) override
    {
        switch (iEntry)
        {
            case NPC_RANSHALLA:
                // Start the altar channeling
                DoChannelTorchSpell(true);
                break;
            case SAY_RANSHALLA_ALTAR_6:
                SetEscortPaused(false);
                break;
            case SAY_PRIESTESS_ALTAR_8:
                // make the gem and its aura respawn
                if (GameObject* pGem = GetClosestGameObjectWithEntry(m_creature, GO_ELUNE_GEM, 10.0f))
                {
                    if (pGem->IsSpawned())
                        break;

                    pGem->SetRespawnTime(90);
                    pGem->Refresh();
                }
                if (GameObject* pAura = GetClosestGameObjectWithEntry(m_creature, GO_ELUNE_AURA, 10.0f))
                {
                    if (pAura->IsSpawned())
                        break;

                    pAura->SetRespawnTime(90);
                    pAura->Refresh();
                }
                break;
            case SAY_PRIESTESS_ALTAR_9:
                // move near the escort npc
                if (Creature* pPriestess = m_creature->GetMap()->GetCreature(m_firstPriestessGuid))
                    pPriestess->GetMotionMaster()->MovePoint(0, aWingThicketLocations[6].m_fX, aWingThicketLocations[6].m_fY, aWingThicketLocations[6].m_fZ);
                break;
            case SAY_PRIESTESS_ALTAR_13:
                // summon the Guardian of Elune
                if (Creature* pGuard = m_creature->SummonCreature(NPC_GUARDIAN_ELUNE, aWingThicketLocations[2].m_fX, aWingThicketLocations[2].m_fY, aWingThicketLocations[2].m_fZ, aWingThicketLocations[2].m_fO, TEMPSPAWN_CORPSE_DESPAWN, 0))
                {
                    pGuard->GetMotionMaster()->MovePoint(0, aWingThicketLocations[5].m_fX, aWingThicketLocations[5].m_fY, aWingThicketLocations[5].m_fZ);
                    m_guardEluneGuid = pGuard->GetObjectGuid();
                }
                // summon the Voice of Elune
                if (GameObject* pAltar = m_creature->GetMap()->GetGameObject(m_altarGuid))
                {
                    if (Creature* pVoice = m_creature->SummonCreature(NPC_VOICE_ELUNE, pAltar->GetPositionX(), pAltar->GetPositionY(), pAltar->GetPositionZ(), 0, TEMPSPAWN_TIMED_DESPAWN, 30000))
                        m_voiceEluneGuid = pVoice->GetObjectGuid();
                }
                break;
            case SAY_VOICE_ALTAR_15:
                // move near the escort npc and continue dialogue
                if (Creature* pPriestess = m_creature->GetMap()->GetCreature(m_secondPriestessGuid))
                {
                    DoScriptText(SAY_PRIESTESS_ALTAR_14, pPriestess);
                    pPriestess->GetMotionMaster()->MovePoint(0, aWingThicketLocations[7].m_fX, aWingThicketLocations[7].m_fY, aWingThicketLocations[7].m_fZ);
                }
                // make the voice of elune cast Bind Wildkin
                if (Creature* pGuard = m_creature->GetMap()->GetCreature(m_guardEluneGuid))
                    pGuard->CastSpell(pGuard, SPELL_BIND_WILDKIN, TRIGGERED_NONE);
                break;
            case SAY_PRIESTESS_ALTAR_19:
                // make the voice of elune leave
                if (Creature* pGuard = m_creature->GetMap()->GetCreature(m_guardEluneGuid))
                {
                    pGuard->GetMotionMaster()->MovePoint(0, aWingThicketLocations[2].m_fX, aWingThicketLocations[2].m_fY, aWingThicketLocations[2].m_fZ);
                    pGuard->ForcedDespawn(4000);
                }
                break;
            case SAY_PRIESTESS_ALTAR_20:
                // make the first priestess leave
                if (Creature* pPriestess = m_creature->GetMap()->GetCreature(m_firstPriestessGuid))
                {
                    pPriestess->GetMotionMaster()->MovePoint(0, aWingThicketLocations[0].m_fX, aWingThicketLocations[0].m_fY, aWingThicketLocations[0].m_fZ);
                    pPriestess->ForcedDespawn(4000);
                }
                break;
            case SAY_PRIESTESS_ALTAR_21:
                // make the second priestess leave
                if (Creature* pPriestess = m_creature->GetMap()->GetCreature(m_secondPriestessGuid))
                {
                    pPriestess->GetMotionMaster()->MovePoint(0, aWingThicketLocations[1].m_fX, aWingThicketLocations[1].m_fY, aWingThicketLocations[1].m_fZ);
                    pPriestess->ForcedDespawn(4000);
                }
                break;
            case DATA_EVENT_END:
                // Turn towards the player
                if (Player* pPlayer = GetPlayerForEscort())
                {
                    m_creature->SetFacingToObject(pPlayer);
                    DoScriptText(SAY_QUEST_END_1, m_creature, pPlayer);
                }
                break;
            case SAY_QUEST_END_2:
                // Turn towards the altar and kneel - quest complete
                if (GameObject* pAltar = m_creature->GetMap()->GetGameObject(m_altarGuid))
                    m_creature->SetFacingToObject(pAltar);
                m_creature->SetStandState(UNIT_STAND_STATE_KNEEL);
                if (Player* pPlayer = GetPlayerForEscort())
                    pPlayer->RewardPlayerAndGroupAtEventExplored(QUEST_GUARDIANS_ALTAR, m_creature);
                m_creature->ForcedDespawn(1 * MINUTE * IN_MILLISECONDS);
                break;
        }
    }

    Creature* GetSpeakerByEntry(uint32 uiEntry) override
    {
        switch (uiEntry)
        {
            case NPC_RANSHALLA:        return m_creature;
            case NPC_VOICE_ELUNE:      return m_creature->GetMap()->GetCreature(m_voiceEluneGuid);
            case NPC_PRIESTESS_DATA_1: return m_creature->GetMap()->GetCreature(m_firstPriestessGuid);
            case NPC_PRIESTESS_DATA_2: return m_creature->GetMap()->GetCreature(m_secondPriestessGuid);

            default:
                return nullptr;
        }
    }

    void UpdateEscortAI(const uint32 uiDiff) override
    {
        DialogueUpdate(uiDiff);

        if (m_uiDelayTimer)
        {
            if (m_uiDelayTimer <= uiDiff)
            {
                m_creature->InterruptNonMeleeSpells(false);
                SetEscortPaused(false);
                // Spell Ranshalla Waiting applies a root aura that triggers an internal timer in the waypoint movement manager
                // We need to manually set the movement manager to the next waypoint in order to reset the timer unless the NPC will wait 3 min before moving again
                m_creature->GetMotionMaster()->SetNextWaypoint(m_uiCurrentWaypoint + 1);
                m_uiDelayTimer = 0;
            }
            else
                m_uiDelayTimer -= uiDiff;
        }

        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        DoMeleeAttackIfReady();
    }
};

UnitAI* GetAI_npc_ranshalla(Creature* pCreature)
{
    return new npc_ranshallaAI(pCreature);
}

bool QuestAccept_npc_ranshalla(Player* pPlayer, Creature* pCreature, const Quest* pQuest)
{
    if (pQuest->GetQuestId() == QUEST_GUARDIANS_ALTAR)
    {
        DoScriptText(SAY_QUEST_START, pCreature);
        pCreature->SetFactionTemporary(FACTION_ESCORT_A_NEUTRAL_PASSIVE, TEMPFACTION_RESTORE_RESPAWN);

        if (npc_ranshallaAI* pEscortAI = dynamic_cast<npc_ranshallaAI*>(pCreature->AI()))
            pEscortAI->Start(false, pPlayer, pQuest, true);

        return true;
    }

    return false;
}

bool GOUse_go_elune_fire(Player* /*pPlayer*/, GameObject* pGo)
{
    // Check if we are using the torches or the altar
    bool bIsAltar = false;

    if (pGo->GetEntry() == GO_ELUNE_ALTAR)
        bIsAltar = true;

    if (Creature* pRanshalla = GetClosestCreatureWithEntry(pGo, NPC_RANSHALLA, 10.0f))
    {
        if (npc_ranshallaAI* pEscortAI = dynamic_cast<npc_ranshallaAI*>(pRanshalla->AI()))
            pEscortAI->DoContinueEscort(bIsAltar);
    }

    return false;
}

enum
{
    SPELL_FOOLS_PLIGHT              = 23504,

    SPELL_DEMONIC_FRENZY            = 23257,
    SPELL_DEMONIC_DOOM              = 23298,
    SPELL_STINGING_TRAUMA           = 23299,

    EMOTE_POISON                    = -1001251,

    NPC_ARTORIUS_THE_AMIABLE        = 14531,
    NPC_ARTORIUS_THE_DOOMBRINGER    = 14535,
    NPC_THE_CLEANER                 = 14503,

    QUEST_STAVE_OF_THE_ANCIENTS     = 7636
};

#define GOSSIP_ITEM                 "Show me your real face, demon."

/*######
## npc_artorius_the_amiable
######*/

/*######
## npc_artorius_the_doombringer
######*/

struct npc_artoriusAI : public ScriptedAI
{
    npc_artoriusAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_bTransform = false;
        m_uiDespawn_Timer = 0;
        Reset();
    }

    uint32 m_uiTransform_Timer;
    uint32 m_uiTransformEmote_Timer;
    bool m_bTransform;

    ObjectGuid m_hunterGuid;
    uint32 m_uiDemonic_Doom_Timer;
    uint32 m_uiDemonic_Frenzy_Timer;
    uint32 m_uiDespawn_Timer;

    void Reset() override
    {
        switch (m_creature->GetEntry())
        {
            case NPC_ARTORIUS_THE_AMIABLE:
                m_creature->SetRespawnDelay(35 * MINUTE);
                m_creature->SetRespawnTime(35 * MINUTE);
                m_creature->NearTeleportTo(7909.71f, -4598.67f, 710.008f, 0.606013f);
                if (m_creature->GetMotionMaster()->GetCurrentMovementGeneratorType() != WAYPOINT_MOTION_TYPE)
                {
                    m_creature->SetDefaultMovementType(WAYPOINT_MOTION_TYPE);
                    m_creature->GetMotionMaster()->Initialize();
                }

                m_creature->SetUInt32Value(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);

                m_uiTransform_Timer = 10000;
                m_uiTransformEmote_Timer = 5000;
                m_bTransform = false;
                m_uiDespawn_Timer = 0;
                break;
            case NPC_ARTORIUS_THE_DOOMBRINGER:
                if (!m_uiDespawn_Timer)
                    m_uiDespawn_Timer = 20 * MINUTE*IN_MILLISECONDS;

                m_hunterGuid.Clear();
                m_uiDemonic_Doom_Timer = 7500;
                m_uiDemonic_Frenzy_Timer = urand(5000, 8000);
                break;
        }
    }

    /** Artorius the Amiable */
    void Transform()
    {
        m_creature->UpdateEntry(NPC_ARTORIUS_THE_DOOMBRINGER);
        Reset();
    }

    void BeginEvent(ObjectGuid playerGuid)
    {
        m_hunterGuid = playerGuid;
        m_creature->GetMotionMaster()->Clear(false);
        m_creature->GetMotionMaster()->MoveIdle();
        m_creature->SetUInt32Value(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_NONE);
        m_bTransform = true;
    }

    /** Artorius the Doombringer */
    void Aggro(Unit* pWho) override
    {
        if (pWho->getClass() == CLASS_HUNTER && (m_hunterGuid.IsEmpty() || m_hunterGuid == pWho->GetObjectGuid()))
        {
            m_hunterGuid = pWho->GetObjectGuid();
        }
        else
            DemonDespawn();
    }

    void JustDied(Unit* /*pKiller*/) override
    {
        uint32 m_respawn_delay_Timer = 2 * HOUR;
        m_creature->SetRespawnDelay(m_respawn_delay_Timer);
        m_creature->SetRespawnTime(m_respawn_delay_Timer);
        m_creature->SaveRespawnTime();
    }

    void DemonDespawn(bool triggered = true)
    {
        m_creature->SetRespawnDelay(15 * MINUTE);
        m_creature->SetRespawnTime(15 * MINUTE);
        m_creature->SaveRespawnTime();

        if (triggered)
        {
            Creature* pCleaner = m_creature->SummonCreature(NPC_THE_CLEANER, m_creature->GetPositionX(), m_creature->GetPositionY(), m_creature->GetPositionZ(), m_creature->GetAngle(m_creature), TEMPSPAWN_DEAD_DESPAWN, 20 * MINUTE*IN_MILLISECONDS);
            if (pCleaner)
            {
                ThreatList const& tList = m_creature->getThreatManager().getThreatList();

                for (auto itr : tList)
                {
                    if (Unit* pUnit = m_creature->GetMap()->GetUnit(itr->getUnitGuid()))
                    {
                        if (pUnit->isAlive())
                        {
                            pCleaner->SetInCombatWith(pUnit);
                            pCleaner->AddThreat(pUnit);
                            pCleaner->AI()->AttackStart(pUnit);
                        }
                    }
                }
            }
        }

        m_creature->ForcedDespawn();
    }

    void SpellHit(Unit* /*pCaster*/, const SpellEntry* pSpell) override
    {
        if (pSpell->Id == 13555 || pSpell->Id == 25295)             // Serpent Sting (Rank 8 or Rank 9)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_STINGING_TRAUMA, CAST_TRIGGERED) == CAST_OK)
                DoScriptText(EMOTE_POISON, m_creature);
        }
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        /** Artorius the Amiable */
        if (m_bTransform)
        {
            if (m_uiTransformEmote_Timer)
            {
                if (m_uiTransformEmote_Timer <= uiDiff)
                {
                    m_creature->HandleEmote(EMOTE_ONESHOT_ROAR);
                    m_uiTransformEmote_Timer = 0;
                }
                else
                    m_uiTransformEmote_Timer -= uiDiff;
            }

            if (m_uiTransform_Timer < uiDiff)
            {
                m_bTransform = false;
                Transform();
            }
            else
                m_uiTransform_Timer -= uiDiff;
        }

        /** Artorius the Doombringer */
        if (m_uiDespawn_Timer)
        {
            if (m_uiDespawn_Timer <= uiDiff)
            {
                if (m_creature->isAlive() && !m_creature->isInCombat())
                    DemonDespawn(false);
            }
            else
                m_uiDespawn_Timer -= uiDiff;
        }

        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        if (m_creature->getThreatManager().getThreatList().size() > 1)
            DemonDespawn();

        if (m_uiDemonic_Frenzy_Timer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_DEMONIC_FRENZY) == CAST_OK)
                m_uiDemonic_Frenzy_Timer = urand(15000, 20000);
        }
        else
            m_uiDemonic_Frenzy_Timer -= uiDiff;

        if (m_uiDemonic_Doom_Timer < uiDiff)
        {
            m_uiDemonic_Doom_Timer = 7500;
            // only attempt to cast this once every 7.5 seconds to give the hunter some leeway
            // LOWER max range for lag...
            if (m_creature->IsWithinDistInMap(m_creature->getVictim(), 25))
                DoCastSpellIfCan(m_creature->getVictim(), SPELL_DEMONIC_DOOM);
        }
        else
            m_uiDemonic_Doom_Timer -= uiDiff;

        DoMeleeAttackIfReady();
    }
};

bool GossipHello_npc_artorius(Player* pPlayer, Creature* pCreature)
{
    if (pPlayer->GetQuestStatus(QUEST_STAVE_OF_THE_ANCIENTS) == QUEST_STATUS_INCOMPLETE)
        pPlayer->ADD_GOSSIP_ITEM(0, GOSSIP_ITEM, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);

    pPlayer->SEND_GOSSIP_MENU(pPlayer->GetGossipTextId(pCreature), pCreature->GetObjectGuid());
    return true;
}

bool GossipSelect_npc_artorius(Player* pPlayer, Creature* pCreature, uint32 /*uiSender*/, uint32 /*uiAction*/)
{
    pPlayer->CLOSE_GOSSIP_MENU();
    ((npc_artoriusAI*)pCreature->AI())->BeginEvent(pPlayer->GetObjectGuid());
    return true;
}

UnitAI* GetAI_npc_artorius(Creature* pCreature)
{
    return new npc_artoriusAI(pCreature);
}

void AddSC_winterspring()
{
    Script* pNewScript = new Script;
    pNewScript->Name = "npc_ranshalla";
    pNewScript->GetAI = &GetAI_npc_ranshalla;
    pNewScript->pQuestAcceptNPC = &QuestAccept_npc_ranshalla;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "go_elune_fire";
    pNewScript->pGOUse = &GOUse_go_elune_fire;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_artorius";
    pNewScript->GetAI = &GetAI_npc_artorius;
    pNewScript->pGossipHello = &GossipHello_npc_artorius;
    pNewScript->pGossipSelect = &GossipSelect_npc_artorius;
    pNewScript->RegisterSelf();
}
