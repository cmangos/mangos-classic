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
SDName: Boss_Ouro
SD%Complete: 90
SDComment: Some minor adjustments may be required
SDCategory: Temple of Ahn'Qiraj
EndScriptData */

#include "precompiled.h"
#include "temple_of_ahnqiraj.h"

enum
{
    // ground spells
    SPELL_SWEEP             = 26103,
    SPELL_SANDBLAST         = 26102,
    SPELL_BOULDER           = 26616,
    SPELL_BERSERK           = 26615,

    // emerge spells
    SPELL_BIRTH             = 26262,                        // The Birth Animation
    SPELL_GROUND_RUPTURE    = 26100,                        // spell not confirmed
    SPELL_SUMMON_BASE       = 26133,                        // summons gameobject 180795

    // submerge spells
    SPELL_SUBMERGE_VISUAL   = 26063,
    SPELL_SUMMON_OURO_MOUNDS = 26058,                       // summons 5 dirt mounds
    SPELL_SUMMON_TRIGGER    = 26284,

    // SPELL_SUMMON_OURO_TRIGG = 26642,
    SPELL_SUMMON_OURO       = 26061,                        // used by the script to summon the boss directly
    //SPELL_QUAKE             = 26093,

    // other spells - not used
    // SPELL_SUMMON_SCARABS    = 26060,                     // triggered after 30 secs - cast by the Dirt Mounds
    SPELL_DIRTMOUND_PASSIVE = 26092,                        // casts 26093 every 1 sec
    // SPELL_SET_OURO_HEALTH   = 26075,                     // removed from DBC
    // SPELL_SAVE_OURO_HEALTH  = 26076,                     // removed from DBC
    // SPELL_TELEPORT_TRIGGER  = 26285,                     // removed from DBC
    // SPELL_SUBMERGE_TRIGGER  = 26104,                     // removed from DBC
    SPELL_SUMMON_OURO_MOUND = 26617,
    // SPELL_SCARABS_PERIODIC  = 26619,                     // cast by the Dirt Mounds in order to spawn the scarabs - removed from DBC

    // summoned npcs
    NPC_OURO                = 15517,
    // NPC_OURO_SCARAB       = 15718,                       // summoned by Dirt Mounds
    NPC_OURO_TRIGGER        = 15717,
    NPC_DIRT_MOUND          = 15712,
};

struct boss_ouroAI : public Scripted_NoMovementAI
{
    boss_ouroAI(Creature* pCreature) : Scripted_NoMovementAI(pCreature)
    {
        m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
        Reset();
    }

    ScriptedInstance* m_pInstance;

    uint32 m_uiSweepTimer;
    uint32 m_uiSandBlastTimer;
    uint32 m_uiSubmergeTimer;
    uint32 m_uiSummonBaseTimer;

    uint32 m_uiSummonMoundTimer;

    bool m_bEnraged;
    bool m_bSubmerged;

    ObjectGuid m_ouroTriggerGuid;

    void Reset() override
    {
        m_uiSweepTimer        = urand(35000, 40000);
        m_uiSandBlastTimer    = urand(30000, 45000);
        m_uiSubmergeTimer     = 90000;
        m_uiSummonBaseTimer   = 2000;

        m_uiSummonMoundTimer  = 10000;

        m_bEnraged            = false;
        m_bSubmerged          = false;
    }

    void Aggro(Unit* /*pWho*/) override
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_OURO, IN_PROGRESS);
    }

    void JustReachedHome() override
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_OURO, FAIL);

        m_creature->ForcedDespawn();
    }

    void JustDied(Unit* /*pKiller*/) override
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_OURO, DONE);
    }

    void JustSummoned(Creature* pSummoned) override
    {
        switch (pSummoned->GetEntry())
        {
            case NPC_OURO_TRIGGER:
                m_ouroTriggerGuid = pSummoned->GetObjectGuid();
                // no break;
            case NPC_DIRT_MOUND:
                pSummoned->GetMotionMaster()->MoveRandomAroundPoint(pSummoned->GetPositionX(), pSummoned->GetPositionY(), pSummoned->GetPositionZ(), 40.0f);
                break;
        }
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        // Return since we have no pTarget
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        if (!m_bSubmerged)
        {
            // Summon sandworm base
            if (m_uiSummonBaseTimer)
            {
                if (m_uiSummonBaseTimer <= uiDiff)
                {
                    // Note: server side spells should be cast directly
                    m_creature->CastSpell(m_creature, SPELL_SUMMON_BASE, true);
                    m_uiSummonBaseTimer = 0;
                }
                else
                    m_uiSummonBaseTimer -= uiDiff;
            }

            // Sweep
            if (m_uiSweepTimer < uiDiff)
            {
                if (DoCastSpellIfCan(m_creature, SPELL_SWEEP) == CAST_OK)
                    m_uiSweepTimer = 20000;
            }
            else
                m_uiSweepTimer -= uiDiff;

            // Sand Blast
            if (m_uiSandBlastTimer < uiDiff)
            {
                if (DoCastSpellIfCan(m_creature, SPELL_SANDBLAST) == CAST_OK)
                    m_uiSandBlastTimer = 22000;
            }
            else
                m_uiSandBlastTimer -= uiDiff;

            if (!m_bEnraged)
            {
                // Enrage at 20% HP
                if (m_creature->GetHealthPercent() < 20.0f)
                {
                    if (DoCastSpellIfCan(m_creature, SPELL_BERSERK) == CAST_OK)
                    {
                        m_bEnraged = true;
                        return;
                    }
                }

                // Submerge
                if (m_uiSubmergeTimer < uiDiff)
                {
                    if (DoCastSpellIfCan(m_creature, SPELL_SUBMERGE_VISUAL) == CAST_OK)
                    {
                        DoCastSpellIfCan(m_creature, SPELL_SUMMON_OURO_MOUNDS, CAST_TRIGGERED);
                        DoCastSpellIfCan(m_creature, SPELL_SUMMON_TRIGGER, CAST_TRIGGERED);

                        m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);

                        m_bSubmerged      = true;
                        m_uiSubmergeTimer = 30000;
                    }
                }
                else
                    m_uiSubmergeTimer -= uiDiff;
            }
            else
            {
                // Summon 1 mound every 10 secs when enraged
                if (m_uiSummonMoundTimer < uiDiff)
                {
                    if (DoCastSpellIfCan(m_creature, SPELL_SUMMON_OURO_MOUND) == CAST_OK)
                        m_uiSummonMoundTimer = 10000;
                }
                else
                    m_uiSummonMoundTimer -= uiDiff;
            }

            // If we are within range melee the target
            if (m_creature->CanReachWithMeleeAttack(m_creature->getVictim()))
                DoMeleeAttackIfReady();
            // Spam Boulder spell when enraged and not tanked
            else if (m_bEnraged)
            {
                if (!m_creature->IsNonMeleeSpellCasted(false))
                {
                    if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
                        DoCastSpellIfCan(pTarget, SPELL_BOULDER);
                }
            }
        }
        else
        {
            // Resume combat
            if (m_uiSubmergeTimer < uiDiff)
            {
                // Teleport to the trigger in order to get a new location
                if (Creature* pTrigger = m_creature->GetMap()->GetCreature(m_ouroTriggerGuid))
                    m_creature->NearTeleportTo(pTrigger->GetPositionX(), pTrigger->GetPositionY(), pTrigger->GetPositionZ(), 0);

                if (DoCastSpellIfCan(m_creature, SPELL_BIRTH) == CAST_OK)
                {
                    m_creature->RemoveAurasDueToSpell(SPELL_SUBMERGE_VISUAL);
                    m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);

                    m_bSubmerged        = false;
                    m_uiSummonBaseTimer = 2000;
                    m_uiSubmergeTimer   = 90000;
                }
            }
            else
                m_uiSubmergeTimer -= uiDiff;
        }
    }
};

CreatureAI* GetAI_boss_ouro(Creature* pCreature)
{
    return new boss_ouroAI(pCreature);
}

struct npc_ouro_spawnerAI : public Scripted_NoMovementAI
{
    npc_ouro_spawnerAI(Creature* pCreature) : Scripted_NoMovementAI(pCreature) {Reset();}

    bool m_bHasSummoned;

    void Reset() override
    {
        m_bHasSummoned = false;

        DoCastSpellIfCan(m_creature, SPELL_DIRTMOUND_PASSIVE);
    }

    void Aggro(Unit* /*pWho*/) override
    {
        if (!m_bHasSummoned)
        {
            DoCastSpellIfCan(m_creature, SPELL_SUMMON_OURO, CAST_TRIGGERED);
            m_bHasSummoned = true;
        }
    }

    void JustSummoned(Creature* pSummoned) override
    {
        // Despanw when Ouro is spawned
        if (pSummoned->GetEntry() == NPC_OURO)
        {
            pSummoned->CastSpell(pSummoned, SPELL_BIRTH, false);
            pSummoned->SetInCombatWithZone();
            m_creature->ForcedDespawn();
        }
    }

    void UpdateAI(const uint32 /*uiDiff*/) override { }
};

CreatureAI* GetAI_npc_ouro_spawner(Creature* pCreature)
{
    return new npc_ouro_spawnerAI(pCreature);
}

void AddSC_boss_ouro()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "boss_ouro";
    pNewScript->GetAI = &GetAI_boss_ouro;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_ouro_spawner";
    pNewScript->GetAI = &GetAI_npc_ouro_spawner;
    pNewScript->RegisterSelf();
}
