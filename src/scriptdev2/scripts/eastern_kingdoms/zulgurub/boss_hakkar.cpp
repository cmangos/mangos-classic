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
SDName: Boss_Hakkar
SD%Complete: 100
SDComment:
SDCategory: Zul'Gurub
EndScriptData */

#include "precompiled.h"
#include "zulgurub.h"

enum
{
    SAY_AGGRO                   = -1309020,
    SAY_FLEEING                 = -1309021,

    SPELL_BLOOD_SIPHON          = 24324,                    // triggers 24322 or 24323 on caster
    SPELL_CORRUPTED_BLOOD       = 24328,
    SPELL_CAUSE_INSANITY        = 24327,
    SPELL_WILL_OF_HAKKAR        = 24178,
    SPELL_ENRAGE                = 24318,

    // The Aspects of all High Priests
    SPELL_ASPECT_OF_JEKLIK      = 24687,
    SPELL_ASPECT_OF_VENOXIS     = 24688,
    SPELL_ASPECT_OF_MARLI       = 24686,
    SPELL_ASPECT_OF_THEKAL      = 24689,
    SPELL_ASPECT_OF_ARLOKK      = 24690
};

struct boss_hakkarAI : public ScriptedAI
{
    boss_hakkarAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
        Reset();
    }

    ScriptedInstance* m_pInstance;

    uint32 m_uiBloodSiphonTimer;
    uint32 m_uiCorruptedBloodTimer;
    uint32 m_uiCauseInsanityTimer;
    uint32 m_uiWillOfHakkarTimer;
    uint32 m_uiEnrageTimer;

    uint32 m_uiAspectOfJeklikTimer;
    uint32 m_uiAspectOfVenoxisTimer;
    uint32 m_uiAspectOfMarliTimer;
    uint32 m_uiAspectOfThekalTimer;
    uint32 m_uiAspectOfArlokkTimer;

    void Reset() override
    {
        m_uiBloodSiphonTimer       = 90000;
        m_uiCorruptedBloodTimer    = 25000;
        m_uiCauseInsanityTimer     = 17000;
        m_uiWillOfHakkarTimer      = 17000;
        m_uiEnrageTimer            = 10 * MINUTE * IN_MILLISECONDS;

        m_uiAspectOfJeklikTimer    = 4000;
        m_uiAspectOfVenoxisTimer   = 7000;
        m_uiAspectOfMarliTimer     = 12000;
        m_uiAspectOfThekalTimer    = 8000;
        m_uiAspectOfArlokkTimer    = 18000;
    }

    void Aggro(Unit* /*who*/) override
    {
        DoScriptText(SAY_AGGRO, m_creature);

        // check if the priest encounters are done
        if (m_pInstance)
        {
            if (m_pInstance->GetData(TYPE_JEKLIK) == DONE)
                m_uiAspectOfJeklikTimer = 0;
            if (m_pInstance->GetData(TYPE_VENOXIS) == DONE)
                m_uiAspectOfVenoxisTimer = 0;
            if (m_pInstance->GetData(TYPE_MARLI) == DONE)
                m_uiAspectOfMarliTimer = 0;
            if (m_pInstance->GetData(TYPE_THEKAL) == DONE)
                m_uiAspectOfThekalTimer = 0;
            if (m_pInstance->GetData(TYPE_ARLOKK) == DONE)
                m_uiAspectOfArlokkTimer = 0;
        }
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        if (m_uiBloodSiphonTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_BLOOD_SIPHON) == CAST_OK)
                m_uiBloodSiphonTimer = 90000;
        }
        else
            m_uiBloodSiphonTimer -= uiDiff;

        // Corrupted Blood Timer
        if (m_uiCorruptedBloodTimer < uiDiff)
        {
            if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
            {
                if (DoCastSpellIfCan(pTarget, SPELL_CORRUPTED_BLOOD) == CAST_OK)
                    m_uiCorruptedBloodTimer = urand(30000, 45000);
            }
        }
        else
            m_uiCorruptedBloodTimer -= uiDiff;

        // Cause Insanity Timer
        if (m_uiCauseInsanityTimer < uiDiff)
        {
            if (m_creature->getThreatManager().getThreatList().size() > 1)
            {
                if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_CAUSE_INSANITY) == CAST_OK)
                    m_uiCauseInsanityTimer = urand(10000, 15000);
            }
            else // Solo case, check again later
                m_uiCauseInsanityTimer = urand(35000, 43000);
        }
        else
            m_uiCauseInsanityTimer -= uiDiff;

        // Will Of Hakkar Timer
        if (m_uiWillOfHakkarTimer < uiDiff)
        {
            if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 1))
            {
                if (DoCastSpellIfCan(pTarget, SPELL_WILL_OF_HAKKAR) == CAST_OK)
                    m_uiWillOfHakkarTimer = urand(25000, 35000);
            }
            else // solo attempt, try again later
                m_uiWillOfHakkarTimer = 25000;
        }
        else
            m_uiWillOfHakkarTimer -= uiDiff;

        if (m_uiEnrageTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_ENRAGE) == CAST_OK)
                m_uiEnrageTimer = 10 * MINUTE * IN_MILLISECONDS;
        }
        else
            m_uiEnrageTimer -= uiDiff;

        // Checking if Jeklik is dead. If not we cast her Aspect
        if (m_uiAspectOfJeklikTimer)
        {
            if (m_uiAspectOfJeklikTimer <= uiDiff)
            {
                if (DoCastSpellIfCan(m_creature, SPELL_ASPECT_OF_JEKLIK) == CAST_OK)
                    m_uiAspectOfJeklikTimer = urand(10000, 14000);
            }
            else
                m_uiAspectOfJeklikTimer -= uiDiff;
        }

        // Checking if Venoxis is dead. If not we cast his Aspect
        if (m_uiAspectOfVenoxisTimer)
        {
            if (m_uiAspectOfVenoxisTimer <= uiDiff)
            {
                if (DoCastSpellIfCan(m_creature, SPELL_ASPECT_OF_VENOXIS) == CAST_OK)
                    m_uiAspectOfVenoxisTimer = 8000;
            }
            else
                m_uiAspectOfVenoxisTimer -= uiDiff;
        }

        // Checking if Marli is dead. If not we cast her Aspect
        if (m_uiAspectOfMarliTimer)
        {
            if (m_uiAspectOfMarliTimer <= uiDiff)
            {
                if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_ASPECT_OF_MARLI) == CAST_OK)
                    m_uiAspectOfMarliTimer = 10000;
            }
            else
                m_uiAspectOfMarliTimer -= uiDiff;
        }

        // Checking if Thekal is dead. If not we cast his Aspect
        if (m_uiAspectOfThekalTimer)
        {
            if (m_uiAspectOfThekalTimer <= uiDiff)
            {
                if (DoCastSpellIfCan(m_creature, SPELL_ASPECT_OF_THEKAL) == CAST_OK)
                    m_uiAspectOfThekalTimer = 15000;
            }
            else
                m_uiAspectOfThekalTimer -= uiDiff;
        }

        // Checking if Arlokk is dead. If yes we cast her Aspect
        if (m_uiAspectOfArlokkTimer)
        {
            if (m_uiAspectOfArlokkTimer <= uiDiff)
            {
                if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_ASPECT_OF_ARLOKK) == CAST_OK)
                {
                    DoResetThreat();
                    m_uiAspectOfArlokkTimer = urand(10000, 15000);
                }
            }
            else
                m_uiAspectOfArlokkTimer -= uiDiff;
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_hakkar(Creature* pCreature)
{
    return new boss_hakkarAI(pCreature);
}

void AddSC_boss_hakkar()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "boss_hakkar";
    pNewScript->GetAI = &GetAI_boss_hakkar;
    pNewScript->RegisterSelf();
}
