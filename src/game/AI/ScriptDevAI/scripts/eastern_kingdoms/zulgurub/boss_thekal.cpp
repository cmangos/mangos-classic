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
SDName: Boss_Thekal
SD%Complete: 90
SDComment: Needs a good shake-up.
SDCategory: Zul'Gurub
EndScriptData

*/

#include "AI/ScriptDevAI/include/precompiled.h"
#include "zulgurub.h"

enum
{
    SAY_AGGRO               = -1309009,
    SAY_DEATH               = -1309010,

    SPELL_MORTAL_CLEAVE     = 22859,
    SPELL_SILENCE           = 23207,
    SPELL_FRENZY            = 23128,
    SPELL_FORCE_PUNCH       = 24189,
    SPELL_CHARGE            = 24408,
    SPELL_ENRAGE            = 23537,
    SPELL_SUMMON_TIGERS     = 24183,
    SPELL_TIGER_FORM        = 24169,
    SPELL_RESURRECT         = 24173,
    SPELL_THEKAL_TRIGGER    = 24172,        // TODO: Need to find use for this

    // Zealot Lor'Khan Spells
    SPELL_LIGHTNING_SHIELD  = 20545,
    SPELL_DISPEL_MAGIC      = 17201,
    SPELL_GREAT_HEAL        = 24208,
    SPELL_DISARM            = 6713,

    // Zealot Zath Spells
    SPELL_SINISTER_STRIKE   = 15581,
    SPELL_GOUGE             = 12540,
    SPELL_KICK              = 15614,
    SPELL_BLIND             = 21060,
    SPELL_DUAL_WIELD        = 674,

    PHASE_NORMAL            = 1,
    PHASE_FAKE_DEATH        = 2,
    PHASE_WAITING           = 3,
    PHASE_TIGER             = 4,
};

// abstract base class for faking death
struct boss_thekalBaseAI : public ScriptedAI
{
    boss_thekalBaseAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_uiPhase = PHASE_NORMAL;
    }

    uint8 m_uiPhase;

    virtual void OnFakeingDeath() {}
    virtual void OnRevive() {}

    void DamageTaken(Unit* /*pKiller*/, uint32& damage, DamageEffectType /*damagetype*/, SpellEntry const* /*spellInfo*/) override
    {
        if (damage < m_creature->GetHealth())
            return;

        // Prevent glitch if in fake death
        if (m_uiPhase == PHASE_FAKE_DEATH || m_uiPhase == PHASE_WAITING)
        {
            damage = 0;
            return;
        }

        // Only init fake in normal phase
        if (m_uiPhase != PHASE_NORMAL)
            return;

        damage = std::min(damage, m_creature->GetHealth() - 1);

        m_creature->InterruptNonMeleeSpells(true);
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

        m_uiPhase = PHASE_FAKE_DEATH;

        OnFakeingDeath();
    }

    void Revive(bool bOnlyFlags = false)
    {
        m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        m_creature->SetStandState(UNIT_STAND_STATE_STAND);

        if (bOnlyFlags)
            return;

        m_creature->SetHealth(m_creature->GetMaxHealth());
        m_uiPhase = PHASE_NORMAL;

        DoResetThreat();
        Reset();

        // Assume Attack
        if (m_creature->getVictim())
            m_creature->GetMotionMaster()->MoveChase(m_creature->getVictim());

        OnRevive();
    }

    void PreventRevive()
    {
        if (m_creature->IsNonMeleeSpellCasted(true))
            m_creature->InterruptNonMeleeSpells(true);

        m_uiPhase = PHASE_WAITING;
    }
};

struct boss_thekalAI : public boss_thekalBaseAI
{
    boss_thekalAI(Creature* pCreature) : boss_thekalBaseAI(pCreature)
    {
        m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
        Reset();
    }

    ScriptedInstance* m_pInstance;

    uint32 m_uiMortalCleaveTimer;
    uint32 m_uiSilenceTimer;
    uint32 m_uiFrenzyTimer;
    uint32 m_uiForcePunchTimer;
    uint32 m_uiChargeTimer;
    uint32 m_uiEnrageTimer;
    uint32 m_uiSummonTigersTimer;
    uint32 m_uiResurrectTimer;

    bool m_bEnraged;

    void Reset() override
    {
        m_uiMortalCleaveTimer   = 4000;
        m_uiSilenceTimer        = 9000;
        m_uiFrenzyTimer         = 30000;
        m_uiForcePunchTimer     = 4000;
        m_uiChargeTimer         = 12000;
        m_uiEnrageTimer         = 32000;
        m_uiSummonTigersTimer   = 25000;
        m_uiResurrectTimer      = 10000;
        m_uiPhase               = PHASE_NORMAL;

        m_bEnraged              = false;

        // remove fake death
        Revive(true);
    }

    void Aggro(Unit* /*pWho*/) override
    {
        DoScriptText(SAY_AGGRO, m_creature);
    }

    void JustDied(Unit* /*pKiller*/) override
    {
        DoScriptText(SAY_DEATH, m_creature);

        if (!m_pInstance)
            return;

        m_pInstance->SetData(TYPE_THEKAL, DONE);

        // remove the two adds
        if (Creature* pZath = m_pInstance->GetSingleCreatureFromStorage(NPC_ZATH))
            pZath->ForcedDespawn();
        if (Creature* pLorkhan = m_pInstance->GetSingleCreatureFromStorage(NPC_LORKHAN))
            pLorkhan->ForcedDespawn();
    }

    void JustReachedHome() override
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_THEKAL, FAIL);
    }

    // Only call in context where m_pInstance is valid
    bool CanPreventAddsResurrect() const
    {
        // If any add is alive, return false
        if (m_pInstance->GetData(TYPE_ZATH) != SPECIAL || m_pInstance->GetData(TYPE_LORKHAN) != SPECIAL)
            return false;

        // Else Prevent them Resurrecting
        if (Creature* pLorkhan = m_pInstance->GetSingleCreatureFromStorage(NPC_LORKHAN))
        {
            if (boss_thekalBaseAI* pFakerAI = dynamic_cast<boss_thekalBaseAI*>(pLorkhan->AI()))
                pFakerAI->PreventRevive();
        }
        if (Creature* pZath = m_pInstance->GetSingleCreatureFromStorage(NPC_ZATH))
        {
            if (boss_thekalBaseAI* pFakerAI = dynamic_cast<boss_thekalBaseAI*>(pZath->AI()))
                pFakerAI->PreventRevive();
        }

        return true;
    }

    void OnFakeingDeath()
    {
        m_uiResurrectTimer = 10000;

        if (m_pInstance)
        {
            m_pInstance->SetData(TYPE_THEKAL, SPECIAL);

            // If both Adds are already dead, don't wait 10 seconds
            if (CanPreventAddsResurrect())
                m_uiResurrectTimer = 1000;
        }
    }

    void OnRevive()
    {
        if (!m_pInstance)
            return;

        // Both Adds are 'dead' enter tiger phase
        if (CanPreventAddsResurrect())
        {
            DoCastSpellIfCan(m_creature, SPELL_TIGER_FORM, CAST_TRIGGERED);
            m_uiPhase = PHASE_TIGER;
        }
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        switch (m_uiPhase)
        {
            case PHASE_FAKE_DEATH:
                if (m_uiResurrectTimer < uiDiff)
                {
                    // resurrect him in any case
                    DoCastSpellIfCan(m_creature, SPELL_RESURRECT);

                    m_uiPhase = PHASE_WAITING;
                    if (m_pInstance)
                    {
                        CanPreventAddsResurrect();
                        m_pInstance->SetData(TYPE_THEKAL, IN_PROGRESS);
                    }
                }
                else
                    m_uiResurrectTimer -= uiDiff;

            // No break needed here
            case PHASE_WAITING:
                return;

            case PHASE_NORMAL:
                if (m_uiMortalCleaveTimer < uiDiff)
                {
                    if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_MORTAL_CLEAVE) == CAST_OK)
                        m_uiMortalCleaveTimer = urand(15000, 20000);
                }
                else
                    m_uiMortalCleaveTimer -= uiDiff;

                if (m_uiSilenceTimer < uiDiff)
                {
                    if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
                    {
                        if (DoCastSpellIfCan(pTarget, SPELL_SILENCE) == CAST_OK)
                            m_uiSilenceTimer = urand(20000, 25000);
                    }
                }
                else
                    m_uiSilenceTimer -= uiDiff;

                break;
            case PHASE_TIGER:
                if (m_uiChargeTimer < uiDiff)
                {
                    if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
                    {
                        if (DoCastSpellIfCan(pTarget, SPELL_CHARGE) == CAST_OK)
                        {
                            DoResetThreat();
                            AttackStart(pTarget);
                            m_uiChargeTimer = urand(15000, 22000);
                        }
                    }
                }
                else
                    m_uiChargeTimer -= uiDiff;

                if (m_uiFrenzyTimer < uiDiff)
                {
                    if (DoCastSpellIfCan(m_creature, SPELL_FRENZY) == CAST_OK)
                        m_uiFrenzyTimer = 30000;
                }
                else
                    m_uiFrenzyTimer -= uiDiff;

                if (m_uiForcePunchTimer < uiDiff)
                {
                    if (DoCastSpellIfCan(m_creature, SPELL_FORCE_PUNCH) == CAST_OK)
                        m_uiForcePunchTimer = urand(16000, 21000);
                }
                else
                    m_uiForcePunchTimer -= uiDiff;

                if (m_uiSummonTigersTimer < uiDiff)
                {
                    if (DoCastSpellIfCan(m_creature, SPELL_SUMMON_TIGERS) == CAST_OK)
                        m_uiSummonTigersTimer = 50000;
                }
                else
                    m_uiSummonTigersTimer -= uiDiff;

                if (!m_bEnraged && m_creature->GetHealthPercent() < 11.0f)
                {
                    if (DoCastSpellIfCan(m_creature, SPELL_ENRAGE) == CAST_OK)
                        m_bEnraged = true;
                }

                break;
        }

        DoMeleeAttackIfReady();
    }
};

/*######
## mob_zealot_lorkhan
######*/

struct mob_zealot_lorkhanAI : public boss_thekalBaseAI
{
    mob_zealot_lorkhanAI(Creature* pCreature) : boss_thekalBaseAI(pCreature)
    {
        m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
        Reset();
    }

    ScriptedInstance* m_pInstance;

    uint32 m_uiLightningShieldTimer;
    uint32 m_uiGreatHealTimer;
    uint32 m_uiDisarmTimer;
    uint32 m_uiDispelTimer;
    uint32 m_uiResurrectTimer;

    void Reset() override
    {
        m_uiLightningShieldTimer = 1000;
        m_uiGreatHealTimer       = 32000;
        m_uiDisarmTimer          = 6000;
        m_uiDispelTimer          = 0;
        m_uiPhase                = PHASE_NORMAL;

        if (m_pInstance)
            m_pInstance->SetData(TYPE_LORKHAN, NOT_STARTED);

        Revive(true);
    }

    void Aggro(Unit* /*pWho*/) override
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_LORKHAN, IN_PROGRESS);
    }

    void OnFakeingDeath()
    {
        m_uiResurrectTimer = 10000;

        if (m_pInstance)
            m_pInstance->SetData(TYPE_LORKHAN, SPECIAL);
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        switch (m_uiPhase)
        {
            case PHASE_FAKE_DEATH:
                if (m_uiResurrectTimer < uiDiff)
                {
                    if (!m_pInstance)
                        return;

                    if (m_pInstance->GetData(TYPE_THEKAL) != SPECIAL || m_pInstance->GetData(TYPE_ZATH) != SPECIAL)
                    {
                        DoCastSpellIfCan(m_creature, SPELL_RESURRECT);
                        m_pInstance->SetData(TYPE_LORKHAN, IN_PROGRESS);
                    }

                    m_uiPhase = PHASE_WAITING;
                }
                else
                    m_uiResurrectTimer -= uiDiff;

            // no break needed here
            case PHASE_WAITING:
                return;

            case PHASE_NORMAL:
                if (m_uiDispelTimer < uiDiff)
                {
                    CreatureList pList = DoFindFriendlyCC(30.0f);

                    Creature* dispelTarget = nullptr;

                    if (!pList.empty())
                        for (auto& itr : pList)
                        {
                            dispelTarget = itr;
                            break;
                        }

                    if (!dispelTarget && (m_creature->isInRoots() || m_creature->GetMaxNegativeAuraModifier(SPELL_AURA_MOD_DECREASE_SPEED)))
                        dispelTarget = m_creature;

                    if (dispelTarget && (DoCastSpellIfCan(dispelTarget, SPELL_DISPEL_MAGIC) == CAST_OK))
                        m_uiDispelTimer = urand(15000, 20000);
                }
                else
                    m_uiDispelTimer -= uiDiff;

                // Lightning_Shield_Timer
                if (m_uiLightningShieldTimer < uiDiff)
                {
                    if (DoCastSpellIfCan(m_creature, SPELL_LIGHTNING_SHIELD) == CAST_OK)
                        m_uiLightningShieldTimer = 61000;
                }
                else
                    m_uiLightningShieldTimer -= uiDiff;

                // Casting Greatheal to Thekal or Zath if they are in meele range.
                // TODO - why this range check?
                if (m_uiGreatHealTimer < uiDiff)
                {
                    if (m_pInstance)
                    {
                        Creature* pThekal = m_pInstance->GetSingleCreatureFromStorage(NPC_THEKAL);
                        Creature* pZath = m_pInstance->GetSingleCreatureFromStorage(NPC_ZATH);

                        switch (urand(0, 1))
                        {
                            case 0:
                                if (pThekal && m_creature->IsWithinDistInMap(pThekal, 3 * ATTACK_DISTANCE))
                                    DoCastSpellIfCan(pThekal, SPELL_GREAT_HEAL);
                                break;
                            case 1:
                                if (pZath && m_creature->IsWithinDistInMap(pZath, 3 * ATTACK_DISTANCE))
                                    DoCastSpellIfCan(pZath, SPELL_GREAT_HEAL);
                                break;
                        }
                    }

                    m_uiGreatHealTimer = urand(15000, 20000);
                }
                else
                    m_uiGreatHealTimer -= uiDiff;

                // Disarm_Timer
                if (m_uiDisarmTimer < uiDiff)
                {
                    if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_DISARM) == CAST_OK)
                        m_uiDisarmTimer = urand(15000, 25000);
                }
                else
                    m_uiDisarmTimer -= uiDiff;

                break;
        }

        DoMeleeAttackIfReady();
    }
};

/*######
## npc_zealot_zath
######*/

struct mob_zealot_zathAI : public boss_thekalBaseAI
{
    mob_zealot_zathAI(Creature* pCreature) : boss_thekalBaseAI(pCreature)
    {
        m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
        Reset();
    }

    ScriptedInstance* m_pInstance;

    uint32 m_uiSinisterStrikeTimer;
    uint32 m_uiGougeTimer;
    uint32 m_uiKickTimer;
    uint32 m_uiBlindTimer;
    uint32 m_uiResurrectTimer;

    void Reset() override
    {
        m_uiSinisterStrikeTimer     = 8000;
        m_uiGougeTimer              = 25000;
        m_uiKickTimer               = 18000;
        m_uiBlindTimer              = 5000;
        m_uiPhase                   = PHASE_NORMAL;

        if (m_pInstance)
            m_pInstance->SetData(TYPE_ZATH, NOT_STARTED);

        Revive(true);
    }

    void Aggro(Unit* /*pWho*/) override
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_ZATH, IN_PROGRESS);

        DoCastSpellIfCan(m_creature, SPELL_DUAL_WIELD);
    }

    void OnFakeingDeath()
    {
        m_uiResurrectTimer = 10000;

        if (m_pInstance)
            m_pInstance->SetData(TYPE_ZATH, SPECIAL);
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        switch (m_uiPhase)
        {
            case PHASE_FAKE_DEATH:
                if (m_uiResurrectTimer < uiDiff)
                {
                    if (!m_pInstance)
                        return;

                    if (m_pInstance->GetData(TYPE_THEKAL) != SPECIAL || m_pInstance->GetData(TYPE_LORKHAN) != SPECIAL)
                    {
                        DoCastSpellIfCan(m_creature, SPELL_RESURRECT);
                        m_pInstance->SetData(TYPE_ZATH, IN_PROGRESS);
                    }

                    m_uiPhase = PHASE_WAITING;
                }
                else
                    m_uiResurrectTimer -= uiDiff;

            // no break needed here
            case PHASE_WAITING:
                return;

            case PHASE_NORMAL:
                // SinisterStrike_Timer
                if (m_uiSinisterStrikeTimer < uiDiff)
                {
                    if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_SINISTER_STRIKE) == CAST_OK)
                        m_uiSinisterStrikeTimer = urand(8000, 16000);
                }
                else
                    m_uiSinisterStrikeTimer -= uiDiff;

                // Gouge_Timer
                if (m_uiGougeTimer < uiDiff)
                {
                    if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_GOUGE) == CAST_OK)
                    {
                        if (m_creature->getThreatManager().getThreat(m_creature->getVictim()))
                            m_creature->getThreatManager().modifyThreatPercent(m_creature->getVictim(), -100);

                        m_uiGougeTimer = urand(17000, 27000);
                    }
                }
                else
                    m_uiGougeTimer -= uiDiff;

                // Kick_Timer
                if (m_uiKickTimer < uiDiff)
                {
                    if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_KICK) == CAST_OK)
                        m_uiKickTimer = urand(15000, 25000);
                }
                else
                    m_uiKickTimer -= uiDiff;

                // Blind_Timer
                if (m_uiBlindTimer < uiDiff)
                {
                    if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_BLIND) == CAST_OK)
                        m_uiBlindTimer = urand(10000, 20000);
                }
                else
                    m_uiBlindTimer -= uiDiff;

                break;
        }

        DoMeleeAttackIfReady();
    }
};

bool EffectDummyCreature_thekal_resurrection(Unit* /*pCaster*/, uint32 uiSpellId, SpellEffectIndex uiEffIndex, Creature* pCreatureTarget, ObjectGuid /*originalCasterGuid*/)
{
    // always check spellid and effectindex
    if (uiSpellId == SPELL_RESURRECT && uiEffIndex == EFFECT_INDEX_0)
    {
        if (boss_thekalBaseAI* pFakerAI = dynamic_cast<boss_thekalBaseAI*>(pCreatureTarget->AI()))
            pFakerAI->Revive();

        // always return true when we are handling this spell and effect
        return true;
    }

    return false;
}

UnitAI* GetAI_boss_thekal(Creature* pCreature)
{
    return new boss_thekalAI(pCreature);
}

UnitAI* GetAI_mob_zealot_lorkhan(Creature* pCreature)
{
    return new mob_zealot_lorkhanAI(pCreature);
}

UnitAI* GetAI_mob_zealot_zath(Creature* pCreature)
{
    return new mob_zealot_zathAI(pCreature);
}

void AddSC_boss_thekal()
{
    Script* pNewScript = new Script;
    pNewScript->Name = "boss_thekal";
    pNewScript->GetAI = &GetAI_boss_thekal;
    pNewScript->pEffectDummyNPC = &EffectDummyCreature_thekal_resurrection;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "mob_zealot_lorkhan";
    pNewScript->GetAI = &GetAI_mob_zealot_lorkhan;
    pNewScript->pEffectDummyNPC = &EffectDummyCreature_thekal_resurrection;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "mob_zealot_zath";
    pNewScript->GetAI = &GetAI_mob_zealot_zath;
    pNewScript->pEffectDummyNPC = &EffectDummyCreature_thekal_resurrection;
    pNewScript->RegisterSelf();
}
