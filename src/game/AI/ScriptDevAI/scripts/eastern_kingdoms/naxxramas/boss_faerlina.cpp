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

#include "AI/ScriptDevAI/include/precompiled.h"
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
};

struct boss_faerlinaAI : public ScriptedAI
{
    boss_faerlinaAI(Creature* creature) : ScriptedAI(creature)
    {
        m_instance = (instance_naxxramas*)creature->GetInstanceData();
        Reset();
    }

    instance_naxxramas* m_instance;

    uint32 m_poisonBoltVolleyTimer;
    uint32 m_rainOfFireTimer;
    uint32 m_enrageTimer;

    void Reset() override
    {
        m_poisonBoltVolleyTimer   = 8 * IN_MILLISECONDS;
        m_rainOfFireTimer         = 16 * IN_MILLISECONDS;
        m_enrageTimer             = 60 * IN_MILLISECONDS;
    }

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

    void JustReachedHome() override
    {
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
                m_enrageTimer = 60 * IN_MILLISECONDS;             // Delay next enrage by 60 sec if Faerlina was already enraged
            }
        }
    }

    void UpdateAI(const uint32 diff) override
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        // Poison Bolt Volley
        if (m_poisonBoltVolleyTimer < diff)
        {
            if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_POISONBOLT_VOLLEY) == CAST_OK)
                m_poisonBoltVolleyTimer = 11 * IN_MILLISECONDS;
        }
        else
            m_poisonBoltVolleyTimer -= diff;

        // Rain Of Fire
        if (m_rainOfFireTimer < diff)
        {
            if (Unit* target = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
            {
                if (DoCastSpellIfCan(target, SPELL_RAIN_OF_FIRE) == CAST_OK)
                    m_rainOfFireTimer = 16 * IN_MILLISECONDS;
            }
        }
        else
            m_rainOfFireTimer -= diff;

        // Enrage Timer
        if (m_enrageTimer < diff && !m_creature->HasAura(SPELL_ENRAGE))
        {
            if (DoCastSpellIfCan(m_creature, SPELL_ENRAGE) == CAST_OK)
            {
                switch (urand(0, 2))
                {
                    case 0: DoScriptText(SAY_ENRAGE_1, m_creature); break;
                    case 1: DoScriptText(SAY_ENRAGE_2, m_creature); break;
                    case 2: DoScriptText(SAY_ENRAGE_3, m_creature); break;
                }
                m_enrageTimer = 60 * IN_MILLISECONDS;
            }
        }
        else
            m_enrageTimer -= diff;

        DoMeleeAttackIfReady();
    }
};

UnitAI* GetAI_boss_faerlina(Creature* creature)
{
    return new boss_faerlinaAI(creature);
}

void AddSC_boss_faerlina()
{
    Script* newScript = new Script;
    newScript->Name = "boss_faerlina";
    newScript->GetAI = &GetAI_boss_faerlina;
    newScript->RegisterSelf();
}
