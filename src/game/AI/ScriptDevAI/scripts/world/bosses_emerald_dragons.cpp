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
SDName: bosses_emerald_dragons
SD%Complete: 99
SDComment: Correct models used by Spirit Shade for each race/gender combination are missing (Lethon)
SDCategory: Emerald Dragon Bosses
EndScriptData

*/

/* ContentData
boss_emerald_dragon -- Superclass for the four dragons
boss_emeriss
boss_lethon
npc_spirit_shade
boss_taerar
boss_ysondre
EndContentData */

#include "AI/ScriptDevAI/include/precompiled.h"
#include "World/WorldState.h"

/*######
## boss_emerald_dragon -- Superclass for the four dragons
######*/

enum
{
    SPELL_MARK_OF_NATURE_PLAYER     = 25040,
    SPELL_MARK_OF_NATURE_AURA       = 25041,
    SPELL_SEEPING_FOG_R             = 24813,                // Summons NPC 15224 (Dream Fog) that cast auras 24777 and 24780 on spawn
    SPELL_SEEPING_FOG_L             = 24814,
    SPELL_NOXIOUS_BREATH            = 24818,
    SPELL_TAILSWEEP                 = 15847,
    // SPELL_SUMMON_PLAYER             = 24776,                // Not used in Classic

    NPC_YSONDRE                     = 14887,
};

struct boss_emerald_dragonAI : public ScriptedAI
{
    boss_emerald_dragonAI(Creature* pCreature) : ScriptedAI(pCreature) { Reset(); }

    uint32 m_uiEventCounter;

    uint32 m_uiSeepingFogTimer;
    uint32 m_uiNoxiousBreathTimer;
    uint32 m_uiTailsweepTimer;

    void Reset() override
    {
        m_uiEventCounter = 1;

        m_uiSeepingFogTimer = urand(15000, 20000);
        m_uiNoxiousBreathTimer = 8000;
        m_uiTailsweepTimer = 4000;
    }

    void EnterCombat(Unit* pEnemy) override
    {
        DoCastSpellIfCan(m_creature, SPELL_MARK_OF_NATURE_AURA, CAST_TRIGGERED);

        ScriptedAI::EnterCombat(pEnemy);
    }

    void KilledUnit(Unit* pVictim) override
    {
        // Mark killed players with Mark of Nature
        if (pVictim->GetTypeId() == TYPEID_PLAYER)
            pVictim->CastSpell(pVictim, SPELL_MARK_OF_NATURE_PLAYER, TRIGGERED_OLD_TRIGGERED, nullptr, nullptr, m_creature->GetObjectGuid());
    }

    void JustSummoned(Creature* pSummoned) override
    {
        if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
            pSummoned->AI()->AttackStart(pTarget);
    }

    void JustDied(Unit* killer) override
    {
        sWorldState.HandleExternalEvent(m_creature->GetEntry() - NPC_YSONDRE + CUSTOM_EVENT_YSONDRE_DIED);
    }

    // Return true, if succeeded
    virtual bool DoSpecialDragonAbility() = 0;

    // Return true to handle shared timers and MeleeAttack
    virtual bool UpdateDragonAI(const uint32 /*uiDiff*/) { return true; }

    void UpdateAI(const uint32 uiDiff) override
    {
        // Return since we have no target
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        // Trigger special ability function at 75, 50 and 25% health
        if (m_creature->GetHealthPercent() < 100.0f - m_uiEventCounter * 25.0f && DoSpecialDragonAbility())
            ++m_uiEventCounter;

        // Call dragon specific virtual function
        if (!UpdateDragonAI(uiDiff))
            return;

        if (m_uiSeepingFogTimer < uiDiff)
        {
            DoCastSpellIfCan(m_creature, SPELL_SEEPING_FOG_R, CAST_TRIGGERED);
            DoCastSpellIfCan(m_creature, SPELL_SEEPING_FOG_L, CAST_TRIGGERED);
            m_uiSeepingFogTimer = urand(120000, 150000);    // Rather Guesswork, but one Fog has 2min duration, hence a bit longer
        }
        else
            m_uiSeepingFogTimer -= uiDiff;

        if (m_uiNoxiousBreathTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_NOXIOUS_BREATH) == CAST_OK)
                m_uiNoxiousBreathTimer = urand(14000, 20000);
        }
        else
            m_uiNoxiousBreathTimer -= uiDiff;

        if (m_uiTailsweepTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_TAILSWEEP) == CAST_OK)
                m_uiTailsweepTimer = urand(15000, 20000);
        }
        else
            m_uiTailsweepTimer -= uiDiff;

        DoMeleeAttackIfReady();
    }
};

/*######
## boss_emeriss
######*/

enum
{
    SAY_EMERISS_AGGRO           = -1000401,
    SAY_CAST_CORRUPTION         = -1000402,

    SPELL_VOLATILE_INFECTION    = 24928,
    SPELL_CORRUPTION_OF_EARTH   = 24910,
    SPELL_PUTRID_MUSHROOM       = 24904,                    // Summons a Putrid Mushroom (GO 180517 with trap spell 24871) on killing a player
    SPELL_DESPAWN_MUSHROOMS     = 24958,                    // Removes all Putrid Mushrooms in case of raid wipe
};

struct boss_emerissAI : public boss_emerald_dragonAI
{
    boss_emerissAI(Creature* pCreature) : boss_emerald_dragonAI(pCreature) { Reset(); }

    uint32 m_uiVolatileInfectionTimer;

    void Reset() override
    {
        boss_emerald_dragonAI::Reset();

        m_uiVolatileInfectionTimer = 12000;
        DoCastSpellIfCan(m_creature, SPELL_DESPAWN_MUSHROOMS, CAST_TRIGGERED);
    }

    void Aggro(Unit* /*pWho*/) override
    {
        DoScriptText(SAY_EMERISS_AGGRO, m_creature);
    }

    void KilledUnit(Unit* pVictim) override
    {
        // summon a mushroom on the spot the player dies
        if (pVictim->GetTypeId() == TYPEID_PLAYER)
            pVictim->CastSpell(pVictim, SPELL_PUTRID_MUSHROOM, TRIGGERED_OLD_TRIGGERED, nullptr, nullptr, m_creature->GetObjectGuid());

        boss_emerald_dragonAI::KilledUnit(pVictim);
    }

    // Corruption of Earth at 75%, 50% and 25%
    bool DoSpecialDragonAbility()
    {
        if (DoCastSpellIfCan(m_creature, SPELL_CORRUPTION_OF_EARTH) == CAST_OK)
        {
            DoScriptText(SAY_CAST_CORRUPTION, m_creature);

            // Successfull cast
            return true;
        }

        return false;
    }

    bool UpdateDragonAI(const uint32 uiDiff)
    {
        // Volatile Infection Timer
        if (m_uiVolatileInfectionTimer < uiDiff)
        {
            Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0);
            if (pTarget && DoCastSpellIfCan(pTarget, SPELL_VOLATILE_INFECTION) == CAST_OK)
                m_uiVolatileInfectionTimer = urand(7000, 12000);
        }
        else
            m_uiVolatileInfectionTimer -= uiDiff;

        return true;
    }
};

UnitAI* GetAI_boss_emeriss(Creature* pCreature)
{
    return new boss_emerissAI(pCreature);
}

/*######
## boss_lethon
######*/

enum
{
    SAY_LETHON_AGGRO            = -1000666,
    SAY_DRAW_SPIRIT             = -1000667,

    SPELL_SHADOW_BOLT_WIRL      = 24834,                    // Periodic aura
    SPELL_DRAW_SPIRIT           = 24811,
    // SPELL_SUMMON_SPIRIT_SHADE   = 24810,                    // Summon spell was removed in TBC, was using SPELL_EFFECT_SUMMON_DEMON
                                                               // Classic uses same code than TBC and WotLK for consistency

    NPC_LETHON                  = 14888,
    NPC_SPIRIT_SHADE            = 15261,                    // Add summoned by Lethon
    SPELL_DARK_OFFERING         = 24804,
    SPELL_SPIRIT_SHAPE_VISUAL   = 24809,
};

struct boss_lethonAI : public boss_emerald_dragonAI
{
    boss_lethonAI(Creature* pCreature) : boss_emerald_dragonAI(pCreature) {}

    void Aggro(Unit* /*pWho*/) override
    {
        DoScriptText(SAY_LETHON_AGGRO, m_creature);
        // Shadow bolt wirl is a periodic aura which triggers a set of shadowbolts every 2 secs
        DoCastSpellIfCan(m_creature, SPELL_SHADOW_BOLT_WIRL, CAST_TRIGGERED);
    }

    // Summon a spirit which moves toward the boss and heals him for each player hit by the spell; used at 75%, 50% and 25%
    bool DoSpecialDragonAbility()
    {
        if (DoCastSpellIfCan(m_creature, SPELL_DRAW_SPIRIT) == CAST_OK)
        {
            DoScriptText(SAY_DRAW_SPIRIT, m_creature);
            return true;
        }

        return false;
    }

    // Need this code here, as SPELL_DRAW_SPIRIT has no Script - or Dummy Effect
    void SpellHitTarget(Unit* pTarget, const SpellEntry* pSpell) override
    {
        // Summon a shade for each player hit
        if (pTarget->GetTypeId() == TYPEID_PLAYER && pSpell->Id == SPELL_DRAW_SPIRIT)
        {
            // Summon this way to be able to cast the shade visual spell with player as original caster
            // This is not currently supported by core but this spell's visual should be dependent on player
            // Also possible that this was no problem due to the special way these NPCs had been summoned in classic times
            if (Creature* pSummoned = pTarget->SummonCreature(NPC_SPIRIT_SHADE, 0.0f, 0.0f, 0.0f, pTarget->GetOrientation(), TEMPSPAWN_TIMED_OR_DEAD_DESPAWN, 60 * IN_MILLISECONDS, false, false, 0, 0, false, false, true))
                pSummoned->CastSpell(pSummoned, SPELL_SPIRIT_SHAPE_VISUAL, TRIGGERED_OLD_TRIGGERED, nullptr, nullptr, pTarget->GetObjectGuid());
        }
    }
};

struct npc_spirit_shadeAI : public ScriptedAI
{
    npc_spirit_shadeAI(Creature* pCreature) : ScriptedAI(pCreature) { Reset(); }

    bool m_bHasHealed;
    uint32 m_uiInitialMovementTimer;

    void Reset() override
    {
        m_bHasHealed = false;
        m_creature->SetVisibility(VISIBILITY_OFF);
        m_uiInitialMovementTimer = 3 * IN_MILLISECONDS;
    }

    void JustDied(Unit* /*pKiller*/) override
    {
        m_creature->ForcedDespawn();
    }

    void MoveInLineOfSight(Unit* pWho) override
    {
        if (m_uiInitialMovementTimer)   // Does nothing while movement is not initiated in UpdateAI()
            return;

        if (pWho->GetEntry() == NPC_LETHON)
        {
            if (!m_bHasHealed && pWho->IsWithinDistInMap(m_creature, 3.0f))
            {
                if (DoCastSpellIfCan(pWho, SPELL_DARK_OFFERING) == CAST_OK)
                {
                    m_bHasHealed = true;
                    m_creature->ForcedDespawn(1000);
                }
            }
            else
                m_creature->GetMotionMaster()->MovePoint(0, pWho->GetPositionX(), pWho->GetPositionY(), pWho->GetPositionZ());
        }
    }

    void AttackStart(Unit* /*pWho*/) override { }

    void UpdateAI(const uint32 uiDiff) override
    {
        // NPC does not start to move when summoned, it waits 4 seconds then removes invisibility and moves towards boss Lethon
        if (m_uiInitialMovementTimer)
        {
            if (m_uiInitialMovementTimer < uiDiff)
            {
                m_creature->SetVisibility(VISIBILITY_ON);
                m_uiInitialMovementTimer = 0;
            }
            else
                m_uiInitialMovementTimer -= uiDiff;
        }
    }
};

UnitAI* GetAI_boss_lethon(Creature* pCreature)
{
    return new boss_lethonAI(pCreature);
}

UnitAI* GetAI_npc_spirit_shade(Creature* pCreature)
{
    return new npc_spirit_shadeAI(pCreature);
}

/*######
## boss_taerar
######*/

enum
{
    SAY_TAERAR_AGGRO        = -1000399,
    SAY_SUMMONSHADE         = -1000400,

    SPELL_ARCANE_BLAST      = 24857,
    SPELL_BELLOWING_ROAR    = 22686,

    SPELL_SUMMON_SHADE_1    = 24841,                        // Summon one NPC 15302 (Shade of Taerar) that cast spells 24839 & 24840
    SPELL_SUMMON_SHADE_2    = 24842,
    SPELL_SUMMON_SHADE_3    = 24843,
    SPELL_SELF_STUN         = 24883,                        // Stuns the main boss until the shades are dead or timer expires
    SPELL_DESPAWN_SHADES    = 24886,

    NPC_SHADE_OF_TAERAR     = 15302,
};

struct boss_taerarAI : public boss_emerald_dragonAI
{
    boss_taerarAI(Creature* pCreature) : boss_emerald_dragonAI(pCreature) { Reset(); }

    uint32 m_uiArcaneBlastTimer;
    uint32 m_uiBellowingRoarTimer;
    uint32 m_uiShadesTimeoutTimer;
    uint8 m_uiShadesDead;

    void Reset() override
    {
        boss_emerald_dragonAI::Reset();

        m_uiArcaneBlastTimer = 12000;
        m_uiBellowingRoarTimer = 30000;
        m_uiShadesTimeoutTimer = 0;                         // The time that Taerar is banished
        m_uiShadesDead = 0;

        // Remove Unselectable if needed
        if (m_creature->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE))
            m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);

        // Despawn all remaining summoned NPCs in case of raid wipe
        DoCastSpellIfCan(m_creature, SPELL_DESPAWN_SHADES, CAST_TRIGGERED);
    }

    void Aggro(Unit* /*pWho*/) override
    {
        DoScriptText(SAY_TAERAR_AGGRO, m_creature);
    }

    // Summon 3 Shades at 75%, 50% and 25% and Banish Self
    bool DoSpecialDragonAbility()
    {
        if (DoCastSpellIfCan(m_creature, SPELL_SELF_STUN) == CAST_OK)
        {
            // Summon the shades at boss position
            DoCastSpellIfCan(m_creature, SPELL_SUMMON_SHADE_1, CAST_TRIGGERED);
            DoCastSpellIfCan(m_creature, SPELL_SUMMON_SHADE_2, CAST_TRIGGERED);
            DoCastSpellIfCan(m_creature, SPELL_SUMMON_SHADE_3, CAST_TRIGGERED);

            // Make boss not selectable when banished
            m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);

            DoScriptText(SAY_SUMMONSHADE, m_creature);
            m_uiShadesTimeoutTimer = 60000;

            return true;
        }

        return false;
    }

    void SummonedCreatureJustDied(Creature* pSummoned) override
    {
        if (pSummoned->GetEntry() == NPC_SHADE_OF_TAERAR)
        {
            ++m_uiShadesDead;

            // If all shades are dead then unbanish the boss
            if (m_uiShadesDead == 3)
                DoUnbanishBoss();
        }
    }

    void DoUnbanishBoss()
    {
        m_creature->RemoveAurasDueToSpell(SPELL_SELF_STUN);
        m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);

        m_uiShadesTimeoutTimer = 0;
        m_uiShadesDead = 0;
    }

    bool UpdateDragonAI(const uint32 uiDiff)
    {
        // Timer to unbanish the boss
        if (m_uiShadesTimeoutTimer)
        {
            if (m_uiShadesTimeoutTimer <= uiDiff)
                DoUnbanishBoss();
            else
                m_uiShadesTimeoutTimer -= uiDiff;

            // Prevent further spells or timer handling while banished
            return false;
        }

        // Arcane Blast Timer
        if (m_uiArcaneBlastTimer < uiDiff)
        {
            Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0);
            if (pTarget && DoCastSpellIfCan(pTarget, SPELL_ARCANE_BLAST) == CAST_OK)
                m_uiArcaneBlastTimer = urand(7000, 12000);
        }
        else
            m_uiArcaneBlastTimer -= uiDiff;

        // Bellowing Roar Timer
        if (m_uiBellowingRoarTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_BELLOWING_ROAR) == CAST_OK)
                m_uiBellowingRoarTimer = urand(20000, 30000);
        }
        else
            m_uiBellowingRoarTimer -= uiDiff;

        return true;
    }
};

UnitAI* GetAI_boss_taerar(Creature* pCreature)
{
    return new boss_taerarAI(pCreature);
}

/*######
## boss_ysondre
######*/

enum
{
    SAY_YSONDRE_AGGRO       = -1000360,
    SAY_SUMMON_DRUIDS       = -1000361,

    SPELL_LIGHTNING_WAVE    = 24819,
    SPELL_SUMMON_DRUIDS     = 24796,    // Summon NPC 15260 (Demented Druid Spirit) that uses spells 6726, 16247 & 24957
};

// Ysondre script
struct boss_ysondreAI : public boss_emerald_dragonAI
{
    boss_ysondreAI(Creature* pCreature) : boss_emerald_dragonAI(pCreature) { Reset(); }

    uint32 m_uiLightningWaveTimer;

    void Reset() override
    {
        boss_emerald_dragonAI::Reset();

        m_uiLightningWaveTimer = 12000;
    }

    void Aggro(Unit* /*pWho*/) override
    {
        DoScriptText(SAY_YSONDRE_AGGRO, m_creature);
    }

    // Summon Druids, one druid per player engaged in combat (actual summon handled in child spell 24795)
    bool DoSpecialDragonAbility()
    {
        if (DoCastSpellIfCan(m_creature, SPELL_SUMMON_DRUIDS, CAST_TRIGGERED) == CAST_OK)
        {
            DoScriptText(SAY_SUMMON_DRUIDS, m_creature);
            return true;
        }

        return false;
    }

    bool UpdateDragonAI(const uint32 uiDiff)
    {
        // Lightning Wave Timer
        if (m_uiLightningWaveTimer < uiDiff)
        {
            Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0);
            if (pTarget && DoCastSpellIfCan(pTarget, SPELL_LIGHTNING_WAVE) == CAST_OK)
                m_uiLightningWaveTimer = urand(7000, 12000);
        }
        else
            m_uiLightningWaveTimer -= uiDiff;

        return true;
    }
};

UnitAI* GetAI_boss_ysondre(Creature* pCreature)
{
    return new boss_ysondreAI(pCreature);
}

void AddSC_bosses_emerald_dragons()
{
    Script* pNewScript = new Script;
    pNewScript->Name = "boss_emeriss";
    pNewScript->GetAI = &GetAI_boss_emeriss;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "boss_lethon";
    pNewScript->GetAI = &GetAI_boss_lethon;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_spirit_shade";
    pNewScript->GetAI = &GetAI_npc_spirit_shade;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "boss_taerar";
    pNewScript->GetAI = &GetAI_boss_taerar;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "boss_ysondre";
    pNewScript->GetAI = &GetAI_boss_ysondre;
    pNewScript->RegisterSelf();
}
