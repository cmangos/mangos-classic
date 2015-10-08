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
SD%Complete: 85
SDComment: Magnetic Pull, Tesla-Chains, Polaritiy-Shift missing (core!)
SDCategory: Naxxramas
EndScriptData */

/* ContentData
boss_thaddius
npc_tesla_coil
boss_stalagg
boss_feugen
EndContentData */

#include "precompiled.h"
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

    // Tesla Coils
    EMOTE_LOSING_LINK               = -1533149,
    EMOTE_TESLA_OVERLOAD            = -1533150,

    // Thaddus
    SAY_AGGRO_1                     = -1533030,
    SAY_AGGRO_2                     = -1533031,
    SAY_AGGRO_3                     = -1533032,
    SAY_SLAY                        = -1533033,
    SAY_ELECT                       = -1533034,
    SAY_DEATH                       = -1533035,
    // Background screams in Instance if Thaddius still alive, needs general support most likely
    SAY_SCREAM1                     = -1533036,
    SAY_SCREAM2                     = -1533037,
    SAY_SCREAM3                     = -1533038,
    SAY_SCREAM4                     = -1533039,
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
    SPELL_STATIC_FIELD              = 28135,
    SPELL_POWERSURGE                = 28134,

    // Tesla Spells
    SPELL_FEUGEN_CHAIN              = 28111,
    SPELL_STALAGG_CHAIN             = 28096,
    SPELL_FEUGEN_TESLA_PASSIVE      = 28109,
    SPELL_STALAGG_TESLA_PASSIVE     = 28097,
    SPELL_SHOCK_OVERLOAD            = 28159,
    SPELL_SHOCK                     = 28099,
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

        m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
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
        m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
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

CreatureAI* GetAI_boss_thaddius(Creature* pCreature)
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
                pCreatureTarget->CastSpell(pCreatureTarget, SPELL_THADIUS_LIGHTNING_VISUAL, false);
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
** npc_tesla_coil
************/

struct npc_tesla_coilAI : public Scripted_NoMovementAI
{
    npc_tesla_coilAI(Creature* pCreature) : Scripted_NoMovementAI(pCreature)
    {
        m_pInstance = (instance_naxxramas*)pCreature->GetInstanceData();
        m_uiSetupTimer = 1 * IN_MILLISECONDS;
        m_uiOverloadTimer = 0;
        m_bReapply = false;
        Reset();
    }

    instance_naxxramas* m_pInstance;
    bool m_bToFeugen;
    bool m_bReapply;

    uint32 m_uiSetupTimer;
    uint32 m_uiOverloadTimer;

    void Reset() override {}
    void MoveInLineOfSight(Unit* /*pWho*/) override {}

    void Aggro(Unit* /*pWho*/) override
    {
        DoScriptText(EMOTE_LOSING_LINK, m_creature);
    }

    // Overwrite this function here to
    // * Keep Chain spells casted when evading after useless EnterCombat caused by 'buffing' the add
    // * To not remove the Passive spells when evading after ie killed a player
    void EnterEvadeMode() override
    {
        m_creature->DeleteThreatList();
        m_creature->CombatStop();
    }

    bool SetupChain()
    {
        // Check, if instance_ script failed or encounter finished
        if (!m_pInstance || m_pInstance->GetData(TYPE_THADDIUS) == DONE)
            return true;

        GameObject* pNoxTeslaFeugen  = m_pInstance->GetSingleGameObjectFromStorage(GO_CONS_NOX_TESLA_FEUGEN);
        GameObject* pNoxTeslaStalagg = m_pInstance->GetSingleGameObjectFromStorage(GO_CONS_NOX_TESLA_STALAGG);

        // Try again, till Tesla GOs are spawned
        if (!pNoxTeslaFeugen || !pNoxTeslaStalagg)
            return false;

        m_bToFeugen = m_creature->GetDistanceOrder(pNoxTeslaFeugen, pNoxTeslaStalagg);

        if (DoCastSpellIfCan(m_creature, m_bToFeugen ? SPELL_FEUGEN_CHAIN : SPELL_STALAGG_CHAIN) == CAST_OK)
            return true;

        return false;
    }

    void ReApplyChain(uint32 uiEntry)
    {
        if (uiEntry)                                        // called from Stalagg/Feugen with their entry
        {
            // Only apply chain to own add
            if ((uiEntry == NPC_FEUGEN && !m_bToFeugen) || (uiEntry == NPC_STALAGG && m_bToFeugen))
                return;

            m_bReapply = true;                              // Reapply Chains on next tick
        }
        else                                                // if called from next tick, needed because otherwise the spell doesn't bind
        {
            m_bReapply = false;
            m_creature->InterruptNonMeleeSpells(true);
            GameObject* pGo = m_pInstance->GetSingleGameObjectFromStorage(m_bToFeugen ? GO_CONS_NOX_TESLA_FEUGEN : GO_CONS_NOX_TESLA_STALAGG);

            if (pGo && pGo->GetGoType() == GAMEOBJECT_TYPE_BUTTON && pGo->getLootState() == GO_ACTIVATED)
                pGo->ResetDoorOrButton();

            DoCastSpellIfCan(m_creature, m_bToFeugen ? SPELL_FEUGEN_CHAIN : SPELL_STALAGG_CHAIN);
        }
    }

    void SetOverloading()
    {
        m_uiOverloadTimer = 14 * IN_MILLISECONDS;           // it takes some time to overload and activate Thaddius
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        m_creature->SelectHostileTarget();

        if (!m_uiOverloadTimer && !m_uiSetupTimer && !m_bReapply)
            return;                                         // Nothing to do this tick

        if (m_uiSetupTimer)
        {
            if (m_uiSetupTimer <= uiDiff)
            {
                if (SetupChain())
                    m_uiSetupTimer = 0;
                else
                    m_uiSetupTimer = 5 * IN_MILLISECONDS;
            }
            else
                m_uiSetupTimer -= uiDiff;
        }

        if (m_uiOverloadTimer)
        {
            if (m_uiOverloadTimer <=  uiDiff)
            {
                m_uiOverloadTimer = 0;
                m_creature->RemoveAurasDueToSpell(m_bToFeugen ? SPELL_FEUGEN_TESLA_PASSIVE : SPELL_STALAGG_TESLA_PASSIVE);
                DoCastSpellIfCan(m_creature,  SPELL_SHOCK_OVERLOAD, CAST_INTERRUPT_PREVIOUS);
                DoScriptText(EMOTE_TESLA_OVERLOAD, m_creature);
                m_pInstance->DoUseDoorOrButton(m_bToFeugen ? GO_CONS_NOX_TESLA_FEUGEN : GO_CONS_NOX_TESLA_STALAGG);
            }
            else
                m_uiOverloadTimer -= uiDiff;
        }

        if (m_bReapply)
            ReApplyChain(0);
    }
};

CreatureAI* GetAI_npc_tesla_coil(Creature* pCreature)
{
    return new npc_tesla_coilAI(pCreature);
}

/************
** boss_thaddiusAddsAI - Superclass for Feugen & Stalagg
************/

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

    void Reset() override
    {
        m_bFakeDeath = false;
        m_bBothDead = false;

        m_uiReviveTimer = 5 * IN_MILLISECONDS;
        m_uiHoldTimer = 2 * IN_MILLISECONDS;
        m_uiWarStompTimer = urand(8 * IN_MILLISECONDS, 10 * IN_MILLISECONDS);

        // We might Reset while faking death, so undo this
        m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        m_creature->SetHealth(m_creature->GetMaxHealth());
        m_creature->SetStandState(UNIT_STAND_STATE_STAND);
    }

    Creature* GetOtherAdd()                                 // For Stalagg returns pFeugen, for Feugen returns pStalagg
    {
        switch (m_creature->GetEntry())
        {
            case NPC_FEUGEN:  return m_pInstance->GetSingleCreatureFromStorage(NPC_STALAGG);
            case NPC_STALAGG: return m_pInstance->GetSingleCreatureFromStorage(NPC_FEUGEN);
            default:
                return nullptr;
        }
    }

    void Aggro(Unit* pWho) override
    {
        if (!m_pInstance)
            return;

        m_pInstance->SetData(TYPE_THADDIUS, IN_PROGRESS);

        if (Creature* pOtherAdd = GetOtherAdd())
        {
            if (!pOtherAdd->isInCombat())
                pOtherAdd->AI()->AttackStart(pWho);
        }
    }

    void JustRespawned() override
    {
        Reset();                                            // Needed to reset the flags properly

        GuidList lTeslaGUIDList;
        if (!m_pInstance)
            return;

        m_pInstance->GetThadTeslaCreatures(lTeslaGUIDList);
        if (lTeslaGUIDList.empty())
            return;

        for (GuidList::const_iterator itr = lTeslaGUIDList.begin(); itr != lTeslaGUIDList.end(); ++itr)
        {
            if (Creature* pTesla = m_pInstance->instance->GetCreature(*itr))
            {
                if (npc_tesla_coilAI* pTeslaAI = dynamic_cast<npc_tesla_coilAI*>(pTesla->AI()))
                    pTeslaAI->ReApplyChain(m_creature->GetEntry());
            }
        }
    }

    void JustReachedHome() override
    {
        if (!m_pInstance)
            return;

        if (Creature* pOther = GetOtherAdd())
        {
            if (boss_thaddiusAddsAI* pOtherAI = dynamic_cast<boss_thaddiusAddsAI*>(pOther->AI()))
            {
                if (pOtherAI->IsCountingDead())
                {
                    pOther->ForcedDespawn();
                    pOther->Respawn();
                }
            }
        }

        // Reapply Chains if needed
        if (!m_creature->HasAura(SPELL_FEUGEN_CHAIN) && !m_creature->HasAura(SPELL_STALAGG_CHAIN))
            JustRespawned();

        m_pInstance->SetData(TYPE_THADDIUS, FAIL);
    }

    void Revive()
    {
        DoResetThreat();
        PauseCombatMovement();
        Reset();
    }

    bool IsCountingDead()
    {
        return m_bFakeDeath || m_creature->isDead();
    }

    void PauseCombatMovement()
    {
        SetCombatMovement(false);
        m_uiHoldTimer = 1500;
    }

    virtual void UpdateAddAI(const uint32 /*uiDiff*/) {}        // Used for Add-specific spells

    void UpdateAI(const uint32 uiDiff) override
    {
        if (m_bBothDead)                                    // This is the case while fighting Thaddius
            return;

        if (m_bFakeDeath)
        {
            if (m_uiReviveTimer < uiDiff)
            {
                if (Creature* pOther = GetOtherAdd())
                {
                    if (boss_thaddiusAddsAI* pOtherAI = dynamic_cast<boss_thaddiusAddsAI*>(pOther->AI()))
                    {
                        if (!pOtherAI->IsCountingDead())    // Raid was to slow to kill the second add
                            Revive();
                        else
                        {
                            m_bBothDead = true;             // Now both adds are counting dead
                            pOtherAI->m_bBothDead = true;
                            // Set both Teslas to overload
                            GuidList lTeslaGUIDList;
                            m_pInstance->GetThadTeslaCreatures(lTeslaGUIDList);
                            for (GuidList::const_iterator itr = lTeslaGUIDList.begin(); itr != lTeslaGUIDList.end(); ++itr)
                            {
                                if (Creature* pTesla = m_pInstance->instance->GetCreature(*itr))
                                {
                                    if (npc_tesla_coilAI* pTeslaAI = dynamic_cast<npc_tesla_coilAI*>(pTesla->AI()))
                                        pTeslaAI->SetOverloading();
                                }
                            }
                        }
                    }
                }
            }
            else
                m_uiReviveTimer -= uiDiff;
            return;
        }

        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        if (m_uiHoldTimer)                                  // A short timer preventing combat movement after revive
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

    void DamageTaken(Unit* pKiller, uint32& uiDamage) override
    {
        if (uiDamage < m_creature->GetHealth())
            return;

        // Prevent glitch if in fake death
        if (m_bFakeDeath)
        {
            uiDamage = 0;
            return;
        }

        // prevent death
        uiDamage = 0;
        m_bFakeDeath = true;

        m_creature->InterruptNonMeleeSpells(false);
        m_creature->SetHealth(0);
        m_creature->StopMoving();
        m_creature->ClearComboPointHolders();
        m_creature->RemoveAllAurasOnDeath();
        m_creature->ModifyAuraState(AURA_STATE_HEALTHLESS_20_PERCENT, false);
        m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
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
        m_uiPowerSurgeTimer = urand(10 * IN_MILLISECONDS, 15 * IN_MILLISECONDS);
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
                m_uiPowerSurgeTimer = urand(10 * IN_MILLISECONDS, 15 * IN_MILLISECONDS);
        }
        else
            m_uiPowerSurgeTimer -= uiDiff;
    }
};

CreatureAI* GetAI_boss_stalagg(Creature* pCreature)
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
    uint32 m_uiMagneticPullTimer;                           // TODO, missing

    void Reset() override
    {
        boss_thaddiusAddsAI::Reset();
        m_uiStaticFieldTimer = urand(10 * IN_MILLISECONDS, 15 * IN_MILLISECONDS);
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
        if (m_uiStaticFieldTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_STATIC_FIELD) == CAST_OK)
                m_uiStaticFieldTimer = urand(10 * IN_MILLISECONDS, 15 * IN_MILLISECONDS);
        }
        else
            m_uiStaticFieldTimer -= uiDiff;
    }
};

CreatureAI* GetAI_boss_feugen(Creature* pCreature)
{
    return new boss_feugenAI(pCreature);
}

void AddSC_boss_thaddius()
{
    Script* pNewScript;

    pNewScript = new Script;
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

    pNewScript = new Script;
    pNewScript->Name = "npc_tesla_coil";
    pNewScript->GetAI = &GetAI_npc_tesla_coil;
    pNewScript->pEffectDummyNPC = &EffectDummyNPC_spell_thaddius_encounter;
    pNewScript->RegisterSelf();
}
