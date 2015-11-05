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
SDName: Boss_Hazzarah
SD%Complete: 100
SDComment:
SDCategory: Zul'Gurub
EndScriptData */

#include "precompiled.h"

enum
{
    SPELL_CHAIN_BURN            = 24684,
    SPELL_SLEEP                 = 24664,
    SPELL_EARTH_SHOCK           = 24685,
    SPELL_SUMMON_ILLUSION_1     = 24681,
    SPELL_SUMMON_ILLUSION_2     = 24728,
    SPELL_SUMMON_ILLUSION_3     = 24729,
};

struct boss_hazzarahAI : public ScriptedAI
{
    boss_hazzarahAI(Creature* pCreature) : ScriptedAI(pCreature) { Reset(); }

    uint32 m_uiManaBurnTimer;
    uint32 m_uiSleepTimer;
    uint32 m_uiEarthShockTimer;
    uint32 m_uiIllusionsTimer;

    void Reset() override
    {
        m_uiManaBurnTimer   = urand(4000, 10000);
        m_uiSleepTimer      = urand(10000, 18000);
        m_uiEarthShockTimer = urand(7000, 14000);
        m_uiIllusionsTimer  = urand(10000, 18000);
    }

    void JustSummoned(Creature* pSummoned) override
    {
        if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
            pSummoned->AI()->AttackStart(pTarget);
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        // ManaBurn_Timer
        if (m_uiManaBurnTimer < uiDiff)
        {
            if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0, SPELL_CHAIN_BURN, SELECT_FLAG_POWER_MANA))
            {
                if (DoCastSpellIfCan(pTarget, SPELL_CHAIN_BURN) == CAST_OK)
                    m_uiManaBurnTimer = urand(8000, 16000);
            }
        }
        else
            m_uiManaBurnTimer -= uiDiff;

        // Sleep_Timer
        if (m_uiSleepTimer < uiDiff)
        {
            if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
            {
                if (DoCastSpellIfCan(pTarget, SPELL_SLEEP) == CAST_OK)
                    m_uiSleepTimer = urand(12000, 20000);
            }
        }
        else
            m_uiSleepTimer -= uiDiff;

        // Earthshock
        if (m_uiEarthShockTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_EARTH_SHOCK) == CAST_OK)
                m_uiEarthShockTimer = urand(9000, 16000);
        }
        else
            m_uiEarthShockTimer -= uiDiff;

        // Illusions_Timer
        if (m_uiIllusionsTimer < uiDiff)
        {
            DoCastSpellIfCan(m_creature, SPELL_SUMMON_ILLUSION_1, CAST_TRIGGERED);
            DoCastSpellIfCan(m_creature, SPELL_SUMMON_ILLUSION_2, CAST_TRIGGERED);
            DoCastSpellIfCan(m_creature, SPELL_SUMMON_ILLUSION_3, CAST_TRIGGERED);

            m_uiIllusionsTimer = urand(15000, 25000);
        }
        else
            m_uiIllusionsTimer -= uiDiff;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_hazzarah(Creature* pCreature)
{
    return new boss_hazzarahAI(pCreature);
}

void AddSC_boss_hazzarah()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "boss_hazzarah";
    pNewScript->GetAI = &GetAI_boss_hazzarah;
    pNewScript->RegisterSelf();
}
