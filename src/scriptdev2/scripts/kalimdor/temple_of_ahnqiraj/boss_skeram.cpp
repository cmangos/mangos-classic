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
SDName: Boss_Skeram
SD%Complete: 90
SDComment: Timers
SDCategory: Temple of Ahn'Qiraj
EndScriptData */

#include "precompiled.h"
#include "temple_of_ahnqiraj.h"

enum
{
    SAY_AGGRO1                  = -1531000,
    SAY_AGGRO2                  = -1531001,
    SAY_AGGRO3                  = -1531002,
    SAY_SLAY1                   = -1531003,
    SAY_SLAY2                   = -1531004,
    SAY_SLAY3                   = -1531005,
    SAY_SPLIT                   = -1531006,
    SAY_DEATH                   = -1531007,

    SPELL_ARCANE_EXPLOSION      = 26192,
    SPELL_EARTH_SHOCK           = 26194,
    SPELL_TRUE_FULFILLMENT      = 785,
    SPELL_TELEPORT_1            = 4801,
    SPELL_TELEPORT_2            = 8195,
    SPELL_TELEPORT_3            = 20449,
    SPELL_INITIALIZE_IMAGE      = 3730,
    SPELL_SUMMON_IMAGES         = 747,
};

struct boss_skeramAI : public ScriptedAI
{
    boss_skeramAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
        Reset();

        // Check this for images, because the Initialize spell hits the target only after aggro
        if (m_creature->IsTemporarySummon())
            m_bIsImage = true;
        else
            m_bIsImage = false;
    }

    ScriptedInstance* m_pInstance;

    uint32 m_uiArcaneExplosionTimer;
    uint32 m_uiFullFillmentTimer;
    uint32 m_uiBlinkTimer;
    uint32 m_uiEarthShockTimer;

    float m_fHpCheck;

    bool m_bIsImage;

    void Reset() override
    {
        m_uiArcaneExplosionTimer = urand(6000, 12000);
        m_uiFullFillmentTimer    = 15000;
        m_uiBlinkTimer           = urand(30000, 45000);
        m_uiEarthShockTimer      = 1200;

        m_fHpCheck               = 75.0f;

        if (m_creature->GetVisibility() != VISIBILITY_ON)
            m_creature->SetVisibility(VISIBILITY_ON);
    }

    void KilledUnit(Unit* /*pVictim*/) override
    {
        switch (urand(0, 2))
        {
            case 0: DoScriptText(SAY_SLAY1, m_creature); break;
            case 1: DoScriptText(SAY_SLAY2, m_creature); break;
            case 2: DoScriptText(SAY_SLAY3, m_creature); break;
        }
    }

    void JustDied(Unit* /*pKiller*/) override
    {
        if (!m_bIsImage)
        {
            DoScriptText(SAY_DEATH, m_creature);

            if (m_pInstance)
                m_pInstance->SetData(TYPE_SKERAM, DONE);
        }
        // Else despawn to avoid looting
        else
            m_creature->ForcedDespawn(1);
    }

    void Aggro(Unit* /*pWho*/) override
    {
        if (m_bIsImage)
            return;

        switch (urand(0, 2))
        {
            case 0: DoScriptText(SAY_AGGRO1, m_creature); break;
            case 1: DoScriptText(SAY_AGGRO2, m_creature); break;
            case 2: DoScriptText(SAY_AGGRO3, m_creature); break;
        }

        if (m_pInstance)
            m_pInstance->SetData(TYPE_SKERAM, IN_PROGRESS);
    }

    void JustReachedHome() override
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_SKERAM, FAIL);

        if (m_bIsImage)
            m_creature->ForcedDespawn();
    }

    void JustSummoned(Creature* pSummoned) override
    {
        if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
            pSummoned->AI()->AttackStart(pTarget);

        DoCastSpellIfCan(pSummoned, SPELL_INITIALIZE_IMAGE, CAST_TRIGGERED);
    }

    // Wrapper to handle the image version initialize
    void DoInitializeImage()
    {
        if (!m_pInstance)
            return;

        // Initialize the health of the clone
        if (Creature* pProphet = m_pInstance->GetSingleCreatureFromStorage(NPC_SKERAM))
        {
            float fHealthPct = pProphet->GetHealthPercent();
            float fMaxHealthPct = 0;

            // The max health depends on the split phase. It's percent * original boss health
            if (fHealthPct < 25.0f)
                fMaxHealthPct = 0.50f;
            else if (fHealthPct < 50.0f)
                fMaxHealthPct = 0.20f;
            else
                fMaxHealthPct = 0.10f;

            // Set the same health percent as the original boss
            m_creature->SetMaxHealth(m_creature->GetMaxHealth()*fMaxHealthPct);
            m_creature->SetHealthPercent(fHealthPct);
        }
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        // Return since we have no target
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        // ArcaneExplosion_Timer
        if (m_uiArcaneExplosionTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_ARCANE_EXPLOSION) == CAST_OK)
                m_uiArcaneExplosionTimer = urand(8000, 18000);
        }
        else
            m_uiArcaneExplosionTimer -= uiDiff;

        // True Fullfilment
        if (m_uiFullFillmentTimer < uiDiff)
        {
            if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 1))
            {
                if (DoCastSpellIfCan(pTarget, SPELL_TRUE_FULFILLMENT) == CAST_OK)
                    m_uiFullFillmentTimer = urand(20000, 30000);
            }
        }
        else
            m_uiFullFillmentTimer -= uiDiff;

        // Blink_Timer
        if (m_uiBlinkTimer < uiDiff)
        {
            switch (urand(0, 2))
            {
                case 0: DoCastSpellIfCan(m_creature, SPELL_TELEPORT_1); break;
                case 1: DoCastSpellIfCan(m_creature, SPELL_TELEPORT_2); break;
                case 2: DoCastSpellIfCan(m_creature, SPELL_TELEPORT_3); break;
            }

            DoResetThreat();
            if (m_creature->GetVisibility() != VISIBILITY_ON)
                m_creature->SetVisibility(VISIBILITY_ON);

            m_uiBlinkTimer = urand(10000, 30000);
        }
        else
            m_uiBlinkTimer -= uiDiff;

        // Earth Shock is cast every 1.2s on the victim if Skeram can't reach them or they are not auto attacking him
        if (m_uiEarthShockTimer < uiDiff)
        {
            if (!m_creature->CanReachWithMeleeAttack(m_creature->getVictim()) || !m_creature->getVictim()->hasUnitState(UNIT_STAT_MELEE_ATTACKING))
            {
                if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_EARTH_SHOCK) == CAST_OK)
                    m_uiEarthShockTimer = 1200;
            }
        }
        else
            m_uiEarthShockTimer -= uiDiff;

        // Summon images at 75%, 50% and 25%
        if (!m_bIsImage && m_creature->GetHealthPercent() < m_fHpCheck)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_SUMMON_IMAGES) == CAST_OK)
            {
                DoScriptText(SAY_SPLIT, m_creature);
                m_fHpCheck -= 25.0f;
                // Teleport shortly after the images are summoned and set invisible to clear the selection (Workaround alert!!!)
                m_creature->SetVisibility(VISIBILITY_OFF);
                m_uiBlinkTimer = 2000;
            }
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_skeram(Creature* pCreature)
{
    return new boss_skeramAI(pCreature);
}

bool EffectDummyCreature_prophet_skeram(Unit* /*pCaster*/, uint32 uiSpellId, SpellEffectIndex uiEffIndex, Creature* pCreatureTarget, ObjectGuid /*originalCasterGuid*/)
{
    // always check spellid and effectindex
    if (uiSpellId == SPELL_INITIALIZE_IMAGE && uiEffIndex == EFFECT_INDEX_0)
    {
        // check for target == caster first
        if (instance_temple_of_ahnqiraj* pInstance = (instance_temple_of_ahnqiraj*)pCreatureTarget->GetInstanceData())
        {
            if (Creature* pProphet = pInstance->GetSingleCreatureFromStorage(NPC_SKERAM))
            {
                if (pProphet == pCreatureTarget)
                    return false;
            }
        }

        if (boss_skeramAI* pSkeramAI = dynamic_cast<boss_skeramAI*>(pCreatureTarget->AI()))
            pSkeramAI->DoInitializeImage();
    }

    return false;
}

void AddSC_boss_skeram()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "boss_skeram";
    pNewScript->GetAI = &GetAI_boss_skeram;
    pNewScript->pEffectDummyNPC = &EffectDummyCreature_prophet_skeram;
    pNewScript->RegisterSelf();
}
