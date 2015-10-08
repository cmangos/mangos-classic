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
SDName: Boss_Marli
SD%Complete: 90
SDComment: Enlarge for small spiders NYI
SDCategory: Zul'Gurub
EndScriptData */

#include "precompiled.h"
#include "zulgurub.h"

enum
{
    SAY_AGGRO                   = -1309005,
    SAY_TRANSFORM               = -1309006,
    SAY_TRANSFORM_BACK          = -1309025,
    SAY_SPIDER_SPAWN            = -1309007,
    SAY_DEATH                   = -1309008,

    // Spider form spells
    SPELL_CORROSIVE_POISON      = 24111,
    SPELL_CHARGE                = 22911,
    SPELL_ENVELOPING_WEBS       = 24110,
    SPELL_POISON_SHOCK          = 24112,                    // purpose of this spell is unk

    // Troll form spells
    SPELL_POISON_VOLLEY         = 24099,
    SPELL_DRAIN_LIFE            = 24300,
    SPELL_ENLARGE               = 24109,                    // purpose of this spell is unk
    SPELL_SPIDER_EGG            = 24082,                    // removed from DBC - should trigger 24081 which summons 15041

    // common spells
    SPELL_SPIDER_FORM           = 24084,
    SPELL_TRANSFORM_BACK        = 24085,
    SPELL_TRASH                 = 3391,
    SPELL_HATCH_EGGS            = 24083,                    // note this should only target 4 eggs!
};

struct boss_marliAI : public ScriptedAI
{
    boss_marliAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
        Reset();
    }

    ScriptedInstance* m_pInstance;

    uint32 m_uiPoisonVolleyTimer;
    uint32 m_uiSpawnSpiderTimer;
    uint32 m_uiChargeTimer;
    uint32 m_uiTransformTimer;
    uint32 m_uiDrainLifeTimer;
    uint32 m_uiCorrosivePoisonTimer;
    uint32 m_uiWebsTimer;
    uint32 m_uiTrashTimer;

    bool m_bIsInPhaseTwo;

    void Reset() override
    {
        m_uiPoisonVolleyTimer   = 15000;
        m_uiSpawnSpiderTimer    = 55000;
        m_uiTransformTimer      = 60000;
        m_uiDrainLifeTimer      = 30000;
        m_uiCorrosivePoisonTimer = 1000;
        m_uiWebsTimer           = 5000;
        m_uiTrashTimer          = 5000;

        m_bIsInPhaseTwo         = false;
    }

    void Aggro(Unit* /*pWho*/) override
    {
        DoScriptText(SAY_AGGRO, m_creature);

        DoCastSpellIfCan(m_creature, SPELL_HATCH_EGGS);
    }

    void JustDied(Unit* /*pKiller*/) override
    {
        DoScriptText(SAY_DEATH, m_creature);

        if (m_pInstance)
            m_pInstance->SetData(TYPE_MARLI, DONE);
    }

    void JustReachedHome() override
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_MARLI, FAIL);
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        // Troll phase
        if (!m_bIsInPhaseTwo)
        {
            if (m_uiPoisonVolleyTimer < uiDiff)
            {
                if (DoCastSpellIfCan(m_creature, SPELL_POISON_VOLLEY) == CAST_OK)
                    m_uiPoisonVolleyTimer = urand(10000, 20000);
            }
            else
                m_uiPoisonVolleyTimer -= uiDiff;

            if (m_uiDrainLifeTimer < uiDiff)
            {
                if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
                {
                    if (DoCastSpellIfCan(pTarget, SPELL_DRAIN_LIFE) == CAST_OK)
                        m_uiDrainLifeTimer = urand(20000, 50000);
                }
            }
            else
                m_uiDrainLifeTimer -= uiDiff;

            if (m_uiSpawnSpiderTimer < uiDiff)
            {
                // Workaround for missing spell 24082 - creature always selects the closest egg for hatch
                if (m_pInstance)
                {
                    if (GameObject* pEgg = GetClosestGameObjectWithEntry(m_creature, GO_SPIDER_EGG, 30.0f))
                    {
                        if (urand(0, 1))
                            DoScriptText(SAY_SPIDER_SPAWN, m_creature);

                        pEgg->Use(m_creature);
                        m_uiSpawnSpiderTimer = 60000;
                    }
                }
            }
            else
                m_uiSpawnSpiderTimer -= uiDiff;
        }
        // Spider phase
        else
        {
            if (m_uiWebsTimer < uiDiff)
            {
                if (DoCastSpellIfCan(m_creature, SPELL_ENVELOPING_WEBS) == CAST_OK)
                {
                    m_uiWebsTimer = urand(15000, 20000);
                    m_uiChargeTimer = 1000;
                }
            }
            else
                m_uiWebsTimer -= uiDiff;

            if (m_uiChargeTimer)
            {
                if (m_uiChargeTimer < uiDiff)
                {
                    // ToDo: research if the selected target shouldn't have the enveloping webs aura
                    if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0, SPELL_CHARGE, SELECT_FLAG_NOT_IN_MELEE_RANGE))
                    {
                        if (DoCastSpellIfCan(pTarget, SPELL_CHARGE) == CAST_OK)
                        {
                            DoResetThreat();
                            m_uiChargeTimer = 0;
                        }
                    }
                }
                else
                    m_uiChargeTimer -= uiDiff;
            }

            if (m_uiCorrosivePoisonTimer < uiDiff)
            {
                if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
                {
                    if (DoCastSpellIfCan(pTarget, SPELL_CORROSIVE_POISON) == CAST_OK)
                        m_uiCorrosivePoisonTimer = urand(25000, 35000);
                }
            }
            else
                m_uiCorrosivePoisonTimer -= uiDiff;
        }

        // Transform from Troll to Spider and back
        if (m_uiTransformTimer < uiDiff)
        {
            if (!m_bIsInPhaseTwo)
            {
                if (DoCastSpellIfCan(m_creature, SPELL_SPIDER_FORM) == CAST_OK)
                {
                    DoScriptText(SAY_TRANSFORM, m_creature);
                    DoResetThreat();
                    m_uiTransformTimer = 60000;
                    m_uiWebsTimer = 5000;
                    m_bIsInPhaseTwo = true;
                }
            }
            else
            {
                if (DoCastSpellIfCan(m_creature, SPELL_TRANSFORM_BACK) == CAST_OK)
                {
                    DoScriptText(SAY_TRANSFORM_BACK, m_creature);
                    m_creature->RemoveAurasDueToSpell(SPELL_SPIDER_FORM);
                    m_bIsInPhaseTwo = false;
                    m_uiTransformTimer = 60000;
                }
            }
        }
        else
            m_uiTransformTimer -= uiDiff;

        if (m_uiTrashTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_TRASH) == CAST_OK)
                m_uiTrashTimer = urand(10000, 20000);
        }
        else
            m_uiTrashTimer -= uiDiff;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_marli(Creature* pCreature)
{
    return new boss_marliAI(pCreature);
}

void AddSC_boss_marli()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "boss_marli";
    pNewScript->GetAI = &GetAI_boss_marli;
    pNewScript->RegisterSelf();
}
