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
SDName: Boss_Fankriss
SD%Complete: 100
SDComment: sound not implemented
SDCategory: Temple of Ahn'Qiraj
EndScriptData */

#include "precompiled.h"
#include "temple_of_ahnqiraj.h"

enum
{
    SOUND_SENTENCE_YOU      = 8588,
    SOUND_SERVE_TO          = 8589,
    SOUND_LAWS              = 8590,
    SOUND_TRESPASS          = 8591,
    SOUND_WILL_BE           = 8592,

    SPELL_MORTAL_WOUND      = 25646,
    SPELL_ENTANGLE_1        = 720,
    SPELL_ENTANGLE_2        = 731,
    SPELL_ENTANGLE_3        = 1121,
    SPELL_SUMMON_WORM_1     = 518,
    SPELL_SUMMON_WORM_2     = 25831,
    SPELL_SUMMON_WORM_3     = 25832,

    NPC_SPAWN_FANKRISS      = 15630,
    NPC_VEKNISS_HATCHLING   = 15962,
};

static const uint32 aSummonWormSpells[3] = {SPELL_SUMMON_WORM_1, SPELL_SUMMON_WORM_2, SPELL_SUMMON_WORM_3};
static const uint32 aEntangleSpells[3] = {SPELL_ENTANGLE_1, SPELL_ENTANGLE_2, SPELL_ENTANGLE_3};

struct boss_fankrissAI : public ScriptedAI
{
    boss_fankrissAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
        Reset();
    }

    ScriptedInstance* m_pInstance;

    uint32 m_uiMortalWoundTimer;
    uint32 m_uiSummonWormTimer;
    uint32 m_uiEntangleTimer;
    uint32 m_uiEntangleSummonTimer;

    ObjectGuid m_EntangleTargetGuid;

    void Reset() override
    {
        m_uiMortalWoundTimer = urand(10000, 15000);
        m_uiSummonWormTimer  = urand(30000, 50000);
        m_uiEntangleTimer    = urand(25000, 40000);
        m_uiEntangleSummonTimer = 0;
    }

    void Aggro(Unit* /*pWho*/) override
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_FANKRISS, IN_PROGRESS);
    }

    void JustReachedHome() override
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_FANKRISS, FAIL);
    }

    void JustDied(Unit* /*pKiller*/) override
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_FANKRISS, DONE);
    }

    void JustSummoned(Creature* pSummoned) override
    {
        if (pSummoned->GetEntry() == NPC_SPAWN_FANKRISS)
        {
            if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
                pSummoned->AI()->AttackStart(pTarget);
        }
        else if (pSummoned->GetEntry() == NPC_VEKNISS_HATCHLING)
        {
            if (Player* pTarget = m_creature->GetMap()->GetPlayer(m_EntangleTargetGuid))
                pSummoned->AI()->AttackStart(pTarget);
        }
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        if (m_uiMortalWoundTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_MORTAL_WOUND) == CAST_OK)
                m_uiMortalWoundTimer = urand(7000, 14000);
        }
        else
            m_uiMortalWoundTimer -= uiDiff;

        if (m_uiSummonWormTimer < uiDiff)
        {
            uint8 uiSpawnIndex = urand(0, 2);
            if (DoCastSpellIfCan(m_creature, aSummonWormSpells[uiSpawnIndex]) == CAST_OK)
                m_uiSummonWormTimer = urand(15000, 40000);
        }
        else
            m_uiSummonWormTimer -= uiDiff;

        // Teleporting Random Target to one of the three tunnels and spawn 4 hatchlings near the gamer.
        if (m_uiEntangleTimer < uiDiff)
        {
            if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0, uint32(0), SELECT_FLAG_PLAYER))
            {
                uint8 uiEntangleIndex = urand(0, 2);
                if (DoCastSpellIfCan(pTarget, aEntangleSpells[uiEntangleIndex]) == CAST_OK)
                {
                    m_EntangleTargetGuid = pTarget->GetObjectGuid();
                    m_uiEntangleSummonTimer = 1000;
                    m_uiEntangleTimer = urand(40000, 70000);
                }
            }
        }
        else
            m_uiEntangleTimer -= uiDiff;

        // Summon 4 Hatchlings around the target
        if (m_uiEntangleSummonTimer)
        {
            if (m_uiEntangleSummonTimer <= uiDiff)
            {
                if (Player* pTarget = m_creature->GetMap()->GetPlayer(m_EntangleTargetGuid))
                {
                    float fX, fY, fZ;
                    for (uint8 i = 0; i < 4; ++i)
                    {
                        m_creature->GetRandomPoint(pTarget->GetPositionX(), pTarget->GetPositionY(), pTarget->GetPositionZ(), 3.0f, fX, fY, fZ);
                        m_creature->SummonCreature(NPC_VEKNISS_HATCHLING, fX, fY, fZ, 0.0f, TEMPSUMMON_TIMED_OOC_DESPAWN, 10000);
                    }
                    m_uiEntangleSummonTimer = 0;
                }
            }
            else
                m_uiEntangleSummonTimer -= uiDiff;
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_fankriss(Creature* pCreature)
{
    return new boss_fankrissAI(pCreature);
}

void AddSC_boss_fankriss()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "boss_fankriss";
    pNewScript->GetAI = &GetAI_boss_fankriss;
    pNewScript->RegisterSelf();
}
