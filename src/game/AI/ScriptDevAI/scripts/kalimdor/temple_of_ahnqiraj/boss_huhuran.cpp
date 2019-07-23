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
SD%Complete: 100
SDComment:
SDCategory: Temple of Ahn'Qiraj
EndScriptData

*/

#include "AI/ScriptDevAI/include/precompiled.h"
#include "temple_of_ahnqiraj.h"

enum
{
    EMOTE_GENERIC_FRENZY_KILL   = -1000001,
    EMOTE_GENERIC_BERSERK       = -1000004,

    SPELL_ENRAGE                = 26051,        // triggers 26052 - Poison Bolt Volley (will hit the 15 closest people)
    SPELL_BERSERK               = 26068,        // triggers 26052 - Poison Bolt Volley (will hit the 15 closest people)
    SPELL_NOXIOUS_POISON        = 26053,
    SPELL_WYVERN_STING          = 26180,        // triggers 26233 on aura removal, doing 500 nature damage if 26180 expires (confirmed) and about 3K if dispelled early (amount unconfirmed)
    SPELL_ACID_SPIT             = 26050
};

struct boss_huhuranAI : public ScriptedAI
{
    boss_huhuranAI(Creature* creature) : ScriptedAI(creature)
    {
        m_instance = (ScriptedInstance*)creature->GetInstanceData();
        Reset();
    }

    ScriptedInstance* m_instance;

    uint32 m_frenzyTimer;
    uint32 m_wyvernTimer;
    uint32 m_spitTimer;
    uint32 m_berserkTimer;
    uint32 m_noxiousPoisonTimer;

    bool m_isBerserk;

    void Reset() override
    {
        m_frenzyTimer         = urand(25, 35) * IN_MILLISECONDS;
        m_wyvernTimer         = urand(35, 45) * IN_MILLISECONDS;
        m_spitTimer           = 8 * IN_MILLISECONDS;
        m_noxiousPoisonTimer  = urand(10, 20) * IN_MILLISECONDS;
        m_berserkTimer        = 5 * MINUTE * IN_MILLISECONDS;
        m_isBerserk           = false;
    }

    void Aggro(Unit* /*who*/) override
    {
        if (m_instance)
            m_instance->SetData(TYPE_HUHURAN, IN_PROGRESS);
    }

    void JustReachedHome() override
    {
        if (m_instance)
            m_instance->SetData(TYPE_HUHURAN, FAIL);
    }

    void JustDied(Unit* /*killer*/) override
    {
        if (m_instance)
            m_instance->SetData(TYPE_HUHURAN, DONE);
    }

    void UpdateAI(const uint32 diff) override
    {
        // Return since we have no target
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        // Frenzy_Timer
        if (!m_isBerserk)
        {
            if (m_frenzyTimer < diff)
            {
                if (DoCastSpellIfCan(m_creature, SPELL_ENRAGE) == CAST_OK)
                {
                    DoScriptText(EMOTE_GENERIC_FRENZY_KILL, m_creature);
                    m_frenzyTimer = urand(10, 20) * IN_MILLISECONDS;;
                }
            }
            else
                m_frenzyTimer -= diff;
        }

        // Wyvern Timer - current victim and the 9 others closest targets
        if (m_wyvernTimer < diff)
        {
            if (DoCastSpellIfCan(nullptr, SPELL_WYVERN_STING) == CAST_OK)
                m_wyvernTimer = urand(25, 35) * IN_MILLISECONDS;;
        }
        else
            m_wyvernTimer -= diff;

        // Spit Timer
        if (m_spitTimer < diff)
        {
            if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_ACID_SPIT) == CAST_OK)
                m_spitTimer = urand(5, 10) * IN_MILLISECONDS;;
        }
        else
            m_spitTimer -= diff;

        // NoxiousPoison_Timer
        if (m_noxiousPoisonTimer < diff)
        {
            if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
            {
                if (DoCastSpellIfCan(pTarget, SPELL_NOXIOUS_POISON) == CAST_OK)
                    m_noxiousPoisonTimer = urand(12, 24) * IN_MILLISECONDS;;
            }
        }
        else
            m_noxiousPoisonTimer -= diff;

        // Berserk
        if (!m_isBerserk && (m_creature->GetHealthPercent() < 30.0f || m_berserkTimer < diff))
        {
            if (DoCastSpellIfCan(m_creature, SPELL_BERSERK) == CAST_OK)
            {
                DoScriptText(EMOTE_GENERIC_BERSERK, m_creature);
                m_isBerserk = true;
            }
        }
        m_berserkTimer -= diff;

        DoMeleeAttackIfReady();
    }
};

UnitAI* GetAI_boss_huhuran(Creature* creature)
{
    return new boss_huhuranAI(creature);
}

void AddSC_boss_huhuran()
{
    Script* pNewScript = new Script;
    pNewScript->Name = "boss_huhuran";
    pNewScript->GetAI = &GetAI_boss_huhuran;
    pNewScript->RegisterSelf();
}
