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
    boss_gothikAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = (instance_naxxramas*)pCreature->GetInstanceData();
        Reset();
    }

    instance_naxxramas* m_pInstance;

    uint8 m_uiPhase;
    uint8 m_uiSpeech;
    uint8 m_teleportCount;
    uint8 m_summonStep;

    uint32 m_summonTimer;
    uint32 m_uiTeleportTimer;
    uint32 m_uiShadowboltTimer;
    uint32 m_uiHarvestSoulTimer;
    uint32 m_uiPhaseTimer;
    uint32 m_uiSpeechTimer;

    void Reset() override
    {
        // Remove immunity
        m_uiPhase = PHASE_SPEECH;
        m_uiSpeech = 1;

        m_uiTeleportTimer = urand(30, 45) * IN_MILLISECONDS; // Teleport every 30-45 seconds.
        m_uiShadowboltTimer = 2 * IN_MILLISECONDS;
        m_uiHarvestSoulTimer = 2500;
        m_uiPhaseTimer = (4 * MINUTE + 34) * IN_MILLISECONDS; // Teleport down at 4:34
        m_uiSpeechTimer = 1 * IN_MILLISECONDS;
        m_summonTimer = 0;
        m_teleportCount = 0;
        m_summonStep = STEP_TRAINEE;

        // Only attack and be attackable while on ground
        SetMeleeEnabled(false);
        SetCombatMovement(false);
        DoCastSpellIfCan(m_creature, SPELL_IMMUNE_ALL, TRIGGERED_OLD_TRIGGERED);
    }

    void Aggro(Unit* /*pWho*/) override
    {
        if (!m_pInstance)
            return;

        m_pInstance->SetData(TYPE_GOTHIK, IN_PROGRESS);
        // Make immune if not already
        if (!m_creature->HasAura(SPELL_IMMUNE_ALL))
            DoCastSpellIfCan(m_creature, SPELL_IMMUNE_ALL, TRIGGERED_OLD_TRIGGERED);

        // Start timer before summoning
        m_summonTimer = 4 * IN_MILLISECONDS;    // First spawn of trainees 24 secs after engage, but the periodic summoning aura already has an internal 20 secs timer
    }

    bool HasPlayersInLeftSide() const
    {
        Map::PlayerList const& lPlayers = m_pInstance->instance->GetPlayers();

        if (lPlayers.isEmpty())
            return false;

        for (const auto& lPlayer : lPlayers)
        {
            if (Player* pPlayer = lPlayer.getSource())
            {
                if (!m_pInstance->IsInRightSideGothikArea(pPlayer) && pPlayer->isAlive())
                    return true;
            }
        }

        return false;
    }

    void KilledUnit(Unit* pVictim) override
    {
        if (pVictim->GetTypeId() == TYPEID_PLAYER)
            DoScriptText(SAY_KILL, m_creature);
    }

    void JustDied(Unit* /*pKiller*/) override
    {
        DoScriptText(SAY_DEATH, m_creature);

        if (m_pInstance)
            m_pInstance->SetData(TYPE_GOTHIK, DONE);
    }

    void EnterEvadeMode() override
    {
        DoCastSpellIfCan(m_creature, SPELL_RESET_GOTHIK_EVENT, CAST_TRIGGERED); // Prevent more adds from spawing

        // Remove summoning and immune auras
        m_creature->RemoveAurasDueToSpell(SPELL_SUMMON_TRAINEE);
        m_creature->RemoveAurasDueToSpell(SPELL_SUMMON_KNIGHT);
        m_creature->RemoveAurasDueToSpell(SPELL_SUMMON_MOUNTED_KNIGHT);
        m_creature->RemoveAurasDueToSpell(SPELL_IMMUNE_ALL);

        if (m_pInstance)
            m_pInstance->SetData(TYPE_GOTHIK, FAIL);

        m_creature->ForcedDespawn();
        m_creature->Respawn();

        ScriptedAI::EnterEvadeMode();
    }

    void JustReachedHome() override
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_GOTHIK, FAIL);

        SetCombatMovement(false);
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        // Summoning auras handling
        if (m_summonTimer)
        {
            if (m_summonTimer < uiDiff)
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
                m_summonTimer -= uiDiff;
        }

        switch (m_uiPhase)
        {
            case PHASE_SPEECH:
                if (m_uiSpeechTimer < uiDiff)
                {
                    switch (m_uiSpeech)
                    {
                        case 1: DoScriptText(SAY_SPEECH_1, m_creature); m_uiSpeechTimer = 4 * IN_MILLISECONDS; break;
                        case 2: DoScriptText(SAY_SPEECH_2, m_creature); m_uiSpeechTimer = 6 * IN_MILLISECONDS; break;
                        case 3: DoScriptText(SAY_SPEECH_3, m_creature); m_uiSpeechTimer = 5 * IN_MILLISECONDS; break;
                        case 4: DoScriptText(SAY_SPEECH_4, m_creature); m_uiPhase = PHASE_BALCONY; break;
                    }
                    m_uiSpeech++;
                }
                else
                    m_uiSpeechTimer -= uiDiff;

            // No break here

            case PHASE_BALCONY:                            // Do nothing but wait to teleport down: summoning is handled by instance script
                if (m_uiPhaseTimer < uiDiff)
                {
                    m_uiPhase = PHASE_TELEPORT_DOWN;
                    m_uiPhaseTimer = 0;
                }
                else
                    m_uiPhaseTimer -= uiDiff;

                break;

            case PHASE_TELEPORT_DOWN:
                if (m_uiPhaseTimer < uiDiff)
                {
                    if (DoCastSpellIfCan(m_creature, SPELL_TELEPORT_RIGHT, CAST_TRIGGERED) == CAST_OK)
                    {
                        m_uiPhase = m_pInstance ? PHASE_TELEPORTING : PHASE_STOP_TELEPORTING;

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
                    m_uiPhaseTimer -= uiDiff;

                break;

            case PHASE_TELEPORTING:                         // Phase is only reached if m_pInstance is valid
                if (m_uiTeleportTimer < uiDiff)
                {
                    uint32 uiTeleportSpell = m_pInstance->IsInRightSideGothikArea(m_creature) ? SPELL_TELEPORT_LEFT : SPELL_TELEPORT_RIGHT;
                    if (DoCastSpellIfCan(m_creature, uiTeleportSpell) == CAST_OK)
                    {
                        m_uiTeleportTimer = urand(30, 45) * IN_MILLISECONDS; // Teleports between 30 seconds and 45 seconds.
                        m_uiShadowboltTimer = 2 * IN_MILLISECONDS;
                        ++m_teleportCount;
                    }
                }
                else
                    m_uiTeleportTimer -= uiDiff;

                // Second time that Gothik teleports back from dead side to living side: open the central gate
                if (m_teleportCount >= 4)
                {
                    m_uiPhase = PHASE_STOP_TELEPORTING;
                    m_pInstance->SetData(TYPE_GOTHIK, SPECIAL);
                }
            // no break here

            case PHASE_STOP_TELEPORTING:
                if (m_uiHarvestSoulTimer < uiDiff)
                {
                    if (DoCastSpellIfCan(m_creature, SPELL_HARVESTSOUL) == CAST_OK)
                        m_uiHarvestSoulTimer = 15 * IN_MILLISECONDS;
                }
                else
                    m_uiHarvestSoulTimer -= uiDiff;

                if (m_uiShadowboltTimer)
                {
                    if (m_uiShadowboltTimer <= uiDiff)
                        m_uiShadowboltTimer = 0;
                    else
                        m_uiShadowboltTimer -= uiDiff;
                }
                // Shadowbolt cooldown finished, cast when ready
                else if (!m_creature->IsNonMeleeSpellCasted(true))
                {
                    // Select valid target
                    if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0, SPELL_SHADOWBOLT, SELECT_FLAG_IN_LOS))
                        DoCastSpellIfCan(pTarget, SPELL_SHADOWBOLT);
                }
                break;
        }
    }
};

UnitAI* GetAI_boss_gothik(Creature* pCreature)
{
    return new boss_gothikAI(pCreature);
}

bool EffectDummyCreature_spell_anchor(Unit* /*pCaster*/, uint32 uiSpellId, SpellEffectIndex uiEffIndex, Creature* pCreatureTarget, ObjectGuid /*originalCasterGuid*/)
{
    if (uiEffIndex != EFFECT_INDEX_0 || pCreatureTarget->GetEntry() != NPC_SUB_BOSS_TRIGGER)
        return true;

    instance_naxxramas* pInstance = (instance_naxxramas*)pCreatureTarget->GetInstanceData();

    if (!pInstance)
        return true;

    switch (uiSpellId)
    {
        case SPELL_A_TO_ANCHOR_1:                           // trigger mobs at high right side
        case SPELL_B_TO_ANCHOR_1:
        case SPELL_C_TO_ANCHOR_1:
        {
            if (Creature* pAnchor2 = pInstance->GetClosestAnchorForGothik(pCreatureTarget, false))
            {
                uint32 uiTriggered = SPELL_A_TO_ANCHOR_2;

                if (uiSpellId == SPELL_B_TO_ANCHOR_1)
                    uiTriggered = SPELL_B_TO_ANCHOR_2;
                else if (uiSpellId == SPELL_C_TO_ANCHOR_1)
                    uiTriggered = SPELL_C_TO_ANCHOR_2;

                pCreatureTarget->CastSpell(pAnchor2, uiTriggered, TRIGGERED_OLD_TRIGGERED);
            }

            return true;
        }
        case SPELL_A_TO_ANCHOR_2:                           // trigger mobs at high left side
        case SPELL_B_TO_ANCHOR_2:
        case SPELL_C_TO_ANCHOR_2:
        {
            CreatureList lTargets;
            pInstance->GetGothikSummonPoints(lTargets, false);

            if (!lTargets.empty())
            {
                CreatureList::iterator itr = lTargets.begin();
                uint32 uiPosition = urand(0, lTargets.size() - 1);
                advance(itr, uiPosition);

                if (Creature* pTarget = (*itr))
                {
                    uint32 uiTriggered = SPELL_A_TO_SKULL;

                    if (uiSpellId == SPELL_B_TO_ANCHOR_2)
                        uiTriggered = SPELL_B_TO_SKULL;
                    else if (uiSpellId == SPELL_C_TO_ANCHOR_2)
                        uiTriggered = SPELL_C_TO_SKULL;

                    pCreatureTarget->CastSpell(pTarget, uiTriggered, TRIGGERED_OLD_TRIGGERED);
                }
            }
            return true;
        }
        case SPELL_A_TO_SKULL:                              // final destination trigger mob
        case SPELL_B_TO_SKULL:
        case SPELL_C_TO_SKULL:
        {
            if (Creature* pGoth = pInstance->GetSingleCreatureFromStorage(NPC_GOTHIK))
            {
                uint32 uiNpcEntry = NPC_SPECT_TRAINEE;

                if (uiSpellId == SPELL_B_TO_SKULL)
                    uiNpcEntry = NPC_SPECT_DEATH_KNIGHT;
                else if (uiSpellId == SPELL_C_TO_SKULL)
                    uiNpcEntry = NPC_SPECT_RIDER;

                pGoth->SummonCreature(uiNpcEntry, pCreatureTarget->GetPositionX(), pCreatureTarget->GetPositionY(), pCreatureTarget->GetPositionZ(), pCreatureTarget->GetOrientation(), TEMPSPAWN_DEAD_DESPAWN, 0);

                if (uiNpcEntry == NPC_SPECT_RIDER)
                    pGoth->SummonCreature(NPC_SPECT_HORSE, pCreatureTarget->GetPositionX(), pCreatureTarget->GetPositionY(), pCreatureTarget->GetPositionZ(), pCreatureTarget->GetOrientation(), TEMPSPAWN_DEAD_DESPAWN, 0);
            }
            return true;
        }
        case SPELL_RESET_GOTHIK_EVENT:
        {
            pCreatureTarget->RemoveAurasDueToSpell(SPELL_SUMMON_TRAINEE);
            pCreatureTarget->RemoveAurasDueToSpell(SPELL_SUMMON_KNIGHT);
            pCreatureTarget->RemoveAurasDueToSpell(SPELL_SUMMON_MOUNTED_KNIGHT);
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
                            if (!player->isAlive())
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
                if (!unitTarget->isInCombat())
                    ((Creature*)unitTarget)->SetInCombatWithZone();
                    unitTarget->AI()->AttackClosestEnemy();
            }
        }
    }
};

void AddSC_boss_gothik()
{
    Script* pNewScript = new Script;
    pNewScript->Name = "boss_gothik";
    pNewScript->GetAI = &GetAI_boss_gothik;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "spell_anchor";
    pNewScript->pEffectDummyNPC = &EffectDummyCreature_spell_anchor;
    pNewScript->RegisterSelf();

    RegisterAuraScript<SummonUnrelenting>("spell_summon_unrelenting");
    RegisterSpellScript<CheckGothikSide>("spell_check_gothik_side");
    RegisterSpellScript<GothikSideAssault>("spell_gothik_side_assault");
}
