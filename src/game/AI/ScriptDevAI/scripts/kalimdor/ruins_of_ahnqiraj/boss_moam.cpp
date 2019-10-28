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
EndScriptData

*/

#include "AI/ScriptDevAI/include/precompiled.h"
#include "AI/ScriptDevAI/base/CombatAI.h"

enum
{
    EMOTE_AGGRO              = -1509000,
    EMOTE_MANA_FULL          = -1509001,
    EMOTE_ENERGIZING         = -1509028,

    SPELL_TRAMPLE            = 15550,
    SPELL_DRAIN_MANA         = 25676,
    SPELL_ARCANE_ERUPTION    = 25672,
    SPELL_SUMMON_MANAFIENDS  = 25684,
    SPELL_ENERGIZE           = 25685,

    NPC_MANA_FIEND           = 15527,
};

enum MoamActions
{
    MOAM_ARCANE_ERUPTION,
    MOAM_TRAMPLE,
    MOAM_MANA_DRAIN,
    MOAM_MANA_FIENDS,
    MOAM_ACTION_MAX,
};

struct boss_moamAI : public CombatAI
{
    boss_moamAI(Creature* creature) : CombatAI(creature, MOAM_ACTION_MAX)
    {
        AddCombatAction(MOAM_ARCANE_ERUPTION, 0u);
        AddCombatAction(MOAM_TRAMPLE, 9000u);
        AddCombatAction(MOAM_MANA_DRAIN, 6000u);
        AddCombatAction(MOAM_MANA_FIENDS, 90000u);
    }

    uint8 m_uiPhase;

    void Reset() override
    {
        CombatAI::Reset();
        m_creature->SetPower(POWER_MANA, 0);
    }

    void Aggro(Unit* /*who*/) override
    {
        DoScriptText(EMOTE_AGGRO, m_creature);
    }

    void JustSummoned(Creature* summoned) override
    {
        if (summoned->GetEntry() == NPC_MANA_FIEND)
        {
            summoned->SetInCombatWithZone();
            if (Unit* target = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0, nullptr, SELECT_FLAG_PLAYER))
            {
                summoned->AddThreat(target, 100000.f);
                summoned->AI()->AttackStart(target);
            }
        }
    }

    void ExecuteAction(uint32 action) override
    {
        switch (action)
        {
            case MOAM_ARCANE_ERUPTION:
            {
                if (DoCastSpellIfCan(nullptr, SPELL_ARCANE_ERUPTION) == CAST_OK)
                {
                    DoScriptText(EMOTE_MANA_FULL, m_creature);
                    ResetCombatAction(action, 5000); // small CD to prevent AI spam
                }
                break;
            }
            case MOAM_TRAMPLE:
            {
                if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_TRAMPLE) == CAST_OK)
                    ResetCombatAction(action, 15000);
                break;
            }
            case MOAM_MANA_DRAIN:
            {
                if (DoCastSpellIfCan(nullptr, SPELL_DRAIN_MANA) == CAST_OK)
                    ResetCombatAction(action, 6000);
                break;
            }
            case MOAM_MANA_FIENDS:
            {
                if (DoCastSpellIfCan(nullptr, SPELL_SUMMON_MANAFIENDS) == CAST_OK)
                {
                    DoCastSpellIfCan(m_creature, SPELL_ENERGIZE);
                    DoScriptText(EMOTE_ENERGIZING, m_creature);
                    ResetCombatAction(action, 90000);
                }
                break;
            }
        }
    }
};

UnitAI* GetAI_boss_moam(Creature* creature)
{
    return new boss_moamAI(creature);
}

void AddSC_boss_moam()
{
    Script* pNewScript = new Script;
    pNewScript->Name = "boss_moam";
    pNewScript->GetAI = &GetAI_boss_moam;
    pNewScript->RegisterSelf();
}
