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
SDName: Boss_Razuvious
SD%Complete: 95%
SDComment: TODO: Deathknight Understudy are supposed to gain Mind Exhaustion debuff when released from player Mind Control
SDCategory: Naxxramas
EndScriptData */

#include "AI/ScriptDevAI/include/sc_common.h"
#include "AI/ScriptDevAI/base/BossAI.h"
#include "naxxramas.h"

enum
{
    SAY_AGGRO1               = 13075,
    SAY_AGGRO2               = 13076,
    SAY_AGGRO3               = 13078,
    SAY_AGGRO4               = 13080,
    SAY_SLAY                 = 13081,
    SAY_UNDERSTUDY_TAUNT_1   = 13077,
    SAY_UNDERSTUDY_TAUNT_2   = 13072,
    SAY_UNDERSTUDY_TAUNT_3   = 13073,
    SAY_UNDERSTUDY_TAUNT_4   = 13074,
    SAY_DEATH                = 13079,
    EMOTE_TRIUMPHANT_SHOOT   = 13082,

    SPELL_UNBALANCING_STRIKE = 26613,
    SPELL_DISRUPTING_SHOUT   = 29107,
    SPELL_HOPELESS           = 29125,

    SPELL_TAUNT              = 29060        // Used by Deathknight Understudy
};

static const float resetZ = 285.0f;         // Above this altitude, Razuvious is outside his combat area (in the stairs) and should reset (leashing)

enum RazuviousActions
{
    RAZUVIOUS_ACTION_MAX,
};

struct boss_razuviousAI : public BossAI
{
    boss_razuviousAI(Creature* creature) : BossAI(creature, RAZUVIOUS_ACTION_MAX), m_instance(static_cast<ScriptedInstance*>(creature->GetInstanceData())), m_oneHit(false)
    {
        m_creature->GetCombatManager().SetLeashingCheck([&](Unit*, float, float, float z) { return z > resetZ; });
        SetDataType(TYPE_RAZUVIOUS);
        AddOnAggroText(SAY_AGGRO1, SAY_AGGRO2, SAY_AGGRO3, SAY_AGGRO4);
        AddOnKillText(SAY_SLAY);
        AddOnDeathText(SAY_DEATH);
    }

    ScriptedInstance* m_instance;
    bool m_oneHit;

    void SpellHit(Unit* /*caster*/, const SpellEntry* spellInfo) override
    {
        // Every time a Deathknight Understudy taunts Razuvious, he will yell its disappointment
        if (spellInfo->Id == SPELL_TAUNT)
        {
            switch (urand(0, 3))
            {
                case 0: DoBroadcastText(SAY_UNDERSTUDY_TAUNT_1, m_creature); break;
                case 1: DoBroadcastText(SAY_UNDERSTUDY_TAUNT_2, m_creature); break;
                case 2: DoBroadcastText(SAY_UNDERSTUDY_TAUNT_3, m_creature); break;
                case 3: DoBroadcastText(SAY_UNDERSTUDY_TAUNT_4, m_creature); break;
            }
        }
    }

    void SpellHitTarget(Unit* target, const SpellEntry* spellInfo) override
    {
        // This emote happens only when Disrupting Shout hit a target with mana
        if (spellInfo->Id == SPELL_DISRUPTING_SHOUT && target->IsPlayer())
        {
            if (target->GetPowerType() == POWER_MANA && !m_oneHit)
            {
                DoBroadcastText(EMOTE_TRIUMPHANT_SHOOT, m_creature);
                m_oneHit = true;
            }
        }
    }

    void OnSpellCast(SpellEntry const* spellInfo, Unit* /*target*/) override
    {
        if (spellInfo->Id == SPELL_DISRUPTING_SHOUT)
            m_oneHit = false;
    }

    void JustDied(Unit* /*killer*/) override
    {
        BossAI::JustDied();
        DoCastSpellIfCan(nullptr, SPELL_HOPELESS, CAST_TRIGGERED);
    }
};

void AddSC_boss_razuvious()
{
    Script* newScript = new Script;
    newScript->Name = "boss_razuvious";
    newScript->GetAI = &GetNewAIInstance<boss_razuviousAI>;
    newScript->RegisterSelf();
}
