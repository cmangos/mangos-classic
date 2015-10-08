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
EndScriptData */

#include "precompiled.h"
#include "naxxramas.h"

enum
{
    // EMOTE_AURA_BLOCKING   = -1533143,
    // EMOTE_AURA_WANE       = -1533144,
    // EMOTE_AURA_FADING     = -1533145,

    SPELL_CORRUPTED_MIND    = 29201,            // this triggers the following spells on targets (based on class): 29185, 29194, 29196, 29198
    SPELL_POISON_AURA       = 29865,
    SPELL_INEVITABLE_DOOM   = 29204,
    SPELL_SUMMON_SPORE      = 29234,
    SPELL_REMOVE_CURSE      = 30281,
    // SPELL_BERSERK         = 26662,

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
    // uint32 m_uiBerserkTimer;
    uint8 m_uiCorruptedMindCount;

    void Reset() override
    {
        m_uiPoisonAuraTimer = 5000;
        m_uiCorruptedMindTimer = 4000;
        m_uiRemoveCurseTimer = 2000;
        m_uiInevitableDoomTimer = MINUTE * 2 * IN_MILLISECONDS;
        m_uiSummonTimer = urand(10000, 15000);              // first seen in vid after approx 12s
        // m_uiBerserkTimer = MINUTE*12*IN_MILLISECONDS;    // not used
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
        if (pSummoned->GetEntry() != NPC_SPORE)
            return;

        if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
            pSummoned->AddThreat(pTarget);
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        /* Berserk - not used
        if (m_uiBerserkTimer < uiDiff)
        {
            DoCastSpellIfCan(m_creature, SPELL_BERSERK);
            m_uiBerserkTimer = 300000;
        }
        else
            m_uiBerserkTimer -= uiDiff;*/

        // Inevitable Doom
        if (m_uiInevitableDoomTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_INEVITABLE_DOOM) == CAST_OK)
                m_uiInevitableDoomTimer = (m_uiCorruptedMindCount <= 5) ? 30000 : 15000;
        }
        else
            m_uiInevitableDoomTimer -= uiDiff;

        // Corrupted Mind
        if (m_uiCorruptedMindTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_CORRUPTED_MIND) == CAST_OK)
            {
                ++m_uiCorruptedMindCount;
                m_uiCorruptedMindTimer = 60000;
            }
        }
        else
            m_uiCorruptedMindTimer -= uiDiff;

        // Summon
        if (m_uiSummonTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_SUMMON_SPORE) == CAST_OK)
                m_uiSummonTimer = 13000;
        }
        else
            m_uiSummonTimer -= uiDiff;

        // Poison Aura
        if (m_uiPoisonAuraTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_POISON_AURA) == CAST_OK)
                m_uiPoisonAuraTimer = 12000;
        }
        else
            m_uiPoisonAuraTimer -= uiDiff;

        // Remove Curse
        if (m_uiRemoveCurseTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_REMOVE_CURSE) == CAST_OK)
                m_uiRemoveCurseTimer = 30000;
        }
        else
            m_uiRemoveCurseTimer -= uiDiff;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_loatheb(Creature* pCreature)
{
    return new boss_loathebAI(pCreature);
}

void AddSC_boss_loatheb()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "boss_loatheb";
    pNewScript->GetAI = &GetAI_boss_loatheb;
    pNewScript->RegisterSelf();
}
