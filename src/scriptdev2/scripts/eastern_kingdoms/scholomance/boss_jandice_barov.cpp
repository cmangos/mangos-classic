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
SDName: Boss_jandicebarov
SD%Complete: 100
SDComment:
SDCategory: Scholomance
EndScriptData */

#include "precompiled.h"

enum
{
    SPELL_CURSE_OF_BLOOD        = 16098,
    SPELL_SUMMON_ILLUSIONS      = 17773,
    SPELL_BANISH                = 8994,
};

struct boss_jandicebarovAI : public ScriptedAI
{
    boss_jandicebarovAI(Creature* pCreature) : ScriptedAI(pCreature) {Reset();}

    uint32 m_uiCurseOfBloodTimer;
    uint32 m_uiIllusionTimer;
    uint32 m_uiBanishTimer;

    void Reset() override
    {
        m_uiCurseOfBloodTimer = 5000;
        m_uiIllusionTimer = 15000;
        m_uiBanishTimer = urand(9000, 13000);
    }

    void JustSummoned(Creature* pSummoned) override
    {
        pSummoned->ApplySpellImmune(0, IMMUNITY_DAMAGE, SPELL_SCHOOL_MASK_MAGIC, true);

        if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
            pSummoned->AI()->AttackStart(pTarget);
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        // Return since we have no target
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        // CurseOfBlood_Timer
        if (m_uiCurseOfBloodTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_CURSE_OF_BLOOD) == CAST_OK)
                m_uiCurseOfBloodTimer = urand(30000, 35000);
        }
        else
            m_uiCurseOfBloodTimer -= uiDiff;

        // Banish
        if (m_uiBanishTimer < uiDiff)
        {
            if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 1))
            {
                if (DoCastSpellIfCan(pTarget, SPELL_BANISH) == CAST_OK)
                    m_uiBanishTimer = urand(17000, 21000);
            }
        }
        else
            m_uiBanishTimer -= uiDiff;

        // Illusion_Timer
        if (m_uiIllusionTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_SUMMON_ILLUSIONS) == CAST_OK)
                m_uiIllusionTimer = 25000;
        }
        else
            m_uiIllusionTimer -= uiDiff;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_jandicebarov(Creature* pCreature)
{
    return new boss_jandicebarovAI(pCreature);
}

void AddSC_boss_jandicebarov()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "boss_jandice_barov";
    pNewScript->GetAI = &GetAI_boss_jandicebarov;
    pNewScript->RegisterSelf();
}
