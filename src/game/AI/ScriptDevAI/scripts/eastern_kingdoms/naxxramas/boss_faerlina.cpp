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
SDName: Boss_Faerlina
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
    SAY_AGGRO_1                 = -1533010,
    SAY_ENRAGE_1                = -1533011,
    SAY_ENRAGE_2                = -1533012,
    SAY_ENRAGE_3                = -1533013,
    SAY_SLAY_1                  = -1533014,
    SAY_SLAY_2                  = -1533015,
    SAY_DEATH                   = -1533016,

    EMOTE_BOSS_GENERIC_FRENZY   = -1000005,

    // SOUND_RANDOM_AGGRO       = 8955,                     // soundId containing the 4 aggro sounds, we not using this

    SPELL_POISONBOLT_VOLLEY     = 28796,
    SPELL_ENRAGE                = 28798,
    SPELL_RAIN_OF_FIRE          = 28794,
    SPELL_WIDOWS_EMBRACE_1      = 28732,                    // Cast onto Faerlina by Mind Controlled adds
    SPELL_WIDOWS_EMBRACE_2      = 28797,                    // Cast onto herself by Faerlina and handle all the cooldown and enrage debuff
    SPELL_INSTAKILL_SELF        = 28748,
};

static const float resetZ = 266.0f;                         // Above this altitude, Faerlina is outside her room and should reset (leashing)

enum FaerlinaActions
{
    FAERLINA_POISON_VOLLEY,
    FAERLINA_RAIN_FIRE,
    FAERLINA_ENRAGE,
    FAERLINA_ACTION_MAX,
};

struct boss_faerlinaAI : public CombatAI
{
    boss_faerlinaAI(Creature* creature) : CombatAI(creature, FAERLINA_ACTION_MAX), m_instance(static_cast<ScriptedInstance*>(creature->GetInstanceData()))
    {
        AddCombatAction(FAERLINA_POISON_VOLLEY, 8000u);
        AddCombatAction(FAERLINA_RAIN_FIRE, 16000u);
        AddCombatAction(FAERLINA_ENRAGE, 60000u);
        m_creature->GetCombatManager().SetLeashingCheck([&](Unit*, float, float, float z)
        {
            return z > resetZ;
        });
    }

    ScriptedInstance* m_instance;

    void Aggro(Unit* /*who*/) override
    {
        DoScriptText(SAY_AGGRO_1, m_creature);

        if (m_instance)
            m_instance->SetData(TYPE_FAERLINA, IN_PROGRESS);
    }

    void KilledUnit(Unit* /*victim*/) override
    {
        DoScriptText(urand(0, 1) ? SAY_SLAY_1 : SAY_SLAY_2, m_creature);
    }

    void JustDied(Unit* /*killer*/) override
    {
        DoScriptText(SAY_DEATH, m_creature);

        if (m_instance)
            m_instance->SetData(TYPE_FAERLINA, DONE);
    }

    void EnterEvadeMode() override
    {
        CombatAI::EnterEvadeMode();

        if (m_instance)
            m_instance->SetData(TYPE_FAERLINA, FAIL);
    }

    // Widow's Embrace prevents Frenzy and Poison bolt by activating 30 sec spell cooldown
    // If target also has Frenzy, it will remove it and delay next one by 60s
    void SpellHit(Unit* /*caster*/, const SpellEntry* spellEntry) override
    {
        if (spellEntry->Id == SPELL_WIDOWS_EMBRACE_1)
        {
            DoCastSpellIfCan(m_creature, SPELL_WIDOWS_EMBRACE_2);   // Start spell category cooldown, delaying next Poison Bolt and Frenzy by 30 secs

            if (m_creature->HasAura(SPELL_ENRAGE))
            {
                m_creature->RemoveAurasDueToSpell(SPELL_ENRAGE);
                ResetCombatAction(FAERLINA_ENRAGE, 60 * IN_MILLISECONDS);   // Delay next enrage by 60 sec if Faerlina was already enraged
            }
        }
    }

    void ExecuteAction(uint32 action) override
    {
        switch (action)
        {
            case FAERLINA_POISON_VOLLEY:
            {
                if (DoCastSpellIfCan(m_creature->GetVictim(), SPELL_POISONBOLT_VOLLEY) == CAST_OK)
                    ResetCombatAction(action, 11 * IN_MILLISECONDS);
                break;
            }
            case FAERLINA_RAIN_FIRE:
            {
                if (Unit* target = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
                {
                    if (DoCastSpellIfCan(target, SPELL_RAIN_OF_FIRE) == CAST_OK)
                        ResetCombatAction(action, 16 * IN_MILLISECONDS);
                }
                break;
            }
            case FAERLINA_ENRAGE:
            {
                if (!m_creature->HasAura(SPELL_ENRAGE))
                {
                    if (DoCastSpellIfCan(m_creature, SPELL_ENRAGE) == CAST_OK)
                    {
                        switch (urand(0, 2))
                        {
                            case 0: DoScriptText(SAY_ENRAGE_1, m_creature); break;
                            case 1: DoScriptText(SAY_ENRAGE_2, m_creature); break;
                            case 2: DoScriptText(SAY_ENRAGE_3, m_creature); break;
                        }
                        ResetCombatAction(action, 60 * IN_MILLISECONDS);
                    }
                }
                break;
            }
            default:
                break;
        }
    }
};

struct WidowEmbrace : public SpellScript
{
    void OnEffectExecute(Spell* spell, SpellEffectIndex effIdx) const override
    {
        if (effIdx == EFFECT_INDEX_1)
        {
            if (Unit* caster = spell->GetCaster())
                caster->CastSpell(nullptr, SPELL_INSTAKILL_SELF, TRIGGERED_OLD_TRIGGERED);       // Self suicide
        }
    }
};

void AddSC_boss_faerlina()
{
    Script* newScript = new Script;
    newScript->Name = "boss_faerlina";
    newScript->GetAI = &GetNewAIInstance<boss_faerlinaAI>;
    newScript->RegisterSelf();

    RegisterSpellScript<WidowEmbrace>("spell_faerlina_widow_embrace");
}
