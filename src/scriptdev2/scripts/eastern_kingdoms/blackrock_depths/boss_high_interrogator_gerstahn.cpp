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
SDName: Boss_High_Interrogator_Gerstahn
SD%Complete: 100
SDComment:
SDCategory: Blackrock Depths
EndScriptData */

#include "precompiled.h"

enum
{
    SPELL_SHADOWWORDPAIN        = 14032,
    SPELL_MANABURN              = 14033,
    SPELL_PSYCHICSCREAM         = 13704,
    SPELL_SHADOWSHIELD          = 12040
};

struct boss_high_interrogator_gerstahnAI : public ScriptedAI
{
    boss_high_interrogator_gerstahnAI(Creature* pCreature) : ScriptedAI(pCreature) {Reset();}

    uint32 m_uiShadowWordPainTimer;
    uint32 m_uiManaBurnTimer;
    uint32 m_uiPsychicScreamTimer;
    uint32 m_uiShadowShieldTimer;

    void Reset() override
    {
        m_uiShadowWordPainTimer = 4000;
        m_uiManaBurnTimer = 14000;
        m_uiPsychicScreamTimer = 32000;
        m_uiShadowShieldTimer = 8000;
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        // Return since we have no target
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        // ShadowWordPain_Timer
        if (m_uiShadowWordPainTimer < uiDiff)
        {
            if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
            {
                if (DoCastSpellIfCan(pTarget, SPELL_SHADOWWORDPAIN) == CAST_OK)
                    m_uiShadowWordPainTimer = 7000;
            }
        }
        else
            m_uiShadowWordPainTimer -= uiDiff;

        // ManaBurn_Timer
        if (m_uiManaBurnTimer < uiDiff)
        {
            if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0, SPELL_MANABURN, SELECT_FLAG_POWER_MANA))
            {
                if (DoCastSpellIfCan(pTarget, SPELL_MANABURN) == CAST_OK)
                    m_uiManaBurnTimer = 10000;
            }
        }
        else
            m_uiManaBurnTimer -= uiDiff;

        // PsychicScream_Timer
        if (m_uiPsychicScreamTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_PSYCHICSCREAM) == CAST_OK)
                m_uiPsychicScreamTimer = 30000;
        }
        else
            m_uiPsychicScreamTimer -= uiDiff;

        // ShadowShield_Timer
        if (m_uiShadowShieldTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_SHADOWSHIELD) == CAST_OK)
                m_uiShadowShieldTimer = 25000;
        }
        else
            m_uiShadowShieldTimer -= uiDiff;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_high_interrogator_gerstahn(Creature* pCreature)
{
    return new boss_high_interrogator_gerstahnAI(pCreature);
}

void AddSC_boss_high_interrogator_gerstahn()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "boss_high_interrogator_gerstahn";
    pNewScript->GetAI = &GetAI_boss_high_interrogator_gerstahn;
    pNewScript->RegisterSelf();
}
