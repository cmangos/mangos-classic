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
SDName: Boss_Ayamiss
SD%Complete: 95
SDComment: Swarmers need movement added for RP before they attack
SDCategory: Ruins of Ahn'Qiraj
EndScriptData

*/

#include "AI/ScriptDevAI/include/precompiled.h"
#include "ruins_of_ahnqiraj.h"
#include "AI/ScriptDevAI/base/CombatAI.h"

enum
{
    EMOTE_GENERIC_FRENZY    = -1000002,

    SPELL_STINGER_SPRAY     = 25749,
    SPELL_POISON_STINGER    = 25748,                // only used in phase1
    SPELL_SUMMON_SWARMER    = 25708,
    SPELL_PARALYZE          = 25725,
    SPELL_LASH              = 25852,
    SPELL_FRENZY            = 8269,
    SPELL_THRASH            = 3391,

    SPELL_FEED              = 25721,                // cast by the Larva when reaches the player on the altar

    SPELL_SWARMER_TELEPORT_1 = 25709,
    SPELL_SWARMER_TELEPORT_2 = 25825,
    SPELL_SWARMER_TELEPORT_3 = 25826,
    SPELL_SWARMER_TELEPORT_4 = 25827,
    SPELL_SWARMER_TELEPORT_5 = 25828,

    SPELL_SUMMON_LARVA_1     = 26538,
    SPELL_SUMMON_LARVA_2     = 26539,

    NPC_LARVA               = 15555,
    NPC_SWARMER             = 15546,
    NPC_HORNET              = 15934,

    PHASE_AIR               = 0,
    PHASE_GROUND            = 1,

    POINT_AIR               = 1,
};

struct SummonLocation
{
    float m_fX, m_fY, m_fZ;
};

// Spawn locations
static const SummonLocation aAyamissSpawnLocs[] =
{
    { -9647.352f, 1578.062f, 55.32f},           // anchor point for swarmers
    { -9717.18f, 1517.72f, 27.4677f},           // teleport location - need to be hardcoded because the player is teleported after the larva is summoned
};

enum AyamissActions
{
    AYAMISS_PHASE_2,
    AYAMISS_FRENZY,
    AYAMISS_PARALYZE,
    AYAMISS_STINGER_SPRAY,
    AYAMISS_POISON_STINGER,
    AYAMISS_SWARMER_ATTACK,
    AYAMISS_SUMMON_SWARMER,
    AYAMISS_LASH,
    AYAMISS_THRASH,
    AYAMISS_ACTION_MAX,
};

struct boss_ayamissAI : public CombatAI
{
    boss_ayamissAI(Creature* creature) : CombatAI(creature, AYAMISS_ACTION_MAX)
    {
        AddTimerlessCombatAction(AYAMISS_PHASE_2, true);
        AddTimerlessCombatAction(AYAMISS_FRENZY, true);
        AddCombatAction(AYAMISS_PARALYZE, 15000u);
        AddCombatAction(AYAMISS_STINGER_SPRAY, 20000, 30000);
        AddCombatAction(AYAMISS_POISON_STINGER, 5000u);
        AddCombatAction(AYAMISS_SUMMON_SWARMER, 5000u);
        AddCombatAction(AYAMISS_SWARMER_ATTACK, 60000u);
        AddCombatAction(AYAMISS_LASH, true);
        AddCombatAction(AYAMISS_THRASH, true);
    }

    uint8 m_phase;

    ObjectGuid m_paralyzeTarget;
    GuidList m_swarmersGuidList;

    void Reset() override
    {
        CombatAI::Reset();
        m_phase = PHASE_AIR;
        SetCombatMovement(false);
        SetMeleeEnabled(false);
        SetCombatScriptStatus(false);
    }

    void Aggro(Unit* /*who*/) override
    {
        m_creature->SetLevitate(true);
        m_creature->SetHover(true);
        SetCombatScriptStatus(true);
        m_creature->GetMotionMaster()->MovePoint(POINT_AIR, -9689.292f, 1547.912f, 48.02729f);
    }

    void JustSummoned(Creature* summoned) override
    {
        // store the swarmers for a future attack
        if (summoned->GetEntry() == NPC_SWARMER)
        {
            m_swarmersGuidList.push_back(summoned->GetObjectGuid());
            uint32 spellId = 0;
            switch (urand(0, 4))
            {
                case 0: spellId = SPELL_SWARMER_TELEPORT_1; break;
                case 1: spellId = SPELL_SWARMER_TELEPORT_2; break;
                case 2: spellId = SPELL_SWARMER_TELEPORT_3; break;
                case 3: spellId = SPELL_SWARMER_TELEPORT_4; break;
                case 4: spellId = SPELL_SWARMER_TELEPORT_5; break;
                default: break;
            }
            summoned->CastSpell(nullptr, spellId, TRIGGERED_OLD_TRIGGERED);
        }
        // move the larva to paralyze target position
        else if (summoned->GetEntry() == NPC_LARVA)
        {
            summoned->SetWalk(false);
            summoned->GetMotionMaster()->MovePoint(1, aAyamissSpawnLocs[3].m_fX, aAyamissSpawnLocs[3].m_fY, aAyamissSpawnLocs[3].m_fZ);
        }
        else if (summoned->GetEntry() == NPC_HORNET)
        {
            if (Unit* target = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
                summoned->AI()->AttackStart(target);
        }
    }

    void MovementInform(uint32 motionType, uint32 pointId) override
    {
        if (motionType != POINT_MOTION_TYPE)
            return;

        if (pointId == POINT_AIR)
        {
            SetCombatScriptStatus(false);
            m_creature->SetImmobilizedState(true);
        }
    }

    void SummonedMovementInform(Creature* summoned, uint32 /*motionType*/, uint32 pointId) override
    {
        if (pointId != 1 || summoned->GetEntry() != NPC_LARVA)
            return;

        // Cast feed on target
        if (Unit* target = m_creature->GetMap()->GetUnit(m_paralyzeTarget))
            summoned->CastSpell(target, SPELL_FEED, TRIGGERED_OLD_TRIGGERED, nullptr, nullptr, m_creature->GetObjectGuid());
    }

    void ExecuteAction(uint32 action) override
    {
        switch (action)
        {
            case AYAMISS_PHASE_2:
            {
                if (m_creature->GetHealthPercent() <= 70.0f) // TODO: Recheck flying
                {
                    m_phase = PHASE_GROUND;
                    SetMeleeEnabled(true);
                    m_creature->SetLevitate(false);
                    m_creature->SetHover(false);
                    DoResetThreat();
                    SetCombatMovement(true, true);

                    ResetCombatAction(AYAMISS_LASH, urand(5000, 8000));
                    ResetCombatAction(AYAMISS_THRASH, urand(3000, 6000));
                }
                break;
            }
            case AYAMISS_FRENZY:
            {
                if (m_creature->GetHealthPercent() < 20.0f)
                {
                    if (DoCastSpellIfCan(nullptr, SPELL_FRENZY) == CAST_OK)
                    {
                        DoScriptText(EMOTE_GENERIC_FRENZY, m_creature);
                        SetActionReadyStatus(action, false);
                    }
                }
                break;
            }
            case AYAMISS_PARALYZE:
            {
                Unit* target = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 1, SPELL_PARALYZE, SELECT_FLAG_PLAYER);
                if (!target)
                    target = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0, SPELL_PARALYZE, SELECT_FLAG_PLAYER);

                if (DoCastSpellIfCan(target, SPELL_PARALYZE) == CAST_OK)
                {
                    m_paralyzeTarget = target->GetObjectGuid();
                    ResetCombatAction(action, 15000);

                    // Summon a larva
                    uint32 spellId = urand(0, 1) ? SPELL_SUMMON_LARVA_1 : SPELL_SUMMON_LARVA_2;
                    m_creature->CastSpell(nullptr, spellId, TRIGGERED_OLD_TRIGGERED);
                }
                break;
            }
            case AYAMISS_STINGER_SPRAY:
            {
                if (DoCastSpellIfCan(nullptr, SPELL_STINGER_SPRAY) == CAST_OK)
                    ResetCombatAction(action, urand(15000, 20000));
                break;
            }
            case AYAMISS_POISON_STINGER:
            {
                if (Unit* target = m_creature->SelectAttackingTarget(ATTACKING_TARGET_TOPAGGRO, 0, SPELL_POISON_STINGER, SELECT_FLAG_PLAYER))
                    if (DoCastSpellIfCan(target, SPELL_POISON_STINGER) == CAST_OK)
                        ResetCombatAction(action, urand(2000, 3000));
                break;
            }
            case AYAMISS_SUMMON_SWARMER:
            {
                if (DoCastSpellIfCan(nullptr, SPELL_SUMMON_SWARMER) == CAST_OK)
                    ResetCombatAction(action, 5000);
                break;
            }
            case AYAMISS_SWARMER_ATTACK:
            {
                for (auto guid : m_swarmersGuidList)
                {
                    if (Creature* temp = m_creature->GetMap()->GetCreature(guid))
                    {
                        if (Unit* target = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0, nullptr, SELECT_FLAG_PLAYER))
                            temp->AI()->AttackStart(target);
                    }
                }
                m_swarmersGuidList.clear();
                ResetCombatAction(action, 60000);
                break;
            }
            case AYAMISS_LASH:
            {
                if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_LASH) == CAST_OK)
                    ResetCombatAction(action, urand(8000, 15000));
                break;
            }
            case AYAMISS_THRASH:
            {
                if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_THRASH) == CAST_OK)
                    ResetCombatAction(action, urand(5000, 7000));
                break;
            }
        }
    }
};

UnitAI* GetAI_boss_ayamiss(Creature* creature)
{
    return new boss_ayamissAI(creature);
}

struct npc_hive_zara_larvaAI : public ScriptedAI
{
    npc_hive_zara_larvaAI(Creature* creature) : ScriptedAI(creature), m_instance(static_cast<instance_ruins_of_ahnqiraj*>(m_creature->GetInstanceData()))
    {

    }

    instance_ruins_of_ahnqiraj* m_instance;

    void Reset() override { }

    void AttackStart(Unit* who) override
    {
        // don't attack anything during the Ayamiss encounter
        if (m_instance)
        {
            if (m_instance->GetData(TYPE_AYAMISS) == IN_PROGRESS)
                return;
        }

        ScriptedAI::AttackStart(who);
    }

    void MoveInLineOfSight(Unit* who) override
    {
        // don't attack anything during the Ayamiss encounter
        if (m_instance)
        {
            if (m_instance->GetData(TYPE_AYAMISS) == IN_PROGRESS)
                return;
        }

        ScriptedAI::MoveInLineOfSight(who);
    }

    void UpdateAI(const uint32 /*uiDiff*/) override
    {
        if (m_instance)
        {
            if (m_instance->GetData(TYPE_AYAMISS) == IN_PROGRESS)
                return;
        }

        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        DoMeleeAttackIfReady();
    }
};

UnitAI* GetAI_npc_hive_zara_larva(Creature* creature)
{
    return new npc_hive_zara_larvaAI(creature);
}

void AddSC_boss_ayamiss()
{
    Script* pNewScript = new Script;
    pNewScript->Name = "boss_ayamiss";
    pNewScript->GetAI = &GetAI_boss_ayamiss;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_hive_zara_larva";
    pNewScript->GetAI = &GetAI_npc_hive_zara_larva;
    pNewScript->RegisterSelf();
}
