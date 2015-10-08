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
SDName: Boss_Kazzak
SD%Complete: 80
SDComment: Spells and timers need to be confirmed.
SDCategory: Bosses
EndScriptData */

#include "precompiled.h"

enum
{
    SAY_INTRO                 = -1000147,
    SAY_AGGRO1                = -1000148,
    SAY_AGGRO2                = -1000149,
    SAY_SURPREME1             = -1000150,
    SAY_SURPREME2             = -1000151,
    SAY_KILL1                 = -1000152,
    SAY_KILL2                 = -1000153,
    SAY_KILL3                 = -1000154,
    SAY_DEATH                 = -1000155,
    EMOTE_FRENZY              = -1000002,
    SAY_RAND1                 = -1000157,
    SAY_RAND2                 = -1000158,

    SPELL_SHADOW_VOLLEY       = 21341,
    SPELL_BERSERK             = 21340,
    SPELL_CLEAVE              = 20691,
    SPELL_THUNDERCLAP         = 26554,
    SPELL_VOIDBOLT            = 21066,
    SPELL_MARK_OF_KAZZAK      = 21056,                  // triggers 21058 when target gets to 0 mana
    SPELL_CAPTURESOUL         = 21053,                  // procs 21054 on kill
    SPELL_TWISTEDREFLECTION   = 21063
};

struct boss_kazzakAI : public ScriptedAI
{
    boss_kazzakAI(Creature* pCreature) : ScriptedAI(pCreature) { Reset(); }

    uint32 m_uiShadowVolleyTimer;
    uint32 m_uiCleaveTimer;
    uint32 m_uiThunderClapTimer;
    uint32 m_uiVoidBoltTimer;
    uint32 m_uiMarkOfKazzakTimer;
    uint32 m_uiTwistedReflectionTimer;
    uint32 m_uiSupremeTimer;

    void Reset() override
    {
        m_uiShadowVolleyTimer       = urand(3000, 12000);
        m_uiCleaveTimer             = 7000;
        m_uiThunderClapTimer        = urand(16000, 20000);
        m_uiVoidBoltTimer           = 30000;
        m_uiMarkOfKazzakTimer       = 25000;
        m_uiTwistedReflectionTimer  = 33000;
        m_uiSupremeTimer            = 3 * MINUTE * IN_MILLISECONDS;
    }

    void JustRespawned() override
    {
        DoScriptText(SAY_INTRO, m_creature);
    }

    void Aggro(Unit* /*pWho*/) override
    {
        DoCastSpellIfCan(m_creature, SPELL_CAPTURESOUL, CAST_TRIGGERED);
        DoScriptText(urand(0, 1) ? SAY_AGGRO1 : SAY_AGGRO2, m_creature);
    }

    void KilledUnit(Unit* pVictim) override
    {
        if (pVictim->GetTypeId() != TYPEID_PLAYER)
            return;

        switch (urand(0, 3))
        {
            case 0: DoScriptText(SAY_KILL1, m_creature); break;
            case 1: DoScriptText(SAY_KILL2, m_creature); break;
            case 2: DoScriptText(SAY_KILL3, m_creature); break;
        }
    }

    void JustDied(Unit* /*pKiller*/) override
    {
        DoScriptText(SAY_DEATH, m_creature);
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        if (m_uiSupremeTimer)
        {
            // Enrage - cast shadowbolt volley every second
            if (m_uiSupremeTimer <= uiDiff)
            {
                if (DoCastSpellIfCan(m_creature, SPELL_BERSERK) == CAST_OK)
                {
                    DoScriptText(urand(0, 1) ? SAY_SURPREME1 : SAY_SURPREME2, m_creature);
                    m_uiSupremeTimer = 0;
                }
            }
            else
                m_uiSupremeTimer -= uiDiff;

            // Cast shadowbolt volley on timer before Berserk
            if (m_uiShadowVolleyTimer < uiDiff)
            {
                if (DoCastSpellIfCan(m_creature, SPELL_SHADOW_VOLLEY) == CAST_OK)
                    m_uiShadowVolleyTimer = urand(4000, 20000);
            }
            else
                m_uiShadowVolleyTimer -= uiDiff;
        }

        if (m_uiCleaveTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_CLEAVE) == CAST_OK)
                m_uiCleaveTimer = urand(8000, 12000);
        }
        else
            m_uiCleaveTimer -= uiDiff;

        if (m_uiThunderClapTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_THUNDERCLAP) == CAST_OK)
                m_uiThunderClapTimer = urand(10000, 14000);
        }
        else
            m_uiThunderClapTimer -= uiDiff;

        if (m_uiVoidBoltTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_VOIDBOLT) == CAST_OK)
                m_uiVoidBoltTimer = urand(15000, 28000);
        }
        else
            m_uiVoidBoltTimer -= uiDiff;

        if (m_uiMarkOfKazzakTimer < uiDiff)
        {
            if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0, SPELL_MARK_OF_KAZZAK, SELECT_FLAG_POWER_MANA))
            {
                if (DoCastSpellIfCan(pTarget, SPELL_MARK_OF_KAZZAK) == CAST_OK)
                    m_uiMarkOfKazzakTimer = 20000;
            }
        }
        else
            m_uiMarkOfKazzakTimer -= uiDiff;

        if (m_uiTwistedReflectionTimer < uiDiff)
        {
            if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
            {
                if (DoCastSpellIfCan(pTarget, SPELL_TWISTEDREFLECTION) == CAST_OK)
                    m_uiTwistedReflectionTimer = 15000;
            }
        }
        else
            m_uiTwistedReflectionTimer -= uiDiff;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_kazzak(Creature* pCreature)
{
    return new boss_kazzakAI(pCreature);
}

void AddSC_boss_kazzakAI()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "boss_kazzak";
    pNewScript->GetAI = &GetAI_boss_kazzak;
    pNewScript->RegisterSelf();
}
