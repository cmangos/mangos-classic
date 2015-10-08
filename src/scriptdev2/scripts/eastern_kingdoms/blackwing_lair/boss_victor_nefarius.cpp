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
SDName: Boss_Victor_Nefarius
SD%Complete: 90
SDComment: Small adjustments needed; Timers
SDCategory: Blackwing Lair
EndScriptData */

#include "precompiled.h"
#include "blackwing_lair.h"

enum
{
    SAY_GAMESBEGIN_1                = -1469004,
    SAY_GAMESBEGIN_2                = -1469005,
    SAY_NEFARIAN_INTRO              = -1469007,

    GOSSIP_ITEM_NEFARIUS_1          = -3469000,
    GOSSIP_ITEM_NEFARIUS_2          = -3469001,
    GOSSIP_ITEM_NEFARIUS_3          = -3469002,
    GOSSIP_TEXT_NEFARIUS_1          = 7134,
    GOSSIP_TEXT_NEFARIUS_2          = 7198,
    GOSSIP_TEXT_NEFARIUS_3          = 7199,

    MAX_DRAKES                      = 5,
    MAX_DRAKE_SUMMONS               = 42,
    NPC_BRONZE_DRAKANOID            = 14263,
    NPC_BLUE_DRAKANOID              = 14261,
    NPC_RED_DRAKANOID               = 14264,
    NPC_GREEN_DRAKANOID             = 14262,
    NPC_BLACK_DRAKANOID             = 14265,
    NPC_CHROMATIC_DRAKANOID         = 14302,

    SPELL_NEFARIUS_BARRIER          = 22663,                // immunity in phase 1
    SPELL_SHADOWBLINK_INTRO         = 22664,
    SPELL_SHADOWBOLT_VOLLEY         = 22665,
    SPELL_SILENCE                   = 22666,
    SPELL_SHADOW_COMMAND            = 22667,
    SPELL_SHADOWBOLT                = 22677,
    SPELL_FEAR                      = 22678,
    SPELL_SHADOWBLINK               = 22681,                // triggers a random from spells (22668 - 22676)

    SPELL_SUMMON_DRAKONID_BONES     = 23363,

    MAP_ID_BWL                      = 469,

    FACTION_BLACK_DRAGON            = 103
};

static const DialogueEntry aIntroDialogue[] =
{
    {SAY_GAMESBEGIN_1,    NPC_LORD_VICTOR_NEFARIUS,     4000},
    {SAY_GAMESBEGIN_2,    NPC_LORD_VICTOR_NEFARIUS,     5000},
    {SPELL_SHADOWBLINK,   0,                            0},
    {0, 0, 0},
};

struct SpawnLocation
{
    float m_fX, m_fY, m_fZ;
};

static const SpawnLocation aNefarianLocs[4] =
{
    { -7599.32f, -1191.72f, 475.545f},                      // opening where red/blue/black darknid spawner appear (ori 3.05433)
    { -7526.27f, -1135.04f, 473.445f},                      // same as above, closest to door (ori 5.75959)
    { -7498.177f, -1273.277f, 481.649f},                    // nefarian spawn location (ori 1.798)
    { -7502.002f, -1256.503f, 476.758f},                    // nefarian fly to this position
};

static const uint32 aPossibleDrake[MAX_DRAKES] = {NPC_BRONZE_DRAKANOID, NPC_BLUE_DRAKANOID, NPC_RED_DRAKANOID, NPC_GREEN_DRAKANOID, NPC_BLACK_DRAKANOID};

// This script is complicated
// Instead of morphing Victor Nefarius we will have him control phase 1
// And then have him spawn "Nefarian" for phase 2
// When phase 2 starts Victor Nefarius will go invisible and stop attacking
// If Nefarian reched home because nef killed the players then nef will trigger this guy to EnterEvadeMode
// and allow players to start the event over
// If nefarian dies then he will kill himself then he will be despawned in Nefarian script
// To prevent players from doing the event twice

// Dev note: Lord Victor Nefarius should despawn completely, then ~5 seconds later Nefarian should appear.

struct boss_victor_nefariusAI : public ScriptedAI, private DialogueHelper
{
    boss_victor_nefariusAI(Creature* pCreature) : ScriptedAI(pCreature),
        DialogueHelper(aIntroDialogue)
    {
        // Select the 2 different drakes that we are going to use until despawned
        // 5 possiblities for the first drake, 4 for the second, 20 total possiblites

        // select two different numbers between 0..MAX_DRAKES-1
        uint8 uiPos1 = urand(0, MAX_DRAKES - 1);
        uint8 uiPos2 = (uiPos1 + urand(1, MAX_DRAKES - 1)) % MAX_DRAKES;

        m_uiDrakeTypeOne = aPossibleDrake[uiPos1];
        m_uiDrakeTypeTwo = aPossibleDrake[uiPos2];

        m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
        InitializeDialogueHelper(m_pInstance);
        Reset();
    }

    ScriptedInstance* m_pInstance;

    uint32 m_uiSpawnedAdds;
    uint32 m_uiAddSpawnTimer;
    uint32 m_uiShadowBoltTimer;
    uint32 m_uiFearTimer;
    uint32 m_uiDrakeTypeOne;
    uint32 m_uiDrakeTypeTwo;
    uint32 m_uiShadowboltVolleyTimer;
    uint32 m_uiSilenceTimer;
    uint32 m_uiShadowCommandTimer;
    uint32 m_uiShadowBlinkTimer;

    void Reset() override
    {
        // Check the map id because the same creature entry is involved in other scripted event in other instance
        if (m_creature->GetMapId() != MAP_ID_BWL)
            return;

        m_uiSpawnedAdds             = 0;
        m_uiAddSpawnTimer           = 10000;
        m_uiShadowBoltTimer         = 3000;
        m_uiFearTimer               = 8000;
        m_uiShadowboltVolleyTimer   = 13000;
        m_uiSilenceTimer            = 23000;
        m_uiShadowCommandTimer      = 30000;
        m_uiShadowBlinkTimer        = 40000;

        // set gossip flag to begin the event
        m_creature->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);

        // Make visible if needed
        if (m_creature->GetVisibility() != VISIBILITY_ON)
            m_creature->SetVisibility(VISIBILITY_ON);
    }

    void Aggro(Unit* /*pWho*/) override
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_NEFARIAN, IN_PROGRESS);
    }

    void JustReachedHome() override
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_NEFARIAN, FAIL);
    }

    void AttackStart(Unit* pWho) override
    {
        if (m_creature->Attack(pWho, false))
        {
            m_creature->AddThreat(pWho);
            m_creature->SetInCombatWith(pWho);
            pWho->SetInCombatWith(m_creature);

            // Only range attack - ToDo: research the distance
            m_creature->GetMotionMaster()->MoveChase(pWho, 30.0f);
        }
    }

    void JustSummoned(Creature* pSummoned) override
    {
        if (m_creature->GetMapId() != MAP_ID_BWL)
            return;

        if (pSummoned->GetEntry() == NPC_NEFARIAN)
        {
            pSummoned->SetWalk(false);

            // see boss_onyxia (also note the removal of this in boss_nefarian)
            pSummoned->SetByteValue(UNIT_FIELD_BYTES_1, 3, UNIT_BYTE1_FLAG_ALWAYS_STAND);
            pSummoned->SetLevitate(true);

            // Let Nefarian fly towards combat area
            pSummoned->GetMotionMaster()->MovePoint(1, aNefarianLocs[3].m_fX, aNefarianLocs[3].m_fY, aNefarianLocs[3].m_fZ);
            DoScriptText(SAY_NEFARIAN_INTRO, pSummoned);
        }
        else
        {
            ++m_uiSpawnedAdds;

            if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
                pSummoned->AI()->AttackStart(pTarget);
        }
    }

    void SummonedMovementInform(Creature* pSummoned, uint32 uiMotionType, uint32 uiPointId) override
    {
        if (m_creature->GetMapId() != MAP_ID_BWL)
            return;

        // If Nefarian has reached combat area, let him attack
        if (pSummoned->GetEntry() == NPC_NEFARIAN && uiMotionType == POINT_MOTION_TYPE && uiPointId == 1)
        {
            if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
                pSummoned->AI()->AttackStart(pTarget);
        }
    }

    void SummonedCreatureJustDied(Creature* pSummoned) override
    {
        if (m_creature->GetMapId() != MAP_ID_BWL)
            return;

        // Despawn self when Nefarian is killed
        if (pSummoned->GetEntry() == NPC_NEFARIAN)
            m_creature->ForcedDespawn();
        else
            pSummoned->CastSpell(pSummoned, SPELL_SUMMON_DRAKONID_BONES, true);
    }

    void JustDidDialogueStep(int32 iEntry) override
    {
        // Start combat after the dialogue is finished
        if (iEntry == SPELL_SHADOWBLINK)
        {
            m_creature->SetStandState(UNIT_STAND_STATE_STAND);
            m_creature->SetFactionTemporary(FACTION_BLACK_DRAGON, TEMPFACTION_RESTORE_REACH_HOME);
            DoCastSpellIfCan(m_creature, SPELL_NEFARIUS_BARRIER, CAST_TRIGGERED);
            DoCastSpellIfCan(m_creature, SPELL_SHADOWBLINK_INTRO, CAST_TRIGGERED);
            m_creature->SetInCombatWithZone();
        }
    }

    void DoStartIntro()
    {
        StartNextDialogueText(SAY_GAMESBEGIN_1);
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        if (m_creature->GetMapId() != MAP_ID_BWL)
            return;

        DialogueUpdate(uiDiff);

        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        // Only do this if we haven't spawned nef yet
        if (m_uiSpawnedAdds < MAX_DRAKE_SUMMONS)
        {
            // Shadowbolt Timer
            if (m_uiShadowBoltTimer < uiDiff)
            {
                if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
                {
                    if (DoCastSpellIfCan(pTarget, SPELL_SHADOWBOLT) == CAST_OK)
                        m_uiShadowBoltTimer = urand(2000, 4000);
                }
            }
            else
                m_uiShadowBoltTimer -= uiDiff;

            // Fear Timer
            if (m_uiFearTimer < uiDiff)
            {
                if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 1))
                {
                    if (DoCastSpellIfCan(pTarget, SPELL_FEAR) == CAST_OK)
                        m_uiFearTimer = urand(10000, 20000);
                }
            }
            else
                m_uiFearTimer -= uiDiff;

            // Shadowbolt Volley
            if (m_uiShadowboltVolleyTimer < uiDiff)
            {
                if (DoCastSpellIfCan(m_creature, SPELL_SHADOWBOLT_VOLLEY) == CAST_OK)
                    m_uiShadowboltVolleyTimer = urand(19000, 28000);
            }
            else
                m_uiShadowboltVolleyTimer -= uiDiff;

            // Silence
            if (m_uiSilenceTimer < uiDiff)
            {
                if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
                {
                    if (DoCastSpellIfCan(pTarget, SPELL_SILENCE) == CAST_OK)
                        m_uiSilenceTimer = urand(14000, 23000);
                }
            }
            else
                m_uiSilenceTimer -= uiDiff;

            // Shadow Command
            if (m_uiShadowCommandTimer < uiDiff)
            {
                if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 1))
                {
                    if (DoCastSpellIfCan(pTarget, SPELL_SHADOW_COMMAND) == CAST_OK)
                        m_uiShadowCommandTimer = urand(24000, 30000);
                }
            }
            else
                m_uiShadowCommandTimer -= uiDiff;

            // ShadowBlink
            if (m_uiShadowBlinkTimer < uiDiff)
            {
                if (DoCastSpellIfCan(m_creature, SPELL_SHADOWBLINK) == CAST_OK)
                    m_uiShadowBlinkTimer = urand(30000, 40000);
            }
            else
                m_uiShadowBlinkTimer -= uiDiff;

            // Add spawning mechanism
            if (m_uiAddSpawnTimer < uiDiff)
            {
                // Spawn 2 random types of creatures at the 2 locations
                uint32 uiCreatureId = 0;

                // 1 in 3 chance it will be a chromatic
                uiCreatureId = urand(0, 2) ? m_uiDrakeTypeOne : uint32(NPC_CHROMATIC_DRAKANOID);
                m_creature->SummonCreature(uiCreatureId, aNefarianLocs[0].m_fX, aNefarianLocs[0].m_fY, aNefarianLocs[0].m_fZ, 5.000f, TEMPSUMMON_TIMED_OOC_OR_CORPSE_DESPAWN, 30000);

                // 1 in 3 chance it will be a chromatic
                uiCreatureId = urand(0, 2) ? m_uiDrakeTypeTwo : uint32(NPC_CHROMATIC_DRAKANOID);
                m_creature->SummonCreature(uiCreatureId, aNefarianLocs[1].m_fX, aNefarianLocs[1].m_fY, aNefarianLocs[1].m_fZ, 5.000, TEMPSUMMON_TIMED_OOC_OR_CORPSE_DESPAWN, 30000);

                // Begin phase 2 by spawning Nefarian
                if (m_uiSpawnedAdds >= MAX_DRAKE_SUMMONS)
                {
                    // Inturrupt any spell casting
                    m_creature->InterruptNonMeleeSpells(false);

                    // Make super invis
                    if (m_creature->GetVisibility() != VISIBILITY_OFF)
                        m_creature->SetVisibility(VISIBILITY_OFF);

                    // Spawn Nefarian
                    // Summon as active, to be able to work proper!
                    m_creature->SummonCreature(NPC_NEFARIAN, aNefarianLocs[2].m_fX, aNefarianLocs[2].m_fY, aNefarianLocs[2].m_fZ, 0, TEMPSUMMON_DEAD_DESPAWN, 0, true);
                }

                m_uiAddSpawnTimer = 4000;
            }
            else
                m_uiAddSpawnTimer -= uiDiff;
        }
    }
};

CreatureAI* GetAI_boss_victor_nefarius(Creature* pCreature)
{
    return new boss_victor_nefariusAI(pCreature);
}

bool GossipHello_boss_victor_nefarius(Player* pPlayer, Creature* pCreature)
{
    if (pCreature->GetMapId() != MAP_ID_BWL)
        return true;

    pPlayer->ADD_GOSSIP_ITEM_ID(GOSSIP_ICON_CHAT, GOSSIP_ITEM_NEFARIUS_1 , GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
    pPlayer->SEND_GOSSIP_MENU(GOSSIP_TEXT_NEFARIUS_1, pCreature->GetObjectGuid());
    return true;
}

bool GossipSelect_boss_victor_nefarius(Player* pPlayer, Creature* pCreature, uint32 /*uiSender*/, uint32 uiAction)
{
    if (pCreature->GetMapId() != MAP_ID_BWL)
        return true;

    switch (uiAction)
    {
        case GOSSIP_ACTION_INFO_DEF+1:
            pCreature->HandleEmote(EMOTE_ONESHOT_TALK);
            pPlayer->ADD_GOSSIP_ITEM_ID(GOSSIP_ICON_CHAT, GOSSIP_ITEM_NEFARIUS_2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
            pPlayer->SEND_GOSSIP_MENU(GOSSIP_TEXT_NEFARIUS_2, pCreature->GetObjectGuid());
            break;
        case GOSSIP_ACTION_INFO_DEF+2:
            pCreature->HandleEmote(EMOTE_ONESHOT_TALK);
            pPlayer->ADD_GOSSIP_ITEM_ID(GOSSIP_ICON_CHAT, GOSSIP_ITEM_NEFARIUS_3, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);
            pPlayer->SEND_GOSSIP_MENU(GOSSIP_TEXT_NEFARIUS_3, pCreature->GetObjectGuid());
            break;
        case GOSSIP_ACTION_INFO_DEF+3:
            pCreature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
            pPlayer->CLOSE_GOSSIP_MENU();
            // Start the intro event
            if (boss_victor_nefariusAI* pBossAI = dynamic_cast<boss_victor_nefariusAI*>(pCreature->AI()))
                pBossAI->DoStartIntro();
            break;
    }
    return true;
}

void AddSC_boss_victor_nefarius()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "boss_victor_nefarius";
    pNewScript->GetAI = &GetAI_boss_victor_nefarius;
    pNewScript->pGossipHello = &GossipHello_boss_victor_nefarius;
    pNewScript->pGossipSelect = &GossipSelect_boss_victor_nefarius;
    pNewScript->RegisterSelf();
}
