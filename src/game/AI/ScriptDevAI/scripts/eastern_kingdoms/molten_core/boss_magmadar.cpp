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
SDName: Boss_Magmadar
SD%Complete: 100
SDComment: Complete
SDCategory: Molten Core
EndScriptData */

#include "precompiled.h"
#include "molten_core.h"

enum
{
    EMOTE_GENERIC_FRENZY_KILL   = -1000001,

    // SPELL_LAVA_BREATH           = 19272,                    // Triggered by SPELL_FRENZY (19451)
    SPELL_FRENZY                = 19451,
    SPELL_MAGMASPIT             = 19449,                    // This is actually a buff he gives himself
    SPELL_PANIC                 = 19408,
    SPELL_LAVABOMB              = 19411,                    // This calls a dummy server side effect that cast spell 20494 to spawn GO 177704
    // SPELL_CONFLAGRATION         = 19428                     // Trap spell for GO 177704
};

struct boss_magmadarAI : public ScriptedAI
{
    boss_magmadarAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
        Reset();
    }

    ScriptedInstance* m_pInstance;

    uint32 m_uiFrenzyTimer;
    uint32 m_uiPanicTimer;
    uint32 m_uiLavabombTimer;

    void Reset() override
    {
        m_uiFrenzyTimer = 30000;
        m_uiPanicTimer = 7000;
        m_uiLavabombTimer = 12000;
    }

    void Aggro(Unit* /*pWho*/) override
    {
        DoCastSpellIfCan(m_creature, SPELL_MAGMASPIT, true);

        if (m_pInstance)
            m_pInstance->SetData(TYPE_MAGMADAR, IN_PROGRESS);
    }

    void JustDied(Unit* /*pKiller*/) override
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_MAGMADAR, DONE);
    }

    void JustReachedHome() override
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_MAGMADAR, NOT_STARTED);
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        // Frenzy_Timer
        if (m_uiFrenzyTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_FRENZY) == CAST_OK)
            {
                DoScriptText(EMOTE_GENERIC_FRENZY_KILL, m_creature);
                m_uiFrenzyTimer = urand(15000, 20000);
            }
        }
        else
            m_uiFrenzyTimer -= uiDiff;

        // Panic_Timer
        if (m_uiPanicTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_PANIC) == CAST_OK)
                m_uiPanicTimer = urand(30000, 40000);
        }
        else
            m_uiPanicTimer -= uiDiff;

        // Lavabomb_Timer
        if (m_uiLavabombTimer < uiDiff)
        {
            if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
            {
                if (DoCastSpellIfCan(pTarget, SPELL_LAVABOMB) == CAST_OK)
                    m_uiLavabombTimer = urand(12000, 15000);
            }
        }
        else
            m_uiLavabombTimer -= uiDiff;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_magmadar(Creature* pCreature)
{
    return new boss_magmadarAI(pCreature);
}

void AddSC_boss_magmadar()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "boss_magmadar";
    pNewScript->GetAI = &GetAI_boss_magmadar;
    pNewScript->RegisterSelf();
}
