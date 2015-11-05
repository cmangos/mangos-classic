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
SDName: scholomance.cpp
SD%Complete: 100
SDComment: Creature support for 10498
SDCategory: Scholomance
EndScriptData */

/* ContentData
npc_spectral_tutor
EndContentData */

#include "precompiled.h"

/*######
## npc_spectral_tutor
######*/

enum
{
    SPELL_IMAGE_PROJECTION          = 17651,
    SPELL_IMAGE_PROJECTION_HEAL     = 17652,
    SPELL_IMAGE_PROJECTION_SUMMON   = 17653,
    SPELL_MANA_BURN                 = 17630,
    SPELL_SILENCE                   = 12528,
};

struct npc_spectral_tutorAI : public ScriptedAI
{
    npc_spectral_tutorAI(Creature* pCreature) : ScriptedAI(pCreature) { Reset(); }

    uint32 m_uiManaBurnTimer;
    uint32 m_uiSilenceTimer;
    uint32 m_uiProjectionTimer;
    uint32 m_uiProjEndTimer;

    void Reset() override
    {
        m_uiProjEndTimer    = 0;
        m_uiManaBurnTimer   = urand(4000, 19000);
        m_uiSilenceTimer    = urand(0, 3000);
        m_uiProjectionTimer = urand(12000, 13000);
    }

    void EnterEvadeMode() override
    {
        if (m_uiProjEndTimer)
            return;

        ScriptedAI::EnterEvadeMode();
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        if (m_uiProjEndTimer)
        {
            if (m_uiProjEndTimer <= uiDiff)
            {
                if (DoCastSpellIfCan(m_creature, SPELL_IMAGE_PROJECTION_HEAL) == CAST_OK)
                    m_uiProjEndTimer = 0;
            }
            else
                m_uiProjEndTimer -= uiDiff;

            // no other actions during Image Projection
            return;
        }

        if (m_uiManaBurnTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_MANA_BURN) == CAST_OK)
                m_uiManaBurnTimer = urand(9000, 26000);
        }
        else
            m_uiManaBurnTimer -= uiDiff;

        if (m_uiSilenceTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_SILENCE) == CAST_OK)
                m_uiSilenceTimer = urand(12000, 26000);
        }
        else
            m_uiSilenceTimer -= uiDiff;

        if (m_uiProjectionTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_IMAGE_PROJECTION) == CAST_OK)
            {
                DoCastSpellIfCan(m_creature, SPELL_IMAGE_PROJECTION_SUMMON, CAST_TRIGGERED);
                m_uiProjEndTimer = 1000;
                m_uiProjectionTimer = urand(18000, 25000);
            }
        }
        else
            m_uiProjectionTimer -= uiDiff;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_spectral_tutor(Creature* pCreature)
{
    return new npc_spectral_tutorAI(pCreature);
}

void AddSC_scholomance()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "npc_spectral_tutor";
    pNewScript->GetAI = &GetAI_npc_spectral_tutor;
    pNewScript->RegisterSelf();
}
