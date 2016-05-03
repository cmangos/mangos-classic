/* Copyright (C) 2006 - 2011 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

/* ScriptData
SDName: Boss Shade of Eranikus
SD%Complete: 90
SDComment:
SDCategory: Sunken Temple
EndScriptData */

#include "precompiled.h"
#include "sunken_temple.h"

enum Spells
{
    SPELL_ACID_BREATH               = 12891,
    SPELL_DEEP_SLUMBER              = 12890,
    SPELL_TRASH                     = 3391,
    SPELL_WAR_STOMP                 = 11876
};

struct boss_shade_of_eranikusAI : public ScriptedAI
{
    boss_shade_of_eranikusAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = (instance_sunken_temple*)pCreature->GetInstanceData();
        Reset();
    }

    instance_sunken_temple* m_pInstance;

    uint32 m_uiAcidBreathTimer;
    uint32 m_uiDeepSlumberTimer;
    uint32 m_uiTrashTimer;
    uint32 m_uiWarStompTimer;

    void Reset()
    {
        m_uiAcidBreathTimer = urand(3000,4000);
        m_uiDeepSlumberTimer = urand(5000,7000);
        m_uiTrashTimer = urand(8000,10000);
        m_uiWarStompTimer = urand(12000,14000);
    }

    void JustReachedHome()
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_SHADE_OF_ERANIKUS, NOT_STARTED);
    }

    void Aggro(Unit* /*pWho*/)
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_SHADE_OF_ERANIKUS, IN_PROGRESS);
    }

    void JustDied(Unit* /*pKiller*/)
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_SHADE_OF_ERANIKUS, DONE);
    }

    void UpdateAI(const uint32 uiDiff)
    {       
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        // Acid Breath
        if (m_uiAcidBreathTimer < uiDiff)
        {
            DoCastSpellIfCan(m_creature->getVictim(), SPELL_ACID_BREATH);
            m_uiAcidBreathTimer = urand(5000,7000);
        }
        else
            m_uiAcidBreathTimer -= uiDiff;

        // Deep Slumber
        if (m_uiDeepSlumberTimer < uiDiff)
        {
            if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
                DoCastSpellIfCan(pTarget, SPELL_DEEP_SLUMBER);
            m_uiDeepSlumberTimer = (20000,25000);
        }
        else
            m_uiDeepSlumberTimer -= uiDiff;

        // Trash
        if (m_uiTrashTimer < uiDiff)
        {
            DoCastSpellIfCan(m_creature, SPELL_TRASH);
            m_uiTrashTimer = urand(5000,7000);
        }
        else
            m_uiTrashTimer -= uiDiff;

        // War Stomp
        if (m_uiWarStompTimer < uiDiff)
        {
            DoCastSpellIfCan(m_creature->getVictim(), SPELL_WAR_STOMP);
            m_uiWarStompTimer = urand(10000,12000);
        }
        else
            m_uiWarStompTimer -= uiDiff;

        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_boss_shade_of_eranikus(Creature* pCreature)
{
    return new boss_shade_of_eranikusAI(pCreature);
}

void AddSC_boss_shade_of_eranikus()
{
    Script* pNewscript;

    pNewscript = new Script;
    pNewscript->Name = "boss_shade_of_eranikus";
    pNewscript->GetAI = &GetAI_boss_shade_of_eranikus;
    pNewscript->RegisterSelf();
}
