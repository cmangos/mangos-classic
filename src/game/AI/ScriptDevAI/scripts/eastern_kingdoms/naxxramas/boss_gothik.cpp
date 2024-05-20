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
#include "naxxramas.h"

enum
{
    SAY_SPEECH_1                = -1533040,
    SAY_SPEECH_2                = -1533140,
    SAY_SPEECH_3                = -1533141,
    SAY_SPEECH_4                = -1533142,

    SAY_KILL                    = -1533041,
    SAY_DEATH                   = -1533042,
    SAY_TELEPORT                = -1533043,

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

struct boss_gothikAI : public ScriptedAI
{
    boss_gothikAI(Creature* creature) : ScriptedAI(creature)
    {
        m_instance = (instance_naxxramas*)creature->GetInstanceData();
        Reset();
    }

    instance_naxxramas* m_instance;

    uint8 m_phase;
    uint8 m_speech;
    uint8 m_teleportCount;
    uint8 m_summonStep;

    uint32 m_summonTimer;
    uint32 m_teleportTimer;
    uint32 m_shadowboltTimer;
    uint32 m_harvestSoulTimer;
    uint32 m_phaseTimer;
    uint32 m_speechTimer;

    void Reset() override
    {
        // Remove immunity
        m_phase = PHASE_SPEECH;
        m_speech = 1;

        m_teleportTimer = urand(30, 45) * IN_MILLISECONDS; // Teleport every 30-45 seconds.
        m_shadowboltTimer = 2 * IN_MILLISECONDS;
        m_harvestSoulTimer = 2500;
        m_phaseTimer = (4 * MINUTE + 34) * IN_MILLISECONDS; // Teleport down at 4:34
        m_speechTimer = 1 * IN_MILLISECONDS;
        m_summonTimer = 0;
        m_teleportCount = 0;
        m_summonStep = STEP_TRAINEE;

        // Only attack and be attackable while on ground
        SetMeleeEnabled(false);
        SetCombatMovement(false);
        DoCastSpellIfCan(m_creature, SPELL_IMMUNE_ALL, TRIGGERED_OLD_TRIGGERED);
    }

    void Aggro(Unit* /*who*/) override
    {
        if (!m_instance)
            return;

        m_instance->SetData(TYPE_GOTHIK, IN_PROGRESS);
        // Make immune if not already
        if (!m_creature->HasAura(SPELL_IMMUNE_ALL))
            DoCastSpellIfCan(m_creature, SPELL_IMMUNE_ALL, TRIGGERED_OLD_TRIGGERED);

        // Start timer before summoning
        m_summonTimer = 4 * IN_MILLISECONDS;    // First spawn of trainees 24 secs after engage, but the periodic summoning aura already has an internal 20 secs timer
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

    void KilledUnit(Unit* victim) override
    {
        if (victim->GetTypeId() == TYPEID_PLAYER)
            DoScriptText(SAY_KILL, m_creature);
    }

    void JustDied(Unit* /*killer*/) override
    {
        DoScriptText(SAY_DEATH, m_creature);

        if (m_instance)
            m_instance->SetData(TYPE_GOTHIK, DONE);
    }

    void EnterEvadeMode() override
    {
        DoCastSpellIfCan(m_creature, SPELL_RESET_GOTHIK_EVENT, CAST_TRIGGERED); // Prevent more adds from spawing

        // Remove summoning and immune auras
        m_creature->RemoveAurasDueToSpell(SPELL_SUMMON_TRAINEE);
        m_creature->RemoveAurasDueToSpell(SPELL_SUMMON_KNIGHT);
        m_creature->RemoveAurasDueToSpell(SPELL_SUMMON_MOUNTED_KNIGHT);
        m_creature->RemoveAurasDueToSpell(SPELL_IMMUNE_ALL);

        if (m_instance)
            m_instance->SetData(TYPE_GOTHIK, FAIL);

        m_creature->SetRespawnDelay(30, true);
        m_creature->ForcedDespawn();
    }

    void UpdateAI(const uint32 diff) override
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->GetVictim())
            return;

        // Summoning auras handling
        if (m_summonTimer)
        {
            if (m_summonTimer < diff)
            {
                switch (m_summonStep) {
                    case STEP_TRAINEE:

                        if (DoCastSpellIfCan(m_creature, SPELL_SUMMON_TRAINEE, CAST_TRIGGERED) == CAST_OK)
                        {
                            m_summonTimer = 45 * IN_MILLISECONDS;
                            ++m_summonStep;
                        }
                        break;
                    case STEP_KNIGHT:
                        if (DoCastSpellIfCan(m_creature, SPELL_SUMMON_KNIGHT, CAST_TRIGGERED) == CAST_OK)
                        {
                            m_summonTimer = 55 * IN_MILLISECONDS;
                            ++m_summonStep;
                        }
                        break;
                    case STEP_RIDER:
                        if (DoCastSpellIfCan(m_creature, SPELL_SUMMON_MOUNTED_KNIGHT, CAST_TRIGGERED) == CAST_OK)
                        {
                            m_summonTimer = 0;
                            m_summonStep = 0;
                        }
                        break;
                    default:
                        break;
                }
            }
            else
                m_summonTimer -= diff;
        }

        switch (m_phase)
        {
            case PHASE_SPEECH:
                if (m_speechTimer < diff)
                {
                    switch (m_speech)
                    {
                        case 1: DoScriptText(SAY_SPEECH_1, m_creature); m_speechTimer = 4 * IN_MILLISECONDS; break;
                        case 2: DoScriptText(SAY_SPEECH_2, m_creature); m_speechTimer = 6 * IN_MILLISECONDS; break;
                        case 3: DoScriptText(SAY_SPEECH_3, m_creature); m_speechTimer = 5 * IN_MILLISECONDS; break;
                        case 4: DoScriptText(SAY_SPEECH_4, m_creature); m_phase = PHASE_BALCONY; break;
                    }
                    m_speech++;
                }
                else
                    m_speechTimer -= diff;

            // No break here

            case PHASE_BALCONY:                            // Do nothing but wait to teleport down: summoning is handled by instance script
                if (m_phaseTimer < diff)
                {
                    m_phase = PHASE_TELEPORT_DOWN;
                    m_phaseTimer = 0;
                }
                else
                    m_phaseTimer -= diff;

                break;

            case PHASE_TELEPORT_DOWN:
                if (m_phaseTimer < diff)
                {
                    if (DoCastSpellIfCan(m_creature, SPELL_TELEPORT_RIGHT, CAST_TRIGGERED) == CAST_OK)
                    {
                        m_phase = m_instance ? PHASE_TELEPORTING : PHASE_STOP_TELEPORTING;

                        DoScriptText(SAY_TELEPORT, m_creature);

                        // Remove Immunity
                        m_creature->RemoveAurasDueToSpell(SPELL_IMMUNE_ALL);
                        SetMeleeEnabled(true);
                        SetCombatMovement(true);
                        DoResetThreat();
                        m_creature->SetInCombatWithZone();
                    }
                }
                else
                    m_phaseTimer -= diff;

                break;

            case PHASE_TELEPORTING:                         // Phase is only reached if m_instance is valid
                if (m_teleportTimer < diff)
                {
                    uint32 teleportSpellId = m_instance->IsInRightSideGothikArea(m_creature) ? SPELL_TELEPORT_LEFT : SPELL_TELEPORT_RIGHT;
                    if (DoCastSpellIfCan(m_creature, teleportSpellId) == CAST_OK)
                    {
                        m_teleportTimer = urand(30, 45) * IN_MILLISECONDS; // Teleports between 30 seconds and 45 seconds.
                        m_shadowboltTimer = 2 * IN_MILLISECONDS;
                        ++m_teleportCount;
                    }
                }
                else
                    m_teleportTimer -= diff;

                // Second time that Gothik teleports back from dead side to living side or less than 30% HP: open the central gate
                if (m_teleportCount >= 4 ||  m_creature->GetHealthPercent() <= 30.0f)
                {
                    m_phase = PHASE_STOP_TELEPORTING;
                    m_instance->SetData(TYPE_GOTHIK, SPECIAL);
                }
            // no break here

            case PHASE_STOP_TELEPORTING:
                if (m_harvestSoulTimer < diff)
                {
                    if (DoCastSpellIfCan(m_creature, SPELL_HARVESTSOUL) == CAST_OK)
                        m_harvestSoulTimer = 15 * IN_MILLISECONDS;
                }
                else
                    m_harvestSoulTimer -= diff;

                if (m_shadowboltTimer)
                {
                    if (m_shadowboltTimer <= diff)
                        m_shadowboltTimer = 0;
                    else
                        m_shadowboltTimer -= diff;
                }
                // Shadowbolt cooldown finished, cast when ready
                else if (!m_creature->IsNonMeleeSpellCasted(true))
                {
                    // Select valid target
                    if (Unit* target = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0, SPELL_SHADOWBOLT, SELECT_FLAG_IN_LOS))
                        DoCastSpellIfCan(target, SPELL_SHADOWBOLT);
                }
                break;
        }
    }
};

UnitAI* GetAI_boss_gothik(Creature* creature)
{
    return new boss_gothikAI(creature);
}

bool EffectDummyCreature_spell_anchor(Unit* /*caster*/, uint32 spellId, SpellEffectIndex effIndex, Creature* creatureTarget, ObjectGuid /*originalCasterGuid*/)
{
    if (effIndex != EFFECT_INDEX_0 || creatureTarget->GetEntry() != NPC_SUB_BOSS_TRIGGER)
        return true;

    instance_naxxramas* instance = (instance_naxxramas*)creatureTarget->GetInstanceData();

    if (!instance)
        return true;

    switch (spellId)
    {
        case SPELL_A_TO_ANCHOR_1:                           // trigger mobs at high right side
        case SPELL_B_TO_ANCHOR_1:
        case SPELL_C_TO_ANCHOR_1:
        {
            if (Creature* pAnchor2 = instance->GetClosestAnchorForGothik(creatureTarget, false))
            {
                uint32 triggered = SPELL_A_TO_ANCHOR_2;

                if (spellId == SPELL_B_TO_ANCHOR_1)
                    triggered = SPELL_B_TO_ANCHOR_2;
                else if (spellId == SPELL_C_TO_ANCHOR_1)
                    triggered = SPELL_C_TO_ANCHOR_2;

                creatureTarget->CastSpell(pAnchor2, triggered, TRIGGERED_OLD_TRIGGERED);
            }

            return true;
        }
        case SPELL_A_TO_ANCHOR_2:                           // trigger mobs at high left side
        case SPELL_B_TO_ANCHOR_2:
        case SPELL_C_TO_ANCHOR_2:
        {
            CreatureList listTargets;
            instance->GetGothikSummonPoints(listTargets, false);

            if (!listTargets.empty())
            {
                CreatureList::iterator itr = listTargets.begin();
                uint32 position = urand(0, listTargets.size() - 1);
                advance(itr, position);

                if (Creature* target = (*itr))
                {
                    uint32 triggered = SPELL_A_TO_SKULL;

                    if (spellId == SPELL_B_TO_ANCHOR_2)
                        triggered = SPELL_B_TO_SKULL;
                    else if (spellId == SPELL_C_TO_ANCHOR_2)
                        triggered = SPELL_C_TO_SKULL;

                    creatureTarget->CastSpell(target, triggered, TRIGGERED_OLD_TRIGGERED);
                }
            }
            return true;
        }
        case SPELL_A_TO_SKULL:                              // final destination trigger mob
        case SPELL_B_TO_SKULL:
        case SPELL_C_TO_SKULL:
        {
            uint32 summonSpellId = SPELL_SUMMON_SPECTRAL_TRAINEE;

            if (spellId == SPELL_B_TO_SKULL)
                summonSpellId = SPELL_SUMMON_SPECTRAL_KNIGHT;
            else if (spellId == SPELL_C_TO_SKULL)
                summonSpellId = SPELL_SUMMON_SPECTRAL_RIVENDARE;

            creatureTarget->CastSpell(creatureTarget, summonSpellId, TRIGGERED_OLD_TRIGGERED);

            return true;
        }
        case SPELL_RESET_GOTHIK_EVENT:
        {
            creatureTarget->RemoveAurasDueToSpell(SPELL_SUMMON_TRAINEE);
            creatureTarget->RemoveAurasDueToSpell(SPELL_SUMMON_KNIGHT);
            creatureTarget->RemoveAurasDueToSpell(SPELL_SUMMON_MOUNTED_KNIGHT);
            return true;
        }

    }

    return true;
};

struct SummonUnrelenting : public AuraScript
{
    void OnPeriodicTrigger(Aura* aura, PeriodicTriggerData& data) const override
    {
        if (Unit* unitTarget =  aura->GetTarget())
        {
            if (instance_naxxramas* instance = (instance_naxxramas*)unitTarget->GetInstanceData())
            {
                if (unitTarget->GetTypeId() == TYPEID_UNIT && unitTarget->GetEntry() == NPC_SUB_BOSS_TRIGGER)
                {
                    uint8 summonMask = 0;
                    switch (aura->GetId())
                    {
                        case SPELL_SUMMON_TRAINEE:
                            summonMask = SUMMON_FLAG_TRAINEE;
                            break;
                        case SPELL_SUMMON_KNIGHT:
                            summonMask = SUMMON_FLAG_KNIGHT;
                            break;
                        case SPELL_SUMMON_MOUNTED_KNIGHT:
                            summonMask = SUMMON_FLAG_RIDER;
                            break;
                        default:
                            break;
                    }
                    if (instance->IsSuitableTriggerForSummon(unitTarget, summonMask))
                        return;
                }
            }
        }
        data.spellInfo = nullptr;
    }
};

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
                    ((Creature*)caster)->SetInCombatWithZone();
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
                if (!unitTarget->IsInCombat())
                {
                    ((Creature*)unitTarget)->SetInCombatWithZone();
                    unitTarget->AI()->AttackClosestEnemy();
                }
            }
        }
    }
};

void AddSC_boss_gothik()
{
    Script* newScript = new Script;
    newScript->Name = "boss_gothik";
    newScript->GetAI = &GetAI_boss_gothik;
    newScript->RegisterSelf();

    newScript = new Script;
    newScript->Name = "spell_anchor";
    newScript->pEffectDummyNPC = &EffectDummyCreature_spell_anchor;
    newScript->RegisterSelf();

    RegisterSpellScript<SummonUnrelenting>("spell_summon_unrelenting");
    RegisterSpellScript<CheckGothikSide>("spell_check_gothik_side");
    RegisterSpellScript<GothikSideAssault>("spell_gothik_side_assault");
}
