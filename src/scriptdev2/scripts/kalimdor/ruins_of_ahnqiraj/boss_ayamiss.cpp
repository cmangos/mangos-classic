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
SD%Complete: 80
SDComment: Timers and summon coords need adjustments
SDCategory: Ruins of Ahn'Qiraj
EndScriptData */

#include "precompiled.h"
#include "ruins_of_ahnqiraj.h"

enum
{
    EMOTE_GENERIC_FRENZY    = -1000002,

    SPELL_STINGER_SPRAY     = 25749,
    SPELL_POISON_STINGER    = 25748,                // only used in phase1
    // SPELL_SUMMON_SWARMER  = 25844,                // might be 25708    - spells were removed since 2.0.1
    SPELL_PARALYZE          = 25725,
    SPELL_LASH              = 25852,
    SPELL_FRENZY            = 8269,
    SPELL_TRASH             = 3391,

    SPELL_FEED              = 25721,                // cast by the Larva when reaches the player on the altar

    NPC_LARVA               = 15555,
    NPC_SWARMER             = 15546,
    NPC_HORNET              = 15934,

    PHASE_AIR               = 0,
    PHASE_GROUND            = 1
};

struct SummonLocation
{
    float m_fX, m_fY, m_fZ;
};

// Spawn locations
static const SummonLocation aAyamissSpawnLocs[] =
{
    { -9674.4707f, 1528.4133f, 22.457f},        // larva
    { -9701.6005f, 1566.9993f, 24.118f},        // larva
    { -9647.352f, 1578.062f, 55.32f},           // anchor point for swarmers
    { -9717.18f, 1517.72f, 27.4677f},           // teleport location - need to be hardcoded because the player is teleported after the larva is summoned
};

struct boss_ayamissAI : public ScriptedAI
{
    boss_ayamissAI(Creature* pCreature) : ScriptedAI(pCreature) {Reset();}

    uint32 m_uiStingerSprayTimer;
    uint32 m_uiPoisonStingerTimer;
    uint32 m_uiSummonSwarmerTimer;
    uint32 m_uiSwarmerAttackTimer;
    uint32 m_uiParalyzeTimer;
    uint32 m_uiLashTimer;
    uint32 m_uiTrashTimer;
    uint8 m_uiPhase;

    bool m_bHasFrenzy;

    ObjectGuid m_paralyzeTarget;
    GuidList m_lSwarmersGuidList;

    void Reset() override
    {
        m_uiStingerSprayTimer   = urand(20000, 30000);
        m_uiPoisonStingerTimer  = 5000;
        m_uiSummonSwarmerTimer  = 5000;
        m_uiSwarmerAttackTimer  = 60000;
        m_uiParalyzeTimer       = 15000;
        m_uiLashTimer           = urand(5000, 8000);
        m_uiTrashTimer          = urand(3000, 6000);

        m_bHasFrenzy            = false;

        m_uiPhase = PHASE_AIR;
        SetCombatMovement(false);
    }

    void Aggro(Unit* /*pWho*/) override
    {
        m_creature->SetLevitate(true);
        m_creature->GetMotionMaster()->MovePoint(0, m_creature->GetPositionX(), m_creature->GetPositionY(), m_creature->GetPositionZ() + 15.0f);
    }

    void JustSummoned(Creature* pSummoned) override
    {
        // store the swarmers for a future attack
        if (pSummoned->GetEntry() == NPC_SWARMER)
            m_lSwarmersGuidList.push_back(pSummoned->GetObjectGuid());
        // move the larva to paralyze target position
        else if (pSummoned->GetEntry() == NPC_LARVA)
        {
            pSummoned->SetWalk(false);
            pSummoned->GetMotionMaster()->MovePoint(1, aAyamissSpawnLocs[3].m_fX, aAyamissSpawnLocs[3].m_fY, aAyamissSpawnLocs[3].m_fZ);
        }
        else if (pSummoned->GetEntry() == NPC_HORNET)
        {
            if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
                pSummoned->AI()->AttackStart(pTarget);
        }
    }

    void SummonedMovementInform(Creature* pSummoned, uint32 /*uiMotionType*/, uint32 uiPointId) override
    {
        if (uiPointId != 1 || pSummoned->GetEntry() != NPC_LARVA)
            return;

        // Cast feed on target
        if (Unit* pTarget = m_creature->GetMap()->GetUnit(m_paralyzeTarget))
            pSummoned->CastSpell(pTarget, SPELL_FEED, true, nullptr, nullptr, m_creature->GetObjectGuid());
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        if (!m_bHasFrenzy && m_creature->GetHealthPercent() < 20.0f)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_FRENZY) == CAST_OK)
            {
                DoScriptText(EMOTE_GENERIC_FRENZY, m_creature);
                m_bHasFrenzy = true;
            }
        }

        // Stinger Spray
        if (m_uiStingerSprayTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_STINGER_SPRAY) == CAST_OK)
                m_uiStingerSprayTimer = urand(15000, 20000);
        }
        else
            m_uiStingerSprayTimer -= uiDiff;

        // Paralyze
        if (m_uiParalyzeTimer < uiDiff)
        {
            Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 1, SPELL_PARALYZE, SELECT_FLAG_PLAYER);
            if (!pTarget)
                pTarget = m_creature->getVictim();

            if (DoCastSpellIfCan(pTarget, SPELL_PARALYZE) == CAST_OK)
            {
                m_paralyzeTarget = pTarget->GetObjectGuid();
                m_uiParalyzeTimer = 15000;

                // Summon a larva
                uint8 uiLoc = urand(0, 1);
                m_creature->SummonCreature(NPC_LARVA, aAyamissSpawnLocs[uiLoc].m_fX, aAyamissSpawnLocs[uiLoc].m_fY, aAyamissSpawnLocs[uiLoc].m_fZ, 0, TEMPSUMMON_TIMED_OOC_OR_CORPSE_DESPAWN, 30000);
            }
        }
        else
            m_uiParalyzeTimer -= uiDiff;

        // Summon Swarmer
        if (m_uiSummonSwarmerTimer < uiDiff)
        {
            // The spell which summons these guys was removed in 2.0.1 -> therefore we need to summon them manually at a random location around the area
            // The summon locations is guesswork - the real location is supposed to be handled by world triggers
            // There should be about 24 swarmers per min
            float fX, fY, fZ;
            for (uint8 i = 0; i < 2; ++i)
            {
                m_creature->GetRandomPoint(aAyamissSpawnLocs[2].m_fX, aAyamissSpawnLocs[2].m_fY, aAyamissSpawnLocs[2].m_fZ, 80.0f, fX, fY, fZ);
                m_creature->SummonCreature(NPC_SWARMER, fX, fY, aAyamissSpawnLocs[2].m_fZ, 0.0f, TEMPSUMMON_CORPSE_DESPAWN, 0);
            }
            m_uiSummonSwarmerTimer = 5000;
        }
        else
            m_uiSummonSwarmerTimer -= uiDiff;

        // All the swarmers attack at a certain period of time
        if (m_uiSwarmerAttackTimer < uiDiff)
        {
            for (GuidList::const_iterator itr = m_lSwarmersGuidList.begin(); itr != m_lSwarmersGuidList.end(); ++itr)
            {
                if (Creature* pTemp = m_creature->GetMap()->GetCreature(*itr))
                {
                    if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
                        pTemp->AI()->AttackStart(pTarget);
                }
            }
            m_lSwarmersGuidList.clear();
            m_uiSwarmerAttackTimer = 60000;
        }
        else
            m_uiSwarmerAttackTimer -= uiDiff;

        if (m_uiPhase == PHASE_AIR)
        {
            // Start ground phase at 70% of HP
            if (m_creature->GetHealthPercent() <= 70.0f)
            {
                m_uiPhase = PHASE_GROUND;
                SetCombatMovement(true);
                m_creature->SetLevitate(false);
                DoResetThreat();

                if (m_creature->getVictim())
                    m_creature->GetMotionMaster()->MoveChase(m_creature->getVictim());
            }

            // Poison Stinger
            if (m_uiPoisonStingerTimer < uiDiff)
            {
                if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_POISON_STINGER) == CAST_OK)
                    m_uiPoisonStingerTimer = urand(2000, 3000);
            }
            else
                m_uiPoisonStingerTimer -= uiDiff;
        }
        else
        {
            if (m_uiLashTimer < uiDiff)
            {
                if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_LASH) == CAST_OK)
                    m_uiLashTimer = urand(8000, 15000);
            }
            else
                m_uiLashTimer -= uiDiff;

            if (m_uiTrashTimer < uiDiff)
            {
                if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_TRASH) == CAST_OK)
                    m_uiTrashTimer = urand(5000, 7000);
            }
            else
                m_uiTrashTimer -= uiDiff;

            DoMeleeAttackIfReady();
        }
    }
};

CreatureAI* GetAI_boss_ayamiss(Creature* pCreature)
{
    return new boss_ayamissAI(pCreature);
}

struct npc_hive_zara_larvaAI : public ScriptedAI
{
    npc_hive_zara_larvaAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = (instance_ruins_of_ahnqiraj*)m_creature->GetInstanceData();
        Reset();
    }

    instance_ruins_of_ahnqiraj* m_pInstance;

    void Reset() override { }

    void AttackStart(Unit* pWho) override
    {
        // don't attack anything during the Ayamiss encounter
        if (m_pInstance)
        {
            if (m_pInstance->GetData(TYPE_AYAMISS) == IN_PROGRESS)
                return;
        }

        ScriptedAI::AttackStart(pWho);
    }

    void MoveInLineOfSight(Unit* pWho) override
    {
        // don't attack anything during the Ayamiss encounter
        if (m_pInstance)
        {
            if (m_pInstance->GetData(TYPE_AYAMISS) == IN_PROGRESS)
                return;
        }

        ScriptedAI::MoveInLineOfSight(pWho);
    }

    void UpdateAI(const uint32 /*uiDiff*/) override
    {
        if (m_pInstance)
        {
            if (m_pInstance->GetData(TYPE_AYAMISS) == IN_PROGRESS)
                return;
        }

        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_hive_zara_larva(Creature* pCreature)
{
    return new npc_hive_zara_larvaAI(pCreature);
}

void AddSC_boss_ayamiss()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "boss_ayamiss";
    pNewScript->GetAI = &GetAI_boss_ayamiss;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_hive_zara_larva";
    pNewScript->GetAI = &GetAI_npc_hive_zara_larva;
    pNewScript->RegisterSelf();
}
