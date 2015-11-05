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
SDName: Boss_Huhuran
SD%Complete: 90
SDComment: Timed enrage NYI; Timers
SDCategory: Temple of Ahn'Qiraj
EndScriptData */

#include "precompiled.h"
#include "temple_of_ahnqiraj.h"

enum
{
    EMOTE_GENERIC_FRENZY_KILL   = -1000001,
    EMOTE_GENERIC_BERSERK       = -1000004,

    SPELL_ENRAGE                = 26051,        // triggers 26052
    SPELL_BERSERK               = 26068,        // triggers 26052
    SPELL_NOXIOUS_POISON        = 26053,
    SPELL_WYVERN_STING          = 26180,
    SPELL_ACID_SPIT             = 26050
};

struct boss_huhuranAI : public ScriptedAI
{
    boss_huhuranAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
        Reset();
    }

    ScriptedInstance* m_pInstance;

    uint32 m_uiFrenzyTimer;
    uint32 m_uiWyvernTimer;
    uint32 m_uiSpitTimer;
    uint32 m_uiNoxiousPoisonTimer;

    bool m_bIsBerserk;

    void Reset() override
    {
        m_uiFrenzyTimer         = urand(25000, 35000);
        m_uiWyvernTimer         = urand(18000, 28000);
        m_uiSpitTimer           = 8000;
        m_uiNoxiousPoisonTimer  = urand(10000, 20000);
        m_bIsBerserk            = false;
    }

    void Aggro(Unit* /*pWho*/) override
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_HUHURAN, IN_PROGRESS);
    }

    void JustReachedHome() override
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_HUHURAN, FAIL);
    }

    void JustDied(Unit* /*pKiller*/) override
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_HUHURAN, DONE);
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        // Return since we have no target
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        // Frenzy_Timer
        if (!m_bIsBerserk)
        {
            if (m_uiFrenzyTimer < uiDiff)
            {
                if (DoCastSpellIfCan(m_creature, SPELL_ENRAGE) == CAST_OK)
                {
                    DoScriptText(EMOTE_GENERIC_FRENZY_KILL, m_creature);
                    m_uiFrenzyTimer = urand(25000, 35000);
                }
            }
            else
                m_uiFrenzyTimer -= uiDiff;
        }

        // Wyvern Timer
        if (m_uiWyvernTimer < uiDiff)
        {
            if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
            {
                if (DoCastSpellIfCan(pTarget, SPELL_WYVERN_STING) == CAST_OK)
                    m_uiWyvernTimer = urand(15000, 32000);
            }
        }
        else
            m_uiWyvernTimer -= uiDiff;

        // Spit Timer
        if (m_uiSpitTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_ACID_SPIT) == CAST_OK)
                m_uiSpitTimer = urand(5000, 10000);
        }
        else
            m_uiSpitTimer -= uiDiff;

        // NoxiousPoison_Timer
        if (m_uiNoxiousPoisonTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_NOXIOUS_POISON) == CAST_OK)
                m_uiNoxiousPoisonTimer = urand(12000, 24000);
        }
        else
            m_uiNoxiousPoisonTimer -= uiDiff;

        // Berserk
        if (!m_bIsBerserk && m_creature->GetHealthPercent() < 30.0f)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_BERSERK) == CAST_OK)
            {
                DoScriptText(EMOTE_GENERIC_BERSERK, m_creature);
                m_bIsBerserk = true;
            }
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_huhuran(Creature* pCreature)
{
    return new boss_huhuranAI(pCreature);
}

void AddSC_boss_huhuran()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "boss_huhuran";
    pNewScript->GetAI = &GetAI_boss_huhuran;
    pNewScript->RegisterSelf();
}
