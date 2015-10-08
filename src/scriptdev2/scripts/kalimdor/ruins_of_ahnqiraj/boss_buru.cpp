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
SDName: Boss_Buru
SD%Complete: 70
SDComment: Timers; Kill eggs on transform NYI; Egg explode damage and Buru stun are missing
SDCategory: Ruins of Ahn'Qiraj
EndScriptData */

#include "precompiled.h"
#include "ruins_of_ahnqiraj.h"

enum
{
    EMOTE_TARGET            = -1509002,

    // boss spells
    SPELL_CREEPING_PLAGUE   = 20512,
    SPELL_DISMEMBER         = 96,
    SPELL_GATHERING_SPEED   = 1834,
    SPELL_FULL_SPEED        = 1557,
    SPELL_THORNS            = 25640,
    SPELL_BURU_TRANSFORM    = 24721,

    // egg spells
    SPELL_SUMMON_HATCHLING  = 1881,
    SPELL_EXPLODE           = 19593,
    SPELL_BURU_EGG_TRIGGER  = 26646,

    // npcs
    NPC_BURU_EGG_TRIGGER    = 15964,
    NPC_BURU_EGG            = 15514,
    NPC_HATCHLING           = 15521,

    PHASE_EGG               = 1,
    PHASE_TRANSFORM         = 2,
};

struct boss_buruAI : public ScriptedAI
{
    boss_buruAI(Creature* pCreature) : ScriptedAI(pCreature) { Reset(); }

    uint8 m_uiPhase;
    uint32 m_uiDismemberTimer;
    uint32 m_uiCreepingPlagueTimer;
    uint32 m_uiGatheringSpeedTimer;
    uint32 m_uiFullSpeedTimer;

    void Reset() override
    {
        m_uiDismemberTimer      = 5000;
        m_uiGatheringSpeedTimer = 9000;
        m_uiCreepingPlagueTimer = 0;
        m_uiFullSpeedTimer      = 60000;
        m_uiPhase               = PHASE_EGG;
    }

    void Aggro(Unit* pWho) override
    {
        DoScriptText(EMOTE_TARGET, m_creature, pWho);
        DoCastSpellIfCan(m_creature, SPELL_THORNS);
        m_creature->FixateTarget(pWho);
    }

    void KilledUnit(Unit* pVictim) override
    {
        // Attack a new random target when a player is killed
        if (pVictim->GetTypeId() == TYPEID_PLAYER)
            DoAttackNewTarget();
    }

    // Wrapper to attack a new target and remove the speed gathering buff
    void DoAttackNewTarget()
    {
        if (m_uiPhase == PHASE_TRANSFORM)
            return;

        m_creature->RemoveAurasDueToSpell(SPELL_FULL_SPEED);
        m_creature->RemoveAurasDueToSpell(SPELL_GATHERING_SPEED);

        if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0, uint32(0), SELECT_FLAG_PLAYER))
        {
            m_creature->FixateTarget(pTarget);
            DoScriptText(EMOTE_TARGET, m_creature, pTarget);
        }

        m_uiFullSpeedTimer      = 60000;
        m_uiGatheringSpeedTimer = 9000;
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        switch (m_uiPhase)
        {
            case PHASE_EGG:

                if (m_uiDismemberTimer < uiDiff)
                {
                    if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_DISMEMBER) == CAST_OK)
                        m_uiDismemberTimer = 5000;
                }
                else
                    m_uiDismemberTimer -= uiDiff;

                if (m_uiFullSpeedTimer)
                {
                    if (m_uiGatheringSpeedTimer < uiDiff)
                    {
                        if (DoCastSpellIfCan(m_creature, SPELL_GATHERING_SPEED) == CAST_OK)
                            m_uiGatheringSpeedTimer = 9000;
                    }
                    else
                        m_uiGatheringSpeedTimer -= uiDiff;

                    if (m_uiFullSpeedTimer <= uiDiff)
                    {
                        if (DoCastSpellIfCan(m_creature, SPELL_FULL_SPEED) == CAST_OK)
                            m_uiFullSpeedTimer = 0;
                    }
                    else
                        m_uiFullSpeedTimer -= uiDiff;
                }

                if (m_creature->GetHealthPercent() < 20.0f)
                {
                    if (DoCastSpellIfCan(m_creature, SPELL_BURU_TRANSFORM) == CAST_OK)
                    {
                        // Not sure of this but the boss should gain full speed in phase II
                        DoCastSpellIfCan(m_creature, SPELL_FULL_SPEED, CAST_TRIGGERED);
                        m_creature->RemoveAurasDueToSpell(SPELL_THORNS);
                        m_creature->FixateTarget(NULL);
                        m_uiPhase = PHASE_TRANSFORM;
                    }
                }

                break;
            case PHASE_TRANSFORM:

                if (m_uiCreepingPlagueTimer < uiDiff)
                {
                    if (DoCastSpellIfCan(m_creature, SPELL_CREEPING_PLAGUE) == CAST_OK)
                        m_uiCreepingPlagueTimer = 6000;
                }
                else
                    m_uiCreepingPlagueTimer -= uiDiff;

                break;
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_buru(Creature* pCreature)
{
    return new boss_buruAI(pCreature);
}

struct npc_buru_eggAI : public Scripted_NoMovementAI
{
    npc_buru_eggAI(Creature* pCreature) : Scripted_NoMovementAI(pCreature)
    {
        m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
        Reset();
    }

    ScriptedInstance* m_pInstance;

    void Reset() override
    { }

    void JustSummoned(Creature* pSummoned) override
    {
        // The purpose of this is unk for the moment
        if (pSummoned->GetEntry() == NPC_BURU_EGG_TRIGGER)
            pSummoned->CastSpell(pSummoned, SPELL_BURU_EGG_TRIGGER, true);
        // The Hatchling should attack a random target
        else if (pSummoned->GetEntry() == NPC_HATCHLING)
        {
            if (m_pInstance)
            {
                if (Creature* pBuru = m_pInstance->GetSingleCreatureFromStorage(NPC_BURU))
                {
                    if (Unit* pTarget = pBuru->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
                        pSummoned->AI()->AttackStart(pTarget);
                }
            }
        }
    }

    void JustDied(Unit* /*pKiller*/) override
    {
        // Explode and Summon hatchling
        DoCastSpellIfCan(m_creature, SPELL_EXPLODE, CAST_TRIGGERED);
        DoCastSpellIfCan(m_creature, SPELL_SUMMON_HATCHLING, CAST_TRIGGERED, m_creature->GetObjectGuid());

        // Reset Buru's target - this might have been done by spell, but currently this is unk to us
        if (m_pInstance)
        {
            if (Creature* pBuru = m_pInstance->GetSingleCreatureFromStorage(NPC_BURU))
            {
                if (boss_buruAI* pBuruAI = dynamic_cast<boss_buruAI*>(pBuru->AI()))
                    pBuruAI->DoAttackNewTarget();
            }
        }
    }

    void UpdateAI(const uint32 /*uiDiff*/) override { }
};

CreatureAI* GetAI_npc_buru_egg(Creature* pCreature)
{
    return new npc_buru_eggAI(pCreature);
}

void AddSC_boss_buru()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "boss_buru";
    pNewScript->GetAI = &GetAI_boss_buru;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_buru_egg";
    pNewScript->GetAI = &GetAI_npc_buru_egg;
    pNewScript->RegisterSelf();
}
