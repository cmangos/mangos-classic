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
SDName: Boss_Lucifron
SD%Complete: 100
SDComment:
SDCategory: Molten Core
EndScriptData */

#include "precompiled.h"
#include "molten_core.h"

enum
{
    SPELL_IMPENDINGDOOM     = 19702,
    SPELL_LUCIFRONCURSE     = 19703,
    SPELL_SHADOWSHOCK       = 19460
};

struct boss_lucifronAI : public ScriptedAI
{
    boss_lucifronAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
        Reset();
    }

    ScriptedInstance* m_pInstance;

    uint32 m_uiImpendingDoomTimer;
    uint32 m_uiLucifronCurseTimer;
    uint32 m_uiShadowShockTimer;

    void Reset() override
    {
        m_uiImpendingDoomTimer = 10000;
        m_uiLucifronCurseTimer = 20000;
        m_uiShadowShockTimer   = 6000;
    }

    void Aggro(Unit* /*pWho*/) override
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_LUCIFRON, IN_PROGRESS);
    }

    void JustDied(Unit* /*pKiller*/) override
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_LUCIFRON, DONE);
    }

    void JustReachedHome() override
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_LUCIFRON, FAIL);
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        // Impending doom timer
        if (m_uiImpendingDoomTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_IMPENDINGDOOM) == CAST_OK)
                m_uiImpendingDoomTimer = 20000;
        }
        else
            m_uiImpendingDoomTimer -= uiDiff;

        // Lucifron's curse timer
        if (m_uiLucifronCurseTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_LUCIFRONCURSE) == CAST_OK)
                m_uiLucifronCurseTimer = 20000;
        }
        else
            m_uiLucifronCurseTimer -= uiDiff;

        // Shadowshock
        if (m_uiShadowShockTimer < uiDiff)
        {
            if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
            {
                if (DoCastSpellIfCan(pTarget, SPELL_SHADOWSHOCK) == CAST_OK)
                    m_uiShadowShockTimer = 6000;
            }
        }
        else
            m_uiShadowShockTimer -= uiDiff;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_lucifron(Creature* pCreature)
{
    return new boss_lucifronAI(pCreature);
}

void AddSC_boss_lucifron()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "boss_lucifron";
    pNewScript->GetAI = &GetAI_boss_lucifron;
    pNewScript->RegisterSelf();
}
