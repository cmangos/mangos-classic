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

#include "AI/ScriptDevAI/base/CombatAI.h"
#include "AI/ScriptDevAI/include/sc_common.h"
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

enum LoathebActions
{
    LOATHEB_POISON_AURA,
    LOATHEB_CORRUPTED_MIND,
    LOATHEB_INEVITABLE_DOOM,
    LOATHEB_REMOVE_CURSE,
    LOATHEB_SUMMON,
    LOATHEB_ACTION_MAX,
};

struct boss_loathebAI : public CombatAI
{
    boss_loathebAI(Creature* creature) : CombatAI(creature, LOATHEB_ACTION_MAX), m_instance(static_cast<ScriptedInstance*>(creature->GetInstanceData()))
    {
        AddCombatAction(LOATHEB_POISON_AURA, 5000u);
        AddCombatAction(LOATHEB_CORRUPTED_MIND, 4000u);
        AddCombatAction(LOATHEB_REMOVE_CURSE, 2000u);
        AddCombatAction(LOATHEB_INEVITABLE_DOOM, 120000u);
        AddCombatAction(LOATHEB_SUMMON, 12000u);
    }

    ScriptedInstance* m_instance;

    uint8 m_corruptedMindCount;

    void Reset() override
    {
        CombatAI::Reset();

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

    void EnterEvadeMode() override
    {
        CombatAI::EnterEvadeMode();

        if (m_instance)
            m_instance->SetData(TYPE_LOATHEB, NOT_STARTED);
    }

    void JustSummoned(Creature* summoned) override
    {
        if (summoned->GetEntry() == NPC_SPORE)
            summoned->SetInCombatWithZone();
    }

    void ExecuteAction(uint32 action) override
    {
        switch (action)
        {
            case LOATHEB_INEVITABLE_DOOM:
            {
                if (DoCastSpellIfCan(m_creature, SPELL_INEVITABLE_DOOM, CAST_TRIGGERED) == CAST_OK)
                    ResetCombatAction(action, ((m_corruptedMindCount <= 5) ? 30 : 15) * IN_MILLISECONDS);
                break;
            }
            case LOATHEB_CORRUPTED_MIND:
            {
                if (DoCastSpellIfCan(m_creature, SPELL_CORRUPTED_MIND, CAST_TRIGGERED) == CAST_OK)
                {
                    ++m_corruptedMindCount;
                    ResetCombatAction(action, 60 * IN_MILLISECONDS);
                }
                break;
            }
            case LOATHEB_SUMMON:
            {
                if (DoCastSpellIfCan(m_creature, SPELL_SUMMON_SPORE, CAST_TRIGGERED) == CAST_OK)
                    ResetCombatAction(action, 12 * IN_MILLISECONDS);
                break;
            }
            case LOATHEB_POISON_AURA:
            {
                if (DoCastSpellIfCan(m_creature, SPELL_POISON_AURA) == CAST_OK)
                    ResetCombatAction(action, 12 * IN_MILLISECONDS);
                break;
            }
            case LOATHEB_REMOVE_CURSE:
            {
                SpellCastResult decurseResult = m_creature->CastSpell(m_creature, SPELL_REMOVE_CURSE, TRIGGERED_OLD_TRIGGERED);
                if (decurseResult == SPELL_CAST_OK || decurseResult == SPELL_FAILED_NOTHING_TO_DISPEL)  // Don't throw an error if there is nothing to dispel
                    ResetCombatAction(action, 30 * IN_MILLISECONDS);
                break;
            }
            default:
                break;
        }
    }
};

struct CorruptedMind : public SpellScript
{
    void OnEffectExecute(Spell* spell, SpellEffectIndex effIdx) const override
    {
        if (effIdx == EFFECT_INDEX_0)
        {
            if (Unit* target = spell->GetUnitTarget())
            {
                // This spell only works on players as it triggers spells that override spell class scripts
                if (target->GetTypeId() != TYPEID_PLAYER)
                    return;

                // Determine which sub-spell to trigger for each healing class
                uint32 spellId = 0;
                switch (target->getClass())
                {
                    case CLASS_PALADIN: spellId = 29196; break;
                    case CLASS_PRIEST: spellId = 29185; break;
                    case CLASS_SHAMAN: spellId = 29198; break;
                    case CLASS_DRUID: spellId = 29194; break;
                    default: break;
                }
                if (spellId != 0)
                    spell->GetCaster()->CastSpell(target, spellId, TRIGGERED_OLD_TRIGGERED, nullptr);
            }
        }
    }
};

void AddSC_boss_loatheb()
{
    Script* newScript = new Script;
    newScript->Name = "boss_loatheb";
    newScript->GetAI = &GetNewAIInstance<boss_loathebAI>;
    newScript->RegisterSelf();

    RegisterSpellScript<CorruptedMind>("spell_loatheb_corrupted_mind");
}
