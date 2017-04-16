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
SDName: Boss_General_Angerforge
SD%Complete: 100
SDComment:
SDCategory: Blackrock Depths
EndScriptData */

#include "precompiled.h"

enum
{
    EMOTE_ALARM                 = -1230035,

    SPELL_FLURRY                = 15088,
    SPELL_ENRAGE                = 15097,
    SPELL_SUNDER_ARMOR          = 15572,

    NPC_ANVILRAGE_MEDIC         = 8894,
    NPC_ANVILRAGE_RESERVIST     = 8901,

    NPC_ELITE_AMOUNT            = 2,
    NPC_NORMAL_AMOUNT           = 8,
};

static const float aAlarmPoint[4] = {717.343f, 22.116f, -45.4321f, 3.1415f};

struct boss_general_angerforgeAI : public ScriptedAI
{
    boss_general_angerforgeAI(Creature* pCreature) : ScriptedAI(pCreature) { Reset(); }

    uint32 m_uiSunderArmorTimer;
    uint32 m_uiAlarmTimer;

    void Reset() override
    {
        m_uiSunderArmorTimer = urand(5 * IN_MILLISECONDS, 10 * IN_MILLISECONDS);
        m_uiAlarmTimer = 0;
    }

    void Aggro(Unit* /*pWho*/) override
    {
        DoCastSpellIfCan(m_creature, SPELL_FLURRY, CAST_AURA_NOT_PRESENT | CAST_TRIGGERED);
        DoCastSpellIfCan(m_creature, SPELL_ENRAGE, CAST_AURA_NOT_PRESENT | CAST_TRIGGERED);
    }

    void SummonAdd(uint32 uiEntry)
    {
        float fX, fY, fZ;
        m_creature->GetRandomPoint(aAlarmPoint[0], aAlarmPoint[1], aAlarmPoint[2], 1.0f, fX, fY, fZ);
        m_creature->SummonCreature(uiEntry, fX, fY, fZ, aAlarmPoint[3], TEMPSUMMON_TIMED_OOC_OR_DEAD_DESPAWN, 30 * IN_MILLISECONDS);
    }

    void JustSummoned(Creature* pSummoned) override
    {
        if (m_creature->getVictim())
            pSummoned->AI()->AttackStart(m_creature->getVictim());
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        // Return since we have no target
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        // Sunder_Armor-Timer
        if (m_uiSunderArmorTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_SUNDER_ARMOR) == CAST_OK)
                m_uiSunderArmorTimer = urand(5 * IN_MILLISECONDS, 15 * IN_MILLISECONDS);
        }
        else
            m_uiSunderArmorTimer -= uiDiff;

        // Alarm-Timer
        if (m_creature->GetHealthPercent() < 30.0f)
        {
            if (m_uiAlarmTimer < uiDiff)
            {
                DoScriptText(EMOTE_ALARM, m_creature);

                for (uint8 i = 0; i < NPC_NORMAL_AMOUNT; i++)
                    SummonAdd(NPC_ANVILRAGE_RESERVIST);
                for (uint8 i = 0; i < NPC_ELITE_AMOUNT; i++)
                    SummonAdd(NPC_ANVILRAGE_MEDIC);

                m_uiAlarmTimer = 3 * MINUTE * IN_MILLISECONDS;
            }
            else
                m_uiAlarmTimer -= uiDiff;
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_general_angerforge(Creature* pCreature)
{
    return new boss_general_angerforgeAI(pCreature);
}

void AddSC_boss_general_angerforge()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "boss_general_angerforge";
    pNewScript->GetAI = &GetAI_boss_general_angerforge;
    pNewScript->RegisterSelf();
}
