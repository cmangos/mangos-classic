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
EndScriptData

*/

#include "AI/ScriptDevAI/include/precompiled.h"
#include "molten_core.h"

enum
{
    SPELL_SHADOWSHOCK       = 19460,
    SPELL_IMPENDINGDOOM     = 19702,
    SPELL_LUCIFRONCURSE     = 19703,
};

struct boss_lucifronAI : public ScriptedAI
{
    boss_lucifronAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
        Reset();
    }

    ScriptedInstance* m_pInstance;

    uint32 m_uiShadowShockTimer;
    uint32 m_uiImpendingDoomTimer;
    uint32 m_uiLucifronCurseTimer;

    void Reset() override
    {
        m_uiShadowShockTimer   = urand(3 * IN_MILLISECONDS, 6 * IN_MILLISECONDS);
        m_uiImpendingDoomTimer = urand(5 * IN_MILLISECONDS, 10 * IN_MILLISECONDS);
        m_uiLucifronCurseTimer = urand(10 * IN_MILLISECONDS, 15 * IN_MILLISECONDS);
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

        // Shadowshock
        if (m_uiShadowShockTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_SHADOWSHOCK) == CAST_OK)
                m_uiShadowShockTimer = urand(3 * IN_MILLISECONDS, 6 * IN_MILLISECONDS);
        }
        else
            m_uiShadowShockTimer -= uiDiff;

        // Impending doom timer
        if (m_uiImpendingDoomTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_IMPENDINGDOOM) == CAST_OK)
                m_uiImpendingDoomTimer = urand(20 * IN_MILLISECONDS, 25 * IN_MILLISECONDS);
        }
        else
            m_uiImpendingDoomTimer -= uiDiff;

        // Lucifron's curse timer
        if (m_uiLucifronCurseTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_LUCIFRONCURSE) == CAST_OK)
                m_uiLucifronCurseTimer = urand(20 * IN_MILLISECONDS, 25 * IN_MILLISECONDS);
        }
        else
            m_uiLucifronCurseTimer -= uiDiff;

        DoMeleeAttackIfReady();
    }
};

UnitAI* GetAI_boss_lucifron(Creature* pCreature)
{
    return new boss_lucifronAI(pCreature);
}

void AddSC_boss_lucifron()
{
    Script* pNewScript = new Script;
    pNewScript->Name = "boss_lucifron";
    pNewScript->GetAI = &GetAI_boss_lucifron;
    pNewScript->RegisterSelf();
}
