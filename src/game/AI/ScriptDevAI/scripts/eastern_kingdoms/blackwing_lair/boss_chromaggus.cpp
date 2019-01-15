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
SDName: Boss_Chromaggus
SD%Complete: 100
SDComment:
SDCategory: Blackwing Lair
EndScriptData

*/

#include "AI/ScriptDevAI/include/precompiled.h"
#include "blackwing_lair.h"

enum
{
    EMOTE_GENERIC_FRENZY_KILL   = -1000001,
    EMOTE_GENERIC_FRENZY        = -1000002,
    EMOTE_SHIMMER               = -1469003,

    SPELL_BREATH_SELECTION      = 23195,
    SPELL_BROOD_AFFLICTION      = 23173,

    SPELL_ELEMENTAL_SHIELD      = 22276,
    SPELL_FRENZY                = 23128,
    SPELL_ENRAGE                = 23537
};

struct boss_chromaggusAI : public ScriptedAI
{
    boss_chromaggusAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        // Select the 2 different breaths that we are going to use until despawned
        DoCastSpellIfCan(m_creature, SPELL_BREATH_SELECTION);

        m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
        Reset();
    }

    ScriptedInstance* m_pInstance;

    uint32 m_uiBreathLeftSpell;
    uint32 m_uiBreathRightSpell;

    uint32 m_uiShimmerTimer;
    uint32 m_uiBreathLeftTimer;
    uint32 m_uiBreathRightTimer;
    uint32 m_uiAfflictionTimer;
    uint32 m_uiFrenzyTimer;
    bool m_bEnraged;

    void Reset() override
    {
        m_uiShimmerTimer    = 0;                                                // Time till we change vurlnerabilites

        if (m_pInstance)
        {
            m_uiBreathLeftSpell = m_pInstance->GetData(TYPE_CHROMA_LBREATH);    // Spell for left breath, stored in instance data
            m_uiBreathRightSpell = m_pInstance->GetData(TYPE_CHROMA_RBREATH);   // Spell for right breath, stored in instance data
            m_uiBreathLeftTimer  = 30000;                                       // Left breath is 30 seconds
            m_uiBreathRightTimer  = 60000;                                      // Right is 1 minute so that we can alternate
        }
        m_uiAfflictionTimer = 7 * IN_MILLISECONDS;
        m_uiFrenzyTimer     = 15000;
        m_bEnraged          = false;
    }

    void Aggro(Unit* /*pWho*/) override
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_CHROMAGGUS, IN_PROGRESS);
    }

    void JustDied(Unit* /*pKiller*/) override
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_CHROMAGGUS, DONE);
    }

    void JustReachedHome() override
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_CHROMAGGUS, FAIL);
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        // Shimmer Timer Timer
        if (m_uiShimmerTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_ELEMENTAL_SHIELD) == CAST_OK)
            {
                DoScriptText(EMOTE_SHIMMER, m_creature);
                m_uiShimmerTimer = 45000;
            }
        }
        else
            m_uiShimmerTimer -= uiDiff;

        // Left Breath Timer
        if (m_uiBreathLeftTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, m_uiBreathLeftSpell) == CAST_OK)
                m_uiBreathLeftTimer = 60000;
        }
        else
            m_uiBreathLeftTimer -= uiDiff;

        // Right Breath Timer
        if (m_uiBreathRightTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, m_uiBreathRightSpell) == CAST_OK)
                m_uiBreathRightTimer = 60000;
        }
        else
            m_uiBreathRightTimer -= uiDiff;

        // Affliction Timer
        if (m_uiAfflictionTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_BROOD_AFFLICTION) == CAST_OK)
                m_uiAfflictionTimer = 7 * IN_MILLISECONDS;
        }
        else
            m_uiAfflictionTimer -= uiDiff;

        // Frenzy Timer
        if (m_uiFrenzyTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_FRENZY) == CAST_OK)
            {
                DoScriptText(EMOTE_GENERIC_FRENZY_KILL, m_creature);
                m_uiFrenzyTimer = 15 * IN_MILLISECONDS;
            }
        }
        else
            m_uiFrenzyTimer -= uiDiff;

        // Enrage if not already enraged and below 20%
        if (!m_bEnraged && m_creature->GetHealthPercent() < 20.0f)
        {
            DoCastSpellIfCan(m_creature, SPELL_ENRAGE);
            DoScriptText(EMOTE_GENERIC_FRENZY, m_creature);
            m_bEnraged = true;
        }

        DoMeleeAttackIfReady();
    }
};

UnitAI* GetAI_boss_chromaggus(Creature* pCreature)
{
    return new boss_chromaggusAI(pCreature);
}

void AddSC_boss_chromaggus()
{
    Script* pNewScript = new Script;
    pNewScript->Name = "boss_chromaggus";
    pNewScript->GetAI = &GetAI_boss_chromaggus;
    pNewScript->RegisterSelf();
}
