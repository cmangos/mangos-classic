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
EndScriptData */

#include "AI/ScriptDevAI/include/sc_common.h"
#include "AI/ScriptDevAI/base/BossAI.h"
#include "naxxramas.h"

enum
{
    EMOTE_BOSS_GENERIC_ENRAGED      = 2384,
    EMOTE_DEVOURS_ZOMBIE_CHOW       = 12242,

    SPELL_DOUBLE_ATTACK             = 19818,
    SPELL_MORTALWOUND               = 25646,
    SPELL_DECIMATE                  = 28374,
    SPELL_DECIMATE_DAMAGE           = 28375,
    SPELL_FRENZY                    = 28371,
    SPELL_BERSERK                   = 26662,
    SPELL_TERRIFYING_ROAR           = 29685,
    SPELL_SUMMON_ZOMBIE_CHOW        = 28216,                // Triggers 28217 every 6 secs
    SPELL_CALL_ALL_ZOMBIE_CHOW      = 29681,                // Triggers 29682
    SPELL_ZOMBIE_CHOW_SEARCH        = 28235,                // Triggers 28236 every 3 secs
    SPELL_ZOMBIE_CHOW_SEARCH_HEAL   = 28238,                // Healing effect
    SPELL_ZOMBIE_CHOW_SEARCH_KILL   = 28239,                // Zombie Chow Suicide effect

    NPC_WORLD_TRIGGER               = 15384,                // Handle the summoning of the zombie chow NPCs
};

enum GluthActions
{
    GLUTH_CHOW_DELAY,
    GLUTH_MORTAL_WOUND,
    GLUTH_DECIMATE,
    GLUTH_ENRAGE,
    GLUTH_ROAR,
    GLUTH_BERSERK,
    GLUTH_ACTION_MAX,
};

struct boss_gluthAI : public BossAI
{
    boss_gluthAI(Creature* creature) : BossAI(creature, GLUTH_ACTION_MAX), m_instance(static_cast<ScriptedInstance*>(creature->GetInstanceData()))
    {
        SetDataType(TYPE_GLUTH);
        AddCombatAction(GLUTH_CHOW_DELAY, true);
        AddCombatAction(GLUTH_MORTAL_WOUND, 10u * IN_MILLISECONDS);
        AddCombatAction(GLUTH_DECIMATE, 105u * IN_MILLISECONDS);
        AddCombatAction(GLUTH_ENRAGE, 10u * IN_MILLISECONDS);
        AddCombatAction(GLUTH_ROAR, 20u * IN_MILLISECONDS);
        AddCombatAction(GLUTH_BERSERK, uint32(390 * IN_MILLISECONDS)); // ~15 seconds after the third Decimate
    }

    ScriptedInstance* m_instance;

    ObjectGuid m_zombieChow;

    uint32 GetSubsequentActionTimer(uint32 id)
    {
        switch (id)
        {
            case GLUTH_MORTAL_WOUND: return 10u * IN_MILLISECONDS;
            case GLUTH_ENRAGE: return urand(10, 12) * IN_MILLISECONDS;
            case GLUTH_ROAR: return 20u * IN_MILLISECONDS;
            case GLUTH_DECIMATE: return urand(100, 110) * IN_MILLISECONDS;
            case GLUTH_BERSERK: return 5 * MINUTE * IN_MILLISECONDS;
            default: return 0; // never occurs but for compiler
        }
    }

    void ReceiveAIEvent(AIEventType eventType, Unit* /*sender*/, Unit* invoker, uint32 /*miscValue*/) override
    {
        if (eventType == AI_EVENT_CUSTOM_A)
        {
            m_zombieChow = invoker->GetObjectGuid();
            DoBroadcastText(EMOTE_DEVOURS_ZOMBIE_CHOW, m_creature, invoker);
            ResetCombatAction(GLUTH_CHOW_DELAY, 1500);
        }
    }

    void JustDied(Unit* killer) override
    {
        BossAI::JustDied(killer);
        StopSummoning();
    }

    void Aggro(Unit* who) override
    {
        BossAI::Aggro(who);

        DoCastSpellIfCan(nullptr, SPELL_ZOMBIE_CHOW_SEARCH, CAST_TRIGGERED | CAST_AURA_NOT_PRESENT);     // Aura periodically looking for NPC 16360
        DoCastSpellIfCan(nullptr, SPELL_CALL_ALL_ZOMBIE_CHOW, CAST_TRIGGERED | CAST_AURA_NOT_PRESENT);   // Aura periodically calling all NPCs 16360
        DoCastSpellIfCan(nullptr, SPELL_DOUBLE_ATTACK, CAST_TRIGGERED | CAST_AURA_NOT_PRESENT);

        // Add zombies summoning aura to the three triggers
        auto creatureList = m_creature->GetMap()->GetCreatures("GLUTH_WORLD_TRIGGER_ZOMBIE_CHOW");
        if (creatureList)
            for (auto& trigger : *creatureList)
                trigger->CastSpell(trigger, SPELL_SUMMON_ZOMBIE_CHOW, TRIGGERED_OLD_TRIGGERED);
    }

    void EnterEvadeMode() override
    {
        BossAI::EnterEvadeMode();
        StopSummoning();
    }

    void StopSummoning()
    {
        // Remove summoning aura from triggers
        auto creatureList = m_creature->GetMap()->GetCreatures("GLUTH_WORLD_TRIGGER_ZOMBIE_CHOW");
        if (creatureList)
            for (auto& trigger : *creatureList)
                trigger->RemoveAllAuras();
    }

    void OnSpellCast(SpellEntry const* spellInfo, Unit* /*target*/) override
    {
        if (spellInfo->Id == SPELL_FRENZY)
            DoBroadcastText(EMOTE_BOSS_GENERIC_ENRAGED, m_creature);
    }

    void ExecuteAction(uint32 action) override
    {
        if (!m_instance)
            return;

        switch (action)
        {
            case GLUTH_CHOW_DELAY:
            {
                Creature* zombieChow = m_creature->GetMap()->GetCreature(m_zombieChow);
                if (!zombieChow || !zombieChow->IsAlive() || (DoCastSpellIfCan(zombieChow, SPELL_ZOMBIE_CHOW_SEARCH_KILL) == CAST_OK))
                    DisableCombatAction(action);
                break;
            }
        }
    }
};

// 28374 - Decimate
struct Decimate : public SpellScript
{
    void OnEffectExecute(Spell* spell, SpellEffectIndex effIdx ) const override
    {
        if (effIdx == EFFECT_INDEX_0)
        {
            if (Unit* unitTarget = spell->GetUnitTarget())
            {
                // Reduce all players and Zombie Chow NPCs HP to 5% max HP
                // Return if not player, pet nor Zombie Chow NPC
                if (unitTarget->GetTypeId() == TYPEID_UNIT && !unitTarget->IsControlledByPlayer() && unitTarget->GetEntry() != NPC_ZOMBIE_CHOW)
                    return;

                int32 damage = unitTarget->GetHealth() - unitTarget->GetMaxHealth() * 0.05f;
                if (damage > 0)
                    spell->GetCaster()->CastCustomSpell(unitTarget, SPELL_DECIMATE_DAMAGE, &damage, nullptr, nullptr, TRIGGERED_INSTANT_CAST);
            }
        }
    }
};

// 29682 - Call All Zombie Chow
struct CallAllZombies : public SpellScript
{
    void OnEffectExecute(Spell* spell, SpellEffectIndex effIdx ) const override
    {
        if (effIdx == EFFECT_INDEX_0)
        {
            if (Unit* unitTarget = spell->GetUnitTarget())
            {
                if (unitTarget->IsAlive())
                {
                    float radius = GetSpellRadius(sSpellRadiusStore.LookupEntry(spell->m_spellInfo->EffectRadiusIndex[effIdx]));
                    unitTarget->GetMotionMaster()->MoveFollow(spell->GetCaster(), radius, 0);
                }
            }
        }
    }
};

// 28236 - Zombie Chow Search
struct ZombieChowSearch : public SpellScript
{
    void OnEffectExecute(Spell* spell, SpellEffectIndex effIdx ) const override
    {
        if (effIdx == EFFECT_INDEX_0)
        {
            if (Unit* unitTarget = spell->GetUnitTarget())
            {
                if (!unitTarget->IsAlive())
                    return;

                Unit* caster = spell->GetCaster();
                if (caster->AI())
                    caster->AI()->SendAIEvent(AI_EVENT_CUSTOM_A, unitTarget, caster); // gluth send to itself
            }
        }
    }
};

// 28238 - Zombie Chow Search
struct ZombieChowSearchHeal : public SpellScript
{
    void OnEffectExecute(Spell* spell, SpellEffectIndex effIdx) const override
    {
        Unit* target = spell->GetUnitTarget();
        if (effIdx == EFFECT_INDEX_0)
            target->SetHealth(target->GetHealth() + target->GetMaxHealth() * 0.05f); // Gain 5% heal
    }
};

// 28239 - Zombie Chow Search
struct ZombieChowSingleTargetInstakill : public SpellScript
{
    void OnEffectExecute(Spell* spell, SpellEffectIndex effIdx) const override
    {
        Unit* target = spell->GetUnitTarget();
        if (effIdx == EFFECT_INDEX_0)
            target->CastSpell(nullptr, SPELL_ZOMBIE_CHOW_SEARCH_HEAL, TRIGGERED_INSTANT_CAST);
    }
};

void AddSC_boss_gluth()
{
    Script* newScript = new Script;
    newScript->Name = "boss_gluth";
    newScript->GetAI = &GetNewAIInstance<boss_gluthAI>;
    newScript->RegisterSelf();

    RegisterSpellScript<Decimate>("spell_gluth_decimate");
    RegisterSpellScript<CallAllZombies>("spell_gluth_call_all_zombies");
    RegisterSpellScript<ZombieChowSearch>("spell_gluth_zombie_search");
    RegisterSpellScript<ZombieChowSearchHeal>("spell_gluth_zombie_search_heal");
    RegisterSpellScript<ZombieChowSingleTargetInstakill>("spell_gluth_single_target_instakill");
}
