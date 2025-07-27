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
SDName: Boss_Gothik
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
    SAY_SPEECH_1                = 13030,
    SAY_SPEECH_2                = 13031,
    SAY_SPEECH_3                = 13032,
    SAY_SPEECH_4                = 13033,

    SAY_KILL                    = 13027,
    SAY_DEATH                   = 13026,
    SAY_TELEPORT                = 13028,

    PHASE_SPEECH                = 0,
    PHASE_BALCONY               = 1,
    PHASE_TELEPORT_DOWN         = 2,
    PHASE_TELEPORTING           = 3,
    PHASE_STOP_TELEPORTING      = 4,

    // Right is right side from Gothik (eastern), i.e. right is living and left is spectral
    SPELL_TELEPORT_RIGHT        = 28025,
    SPELL_TELEPORT_LEFT         = 28026,

    SPELL_HARVESTSOUL           = 28679,
    SPELL_SHADOWBOLT            = 29317,
    SPELL_IMMUNE_ALL            = 29230,            // Cast during balcony phase to make Gothik unattackable

    SPELL_RESET_GOTHIK_EVENT    = 28035,

    // Spectral side spells
    SPELL_SUMMON_SPECTRAL_TRAINEE   = 27921,
    SPELL_SUMMON_SPECTRAL_KNIGHT    = 27932,
    SPELL_SUMMON_SPECTRAL_RIVENDARE = 27939,
    SPELL_CHECK_SPECTRAL_SIDE       = 28749,

    // Unrelenting side spell
    SPELL_CHECK_UNRELENTING_SIDE    = 29875,
};

enum GothikActions
{
    GOTHIK_TELEPORT,
    GOTHIK_STOP_TELE,
    GOTHIK_ACTIONS_MAX,
    GOTHIK_GROUND_PHASE,
    GOTHIK_SUMMON_TRAINEE,
    GOTHIK_SUMMON_DEATH_KNIGHT,
    GOTHIK_SUMMON_RIDER,
    GOTHIK_SPEECH,
};

struct boss_gothikAI : public BossAI
{
    boss_gothikAI(Creature* creature) : BossAI(creature, GOTHIK_ACTIONS_MAX), m_instance(static_cast<instance_naxxramas*>(creature->GetInstanceData())), m_phase(PHASE_SPEECH),
        m_speechStep(0), m_teleportCount(0)
    {
        SetDataType(TYPE_GOTHIK);
        AddOnKillText(SAY_KILL);
        AddOnDeathText(SAY_DEATH);
        AddTimerlessCombatAction(GOTHIK_STOP_TELE, true);
        AddCustomAction(GOTHIK_SUMMON_TRAINEE, true, [&]() { StartTraineeSummons(); });
        AddCustomAction(GOTHIK_SUMMON_DEATH_KNIGHT, true, [&]() { StartKnightSummons(); });
        AddCustomAction(GOTHIK_SUMMON_RIDER, true, [&]() { StartRiderSummons(); });
        AddCombatAction(GOTHIK_TELEPORT, true);
        AddCustomAction(GOTHIK_GROUND_PHASE, true, [&]() { HandleGroundPhase(); });
        AddCustomAction(GOTHIK_SPEECH, true, [&]()
        {
            switch (m_speechStep)
            {
                case 0: DoBroadcastText(SAY_SPEECH_1, m_creature); DoPlaySoundToSet(m_creature, 8807); ResetTimer(GOTHIK_SPEECH, 4s); break;
                case 1: DoBroadcastText(SAY_SPEECH_2, m_creature); ResetTimer(GOTHIK_SPEECH, 6s); break;
                case 2: DoBroadcastText(SAY_SPEECH_3, m_creature); ResetTimer(GOTHIK_SPEECH, 5s); break;
                case 3:
                {
                    DoBroadcastText(SAY_SPEECH_4, m_creature);
                    m_phase = PHASE_BALCONY;
                    break;
                }
            }
            ++m_speechStep;
        });
    }

    instance_naxxramas* m_instance;

    GuidVector m_summonedAddGuids;

    uint8 m_phase;
    uint8 m_speechStep;
    uint8 m_teleportCount;

    void Reset() override
    {
        BossAI::Reset();
        m_creature->RemoveAurasDueToSpell(SPELL_IMMUNE_ALL);
        // Remove immunity
        m_phase = PHASE_SPEECH;
        m_speechStep = 0;

        m_teleportCount = 0;

        // Only attack and be attackable while on ground
        SetMeleeEnabled(false);
        SetCombatMovement(false);
        SetReactState(REACT_PASSIVE);
        SetCombatScriptStatus(false);

        // Despawn Adds
        DespawnGuids(m_summonedAddGuids);
    }

    void HandleGroundPhase()
    {
        m_creature->CastSpell(nullptr, SPELL_TELEPORT_RIGHT, TRIGGERED_OLD_TRIGGERED);
        m_phase = m_instance ? PHASE_TELEPORTING : PHASE_STOP_TELEPORTING;

        DoBroadcastText(SAY_TELEPORT, m_creature);

        // Remove Immunity
        m_creature->RemoveAurasDueToSpell(SPELL_IMMUNE_ALL);
        SetMeleeEnabled(true);
        SetCombatMovement(true);
        SetReactState(REACT_AGGRESSIVE);

        DoResetThreat();
        m_creature->SetInCombatWithZone();
        SetRangedMode(true, 100.f, TYPE_FULL_CASTER);
        AttackClosestEnemy();
        ResetCombatAction(GOTHIK_TELEPORT, 20s);
        AddInitialCooldowns();
    }

    void Aggro(Unit* /*who*/) override
    {
        BossAI::Aggro();
        m_creature->SetInCombatWithZone();
        m_creature->SetTarget(nullptr);
        SetCombatScriptStatus(true);
        SetCombatMovement(false);
        SetMeleeEnabled(false);
        // Make immune if not already
        if (!m_creature->HasAura(SPELL_IMMUNE_ALL))
            DoCastSpellIfCan(nullptr, SPELL_IMMUNE_ALL, TRIGGERED_OLD_TRIGGERED);

        // Start timer before summoning
        ResetTimer(GOTHIK_SPEECH, 1s);

        ResetTimer(GOTHIK_SUMMON_TRAINEE, 4s);
        ResetTimer(GOTHIK_SUMMON_DEATH_KNIGHT, 49s);
        ResetTimer(GOTHIK_SUMMON_RIDER, 104s);
        ResetTimer(GOTHIK_GROUND_PHASE, 4min + 34s);
    }

    bool IsCentralDoorClosed() const
    {
        return m_instance && m_instance->GetData(TYPE_GOTHIK) != SPECIAL;
    }

    void ProcessCentralDoor()
    {
        if (IsCentralDoorClosed())
        {
            m_instance->SetData(TYPE_GOTHIK, SPECIAL);
        }
    }

    bool HasPlayersInLeftSide() const
    {
        Map::PlayerList const& players = m_instance->instance->GetPlayers();

        if (players.isEmpty())
            return false;

        for (const auto& playerGuid : players)
        {
            if (Player* player = playerGuid.getSource())
            {
                if (!m_instance->IsInRightSideGothikArea(player) && player->IsAlive())
                    return true;
            }
        }

        return false;
    }

    void EnterEvadeMode() override
    {
        BossAI::EnterEvadeMode();
        DoCastSpellIfCan(nullptr, SPELL_RESET_GOTHIK_EVENT, CAST_TRIGGERED); // Prevent more adds from spawing

        m_creature->SetRespawnDelay(30, true);
        m_creature->ForcedDespawn();
    }

    void JustSummoned(Creature* summoned) override
    {
        if (HasPlayersInLeftSide())
            summoned->SetInCombatWithZone();
        m_summonedAddGuids.push_back(summoned->GetObjectGuid());
    }

    void SummonedCreatureJustDied(Creature* summoned) override
    {
        switch (summoned->GetEntry())
        {
            // Wrong caster, it expected to be pSummoned.
            // Mangos deletes the spell event at caster death, so for delayed spell like this
            // it's just a workaround. Does not affect other than the visual though (+ spell takes longer to "travel")
            case NPC_UNREL_TRAINEE:         summoned->CastSpell(nullptr, SPELL_A_TO_ANCHOR_1, TRIGGERED_OLD_TRIGGERED); break;
            case NPC_UNREL_DEATH_KNIGHT:    summoned->CastSpell(nullptr, SPELL_B_TO_ANCHOR_1, TRIGGERED_OLD_TRIGGERED); break;
            case NPC_UNREL_RIDER:           summoned->CastSpell(nullptr, SPELL_C_TO_ANCHOR_1, TRIGGERED_OLD_TRIGGERED); break;
        }
    }

    void StartTraineeSummons()
    {
        m_creature->CastSpell(nullptr, SPELL_SUMMON_TRAINEE_PERIODIC, TRIGGERED_IGNORE_CURRENT_CASTED_SPELL);
    }

    void StartKnightSummons()
    {
        m_creature->CastSpell(nullptr, SPELL_SUMMON_KNIGHT_PERIODIC, TRIGGERED_IGNORE_CURRENT_CASTED_SPELL);
    }

    void StartRiderSummons()
    {
        m_creature->CastSpell(nullptr, SPELL_SUMMON_MOUNTED_KNIGHT_PERIODIC, TRIGGERED_IGNORE_CURRENT_CASTED_SPELL);
    }

    void HandleOpenGates()
    {
        ProcessCentralDoor();
        Map::PlayerList const& players = m_instance->instance->GetPlayers();
        for (const auto& playerRef : players)
        {
            if (Player* player = playerRef.getSource())
            {
                for (ObjectGuid guid : m_summonedAddGuids)
                {
                    if (Creature* add = m_creature->GetMap()->GetCreature(guid))
                    {
                        // attack other side
                        if (add->IsAlive() && m_instance->IsInRightSideGothikArea(player) != m_instance->IsInRightSideGothikArea(add))
                        {
                            add->AddThreat(player);
                        }
                    }
                }
            }
        }

        for (ObjectGuid guid : m_summonedAddGuids)
        {
            if (Creature* add = m_creature->GetMap()->GetCreature(guid))
            {
                if (add->IsAlive() && !add->GetVictim()) // spawn that doesnt have target - attack closest
                    add->AI()->AttackClosestEnemy();
            }
        }
    }

    std::chrono::milliseconds GetSubsequentActionTimer(uint32 action)
    {
        switch (action)
        {
            case GOTHIK_TELEPORT: return 20s;
        }
        return 0s;
    }

    void ExecuteAction(uint32 action) override
    {
        switch (action)
        {
            case GOTHIK_STOP_TELE:
            {
                if (m_teleportCount >= 4 || m_creature->GetHealthPercent() <= 30.0f)
                {
                    m_phase = PHASE_STOP_TELEPORTING;
                    HandleOpenGates();
                    DisableCombatAction(action);
                }
                return;
            }
            case GOTHIK_TELEPORT:
            {
                uint32 teleportSpell = m_instance->IsInRightSideGothikArea(m_creature) ? SPELL_TELEPORT_LEFT : SPELL_TELEPORT_RIGHT;
                if (DoCastSpellIfCan(nullptr, teleportSpell) == CAST_OK)
                {
                    ++m_teleportCount;
                    break;
                }
                return;
            }
        }
        ResetCombatAction(action, GetSubsequentActionTimer(action));
    }
};

// 27892, 27928, 27935 - To Anchor 1
struct ToAnchorOne : public SpellScript
{
    void OnEffectExecute(Spell* spell, SpellEffectIndex effIdx) const override
    {
        if (effIdx != EFFECT_INDEX_0 || !spell->GetUnitTarget())
            return;

        instance_naxxramas* instance = dynamic_cast<instance_naxxramas*>(spell->GetCaster()->GetInstanceData());
        if (!instance)
            return;

        uint32 nextAnchor = 0;
        switch (spell->m_spellInfo->Id)
        {
            case SPELL_A_TO_ANCHOR_1: nextAnchor = SPELL_A_TO_ANCHOR_2; break;
            case SPELL_B_TO_ANCHOR_1: nextAnchor = SPELL_B_TO_ANCHOR_2; break;
            case SPELL_C_TO_ANCHOR_1: nextAnchor = SPELL_C_TO_ANCHOR_2; break;
        }

        if (nextAnchor)
            spell->GetUnitTarget()->CastSpell(nullptr, nextAnchor, TRIGGERED_OLD_TRIGGERED);
    }
};

// 27893, 27929, 27936 - To Anchor 2
struct ToAnchorTwo : public SpellScript
{
    void OnInit(Spell* spell) const override
    {
        spell->SetMaxAffectedTargets(1);
    }

    void OnEffectExecute(Spell* spell, SpellEffectIndex effIdx) const override
    {
        if (effIdx != EFFECT_INDEX_0 || !spell->GetUnitTarget())
            return;

        instance_naxxramas* instance = dynamic_cast<instance_naxxramas*>(spell->GetCaster()->GetInstanceData());
        if (!instance)
            return;

        uint32 choiceId = 0;
        switch (spell->m_spellInfo->Id)
        {
            case SPELL_A_TO_ANCHOR_2: choiceId = SPELL_CHOOSE_RANDOM_SKULL_PILE_A; break;
            case SPELL_B_TO_ANCHOR_2: choiceId = SPELL_CHOOSE_RANDOM_SKULL_PILE_B; break;
            case SPELL_C_TO_ANCHOR_2: choiceId = SPELL_CHOOSE_RANDOM_SKULL_PILE_C; break;
        }

        if (choiceId)
            spell->GetUnitTarget()->CastSpell(nullptr, choiceId, TRIGGERED_OLD_TRIGGERED);
    }
};

// 27896, 27930, 27938 - Choose Random Skull Pile
struct ChooseRandomSkullPile : public SpellScript
{
    void OnEffectExecute(Spell* spell, SpellEffectIndex effIdx) const override
    {
        if (effIdx != EFFECT_INDEX_0 || !spell->GetUnitTarget())
            return;

        uint32 nextAnchor = 0;
        switch (spell->m_spellInfo->Id)
        {
            case SPELL_CHOOSE_RANDOM_SKULL_PILE_A: nextAnchor = SPELL_A_TO_SKULL; break;
            case SPELL_CHOOSE_RANDOM_SKULL_PILE_B: nextAnchor = SPELL_B_TO_SKULL; break;
            case SPELL_CHOOSE_RANDOM_SKULL_PILE_C: nextAnchor = SPELL_C_TO_SKULL; break;
        }

        if (nextAnchor)
            spell->GetCaster()->CastSpell(spell->GetUnitTarget(), nextAnchor, TRIGGERED_OLD_TRIGGERED);
    }
};

// 27915, 27931, 27937 - Anchor to Skulls
struct AnchorToSkull : public SpellScript
{
    void OnEffectExecute(Spell* spell, SpellEffectIndex effIdx) const override
    {
        if (effIdx != EFFECT_INDEX_0 || !spell->GetUnitTarget())
            return;

        instance_naxxramas* instance = dynamic_cast<instance_naxxramas*>(spell->GetCaster()->GetInstanceData());
        if (!instance)
            return;

        uint32 summonSpellId = 0;
        uint32 summonSpellId2 = 0;
        switch (spell->m_spellInfo->Id)
        {
            case SPELL_A_TO_SKULL: summonSpellId = SPELL_SUMMON_SPECTRAL_TRAINEE; break;
            case SPELL_B_TO_SKULL: summonSpellId = SPELL_SUMMON_SPECTRAL_KNIGHT; break;
            case SPELL_C_TO_SKULL: summonSpellId = SPELL_SUMMON_SPECTRAL_RIVENDARE; break;
        }

        if (summonSpellId)
            spell->GetUnitTarget()->CastSpell(nullptr, summonSpellId, TRIGGERED_OLD_TRIGGERED);
    }
};

// 28749 - Check Spectral Side
// 29875 - Check Unrelenting Side
struct CheckGothikSide : public SpellScript
{
    void OnEffectExecute(Spell* spell, SpellEffectIndex /* effIdx */) const override
    {
        if (Unit* caster = spell->GetCaster())
        {
            if (instance_naxxramas* instance = (instance_naxxramas*)caster->GetInstanceData())
            {
                // If central gate is open, attack any one (central gate is only closed during IN_PROGRESS)
                if (instance->GetData(TYPE_GOTHIK) != IN_PROGRESS)
                {
                    static_cast<Creature*>(caster)->SetInCombatWithZone();
                    caster->AI()->AttackClosestEnemy();
                }
                // Else look for a target in the side the summoned NPC is
                else
                {
                    Map::PlayerList const& playersList = instance->instance->GetPlayers();
                    if (playersList.isEmpty())
                        return;

                    for (const auto& playerInList : playersList)
                    {
                        if (Player* player = playerInList.getSource())
                        {
                            if (!player->IsAlive())
                                return;

                            if (spell->m_spellInfo->Id == SPELL_CHECK_UNRELENTING_SIDE && instance->IsInRightSideGothikArea(player))
                                caster->AI()->AttackClosestEnemy();
                            else if (spell->m_spellInfo->Id == SPELL_CHECK_SPECTRAL_SIDE && !instance->IsInRightSideGothikArea(player))
                                caster->AI()->AttackClosestEnemy();
                            else
                                return;
                        }
                    }
                }
            }
        }
    }
};

// 28781 - Spectral Side Assault
// 29874 - Unrelenting Side Assault
struct GothikSideAssault : public SpellScript
{
    void OnEffectExecute(Spell* spell, SpellEffectIndex /* effIdx */) const override
    {
        if (Unit* unitTarget = spell->GetUnitTarget())
        {
            if (instance_naxxramas* instance = (instance_naxxramas*)unitTarget->GetInstanceData())
            {
                // If for some reason one ore more creatures are not in combat when the central gates open
                // Set them in combat with zone
                if (!unitTarget->IsInCombat() && unitTarget->IsCreature())
                {
                    static_cast<Creature*>(unitTarget)->SetInCombatWithZone();
                    unitTarget->AI()->AttackClosestEnemy();
                }
            }
        }
    }
};

// 28035 - Reset Gothik Event
struct ResetGothikEvent : public SpellScript
{
    void OnEffectExecute(Spell* spell, SpellEffectIndex /*effIdx*/) const override
    {
        Unit* target = spell->GetUnitTarget();
        if (!target->IsCreature())
            return;
        target->RemoveAurasDueToSpell(SPELL_SUMMON_TRAINEE_PERIODIC);
        target->RemoveAurasDueToSpell(SPELL_SUMMON_KNIGHT_PERIODIC);
        target->RemoveAurasDueToSpell(SPELL_SUMMON_MOUNTED_KNIGHT_PERIODIC);
    }
};

// 29317 - Shadowbolt
struct ShadowboltGothik : public SpellScript
{
    void OnEffectExecute(Spell* spell, SpellEffectIndex effIdx) const override
    {
        if (effIdx == EFFECT_INDEX_1 && spell->GetScriptValue() != 1)
        {
            if (urand(0, 1) == 0) // not always
                return;
            Unit* caster = spell->GetCaster();
            Unit* target = caster->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0, 29317, SELECT_FLAG_PLAYER);
            if (target)
            {
                SpellCastArgs args;
                args.SetTarget(target);
                args.SetScriptValue(1);
                caster->CastSpell(args, spell->m_spellInfo, TRIGGERED_IGNORE_COOLDOWNS | TRIGGERED_IGNORE_CURRENT_CASTED_SPELL | TRIGGERED_IGNORE_GCD, nullptr, nullptr, ObjectGuid(), spell->m_spellInfo);
            }
        }
    }
};

void AddSC_boss_gothik()
{
    Script* newScript = new Script;
    newScript->Name = "boss_gothik";
    newScript->GetAI = &GetNewAIInstance<boss_gothikAI>;
    newScript->RegisterSelf();

    RegisterSpellScript<ToAnchorOne>("spell_to_anchor_one");
    RegisterSpellScript<ToAnchorTwo>("spell_to_anchor_two");
    RegisterSpellScript<ChooseRandomSkullPile>("spell_choose_random_skull_pile");
    RegisterSpellScript<AnchorToSkull>("spell_anchor_to_skull");
    RegisterSpellScript<CheckGothikSide>("spell_check_gothik_side");
    RegisterSpellScript<GothikSideAssault>("spell_gothik_side_assault");
    RegisterSpellScript<ResetGothikEvent>("spell_reset_gothik_event");
    RegisterSpellScript<ShadowboltGothik>("spell_shadowbolt_gothik");
}
