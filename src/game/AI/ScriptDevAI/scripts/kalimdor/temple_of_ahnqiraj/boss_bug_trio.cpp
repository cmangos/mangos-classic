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
SDComment: Summon Player spell NYI (when current target is unreachable); Consume mechanics is missing (only dummy implementation)
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
    SPELL_DISPEL            = 25808,
    SPELL_FEAR              = 26580,
    SPELL_RAVAGE            = 3242,
    SPELL_SUMMON_BROOD      = 25789,
    SPELL_DESPAWN_BROOD     = 25792,
};

struct Location
{
    float m_fX, m_fY, m_fZ;
};

static const Location resetPoint = { -8582.0f, 2047.0f, -1.62f };

struct boss_silithidRoyaltyAI : public ScriptedAI
{
    boss_silithidRoyaltyAI(Creature* creature) : ScriptedAI(creature)
    {
        m_instance = (ScriptedInstance*)creature->GetInstanceData();
        Reset();
    }

    ScriptedInstance* m_instance;

    uint32 m_deathAbility;
    uint32 m_resetCheckTimer;

    void Reset() override
    {
        m_resetCheckTimer = 2 * IN_MILLISECONDS;
    }

    void Aggro(Unit* /*who*/) override
    {
        if (m_instance)
            m_instance->SetData(TYPE_BUG_TRIO, IN_PROGRESS);
    }

    void JustReachedHome() override
    {
        // Clean-up stage
        DoCastSpellIfCan(m_creature, SPELL_DESPAWN_BROOD, CAST_TRIGGERED);

        if (m_instance)
            m_instance->SetData(TYPE_BUG_TRIO, FAIL);
    }

    // Handle damage to trigger consume when the two bosses that are killed first
    void DamageTaken(Unit* doneBy, uint32& damage, DamageEffectType damagetype, SpellEntry const* spellInfo) override
    {
        if (!m_instance)
            return;

        if (m_creature->GetHealth() <= damage)
        {
            // Inform the instance script that we are ready to be consumed
            // The instance script keeps track of how many bosses are already defeated
            m_instance->SetData(TYPE_BUG_TRIO, SPECIAL);

            // Ask the instance script if we are the last boss alive
            // If not, let be consumed
            if (m_instance && m_instance->GetData(TYPE_BUG_TRIO) != DONE)
            {
                damage = 0;
                m_creature->SetHealth(1);
                DoDummyConsume();
                return;
            }
        }

        ScriptedAI::DamageTaken(doneBy, damage, damagetype, spellInfo);
    }

    void JustDied(Unit* /* killer */) override
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
        DoCastSpellIfCan(m_creature, m_deathAbility);
    }

    // Return true to handle shared timers and MeleeAttack
    virtual bool UpdateBugAI(const uint32 /*diff*/) { return true; }

    void UpdateAI(const uint32 diff) override
    {
        // Return since we have no target
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        // Call silithid specific virtual function
        if (!UpdateBugAI(diff))
            return;

        // Periodically check that we are not leashed out of encounter area
        if (m_resetCheckTimer < diff)
        {
            if (m_instance && m_creature->GetDistance(resetPoint.m_fX, resetPoint.m_fY, resetPoint.m_fZ, DIST_CALC_COMBAT_REACH) < 10.0f)
                m_instance->SetData(TYPE_BUG_TRIO, FAIL);
            m_resetCheckTimer = 2 * IN_MILLISECONDS;
        }
        else
            m_resetCheckTimer -= diff;
    }
};

struct boss_kriAI : public boss_silithidRoyaltyAI
{
    boss_kriAI(Creature* creature) : boss_silithidRoyaltyAI(creature)
    {
        m_instance = (ScriptedInstance*)creature->GetInstanceData();
        Reset();
    }

    ScriptedInstance* m_instance;

    uint32 m_cleaveTimer;
    uint32 m_toxicVolleyTimer;
    uint32 m_thrashTimer;

    void Reset() override
    {
        m_deathAbility     = SPELL_SUMMON_CLOUD;
        m_cleaveTimer      = urand(4, 8) * IN_MILLISECONDS;
        m_toxicVolleyTimer = urand(6, 30) * IN_MILLISECONDS;
        m_thrashTimer      = 6 * IN_MILLISECONDS;

        m_creature->SetStandState(UNIT_STAND_STATE_STAND);
    }

    bool UpdateBugAI(const uint32 diff) override
    {
        // Cleave
        if (m_cleaveTimer < diff)
        {
            if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_CLEAVE) == CAST_OK)
                m_cleaveTimer = urand(5, 12) * IN_MILLISECONDS;
        }
        else
            m_cleaveTimer -= diff;

        // Toxic Volley
        if (m_toxicVolleyTimer < diff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_TOXIC_VOLLEY) == CAST_OK)
                m_toxicVolleyTimer = urand(15, 25) * IN_MILLISECONDS;
        }
        else
            m_toxicVolleyTimer -= diff;

        // Thrash
        if (m_thrashTimer < diff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_THRASH) == CAST_OK)
                m_thrashTimer = urand(2, 6) * IN_MILLISECONDS;
        }
        else
            m_thrashTimer -= diff;

        DoMeleeAttackIfReady();

        return true;
    }
};

struct boss_vemAI : public boss_silithidRoyaltyAI
{
    boss_vemAI(Creature* creature) : boss_silithidRoyaltyAI(creature)
    {
        m_instance = (ScriptedInstance*)creature->GetInstanceData();
        Reset();
    }

    ScriptedInstance* m_instance;

    uint32 m_chargeTimer;
    uint32 m_knockAwayTimer;
    uint32 m_knockDownTimer;

    void Reset() override
    {
        m_deathAbility     = SPELL_VENGEANCE;

        m_chargeTimer     = urand(15, 27) * IN_MILLISECONDS;
        m_knockAwayTimer  = urand(10, 20) * IN_MILLISECONDS;
        m_knockDownTimer  = urand(5, 8) * IN_MILLISECONDS;

        m_creature->SetStandState(UNIT_STAND_STATE_STAND);

        m_creature->SetWalk(false);
    }

    bool UpdateBugAI(const uint32 diff) override
    {
        // Knock Away
        if (m_knockAwayTimer < diff)
        {
            if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_KNOCK_AWAY) == CAST_OK)
                m_knockAwayTimer = urand(10, 20) * IN_MILLISECONDS;
        }
        else
            m_knockAwayTimer -= diff;

        // Knockdown
        if (m_knockDownTimer < diff)
        {
            if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_KNOCKDOWN) == CAST_OK)
                m_knockDownTimer = urand(15, 20) * IN_MILLISECONDS;
        }
        else
            m_knockDownTimer -= diff;

        // Charge Timer
        if (m_chargeTimer < diff)
        {
            if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0, SPELL_CHARGE, SELECT_FLAG_NOT_IN_MELEE_RANGE))
            {
                if (DoCastSpellIfCan(pTarget, SPELL_CHARGE) == CAST_OK)
                    m_chargeTimer = urand(8, 16) * IN_MILLISECONDS;
            }
        }
        else
            m_chargeTimer -= diff;

        DoMeleeAttackIfReady();

        return true;
    }
};

struct boss_yaujAI : public boss_silithidRoyaltyAI
{
    boss_yaujAI(Creature* creature) : boss_silithidRoyaltyAI(creature)
    {
        m_instance = (ScriptedInstance*)creature->GetInstanceData();
        Reset();
    }

    ScriptedInstance* m_instance;

    uint32 m_healTimer;
    uint32 m_fearTimer;
    uint32 m_ravageTimer;
    uint32 m_dispellTimer;

    void Reset() override
    {
        m_deathAbility    = SPELL_SUMMON_BROOD;

        m_healTimer       = urand(20, 30) * IN_MILLISECONDS;
        m_fearTimer       = urand(12, 24) * IN_MILLISECONDS;
        m_ravageTimer     = 12 * IN_MILLISECONDS;
        m_dispellTimer    = urand(7, 10) * IN_MILLISECONDS;

        m_creature->SetStandState(UNIT_STAND_STATE_STAND);
    }

    bool UpdateBugAI(const uint32 diff) override
    {
        // Fear
        if (m_fearTimer < diff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_FEAR) == CAST_OK)
            {
                DoResetThreat();
                m_fearTimer = 20 * IN_MILLISECONDS;
            }
        }
        else
            m_fearTimer -= diff;

        // Dispel
        if (m_dispellTimer < diff)
        {
            Unit* dispelTarget = m_creature->SelectRandomFriendlyTarget(nullptr, 100.0f);   // Look for any friendly unit in spell range, self dispel if none
            if (!dispelTarget)
                dispelTarget = m_creature;
            DoCastSpellIfCan(dispelTarget, SPELL_DISPEL);   // Don't check against CAST_OK because cast will fail if there is nothing to dispel but this is fine
            m_dispellTimer = urand(7, 10) * IN_MILLISECONDS;
        }
        else
            m_dispellTimer -= diff;

        // Heal self
        if (m_healTimer < diff)
        {
            if (m_creature->GetHealthPercent() < 100.0f)
            {
                if (DoCastSpellIfCan(m_creature, SPELL_HEAL) == CAST_OK)
                    m_healTimer = urand(10, 30) * IN_MILLISECONDS;
            }
        }
        else
            m_healTimer -= diff;

        // Ravage
        if (m_ravageTimer < diff)
        {
            if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_RAVAGE) == CAST_OK)
                m_ravageTimer = urand(10, 15) * IN_MILLISECONDS;
        }
        else
            m_ravageTimer -= diff;

        DoMeleeAttackIfReady();

        return true;
    }
};

UnitAI* GetAI_boss_yauj(Creature* creature)
{
    return new boss_yaujAI(creature);
}

UnitAI* GetAI_boss_vem(Creature* creature)
{
    return new boss_vemAI(creature);
}

UnitAI* GetAI_boss_kri(Creature* creature)
{
    return new boss_kriAI(creature);
}

void AddSC_bug_trio()
{
    Script* newScript = new Script;
    newScript->Name = "boss_kri";
    newScript->GetAI = &GetAI_boss_kri;
    newScript->RegisterSelf();

    newScript = new Script;
    newScript->Name = "boss_vem";
    newScript->GetAI = &GetAI_boss_vem;
    newScript->RegisterSelf();

    newScript = new Script;
    newScript->Name = "boss_yauj";
    newScript->GetAI = &GetAI_boss_yauj;
    newScript->RegisterSelf();
}
