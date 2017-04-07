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
SDName: Boss_Garr
SD%Complete: 80
SDComment: Firesworn erruption needs to be revisited
SDCategory: Molten Core
EndScriptData */

#include "precompiled.h"
#include "molten_core.h"

enum
{
    // Garr spells
    SPELL_ANTIMAGICPULSE        = 19492,
    SPELL_MAGMASHACKLES         = 19496,
    SPELL_ENRAGE                = 19516,
    SPELL_SEPARATION_ANXIETY    = 23487,                    // Aura cast on himself by Garr, if adds move out of range, they will cast spell 23492 on themselves

    // Add spells
    SPELL_THRASH                = 8876,
    SPELL_IMMOLATE              = 15733,
    SPELL_ERUPTION              = 19497,
    SPELL_MASSIVE_ERUPTION      = 20483,                    // TODO possible on death
};

struct boss_garrAI : public ScriptedAI
{
    boss_garrAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
        Reset();
    }

    ScriptedInstance* m_pInstance;

    uint32 m_uiAntiMagicPulseTimer;
    uint32 m_uiMagmaShacklesTimer;

    void Reset() override
    {
        m_uiAntiMagicPulseTimer = 25000;
        m_uiMagmaShacklesTimer = 15000;
    }

    void Aggro(Unit* /*pWho*/) override
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_GARR, IN_PROGRESS);
        // Garr has a 100 yard aura to keep track of the distance of each of his adds, they will enrage if moved out of it
        DoCastSpellIfCan(m_creature, SPELL_SEPARATION_ANXIETY, CAST_TRIGGERED | CAST_AURA_NOT_PRESENT);
    }

    void JustDied(Unit* /*pKiller*/) override
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_GARR, DONE);
    }

    void JustReachedHome() override
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_GARR, FAIL);
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        // AntiMagicPulse_Timer
        if (m_uiAntiMagicPulseTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_ANTIMAGICPULSE) == CAST_OK)
                m_uiAntiMagicPulseTimer = urand(10000, 15000);
        }
        else
            m_uiAntiMagicPulseTimer -= uiDiff;

        // MagmaShackles_Timer
        if (m_uiMagmaShacklesTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_MAGMASHACKLES) == CAST_OK)
                m_uiMagmaShacklesTimer = urand(8000, 12000);
        }
        else
            m_uiMagmaShacklesTimer -= uiDiff;

        DoMeleeAttackIfReady();
    }
};

struct mob_fireswornAI : public ScriptedAI
{
    mob_fireswornAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
        Reset();

        DoCastSpellIfCan(m_creature, SPELL_THRASH, CAST_TRIGGERED | CAST_AURA_NOT_PRESENT);
        DoCastSpellIfCan(m_creature, SPELL_IMMOLATE, CAST_TRIGGERED | CAST_AURA_NOT_PRESENT);
    }

    ScriptedInstance* m_pInstance;

    void Reset() override {}

    void JustDied(Unit* /*pKiller*/) override
    {
        if (m_pInstance)
        {
            if (Creature* pGarr = m_pInstance->GetSingleCreatureFromStorage(NPC_GARR))
                pGarr->CastSpell(pGarr, SPELL_ENRAGE, TRIGGERED_OLD_TRIGGERED, nullptr, nullptr, m_creature->GetObjectGuid());
        }
    }
    void JustReachedHome() override
    {
        DoCastSpellIfCan(m_creature, SPELL_THRASH, CAST_TRIGGERED | CAST_AURA_NOT_PRESENT);
        DoCastSpellIfCan(m_creature, SPELL_IMMOLATE, CAST_TRIGGERED | CAST_AURA_NOT_PRESENT);
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        // Cast Erruption and let them die
        if (m_creature->GetHealthPercent() <= 10.0f)
        {
            DoCastSpellIfCan(m_creature->getVictim(), SPELL_ERUPTION);
            m_creature->SetDeathState(JUST_DIED);
            m_creature->RemoveCorpse();
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_garr(Creature* pCreature)
{
    return new boss_garrAI(pCreature);
}

CreatureAI* GetAI_mob_firesworn(Creature* pCreature)
{
    return new mob_fireswornAI(pCreature);
}

void AddSC_boss_garr()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "boss_garr";
    pNewScript->GetAI = &GetAI_boss_garr;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "mob_firesworn";
    pNewScript->GetAI = &GetAI_mob_firesworn;
    pNewScript->RegisterSelf();
}
