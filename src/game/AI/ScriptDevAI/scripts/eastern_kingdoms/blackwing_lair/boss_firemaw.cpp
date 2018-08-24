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
SDName: Boss_Firemaw
SD%Complete: 100
SDComment:
SDCategory: Blackwing Lair
EndScriptData

*/

#include "AI/ScriptDevAI/include/precompiled.h"
#include "blackwing_lair.h"

enum
{
    SPELL_SHADOW_FLAME          = 22539,
    SPELL_WING_BUFFET           = 23339,
    SPELL_FLAME_BUFFET          = 23341,
    SPELL_THRASH                = 3391,
};

struct boss_firemawAI : public ScriptedAI
{
    boss_firemawAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
        Reset();
    }

    ScriptedInstance* m_pInstance;

    uint32 m_uiShadowFlameTimer;
    uint32 m_uiWingBuffetTimer;
    uint32 m_uiThrashTimer;
    uint32 m_uiFlameBuffetTimer;

    void Reset() override
    {
        m_uiShadowFlameTimer        = 18 * IN_MILLISECONDS;
        m_uiWingBuffetTimer         = 30 * IN_MILLISECONDS;
        m_uiThrashTimer             = 6 * IN_MILLISECONDS;
        m_uiFlameBuffetTimer        = 5000;
    }

    void Aggro(Unit* /*pWho*/) override
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_FIREMAW, IN_PROGRESS);
    }

    void JustDied(Unit* /*pKiller*/) override
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_FIREMAW, DONE);
    }

    void JustReachedHome() override
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_FIREMAW, FAIL);
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        // Shadow Flame Timer
        if (m_uiShadowFlameTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_SHADOW_FLAME) == CAST_OK)
                m_uiShadowFlameTimer = urand(15 * IN_MILLISECONDS, 18 * IN_MILLISECONDS);
        }
        else
            m_uiShadowFlameTimer -= uiDiff;

        // Wing Buffet Timer
        if (m_uiWingBuffetTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_WING_BUFFET) == CAST_OK)
            {
                if (m_creature->getThreatManager().getThreat(m_creature->getVictim()))
                    m_creature->getThreatManager().modifyThreatPercent(m_creature->getVictim(), -50);

                m_uiWingBuffetTimer = urand(30 * IN_MILLISECONDS, 35 * IN_MILLISECONDS);
            }
        }
        else
            m_uiWingBuffetTimer -= uiDiff;

        // Thrash Timer
        if (m_uiThrashTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_THRASH) == CAST_OK)
            {
                m_uiThrashTimer = urand(2 * IN_MILLISECONDS, 6 * IN_MILLISECONDS);
            }
        }
        else
            m_uiThrashTimer -= uiDiff;

        // Flame Buffet Timer
        if (m_uiFlameBuffetTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_FLAME_BUFFET) == CAST_OK)
                m_uiFlameBuffetTimer = 5000;
        }
        else
            m_uiFlameBuffetTimer -= uiDiff;

        DoMeleeAttackIfReady();
    }
};
UnitAI* GetAI_boss_firemaw(Creature* pCreature)
{
    return new boss_firemawAI(pCreature);
}

void AddSC_boss_firemaw()
{
    Script* pNewScript = new Script;
    pNewScript->Name = "boss_firemaw";
    pNewScript->GetAI = &GetAI_boss_firemaw;
    pNewScript->RegisterSelf();
}
