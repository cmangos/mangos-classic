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
SDName: Boss_Thaddius
SD%Complete: 95
SDComment:
SDCategory: Naxxramas
EndScriptData

*/

#include "AI/ScriptDevAI/include/precompiled.h"/* ContentData
boss_thaddius
npc_tesla_coil
boss_stalagg
boss_feugen
EndContentData */


#include "naxxramas.h"

enum
{
    // Stalagg
    SAY_STAL_AGGRO                  = -1533023,
    SAY_STAL_SLAY                   = -1533024,
    SAY_STAL_DEATH                  = -1533025,

    // Feugen
    SAY_FEUG_AGGRO                  = -1533026,
    SAY_FEUG_SLAY                   = -1533027,
    SAY_FEUG_DEATH                  = -1533028,

    // Thaddus
    SAY_AGGRO_1                     = -1533030,
    SAY_AGGRO_2                     = -1533031,
    SAY_AGGRO_3                     = -1533032,
    SAY_SLAY                        = -1533033,
    SAY_ELECT                       = -1533034,
    SAY_DEATH                       = -1533035,
    EMOTE_POLARITY_SHIFT            = -1533151,

    // Thaddius Spells
    SPELL_THADIUS_SPAWN             = 28160,
    SPELL_THADIUS_LIGHTNING_VISUAL  = 28136,
    SPELL_BALL_LIGHTNING            = 28299,
    SPELL_CHAIN_LIGHTNING           = 28167,
    SPELL_POLARITY_SHIFT            = 28089,
    SPELL_BESERK                    = 27680,

    // Stalagg & Feugen Spells
    SPELL_WARSTOMP                  = 28125,
    SPELL_MAGNETIC_PULL_A           = 28338,
    SPELL_MAGNETIC_PULL_B           = 28339,
    SPELL_STATIC_FIELD              = 28135,
    SPELL_POWERSURGE                = 28134,
//    SPELL_FEUGEN_INITIALIZE         = 28138,      // usage unknown
//    SPELL_STALAGG_PASSIVE           = 28139,      // usage unknown

    // Tesla Spells
    SPELL_FEUGEN_CHAIN              = 28111,
    SPELL_STALAGG_CHAIN             = 28096,
    SPELL_FEUGEN_TESLA_PASSIVE      = 28109,
    SPELL_STALAGG_TESLA_PASSIVE     = 28097,
    SPELL_SHOCK_OVERLOAD            = 28159,
//    SPELL_SHOCK                     = 28099,      // Used in SpellEffects.cpp
    SPELL_TRIGGER_TESLAS            = 28359
};

/************
** boss_thaddius
************/

// Actually this boss behaves like a NoMovement Boss (SPELL_BALL_LIGHTNING) - but there are some movement packages used, unknown what this means!
struct boss_thaddiusAI : public Scripted_NoMovementAI
{
    boss_thaddiusAI(Creature* pCreature) : Scripted_NoMovementAI(pCreature)
    {
        m_pInstance = (instance_naxxramas*)pCreature->GetInstanceData();
        Reset();
    }

    instance_naxxramas* m_pInstance;

    uint32 m_uiPolarityShiftTimer;
    uint32 m_uiChainLightningTimer;
    uint32 m_uiBallLightningTimer;
    uint32 m_uiBerserkTimer;

    void Reset() override
    {
        m_uiPolarityShiftTimer = 15 * IN_MILLISECONDS;
        m_uiChainLightningTimer = 8 * IN_MILLISECONDS;
        m_uiBallLightningTimer = 1 * IN_MILLISECONDS;
        m_uiBerserkTimer = 6 * MINUTE * IN_MILLISECONDS;

        m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_IMMUNE_TO_PLAYER);
        DoCastSpellIfCan(m_creature, SPELL_THADIUS_SPAWN);
    }

    void Aggro(Unit* /*pWho*/) override
    {
        switch (urand(0, 2))
        {
            case 0: DoScriptText(SAY_AGGRO_1, m_creature); break;
            case 1: DoScriptText(SAY_AGGRO_2, m_creature); break;
            case 2: DoScriptText(SAY_AGGRO_3, m_creature); break;
        }

        // Make Attackable
        m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_IMMUNE_TO_PLAYER);
    }

    void JustReachedHome() override
    {
        if (m_pInstance)
        {
            m_pInstance->SetData(TYPE_THADDIUS, FAIL);

            // Respawn Adds:
            Creature* pFeugen  = m_pInstance->GetSingleCreatureFromStorage(NPC_FEUGEN);
            Creature* pStalagg = m_pInstance->GetSingleCreatureFromStorage(NPC_STALAGG);
            if (pFeugen)
            {
                pFeugen->ForcedDespawn();
                pFeugen->Respawn();
            }
            if (pStalagg)
            {
                pStalagg->ForcedDespawn();
                pStalagg->Respawn();
            }
        }
    }

    void KilledUnit(Unit* pVictim) override
    {
        if (pVictim->GetTypeId() != TYPEID_PLAYER)
            return;

        DoScriptText(SAY_SLAY, m_creature);
    }

    void JustDied(Unit* /*pKiller*/) override
    {
        DoScriptText(SAY_DEATH, m_creature);

        if (m_pInstance)
        {
            m_pInstance->SetData(TYPE_THADDIUS, DONE);

            // Force Despawn of Adds
            Creature* pFeugen  = m_pInstance->GetSingleCreatureFromStorage(NPC_FEUGEN);
            Creature* pStalagg = m_pInstance->GetSingleCreatureFromStorage(NPC_STALAGG);

            if (pFeugen)
                pFeugen->ForcedDespawn();
            if (pStalagg)
                pStalagg->ForcedDespawn();
        }
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        if (!m_pInstance)
            return;

        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        // Berserk
        if (m_uiBerserkTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_BESERK) == CAST_OK)                  // allow combat movement?
                m_uiBerserkTimer = 10 * MINUTE * IN_MILLISECONDS;
        }
        else
            m_uiBerserkTimer -= uiDiff;

        // Polarity Shift
        if (m_uiPolarityShiftTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_POLARITY_SHIFT, CAST_INTERRUPT_PREVIOUS) == CAST_OK)
            {
                DoScriptText(SAY_ELECT, m_creature);
                DoScriptText(EMOTE_POLARITY_SHIFT, m_creature);
                m_uiPolarityShiftTimer = 30 * IN_MILLISECONDS;
            }
        }
        else
            m_uiPolarityShiftTimer -= uiDiff;

        // Chain Lightning
        if (m_uiChainLightningTimer < uiDiff)
        {
            Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0);
            if (pTarget && DoCastSpellIfCan(pTarget, SPELL_CHAIN_LIGHTNING) == CAST_OK)
                m_uiChainLightningTimer = 15 * IN_MILLISECONDS;
        }
        else
            m_uiChainLightningTimer -= uiDiff;

        // Ball Lightning if target not in melee range
        // TODO: Verify, likely that the boss should attack any enemy in melee range before starting to cast
        if (!m_creature->CanReachWithMeleeAttack(m_creature->getVictim()))
        {
            if (m_uiBallLightningTimer < uiDiff)
            {
                if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_BALL_LIGHTNING) == CAST_OK)
                    m_uiBallLightningTimer = 1 * IN_MILLISECONDS;
            }
            else
                m_uiBallLightningTimer -= uiDiff;
        }
        else
            DoMeleeAttackIfReady();
    }
};

UnitAI* GetAI_boss_thaddius(Creature* pCreature)
{
    return new boss_thaddiusAI(pCreature);
}

bool EffectDummyNPC_spell_thaddius_encounter(Unit* /*pCaster*/, uint32 uiSpellId, SpellEffectIndex uiEffIndex, Creature* pCreatureTarget, ObjectGuid /*originalCasterGuid*/)
{
    switch (uiSpellId)
    {
        case SPELL_SHOCK_OVERLOAD:
            if (uiEffIndex == EFFECT_INDEX_0)
            {
                // Only do something to Thaddius, and on the first hit.
                if (pCreatureTarget->GetEntry() != NPC_THADDIUS || !pCreatureTarget->HasAura(SPELL_THADIUS_SPAWN))
                    return true;
                // remove Stun and then Cast
                pCreatureTarget->RemoveAurasDueToSpell(SPELL_THADIUS_SPAWN);
                pCreatureTarget->CastSpell(pCreatureTarget, SPELL_THADIUS_LIGHTNING_VISUAL, TRIGGERED_NONE);
            }
            return true;
        case SPELL_THADIUS_LIGHTNING_VISUAL:
            if (uiEffIndex == EFFECT_INDEX_0 && pCreatureTarget->GetEntry() == NPC_THADDIUS)
            {
                if (instance_naxxramas* pInstance = (instance_naxxramas*)pCreatureTarget->GetInstanceData())
                {
                    if (Player* pPlayer = pInstance->GetPlayerInMap(true, false))
                        pCreatureTarget->AI()->AttackStart(pPlayer);
                }
            }
            return true;
    }
    return false;
}

/************
** boss_thaddiusAddsAI - Superclass for Feugen & Stalagg
************/

// The superclass and add specific structure will use Feugen NPC as the master NPC that will initialise and synchronise both adds.
// This design is hinted in Classic spells DBCs

struct boss_thaddiusAddsAI : public ScriptedAI
{
    boss_thaddiusAddsAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = (instance_naxxramas*)pCreature->GetInstanceData();
        Reset();
    }

    instance_naxxramas* m_pInstance;

    bool m_bFakeDeath;
    bool m_bBothDead;

    uint32 m_uiHoldTimer;
    uint32 m_uiWarStompTimer;
    uint32 m_uiReviveTimer;
    uint32 m_uiShockOverloadTimer;

    void Reset() override
    {
        m_bFakeDeath = false;
        m_bBothDead = false;

        m_uiReviveTimer = 5 * IN_MILLISECONDS;
        m_uiHoldTimer = 2 * IN_MILLISECONDS;
        m_uiWarStompTimer = urand(8 * IN_MILLISECONDS, 10 * IN_MILLISECONDS);
        m_uiShockOverloadTimer = 0;

        // We might Reset while faking death, so undo this
        m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        m_creature->SetImmuneToPlayer(false);
        m_creature->SetHealth(m_creature->GetMaxHealth());
        m_creature->SetStandState(UNIT_STAND_STATE_STAND);
    }

    Creature* GetOtherAdd(bool hasToBeAlive = false) const
    // For Stalagg returns pFeugen, for Feugen returns pStalagg
    {
        Creature* otherAdd = nullptr;
        switch (m_creature->GetEntry())
        {
            case NPC_FEUGEN:  otherAdd = m_pInstance->GetSingleCreatureFromStorage(NPC_STALAGG); break;
            case NPC_STALAGG: otherAdd = m_pInstance->GetSingleCreatureFromStorage(NPC_FEUGEN); break;
            default:
                return nullptr;
        }

        // Check if the other add needs to be alive and return nullptr if dead of faking death
        if (hasToBeAlive && otherAdd)
        {
            if (boss_thaddiusAddsAI* otherAddAI = dynamic_cast<boss_thaddiusAddsAI*>(otherAdd->AI()))
            {
                if (otherAddAI->IsCountingDead())
                    return nullptr;
            }
        }

        return otherAdd;
    }

    void Aggro(Unit* pWho) override
    {
        if (!m_pInstance)
            return;

        m_pInstance->SetData(TYPE_THADDIUS, IN_PROGRESS);

        if (Creature* pOtherAdd = GetOtherAdd())
        {
            if (!pOtherAdd->isInCombat())
                pOtherAdd->SetInCombatWithZone();
        }
    }

    void JustReachedHome() override
    {
        if (!m_pInstance)
            return;

        m_pInstance->SetData(TYPE_THADDIUS, FAIL);
    }

    bool DoMagneticPullIfCan()
    {
        if (!m_bFakeDeath && GetOtherAdd())
        {
            if (DoCastSpellIfCan(GetOtherAdd(), (m_creature->GetEntry() == NPC_FEUGEN ? SPELL_MAGNETIC_PULL_A : SPELL_MAGNETIC_PULL_B), TRIGGERED_OLD_TRIGGERED))
            {
                DoStopAttack();
                DoResetThreat();
                if (Unit* target = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0, uint32(0), SELECT_FLAG_IN_MELEE_RANGE))
                {
                    AttackStart(target);
                    m_creature->SetInFront(target);
                }
                return true;
            }
        }
        return false;
    }

    bool IsCountingDead() const
    {
        return m_bFakeDeath || m_creature->isDead();
    }


    virtual void UpdateAddAI(const uint32 /*uiDiff*/) {}        // Used for Add-specific spells

    void UpdateAI(const uint32 uiDiff) override
    {
        if (m_bBothDead)                                        // This is the case while fighting Thaddius
        {
            if (m_uiShockOverloadTimer)                         // Timer before triggering phase 2 with spell Shock Overload
            {
                if (m_uiShockOverloadTimer <= uiDiff)
                {
                    if (DoCastSpellIfCan(m_creature, SPELL_TRIGGER_TESLAS) == CAST_OK)
                        m_uiShockOverloadTimer = 0;
                }
                else
                    m_uiShockOverloadTimer -= uiDiff;
            }
            return;
        }

        if (m_bFakeDeath)
        {
            if (m_uiReviveTimer < uiDiff)
            {
                if (Creature* pOther = GetOtherAdd())
                {
                    if (boss_thaddiusAddsAI* pOtherAI = dynamic_cast<boss_thaddiusAddsAI*>(pOther->AI()))
                    {
                        if (!pOtherAI->IsCountingDead())    // Raid was to slow to kill the second add
                        {
                            DoResetThreat();
                            SetCombatMovement(false);
                            m_uiHoldTimer = 1500;
                            Reset();
                        }
                        else
                        {
                            m_bBothDead = true;             // Now both adds are counting dead
                            pOtherAI->m_bBothDead = true;
                            // Set both Teslas to overload
                            m_uiShockOverloadTimer = 14 * IN_MILLISECONDS;
                        }
                    }
                }
            }
            else
                m_uiReviveTimer -= uiDiff;
            return;
        }

        // If no target do nothing
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        if (m_uiHoldTimer)                      // A short timer preventing combat movement after revive
        {
            if (m_uiHoldTimer <= uiDiff)
            {
                SetCombatMovement(true);
                m_creature->GetMotionMaster()->MoveChase(m_creature->getVictim());
                m_uiHoldTimer = 0;
            }
            else
                m_uiHoldTimer -= uiDiff;
        }

        if (m_uiWarStompTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_WARSTOMP) == CAST_OK)
                m_uiWarStompTimer = urand(8 * IN_MILLISECONDS, 10 * IN_MILLISECONDS);
        }
        else
            m_uiWarStompTimer -= uiDiff;

        UpdateAddAI(uiDiff);                    // For Add Specific Abilities

        DoMeleeAttackIfReady();
    }

    void DamageTaken(Unit* pKiller, uint32& damage, DamageEffectType /*damagetype*/, SpellEntry const* /*spellInfo*/) override
    {
        if (damage < m_creature->GetHealth())
            return;

        // Prevent glitch if in fake death
        if (m_bFakeDeath)
        {
            damage = std::min(damage, m_creature->GetHealth() - 1);
            return;
        }

        // prevent death
        damage = std::min(damage, m_creature->GetHealth() - 1);
        m_bFakeDeath = true;

        m_creature->InterruptNonMeleeSpells(false);
        m_creature->StopMoving();
        m_creature->ClearComboPointHolders();
        m_creature->RemoveAllAurasOnDeath();
        m_creature->ModifyAuraState(AURA_STATE_HEALTHLESS_20_PERCENT, false);
        m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        m_creature->SetImmuneToPlayer(true);
        m_creature->ClearAllReactives();
        m_creature->GetMotionMaster()->Clear();
        m_creature->GetMotionMaster()->MoveIdle();
        m_creature->SetStandState(UNIT_STAND_STATE_DEAD);

        JustDied(pKiller);                                  // Texts
    }
};

/************
** boss_stalagg
************/

struct boss_stalaggAI : public boss_thaddiusAddsAI
{
    boss_stalaggAI(Creature* pCreature) : boss_thaddiusAddsAI(pCreature)
    {
        Reset();
    }
    uint32 m_uiPowerSurgeTimer;

    void Reset() override
    {
        boss_thaddiusAddsAI::Reset();
        m_uiPowerSurgeTimer = urand(10, 15) * IN_MILLISECONDS;
    }

    void Aggro(Unit* pWho) override
    {
        DoScriptText(SAY_STAL_AGGRO, m_creature);
        boss_thaddiusAddsAI::Aggro(pWho);
    }

    void JustDied(Unit* /*pKiller*/) override
    {
        DoScriptText(SAY_STAL_DEATH, m_creature);
    }

    void KilledUnit(Unit* pVictim) override
    {
        if (pVictim->GetTypeId() == TYPEID_PLAYER)
            DoScriptText(SAY_STAL_SLAY, m_creature);
    }

    void UpdateAddAI(const uint32 uiDiff)
    {
        if (m_uiPowerSurgeTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_POWERSURGE) == CAST_OK)
                m_uiPowerSurgeTimer = urand(10, 15) * IN_MILLISECONDS;
        }
        else
            m_uiPowerSurgeTimer -= uiDiff;
    }
};

UnitAI* GetAI_boss_stalagg(Creature* pCreature)
{
    return new boss_stalaggAI(pCreature);
}

/************
** boss_feugen
************/

struct boss_feugenAI : public boss_thaddiusAddsAI
{
    boss_feugenAI(Creature* pCreature) : boss_thaddiusAddsAI(pCreature)
    {
        Reset();
    }
    uint32 m_uiStaticFieldTimer;
    uint32 m_uiMagneticPullTimer;

    void Reset() override
    {
        boss_thaddiusAddsAI::Reset();
        m_uiStaticFieldTimer = urand(10, 15) * IN_MILLISECONDS;
        m_uiMagneticPullTimer = 20 * IN_MILLISECONDS;
    }

    void Aggro(Unit* pWho) override
    {
        DoScriptText(SAY_FEUG_AGGRO, m_creature);
        boss_thaddiusAddsAI::Aggro(pWho);
    }

    void JustDied(Unit* /*pKiller*/) override
    {
        DoScriptText(SAY_FEUG_DEATH, m_creature);
    }

    void KilledUnit(Unit* pVictim) override
    {
        if (pVictim->GetTypeId() == TYPEID_PLAYER)
            DoScriptText(SAY_FEUG_SLAY, m_creature);
    }

    void UpdateAddAI(const uint32 uiDiff)
    {
        if (m_uiMagneticPullTimer < uiDiff)
        {
            if (!m_bFakeDeath && GetOtherAdd(true))     // No need to check if we have a victim that Stalagg can pull: this is already done in super class
            {
                if (DoMagneticPullIfCan())              // This will return false if Stalagg has no victim we can pull
                {
                    if (boss_thaddiusAddsAI* stalaggAI = dynamic_cast<boss_thaddiusAddsAI*>(GetOtherAdd()->AI()))
                    {
                        stalaggAI->DoMagneticPullIfCan();
                        m_uiMagneticPullTimer = 20 * IN_MILLISECONDS;
                    }
                }
            }
        }
        else
            m_uiMagneticPullTimer -= uiDiff;

        if (m_uiStaticFieldTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_STATIC_FIELD) == CAST_OK)
                m_uiStaticFieldTimer = urand(10, 15) * IN_MILLISECONDS;
        }
        else
            m_uiStaticFieldTimer -= uiDiff;
    }
};

UnitAI* GetAI_boss_feugen(Creature* pCreature)
{
    return new boss_feugenAI(pCreature);
}

void AddSC_boss_thaddius()
{
    Script* pNewScript = new Script;
    pNewScript->Name = "boss_thaddius";
    pNewScript->GetAI = &GetAI_boss_thaddius;
    pNewScript->pEffectDummyNPC = &EffectDummyNPC_spell_thaddius_encounter;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "boss_stalagg";
    pNewScript->GetAI = &GetAI_boss_stalagg;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "boss_feugen";
    pNewScript->GetAI = &GetAI_boss_feugen;
    pNewScript->RegisterSelf();
}
