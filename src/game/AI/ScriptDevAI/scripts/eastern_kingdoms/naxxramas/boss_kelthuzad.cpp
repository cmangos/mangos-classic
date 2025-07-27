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
SDName: Boss_KelThuzad
SD%Complete: 99
SDComment: Phase 1 summoning timers and/or aura stacking are to be improved once Classic sniffs are available
SDCategory: Naxxramas
EndScriptData

*/

#include "AI/ScriptDevAI/include/sc_common.h"
#include "AI/ScriptDevAI/base/BossAI.h"
#include "naxxramas.h"

enum
{
    SAY_SUMMON_MINIONS                  = 12999,         // start of phase 1

    SAY_AGGRO1                          = 12995,
    SAY_AGGRO2                          = 12996,
    SAY_AGGRO3                          = 12997,

    SAY_SLAY1                           = 13021,
    SAY_SLAY2                           = 13022,

    SAY_DEATH                           = 13019,

    SAY_CHAIN1                          = 13017,
    SAY_CHAIN2                          = 13018,
    SAY_FROST_BLAST                     = 13020,

    SAY_SPECIAL1_MANA_DET               = 13492,

    SAY_REQUEST_AID                     = 12998,         // Start of phase 3
    SAY_ANSWER_REQUEST                  = 12994,         // Lich King answer

    // Phase 1 spells
    SPELL_SUMMON_TYPE_A                 = 28421,            // Summon 1 Soldier of the Frozen Waste
    SPELL_SUMMON_TYPE_B                 = 28422,            // Summon 1 Unstoppable Abomination
    SPELL_SUMMON_TYPE_C                 = 28423,            // Summon 1 Soul Weaver

    SPELL_CHANNEL_VISUAL_EFFECT         = 29422,            // Handle the application of summoning auras

    // Call forth one of the add to the center of the room
    // Soldier of the Frozen Waste (triggers 28415)
    SPELL_SUMMON_PERIODIC_A_1           = 29410,            // Every 5 secs
    SPELL_SUMMON_PERIODIC_A_2           = 29391,            // Every 4 secs
    SPELL_SUMMON_PERIODIC_A_3           = 28425,            // Every 3 secs
    SPELL_SUMMON_PERIODIC_A_4           = 29392,            // Every 2 secs
    SPELL_SUMMON_PERIODIC_A_5           = 29409,            // Every 1 secs
    // Unstoppable Abomination (triggers 28416)
    SPELL_SUMMON_PERIODIC_B_1           = 28426,            // Every 30 secs
    SPELL_SUMMON_PERIODIC_B_2           = 29393,            // Every 25 secs
    SPELL_SUMMON_PERIODIC_B_3           = 29394,            // Every 20 secs
    SPELL_SUMMON_PERIODIC_B_4           = 29398,            // Every 15 secs
    SPELL_SUMMON_PERIODIC_B_5           = 29411,            // Every 10 secs
    // Soul Weaver (triggers 28417)
    SPELL_SUMMON_PERIODIC_C_1           = 29399,            // Every 40 secs
    SPELL_SUMMON_PERIODIC_C_2           = 29400,            // Every 35 secs
    SPELL_SUMMON_PERIODIC_C_3           = 28427,            // Every 30 secs
    SPELL_SUMMON_PERIODIC_C_4           = 29401,            // Every 20 secs
    SPELL_SUMMON_PERIODIC_C_5           = 29412,            // Every 15 secs

    MAX_SUMMON_TICKS                    = 5 * MINUTE + 10,  // Adds despawn after 5 min 10 secs
    MAX_CHANNEL_TICKS                   = 5 * MINUTE + 25,  // Start phase 2 after 5 min 25 secs

    NPC_WORLD_TRIGGER                   = 15384,            // Handle the summoning of the NPCs in each alcove

    // Phase 2 spells
    SPELL_FROST_BOLT                    = 28478,
    SPELL_FROST_BOLT_NOVA               = 28479,

    SPELL_CHAINS_OF_KELTHUZAD           = 28408,
    SPELL_CHAINS_OF_KELTHUZAD_SCALE     = 28409,
    SPELL_CHAINS_OF_KELTHUZAD_TARGET    = 28410,

    SPELL_MANA_DETONATION               = 27819,
    SPELL_SHADOW_FISSURE                = 27810,
    SPELL_FROST_BLAST                   = 27808,            // Frost Blast: stun spell
    SPELL_FROST_BLAST_DAMAGE            = 29879,            // Frost Blast: damage spell

    SPELL_SUMMON_TYPE_D                 = 28454,            // Summon 1 Guardian of Icecrown
    SPELL_SUMMON_PERIODIC_D             = 28453,            // One Guardian of Icecrown summoned every 10 secs
    SPELL_GUARDIAN_INIT                 = 29898,            // Cast at start of phase 3 by Kel'Thuzad (start of phase 2 in sniffs but this way we avoid running a timer needlessly)

    MAX_CONTROLLED_TARGETS              = 4,                // 5 targets are mind controlled by Chains of Kel'Thuzad but the main tank is always the fifth one
};

static const uint32 phaseOneAdds[] = {NPC_SOLDIER_FROZEN, NPC_UNSTOPPABLE_ABOM, NPC_SOUL_WEAVER};

// List which spell must be used at which point in phase 1 to summon a given NPC type
static const std::vector<std::vector<uint32>> soulWeaverSpellsTimers {
        {1,   SPELL_SUMMON_PERIODIC_C_1},   // 1 for 40 sec
        {42,  SPELL_SUMMON_PERIODIC_C_4},   // 1 for 20 sec
        {63,  SPELL_SUMMON_PERIODIC_C_3},   // 3 for 90 sec
        {154, SPELL_SUMMON_PERIODIC_C_5},   // 9 for 135 sec
        {290, 0}                            // Total: 14 Soul Weavers
};

static const std::vector<std::vector<uint32>> unstoppableAbominationSpellsTimers {
        {15,  SPELL_SUMMON_PERIODIC_B_1},   // 2 for 60 sec
        {76,  SPELL_SUMMON_PERIODIC_B_2},   // 2 for 50 sec
        {127, SPELL_SUMMON_PERIODIC_B_3},   // 5 for 100 sec
        {228, SPELL_SUMMON_PERIODIC_B_4},   // 3 for 45 sec
        {259, SPELL_SUMMON_PERIODIC_B_5},   // 2 for 20 sec
        {291, 0}                            // Total: 14 Unstoppable Abominations
};

static const std::vector<std::vector<uint32>> soldierFrozenWasteSpellsTimers {
        {7,   SPELL_SUMMON_PERIODIC_A_1},   // 13 for 65 secs
        {73,  SPELL_SUMMON_PERIODIC_A_2},   // 15 for 60 secs
        {134, SPELL_SUMMON_PERIODIC_A_3},   // 18 for 54 secs
        {189, SPELL_SUMMON_PERIODIC_A_4},   // 27 for 54 secs
        {244, SPELL_SUMMON_PERIODIC_A_5},   // 47 for 47 secs
        {292, 0}                            // Total: 120 Soldiers of the Frozen Wastes
};

// List all spells used to summon a given NPC type during phase 1
static const std::map<uint32, std::vector<uint32>> summoningSpells {
        {NPC_SOUL_WEAVER,      {SPELL_SUMMON_PERIODIC_C_1, SPELL_SUMMON_PERIODIC_C_2, SPELL_SUMMON_PERIODIC_C_3, SPELL_SUMMON_PERIODIC_C_4, SPELL_SUMMON_PERIODIC_C_5}},
        {NPC_UNSTOPPABLE_ABOM, {SPELL_SUMMON_PERIODIC_B_1, SPELL_SUMMON_PERIODIC_B_2, SPELL_SUMMON_PERIODIC_B_3, SPELL_SUMMON_PERIODIC_B_4, SPELL_SUMMON_PERIODIC_B_5}},
        {NPC_SOLDIER_FROZEN,   {SPELL_SUMMON_PERIODIC_A_1, SPELL_SUMMON_PERIODIC_A_2, SPELL_SUMMON_PERIODIC_A_3, SPELL_SUMMON_PERIODIC_A_4, SPELL_SUMMON_PERIODIC_A_5}}
};

enum Phase
{
    PHASE_NORMAL,
    PHASE_GUARDIANS,
};

enum KelThuzadActions
{
    KELTHUZAD_PHASE_GUARDIANS,
    KELTHUZAD_ACTIONS_MAX,
    KELTHUZAD_LICH_KING_ANSWER,
    KELTHUZAD_STOP_GUARDIAN_SPAWNS,
};

struct boss_kelthuzadAI : public BossAI
{
    boss_kelthuzadAI(Creature* creature) : BossAI(creature, KELTHUZAD_ACTIONS_MAX), m_instance(dynamic_cast<instance_naxxramas*>(creature->GetInstanceData()))
    {
        SetDataType(TYPE_KELTHUZAD);
        AddOnKillText(SAY_SLAY1, SAY_SLAY2);
        AddOnDeathText(SAY_DEATH);
        AddOnAggroText(SAY_AGGRO1, SAY_AGGRO2, SAY_AGGRO3);
        AddCustomAction(KELTHUZAD_LICH_KING_ANSWER, true, [&]() { HandleLichKingAnswer(); });
        AddTimerlessCombatAction(KELTHUZAD_PHASE_GUARDIANS, false);
        AddCustomAction(KELTHUZAD_STOP_GUARDIAN_SPAWNS, true, [&]() { StopSpawningGuardians(); });
    }

    instance_naxxramas* m_instance;

    uint8 m_phase;
    uint32 m_summonTicks;

    std::map<ObjectGuid, GuidVector> m_introMobsList;

    void Reset() override
    {
        BossAI::Reset();
        m_summonTicks = 0;

        m_phase                = PHASE_NORMAL;

        m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PLAYER | UNIT_FLAG_UNINTERACTIBLE);
        SetMeleeEnabled(false);
    }

    void EnterEvadeMode() override
    {
        DespawnIntroCreatures();

        m_creature->InterruptNonMeleeSpells(false);

        if (m_creature->HasAura(SPELL_CHANNEL_VISUAL))
        {
            m_creature->RemoveAurasDueToSpell(SPELL_CHANNEL_VISUAL);
            m_creature->RemoveAurasDueToSpell(SPELL_CHANNEL_VISUAL_EFFECT);
        }

        BossAI::EnterEvadeMode();
    }

    void SpellHit(Unit* /*caster*/, const SpellEntry* spellInfo) override
    {
        // Phase 1 start
        if (spellInfo->Id == SPELL_CHANNEL_VISUAL)
        {
            DoBroadcastText(SAY_SUMMON_MINIONS, m_creature);
        }
        // Phase 1 periodic update every 1 second (everything in phase 1 is handled here rather than in UpdateAI())
        else if (spellInfo->Id == SPELL_CHANNEL_VISUAL_EFFECT)
        {
            ++m_summonTicks;
            switch (m_summonTicks)
            {
                case 4:
                    SummonIntroCreatures(NPC_SOLDIER_FROZEN, 9);
                    break;
                case 9:
                    SummonIntroCreatures(NPC_UNSTOPPABLE_ABOM, 3);
                    break;
                case 12:
                    SummonIntroCreatures(NPC_SOUL_WEAVER, 1);
                    break;
                case MAX_SUMMON_TICKS:
                    DespawnIntroCreatures();    // Will leave those that for some reason are in combat
                    switch (urand(0, 2))
                    {
                        case 0: DoBroadcastText(SAY_AGGRO1, m_creature); break;
                        case 1: DoBroadcastText(SAY_AGGRO2, m_creature); break;
                        case 2: DoBroadcastText(SAY_AGGRO3, m_creature); break;
                    }
                    break;
                case MAX_CHANNEL_TICKS:
                    StartPhase2();
                    break;
                default:
                    UpdateSummoning();
                    // Kel'Thuzad does not enter combat until phase 2, so we check every second if there are still players alive and force him to evade otherwise
                    if (!m_instance->GetPlayerInMap(true, false))
                        EnterEvadeMode();
                    break;
            }
        }
    }

    // Remove all periodic auras on Kel'Thuzad used to summon a given NPC type
    void StopAllSummoningForNPC(uint32 entry)
    {
        auto iter = summoningSpells.begin();
        while (iter != summoningSpells.end())
        {
            if (iter->first == entry)
                for (auto aura : iter->second)
                    m_creature->RemoveAurasDueToSpell(aura);
            ++iter;
        }
    }

    // Called every tick (1 second) to check if the periodic summoning auras on Kel'Thuzad need to be updated or removed
    void UpdateSummoning()
    {
        for (auto add : phaseOneAdds)
        {
            uint32 summoningAura = 0;
            std::vector<std::vector<uint32>> spellTimers;
            switch (add)
            {
                case NPC_SOUL_WEAVER:
                    spellTimers = soulWeaverSpellsTimers;
                    break;
                case NPC_UNSTOPPABLE_ABOM:
                    spellTimers = unstoppableAbominationSpellsTimers;
                    break;
                case NPC_SOLDIER_FROZEN:
                    spellTimers = soldierFrozenWasteSpellsTimers;
                    break;
                default:
                    return;
            }

            // Iterate the summoning auras timeline for the current add as long as the tick is in range
            // then exit with the last valid summoning aura
            for (const auto& spellTimer : spellTimers)
            {
                if (spellTimer[0] <= m_summonTicks)
                    summoningAura = spellTimer[1];
                else
                    break;
            }

            if (summoningAura == 0) // beginning
                continue;

            // Do not update aura for current add type if summoning aura is already present
            if (m_creature->HasAura(summoningAura))
                continue;

            // Clear all summoning auras for current add before applying the new one (especially needed in case new one is NULL)
            StopAllSummoningForNPC(add);
            DoCastSpellIfCan(m_creature, summoningAura, CAST_TRIGGERED);
        }
    }

    void DespawnIntroCreatures()
    {
        if (m_instance)
        {
            for (auto itr : m_introMobsList)
            {
                for (auto guid : itr.second)
                {
                    if (Creature* creature = m_instance->instance->GetCreature(guid))
                    {
                        if (creature->IsAlive() && !creature->IsInCombat())
                            creature->ForcedDespawn();
                    }
                }
            }
        }

        m_introMobsList.clear();
    }

    // Summon three type of adds in each of the surrounding alcoves
    bool SummonIntroCreatures(uint32 entry, uint8 count)
    {
        if (!m_instance)
            return false;

        float newX, newY, newZ;

        std::vector<Creature*> const* summoningTriggers = m_creature->GetMap()->GetCreatures("KEL_THUZAD_WORLD_TRIGGERS_ADDS");

        // Spawn all the adds for phase 1 from each of the trigger NPCs
        for (auto& trigger : *summoningTriggers)
        {
            // "How many NPCs per type" is stored in a vector: {npc_entry:number_of_npcs}
            for (uint8 i = 0; i < count; ++i)
            {
                m_creature->GetRandomPoint(trigger->GetPositionX(), trigger->GetPositionY(), trigger->GetPositionZ(), 12.0f, newX, newY, newZ);
                if (Creature* summoned = m_creature->SummonCreature(entry, newX, newY, newZ, 0.0f, TEMPSPAWN_CORPSE_DESPAWN, 5 * MINUTE * IN_MILLISECONDS))
                {
                    if (summoned->AI())
                        summoned->AI()->SetReactState(REACT_PASSIVE);   // Intro mobs only attack if engaged or hostile target in range
                }
            }
        }
        return true;
    }

    // Every time an add dies, randomly summon the same type on one of the seven corners
    void SummonedCreatureJustDied(Creature* summoned) override
    {
        switch (summoned->GetEntry())
        {
            case NPC_SOLDIER_FROZEN:
            case NPC_SOUL_WEAVER:
            case NPC_UNSTOPPABLE_ABOM:
                m_introMobsList.erase(summoned->GetObjectGuid());
                break;
            default:
                break;
        }
    }

    void StartPhase2()
    {
        m_phase = PHASE_NORMAL;

        m_creature->InterruptNonMeleeSpells(false);
        m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PLAYER | UNIT_FLAG_UNINTERACTIBLE);
        m_creature->SetInCombatWithZone();
        SetCombatMovement(true);
        m_creature->GetMotionMaster()->MoveChase(m_creature->GetVictim());

        AddInitialCooldowns();
    }

    void StopSpawningGuardians()
    {
        m_creature->RemoveAurasDueToSpell(SPELL_SUMMON_PERIODIC_D);
    }

    void JustSummoned(Creature* summoned) override
    {
        switch (summoned->GetEntry())
        {
            case NPC_GUARDIAN:
            {
                summoned->SetInCombatWithZone();
                break;
            }
            case NPC_SOLDIER_FROZEN:
            case NPC_UNSTOPPABLE_ABOM:
            case NPC_SOUL_WEAVER:
                m_introMobsList[summoned->GetSpawnerGuid()].push_back(summoned->GetObjectGuid());
                summoned->SetCanCallForAssistance(false);
                summoned->GetMotionMaster()->MoveRandomAroundPoint(summoned->GetPositionX(), summoned->GetPositionY(), summoned->GetPositionZ(), 3.f);
                break;
        }
    }

    void SummonedMovementInform(Creature* summoned, uint32 motionType, uint32 pointId) override
    {
        if (motionType == POINT_MOTION_TYPE && pointId == 0)
            summoned->SetInCombatWithZone();
    }

    void OnSpellCast(SpellEntry const* spellInfo, Unit* target) override
    {
        switch (spellInfo->Id)
        {
            case SPELL_CHAINS_OF_KELTHUZAD: DoScriptText(urand(0, 1) ? SAY_CHAIN1 : SAY_CHAIN2, m_creature); break;
            case SPELL_FROST_BLAST: if (urand(0, 1)) DoScriptText(SAY_FROST_BLAST, m_creature); break;
            case SPELL_MANA_DETONATION: if (urand(0, 1)) DoScriptText(SAY_SPECIAL1_MANA_DET, m_creature); break;
        }
    }

    void HandleLichKingAnswer()
    {
        // Start summoning Guardians of Icecrown
        m_creature->CastSpell(nullptr, SPELL_SUMMON_PERIODIC_D, TRIGGERED_OLD_TRIGGERED);
        // Start checking if Guardians are shackled or not
        m_creature->CastSpell(nullptr, SPELL_GUARDIAN_INIT, TRIGGERED_OLD_TRIGGERED);

        if (Creature* lichKing = m_instance->GetSingleCreatureFromStorage(NPC_THE_LICHKING))
            DoScriptText(SAY_ANSWER_REQUEST, lichKing);

        m_instance->DoUseDoorOrButton(GO_KELTHUZAD_WINDOW_1);
        m_instance->DoUseDoorOrButton(GO_KELTHUZAD_WINDOW_2);
        m_instance->DoUseDoorOrButton(GO_KELTHUZAD_WINDOW_3);
        m_instance->DoUseDoorOrButton(GO_KELTHUZAD_WINDOW_4);
        ResetTimer(KELTHUZAD_STOP_GUARDIAN_SPAWNS, 55s);
    }

    void ExecuteAction(uint32 action) override
    {
        switch (action)
        {
            case KELTHUZAD_PHASE_GUARDIANS:
            {
                if (m_creature->GetHealthPercent() < 40.0f)
                {
                    ResetTimer(KELTHUZAD_LICH_KING_ANSWER, 4000);
                    DoScriptText(SAY_REQUEST_AID, m_creature);
                    DisableCombatAction(action);
                }
                return;
            }
        }
    }
};

/*########################
#   npc_icecrown_guardian
########################*/

enum {
    SPELL_GUARDIAN_PASSIVE      = 29897,
    SPELL_GUARDIAN_SHACKLE      = 29899,
    SPELL_BLOOD_TAP             = 28470,
};

// This NPC handles the spell sync between the player that is web wrapped (with a DoT) and the related Web Wrap NPC
struct npc_icecrown_guardianAI : public ScriptedAI
{
    npc_icecrown_guardianAI(Creature* creature) : ScriptedAI(creature), m_instance(dynamic_cast<instance_naxxramas*>(creature->GetInstanceData()))
    {
        Reset();
    }

    instance_naxxramas* m_instance;

    ObjectGuid m_victimGuid;
    uint32 m_checkCurrentVictimTimer;

    void Reset() override
    {
        m_victimGuid.Clear();
        m_checkCurrentVictimTimer = 2 * IN_MILLISECONDS;    // Check every 2 seconds that we still have the same target

        DoCastSpellIfCan(nullptr, SPELL_GUARDIAN_PASSIVE, CAST_AURA_NOT_PRESENT);
    }

    void JustReachedHome() override
    {
        m_creature->ForcedDespawn();
    }

    void EnterEvadeMode() override
    {
        m_creature->RemoveAurasDueToSpell(SPELL_BLOOD_TAP);

        ScriptedAI::EnterEvadeMode();
    }

    void KilledUnit(Unit* /*victim*/) override
    {
        DoCastSpellIfCan(m_creature->GetVictim(), SPELL_BLOOD_TAP);
    }

    void UpdateAI(const uint32 diff) override
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->GetVictim())
            return;

        if (m_checkCurrentVictimTimer < diff)
        {
            // Get current victim GUID if not already assigned
            if (!m_victimGuid && m_creature->GetVictim())
                m_victimGuid = m_creature->GetVictim()->GetObjectGuid();

            // If we changed our target: cast Blood Tap
            if (m_victimGuid != m_creature->GetVictim()->GetObjectGuid())
            {
                DoCastSpellIfCan(m_creature->GetVictim(), SPELL_BLOOD_TAP);
                m_victimGuid = m_creature->GetVictim()->GetObjectGuid();
            }
            m_checkCurrentVictimTimer = 2 * IN_MILLISECONDS;    // Check every 2 seconds that we still have the same target
        }
        else
            m_checkCurrentVictimTimer -= diff;

        DoMeleeAttackIfReady();
    }
};

// 28415 - Summon Type A Trigger
// 28416 - Summon Type B Trigger
// 28417 - Summon Type C Trigger
// 28455 - Summon Type D Trigger
struct TriggerKTAdd : public SpellScript
{
    void OnEffectExecute(Spell* spell, SpellEffectIndex /* effIdx */) const override
    {
        if (Unit* unitTarget = spell->GetUnitTarget())
        {
            uint32 summonId = 0;
            switch (spell->m_spellInfo->Id)
            {
                case 28415: summonId = SPELL_SUMMON_TYPE_A; break;
                case 28416: summonId = SPELL_SUMMON_TYPE_B; break;
                case 28417: summonId = SPELL_SUMMON_TYPE_C; break;
                case 28455: summonId = SPELL_SUMMON_TYPE_D; break;
                default: break;
            }

            if (summonId)
                unitTarget->CastSpell(nullptr, summonId, TRIGGERED_OLD_TRIGGERED, nullptr, nullptr, spell->GetCaster()->GetObjectGuid(), nullptr);
        }
    }
};

// 28408 - Chains of Kel'Thuzad
struct ChainsKelThuzad : public SpellScript
{
    void OnEffectExecute(Spell* spell, SpellEffectIndex /* effIdx */) const override
    {
        // Select four random players plus the main tank and mind control them all. Evil.
        Creature* caster = (Creature*)spell->GetCaster();

        if (!caster)
            return;

        SelectAttackingTargetParams chainsParams;
        chainsParams.range.minRange = 0;
        chainsParams.range.maxRange = 100;

        std::vector<Unit*> targets;
        // Skip the main tank because we will add it later
        caster->SelectAttackingTargets(targets, ATTACKING_TARGET_ALL_SUITABLE, 0, nullptr, SELECT_FLAG_PLAYER | SELECT_FLAG_SKIP_TANK, chainsParams);

        // Prevent the mind control to happen if all remaining players would be targeted
        if (targets.size() <= MAX_CONTROLLED_TARGETS)
            return;

        std::shuffle(targets.begin(), targets.end(), *GetRandomGenerator());
        targets.resize(MAX_CONTROLLED_TARGETS);

        DoBroadcastText(urand(0, 1) ? SAY_CHAIN1 : SAY_CHAIN2, caster);

        // Mind control the random targets
        for (auto& target : targets)
        {
            caster->CastSpell(target, SPELL_CHAINS_OF_KELTHUZAD_TARGET, TRIGGERED_OLD_TRIGGERED);
            target->CastSpell(target, SPELL_CHAINS_OF_KELTHUZAD_SCALE, TRIGGERED_OLD_TRIGGERED);
        }

        // Mind control the main tank
        caster->CastSpell(caster->GetVictim(), SPELL_CHAINS_OF_KELTHUZAD_TARGET, TRIGGERED_OLD_TRIGGERED);
        caster->GetVictim()->CastSpell(caster->GetVictim(), SPELL_CHAINS_OF_KELTHUZAD_SCALE, TRIGGERED_OLD_TRIGGERED);
    }
};

// 27808 - Frost Blast
struct FrostBlast : public AuraScript
{
    void OnPeriodicTrigger(Aura* aura, PeriodicTriggerData& /* data */) const override
    {
        Unit* caster = aura->GetCaster();
        if (!caster)
            return;

        // Do damage onto the player equal to 26% of his/her full hit points on every tick
        Unit* target = aura->GetTarget();
        int32 basePointsDamage = target->GetMaxHealth() * 26 / 100;
        caster->CastCustomSpell(target, SPELL_FROST_BLAST_DAMAGE, &basePointsDamage, nullptr, nullptr, TRIGGERED_OLD_TRIGGERED);
    }
};

// 29897 - Guardian of Icecrown Passive
struct GuardianPeriodic : public AuraScript
{
    void OnPeriodicTrigger(Aura* aura, PeriodicTriggerData& data) const override
    {
        // Periodically inform the instance if the target is controlled through Shackle Undead or not
        if (Unit* target = aura->GetTarget())
        {
            if (target->HasAuraType(SPELL_AURA_MOD_STUN))   // Shackle Undead applies SPELL_AURA_MOD_STUN, so we cover all ranks of the spell this way
                data.spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(SPELL_GUARDIAN_SHACKLE); // Summon Ice Block
        }
    }
};

// 27819 - Detonate Mana
struct DetonateManaKelThuzad : public AuraScript
{
    void OnPeriodicTrigger(Aura* aura, PeriodicTriggerData& data) const override
    {
        // 50% Mana Burn
        data.caster = aura->GetTarget();
        data.target = nullptr;
        data.spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(27820);
        data.basePoints[0] = (int32)aura->GetTarget()->GetPower(POWER_MANA) * 0.5f;
        aura->GetTarget()->ModifyPower(POWER_MANA, -data.basePoints[0]);
    }
};

void AddSC_boss_kelthuzad()
{
    Script* newScript = new Script;
    newScript->Name = "boss_kelthuzad";
    newScript->GetAI = &GetNewAIInstance<boss_kelthuzadAI>;
    newScript->RegisterSelf();

    newScript = new Script;
    newScript->Name = "npc_icecrown_guardian";
    newScript->GetAI = &GetNewAIInstance<npc_icecrown_guardianAI>;
    newScript->RegisterSelf();

    RegisterSpellScript<TriggerKTAdd>("spell_trigger_KT_add");
    RegisterSpellScript<ChainsKelThuzad>("spell_chains_kel_thuzad");
    RegisterSpellScript<FrostBlast>("spell_kel_thuzad_frost_blast");
    RegisterSpellScript<GuardianPeriodic>("spell_icecrown_guardian_periodic");
    RegisterSpellScript<DetonateManaKelThuzad>("spell_detonate_mana_kt");
}
