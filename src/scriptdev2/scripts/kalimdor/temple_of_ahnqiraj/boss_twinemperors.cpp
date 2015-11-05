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
SDName: Boss_Twinemperors
SD%Complete: 95
SDComment: Timers
SDCategory: Temple of Ahn'Qiraj
EndScriptData */

#include "precompiled.h"
#include "temple_of_ahnqiraj.h"

enum
{
    // yells
    SAY_VEKLOR_AGGRO_1          = -1531019,
    SAY_VEKLOR_AGGRO_2          = -1531020,
    SAY_VEKLOR_AGGRO_3          = -1531021,
    SAY_VEKLOR_AGGRO_4          = -1531022,
    SAY_VEKLOR_SLAY             = -1531023,
    SAY_VEKLOR_DEATH            = -1531024,
    SAY_VEKLOR_SPECIAL          = -1531025,

    SAY_VEKNILASH_AGGRO_1       = -1531026,
    SAY_VEKNILASH_AGGRO_2       = -1531027,
    SAY_VEKNILASH_AGGRO_3       = -1531028,
    SAY_VEKNILASH_AGGRO_4       = -1531029,
    SAY_VEKNILASH_SLAY          = -1531030,
    SAY_VEKNILASH_DEATH         = -1531031,
    SAY_VEKNILASH_SPECIAL       = -1531032,

    // common spells
    SPELL_TWIN_TELEPORT         = 799,
    SPELL_TWIN_TELEPORT_STUN    = 800,
    SPELL_HEAL_BROTHER          = 7393,
    SPELL_TWIN_TELEPORT_VISUAL  = 26638,

    // veklor spells
    SPELL_ARCANE_BURST          = 568,
    SPELL_EXPLODE_BUG           = 804,          // targets 15316 or 15317
    SPELL_SHADOW_BOLT           = 26006,
    SPELL_BLIZZARD              = 26607,
    SPELL_FRENZY                = 27897,

    // veknilash spells
    SPELL_MUTATE_BUG            = 802,          // targets 15316 or 15317
    SPELL_UPPERCUT              = 26007,
    SPELL_UNBALANCING_STRIKE    = 26613,
    SPELL_BERSERK               = 27680,
};

struct boss_twin_emperorsAI : public ScriptedAI
{
    boss_twin_emperorsAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = (instance_temple_of_ahnqiraj*)pCreature->GetInstanceData();
        Reset();
    }

    instance_temple_of_ahnqiraj* m_pInstance;

    uint32 m_uiBerserkTimer;
    uint32 m_uiBugAbilityTimer;
    uint32 m_uiTeleportTimer;

    void Reset() override
    {
        m_uiTeleportTimer   = 35000;
        m_uiBugAbilityTimer = urand(7000, 14000);
        m_uiBerserkTimer    = 15 * MINUTE * IN_MILLISECONDS;
    }

    // Workaround for the shared health pool
    void DamageTaken(Unit* /*pDoneBy*/, uint32& uiDamage) override
    {
        if (!m_pInstance)
            return;

        if (Creature* pTwin = m_pInstance->GetSingleCreatureFromStorage(m_creature->GetEntry() == NPC_VEKLOR ? NPC_VEKNILASH : NPC_VEKLOR))
        {
            float fDamPercent = ((float)uiDamage) / ((float)m_creature->GetMaxHealth());
            uint32 uiTwinDamage = (uint32)(fDamPercent * ((float)pTwin->GetMaxHealth()));
            uint32 uiTwinHealth = pTwin->GetHealth() - uiTwinDamage;
            pTwin->SetHealth(uiTwinHealth > 0 ? uiTwinHealth : 0);

            if (!uiTwinHealth)
            {
                pTwin->SetDeathState(JUST_DIED);
                pTwin->RemoveFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_LOOTABLE);
            }
        }
    }

    // Workaround for the shared health pool
    void HealedBy(Unit* pHealer, uint32& uiHealedAmount) override
    {
        if (!m_pInstance)
            return;

        if (Creature* pTwin = m_pInstance->GetSingleCreatureFromStorage(m_creature->GetEntry() == NPC_VEKLOR ? NPC_VEKNILASH : NPC_VEKLOR))
        {
            float fHealPercent = ((float)uiHealedAmount) / ((float)m_creature->GetMaxHealth());
            uint32 uiTwinHeal = (uint32)(fHealPercent * ((float)pTwin->GetMaxHealth()));
            uint32 uiTwinHealth = pTwin->GetHealth() + uiTwinHeal;
            pTwin->SetHealth(uiTwinHealth < pTwin->GetMaxHealth() ? uiTwinHealth : pTwin->GetMaxHealth());
        }
    }

    void Aggro(Unit* /*pWho*/) override
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_TWINS, IN_PROGRESS);
    }

    void JustDied(Unit* /*pKiller*/) override
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_TWINS, DONE);
    }

    void JustReachedHome() override
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_TWINS, FAIL);
    }

    void SpellHit(Unit* /*pCaster*/, const SpellEntry* pSpell) override
    {
        if (pSpell->Id == SPELL_TWIN_TELEPORT)
        {
            DoTeleportAbility();
            DoResetThreat();
            DoCastSpellIfCan(m_creature, SPELL_TWIN_TELEPORT_STUN, CAST_TRIGGERED);
            DoCastSpellIfCan(m_creature, SPELL_TWIN_TELEPORT_VISUAL, CAST_TRIGGERED);
        }
    }

    // Return true, if succeeded
    virtual void DoTeleportAbility() {}
    virtual bool DoHandleBugAbility() = 0;
    virtual bool DoHandleBerserk() = 0;

    // Return true to handle shared timers and MeleeAttack
    virtual bool UpdateEmperorAI(const uint32 /*uiDiff*/) { return true; }

    void UpdateAI(const uint32 uiDiff) override
    {
        // Return since we have no target
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        // Call emperor specific virtual function
        if (!UpdateEmperorAI(uiDiff))
            return;

        if (m_uiTeleportTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_TWIN_TELEPORT) == CAST_OK)
                m_uiTeleportTimer = 35000;
        }
        else
            m_uiTeleportTimer -= uiDiff;

        if (m_uiBugAbilityTimer < uiDiff)
        {
            if (DoHandleBugAbility())
                m_uiBugAbilityTimer = urand(10000, 17000);
        }
        else
            m_uiBugAbilityTimer -= uiDiff;

        if (m_uiBerserkTimer)
        {
            if (m_uiBerserkTimer <= uiDiff)
            {
                if (DoHandleBerserk())
                    m_uiBerserkTimer = 0;
            }
            else
                m_uiBerserkTimer -= uiDiff;
        }
    }
};

struct boss_veknilashAI : public boss_twin_emperorsAI
{
    boss_veknilashAI(Creature* pCreature) : boss_twin_emperorsAI(pCreature) { Reset(); }

    uint32 m_uiUppercutTimer;
    uint32 m_uiUnbalancingStrikeTimer;

    void Reset() override
    {
        boss_twin_emperorsAI::Reset();

        m_uiUppercutTimer           = urand(14000, 29000);
        m_uiUnbalancingStrikeTimer = urand(8000, 18000);
    }

    void MoveInLineOfSight(Unit* pWho) override
    {
        if (m_pInstance && m_pInstance->GetData(TYPE_TWINS) == IN_PROGRESS && pWho->GetEntry() == NPC_VEKLOR && pWho->IsWithinDistInMap(m_creature, 60.0f))
            DoCastSpellIfCan(pWho, SPELL_HEAL_BROTHER);

        ScriptedAI::MoveInLineOfSight(pWho);
    }

    void Aggro(Unit* pWho) override
    {
        boss_twin_emperorsAI::Aggro(pWho);

        switch (urand(0, 3))
        {
            case 0: DoScriptText(SAY_VEKNILASH_AGGRO_1, m_creature); break;
            case 1: DoScriptText(SAY_VEKNILASH_AGGRO_2, m_creature); break;
            case 2: DoScriptText(SAY_VEKNILASH_AGGRO_3, m_creature); break;
            case 3: DoScriptText(SAY_VEKNILASH_AGGRO_4, m_creature); break;
        }
    }

    void KilledUnit(Unit* /*pVictim*/) override
    {
        DoScriptText(SAY_VEKNILASH_SLAY, m_creature);
    }

    void JustDied(Unit* pKiller) override
    {
        boss_twin_emperorsAI::JustDied(pKiller);

        DoScriptText(SAY_VEKNILASH_DEATH, m_creature);
    }

    bool DoHandleBugAbility()
    {
        if (DoCastSpellIfCan(m_creature, SPELL_MUTATE_BUG) == CAST_OK)
            return true;

        return false;
    }

    bool DoHandleBerserk()
    {
        if (DoCastSpellIfCan(m_creature, SPELL_BERSERK) == CAST_OK)
            return true;

        return false;
    }

    // Only Vek'nilash handles the teleport for both of them
    void DoTeleportAbility()
    {
        if (!m_pInstance)
            return;

        if (Creature* pVeklor = m_pInstance->GetSingleCreatureFromStorage(NPC_VEKLOR))
        {
            float fTargetX, fTargetY, fTargetZ, fTargetOrient;
            pVeklor->GetPosition(fTargetX, fTargetY, fTargetZ);
            fTargetOrient = pVeklor->GetOrientation();

            pVeklor->NearTeleportTo(m_creature->GetPositionX(), m_creature->GetPositionY(), m_creature->GetPositionZ(), m_creature->GetOrientation(), true);
            m_creature->NearTeleportTo(fTargetX, fTargetY, fTargetZ, fTargetOrient, true);
        }
    }

    bool UpdateEmperorAI(const uint32 uiDiff)
    {
        if (m_uiUnbalancingStrikeTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_UNBALANCING_STRIKE) == CAST_OK)
                m_uiUnbalancingStrikeTimer = urand(8000, 20000);
        }
        else
            m_uiUnbalancingStrikeTimer -= uiDiff;

        if (m_uiUppercutTimer < uiDiff)
        {
            if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0, SPELL_UPPERCUT, SELECT_FLAG_IN_MELEE_RANGE))
            {
                if (DoCastSpellIfCan(pTarget, SPELL_UPPERCUT) == CAST_OK)
                    m_uiUppercutTimer = urand(15000, 30000);
            }
        }
        else
            m_uiUppercutTimer -= uiDiff;

        DoMeleeAttackIfReady();

        return true;
    }
};

struct boss_veklorAI : public boss_twin_emperorsAI
{
    boss_veklorAI(Creature* pCreature) : boss_twin_emperorsAI(pCreature) { Reset(); }

    uint32 m_uiShadowBoltTimer;
    uint32 m_uiBlizzardTimer;
    uint32 m_uiArcaneBurstTimer;

    void Reset() override
    {
        boss_twin_emperorsAI::Reset();

        m_uiShadowBoltTimer    = 1000;
        m_uiBlizzardTimer      = urand(15000, 20000);
        m_uiArcaneBurstTimer   = 1000;
    }

    void MoveInLineOfSight(Unit* pWho) override
    {
        if (m_pInstance && m_pInstance->GetData(TYPE_TWINS) == IN_PROGRESS && pWho->GetEntry() == NPC_VEKNILASH && pWho->IsWithinDistInMap(m_creature, 60.0f))
            DoCastSpellIfCan(pWho, SPELL_HEAL_BROTHER);

        ScriptedAI::MoveInLineOfSight(pWho);
    }

    void Aggro(Unit* pWho) override
    {
        boss_twin_emperorsAI::Aggro(pWho);

        switch (urand(0, 3))
        {
            case 0: DoScriptText(SAY_VEKLOR_AGGRO_1, m_creature); break;
            case 1: DoScriptText(SAY_VEKLOR_AGGRO_2, m_creature); break;
            case 2: DoScriptText(SAY_VEKLOR_AGGRO_3, m_creature); break;
            case 3: DoScriptText(SAY_VEKLOR_AGGRO_4, m_creature); break;
        }
    }

    void KilledUnit(Unit* /*pVictim*/) override
    {
        DoScriptText(SAY_VEKLOR_SLAY, m_creature);
    }

    void JustDied(Unit* pKiller) override
    {
        boss_twin_emperorsAI::JustDied(pKiller);

        DoScriptText(SAY_VEKLOR_DEATH, m_creature);
    }

    void AttackStart(Unit* pWho) override
    {
        if (m_creature->Attack(pWho, false))
        {
            m_creature->AddThreat(pWho);
            m_creature->SetInCombatWith(pWho);
            pWho->SetInCombatWith(m_creature);
            m_creature->GetMotionMaster()->MoveChase(pWho, 20.0f);
        }
    }

    bool DoHandleBugAbility()
    {
        if (DoCastSpellIfCan(m_creature, SPELL_EXPLODE_BUG) == CAST_OK)
            return true;

        return false;
    }

    bool DoHandleBerserk()
    {
        if (DoCastSpellIfCan(m_creature, SPELL_FRENZY) == CAST_OK)
            return true;

        return false;
    }

    bool UpdateEmperorAI(const uint32 uiDiff)
    {
        if (m_uiShadowBoltTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_SHADOW_BOLT) == CAST_OK)
                m_uiShadowBoltTimer = 2000;
        }
        else
            m_uiShadowBoltTimer -= uiDiff;

        if (m_uiBlizzardTimer < uiDiff)
        {
            if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
            {
                if (DoCastSpellIfCan(pTarget, SPELL_BLIZZARD) == CAST_OK)
                    m_uiBlizzardTimer = urand(15000, 30000);
            }
        }
        else
            m_uiBlizzardTimer -= uiDiff;

        if (m_uiArcaneBurstTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_ARCANE_BURST) == CAST_OK)
                m_uiArcaneBurstTimer = 5000;
        }
        else
            m_uiArcaneBurstTimer -= uiDiff;

        return true;
    }
};

CreatureAI* GetAI_boss_veknilash(Creature* pCreature)
{
    return new boss_veknilashAI(pCreature);
}

CreatureAI* GetAI_boss_veklor(Creature* pCreature)
{
    return new boss_veklorAI(pCreature);
}

void AddSC_boss_twinemperors()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "boss_veknilash";
    pNewScript->GetAI = &GetAI_boss_veknilash;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "boss_veklor";
    pNewScript->GetAI = &GetAI_boss_veklor;
    pNewScript->RegisterSelf();
}
