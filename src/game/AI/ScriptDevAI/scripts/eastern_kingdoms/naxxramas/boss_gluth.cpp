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
SDName: Boss_Gluth
SD%Complete: 99
SDComment: Though gameplay is Blizzlike, logic in Zombie Chow Search and Call All Zombie Chow is guess work and can probably be improved.
           Also, based on similar encounters, Trigger NPCs for the summoning should probably be spawned by a (currently unknown) spell
           and the despawn of all adds should also be handled by a spell.
SDCategory: Naxxramas
EndScriptData

*/

#include "AI/ScriptDevAI/include/precompiled.h"
#include "naxxramas.h"

enum
{
    EMOTE_BOSS_GENERIC_ENRAGED      = -1000003,

    SPELL_DOUBLE_ATTACK             = 19818,
    SPELL_MORTALWOUND               = 25646,
    SPELL_DECIMATE                  = 28374,
    SPELL_ENRAGE                    = 28371,
    SPELL_BERSERK                   = 26662,
    SPELL_TERRIFYING_ROAR           = 29685,
    SPELL_SUMMON_ZOMBIE_CHOW        = 28216,                // Triggers 28217 every 6 secs
    SPELL_CALL_ALL_ZOMBIE_CHOW      = 29681,                // Triggers 29682
    SPELL_ZOMBIE_CHOW_SEARCH        = 28235,                // Triggers 28236 every 3 secs
    SPELL_ZOMBIE_CHOW_SEARCH_HEAL   = 28238,                // Healing effect

    NPC_WORLD_TRIGGER               = 15384,                // Handle the summoning of the zombie chow NPCs
};

struct boss_gluthAI : public ScriptedAI
{
    boss_gluthAI(Creature* creature) : ScriptedAI(creature)
    {
        m_instance = (instance_naxxramas*)creature->GetInstanceData();
        Reset();

        DoCastSpellIfCan(m_creature, SPELL_DOUBLE_ATTACK, CAST_TRIGGERED | CAST_AURA_NOT_PRESENT);
    }

    instance_naxxramas* m_instance;

    uint32 m_mortalWoundTimer;
    uint32 m_decimateTimer;
    uint32 m_enrageTimer;
    uint32 m_roarTimer;
    uint32 m_berserkTimer;

    CreatureList m_summoningTriggers;

    void Reset() override
    {
        m_mortalWoundTimer  = 10 * IN_MILLISECONDS;
        m_decimateTimer     = 105 * IN_MILLISECONDS;
        m_enrageTimer       = 10 * IN_MILLISECONDS;
        m_roarTimer         = 20 * IN_MILLISECONDS;
        m_berserkTimer      = 6.5 * MINUTE * IN_MILLISECONDS; // ~15 seconds after the third Decimate
    }

    void JustDied(Unit* /*killer*/) override
    {
        if (m_instance)
            m_instance->SetData(TYPE_GLUTH, DONE);

        StopSummoning();
    }

    void Aggro(Unit* /*who*/) override
    {
        if (m_instance)
            m_instance->SetData(TYPE_GLUTH, IN_PROGRESS);

        DoCastSpellIfCan(m_creature, SPELL_ZOMBIE_CHOW_SEARCH, CAST_TRIGGERED | CAST_AURA_NOT_PRESENT);     // Aura periodically looking for NPC 16360
        DoCastSpellIfCan(m_creature, SPELL_CALL_ALL_ZOMBIE_CHOW, CAST_TRIGGERED | CAST_AURA_NOT_PRESENT);   // Aura periodically calling all NPCs 16360

        // Add zombies summoning aura to the three triggers
        GetCreatureListWithEntryInGrid(m_summoningTriggers, m_creature, NPC_WORLD_TRIGGER, 100.0f);
        for (auto& trigger : m_summoningTriggers)
            trigger->CastSpell(trigger, SPELL_SUMMON_ZOMBIE_CHOW, TRIGGERED_OLD_TRIGGERED);
    }

    void KilledUnit(Unit* victim) override
    {
        if (victim->GetEntry() == NPC_ZOMBIE_CHOW)
            DoCastSpellIfCan(m_creature, SPELL_ZOMBIE_CHOW_SEARCH_HEAL, CAST_TRIGGERED);
    }

    void JustReachedHome() override
    {
        if (m_instance)
            m_instance->SetData(TYPE_GLUTH, FAIL);

        DoCastSpellIfCan(m_creature, SPELL_DOUBLE_ATTACK, CAST_TRIGGERED | CAST_AURA_NOT_PRESENT);
        StopSummoning();
    }

    void StopSummoning()
    {
        // Remove summoning aura from triggers
        for (auto& trigger : m_summoningTriggers)
            trigger->RemoveAllAuras();
    }

    void UpdateAI(const uint32 diff) override
    {
        // Do nothing if no target or if we are currently stunned (Decimate effect)
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim() || m_creature->HasAuraType(SPELL_AURA_MOD_STUN))
            return;

        // Mortal Wound
        if (m_mortalWoundTimer < diff)
        {
            if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_MORTALWOUND) == CAST_OK)
                m_mortalWoundTimer = 10 * IN_MILLISECONDS;
        }
        else
            m_mortalWoundTimer -= diff;

        // Decimate
        if (m_decimateTimer < diff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_DECIMATE, CAST_TRIGGERED) == CAST_OK)
                m_decimateTimer = urand(100, 110) * IN_MILLISECONDS;
        }
        else
            m_decimateTimer -= diff;

        // Enrage
        if (m_enrageTimer < diff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_ENRAGE) == CAST_OK)
            {
                DoScriptText(EMOTE_BOSS_GENERIC_ENRAGED, m_creature);
                m_enrageTimer = urand(10, 12) * IN_MILLISECONDS;
            }
        }
        else
            m_enrageTimer -= diff;

        // Terrifying Roar
        if (m_roarTimer < diff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_TERRIFYING_ROAR) == CAST_OK)
                m_roarTimer = 20 * IN_MILLISECONDS;
        }
        else
            m_roarTimer -= diff;

        // Berserk
        if (m_berserkTimer < diff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_BERSERK) == CAST_OK)
                m_berserkTimer = 5 * MINUTE * IN_MILLISECONDS;
        }
        else
            m_berserkTimer -= diff;

        DoMeleeAttackIfReady();
    }
};

UnitAI* GetAI_boss_gluth(Creature* creature)
{
    return new boss_gluthAI(creature);
}

void AddSC_boss_gluth()
{
    Script* newScript = new Script;
    newScript->Name = "boss_gluth";
    newScript->GetAI = &GetAI_boss_gluth;
    newScript->RegisterSelf();
}
