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
 SDName: Boss_Sartura
 SD%Complete: 100
 SDComment:
 SDCategory: Temple of Ahn'Qiraj
 EndScriptData
 */

#include "AI/ScriptDevAI/include/precompiled.h"
#include "temple_of_ahnqiraj.h"

enum
{
    SAY_AGGRO                   = -1531008,
    SAY_SLAY                    = -1531009,
    SAY_DEATH                   = -1531010,

    EMOTE_FRENZY                = -1000002,
    EMOTE_BERSERK               = -1000004,

    SPELL_ENRAGE                = 8269,
    SPELL_SUNDERING_CLEAVE      = 25174,
    SPELL_WHIRLWIND             = 26083,
    SPELL_BERSERK               = 27680,
};

struct boss_sarturaAI : public ScriptedAI
{
    boss_sarturaAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
        Reset();
    }

    ScriptedInstance* m_pInstance;

    uint32 m_uiWhirlWindTimer;
    uint32 m_uiSunderingCleaveTimer;
    uint32 m_uiBerserkTimer;

    bool m_bIsEnraged;

    void Reset() override
    {
        m_uiWhirlWindTimer = urand(10, 20) * IN_MILLISECONDS;
        m_uiSunderingCleaveTimer = urand(2, 5) * IN_MILLISECONDS;
        m_uiBerserkTimer = 10 * MINUTE * IN_MILLISECONDS;

        m_bIsEnraged = false;
    }

    void Aggro(Unit* /*pWho*/) override
    {
        DoScriptText(SAY_AGGRO, m_creature);

        if (m_pInstance)
            m_pInstance->SetData(TYPE_SARTURA, IN_PROGRESS);
    }

    void KilledUnit(Unit* /*pVictim*/) override
    {
        DoScriptText(SAY_SLAY, m_creature);
    }

    void JustDied(Unit* /*pKiller*/) override
    {
        DoScriptText(SAY_DEATH, m_creature);

        if (m_pInstance)
            m_pInstance->SetData(TYPE_SARTURA, DONE);
    }

    void JustReachedHome() override
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_SARTURA, FAIL);
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        // Berserk
        if (m_uiBerserkTimer)
        {
            if (m_uiBerserkTimer <= uiDiff)
            {
                if (DoCastSpellIfCan(m_creature, SPELL_BERSERK) == CAST_OK)
                {
                    DoScriptText(EMOTE_BERSERK, m_creature);
                    m_uiBerserkTimer = 0;
                }
            }
            else
                m_uiBerserkTimer -= uiDiff;
        }

        // Enrage
        if (!m_bIsEnraged && m_creature->GetHealthPercent() <= 25.0f)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_ENRAGE) == CAST_OK)
            {
                DoScriptText(EMOTE_FRENZY, m_creature);
                m_bIsEnraged = true;
            }
        }

        // Whirlwind
        if (m_uiWhirlWindTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_WHIRLWIND) == CAST_OK)
                m_uiWhirlWindTimer = urand(20, 25) * IN_MILLISECONDS;
        }
        else
            m_uiWhirlWindTimer -= uiDiff;

        // Sundering Cleave
        if (m_uiSunderingCleaveTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_SUNDERING_CLEAVE) == CAST_OK)
                m_uiSunderingCleaveTimer = urand(2, 5) * IN_MILLISECONDS;
        }
        else
            m_uiSunderingCleaveTimer -= uiDiff;

        DoMeleeAttackIfReady();
    }
};

UnitAI* GetAI_boss_sartura(Creature* pCreature)
{
    return new boss_sarturaAI(pCreature);
}

void AddSC_boss_sartura()
{
    Script* pNewScript = new Script;
    pNewScript->Name = "boss_sartura";
    pNewScript->GetAI = &GetAI_boss_sartura;
    pNewScript->RegisterSelf();
}
