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
SD%Complete: 95
SDComment: 'Summon Player' missing
SDCategory: Molten Core
EndScriptData

*/

#include "AI/ScriptDevAI/include/precompiled.h"
#include "molten_core.h"

enum
{
    EMOTE_MASSIVE_ERUPTION      = -1409025,

    // Garr spells
    SPELL_ANTIMAGICPULSE        = 19492,
    SPELL_MAGMASHACKLES         = 19496,
    SPELL_ENRAGE                = 19516,
    SPELL_SUMMON_PLAYER         = 20477,
    SPELL_ERUPTION_TRIGGER      = 20482,
    SPELL_SEPARATION_ANXIETY    = 23487,                    // Aura cast on himself by Garr, if adds move out of range, they will cast spell 23492 on themselves

    // Firesworn spells
    SPELL_ERUPTION              = 19497,
    SPELL_ENRAGE_TRIGGER        = 19515,
    SPELL_MASSIVE_ERUPTION      = 20483,
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
    uint32 m_uiMassiveEruptionTimer;

    void Reset() override
    {
        m_uiAntiMagicPulseTimer = urand(10 * IN_MILLISECONDS, 15 * IN_MILLISECONDS);
        m_uiMagmaShacklesTimer = urand(5 * IN_MILLISECONDS, 10 * IN_MILLISECONDS);
        m_uiMassiveEruptionTimer = 6 * MINUTE * IN_MILLISECONDS;
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

    void SpellHit(Unit* /*pCaster*/, const SpellEntry* pSpell) override
    {
        if (pSpell->Id == SPELL_ENRAGE_TRIGGER)
            DoCastSpellIfCan(m_creature, SPELL_ENRAGE, CAST_TRIGGERED);
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        // AntiMagicPulse_Timer
        if (m_uiAntiMagicPulseTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_ANTIMAGICPULSE) == CAST_OK)
                m_uiAntiMagicPulseTimer = urand(15 * IN_MILLISECONDS, 20 * IN_MILLISECONDS);
        }
        else
            m_uiAntiMagicPulseTimer -= uiDiff;

        // MagmaShackles_Timer
        if (m_uiMagmaShacklesTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_MAGMASHACKLES) == CAST_OK)
                m_uiMagmaShacklesTimer = urand(10 * IN_MILLISECONDS, 15 * IN_MILLISECONDS);
        }
        else
            m_uiMagmaShacklesTimer -= uiDiff;

        // MassiveEruption_Timer
        if (m_uiMassiveEruptionTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_ERUPTION_TRIGGER) == CAST_OK)
            {
                DoScriptText(EMOTE_MASSIVE_ERUPTION, m_creature);
                m_uiMassiveEruptionTimer = 20 * IN_MILLISECONDS;
            }
        }
        else
            m_uiMassiveEruptionTimer -= uiDiff;

        DoMeleeAttackIfReady();
    }
};

struct mob_fireswornAI : public ScriptedAI
{
    mob_fireswornAI(Creature* pCreature) : ScriptedAI(pCreature) {}

    void Reset() override {}

    void JustDied(Unit* pKiller) override
    {
        if (m_creature != pKiller)
            DoCastSpellIfCan(m_creature, SPELL_ERUPTION);

        DoCastSpellIfCan(m_creature, SPELL_ENRAGE_TRIGGER, CAST_TRIGGERED);
    }

    void SpellHit(Unit* /*pCaster*/, const SpellEntry* pSpell) override
    {
        if (pSpell->Id == SPELL_ERUPTION_TRIGGER)
            DoCastSpellIfCan(m_creature, SPELL_MASSIVE_ERUPTION);
    }

    void UpdateAI(const uint32 /*uiDiff*/) override
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        DoMeleeAttackIfReady();
    }
};

UnitAI* GetAI_boss_garr(Creature* pCreature)
{
    return new boss_garrAI(pCreature);
}

UnitAI* GetAI_mob_firesworn(Creature* pCreature)
{
    return new mob_fireswornAI(pCreature);
}

void AddSC_boss_garr()
{
    Script* pNewScript = new Script;
    pNewScript->Name = "boss_garr";
    pNewScript->GetAI = &GetAI_boss_garr;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "mob_firesworn";
    pNewScript->GetAI = &GetAI_mob_firesworn;
    pNewScript->RegisterSelf();
}
