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
SDName: bug_trio
SD%Complete: 90
SDComment: Summon Player spell NYI; Consume mechanics is missing (only dummy implementation)
SDCategory: Temple of Ahn'Qiraj
EndScriptData

*/

#include "AI/ScriptDevAI/include/precompiled.h"
#include "temple_of_ahnqiraj.h"

enum
{
    EMOTE_CONSUMED          = -1531048,

    // Lord Kri
    SPELL_CLEAVE            = 26350,
    SPELL_TOXIC_VOLLEY      = 25812,
    SPELL_SUMMON_CLOUD      = 26590,            // summons 15933
    SPELL_THRASH            = 3391,

    // Vem
    SPELL_KNOCK_AWAY        = 18670,
    SPELL_KNOCKDOWN         = 19128,
    SPELL_CHARGE            = 26561,
    SPELL_VENGEANCE         = 25790,

    // Princess Yauj
    SPELL_HEAL              = 25807,
    SPELL_FEAR              = 26580,
    SPELL_RAVAGE            = 3242,
    SPELL_SUMMON_BROOD      = 25789,
};

struct Location
{
    float m_fX, m_fY, m_fZ;
};

static const Location resetPoint = { -8582.0f, 2047.0f, -1.62f };

struct boss_silithidRoyaltyAI : public ScriptedAI
{
    boss_silithidRoyaltyAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
        Reset();
    }

    ScriptedInstance* m_pInstance;

    uint32 m_uiDeathAbility;
    uint32 m_uiResetCheckTimer;

    void Reset() override
    {
        m_uiResetCheckTimer = 2 * IN_MILLISECONDS;
    }

    void Aggro(Unit* /*pWho*/) override
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_BUG_TRIO, IN_PROGRESS);
    }

    void JustReachedHome() override
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_BUG_TRIO, FAIL);
    }

    // Handle damage to trigger consume when the two bosses that are killed first
    void DamageTaken(Unit* pDoneBy, uint32& damage, DamageEffectType damagetype, SpellEntry const* spellInfo) override
    {
        if (!m_pInstance)
            return;

        if (m_creature->GetHealth() <= damage)
        {
            // Inform the instance script that we are ready to be consumed
            // The instance script keeps track of how many bosses are already defeated
            m_pInstance->SetData(TYPE_BUG_TRIO, SPECIAL);

            // Ask the instance script if we are the last boss alive
            // If not, let be consumed
            if (m_pInstance && m_pInstance->GetData(TYPE_BUG_TRIO) != DONE)
            {
                damage = 0;
                m_creature->SetHealth(1);
                DoDummyConsume();
                return;
            }
        }

        ScriptedAI::DamageTaken(pDoneBy, damage, damagetype, spellInfo);
    }

    void JustDied(Unit* pKiller) override
    {
        DoSpecialAbility();
    }

    void DoDummyConsume()
    {
        DoScriptText(EMOTE_CONSUMED, m_creature);
        m_creature->SetStandState(UNIT_STAND_STATE_DEAD);
        DoSpecialAbility();
        m_creature->ForcedDespawn(3000);
    }

    void DoSpecialAbility()
    {
        DoCastSpellIfCan(m_creature, m_uiDeathAbility);
    }

    // Return true to handle shared timers and MeleeAttack
    virtual bool UpdateBugAI(const uint32 /*uiDiff*/) { return true; }

    void UpdateAI(const uint32 uiDiff) override
    {
        // Return since we have no target
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        // Call silithid specific virtual function
        if (!UpdateBugAI(uiDiff))
            return;

        // Periodically check that we are not leashed out of encounter area
        if (m_uiResetCheckTimer < uiDiff)
        {
            if (m_pInstance && m_creature->GetDistance(resetPoint.m_fX, resetPoint.m_fY, resetPoint.m_fZ, DIST_CALC_COMBAT_REACH) < 10.0f)
                m_pInstance->SetData(TYPE_BUG_TRIO, FAIL);
            m_uiResetCheckTimer = 2 * IN_MILLISECONDS;
        }
        else
            m_uiResetCheckTimer -= uiDiff;
    }
};

struct boss_kriAI : public boss_silithidRoyaltyAI
{
    boss_kriAI(Creature* pCreature) : boss_silithidRoyaltyAI(pCreature)
    {
        m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
        Reset();
    }

    ScriptedInstance* m_pInstance;

    uint32 m_uiCleaveTimer;
    uint32 m_uiToxicVolleyTimer;
    uint32 m_uiThrashTimer;

    void Reset() override
    {
        m_uiDeathAbility     = SPELL_SUMMON_CLOUD;
        m_uiCleaveTimer      = urand(4, 8) * IN_MILLISECONDS;
        m_uiToxicVolleyTimer = urand(6, 30) * IN_MILLISECONDS;
        m_uiThrashTimer      = 6 * IN_MILLISECONDS;

        m_creature->SetStandState(UNIT_STAND_STATE_STAND);
    }

    bool UpdateBugAI(const uint32 uiDiff) override
    {
        // Return since we have no target
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return false;

        // Cleave
        if (m_uiCleaveTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_CLEAVE) == CAST_OK)
                m_uiCleaveTimer = urand(5, 12) * IN_MILLISECONDS;
        }
        else
            m_uiCleaveTimer -= uiDiff;

        // Toxic Volley
        if (m_uiToxicVolleyTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_TOXIC_VOLLEY) == CAST_OK)
                m_uiToxicVolleyTimer = urand(15, 25) * IN_MILLISECONDS;
        }
        else
            m_uiToxicVolleyTimer -= uiDiff;

        // Thrash
        if (m_uiThrashTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_THRASH) == CAST_OK)
                m_uiThrashTimer = urand(2, 6) * IN_MILLISECONDS;
        }
        else
            m_uiThrashTimer -= uiDiff;

        DoMeleeAttackIfReady();

        return true;
    }
};

struct boss_vemAI : public boss_silithidRoyaltyAI
{
    boss_vemAI(Creature* pCreature) : boss_silithidRoyaltyAI(pCreature)
    {
        m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
        Reset();
    }

    ScriptedInstance* m_pInstance;

    uint32 m_uiChargeTimer;
    uint32 m_uiKnockAwayTimer;
    uint32 m_uiKnockDownTimer;

    void Reset() override
    {
        m_uiDeathAbility     = SPELL_VENGEANCE;

        m_uiChargeTimer     = urand(15, 27) * IN_MILLISECONDS;
        m_uiKnockAwayTimer  = urand(10, 20) * IN_MILLISECONDS;
        m_uiKnockDownTimer  = urand(5, 8) * IN_MILLISECONDS;

        m_creature->SetStandState(UNIT_STAND_STATE_STAND);

        m_creature->SetWalk(false);
    }

    bool UpdateBugAI(const uint32 uiDiff) override
    {
        // Return since we have no target
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return false;

        // Knock Away
        if (m_uiKnockAwayTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_KNOCK_AWAY) == CAST_OK)
                m_uiKnockAwayTimer = urand(10, 20) * IN_MILLISECONDS;
        }
        else
            m_uiKnockAwayTimer -= uiDiff;

        // Knockdown
        if (m_uiKnockDownTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_KNOCKDOWN) == CAST_OK)
                m_uiKnockDownTimer = urand(15, 20) * IN_MILLISECONDS;
        }
        else
            m_uiKnockDownTimer -= uiDiff;

        // Charge Timer
        if (m_uiChargeTimer < uiDiff)
        {
            if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0, SPELL_CHARGE, SELECT_FLAG_NOT_IN_MELEE_RANGE))
            {
                if (DoCastSpellIfCan(pTarget, SPELL_CHARGE) == CAST_OK)
                    m_uiChargeTimer = urand(8, 16) * IN_MILLISECONDS;
            }
        }
        else
            m_uiChargeTimer -= uiDiff;

        DoMeleeAttackIfReady();

        return true;
    }
};

struct boss_yaujAI : public boss_silithidRoyaltyAI
{
    boss_yaujAI(Creature* pCreature) : boss_silithidRoyaltyAI(pCreature)
    {
        m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
        Reset();
    }

    ScriptedInstance* m_pInstance;

    uint32 m_uiHealTimer;
    uint32 m_uiFearTimer;
    uint32 m_uiRavageTimer;

    void Reset() override
    {
        m_uiDeathAbility    = SPELL_SUMMON_BROOD;

        m_uiHealTimer       = urand(20, 30) * IN_MILLISECONDS;
        m_uiFearTimer       = urand(12, 24) * IN_MILLISECONDS;
        m_uiRavageTimer     = 12 * IN_MILLISECONDS;

        m_creature->SetStandState(UNIT_STAND_STATE_STAND);
    }

    bool UpdateBugAI(const uint32 uiDiff) override
    {
        // Return since we have no target
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return false;

        // Fear
        if (m_uiFearTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_FEAR) == CAST_OK)
            {
                DoResetThreat();
                m_uiFearTimer = 20 * IN_MILLISECONDS;
            }
        }
        else
            m_uiFearTimer -= uiDiff;

        // Heal
        if (m_uiHealTimer < uiDiff)
        {
            if (Unit* pTarget = DoSelectLowestHpFriendly(100.0f))
            {
                if (DoCastSpellIfCan(pTarget, SPELL_HEAL) == CAST_OK)
                    m_uiHealTimer = urand(10, 30) * IN_MILLISECONDS;
            }
        }
        else
            m_uiHealTimer -= uiDiff;

        // Ravage
        if (m_uiRavageTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_RAVAGE) == CAST_OK)
                m_uiRavageTimer = urand(10, 15) * IN_MILLISECONDS;
        }
        else
            m_uiRavageTimer -= uiDiff;

        DoMeleeAttackIfReady();

        return true;
    }
};

UnitAI* GetAI_boss_yauj(Creature* pCreature)
{
    return new boss_yaujAI(pCreature);
}

UnitAI* GetAI_boss_vem(Creature* pCreature)
{
    return new boss_vemAI(pCreature);
}

UnitAI* GetAI_boss_kri(Creature* pCreature)
{
    return new boss_kriAI(pCreature);
}

void AddSC_bug_trio()
{
    Script* pNewScript = new Script;
    pNewScript->Name = "boss_kri";
    pNewScript->GetAI = &GetAI_boss_kri;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "boss_vem";
    pNewScript->GetAI = &GetAI_boss_vem;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "boss_yauj";
    pNewScript->GetAI = &GetAI_boss_yauj;
    pNewScript->RegisterSelf();
}
