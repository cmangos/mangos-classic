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
 SDName: Boss_Venoxis
 SD%Complete: 100
 SDComment:
 SDCategory: Zul'Gurub
 EndScriptData */

#include "precompiled.h"
#include "zulgurub.h"

enum
{
    SAY_TRANSFORM = -1309000,
    SAY_DEATH = -1309001,

    // troll spells
    SPELL_HOLY_FIRE = 23860,
    SPELL_HOLY_WRATH = 23979,
    SPELL_HOLY_NOVA = 23858,

    SPELL_TRASH = 3391,
    SPELL_DISPELL = 23859,
    SPELL_RENEW = 23895,

    // snake form
    SPELL_VENOMSPIT = 23862,
    SPELL_POISON_CLOUD = 23861,

    SPELL_PARASITIC_SERPENT = 23865,
    SPELL_SUMMON_PARASITIC_SERPENT = 23866,
    SPELL_PARASITIC_SERPENT_TRIGGER = 23867,

    // common spells
    SPELL_SNAKE_FORM = 23849,
    SPELL_FRENZY = 23537
};

struct boss_venoxisAI : public ScriptedAI
{
    boss_venoxisAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
        Reset();
    }

    ScriptedInstance* m_pInstance;

    uint32 m_uiHolyWrathTimer;
    uint32 m_uiHolyNovaTimer;
    uint32 m_uiHolyFireTimer;

    uint32 m_uiVenomSpitTimer;
    uint32 m_uiRenewTimer;
    uint32 m_uiPoisonCloudTimer;
    uint32 m_uiDispellTimer;
    uint32 m_uiTrashTimer;
    uint32 m_uiParasiticTimer;

    bool m_bPhaseTwo;
    bool m_bInBerserk;

    void Reset() override
    {
        m_creature->RemoveAllAuras();
        m_creature->GetCharmInfo()->SetReactState(REACT_PASSIVE);

        if (m_pInstance)
            m_pInstance->SetData(TYPE_VENOXIS, NOT_STARTED);

        m_bPhaseTwo = false;
        m_bInBerserk = false;
    }

    void JustReachedHome() override
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_VENOXIS, FAIL);
    }

    void JustDied(Unit* /*pKiller*/) override
    {
        DoScriptText(SAY_DEATH, m_creature);

        if (m_pInstance)
            m_pInstance->SetData(TYPE_VENOXIS, DONE);

        m_creature->RemoveAllAuras();
    }

    void EnterCombat(Unit* /*who*/) override
    {
        // Always running events
        m_uiTrashTimer = 5000;

        // Phase one events (regular form)
        m_uiHolyNovaTimer = 5000;
        m_uiDispellTimer = 35000;
        m_uiHolyFireTimer = 10000;
        m_uiHolyWrathTimer = 60000;
        m_uiRenewTimer = 30000;

        m_uiVenomSpitTimer = 5000;
        m_uiPoisonCloudTimer = 10000;
        m_uiParasiticTimer = 30000;

        m_bPhaseTwo = false;
        m_bInBerserk = false;

        if (m_pInstance)
            m_pInstance->SetData(TYPE_VENOXIS, IN_PROGRESS);
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        // Troll phase
        if (!m_bPhaseTwo)
        {
            if (m_uiDispellTimer < uiDiff)
            {
                if (DoCastSpellIfCan(m_creature, SPELL_DISPELL) == CAST_OK)
                    m_uiDispellTimer = urand(15000, 20000);
            }
            else
                m_uiDispellTimer -= uiDiff;

            if (m_uiRenewTimer < uiDiff)
            {
                if (DoCastSpellIfCan(m_creature, SPELL_RENEW) == CAST_OK)
                    m_uiRenewTimer = urand(25000, 30000);
            }
            else
                m_uiRenewTimer -= uiDiff;

            if (m_uiHolyNovaTimer < uiDiff)
            {
                uint8 uiTargetsInRange = 0;
                // See how many targets are in melee range
                ThreatList const& tList = m_creature->getThreatManager().getThreatList();
                for (ThreatList::const_iterator iter = tList.begin(); iter != tList.end(); ++iter)
                {
                    if (Unit* pTempTarget = m_creature->GetMap()->GetUnit((*iter)->getUnitGuid()))
                    {
                        if (pTempTarget->GetTypeId() == TYPEID_PLAYER && m_creature->CanReachWithMeleeAttack(pTempTarget))
                            ++uiTargetsInRange;
                    }
                }

                // If there are more targets in melee range cast holy nova, else holy fire
                // not sure which is the minimum targets for holy nova
                if (uiTargetsInRange >= 3)
                    DoCastSpellIfCan(m_creature, SPELL_HOLY_NOVA);

                m_uiHolyNovaTimer = urand(45000, 75000);
            }
            else
                m_uiHolyNovaTimer -= uiDiff;

            if (m_uiHolyWrathTimer < uiDiff)
            {
                if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
                    DoCastSpellIfCan(pTarget, SPELL_HOLY_WRATH);

                m_uiHolyWrathTimer = urand(45000, 60000);
            }
            else
                m_uiHolyWrathTimer -= uiDiff;

            if (m_uiHolyFireTimer < uiDiff)
            {
                if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
                    DoCastSpellIfCan(pTarget, SPELL_HOLY_FIRE);

                m_uiHolyNovaTimer = urand(45000, 60000);
            }
            else
                m_uiHolyFireTimer -= uiDiff;

            // Transform at 50% hp
            if (m_creature->GetHealthPercent() < 50.0f)
            {
                if (DoCastSpellIfCan(m_creature, SPELL_SNAKE_FORM, CAST_INTERRUPT_PREVIOUS) == CAST_OK)
                {
                    DoCastSpellIfCan(m_creature, SPELL_PARASITIC_SERPENT, CAST_TRIGGERED);
                    DoScriptText(SAY_TRANSFORM, m_creature);
                    DoResetThreat();
                    m_bPhaseTwo = true;
                }
            }
        }
        // Snake phase
        else
        {
            if (m_uiPoisonCloudTimer < uiDiff)
            {
                if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
                {
                    if (DoCastSpellIfCan(pTarget, SPELL_POISON_CLOUD) == CAST_OK)
                        m_uiVenomSpitTimer = urand(15000, 20000);
                }
            }
            else
                m_uiPoisonCloudTimer -= uiDiff;

            if (m_uiVenomSpitTimer < uiDiff)
            {
                if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
                {
                    if (DoCastSpellIfCan(pTarget, SPELL_VENOMSPIT) == CAST_OK)
                        m_uiVenomSpitTimer = urand(5000, 15000);
                }
            }
            else
                m_uiVenomSpitTimer -= uiDiff;

            if (m_uiParasiticTimer < uiDiff)
            {
                if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
                {
                    if (DoCastSpellIfCan(pTarget, SPELL_SUMMON_PARASITIC_SERPENT) == CAST_OK)
                        m_uiParasiticTimer = 15000;
                }
            }
            else
                m_uiParasiticTimer -= uiDiff;
        }

        if (m_uiTrashTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_TRASH) == CAST_OK)
                m_uiTrashTimer = urand(10000, 20000);
        }
        else
            m_uiTrashTimer -= uiDiff;

        if (!m_bInBerserk && m_creature->GetHealthPercent() < 11.0f)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_FRENZY) == CAST_OK)
                m_bInBerserk = true;
        }

        DoMeleeAttackIfReady();
    }

private:
    bool _transformed;
    bool _frenzied;
};

CreatureAI* GetAI_boss_venoxis(Creature* pCreature)
{
    return new boss_venoxisAI(pCreature);
}

void AddSC_boss_venoxis()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "boss_venoxis";
    pNewScript->GetAI = &GetAI_boss_venoxis;
    pNewScript->RegisterSelf();
}
