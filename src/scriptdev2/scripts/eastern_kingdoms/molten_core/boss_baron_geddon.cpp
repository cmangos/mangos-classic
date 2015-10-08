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
SDName: Boss_Baron_Geddon
SD%Complete: 100
SDComment: Armaggedon is not working properly (core issue)
SDCategory: Molten Core
EndScriptData */

#include "precompiled.h"
#include "molten_core.h"

enum
{
    EMOTE_SERVICE               = -1409000,

    SPELL_INFERNO               = 19695,
    SPELL_IGNITE_MANA           = 19659,
    SPELL_LIVING_BOMB           = 20475,
    SPELL_ARMAGEDDON            = 20478
};

struct boss_baron_geddonAI : public ScriptedAI
{
    boss_baron_geddonAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
        Reset();
    }

    ScriptedInstance* m_pInstance;

    bool m_bIsArmageddon;
    uint32 m_uiInfernoTimer;
    uint32 m_uiIgniteManaTimer;
    uint32 m_uiLivingBombTimer;

    void Reset() override
    {
        m_bIsArmageddon = false;
        m_uiInfernoTimer = 45000;
        m_uiIgniteManaTimer = 30000;
        m_uiLivingBombTimer = 35000;
    }

    void Aggro(Unit* /*pWho*/) override
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_GEDDON, IN_PROGRESS);
    }

    void JustDied(Unit* /*pKiller*/) override
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_GEDDON, DONE);
    }

    void JustReachedHome() override
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_GEDDON, NOT_STARTED);
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        if (m_bIsArmageddon)                                // Do nothing untill armageddon triggers
            return;

        // If we are <2% hp cast Armageddom
        if (m_creature->GetHealthPercent() <= 2.0f && !m_bIsArmageddon)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_ARMAGEDDON, CAST_INTERRUPT_PREVIOUS) == CAST_OK)
            {
                DoScriptText(EMOTE_SERVICE, m_creature);
                m_bIsArmageddon = true;
                return;
            }
        }

        // Inferno_Timer
        if (m_uiInfernoTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_INFERNO) == CAST_OK)
                m_uiInfernoTimer = 45000;
        }
        else
            m_uiInfernoTimer -= uiDiff;

        // Ignite Mana Timer
        if (m_uiIgniteManaTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_IGNITE_MANA) == CAST_OK)
                m_uiIgniteManaTimer = 30000;
        }
        else
            m_uiIgniteManaTimer -= uiDiff;

        // Living Bomb Timer
        if (m_uiLivingBombTimer < uiDiff)
        {
            if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
            {
                if (DoCastSpellIfCan(pTarget, SPELL_LIVING_BOMB) == CAST_OK)
                    m_uiLivingBombTimer = 35000;
            }
        }
        else
            m_uiLivingBombTimer -= uiDiff;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_baron_geddon(Creature* pCreature)
{
    return new boss_baron_geddonAI(pCreature);
}

void AddSC_boss_baron_geddon()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "boss_baron_geddon";
    pNewScript->GetAI = &GetAI_boss_baron_geddon;
    pNewScript->RegisterSelf();
}
