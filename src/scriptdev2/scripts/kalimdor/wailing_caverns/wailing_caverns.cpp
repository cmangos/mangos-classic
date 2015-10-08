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
SDName: wailing_caverns
SD%Complete: 90
SDComment: Missing vipers emerge effect, Naralex doesn't fly at exit(Core issue)
SDCategory: Wailing Caverns
EndScriptData */

#include "precompiled.h"
#include "wailing_caverns.h"
#include "escort_ai.h"

enum
{
    SAY_PREPARE             = -1043001,
    SAY_FIRST_CORNER        = -1043002,
    SAY_CONTINUE            = -1043003,
    SAY_CIRCLE_BANISH       = -1043004,
    SAY_PURIFIED            = -1043005,
    SAY_NARALEX_CHAMBER     = -1043006,
    SAY_BEGIN_RITUAL        = -1043007,
    SAY_MUTANUS             = -1043012,
    SAY_NARALEX_AWAKE       = -1043013,
    SAY_AWAKE               = -1043014,
    SAY_NARALEX_THANKYOU    = -1043015,
    SAY_FAREWELL            = -1043016,
    SAY_AGGRO_1             = -1043017,                     // Random between the first 2
    SAY_AGGRO_2             = -1043018,
    SAY_AGGRO_3             = -1043019,                     // During the awakening ritual

    EMOTE_RITUAL_BEGIN      = -1043008,
    EMOTE_NARALEX_AWAKE     = -1043009,
    EMOTE_BREAK_THROUGH     = -1043010,
    EMOTE_VISION            = -1043011,

    GOSSIP_ITEM_BEGIN       = -3043000,
    TEXT_ID_BEGIN           = 699,
    TEXT_ID_DISCIPLE        = 698,

    SPELL_MARK              = 5232,                         // Buff before the fight. To be removed after 4.0.3
    SPELL_SLEEP             = 1090,
    SPELL_POTION            = 8141,
    SPELL_CLEANSING         = 6270,
    SPELL_AWAKENING         = 6271,
    SPELL_SHAPESHIFT        = 8153,

    NPC_DEVIATE_RAPTOR      = 3636,                         // 2 of them at the first stop
    NPC_DEVIATE_VIPER       = 5755,                         // 3 of them at the circle
    NPC_DEVIATE_MOCCASIN    = 5762,                         // 6 of them at Naralex chamber
    NPC_NIGHTMARE_ECTOPLASM = 5763,                         // 10 of them at Naralex chamber
    NPC_MUTANUS             = 3654
};

// Distance, Angle or Offset
static const float aSummonPositions[5][2] =
{
    {50.0f, -0.507f},                                       // First Raptors
    {53.0f, -0.603f},
    { 7.0f, 1.73f},                                         // Vipers
    {45.0f, 5.16f},                                         // Chamber
    {47.0f, 0.5901f}                                        // Mutanus
};

struct npc_disciple_of_naralexAI : public npc_escortAI
{
    npc_disciple_of_naralexAI(Creature* pCreature) : npc_escortAI(pCreature)
    {
        m_pInstance = (instance_wailing_caverns*)pCreature->GetInstanceData();
        Reset();
    }

    instance_wailing_caverns* m_pInstance;

    uint32 m_uiEventTimer;
    uint32 m_uiSleepTimer;
    uint32 m_uiPotionTimer;
    uint32 m_uiCleansingTimer;
    uint8 m_uiSummonedAlive;
    bool m_bIsFirstHit;

    uint32 m_uiPoint;
    uint8 m_uiSubeventPhase;

    void Reset() override
    {
        m_uiSleepTimer      = 5000;
        m_uiPotionTimer     = 5000;
        m_uiCleansingTimer  = 0;
        m_bIsFirstHit       = false;                        // Used to trigger the combat yell; reset at every evade

        // Don't reset mob counter during the event
        if (!HasEscortState(STATE_ESCORT_ESCORTING))
        {
            m_uiEventTimer = 0;

            m_uiPoint = 0;
            m_uiSubeventPhase = 0;

            m_uiSummonedAlive = 0;
        }
    }

    void JustRespawned() override
    {
        npc_escortAI::JustRespawned();

        // Reset event if can
        if (m_pInstance && m_pInstance->GetData(TYPE_DISCIPLE) != DONE)
            m_pInstance->SetData(TYPE_DISCIPLE, FAIL);
    }

    void AttackedBy(Unit* pAttacker) override
    {
        if (!m_bIsFirstHit)
        {
            if (pAttacker->GetEntry() == NPC_MUTANUS)
                DoScriptText(SAY_MUTANUS, m_creature, pAttacker);
            // Check if already in ritual
            else if (m_uiPoint >= 30)
                DoScriptText(SAY_AGGRO_3, m_creature, pAttacker);
            else
                // Aggro 1 should be in 90% of the cases
                DoScriptText(roll_chance_i(90) ? SAY_AGGRO_1 : SAY_AGGRO_2, m_creature, pAttacker);

            m_bIsFirstHit = true;
        }
    }

    // Overwrite the evade function, to change the combat stop function (keep casting some spells)
    void EnterEvadeMode() override
    {
        // Do not stop casting at these points
        if (m_uiPoint == 15 || m_uiPoint == 32)
        {
            m_creature->SetLootRecipient(nullptr);
            m_creature->DeleteThreatList();
            m_creature->CombatStop(false);
            Reset();

            // Remove running
            m_creature->SetWalk(true);
        }
        else
            npc_escortAI::EnterEvadeMode();
    }

    void JustStartedEscort() override
    {
        DoScriptText(SAY_PREPARE, m_creature);

        if (m_pInstance)
            m_pInstance->SetData(TYPE_DISCIPLE, IN_PROGRESS);
    }

    void WaypointReached(uint32 uiPointId) override
    {
        switch (uiPointId)
        {
            case 7:
                DoScriptText(SAY_FIRST_CORNER, m_creature);
                m_uiSubeventPhase = 0;
                m_uiEventTimer = 2000;
                m_uiPoint = uiPointId;
                SetEscortPaused(true);
                break;
            case 15:
                m_uiSubeventPhase = 0;
                m_uiEventTimer = 2000;
                m_uiPoint = uiPointId;
                SetEscortPaused(true);
                break;
            case 26:
                DoScriptText(SAY_NARALEX_CHAMBER, m_creature);
                break;
            case 32:
                if (Creature* pNaralex = m_pInstance->GetSingleCreatureFromStorage(NPC_NARALEX))
                    m_creature->SetFacingToObject(pNaralex);

                m_creature->SetStandState(UNIT_STAND_STATE_KNEEL);
                m_uiEventTimer = 2000;
                m_uiSubeventPhase = 0;
                m_uiPoint = uiPointId;
                SetEscortPaused(true);
                break;
        }
    }

    void JustSummoned(Creature* pSummoned) override
    {
        // Attack the disciple
        pSummoned->AI()->AttackStart(m_creature);

        ++m_uiSummonedAlive;
    }

    void SummonedCreatureJustDied(Creature* /*pSummoned*/) override
    {
        if (m_uiSummonedAlive == 0)
            return;                                         // Actually if this happens, something went wrong before

        --m_uiSummonedAlive;

        // Continue Event if all are dead and we are in a stopped subevent
        if (m_uiSummonedAlive == 0 && m_uiEventTimer == 0)
            m_uiEventTimer = 1000;
    }

    // Summon mobs at calculated points
    void DoSpawnMob(uint32 uiEntry, float fDistance, float fAngle)
    {
        float fX, fY, fZ;
        m_creature->GetNearPoint(m_creature, fX, fY, fZ, 0, fDistance, fAngle);

        m_creature->SummonCreature(uiEntry, fX, fY, fZ, 0, TEMPSUMMON_TIMED_OOC_DESPAWN, 20000);
    }

    void UpdateEscortAI(const uint32 uiDiff) override
    {
        if (m_uiEventTimer)
        {
            if (m_uiEventTimer <= uiDiff)
            {
                switch (m_uiPoint)
                {
                        // Corner stop -> raptors
                    case 7:
                        switch (m_uiSubeventPhase)
                        {
                            case 0:
                                // Summon raptors at first stop
                                DoSpawnMob(NPC_DEVIATE_RAPTOR, aSummonPositions[0][0], aSummonPositions[0][1]);
                                DoSpawnMob(NPC_DEVIATE_RAPTOR, aSummonPositions[1][0], aSummonPositions[1][1]);
                                m_uiEventTimer = 0;
                                ++m_uiSubeventPhase;
                                break;
                            case 1:
                                // After the summoned mobs are killed continue
                                DoScriptText(SAY_CONTINUE, m_creature);
                                SetEscortPaused(false);
                                m_uiEventTimer = 0;
                                break;
                        }
                        break;
                        // Circle stop -> vipers
                    case 15:
                        switch (m_uiSubeventPhase)
                        {
                            case 0:
                                DoScriptText(SAY_CIRCLE_BANISH, m_creature);
                                m_uiEventTimer = 2000;
                                ++m_uiSubeventPhase;
                                break;
                            case 1:
                                DoCastSpellIfCan(m_creature, SPELL_CLEANSING);
                                m_uiEventTimer = 20000;
                                ++m_uiSubeventPhase;
                                break;
                            case 2:
                                // Summon vipers at the first circle
                                for (uint8 i = 0; i < 3; ++i)
                                    DoSpawnMob(NPC_DEVIATE_VIPER, aSummonPositions[2][0], aSummonPositions[2][1] + 2 * M_PI_F / 3 * i);
                                m_uiEventTimer = 0;
                                ++m_uiSubeventPhase;
                                break;
                            case 3:
                                // Wait for casting to be complete - TODO, this might have to be done better
                                ++m_uiSubeventPhase;
                                m_uiEventTimer = 10000;
                                break;
                            case 4:
                                DoScriptText(SAY_PURIFIED, m_creature);
                                m_uiEventTimer = 0;
                                ++m_uiPoint;             // Increment this in order to avoid the special case evade
                                SetEscortPaused(false);
                                break;
                        }
                        break;
                        // Chamber stop -> ritual and final boss
                    case 32:
                        switch (m_uiSubeventPhase)
                        {
                            case 0:
                                DoScriptText(SAY_BEGIN_RITUAL, m_creature);
                                m_creature->SetStandState(UNIT_STAND_STATE_STAND);
                                m_uiEventTimer = 2000;
                                ++m_uiSubeventPhase;
                                break;
                            case 1:
                                DoCastSpellIfCan(m_creature, SPELL_AWAKENING);
                                DoScriptText(EMOTE_RITUAL_BEGIN, m_creature);
                                m_uiEventTimer = 4000;
                                ++m_uiSubeventPhase;
                                break;
                            case 2:
                                if (Creature* pNaralex = m_pInstance->GetSingleCreatureFromStorage(NPC_NARALEX))
                                    DoScriptText(EMOTE_NARALEX_AWAKE, pNaralex);
                                m_uiEventTimer = 5000;
                                ++m_uiSubeventPhase;
                                break;
                            case 3:
                                // First set of mobs
                                for (uint8 i = 0; i < 3; ++i)
                                    DoSpawnMob(NPC_DEVIATE_MOCCASIN, aSummonPositions[3][0], aSummonPositions[3][1] + M_PI_F / 3 * i);
                                m_uiEventTimer = 20000;
                                ++m_uiSubeventPhase;
                                break;
                            case 4:
                                // Second set of mobs
                                for (uint8 i = 0; i < 7; ++i)
                                    DoSpawnMob(NPC_NIGHTMARE_ECTOPLASM, aSummonPositions[3][0], aSummonPositions[3][1] + M_PI_F / 7 * i);
                                m_uiEventTimer = 0;
                                ++m_uiSubeventPhase;
                                break;
                            case 5:
                                // Advance only when all mobs are dead
                                if (Creature* pNaralex = m_pInstance->GetSingleCreatureFromStorage(NPC_NARALEX))
                                    DoScriptText(EMOTE_BREAK_THROUGH, pNaralex);
                                ++m_uiSubeventPhase;
                                m_uiEventTimer = 10000;
                                break;
                            case 6:
                                // Mutanus
                                if (Creature* pNaralex = m_pInstance->GetSingleCreatureFromStorage(NPC_NARALEX))
                                    DoScriptText(EMOTE_VISION, pNaralex);
                                DoSpawnMob(NPC_MUTANUS, aSummonPositions[4][0], aSummonPositions[4][1]);
                                m_uiEventTimer = 0;
                                ++m_uiSubeventPhase;
                                break;
                            case 7:
                                // Awaken Naralex after mutanus is defeated
                                if (Creature* pNaralex = m_pInstance->GetSingleCreatureFromStorage(NPC_NARALEX))
                                {
                                    pNaralex->SetStandState(UNIT_STAND_STATE_SIT);
                                    DoScriptText(SAY_NARALEX_AWAKE, pNaralex);
                                }
                                m_creature->InterruptNonMeleeSpells(false, SPELL_AWAKENING);
                                m_creature->RemoveAurasDueToSpell(SPELL_AWAKENING);
                                m_pInstance->SetData(TYPE_DISCIPLE, DONE);
                                ++m_uiSubeventPhase;
                                m_uiEventTimer = 2000;
                                break;
                            case 8:
                                DoScriptText(SAY_AWAKE, m_creature);
                                m_uiEventTimer = 5000;
                                ++m_uiSubeventPhase;
                                break;
                            case 9:
                                if (Creature* pNaralex = m_pInstance->GetSingleCreatureFromStorage(NPC_NARALEX))
                                {
                                    DoScriptText(SAY_NARALEX_THANKYOU, pNaralex);
                                    pNaralex->SetStandState(UNIT_STAND_STATE_STAND);
                                }
                                m_uiEventTimer = 10000;
                                ++m_uiSubeventPhase;
                                break;
                            case 10:
                                // Shapeshift into a bird
                                if (Creature* pNaralex = m_pInstance->GetSingleCreatureFromStorage(NPC_NARALEX))
                                {
                                    DoScriptText(SAY_FAREWELL, pNaralex);
                                    pNaralex->CastSpell(pNaralex, SPELL_SHAPESHIFT, false);
                                }
                                DoCastSpellIfCan(m_creature, SPELL_SHAPESHIFT);
                                m_uiEventTimer = 10000;
                                ++m_uiSubeventPhase;
                                break;
                            case 11:
                                SetEscortPaused(false);
                                m_creature->SetLevitate(true);
                                SetRun();
                                // Send them flying somewhere outside of the room
                                if (Creature* pNaralex = m_pInstance->GetSingleCreatureFromStorage(NPC_NARALEX))
                                {
                                    // ToDo: Make Naralex fly
                                    // sort of a hack, compare to boss_onyxia
                                    pNaralex->SetByteValue(UNIT_FIELD_BYTES_1, 3, UNIT_BYTE1_FLAG_ALWAYS_STAND);

                                    // Set to flying
                                    pNaralex->SetLevitate(true);
                                    pNaralex->SetWalk(false);

                                    // Set following
                                    pNaralex->GetMotionMaster()->MoveFollow(m_creature, 5.0f, 0);
                                    // Despawn after some time
                                    pNaralex->ForcedDespawn(30000);
                                }
                                m_uiEventTimer = 0;
                                break;
                        }
                        break;
                }
            }
            else
                m_uiEventTimer -= uiDiff;
        }

        if (m_uiPotionTimer < uiDiff)
        {
            // if health lower than 80%, cast heal
            if (m_creature->GetHealthPercent() < 80.0f)
            {
                if (DoCastSpellIfCan(m_creature, SPELL_POTION) == CAST_OK)
                    m_uiPotionTimer = 45000;
            }
            else
                m_uiPotionTimer = 5000;
        }
        else
            m_uiPotionTimer -= uiDiff;

        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        if (m_uiSleepTimer < uiDiff)
        {
            if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 1))
            {
                if (DoCastSpellIfCan(pTarget, SPELL_SLEEP) == CAST_OK)
                    m_uiSleepTimer = 30000;
            }
        }
        else
            m_uiSleepTimer -= uiDiff;

        DoMeleeAttackIfReady();
    }
};

bool GossipHello_npc_disciple_of_naralex(Player* pPlayer, Creature* pCreature)
{
    ScriptedInstance* m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();

    if (pCreature->isQuestGiver())
        pPlayer->PrepareQuestMenu(pCreature->GetObjectGuid());

    // Buff the players
    pCreature->CastSpell(pPlayer, SPELL_MARK, false);

    if (m_pInstance && m_pInstance->GetData(TYPE_DISCIPLE) == SPECIAL)
    {
        pPlayer->ADD_GOSSIP_ITEM_ID(GOSSIP_ICON_CHAT, GOSSIP_ITEM_BEGIN, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
        pPlayer->SEND_GOSSIP_MENU(TEXT_ID_BEGIN, pCreature->GetObjectGuid());
    }
    else
        pPlayer->SEND_GOSSIP_MENU(TEXT_ID_DISCIPLE, pCreature->GetObjectGuid());

    return true;
}

bool GossipSelect_npc_disciple_of_naralex(Player* pPlayer, Creature* pCreature, uint32 /*uiSender*/, uint32 uiAction)
{
    ScriptedInstance* m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();

    if (!m_pInstance)
        return false;

    if (uiAction == GOSSIP_ACTION_INFO_DEF + 1)
    {
        if (npc_disciple_of_naralexAI* pEscortAI = dynamic_cast<npc_disciple_of_naralexAI*>(pCreature->AI()))
        {
            pEscortAI->Start(false, pPlayer);               // Note: after 4.0.3 set him run = true
            pCreature->SetFactionTemporary(FACTION_ESCORT_N_NEUTRAL_ACTIVE, TEMPFACTION_RESTORE_RESPAWN);
        }
        pPlayer->CLOSE_GOSSIP_MENU();
    }
    return true;
}

CreatureAI* GetAI_npc_disciple_of_naralex(Creature* pCreature)
{
    return new npc_disciple_of_naralexAI(pCreature);
}

void AddSC_wailing_caverns()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "npc_disciple_of_naralex";
    pNewScript->GetAI = &GetAI_npc_disciple_of_naralex;
    pNewScript->pGossipHello =  &GossipHello_npc_disciple_of_naralex;
    pNewScript->pGossipSelect = &GossipSelect_npc_disciple_of_naralex;
    pNewScript->RegisterSelf();
}
