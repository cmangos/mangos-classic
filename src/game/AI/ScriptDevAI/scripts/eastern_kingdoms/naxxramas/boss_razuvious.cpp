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
EndScriptData

*/

#include "AI/ScriptDevAI/include/precompiled.h"
#include "naxxramas.h"

enum
{
    SAY_AGGRO1               = -1533120,
    SAY_AGGRO2               = -1533121,
    SAY_AGGRO3               = -1533122,
    SAY_AGGRO4               = -1533123,
    SAY_SLAY                 = -1533124,
    SAY_UNDERSTUDY_TAUNT_1   = -1533125,
    SAY_UNDERSTUDY_TAUNT_2   = -1533126,
    SAY_UNDERSTUDY_TAUNT_3   = -1533127,
    SAY_UNDERSTUDY_TAUNT_4   = -1533128,
    SAY_DEATH                = -1533129,
    EMOTE_TRIUMPHANT_SHOOT   = -1533158,

    SPELL_UNBALANCING_STRIKE = 26613,
    SPELL_DISRUPTING_SHOUT   = 29107,
    SPELL_HOPELESS           = 29125,

    SPELL_TAUNT              = 29060        // Used by Deathknight Understudy
};

struct boss_razuviousAI : public ScriptedAI
{
    boss_razuviousAI(Creature* creature) : ScriptedAI(creature)
    {
        m_instance = (instance_naxxramas*)creature->GetInstanceData();
        Reset();
    }

    instance_naxxramas* m_instance;

    uint32 m_unbalancingStrikeTimer;
    uint32 m_disruptingShoutTimer;

    void Reset() override
    {
        m_unbalancingStrikeTimer = 30 * IN_MILLISECONDS;
        m_disruptingShoutTimer   = 25 * IN_MILLISECONDS;
    }

    void KilledUnit(Unit* /*victim*/) override
    {
        DoScriptText(SAY_SLAY, m_creature);
    }

    void SpellHit(Unit* caster, const SpellEntry* spell) override
    {
        // Every time a Deathknight Understudy taunts Razuvious, he will yell its disappointment
        if (spell->Id == SPELL_TAUNT)
        {
            switch (urand(0, 3))
            {
                case 0: DoScriptText(SAY_UNDERSTUDY_TAUNT_1, caster); break;
                case 1: DoScriptText(SAY_UNDERSTUDY_TAUNT_2, caster); break;
                case 2: DoScriptText(SAY_UNDERSTUDY_TAUNT_3, caster); break;
                case 3: DoScriptText(SAY_UNDERSTUDY_TAUNT_4, caster); break;
            }
        }
    }

    void SpellHitTarget(Unit* target, const SpellEntry* spell) override
    {
        // This emote happens only when Disrupting Shout hit a target with mana
        if (spell->Id == SPELL_DISRUPTING_SHOUT && target->GetTypeId() == TYPEID_PLAYER)
        {
            if (((Player*)target)->GetPowerType() == POWER_MANA)
                DoScriptText(EMOTE_TRIUMPHANT_SHOOT, m_creature);
        }
    }

    void JustDied(Unit* /*killer*/) override
    {
        DoScriptText(SAY_DEATH, m_creature);

        DoCastSpellIfCan(m_creature, SPELL_HOPELESS, CAST_TRIGGERED);

        if (m_instance)
            m_instance->SetData(TYPE_RAZUVIOUS, DONE);
    }

    void Aggro(Unit* /*who*/) override
    {
        switch (urand(0, 3))
        {
            case 0: DoScriptText(SAY_AGGRO1, m_creature); break;
            case 1: DoScriptText(SAY_AGGRO2, m_creature); break;
            case 2: DoScriptText(SAY_AGGRO3, m_creature); break;
            case 3: DoScriptText(SAY_AGGRO4, m_creature); break;
        }

        if (m_instance)
            m_instance->SetData(TYPE_RAZUVIOUS, IN_PROGRESS);
    }

    void JustReachedHome() override
    {
        if (m_instance)
            m_instance->SetData(TYPE_RAZUVIOUS, FAIL);
    }

    void UpdateAI(const uint32 diff) override
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        // Unbalancing Strike
        if (m_unbalancingStrikeTimer < diff)
        {
            if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_UNBALANCING_STRIKE) == CAST_OK)
                m_unbalancingStrikeTimer = 30 * IN_MILLISECONDS;
        }
        else
            m_unbalancingStrikeTimer -= diff;

        // Disrupting Shout
        if (m_disruptingShoutTimer < diff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_DISRUPTING_SHOUT) == CAST_OK)
                m_disruptingShoutTimer = 25 * IN_MILLISECONDS;
        }
        else
            m_disruptingShoutTimer -= diff;

        DoMeleeAttackIfReady();
    }
};

UnitAI* GetAI_boss_razuvious(Creature* creature)
{
    return new boss_razuviousAI(creature);
}

void AddSC_boss_razuvious()
{
    Script* newScript = new Script;
    newScript->Name = "boss_razuvious";
    newScript->GetAI = &GetAI_boss_razuvious;
    newScript->RegisterSelf();
}
