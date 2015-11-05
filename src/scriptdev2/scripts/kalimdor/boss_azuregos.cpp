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
SDName: Boss_Azuregos
SD%Complete: 90
SDComment: Spell reflect not effecting dots (Core problem)
SDCategory: Azshara
EndScriptData */

#include "precompiled.h"

enum
{
    SAY_TELEPORT                = -1000100,

    SPELL_ARCANE_VACUUM         = 21147,
    SPELL_MARK_OF_FROST_PLAYER  = 23182,
    SPELL_MARK_OF_FROST_AURA    = 23184,                    // Triggers 23186 on players that have 23182; unfortunatelly 23183 is missing from dbc
    SPELL_MANA_STORM            = 21097,
    SPELL_CHILL                 = 21098,
    SPELL_FROST_BREATH          = 21099,
    SPELL_REFLECT               = 22067,
    SPELL_CLEAVE                = 19983,                    // Was 8255; this one is from wowhead and seems to be the correct one
    SPELL_ENRAGE                = 23537,
};

struct boss_azuregosAI : public ScriptedAI
{
    boss_azuregosAI(Creature* pCreature) : ScriptedAI(pCreature) {Reset();}

    uint32 m_uiManaStormTimer;
    uint32 m_uiChillTimer;
    uint32 m_uiBreathTimer;
    uint32 m_uiTeleportTimer;
    uint32 m_uiReflectTimer;
    uint32 m_uiCleaveTimer;
    bool m_bEnraged;

    void Reset() override
    {
        m_uiManaStormTimer  = urand(5000, 17000);
        m_uiChillTimer      = urand(10000, 30000);
        m_uiBreathTimer     = urand(2000, 8000);
        m_uiTeleportTimer   = 30000;
        m_uiReflectTimer    = urand(15000, 30000);
        m_uiCleaveTimer     = 7000;
        m_bEnraged          = false;
    }

    void KilledUnit(Unit* pVictim) override
    {
        // Mark killed players with Mark of Frost
        if (pVictim->GetTypeId() == TYPEID_PLAYER)
            pVictim->CastSpell(pVictim, SPELL_MARK_OF_FROST_PLAYER, true, nullptr, nullptr, m_creature->GetObjectGuid());
    }

    void Aggro(Unit* /*pWho*/) override
    {
        // Boss aura which triggers the stun effect on dead players who resurrect
        DoCastSpellIfCan(m_creature, SPELL_MARK_OF_FROST_AURA);
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        // Return since we have no target
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        if (m_uiTeleportTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_ARCANE_VACUUM) == CAST_OK)
            {
                DoScriptText(SAY_TELEPORT, m_creature);
                m_uiTeleportTimer = 30000;
            }
        }
        else
            m_uiTeleportTimer -= uiDiff;

        // Chill Timer
        if (m_uiChillTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_CHILL) == CAST_OK)
                m_uiChillTimer = urand(13000, 25000);
        }
        else
            m_uiChillTimer -= uiDiff;

        // Breath Timer
        if (m_uiBreathTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_FROST_BREATH) == CAST_OK)
                m_uiBreathTimer = urand(10000, 15000);
        }
        else
            m_uiBreathTimer -= uiDiff;

        // Mana Storm Timer
        if (m_uiManaStormTimer < uiDiff)
        {
            if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
            {
                if (DoCastSpellIfCan(pTarget, SPELL_MANA_STORM) == CAST_OK)
                    m_uiManaStormTimer = urand(7500, 12500);
            }
        }
        else
            m_uiManaStormTimer -= uiDiff;

        // Reflect Timer
        if (m_uiReflectTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_REFLECT) == CAST_OK)
                m_uiReflectTimer = urand(20000, 35000);
        }
        else
            m_uiReflectTimer -= uiDiff;

        // Cleave Timer
        if (m_uiCleaveTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_CLEAVE) == CAST_OK)
                m_uiCleaveTimer = 7000;
        }
        else
            m_uiCleaveTimer -= uiDiff;

        // EnrageTimer
        if (!m_bEnraged && m_creature->GetHealthPercent() < 26.0f)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_ENRAGE) == CAST_OK)
                m_bEnraged = true;
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_azuregos(Creature* pCreature)
{
    return new boss_azuregosAI(pCreature);
}

void AddSC_boss_azuregos()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "boss_azuregos";
    pNewScript->GetAI = &GetAI_boss_azuregos;
    pNewScript->RegisterSelf();
}
