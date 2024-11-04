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
SDName: Boss_Razorgore
SD%Complete: 95
SDComment: Threat management after Mind Control is released needs core support (boss should have aggro on its previous controller and its previous victim should have threat transfered from boss to controlling player)
SDCategory: Blackwing Lair
EndScriptData

*/

#include "AI/ScriptDevAI/include/sc_common.h"
#include "blackwing_lair.h"
#include "AI/ScriptDevAI/base/CombatAI.h"

enum
{
    SAY_EGGS_BROKEN_1           = -1469022,
    SAY_EGGS_BROKEN_2           = -1469023,
    SAY_EGGS_BROKEN_3           = -1469024,

    SPELL_POSSESS_VISUAL        = 23014,                    // visual effect and increase the damage taken
    SPELL_DESTROY_EGG           = 19873,
    // SPELL_POSSESS               = 19832,                    // actual possess spell
    SPELL_DRAGON_ORB            = 23021,                    // shrink - not in sniff
    SPELL_MIND_EXHAUSTION       = 23958,

    SPELL_CLEAVE                = 19632,
    SPELL_WARSTOMP              = 24375,
    SPELL_FIREBALL_VOLLEY       = 22425,
    SPELL_CONFLAGRATION         = 23023,

    SPELL_DOUBLE_ATTACK         = 18943,
};

enum RazorgoreActions
{
    RAZORGORE_CONFLAGRATION,
    RAZORGORE_FIREBALL_VOLLEY,
    RAZORGORE_WAR_STOMP,
    RAZORGORE_CLEAVE,
    RAZORGORE_ACTION_MAX,
};

struct boss_razorgoreAI : public CombatAI
{
    boss_razorgoreAI(Creature* creature) : CombatAI(creature, RAZORGORE_ACTION_MAX), m_instance(static_cast<instance_blackwing_lair*>(creature->GetInstanceData()))
    {
        AddCombatAction(RAZORGORE_CONFLAGRATION, 10000, 15000);
        AddCombatAction(RAZORGORE_FIREBALL_VOLLEY, 15000, 20000);
        AddCombatAction(RAZORGORE_WAR_STOMP, 30000u);
        AddCombatAction(RAZORGORE_CLEAVE, 4000, 8000);
        SetDeathPrevention(true);
        m_creature->SetWalk(false);
        if (m_instance)
        {
            m_creature->GetCombatManager().SetLeashingCheck([](Unit* unit, float x, float y, float z)
            {
                return static_cast<ScriptedInstance*>(unit->GetInstanceData())->GetPlayerInMap(true, false) == nullptr;
            });
        }
    }

    instance_blackwing_lair* m_instance;

    void Reset() override
    {
        CombatAI::Reset();
        SetDeathPrevention(true);

        DoCastSpellIfCan(nullptr, SPELL_DOUBLE_ATTACK, CAST_AURA_NOT_PRESENT | CAST_TRIGGERED);
    }

    void Aggro(Unit* attacker) override
    {
        m_creature->RemoveAurasDueToSpell(SPELL_POSSESS_VISUAL);
    }

    void ReceiveAIEvent(AIEventType eventType, Unit* /*sender*/, Unit* /*invoker*/, uint32 /*miscValue*/) override
    {
        if (eventType == AI_EVENT_CUSTOM_A)
        {
            SetDeathPrevention(false);
            SetMeleeEnabled(true);
            m_creature->RemoveAurasDueToSpell(SPELL_POSSESS_VISUAL);
            m_creature->CastSpell(nullptr, SPELL_WARMING_FLAMES, TRIGGERED_OLD_TRIGGERED);
        }
    }

    void JustPreventedDeath(Unit* attacker) override
    {
        m_instance->SetData(TYPE_RAZORGORE, FAIL);
    }

    void JustReachedHome() override
    {
        if (m_instance)
            m_instance->SetData(TYPE_RAZORGORE, FAIL);
    }

    void ExecuteAction(uint32 action) override
    {
        switch (action)
        {
            case RAZORGORE_CONFLAGRATION:
            {
                if (DoCastSpellIfCan(nullptr, SPELL_CONFLAGRATION) == CAST_OK)
                    ResetCombatAction(action, urand(15000, 25000));
                break;
            }
            case RAZORGORE_FIREBALL_VOLLEY:
            {
                if (DoCastSpellIfCan(nullptr, SPELL_FIREBALL_VOLLEY) == CAST_OK)
                    ResetCombatAction(action, urand(15000, 20000));
                break;
            }
            case RAZORGORE_WAR_STOMP:
            {
                if (DoCastSpellIfCan(nullptr, SPELL_WARSTOMP) == CAST_OK)
                    ResetCombatAction(action, 30000);
                break;
            }
            case RAZORGORE_CLEAVE:
            {
                if (DoCastSpellIfCan(m_creature->GetVictim(), SPELL_CLEAVE) == CAST_OK)
                    ResetCombatAction(action, urand(4000, 8000));
                break;
            }
        }
    }
};

struct npc_blackwing_orbAI : public ScriptedAI
{
    npc_blackwing_orbAI(Creature* creature) : ScriptedAI(creature), m_instance(static_cast<instance_blackwing_lair*>(creature->GetInstanceData()))
    {
        Reset();
    }

    instance_blackwing_lair* m_instance;

    uint32 m_uiIntroVisualTimer;

    void Reset() override
    {
        m_uiIntroVisualTimer = 4000;
    }

    void SpellHit(Unit* /*caster*/, const SpellEntry* spellInfo) override
    {
        // If hit by Razorgore's fireball: explodes everything in the room
        if (spellInfo->Id == SPELL_FIREBALL)
        {
            if (Creature* pTemp = m_creature->SummonCreature(NPC_ORB_DOMINATION, m_creature->GetPositionX(), m_creature->GetPositionY(), m_creature->GetPositionZ(), 0, TEMPSPAWN_TIMED_DESPAWN, 5 * IN_MILLISECONDS))
                DoScriptText(EMOTE_ORB_SHUT_OFF, pTemp);
            m_creature->CastSpell(m_creature, SPELL_EXPLODE_ORB, TRIGGERED_IGNORE_UNATTACKABLE_FLAG);
        }
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        // Set visual only on OOC timer
        if (m_uiIntroVisualTimer)
        {
            if (m_uiIntroVisualTimer <= uiDiff)
            {
                if (!m_instance)
                {
                    script_error_log("Instance Blackwing Lair: ERROR Failed to load instance data for this instace.");
                    return;
                }

                // If Razorgore is not respawned yet: wait
                if (Creature* pRazorgore = m_instance->GetSingleCreatureFromStorage(NPC_RAZORGORE))
                {
                    if (!(pRazorgore->IsAlive()))
                    {
                        m_uiIntroVisualTimer = 2000;
                        return;
                    }
                }

                // If Grethok the Controller is here and spawned, start the visual, else wait for him
                if (Creature* grethok = GetClosestCreatureWithEntry(m_creature, NPC_GRETHOK_CONTROLLER, 2.0f))
                {
                    if (grethok->IsAlive())
                    {
                        m_creature->CastSpell(m_creature, SPELL_POSSESS_VISUAL, TRIGGERED_OLD_TRIGGERED);
                        grethok->CastSpell(grethok, SPELL_CONTROL_ORB, TRIGGERED_OLD_TRIGGERED);
                        m_uiIntroVisualTimer = 0;
                    }
                }
                else
                    m_uiIntroVisualTimer = 2000;
            }
            else
                m_uiIntroVisualTimer -= uiDiff;
        }
    }
};

bool ProcessEventIdRazorgorePossess(uint32 eventId, Object* source, Object* target, bool isStart)
{
    if (!source->IsPlayer())
        return true;

    Player* player = static_cast<Player*>(source);
    if (Creature* trigger = static_cast<ScriptedInstance*>(player->GetMap()->GetInstanceData())->GetSingleCreatureFromStorage(NPC_BLACKWING_ORB_TRIGGER))
        trigger->CastSpell(nullptr, SPELL_POSSESS_VISUAL, TRIGGERED_OLD_TRIGGERED);
    
    return true;
}

struct DestroyEgg : public SpellScript
{
    void OnEffectExecute(Spell* spell, SpellEffectIndex effIdx) const override
    {
        GameObject* target = spell->GetGOTarget();
        Unit* razorgore = spell->GetCaster();
        if (!target || !razorgore)
            return;

        if (ScriptedInstance* pInstance = static_cast<ScriptedInstance*>(target->GetInstanceData()))
        {
            if (urand(0, 1))
            {
                switch (urand(0, 2))
                {
                    case 0: DoScriptText(SAY_EGGS_BROKEN_1, razorgore); break;
                    case 1: DoScriptText(SAY_EGGS_BROKEN_2, razorgore); break;
                    case 2: DoScriptText(SAY_EGGS_BROKEN_3, razorgore); break;
                }
            }

            // Store the eggs which are destroyed, in order to count them for the second phase
            pInstance->SetData64(DATA_DRAGON_EGG, target->GetObjectGuid());
        }
    }
};

struct ExplosionRazorgore : public SpellScript
{
    bool OnCheckTarget(const Spell* spell, Unit* target, SpellEffectIndex effIdx) const override
    {
        if (target->IsPlayer())
            return true;

        static std::set<uint32> validEntries = { NPC_GRETHOK_CONTROLLER, NPC_BLACKWING_GUARDSMAN, NPC_BLACKWING_LEGIONNAIRE , NPC_BLACKWING_MAGE , NPC_DRAGONSPAWN, NPC_RAZORGORE };
        return validEntries.find(target->GetEntry()) != validEntries.end();
    }
};

struct PossessRazorgore : public AuraScript
{
    void OnApply(Aura* aura, bool apply) const override
    {
        if (aura->GetEffIndex() == EFFECT_INDEX_0)
        {
            if (apply)
            {
                if (Unit* caster = aura->GetCaster())
                {
                    caster->CastSpell(caster, SPELL_MIND_EXHAUSTION, TRIGGERED_OLD_TRIGGERED);
                }
                aura->GetTarget()->CastSpell(nullptr, SPELL_DRAGON_ORB, TRIGGERED_OLD_TRIGGERED);
            }
            else
            {
                if (Unit* target = aura->GetTarget())
                {
                    target->RemoveAurasDueToSpell(SPELL_POSSESS_VISUAL);
                    target->RemoveAurasDueToSpell(SPELL_DRAGON_ORB);
                }
            }
        }
    }
};

struct CalmDragonkin : public SpellScript
{
    SpellCastResult OnCheckCast(Spell* spell, bool strict) const override
    {
        if (spell->m_targets.getUnitTarget()->GetEntry() == NPC_RAZORGORE)
            return SPELL_FAILED_BAD_TARGETS;

        return SPELL_CAST_OK;
    }
};

void AddSC_boss_razorgore()
{
    Script* pNewScript = new Script;
    pNewScript->Name = "boss_razorgore";
    pNewScript->GetAI = &GetNewAIInstance<boss_razorgoreAI>;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_blackwing_orb";
    pNewScript->GetAI = &GetNewAIInstance<npc_blackwing_orbAI>;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "event_razorgore_possess";
    pNewScript->pProcessEventId = &ProcessEventIdRazorgorePossess;
    pNewScript->RegisterSelf();

    RegisterSpellScript<DestroyEgg>("spell_destroy_egg");
    RegisterSpellScript<ExplosionRazorgore>("spell_explosion_razorgore");
    RegisterSpellScript<PossessRazorgore>("spell_possess_razorgore");
    RegisterSpellScript<CalmDragonkin>("spell_calm_dragonkin");
}
