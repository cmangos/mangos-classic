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
SDName: Boss_Herod
SD%Complete: 95
SDComment: Should in addition spawn Myrmidons in the hallway outside
SDCategory: Scarlet Monastery
EndScriptData */

#include "precompiled.h"
#include "escort_ai.h"

enum
{
    SAY_AGGRO              = -1189000,
    SAY_WHIRLWIND          = -1189001,
    SAY_ENRAGE             = -1189002,
    SAY_KILL               = -1189003,
    EMOTE_GENERIC_ENRAGED  = -1000003,

    SAY_TRAINEE_SPAWN      = -1189035,

    SPELL_RUSHINGCHARGE    = 8260,
    SPELL_CLEAVE           = 15496,
    SPELL_WHIRLWIND        = 8989,
    SPELL_FRENZY           = 8269,

    NPC_SCARLET_TRAINEE    = 6575
};

struct boss_herodAI : public ScriptedAI
{
    boss_herodAI(Creature* pCreature) : ScriptedAI(pCreature) {Reset();}

    bool m_bEnrage;
    bool m_bTraineeSay;

    uint32 m_uiCleaveTimer;
    uint32 m_uiWhirlwindTimer;

    void Reset() override
    {
        m_bTraineeSay = false;
        m_bEnrage     = false;

        m_uiCleaveTimer    = 7500;
        m_uiWhirlwindTimer = 14500;
    }

    void Aggro(Unit* /*pWho*/) override
    {
        DoScriptText(SAY_AGGRO, m_creature);
        DoCastSpellIfCan(m_creature, SPELL_RUSHINGCHARGE);
    }

    void SummonedCreature(Creature* pSummoned)
    {
        // make first Scarlet Trainee say text
        if (pSummoned->GetEntry() == NPC_SCARLET_TRAINEE && !m_bTraineeSay)
        {
            DoScriptText(SAY_TRAINEE_SPAWN, pSummoned);
            m_bTraineeSay = true;
        }
    }

    void KilledUnit(Unit* /*pVictim*/) override
    {
        DoScriptText(SAY_KILL, m_creature);
    }

    void JustDied(Unit* /*pKiller*/) override
    {
        for (uint8 i = 0; i < 20; ++i)
            m_creature->SummonCreature(NPC_SCARLET_TRAINEE, 1939.18f, -431.58f, 17.09f, 6.22f, TEMPSUMMON_TIMED_OOC_OR_DEAD_DESPAWN, 600000);
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        // If we are < 30% hp enrage
        if (!m_bEnrage && m_creature->GetHealthPercent() <= 30.0f && !m_creature->IsNonMeleeSpellCasted(false))
        {
            if (DoCastSpellIfCan(m_creature, SPELL_FRENZY) == CAST_OK)
            {
                DoScriptText(EMOTE_GENERIC_ENRAGED, m_creature);
                DoScriptText(SAY_ENRAGE, m_creature);
                m_bEnrage = true;
            }
        }

        // Cleave
        if (m_uiCleaveTimer < uiDiff)
        {
            DoCastSpellIfCan(m_creature->getVictim(), SPELL_CLEAVE);
            m_uiCleaveTimer = urand(7500, 17500);
        }
        else
            m_uiCleaveTimer -= uiDiff;

        if (m_uiWhirlwindTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_WHIRLWIND) == CAST_OK)
            {
                DoScriptText(SAY_WHIRLWIND, m_creature);
                m_uiWhirlwindTimer = urand(15000, 25000);
            }
        }
        else
            m_uiWhirlwindTimer -= uiDiff;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_herod(Creature* pCreature)
{
    return new boss_herodAI(pCreature);
}

struct mob_scarlet_traineeAI : public npc_escortAI
{
    mob_scarlet_traineeAI(Creature* pCreature) : npc_escortAI(pCreature)
    {
        m_uiStartTimer = urand(1000, 6000);
        Reset();
    }

    uint32 m_uiStartTimer;

    void Reset() override { }
    void WaypointReached(uint32 /*uiPointId*/) override {}

    void UpdateEscortAI(const uint32 uiDiff) override
    {
        if (m_uiStartTimer)
        {
            if (m_uiStartTimer <= uiDiff)
            {
                Start(true);
                m_uiStartTimer = 0;
            }
            else
                m_uiStartTimer -= uiDiff;
        }

        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_scarlet_trainee(Creature* pCreature)
{
    return new mob_scarlet_traineeAI(pCreature);
}

void AddSC_boss_herod()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "boss_herod";
    pNewScript->GetAI = &GetAI_boss_herod;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "mob_scarlet_trainee";
    pNewScript->GetAI = &GetAI_mob_scarlet_trainee;
    pNewScript->RegisterSelf();
}
