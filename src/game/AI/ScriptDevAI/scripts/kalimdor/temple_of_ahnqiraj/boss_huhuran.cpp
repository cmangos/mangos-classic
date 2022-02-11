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
SDName: Boss_Huhuran
SD%Complete: 100
SDComment:
SDCategory: Temple of Ahn'Qiraj
EndScriptData

*/

#include "AI/ScriptDevAI/include/sc_common.h"
#include "temple_of_ahnqiraj.h"
#include "AI/ScriptDevAI/base/CombatAI.h"
#include "Spells/Scripts/SpellScript.h"

enum
{
    EMOTE_GENERIC_FRENZY_KILL   = -1000001,
    EMOTE_GENERIC_BERSERK       = -1000004,

    SPELL_FRENZY                = 26051,        // triggers 26052 - Poison Bolt Volley (will hit the 15 closest people)
    SPELL_BERSERK               = 26068,        // triggers 26052 - Poison Bolt Volley (will hit the 15 closest people)
    SPELL_NOXIOUS_POISON        = 26053,
    SPELL_WYVERN_STING          = 26180,        // triggers 26233 on aura removal, doing 500 nature damage if 26180 expires (confirmed) and about 3K if dispelled early (amount unconfirmed)
    SPELL_WYVERN_STING_DAMAGE   = 26233,
    SPELL_ACID_SPIT             = 26050
};

enum HuhuranActions
{
    HUHURAN_BERSERK,
    HUHURAN_BERSERK_HP_CHECK,
    HUHURAN_FRENZY,
    HUHURAN_SPIT,
    HUHURAN_NOXIOUS_POISON,
    HUHURAN_WYVERN,
    HUHURAN_ACTION_MAX,
};

struct boss_huhuranAI : public CombatAI
{
    boss_huhuranAI(Creature* creature) : CombatAI(creature, HUHURAN_ACTION_MAX), m_instance(static_cast<ScriptedInstance*>(creature->GetInstanceData()))
    {
        AddTimerlessCombatAction(HUHURAN_BERSERK_HP_CHECK, true);
        AddCombatAction(HUHURAN_BERSERK, 5u * MINUTE * IN_MILLISECONDS);
        AddCombatAction(HUHURAN_FRENZY, 25000, 35000);
        AddCombatAction(HUHURAN_SPIT, 8u * IN_MILLISECONDS);
        AddCombatAction(HUHURAN_NOXIOUS_POISON, 10000, 20000);
        AddCombatAction(HUHURAN_WYVERN, 35000, 45000);
    }

    ScriptedInstance* m_instance;

    void Aggro(Unit* /*who*/) override
    {
        if (m_instance)
            m_instance->SetData(TYPE_HUHURAN, IN_PROGRESS);
    }

    void JustReachedHome() override
    {
        if (m_instance)
            m_instance->SetData(TYPE_HUHURAN, FAIL);
    }

    void JustDied(Unit* /*killer*/) override
    {
        if (m_instance)
            m_instance->SetData(TYPE_HUHURAN, DONE);
    }

    void ExecuteAction(uint32 action) override
    {
        switch (action)
        {
            case HUHURAN_BERSERK_HP_CHECK:
                if (m_creature->GetHealthPercent() > 30.0f)
                    return;
            case HUHURAN_BERSERK:
            {
                if (DoCastSpellIfCan(nullptr, SPELL_BERSERK) == CAST_OK)
                {
                    DoScriptText(EMOTE_GENERIC_BERSERK, m_creature);
                    SetActionReadyStatus(HUHURAN_BERSERK_HP_CHECK, false);
                    DisableCombatAction(HUHURAN_BERSERK);
                    DisableCombatAction(HUHURAN_FRENZY);
                }
                break;
            }
            case HUHURAN_FRENZY:
            {
                if (DoCastSpellIfCan(nullptr, SPELL_FRENZY) == CAST_OK)
                {
                    DoScriptText(EMOTE_GENERIC_FRENZY_KILL, m_creature);
                    ResetCombatAction(action, urand(10, 20) * IN_MILLISECONDS);
                }
                break;
            }
            case HUHURAN_SPIT:
            {
                if (DoCastSpellIfCan(m_creature->GetVictim(), SPELL_ACID_SPIT) == CAST_OK)
                    ResetCombatAction(action, urand(5, 10) * IN_MILLISECONDS);
                break;
            }
            case HUHURAN_NOXIOUS_POISON:
            {
                if (Unit* target = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0, SPELL_NOXIOUS_POISON, SELECT_FLAG_PLAYER))
                    if (DoCastSpellIfCan(target, SPELL_NOXIOUS_POISON) == CAST_OK)
                        ResetCombatAction(action, urand(12, 24) * IN_MILLISECONDS);
                break;
            }
            case HUHURAN_WYVERN:
            {
                if (DoCastSpellIfCan(nullptr, SPELL_WYVERN_STING) == CAST_OK)
                    ResetCombatAction(action, urand(25, 35) * IN_MILLISECONDS);
                break;
            }
        }
    }
};

struct HuhuranWyvernSting : public AuraScript // TODO: Add spell script for filtering
{
    void OnApply(Aura* aura, bool apply) const override
    {
        if (!apply)
        {
            if (Unit* caster = aura->GetCaster())
            {
                int32 natureDamage = (aura->GetRemoveMode() == AURA_REMOVE_BY_DISPEL) ? 3000 : 500;  // Deal 500 nature damage on spell expire but 3000 if dispelled
                caster->CastCustomSpell(aura->GetTarget(), 26233, &natureDamage, nullptr, nullptr, TRIGGERED_OLD_TRIGGERED);
            }
        }
    }
};

void AddSC_boss_huhuran()
{
    Script* pNewScript = new Script;
    pNewScript->Name = "boss_huhuran";
    pNewScript->GetAI = &GetNewAIInstance<boss_huhuranAI>;
    pNewScript->RegisterSelf();

    RegisterSpellScript<HuhuranWyvernSting>("spell_huhuran_wyvern_string");
}
