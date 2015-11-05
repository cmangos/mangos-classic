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
SDName: Boss_Moam
SD%Complete: 100
SDComment:
SDCategory: Ruins of Ahn'Qiraj
EndScriptData */

#include "precompiled.h"

enum
{
    EMOTE_AGGRO              = -1509000,
    EMOTE_MANA_FULL          = -1509001,
    EMOTE_ENERGIZING         = -1509028,

    SPELL_TRAMPLE            = 15550,
    SPELL_DRAIN_MANA         = 25671,
    SPELL_ARCANE_ERUPTION    = 25672,
    SPELL_SUMMON_MANAFIEND_1 = 25681,
    SPELL_SUMMON_MANAFIEND_2 = 25682,
    SPELL_SUMMON_MANAFIEND_3 = 25683,
    SPELL_ENERGIZE           = 25685,

    PHASE_ATTACKING          = 0,
    PHASE_ENERGIZING         = 1
};

struct boss_moamAI : public ScriptedAI
{
    boss_moamAI(Creature* pCreature) : ScriptedAI(pCreature) {Reset();}

    uint8 m_uiPhase;

    uint32 m_uiTrampleTimer;
    uint32 m_uiManaDrainTimer;
    uint32 m_uiCheckoutManaTimer;
    uint32 m_uiSummonManaFiendsTimer;

    void Reset() override
    {
        m_uiTrampleTimer            = 9000;
        m_uiManaDrainTimer          = 3000;
        m_uiSummonManaFiendsTimer   = 90000;
        m_uiCheckoutManaTimer       = 1500;
        m_uiPhase                   = PHASE_ATTACKING;
        m_creature->SetPower(POWER_MANA, 0);
        m_creature->SetMaxPower(POWER_MANA, 0);
    }

    void Aggro(Unit* /*pWho*/) override
    {
        DoScriptText(EMOTE_AGGRO, m_creature);
        m_creature->SetMaxPower(POWER_MANA, m_creature->GetCreatureInfo()->MaxLevelMana);
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        switch (m_uiPhase)
        {
            case PHASE_ATTACKING:
                if (m_uiCheckoutManaTimer <= uiDiff)
                {
                    m_uiCheckoutManaTimer = 1500;
                    if (m_creature->GetPower(POWER_MANA) * 100 / m_creature->GetMaxPower(POWER_MANA) > 75.0f)
                    {
                        DoCastSpellIfCan(m_creature, SPELL_ENERGIZE);
                        DoScriptText(EMOTE_ENERGIZING, m_creature);
                        m_uiPhase = PHASE_ENERGIZING;
                        return;
                    }
                }
                else
                    m_uiCheckoutManaTimer -= uiDiff;

                if (m_uiSummonManaFiendsTimer <= uiDiff)
                {
                    DoCastSpellIfCan(m_creature->getVictim(), SPELL_SUMMON_MANAFIEND_1, CAST_TRIGGERED);
                    DoCastSpellIfCan(m_creature->getVictim(), SPELL_SUMMON_MANAFIEND_2, CAST_TRIGGERED);
                    DoCastSpellIfCan(m_creature->getVictim(), SPELL_SUMMON_MANAFIEND_3, CAST_TRIGGERED);
                    m_uiSummonManaFiendsTimer = 90000;
                }
                else
                    m_uiSummonManaFiendsTimer -= uiDiff;

                if (m_uiManaDrainTimer <= uiDiff)
                {
                    if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0, SPELL_DRAIN_MANA, SELECT_FLAG_POWER_MANA))
                    {
                        if (DoCastSpellIfCan(pTarget, SPELL_DRAIN_MANA) == CAST_OK)
                            m_uiManaDrainTimer = urand(2000, 6000);
                    }
                }
                else
                    m_uiManaDrainTimer -= uiDiff;

                if (m_uiTrampleTimer <= uiDiff)
                {
                    DoCastSpellIfCan(m_creature->getVictim(), SPELL_TRAMPLE);
                    m_uiTrampleTimer = 15000;
                }
                else
                    m_uiTrampleTimer -= uiDiff;

                DoMeleeAttackIfReady();
                break;
            case PHASE_ENERGIZING:
                if (m_uiCheckoutManaTimer <= uiDiff)
                {
                    m_uiCheckoutManaTimer = 1500;
                    if (m_creature->GetPower(POWER_MANA) == m_creature->GetMaxPower(POWER_MANA))
                    {
                        m_creature->RemoveAurasDueToSpell(SPELL_ENERGIZE);
                        DoCastSpellIfCan(m_creature, SPELL_ARCANE_ERUPTION);
                        DoScriptText(EMOTE_MANA_FULL, m_creature);
                        m_uiPhase = PHASE_ATTACKING;
                        return;
                    }
                }
                else
                    m_uiCheckoutManaTimer -= uiDiff;
                break;
        }
    }
};

CreatureAI* GetAI_boss_moam(Creature* pCreature)
{
    return new boss_moamAI(pCreature);
}

void AddSC_boss_moam()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "boss_moam";
    pNewScript->GetAI = &GetAI_boss_moam;
    pNewScript->RegisterSelf();
}
