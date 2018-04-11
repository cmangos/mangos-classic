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
SDName: Boss_Nalorakk
SD%Complete: 95
SDComment: Small adjustments may be required
SDCategory: Zul'Aman
EndScriptData */

#include "AI/ScriptDevAI/include/precompiled.h"
#include "zulaman.h"

enum
{
    SAY_EVENT1_SACRIFICE    = -1568014,
    SAY_EVENT2_SACRIFICE    = -1568015,

    SAY_AGGRO               = -1568016,
    SAY_SURGE               = -1568017,
    SAY_TOBEAR              = -1568018,
    SAY_TOTROLL             = -1568019,
    SAY_BERSERK             = -1568020,
    SAY_SLAY1               = -1568021,
    SAY_SLAY2               = -1568022,
    SAY_DEATH               = -1568023,

    SPELL_BERSERK           = 45078,                        // unsure, this increases damage, size and speed

    // Defines for Troll form
    SPELL_BRUTAL_SWIPE      = 42384,
    SPELL_MANGLE            = 42389,
    SPELL_SURGE             = 42402,
    SPELL_BEAR_SHAPE        = 42377,

    // Defines for Bear form
    SPELL_LACERATING_SLASH  = 42395,
    SPELL_REND_FLESH        = 42397,
    SPELL_DEAFENING_ROAR    = 42398
};

struct boss_nalorakkAI : public ScriptedAI
{
    boss_nalorakkAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = (instance_zulaman*)pCreature->GetInstanceData();
        m_uiCurrentWave = 0;
        Reset();
    }

    instance_zulaman* m_pInstance;

    uint32 m_uiChangeFormTimer;
    uint32 m_uiBrutalSwipeTimer;
    uint32 m_uiMangleTimer;
    uint32 m_uiSurgeTimer;
    uint32 m_uiLaceratingSlashTimer;
    uint32 m_uiRendFleshTimer;
    uint32 m_uiDeafeningRoarTimer;
    uint32 m_uiBerserkTimer;
    uint8 m_uiCurrentWave;
    bool m_bIsInBearForm;

    void Reset() override
    {
        m_uiChangeFormTimer         = 45000;
        m_uiBrutalSwipeTimer        = 12000;
        m_uiMangleTimer             = 15000;
        m_uiSurgeTimer              = 20000;
        m_uiLaceratingSlashTimer    = 6000;
        m_uiRendFleshTimer          = 6000;
        m_uiDeafeningRoarTimer      = 20000;
        m_uiBerserkTimer            = 10 * MINUTE * IN_MILLISECONDS;
        m_bIsInBearForm             = false;
    }

    void MoveInLineOfSight(Unit* pWho) override
    {
        ScriptedAI::MoveInLineOfSight(pWho);

        if (m_pInstance && m_pInstance->IsBearPhaseInProgress())
            return;

        if (pWho->GetTypeId() == TYPEID_PLAYER && !((Player*)pWho)->isGameMaster() && m_creature->IsWithinDistInMap(pWho, aBearEventInfo[m_uiCurrentWave].fAggroDist))
        {
            DoScriptText(aBearEventInfo[m_uiCurrentWave].iYellId, m_creature);
            if (m_pInstance)
                m_pInstance->SendNextBearWave(pWho);
        }
    }

    void MovementInform(uint32 uiMotionType, uint32 uiPointId) override
    {
        if (uiMotionType != POINT_MOTION_TYPE)
            return;

        if (uiPointId)
        {
            m_creature->SetFacingTo(aBearEventInfo[m_uiCurrentWave].fO);

            if (m_uiCurrentWave < MAX_BEAR_WAVES - 1)
            {
                if (m_pInstance)
                    m_pInstance->SetBearEventProgress(false);
                ++m_uiCurrentWave;
            }
            else
            {
                // Set the instance data to fail on movement inform because we are not moving the boss to home position
                if (m_pInstance)
                    m_pInstance->SetData(TYPE_NALORAKK, FAIL);
            }
        }
    }

    // Nalorakk evades only after the trash waves are finished
    void EnterEvadeMode() override
    {
        m_creature->RemoveAllAurasOnEvade();
        m_creature->CombatStop(true);
        m_creature->LoadCreatureAddon(true);

        // Boss should evade on the top of the platform
        if (m_creature->isAlive())
            m_creature->GetMotionMaster()->MovePoint(1, aBearEventInfo[m_uiCurrentWave].fX, aBearEventInfo[m_uiCurrentWave].fY, aBearEventInfo[m_uiCurrentWave].fZ);

        m_creature->SetLootRecipient(nullptr);

        Reset();
    }

    void Aggro(Unit* /*pWho*/) override
    {
        DoScriptText(SAY_AGGRO, m_creature);

        if (m_pInstance)
            m_pInstance->SetData(TYPE_NALORAKK, IN_PROGRESS);
    }

    void KilledUnit(Unit* /*pVictim*/) override
    {
        DoScriptText(urand(0, 1) ? SAY_SLAY1 : SAY_SLAY2, m_creature);
    }

    void JustDied(Unit* /*pKiller*/) override
    {
        DoScriptText(SAY_DEATH, m_creature);

        if (!m_pInstance)
            return;

        m_pInstance->SetData(TYPE_NALORAKK, DONE);
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        // Return since we have no target
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        // Berserking
        if (m_uiBerserkTimer)
        {
            if (m_uiBerserkTimer <= uiDiff)
            {
                if (DoCastSpellIfCan(m_creature, SPELL_BERSERK) == CAST_OK)
                {
                    DoScriptText(SAY_BERSERK, m_creature);
                    m_uiBerserkTimer = 0;
                }
            }
            else
                m_uiBerserkTimer -= uiDiff;
        }

        // Spells for Troll Form (only to be casted if we NOT have bear phase aura)
        if (!m_bIsInBearForm)
        {
            // Brutal Swipe (some sources may say otherwise, but I've never seen this in Bear form)
            if (m_uiBrutalSwipeTimer < uiDiff)
            {
                if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_BRUTAL_SWIPE) == CAST_OK)
                    m_uiBrutalSwipeTimer = urand(7000, 15000);
            }
            else
                m_uiBrutalSwipeTimer -= uiDiff;

            // Mangle
            if (m_uiMangleTimer < uiDiff)
            {
                if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_MANGLE) == CAST_OK)
                    m_uiMangleTimer = urand(3000, 15000);
            }
            else
                m_uiMangleTimer -= uiDiff;

            // Surge
            if (m_uiSurgeTimer < uiDiff)
            {
                // select a random unit other than the main tank
                Unit* pTtarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 1);

                // if there aren't other units, cast on the tank
                if (!pTtarget)
                    pTtarget = m_creature->getVictim();

                if (DoCastSpellIfCan(pTtarget, SPELL_SURGE) == CAST_OK)
                {
                    DoScriptText(SAY_SURGE, m_creature);
                    m_uiSurgeTimer = urand(15000, 32500);
                }
            }
            else
                m_uiSurgeTimer -= uiDiff;

            // Change to Bear Form if we're in Troll Form for 45sec
            if (m_uiChangeFormTimer < uiDiff)
            {
                if (DoCastSpellIfCan(m_creature, SPELL_BEAR_SHAPE, CAST_INTERRUPT_PREVIOUS) == CAST_OK)
                {
                    DoScriptText(SAY_TOBEAR, m_creature);
                    m_uiChangeFormTimer         = 30000;
                    m_bIsInBearForm             = true;
                    // Reset bear form timers
                    m_uiLaceratingSlashTimer    = urand(6000, 25000);
                    m_uiRendFleshTimer          = urand(6000, 25000);
                    m_uiDeafeningRoarTimer      = urand(15000, 25000);
                }
            }
            else
                m_uiChangeFormTimer -= uiDiff;
        }
        // Spells for Bear Form (only to be casted if we have bear phase aura)
        else
        {
            // Timer to yell and reset spell timers when bear aura expires
            if (m_uiChangeFormTimer < uiDiff)
            {
                DoScriptText(SAY_TOTROLL, m_creature);
                m_uiChangeFormTimer     = 45000;
                m_bIsInBearForm         = false;
                // Reset troll form timers
                m_uiSurgeTimer          = urand(15000, 32000);
                m_uiBrutalSwipeTimer    = urand(7000, 20000);
                m_uiMangleTimer         = urand(3000, 20000);
            }
            else
                m_uiChangeFormTimer -= uiDiff;

            // Lacerating Slash
            if (m_uiLaceratingSlashTimer < uiDiff)
            {
                if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_LACERATING_SLASH) == CAST_OK)
                    m_uiLaceratingSlashTimer = urand(6000, 20000);
            }
            else
                m_uiLaceratingSlashTimer -= uiDiff;

            // Rend Flesh
            if (m_uiRendFleshTimer < uiDiff)
            {
                if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_REND_FLESH) == CAST_OK)
                    m_uiRendFleshTimer = urand(6000, 20000);
            }
            else
                m_uiRendFleshTimer -= uiDiff;

            // Deafening Roar
            if (m_uiDeafeningRoarTimer < uiDiff)
            {
                if (DoCastSpellIfCan(m_creature, SPELL_DEAFENING_ROAR) == CAST_OK)
                    m_uiDeafeningRoarTimer = urand(15000, 25000);
            }
            else
                m_uiDeafeningRoarTimer -= uiDiff;
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_nalorakk(Creature* pCreature)
{
    return new boss_nalorakkAI(pCreature);
}

void AddSC_boss_nalorakk()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "boss_nalorakk";
    pNewScript->GetAI = &GetAI_boss_nalorakk;
    pNewScript->RegisterSelf();
}
