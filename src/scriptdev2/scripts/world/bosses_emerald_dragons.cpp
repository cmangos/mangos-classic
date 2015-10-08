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
SD%Complete: 95
SDComment: Missing correct behaviour of used trigger NPCs, some spell issues, summon player NYI
SDCategory: Emerald Dragon Bosses
EndScriptData */

/* ContentData
boss_emerald_dragon -- Superclass for the four dragons
boss_emeriss
boss_lethon
npc_spirit_shade
boss_taerar
boss_ysondre
EndContentData */

#include "precompiled.h"

/*######
## boss_emerald_dragon -- Superclass for the four dragons
######*/

enum
{
    SPELL_MARK_OF_NATURE_PLAYER     = 25040,
    SPELL_MARK_OF_NATURE_AURA       = 25041,
    SPELL_SEEPING_FOG_R             = 24813,                // Summons 15224 'Dream Fog'
    SPELL_SEEPING_FOG_L             = 24814,
    SPELL_DREAM_FOG                 = 24777,                // Used by summoned Adds
    SPELL_NOXIOUS_BREATH            = 24818,
    SPELL_TAILSWEEP                 = 15847,
    SPELL_SUMMON_PLAYER             = 24776,                // NYI

    NPC_DREAM_FOG                   = 15224,
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
            pVictim->CastSpell(pVictim, SPELL_MARK_OF_NATURE_PLAYER, true, NULL, NULL, m_creature->GetObjectGuid());
    }

    void JustSummoned(Creature* pSummoned) override
    {
        if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
            pSummoned->AI()->AttackStart(pTarget);

        if (pSummoned->GetEntry() == NPC_DREAM_FOG)
            pSummoned->CastSpell(pSummoned, SPELL_DREAM_FOG, true, NULL, NULL, m_creature->GetObjectGuid());
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
                m_uiTailsweepTimer = 2000;
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
    SPELL_PUTRID_MUSHROOM       = 24904,                    // Summons a mushroom on killing a player
};

struct boss_emerissAI : public boss_emerald_dragonAI
{
    boss_emerissAI(Creature* pCreature) : boss_emerald_dragonAI(pCreature) { Reset(); }

    uint32 m_uiVolatileInfectionTimer;

    void Reset() override
    {
        boss_emerald_dragonAI::Reset();

        m_uiVolatileInfectionTimer = 12000;
    }

    void Aggro(Unit* /*pWho*/) override
    {
        DoScriptText(SAY_EMERISS_AGGRO, m_creature);
    }

    void KilledUnit(Unit* pVictim) override
    {
        // summon a mushroom on the spot the player dies
        if (pVictim->GetTypeId() == TYPEID_PLAYER)
            pVictim->CastSpell(pVictim, SPELL_PUTRID_MUSHROOM, true, NULL, NULL, m_creature->GetObjectGuid());

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

CreatureAI* GetAI_boss_emeriss(Creature* pCreature)
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
    SPELL_SUMMON_SPIRIT_SHADE   = 24810,                    // Summon spell was removed, was SPELL_EFFECT_SUMMON_DEMON

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
        // Shadow bolt wirl is a periodic aura which triggers a set of shadowbolts every 2 secs; may need some core tunning
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

    // Need this code here, as SPELL_DRAW_SPIRIT has no Script- or Dummyeffect
    void SpellHitTarget(Unit* pTarget, const SpellEntry* pSpell) override
    {
        // Summon a shade for each player hit
        if (pTarget->GetTypeId() == TYPEID_PLAYER && pSpell->Id == SPELL_DRAW_SPIRIT)
        {
            // Summon this way, to be able to cast the shade visual spell with player as original caster
            // This might not be supported currently by core, but this spell's visual should be dependend on the player
            // Also possible that this was no problem due to the special way these NPCs had been summoned in classic times
            if (Creature* pSummoned = pTarget->SummonCreature(NPC_SPIRIT_SHADE, 0.0f, 0.0f, 0.0f, pTarget->GetOrientation(), TEMPSUMMON_DEAD_DESPAWN, 0))
                pSummoned->CastSpell(pSummoned, SPELL_SPIRIT_SHAPE_VISUAL, true, NULL, NULL, pTarget->GetObjectGuid());
        }
    }

    void JustSummoned(Creature* pSummoned) override
    {
        // Move the shade to lethon
        if (pSummoned->GetEntry() == NPC_SPIRIT_SHADE)
            pSummoned->GetMotionMaster()->MoveFollow(m_creature, 0.0f, 0.0f);
        else
            boss_emerald_dragonAI::JustSummoned(pSummoned);
    }
};

struct npc_spirit_shadeAI : public ScriptedAI
{
    npc_spirit_shadeAI(Creature* pCreature) : ScriptedAI(pCreature) { Reset(); }

    bool m_bHasHealed;

    void Reset() override
    {
        m_bHasHealed = false;
    }

    void MoveInLineOfSight(Unit* pWho) override
    {
        if (!m_bHasHealed && pWho->GetEntry() == NPC_LETHON && pWho->IsWithinDistInMap(m_creature, 3.0f))
        {
            if (DoCastSpellIfCan(pWho, SPELL_DARK_OFFERING) == CAST_OK)
            {
                m_bHasHealed = true;
                m_creature->ForcedDespawn(1000);
            }
        }
    }

    void AttackStart(Unit* /*pWho*/) override { }

    void UpdateAI(const uint32 /*uiDiff*/) override { }
};

CreatureAI* GetAI_boss_lethon(Creature* pCreature)
{
    return new boss_lethonAI(pCreature);
}

CreatureAI* GetAI_npc_spirit_shade(Creature* pCreature)
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

    SPELL_SUMMON_SHADE_1    = 24841,
    SPELL_SUMMON_SHADE_2    = 24842,
    SPELL_SUMMON_SHADE_3    = 24843,
    SPELL_SELF_STUN         = 24883,                        // Stunns the main boss until the shades are dead or timer expires

    NPC_SHADE_OF_TAERAR     = 15302,
    SPELL_POSIONCLOUD       = 24840,
    SPELL_POSIONBREATH      = 20667
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

CreatureAI* GetAI_boss_taerar(Creature* pCreature)
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
    SPELL_SUMMON_DRUIDS     = 24795,

    // druid spells
    SPELL_MOONFIRE          = 21669
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

    // Summon Druids - TODO FIXME (spell not understood)
    bool DoSpecialDragonAbility()
    {
        DoScriptText(SAY_SUMMON_DRUIDS, m_creature);

        for (int i = 0; i < 10; ++i)
            DoCastSpellIfCan(m_creature, SPELL_SUMMON_DRUIDS, CAST_TRIGGERED);

        return true;
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

CreatureAI* GetAI_boss_ysondre(Creature* pCreature)
{
    return new boss_ysondreAI(pCreature);
}

void AddSC_bosses_emerald_dragons()
{
    Script* pNewScript;

    pNewScript = new Script;
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
