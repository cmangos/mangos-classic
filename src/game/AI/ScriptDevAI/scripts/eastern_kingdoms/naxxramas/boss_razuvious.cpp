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
SDName: Boss_Razuvious
SD%Complete: 90%
SDComment: TODO: Timers and sounds need confirmation
SDCategory: Naxxramas
EndScriptData

*/

#include "AI/ScriptDevAI/include/precompiled.h"
#include "naxxramas.h"

enum
{
    SAY_AGGRO1               = -1533120,
    SAY_AGGRO2               = -1533121,
    SAY_AGGRO3               = -1533122,
    SAY_AGGRO4               = -1533123,
    SAY_SLAY1                = -1533124,
    SAY_SLAY2                = -1533125,
    SAY_TRIUMPHANT1          = -1533126,
    SAY_TRIUMPHANT2          = -1533127,
    SAY_TRIUMPHANT3          = -1533128,
    SAY_DEATH                = -1533129,

    SPELL_UNBALANCING_STRIKE = 26613,
    SPELL_DISRUPTING_SHOUT   = 29107,
    SPELL_HOPELESS           = 29125
};

struct boss_razuviousAI : public ScriptedAI
{
    boss_razuviousAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = (instance_naxxramas*)pCreature->GetInstanceData();
        Reset();
    }

    instance_naxxramas* m_pInstance;

    uint32 m_uiUnbalancingStrikeTimer;
    uint32 m_uiDisruptingShoutTimer;

    void Reset() override
    {
        m_uiUnbalancingStrikeTimer = 30000;                 // 30 seconds
        m_uiDisruptingShoutTimer   = 25000;                 // 25 seconds
    }

    void KilledUnit(Unit* /*Victim*/) override
    {
        if (urand(0, 3))
            return;

        switch (urand(0, 1))
        {
            case 0: DoScriptText(SAY_SLAY1, m_creature); break;
            case 1: DoScriptText(SAY_SLAY2, m_creature); break;
        }
    }

    void JustDied(Unit* /*pKiller*/) override
    {
        DoScriptText(SAY_DEATH, m_creature);

        DoCastSpellIfCan(m_creature, SPELL_HOPELESS, CAST_TRIGGERED);

        if (m_pInstance)
            m_pInstance->SetData(TYPE_RAZUVIOUS, DONE);
    }

    void Aggro(Unit* /*pWho*/) override
    {
        switch (urand(0, 3))
        {
            case 0: DoScriptText(SAY_AGGRO1, m_creature); break;
            case 1: DoScriptText(SAY_AGGRO2, m_creature); break;
            case 2: DoScriptText(SAY_AGGRO3, m_creature); break;
            case 3: DoScriptText(SAY_AGGRO4, m_creature); break;
        }

        if (m_pInstance)
            m_pInstance->SetData(TYPE_RAZUVIOUS, IN_PROGRESS);
    }

    void JustReachedHome() override
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_RAZUVIOUS, FAIL);
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        // Unbalancing Strike
        if (m_uiUnbalancingStrikeTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_UNBALANCING_STRIKE) == CAST_OK)
                m_uiUnbalancingStrikeTimer = 30000;
        }
        else
            m_uiUnbalancingStrikeTimer -= uiDiff;

        // Disrupting Shout
        if (m_uiDisruptingShoutTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_DISRUPTING_SHOUT) == CAST_OK)
                m_uiDisruptingShoutTimer = 25000;

            // TODO need core support to make this yells happen only when spell Disrupting Shout hit a target
            // For now, Razuvious will yell it at each cast
            switch (urand(0, 3))
            {
                case 0: DoScriptText(SAY_TRIUMPHANT1, m_creature); break;
                case 1: DoScriptText(SAY_TRIUMPHANT2, m_creature); break;
                case 2: DoScriptText(SAY_TRIUMPHANT3, m_creature); break;
            }
        }
        else
            m_uiDisruptingShoutTimer -= uiDiff;

        DoMeleeAttackIfReady();
    }
};

UnitAI* GetAI_boss_razuvious(Creature* pCreature)
{
    return new boss_razuviousAI(pCreature);
}

void AddSC_boss_razuvious()
{
    Script* pNewScript = new Script;
    pNewScript->Name = "boss_razuvious";
    pNewScript->GetAI = &GetAI_boss_razuvious;
    pNewScript->RegisterSelf();
}
