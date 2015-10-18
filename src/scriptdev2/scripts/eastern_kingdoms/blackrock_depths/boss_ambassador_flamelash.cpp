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
SDName: Boss_Ambassador_Flamelash
SD%Complete: 90
SDComment: Texts probably missing; Spirits handling could be improved.
SDCategory: Blackrock Depths
EndScriptData */

#include "precompiled.h"
#include "blackrock_depths.h"

enum
{
    SPELL_FIREBLAST             = 15573,
    SPELL_BURNING_SPIRIT        = 13489,

    NPC_BURNING_SPIRIT          = 9178,
};

struct boss_ambassador_flamelashAI : public ScriptedAI
{
    boss_ambassador_flamelashAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
        Reset();
    }

    ScriptedInstance* m_pInstance;

    uint32 m_uiSpiritTimer[MAX_DWARF_RUNES];

    GuidSet m_sSpiritsGuidsSet;

    void Reset() override
    {
        for (uint8 i = 0; i < MAX_DWARF_RUNES; ++i)
            m_uiSpiritTimer[i] = urand(0, 1000);

        m_sSpiritsGuidsSet.clear();
    }

    // function that will summon spirits periodically
    void DoSummonSpirit(uint8 uiIndex)
    {
        if (!m_pInstance)
            return;

        if (GameObject* pRune = m_pInstance->GetSingleGameObjectFromStorage(GO_DWARFRUNE_A01 + uiIndex))
            m_creature->SummonCreature(NPC_BURNING_SPIRIT, pRune->GetPositionX(), pRune->GetPositionY(), pRune->GetPositionZ(), m_creature->GetAngle(m_creature), TEMPSUMMON_TIMED_OOC_OR_DEAD_DESPAWN, 60000);
    }

    void MoveInLineOfSight(Unit* pWho) override
    {
        ScriptedAI::MoveInLineOfSight(pWho);

        if (pWho->GetEntry() == NPC_BURNING_SPIRIT && pWho->isAlive() && m_sSpiritsGuidsSet.find(pWho->GetObjectGuid()) != m_sSpiritsGuidsSet.end() &&
                pWho->IsWithinDistInMap(m_creature, 2 * CONTACT_DISTANCE))
        {
            pWho->CastSpell(m_creature, SPELL_BURNING_SPIRIT, true);
            m_sSpiritsGuidsSet.erase(pWho->GetObjectGuid());
        }
    }

    void Aggro(Unit* /*pWho*/) override
    {
        DoCastSpellIfCan(m_creature, SPELL_FIREBLAST);

        if (m_pInstance)
            m_pInstance->SetData(TYPE_FLAMELASH, IN_PROGRESS);
    }

    void JustDied(Unit* /*pKiller*/) override
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_FLAMELASH, DONE);
    }

    void JustReachedHome() override
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_FLAMELASH, FAIL);
    }

    void JustSummoned(Creature* pSummoned) override
    {
        pSummoned->GetMotionMaster()->MoveFollow(m_creature, 0, 0);
        m_sSpiritsGuidsSet.insert(pSummoned->GetObjectGuid());
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        // Return since we have no target
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        // m_uiSpiritTimer
        for (uint8 i = 0; i < MAX_DWARF_RUNES; ++i)
        {
            if (m_uiSpiritTimer[i] < uiDiff)
            {
                DoSummonSpirit(i);
                m_uiSpiritTimer[i] = urand(15000, 30000);
            }
            else
                m_uiSpiritTimer[i] -= uiDiff;
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_ambassador_flamelash(Creature* pCreature)
{
    return new boss_ambassador_flamelashAI(pCreature);
}

void AddSC_boss_ambassador_flamelash()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "boss_ambassador_flamelash";
    pNewScript->GetAI = &GetAI_boss_ambassador_flamelash;
    pNewScript->RegisterSelf();
}
