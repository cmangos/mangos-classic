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
SDName: Boss_Ouro
SD%Complete: 90
SDComment: Some minor adjustments may be required
SDCategory: Temple of Ahn'Qiraj
EndScriptData

*/

#include "AI/ScriptDevAI/include/sc_common.h"
#include "temple_of_ahnqiraj.h"
#include "AI/ScriptDevAI/base/CombatAI.h"
#include "Spells/Scripts/SpellScript.h"

enum
{
    // ground spells
    SPELL_DOUBLE_ATTACK     = 18941,
    SPELL_SWEEP             = 26103,
    SPELL_SANDBLAST         = 26102,
    SPELL_BOULDER           = 26616,
    SPELL_BERSERK           = 26615,

    // emerge spells
    SPELL_BIRTH_SPAWN       = 26586,
    SPELL_GROUND_RUPTURE    = 26100,
    SPELL_SUMMON_BASE       = 26133,                        // summons gameobject 180795

    // submerge spells
    SPELL_SUBMERGE_VISUAL   = 26063,
    SPELL_SUMMON_OURO_MOUNDS = 26058,                       // summons 5 dirt mounds
    SPELL_SUMMON_TRIGGER    = 26284,

    SPELL_SUMMON_OURO_TRIGG = 26642,
    SPELL_QUAKE             = 26093,
    SPELL_DREAM_FOG         = 24780,                        // on mounds

    // other spells - not used
    SPELL_SUMMON_SCARABS    = 26060,                        // triggered after 30 secs - cast by the Dirt Mounds
    SPELL_DIRTMOUND_PASSIVE = 26092,                        // casts 26093 every 1 sec - removed from DBC (readded)
    // SPELL_SET_OURO_HEALTH   = 26075,                     // unused - done through AI
    // SPELL_SAVE_OURO_HEALTH  = 26076,                     // unused - done through AI
    // SPELL_TELEPORT_TRIGGER  = 26285,                     // unused
    // SPELL_SUBMERGE_TRIGGER  = 26104,                     // unused
    SPELL_SUMMON_OURO_MOUND = 26617,
    SPELL_SCARABS_PERIODIC  = 26619,

    // summoned npcs
    NPC_OURO                = 15517,
    // NPC_OURO_SCARAB       = 15718,                       // summoned by Dirt Mounds
    NPC_OURO_TRIGGER        = 15717,
    NPC_DIRT_MOUND          = 15712,
};

enum OuroActions
{
    OURO_SPAWN,
    OURO_ENRAGE,
    OURO_SUBMERGE,
    OURO_BOULDER,
    OURO_SAND_BLAST,
    OURO_SWEEP,
    OURO_DOUBLE_ATTACK,
    OURO_NO_MELEE_BURROW,
    OURO_ACTION_MAX,
    OURO_SPAWN_DELAY_ATTACK,
};

struct npc_ouro_triggerAI : public ScriptedAI // needs to be before Ouro for compilation reasons
{
    npc_ouro_triggerAI(Creature* creature) : ScriptedAI(creature)
    {
        SetReactState(REACT_PASSIVE);
        SetCombatMovement(false);
    }

    void Reset() override
    {

    }

    float m_ouroHealth;

    void JustRespawned() override
    {
        if (Unit* ouro = m_creature->GetSpawner())
            m_ouroHealth = ouro->GetHealthPercent();
    }
};

struct boss_ouroAI : public CombatAI
{
    boss_ouroAI(Creature* creature) : CombatAI(creature, OURO_ACTION_MAX), m_instance(static_cast<ScriptedInstance*>(creature->GetInstanceData())), m_firstMound(true)
    {
        AddTimerlessCombatAction(OURO_ENRAGE, true);
        AddCombatAction(OURO_SPAWN, 300u);
        AddCombatAction(OURO_SUBMERGE, 90000u);
        AddCombatAction(OURO_BOULDER, true);
        AddCombatAction(OURO_SAND_BLAST, 25000u);
        AddCombatAction(OURO_SWEEP, 20000u);
        AddCombatAction(OURO_DOUBLE_ATTACK, urand(1000, 10000));
        AddCombatAction(OURO_NO_MELEE_BURROW, 2000u);
        AddCustomAction(OURO_SPAWN_DELAY_ATTACK, true, [&]() {HandleSpawnDelay(); });
        m_creature->SetImmobilizedState(true);
    }

    ScriptedInstance* m_instance;
    int32 m_rangeCheckState;
    bool m_firstMound;
    uint32 m_burrowCounter;
    ObjectGuid m_base;

    void Reset() override
    {
        CombatAI::Reset();
        SetCombatMovement(false);
        SetMeleeEnabled(false);

        m_burrowCounter = 0;
    }

    void JustRespawned() override
    {
        CombatAI::JustRespawned();
        m_creature->CastSpell(nullptr, SPELL_SUMMON_BASE, TRIGGERED_OLD_TRIGGERED);
        m_creature->SetInCombatWithZone();
        if (!m_creature->IsInCombat()) // noone left alive
            JustReachedHome(); // handles fail
    }

    void JustReachedHome() override
    {
        if (m_instance)
            m_instance->SetData(TYPE_OURO, FAIL);

        m_creature->ForcedDespawn();
        if (GameObject* go = m_creature->GetMap()->GetGameObject(m_base))
        {
            go->SetLootState(GO_JUST_DEACTIVATED);
            go->SetForcedDespawn();
        }
    }

    void JustDied(Unit* /*killer*/) override
    {
        if (m_instance)
            m_instance->SetData(TYPE_OURO, DONE);
    }

    void JustSummoned(GameObject* go) override
    {
        m_base = go->GetObjectGuid();
    }

    void JustSummoned(Creature* summoned) override
    {
        switch (summoned->GetEntry())
        {
            case NPC_DIRT_MOUND:
                if (m_firstMound)
                    summoned->CastSpell(nullptr, SPELL_SUMMON_OURO_TRIGG, TRIGGERED_OLD_TRIGGERED);
                else
                    summoned->CastSpell(nullptr, SPELL_SCARABS_PERIODIC, TRIGGERED_OLD_TRIGGERED);
                m_firstMound = false;
                break;
        }
    }

    void SpellHitTarget(Unit* target, const SpellEntry* spell) override
    {
        if (spell->Id == SPELL_SANDBLAST)
            m_creature->getThreatManager().modifyThreatPercent(target, -100);
    }

    void Burrow()
    {
        if (DoCastSpellIfCan(nullptr, SPELL_SUBMERGE_VISUAL) == CAST_OK)
        {
            SetCombatScriptStatus(true);
            SetMeleeEnabled(false);
            DoCastSpellIfCan(nullptr, SPELL_SUMMON_OURO_MOUNDS, CAST_TRIGGERED);
            DoCastSpellIfCan(nullptr, SPELL_SUMMON_TRIGGER, CAST_TRIGGERED);

            m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_UNINTERACTIBLE);
            m_creature->ForcedDespawn(2000);
            DisableCombatAction(OURO_SUBMERGE);
            DisableCombatAction(OURO_NO_MELEE_BURROW);
            if (GameObject* go = m_creature->GetMap()->GetGameObject(m_base))
            {
                go->SetLootState(GO_JUST_DEACTIVATED);
                go->SetForcedDespawn();
            }
        }
    }

    void HandleSpawnDelay()
    {
        SetMeleeEnabled(true);
        SetCombatScriptStatus(false);
        AttackClosestEnemy();
        if (m_creature->GetVictim())
            DoCastSpellIfCan(m_creature->GetVictim(), SPELL_GROUND_RUPTURE);
    }

    void ExecuteAction(uint32 action) override
    {
        switch (action)
        {
            case OURO_SPAWN:
            {
                DoCastSpellIfCan(nullptr, SPELL_BIRTH_SPAWN);
                if (m_creature->GetUInt32Value(UNIT_CREATED_BY_SPELL))
                {
                    if (Creature* trigger = GetClosestCreatureWithEntry(m_creature, NPC_OURO_TRIGGER, 100.f)) // if not sufficient in the future, do using SPELL_SET_OURO_HEALTH
                        m_creature->SetHealthPercent(static_cast<npc_ouro_triggerAI*>(trigger->AI())->m_ouroHealth);
                }
                else
                {
                    if (m_instance)
                        m_instance->SetData(TYPE_OURO, IN_PROGRESS);                    
                }
                SetCombatScriptStatus(true);
                if (GameObject* go = m_creature->GetMap()->GetGameObject(m_base))
                    go->SendGameObjectCustomAnim(m_base);
                ResetTimer(OURO_SPAWN_DELAY_ATTACK, 4000);
                DisableCombatAction(action);
                break;
            }
            case OURO_ENRAGE:
            {
                if (m_creature->GetHealthPercent() < 20.0f)
                {
                    if (DoCastSpellIfCan(nullptr, SPELL_BERSERK) == CAST_OK)
                    {
                        m_firstMound = false; // in order to avoid spawning more ouros
                        ResetCombatAction(OURO_BOULDER, 500);
                        SetActionReadyStatus(action, false);
                        DisableCombatAction(OURO_SUBMERGE);
                        DisableCombatAction(OURO_NO_MELEE_BURROW);
                        return;
                    }
                }
                break;
            }
            case OURO_SUBMERGE:
            {
                Burrow();
                break;
            }
            case OURO_SAND_BLAST:
            {
                if (DoCastSpellIfCan(nullptr, SPELL_SANDBLAST) == CAST_OK)
                    ResetCombatAction(action, 25000);
                break;
            }
            case OURO_SWEEP:
            {
                if (DoCastSpellIfCan(nullptr, SPELL_SWEEP) == CAST_OK)
                    ResetCombatAction(action, 20000);
                break;
            }
            case OURO_DOUBLE_ATTACK:
            {
                if (DoCastSpellIfCan(nullptr, SPELL_DOUBLE_ATTACK) == CAST_OK)
                    ResetCombatAction(action, urand(7000, 13000));
                break;
            }
            case OURO_BOULDER:
            {
                uint32 timer = 500;
                if (m_creature->GetVictim() && m_creature->CanReachWithMeleeAttack(m_creature->GetVictim()))
                    m_rangeCheckState = -1;
                else
                {
                    ++m_rangeCheckState;
                    if (m_rangeCheckState > 1)
                        if (Unit* target = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0, SPELL_BOULDER, SELECT_FLAG_PLAYER))
                            if (DoCastSpellIfCan(target, SPELL_BOULDER) == CAST_OK)
                                timer = 2500;
                }
                ResetCombatAction(action, timer);
                break;
            }
            case OURO_NO_MELEE_BURROW:
            {
                if (m_creature->GetVictim() && m_creature->CanReachWithMeleeAttack(m_creature->GetVictim()))
                    m_burrowCounter = 0;
                else
                {
                    ++m_burrowCounter;
                    if (m_burrowCounter >= 10)
                        Burrow();
                }
                ResetCombatAction(action, 1000);
                break;
            }
        }
    }
};

struct npc_ouro_spawnerAI : public Scripted_NoMovementAI
{
    npc_ouro_spawnerAI(Creature* creature) : Scripted_NoMovementAI(creature) {Reset();}

    bool m_bHasSummoned;

    void Reset() override
    {
        m_bHasSummoned = false;

        DoCastSpellIfCan(nullptr, SPELL_DIRTMOUND_PASSIVE, CAST_AURA_NOT_PRESENT | CAST_TRIGGERED);
    }

    void Aggro(Unit* /*who*/) override
    {
        if (!m_bHasSummoned)
        {
            m_creature->SummonCreature(NPC_OURO, m_creature->GetPositionX(), m_creature->GetPositionY(), m_creature->GetPositionZ(), m_creature->GetOrientation(), TEMPSPAWN_DEAD_DESPAWN, 0, true);
            m_bHasSummoned = true;
        }
    }

    void JustSummoned(Creature* summoned) override
    {
        // Despawn when Ouro is spawned
        if (summoned->GetEntry() == NPC_OURO)
        {
            m_creature->ForcedDespawn();
            m_creature->SetRespawnDelay(604800);
        }
    }

    void UpdateAI(const uint32 /*uiDiff*/) override { }
};

struct npc_ouro_moundAI : public ScriptedAI
{
    npc_ouro_moundAI(Creature* creature) : ScriptedAI(creature, 0)
    {
        SetReactState(REACT_DEFENSIVE);
        AddCustomAction(1, true, [&]() { PickNewTarget(); });
    }

    void Reset() override
    {
        DoCastSpellIfCan(nullptr, SPELL_DIRTMOUND_PASSIVE, CAST_AURA_NOT_PRESENT | CAST_TRIGGERED);
    }

    void EnterEvadeMode() override
    {
        if (m_creature->GetInstanceData() && m_creature->HasAura(SPELL_SUMMON_OURO_TRIGG))
            static_cast<ScriptedInstance*>(m_creature->GetInstanceData())->SetData(TYPE_OURO, FAIL);
        ScriptedAI::EnterEvadeMode();
        m_creature->ForcedDespawn(1000); // despawn on wipe
    }

    void JustSummoned(Creature* summoned) override
    {
        m_creature->ForcedDespawn(1);
    }

    void PickNewTarget()
    {
        m_creature->SetInCombatWithZone();
        DoResetThreat();
        if (Unit* target = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0, nullptr, SELECT_FLAG_PLAYER))
        {
            m_creature->AddThreat(target, 1000000.f);
            AttackStart(target);
        }
        ResetTimer(1, urand(5000, 10000));
    }

    void JustRespawned() override
    {
        ScriptedAI::JustRespawned();
        ResetTimer(1, 1000); // delayed first target
    }
};

enum OuroScarabActions
{
    OURO_SCARAB_ACTION_MAX,
    OURO_SCARAB_DELAYED_ATTACK,
};

// TODO: verify 20477
struct OuroScarab : public CombatAI
{
    OuroScarab(Creature* creature) : CombatAI(creature, OURO_SCARAB_ACTION_MAX)
    {
        SetReactState(REACT_DEFENSIVE);
        AddCustomAction(OURO_SCARAB_DELAYED_ATTACK, 4000u, [&]() {HandleDelayedAttack(); });
    }

    void JustRespawned() override
    {
        CombatAI::JustRespawned();
        m_creature->SetCorpseDelay(5);
    }

    void HandleDelayedAttack()
    {
        SetReactState(REACT_AGGRESSIVE);
        m_creature->SetInCombatWithZone();
        if (Unit* target = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
        {
            m_creature->AddThreat(target, 1000000.f);
            AttackStart(target);
        }
    }

    void EnterEvadeMode() override
    {
        ScriptedAI::EnterEvadeMode();
        m_creature->ForcedDespawn(1000); // despawn on wipe
    }

    void ExecuteAction(uint32 action) override
    {
    }
};

struct PeriodicScarabTrigger : public AuraScript
{
    void OnPeriodicTrigger(Aura* aura, PeriodicTriggerData& data) const override
    {
        data.spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(SPELL_SUMMON_SCARABS);
        data.caster = aura->GetTarget();
        data.target = nullptr;
    }
};

void AddSC_boss_ouro()
{
    Script* pNewScript = new Script;
    pNewScript->Name = "boss_ouro";
    pNewScript->GetAI = &GetNewAIInstance<boss_ouroAI>;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_ouro_spawner";
    pNewScript->GetAI = &GetNewAIInstance<npc_ouro_spawnerAI>;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_ouro_mound";
    pNewScript->GetAI = &GetNewAIInstance<npc_ouro_moundAI>;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_ouro_trigger";
    pNewScript->GetAI = &GetNewAIInstance<npc_ouro_triggerAI>;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_ouro_scarab";
    pNewScript->GetAI = &GetNewAIInstance<OuroScarab>;
    pNewScript->RegisterSelf();

    RegisterSpellScript<PeriodicScarabTrigger>("spell_periodic_scarab_trigger");
}
