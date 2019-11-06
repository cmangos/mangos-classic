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
    boss_loathebAI(Creature* creature) : ScriptedAI(creature)
    {
        m_instance = (instance_naxxramas*)creature->GetInstanceData();
        Reset();
    }

    instance_naxxramas* m_instance;

    uint32 m_poisonAuraTimer;
    uint32 m_corruptedMindTimer;
    uint32 m_inevitableDoomTimer;
    uint32 m_removeCurseTimer;
    uint32 m_summonTimer;
    uint8 m_corruptedMindCount;

    void Reset() override
    {
        m_poisonAuraTimer = 5 * IN_MILLISECONDS;
        m_corruptedMindTimer = 4 * IN_MILLISECONDS;
        m_removeCurseTimer = 2 * IN_MILLISECONDS;
        m_inevitableDoomTimer = 2 * MINUTE * IN_MILLISECONDS;
        m_summonTimer = 12 * IN_MILLISECONDS;
        m_corruptedMindCount = 0;
    }

    void Aggro(Unit* /*who*/) override
    {
        if (m_instance)
            m_instance->SetData(TYPE_LOATHEB, IN_PROGRESS);
    }

    void JustDied(Unit* /*killer*/) override
    {
        if (m_instance)
            m_instance->SetData(TYPE_LOATHEB, DONE);
    }

    void JustReachedHome() override
    {
        if (m_instance)
            m_instance->SetData(TYPE_LOATHEB, NOT_STARTED);
    }

    void JustSummoned(Creature* summoned) override
    {
        if (summoned->GetEntry() == NPC_SPORE)
            summoned->SetInCombatWithZone();
    }

    void UpdateAI(const uint32 diff) override
    {
        // Do nothing if no target
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        // Inevitable Doom
        if (m_inevitableDoomTimer < diff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_INEVITABLE_DOOM, CAST_TRIGGERED) == CAST_OK)
                m_inevitableDoomTimer = ((m_corruptedMindCount <= 5) ? 30 : 15) * IN_MILLISECONDS;
        }
        else
            m_inevitableDoomTimer -= diff;

        // Corrupted Mind
        if (m_corruptedMindTimer < diff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_CORRUPTED_MIND, CAST_TRIGGERED) == CAST_OK)
            {
                ++m_corruptedMindCount;
                m_corruptedMindTimer = 60 * IN_MILLISECONDS;
            }
        }
        else
            m_corruptedMindTimer -= diff;

        // Summon
        if (m_summonTimer < diff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_SUMMON_SPORE, CAST_TRIGGERED) == CAST_OK)
                m_summonTimer = 12 * IN_MILLISECONDS;
        }
        else
            m_summonTimer -= diff;

        // Poison Aura
        if (m_poisonAuraTimer < diff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_POISON_AURA) == CAST_OK)
                m_poisonAuraTimer = 12 * IN_MILLISECONDS;
        }
        else
            m_poisonAuraTimer -= diff;

        // Remove Curse
        if (m_removeCurseTimer < diff)
        {
            SpellCastResult decurseResult = m_creature->CastSpell(m_creature, SPELL_REMOVE_CURSE, TRIGGERED_OLD_TRIGGERED);
            if (decurseResult == SPELL_CAST_OK || decurseResult == SPELL_FAILED_NOTHING_TO_DISPEL)  // Don't throw an error if there is nothing to dispel
                m_removeCurseTimer = 30 * IN_MILLISECONDS;
        }
        else
            m_removeCurseTimer -= diff;

        DoMeleeAttackIfReady();
    }
};

UnitAI* GetAI_boss_loatheb(Creature* creature)
{
    return new boss_loathebAI(creature);
}

void AddSC_boss_loatheb()
{
    Script* newScript = new Script;
    newScript->Name = "boss_loatheb";
    newScript->GetAI = &GetAI_boss_loatheb;
    newScript->RegisterSelf();
}
