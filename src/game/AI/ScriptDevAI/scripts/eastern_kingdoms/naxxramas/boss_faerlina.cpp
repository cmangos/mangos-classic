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

#include "AI/ScriptDevAI/include/sc_common.h"
#include "AI/ScriptDevAI/base/BossAI.h"
#include "naxxramas.h"

enum
{
    SAY_AGGRO_1                 = 12856, // TODO: Check texts
    SAY_ENRAGE_1                = 12857,
    SAY_ENRAGE_2                = 12858,
    SAY_ENRAGE_3                = 12859,
    SAY_SLAY_1                  = 12854,
    SAY_SLAY_2                  = 12855,
    SAY_DEATH                   = 12853,

    EMOTE_BOSS_GENERIC_FRENZY   = 1191,

    // SOUND_RANDOM_AGGRO       = 8955,                     // soundId containing the 4 aggro sounds, we not using this

    SPELL_POISONBOLT_VOLLEY     = 28796,
    SPELL_ENRAGE                = 28798,
    SPELL_RAIN_OF_FIRE          = 28794,
    SPELL_WIDOWS_EMBRACE        = 28732,                    // Cast onto Faerlina by Mind Controlled adds
    SPELL_WIDOWS_EMBRACE_CD     = 28797,                    // Cast onto herself by Faerlina and handle all the cooldown and enrage debuff
    SPELL_INSTAKILL_SELF        = 28748,
};

static const float resetZ = 266.0f;                         // Above this altitude, Faerlina is outside her room and should reset (leashing)

enum FaerlinaActions
{
    FAERLINA_ACTION_MAX,
};

struct boss_faerlinaAI : public BossAI
{
    boss_faerlinaAI(Creature* creature) : BossAI(creature, FAERLINA_ACTION_MAX), m_instance(static_cast<ScriptedInstance*>(creature->GetInstanceData()))
    {
        SetDataType(TYPE_FAERLINA);
        AddOnAggroText(SAY_AGGRO_1);
        AddOnKillText(SAY_SLAY_1, SAY_SLAY_2);
        AddOnDeathText(SAY_DEATH);
        m_creature->GetCombatManager().SetLeashingCheck([&](Unit*, float, float, float z)
        {
            return z > resetZ;
        });
    }

    ScriptedInstance* m_instance;

    void OnSpellCast(SpellEntry const* spellInfo, Unit* /*target*/) override
    {
        if (spellInfo->Id == SPELL_ENRAGE)
        {
            switch (urand(0, 2))
            {
                case 0: DoBroadcastText(SAY_ENRAGE_1, m_creature); break;
                case 1: DoBroadcastText(SAY_ENRAGE_2, m_creature); break;
                case 2: DoBroadcastText(SAY_ENRAGE_3, m_creature); break;
            }
        }
    }
};

// 28732 - Widow's Embrace
struct WidowEmbrace : public SpellScript, public AuraScript
{
    void OnEffectExecute(Spell* spell, SpellEffectIndex effIdx) const override
    {
        if (effIdx == EFFECT_INDEX_1)
        {
            if (Unit* caster = spell->GetCaster())
                caster->CastSpell(nullptr, SPELL_INSTAKILL_SELF, TRIGGERED_OLD_TRIGGERED);       // Self suicide
        }
    }

    void OnApply(Aura* aura, bool apply) const override
    {
        if (!apply)
            return;
        Unit* target = aura->GetTarget();
        target->RemoveAurasDueToSpell(SPELL_ENRAGE);
        target->CastSpell(nullptr, SPELL_WIDOWS_EMBRACE_CD, TRIGGERED_OLD_TRIGGERED);
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
