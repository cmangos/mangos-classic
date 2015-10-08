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
SDName: Boss_Patchwerk
SD%Complete: 100
SDComment:
SDCategory: Naxxramas
EndScriptData */

#include "precompiled.h"
#include "naxxramas.h"

enum
{
    SAY_AGGRO1            = -1533017,
    SAY_AGGRO2            = -1533018,
    SAY_SLAY              = -1533019,
    SAY_DEATH             = -1533020,

    EMOTE_GENERIC_BERSERK   = -1000004,
    EMOTE_GENERIC_ENRAGED   = -1000003,

    SPELL_HATEFULSTRIKE   = 28308,
    SPELL_ENRAGE          = 28131,
    SPELL_BERSERK         = 26662,
    SPELL_SLIMEBOLT       = 32309
};

struct boss_patchwerkAI : public ScriptedAI
{
    boss_patchwerkAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = (instance_naxxramas*)pCreature->GetInstanceData();
        Reset();
    }

    instance_naxxramas* m_pInstance;

    uint32 m_uiHatefulStrikeTimer;
    uint32 m_uiBerserkTimer;
    uint32 m_uiSlimeboltTimer;
    bool   m_bEnraged;
    bool   m_bBerserk;

    void Reset() override
    {
        m_uiHatefulStrikeTimer = 1000;                      // 1 second
        m_uiBerserkTimer = MINUTE * 6 * IN_MILLISECONDS;    // 6 minutes
        m_uiSlimeboltTimer = 10000;
        m_bEnraged = false;
        m_bBerserk = false;
    }

    void KilledUnit(Unit* /*pVictim*/) override
    {
        if (urand(0, 4))
            return;

        DoScriptText(SAY_SLAY, m_creature);
    }

    void JustDied(Unit* /*pKiller*/) override
    {
        DoScriptText(SAY_DEATH, m_creature);

        if (m_pInstance)
            m_pInstance->SetData(TYPE_PATCHWERK, DONE);
    }

    void Aggro(Unit* /*pWho*/) override
    {
        DoScriptText(urand(0, 1) ? SAY_AGGRO1 : SAY_AGGRO2, m_creature);

        if (m_pInstance)
            m_pInstance->SetData(TYPE_PATCHWERK, IN_PROGRESS);
    }

    void JustReachedHome() override
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_PATCHWERK, FAIL);
    }

    void DoHatefulStrike()
    {
        // The ability is used on highest HP target choosen of the top 2 (3 heroic) targets on threat list being in melee range
        Unit* pTarget = nullptr;
        uint32 uiHighestHP = 0;
        uint32 uiTargets = 2;

        ThreatList const& tList = m_creature->getThreatManager().getThreatList();
        if (tList.size() > 1)                               // Check if more than two targets, and start loop with second-most aggro
        {
            ThreatList::const_iterator iter = tList.begin();
            std::advance(iter, 1);
            for (; iter != tList.end(); ++iter)
            {
                if (!uiTargets)
                    break;

                if (Unit* pTempTarget = m_creature->GetMap()->GetUnit((*iter)->getUnitGuid()))
                {
                    if (m_creature->CanReachWithMeleeAttack(pTempTarget))
                    {
                        if (pTempTarget->GetHealth() > uiHighestHP)
                        {
                            uiHighestHP = pTempTarget->GetHealth();
                            pTarget = pTempTarget;
                        }
                        --uiTargets;
                    }
                }
            }
        }

        if (!pTarget)
            pTarget = m_creature->getVictim();

        DoCastSpellIfCan(pTarget, SPELL_HATEFULSTRIKE);
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        // Hateful Strike
        if (m_uiHatefulStrikeTimer < uiDiff)
        {
            DoHatefulStrike();
            m_uiHatefulStrikeTimer = 1000;
        }
        else
            m_uiHatefulStrikeTimer -= uiDiff;

        // Soft Enrage at 5%
        if (!m_bEnraged)
        {
            if (m_creature->GetHealthPercent() < 5.0f)
            {
                if (DoCastSpellIfCan(m_creature, SPELL_ENRAGE) == CAST_OK)
                {
                    DoScriptText(EMOTE_GENERIC_ENRAGED, m_creature);
                    m_bEnraged = true;
                }
            }
        }

        // Berserk after 6 minutes
        if (!m_bBerserk)
        {
            if (m_uiBerserkTimer < uiDiff)
            {
                if (DoCastSpellIfCan(m_creature, SPELL_BERSERK) == CAST_OK)
                {
                    DoScriptText(EMOTE_GENERIC_BERSERK, m_creature);
                    m_bBerserk = true;
                }
            }
            else
                m_uiBerserkTimer -= uiDiff;
        }
        else
        {
            // Slimebolt - casted only while Berserking to prevent kiting
            if (m_uiSlimeboltTimer < uiDiff)
            {
                DoCastSpellIfCan(m_creature->getVictim(), SPELL_SLIMEBOLT);
                m_uiSlimeboltTimer = 5000;
            }
            else
                m_uiSlimeboltTimer -= uiDiff;
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_patchwerk(Creature* pCreature)
{
    return new boss_patchwerkAI(pCreature);
}

void AddSC_boss_patchwerk()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "boss_patchwerk";
    pNewScript->GetAI = &GetAI_boss_patchwerk;
    pNewScript->RegisterSelf();
}
