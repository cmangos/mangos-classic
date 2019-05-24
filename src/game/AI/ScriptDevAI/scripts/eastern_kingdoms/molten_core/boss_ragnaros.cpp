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
SDName: Boss_Ragnaros
SD%Complete: 99
SDComment: Submerge emote is not played
SDCategory: Molten Core
EndScriptData

*/

#include "AI/ScriptDevAI/include/precompiled.h"
#include "molten_core.h"

enum
{
    SAY_ARRIVAL5_RAG            = -1409012,
    SAY_REINFORCEMENTS_1        = -1409013,
    SAY_REINFORCEMENTS_2        = -1409014,
    SAY_HAMMER                  = -1409015,
    SAY_WRATH                   = -1409016,
    SAY_KILL                    = -1409017,
    SAY_MAGMABURST              = -1409018,

    SPELL_WRATH_OF_RAGNAROS     = 20566,
    SPELL_ELEMENTAL_FIRE        = 20563,                    // Aura, proc spell 20564 when doing melee damage
    SPELL_MAGMA_BLAST           = 20565,                    // Ranged attack if nobody is in melee range
    SPELL_MELT_WEAPON           = 21387,
    SPELL_RAGNA_SUBMERGE        = 21107,                    // Stealth aura
    SPELL_RAGNA_SUBMERGE_VISUAL = 20567,                    // Visual for submerging into lava
    SPELL_RAGNA_EMERGE          = 20568,                    // Emerge from lava
    SPELL_SUBMERGE_EFFECT       = 21859,                    // Make Ragnaros immune and passive while submerged
    SPELL_ELEMENTAL_FIRE_KILL   = 19773,
    SPELL_MIGHT_OF_RAGNAROS     = 21154,
    SPELL_INTENSE_HEAT          = 21155,
    SPELL_LAVA_BURST            = 21908,                    // Randomly trigger one of spells 21886, 21900 - 21907 which summons Go 178088
    SPELL_SUMMON_SONS_FLAME     = 21108,                    // Trigger the eight spells summoning the Son of Flame adds

    NB_ADDS_IN_SUBMERGE         = 8,
    NPC_SON_OF_FLAME            = 12143,
    NPC_FLAME_OF_RAGNAROS       = 13148,

    PHASE_EMERGING              = 0,
    PHASE_EMERGED               = 1,
    PHASE_SUBMERGING            = 2,
    PHASE_SUBMERGED             = 3
};

struct boss_ragnarosAI : public Scripted_NoMovementAI
{
    boss_ragnarosAI(Creature* pCreature) : Scripted_NoMovementAI(pCreature)
    {
        m_pInstance = (instance_molten_core*)pCreature->GetInstanceData();
        m_uiEnterCombatTimer = 0;
        m_bHasAggroYelled = false;
        Reset();
    }

    instance_molten_core* m_pInstance;

    uint32 m_uiEnterCombatTimer;
    uint32 m_uiWrathOfRagnarosTimer;
    uint32 m_uiHammerTimer;
    uint32 m_uiMagmaBlastTimer;
    uint32 m_uiLavaBurstTimer;
    uint32 m_uiNextPhaseTimer;
    uint32 m_uiAddCount;
    uint32 m_uiPhase;

    bool m_bHasAggroYelled;
    bool m_bHasYelledMagmaBurst;
    bool m_bHasSubmergedOnce;

    void Reset() override
    {
        m_uiWrathOfRagnarosTimer = 30000;
        m_uiHammerTimer = 11000;
        m_uiMagmaBlastTimer = 2000;
        m_uiLavaBurstTimer = 20 * IN_MILLISECONDS;
        m_uiNextPhaseTimer = 3 * MINUTE * IN_MILLISECONDS;
        m_uiAddCount = 0;
        m_uiPhase = PHASE_EMERGED;

        m_bHasYelledMagmaBurst = false;
        m_bHasSubmergedOnce = false;
    }

    void KilledUnit(Unit* pVictim) override
    {
        if (pVictim->GetTypeId() != TYPEID_PLAYER)
            return;

        if (urand(0, 3))
            return;

        DoScriptText(SAY_KILL, m_creature);
    }

    void JustDied(Unit* /*pKiller*/) override
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_RAGNAROS, DONE);
    }

    void Aggro(Unit* pWho) override
    {
        if (pWho->GetTypeId() == TYPEID_UNIT && pWho->GetEntry() == NPC_MAJORDOMO)
            return;

        DoCastSpellIfCan(m_creature, SPELL_MELT_WEAPON);
        DoCastSpellIfCan(m_creature, SPELL_ELEMENTAL_FIRE);

        if (m_pInstance)
            m_pInstance->SetData(TYPE_RAGNAROS, IN_PROGRESS);
    }

    void EnterEvadeMode() override
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_RAGNAROS, FAIL);

        // Reset flag if Ragnaros had been submerged
        if (m_uiPhase != PHASE_EMERGED)
            DoCastSpellIfCan(m_creature, SPELL_RAGNA_EMERGE);
        if (m_creature->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE))
            m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);

        ScriptedAI::EnterEvadeMode();
    }

    void SummonedCreatureJustDied(Creature* pSummmoned) override
    {
        // If all Sons of Flame are dead, trigger emerge
        if (pSummmoned->GetEntry() == NPC_SON_OF_FLAME)
        {
            m_uiAddCount--;

            // If last add killed then emerge soonish
            if (m_uiAddCount == 0)
                m_uiNextPhaseTimer = std::min(m_uiNextPhaseTimer, (uint32)1000);
        }
    }

    void JustSummoned(Creature* pSummoned) override
    {
        if (pSummoned->GetEntry() == NPC_SON_OF_FLAME)
        {
            if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
                pSummoned->AI()->AttackStart(pTarget);
        }
        else if (pSummoned->GetEntry() == NPC_FLAME_OF_RAGNAROS)
            pSummoned->CastSpell(pSummoned, SPELL_INTENSE_HEAT, TRIGGERED_OLD_TRIGGERED, nullptr, nullptr, m_creature->GetObjectGuid());
    }

    void SpellHitTarget(Unit* pTarget, const SpellEntry* pSpell) override
    {
        // As Majordomo is now killed, the last timer (until attacking) must be handled with ragnaros script
        if (pSpell->Id == SPELL_ELEMENTAL_FIRE_KILL && pTarget->GetTypeId() == TYPEID_UNIT && pTarget->GetEntry() == NPC_MAJORDOMO)
            m_uiEnterCombatTimer = 10000;
    }

    // Custom threat management for targets in melee range
    bool CanMeleeTargetInRange()
    {
        // If a target is found in melee range (descending threat), attack it
        if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_TOPAGGRO, 0, uint32(0), SELECT_FLAG_IN_MELEE_RANGE))
        {
            // Target is not current victim, force select and attack it
            if (pTarget != m_creature->getVictim())
            {
                AttackStart(pTarget);
                m_creature->SetInFront(pTarget);
            }
            // Make sure our attack is ready
            if (m_creature->isAttackReady())
            {
                m_creature->AttackerStateUpdate(pTarget);
                m_creature->resetAttackTimer();
                m_bHasYelledMagmaBurst = false;
            }
            return true;
        }
        return false;
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        // Introduction
        if (m_uiEnterCombatTimer)
        {
            if (m_uiEnterCombatTimer <=  uiDiff)
            {
                if (!m_bHasAggroYelled)
                {
                    m_uiEnterCombatTimer = 3000;
                    m_bHasAggroYelled = true;
                    DoScriptText(SAY_ARRIVAL5_RAG, m_creature);
                }
                else
                {
                    m_uiEnterCombatTimer = 0;
                    // If we don't remove this passive flag, he will be unattackable after evading, this way he will enter combat
                    m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PLAYER);
                    if (m_pInstance)
                    {
                        if (Player* pPlayer = m_pInstance->GetPlayerInMap(true, false))
                        {
                            m_creature->AI()->AttackStart(pPlayer);
                            return;
                        }
                    }
                }
            }
            else
                m_uiEnterCombatTimer -= uiDiff;
        }

        // Regular fight
        // Return since we have no target
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        // Update phase switching timer
        m_uiNextPhaseTimer -= uiDiff;

        switch (m_uiPhase)
        {
            case PHASE_EMERGED:
            {
                // Wrath Of Ragnaros Timer
                if (m_uiWrathOfRagnarosTimer < uiDiff)
                {
                    if (DoCastSpellIfCan(m_creature, SPELL_WRATH_OF_RAGNAROS) == CAST_OK)
                    {
                        DoScriptText(SAY_WRATH, m_creature);
                        m_uiWrathOfRagnarosTimer = 25000;
                    }
                }
                else
                    m_uiWrathOfRagnarosTimer -= uiDiff;

                // Lava Burst Timer
                if (m_uiLavaBurstTimer < uiDiff)
                {
                    if (DoCastSpellIfCan(m_creature, SPELL_LAVA_BURST) == CAST_OK)
                        m_uiLavaBurstTimer = urand(5000, 25000);                   // Upper value is guess work based on videos. It could be up to 45 secs
                }
                else
                    m_uiLavaBurstTimer -= uiDiff;

                // Hammer of Ragnaros
                if (m_uiHammerTimer < uiDiff)
                {
                    if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0, SPELL_MIGHT_OF_RAGNAROS, SELECT_FLAG_POWER_MANA))
                    {
                        if (DoCastSpellIfCan(pTarget, SPELL_MIGHT_OF_RAGNAROS) == CAST_OK)
                        {
                            DoScriptText(SAY_HAMMER, m_creature);
                            m_uiHammerTimer = urand(11000, 30000);
                        }
                    }
                    else
                        m_uiHammerTimer = urand(11000, 30000);
                }
                else
                    m_uiHammerTimer -= uiDiff;

                // Submerge Timer
                if (m_uiNextPhaseTimer < uiDiff)
                {
                    // Submerge and attack again after 90 secs
                    DoCastSpellIfCan(m_creature, SPELL_RAGNA_SUBMERGE_VISUAL, CAST_TRIGGERED);
                    m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);

                    // Say dependend if first time or not
                    DoScriptText(!m_bHasSubmergedOnce ? SAY_REINFORCEMENTS_1 : SAY_REINFORCEMENTS_2, m_creature);
                    m_bHasSubmergedOnce = true;

                    // Summon 8 elementals around the boss
                    if (DoCastSpellIfCan(m_creature, SPELL_SUMMON_SONS_FLAME) == CAST_OK)
                        m_uiAddCount = NB_ADDS_IN_SUBMERGE;

                    m_uiNextPhaseTimer = 1000;
                    m_uiPhase = PHASE_SUBMERGING;

                    return;
                }

                // Range check for melee target, if nobody is found in range, then cast magma blast on random target
                // If we are within range melee the target (done in CanMeleeTargetInRange())
                if (m_creature->IsNonMeleeSpellCasted(false) || !m_creature->getVictim())
                    return;

                if (!CanMeleeTargetInRange())
                {
                    // Magma Burst Timer
                    if (m_uiMagmaBlastTimer < uiDiff)
                    {
                        if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
                        {
                            if (DoCastSpellIfCan(pTarget, SPELL_MAGMA_BLAST) == CAST_OK)
                            {
                                if (!m_bHasYelledMagmaBurst)
                                {
                                    DoScriptText(SAY_MAGMABURST, m_creature);
                                    m_bHasYelledMagmaBurst = true;
                                }
                                m_uiMagmaBlastTimer = 1000;         // Spam this!
                            }
                        }
                    }
                    else
                        m_uiMagmaBlastTimer -= uiDiff;
                }
                return;
            }
            case PHASE_SUBMERGING:
            {
                if (m_uiNextPhaseTimer < uiDiff)
                {
                    m_creature->HandleEmote(EMOTE_STATE_SUBMERGED);
                    DoCastSpellIfCan(m_creature, SPELL_RAGNA_SUBMERGE, CAST_TRIGGERED);
                    DoCastSpellIfCan(m_creature, SPELL_SUBMERGE_EFFECT, CAST_TRIGGERED);
                    m_uiNextPhaseTimer = 90 * IN_MILLISECONDS;
                    m_uiPhase = PHASE_SUBMERGED;
                }
                return;
            }
            case PHASE_SUBMERGED:
            {
                // Timer to check when Ragnaros should emerge (is set to soonish, when last add is killed)
                if (m_uiNextPhaseTimer < uiDiff)
                {
                    // Become emerged again
                    m_creature->HandleEmote(EMOTE_STATE_STAND);
                    m_creature->RemoveAurasDueToSpell(SPELL_RAGNA_SUBMERGE);
                    m_creature->RemoveAurasDueToSpell(SPELL_SUBMERGE_EFFECT);
                    m_uiNextPhaseTimer = 500;
                    m_uiPhase = PHASE_EMERGING;
                }

                // Do nothing while submerged
                return;
            }
            case PHASE_EMERGING:
            {
                if (m_uiNextPhaseTimer < uiDiff)
                {
                    DoCastSpellIfCan(m_creature, SPELL_RAGNA_EMERGE);
                    m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                    m_uiPhase = PHASE_EMERGED;
                    m_uiNextPhaseTimer = 3 * MINUTE * IN_MILLISECONDS;
                    m_uiMagmaBlastTimer = 3000;                 // Delay the magma blast after emerge
                }
            }
        }
    }
};

UnitAI* GetAI_boss_ragnaros(Creature* pCreature)
{
    return new boss_ragnarosAI(pCreature);
}

void AddSC_boss_ragnaros()
{
    Script* pNewScript = new Script;
    pNewScript->Name = "boss_ragnaros";
    pNewScript->GetAI = &GetAI_boss_ragnaros;
    pNewScript->RegisterSelf();
}
