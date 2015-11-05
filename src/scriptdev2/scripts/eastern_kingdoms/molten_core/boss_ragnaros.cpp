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
SD%Complete: 70
SDComment: Melee/ Range Combat behavior is not correct(any enemy in melee range, not only getVictim), Some abilities are missing
SDCategory: Molten Core
EndScriptData */

#include "precompiled.h"
#include "molten_core.h"

/* There have been quite some bugs about his spells, keep this as reference untill all finished
 * Missing features (based on wowwiki)
 *   Lava Burst - this spell is handled by Go 178088 which is summoned by spells 21886, 21900 - 21907
 */

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
    SPELL_ELEMENTAL_FIRE        = 20564,
    SPELL_MAGMA_BLAST           = 20565,                    // Ranged attack if nobody is in melee range
    SPELL_MELT_WEAPON           = 21387,
    SPELL_RAGNA_SUBMERGE        = 21107,                    // Stealth aura
    SPELL_RAGNA_EMERGE          = 20568,                    // Emerge from lava
    SPELL_ELEMENTAL_FIRE_KILL   = 19773,
    SPELL_MIGHT_OF_RAGNAROS     = 21154,
    SPELL_INTENSE_HEAT          = 21155,

    MAX_ADDS_IN_SUBMERGE        = 8,
    NPC_SON_OF_FLAME            = 12143,
    NPC_FLAME_OF_RAGNAROS       = 13148,
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
    uint32 m_uiElementalFireTimer;
    uint32 m_uiSubmergeTimer;
    uint32 m_uiAttackTimer;
    uint32 m_uiAddCount;

    bool m_bHasAggroYelled;
    bool m_bHasYelledMagmaBurst;
    bool m_bHasSubmergedOnce;
    bool m_bIsSubmerged;

    void Reset() override
    {
        m_uiWrathOfRagnarosTimer = 30000;                   // TODO Research more, according to wowwiki 25s, but timers up to 34s confirmed
        m_uiHammerTimer = 11000;                            // TODO wowwiki states 20-30s timer, but ~11s confirmed
        m_uiMagmaBlastTimer = 2000;
        m_uiElementalFireTimer = 3000;
        m_uiSubmergeTimer = 3 * MINUTE * IN_MILLISECONDS;
        m_uiAttackTimer = 90 * IN_MILLISECONDS;
        m_uiAddCount = 0;

        m_bHasYelledMagmaBurst = false;
        m_bHasSubmergedOnce = false;
        m_bIsSubmerged = false;
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

        if (m_pInstance)
            m_pInstance->SetData(TYPE_RAGNAROS, IN_PROGRESS);
    }

    void EnterEvadeMode() override
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_RAGNAROS, FAIL);

        // Reset flag if had been submerged
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
                m_uiAttackTimer = std::min(m_uiAttackTimer, (uint32)1000);
        }
    }

    void JustSummoned(Creature* pSummoned) override
    {
        if (pSummoned->GetEntry() == NPC_SON_OF_FLAME)
        {
            if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
                pSummoned->AI()->AttackStart(pTarget);

            ++m_uiAddCount;
        }
        else if (pSummoned->GetEntry() == NPC_FLAME_OF_RAGNAROS)
            pSummoned->CastSpell(pSummoned, SPELL_INTENSE_HEAT, true, nullptr, nullptr, m_creature->GetObjectGuid());
    }

    void SpellHitTarget(Unit* pTarget, const SpellEntry* pSpell) override
    {
        // As Majordomo is now killed, the last timer (until attacking) must be handled with ragnaros script
        if (pSpell->Id == SPELL_ELEMENTAL_FIRE_KILL && pTarget->GetTypeId() == TYPEID_UNIT && pTarget->GetEntry() == NPC_MAJORDOMO)
            m_uiEnterCombatTimer = 10000;
    }

    void UpdateAI(const uint32 uiDiff) override
    {
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
                    m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PASSIVE);
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
        // Return since we have no target
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        if (m_bIsSubmerged)
        {
            // Timer to check when Ragnaros should emerge (is set to soonish, when last add is killed)
            if (m_uiAttackTimer < uiDiff)
            {
                // Become emerged again
                DoCastSpellIfCan(m_creature, SPELL_RAGNA_EMERGE);
                m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                m_uiSubmergeTimer = 3 * MINUTE * IN_MILLISECONDS;
                m_uiMagmaBlastTimer = 3000;                 // Delay the magma blast after emerge
                m_bIsSubmerged = false;
            }
            else
                m_uiAttackTimer -= uiDiff;

            // Do nothing while submerged
            return;
        }

        // Wrath Of Ragnaros Timer
        if (m_uiWrathOfRagnarosTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_WRATH_OF_RAGNAROS) == CAST_OK)
            {
                DoScriptText(SAY_WRATH, m_creature);
                m_uiWrathOfRagnarosTimer = 30000;
            }
        }
        else
            m_uiWrathOfRagnarosTimer -= uiDiff;

        // Elemental Fire Timer
        if (m_uiElementalFireTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_ELEMENTAL_FIRE) == CAST_OK)
                m_uiElementalFireTimer = urand(10000, 14000);
        }
        else
            m_uiElementalFireTimer -= uiDiff;

        // Hammer of Ragnaros
        if (m_uiHammerTimer < uiDiff)
        {
            if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0, SPELL_MIGHT_OF_RAGNAROS, SELECT_FLAG_POWER_MANA))
            {
                if (DoCastSpellIfCan(pTarget, SPELL_MIGHT_OF_RAGNAROS) == CAST_OK)
                {
                    DoScriptText(SAY_HAMMER, m_creature);
                    m_uiHammerTimer = 11000;
                }
            }
            else
                m_uiHammerTimer = 11000;
        }
        else
            m_uiHammerTimer -= uiDiff;

        // Submerge Timer
        if (m_uiSubmergeTimer < uiDiff)
        {
            // Submerge and attack again after 90 secs
            DoCastSpellIfCan(m_creature, SPELL_RAGNA_SUBMERGE, CAST_INTERRUPT_PREVIOUS);
            m_creature->HandleEmote(EMOTE_ONESHOT_SUBMERGE);
            m_bIsSubmerged = true;
            m_uiAttackTimer = 90 * IN_MILLISECONDS;

            m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);

            // Say dependend if first time or not
            DoScriptText(!m_bHasSubmergedOnce ? SAY_REINFORCEMENTS_1 : SAY_REINFORCEMENTS_2, m_creature);
            m_bHasSubmergedOnce = true;

            // Summon 8 elementals at random points around the boss
            float fX, fY, fZ;
            for (uint8 i = 0; i < MAX_ADDS_IN_SUBMERGE; ++i)
            {
                m_creature->GetRandomPoint(m_creature->GetPositionX(), m_creature->GetPositionY(), m_creature->GetPositionZ(), 30.0f, fX, fY, fZ);
                m_creature->SummonCreature(NPC_SON_OF_FLAME, fX, fY, fZ, 0.0f, TEMPSUMMON_TIMED_OOC_DESPAWN, 1000);
            }

            return;
        }
        else
            m_uiSubmergeTimer -= uiDiff;

        // TODO this actually should select _any_ enemy in melee range, not only the tank
        // Range check for melee target, if nobody is found in range, then cast magma blast on random
        // If we are within range melee the target
        if (m_creature->IsNonMeleeSpellCasted(false) || !m_creature->getVictim())
            return;

        if (m_creature->CanReachWithMeleeAttack(m_creature->getVictim()))
        {
            // Make sure our attack is ready
            if (m_creature->isAttackReady())
            {
                m_creature->AttackerStateUpdate(m_creature->getVictim());
                m_creature->resetAttackTimer();
                m_bHasYelledMagmaBurst = false;
            }
        }
        else
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
                        m_uiMagmaBlastTimer = 1000;         // Spamm this!
                    }
                }
            }
            else
                m_uiMagmaBlastTimer -= uiDiff;
        }
    }
};

CreatureAI* GetAI_boss_ragnaros(Creature* pCreature)
{
    return new boss_ragnarosAI(pCreature);
}

void AddSC_boss_ragnaros()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "boss_ragnaros";
    pNewScript->GetAI = &GetAI_boss_ragnaros;
    pNewScript->RegisterSelf();
}
