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
SDName: Boss_Loatheb
SD%Complete: 100
SDComment:
SDCategory: Naxxramas
EndScriptData

*/

#include "AI/ScriptDevAI/include/precompiled.h"
#include "naxxramas.h"

enum
{
    SPELL_CORRUPTED_MIND    = 29201,            // this triggers the following spells on targets (based on class): 29185, 29194, 29196, 29198
    SPELL_POISON_AURA       = 29865,
    SPELL_INEVITABLE_DOOM   = 29204,
    SPELL_SUMMON_SPORE      = 29234,
    SPELL_REMOVE_CURSE      = 30281,

    NPC_SPORE               = 16286
};

struct boss_loathebAI : public ScriptedAI
{
    boss_loathebAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = (instance_naxxramas*)pCreature->GetInstanceData();
        Reset();
    }

    instance_naxxramas* m_pInstance;

    uint32 m_uiPoisonAuraTimer;
    uint32 m_uiCorruptedMindTimer;
    uint32 m_uiInevitableDoomTimer;
    uint32 m_uiRemoveCurseTimer;
    uint32 m_uiSummonTimer;
    uint8 m_uiCorruptedMindCount;

    void Reset() override
    {
        m_uiPoisonAuraTimer = 5 * IN_MILLISECONDS;
        m_uiCorruptedMindTimer = 4 * IN_MILLISECONDS;
        m_uiRemoveCurseTimer = 2 * IN_MILLISECONDS;
        m_uiInevitableDoomTimer = 2 * MINUTE * IN_MILLISECONDS;
        m_uiSummonTimer = 12 * IN_MILLISECONDS;
        m_uiCorruptedMindCount = 0;
    }

    void Aggro(Unit* /*pWho*/) override
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_LOATHEB, IN_PROGRESS);
    }

    void JustDied(Unit* /*pKiller*/) override
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_LOATHEB, DONE);
    }

    void JustReachedHome() override
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_LOATHEB, NOT_STARTED);
    }

    void JustSummoned(Creature* pSummoned) override
    {
        if (pSummoned->GetEntry() == NPC_SPORE)
            pSummoned->SetInCombatWithZone();
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        // Do nothing if no target
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        // Inevitable Doom
        if (m_uiInevitableDoomTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_INEVITABLE_DOOM, CAST_TRIGGERED) == CAST_OK)
                m_uiInevitableDoomTimer = ((m_uiCorruptedMindCount <= 5) ? 30 : 15) * IN_MILLISECONDS;
        }
        else
            m_uiInevitableDoomTimer -= uiDiff;

        // Corrupted Mind
        if (m_uiCorruptedMindTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_CORRUPTED_MIND, CAST_TRIGGERED) == CAST_OK)
            {
                ++m_uiCorruptedMindCount;
                m_uiCorruptedMindTimer = 60 * IN_MILLISECONDS;
            }
        }
        else
            m_uiCorruptedMindTimer -= uiDiff;

        // Summon
        if (m_uiSummonTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_SUMMON_SPORE, CAST_TRIGGERED) == CAST_OK)
                m_uiSummonTimer = 12 * IN_MILLISECONDS;
        }
        else
            m_uiSummonTimer -= uiDiff;

        // Poison Aura
        if (m_uiPoisonAuraTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_POISON_AURA) == CAST_OK)
                m_uiPoisonAuraTimer = 12 * IN_MILLISECONDS;
        }
        else
            m_uiPoisonAuraTimer -= uiDiff;

        // Remove Curse
        if (m_uiRemoveCurseTimer < uiDiff)
        {
            SpellCastResult decurseResult = m_creature->CastSpell(m_creature, SPELL_REMOVE_CURSE, TRIGGERED_OLD_TRIGGERED);
            if (decurseResult == SPELL_CAST_OK || decurseResult == SPELL_FAILED_NOTHING_TO_DISPEL)  // Don't throw an error if there is nothing to dispel
                m_uiRemoveCurseTimer = 30 * IN_MILLISECONDS;
        }
        else
            m_uiRemoveCurseTimer -= uiDiff;

        DoMeleeAttackIfReady();
    }
};

UnitAI* GetAI_boss_loatheb(Creature* pCreature)
{
    return new boss_loathebAI(pCreature);
}

void AddSC_boss_loatheb()
{
    Script* pNewScript = new Script;
    pNewScript->Name = "boss_loatheb";
    pNewScript->GetAI = &GetAI_boss_loatheb;
    pNewScript->RegisterSelf();
}
