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
SDName: Boss_Patchwerk
SD%Complete: 100
SDComment:
SDCategory: Naxxramas
EndScriptData

*/

#include "AI/ScriptDevAI/include/sc_common.h"
#include "naxxramas.h"

enum
{
    SAY_AGGRO1                  = -1533017,
    SAY_AGGRO2                  = -1533018,
    SAY_SLAY                    = -1533019,
    SAY_DEATH                   = -1533020,

    EMOTE_GENERIC_BERSERK       = -1000004,
    EMOTE_GENERIC_ENRAGED       = -1000003,

    SPELL_HATEFULSTRIKE_PRIMER  = 28307,
    SPELL_ENRAGE                = 28131,
    SPELL_BERSERK               = 26662,
    SPELL_SLIMEBOLT             = 32309
};

struct boss_patchwerkAI : public ScriptedAI
{
    boss_patchwerkAI(Creature* creature) : ScriptedAI(creature)
    {
        m_instance = (instance_naxxramas*)creature->GetInstanceData();
        Reset();
    }

    instance_naxxramas* m_instance;

    uint32 m_hatefulStrikeTimer;
    uint32 m_berserkTimer;
    uint32 m_berserkSlimeBoltTimer;
    uint32 m_slimeboltTimer;
    bool   m_isEnraged;
    bool   m_isBerserk;

    void Reset() override
    {
        m_hatefulStrikeTimer = 1.2 * IN_MILLISECONDS;
        m_berserkTimer = 7 * MINUTE * IN_MILLISECONDS;                        // Basic berserk
        m_berserkSlimeBoltTimer = m_berserkTimer + 30 * IN_MILLISECONDS;    // Slime Bolt berserk
        m_slimeboltTimer = 10* IN_MILLISECONDS;
        m_isEnraged = false;
        m_isBerserk = false;
    }

    void KilledUnit(Unit* /*victim*/) override
    {
        if (urand(0, 4))
            return;

        DoScriptText(SAY_SLAY, m_creature);
    }

    void JustDied(Unit* /*killer*/) override
    {
        DoScriptText(SAY_DEATH, m_creature);

        if (m_instance)
            m_instance->SetData(TYPE_PATCHWERK, DONE);
    }

    void Aggro(Unit* /*who*/) override
    {
        DoScriptText(urand(0, 1) ? SAY_AGGRO1 : SAY_AGGRO2, m_creature);

        if (m_instance)
            m_instance->SetData(TYPE_PATCHWERK, IN_PROGRESS);
    }

    void JustReachedHome() override
    {
        if (m_instance)
            m_instance->SetData(TYPE_PATCHWERK, FAIL);
    }

    void UpdateAI(const uint32 diff) override
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->GetVictim())
            return;

        // Hateful Strike
        if (m_hatefulStrikeTimer < diff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_HATEFULSTRIKE_PRIMER) == CAST_OK)
                m_hatefulStrikeTimer = 1.2 * IN_MILLISECONDS;
        }
        else
            m_hatefulStrikeTimer -= diff;

        // Soft Enrage at 5%
        if (!m_isEnraged)
        {
            if (m_creature->GetHealthPercent() < 5.0f)
            {
                if (DoCastSpellIfCan(m_creature, SPELL_ENRAGE) == CAST_OK)
                {
                    DoScriptText(EMOTE_GENERIC_ENRAGED, m_creature);
                    m_isEnraged = true;
                }
            }
        }

        // Berserk after 7 minutes
        if (!m_isBerserk)
        {
            if (m_berserkTimer < diff)
            {
                if (DoCastSpellIfCan(m_creature, SPELL_BERSERK) == CAST_OK)
                {
                    DoScriptText(EMOTE_GENERIC_BERSERK, m_creature);
                    m_isBerserk = true;
                }
            }
            else
                m_berserkTimer -= diff;
        }
        else if (m_berserkSlimeBoltTimer < diff)
        {
            // Slimebolt - casted only 30 seconds after Berserking to prevent kiting
            if (m_slimeboltTimer < diff)
            {
                DoCastSpellIfCan(m_creature->GetVictim(), SPELL_SLIMEBOLT);
                m_slimeboltTimer = 1 * IN_MILLISECONDS;
            }
            else
                m_slimeboltTimer -= diff;
        }

        DoMeleeAttackIfReady();
    }
};

UnitAI* GetAI_boss_patchwerk(Creature* creature)
{
    return new boss_patchwerkAI(creature);
}

void AddSC_boss_patchwerk()
{
    Script* newScript = new Script;
    newScript->Name = "boss_patchwerk";
    newScript->GetAI = &GetAI_boss_patchwerk;
    newScript->RegisterSelf();
}
