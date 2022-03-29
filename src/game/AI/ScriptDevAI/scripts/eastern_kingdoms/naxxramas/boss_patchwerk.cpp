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

#include "AI/ScriptDevAI/base/CombatAI.h"
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
    SPELL_HATEFULSTRIKE         = 28308,
    SPELL_ENRAGE                = 28131,
    SPELL_BERSERK               = 26662,
    SPELL_SLIMEBOLT             = 32309
};

enum PatchwerkActions
{
    PATCHWERK_HATEFUL_STRIKE,
    PATCHWERK_BERSERK_HP_CHECK,
    PATCHWERK_BERSERK,
    PATCHWERK_BERSERK_SILMEBOLT,
    PATCHWER_ACTIONS_MAX,
};

struct boss_patchwerkAI : public CombatAI
{
    boss_patchwerkAI(Creature* creature) : CombatAI(creature, PATCHWER_ACTIONS_MAX), m_instance(static_cast<ScriptedInstance*>(creature->GetInstanceData()))
    {
        AddTimerlessCombatAction(PATCHWERK_BERSERK_HP_CHECK, true);         // Soft enrage à 5%
        AddCombatAction(PATCHWERK_HATEFUL_STRIKE, 1200u);
        AddCombatAction(PATCHWERK_BERSERK, 7u * MINUTE * IN_MILLISECONDS);  // Basic berserk
        AddCombatAction(PATCHWERK_BERSERK_SILMEBOLT, true);                 // Slimebolt - casted only 30 seconds after berserking to prevent kiting
    }

    ScriptedInstance* m_instance;

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

    void EnterEvadeMode() override
    {
        CombatAI::EnterEvadeMode();

        if (m_instance)
            m_instance->SetData(TYPE_PATCHWERK, FAIL);
    }

    void ExecuteAction(uint32 action) override
    {
        switch (action)
        {
            case PATCHWERK_BERSERK_HP_CHECK:
            {
                if (m_creature->GetHealthPercent() < 5.0f)
                {
                    if (DoCastSpellIfCan(m_creature, SPELL_ENRAGE) == CAST_OK)
                    {
                        DoScriptText(EMOTE_GENERIC_ENRAGED, m_creature);
                        DisableCombatAction(action);
                    }
                }
                break;
            }
            case PATCHWERK_HATEFUL_STRIKE:
            {
                if (DoCastSpellIfCan(m_creature, SPELL_HATEFULSTRIKE_PRIMER) == CAST_OK)
                    ResetCombatAction(action, 1.2 * IN_MILLISECONDS);
                break;
            }
            case PATCHWERK_BERSERK:
            {
                if (DoCastSpellIfCan(m_creature, SPELL_BERSERK) == CAST_OK)
                {
                    DoScriptText(EMOTE_GENERIC_BERSERK, m_creature);
                    DisableCombatAction(action);
                    ResetCombatAction(PATCHWERK_BERSERK_SILMEBOLT, 30 * IN_MILLISECONDS);
                }
                break;
            }
            case PATCHWERK_BERSERK_SILMEBOLT:
            {
                if (DoCastSpellIfCan(m_creature->GetVictim(), SPELL_SLIMEBOLT) == CAST_OK)
                    ResetCombatAction(PATCHWERK_BERSERK_SILMEBOLT, 1 * IN_MILLISECONDS);
                break;
            }
            default:
                break;
        }
    }
};

struct HatefulStrikePrimer : public SpellScript
{
    void OnEffectExecute(Spell* spell, SpellEffectIndex effIdx) const override
    {
        if (effIdx == EFFECT_INDEX_0)
        {
            if (Unit* caster = spell->GetCaster())
            {
                // Target is filtered in Spell::FilterTargetMap
                if (Unit* unitTarget = spell->GetUnitTarget())
                    caster->CastSpell(unitTarget, SPELL_HATEFULSTRIKE, TRIGGERED_NONE);
            }
        }
    }
};

void AddSC_boss_patchwerk()
{
    Script* newScript = new Script;
    newScript->Name = "boss_patchwerk";
    newScript->GetAI = &GetNewAIInstance<boss_patchwerkAI>;
    newScript->RegisterSelf();

    RegisterSpellScript<HatefulStrikePrimer>("spell_patchwerk_hatefulstrike");
}
