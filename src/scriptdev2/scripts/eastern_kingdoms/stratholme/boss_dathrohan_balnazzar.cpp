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
SDName: Boss_Dathrohan_Balnazzar
SD%Complete: 95
SDComment: Possibly need to fix/improve summons after death
SDCategory: Stratholme
EndScriptData */

#include "precompiled.h"
#include "stratholme.h"

enum
{
    SAY_AGGRO                       = -1329016,
    SAY_TRANSFORM                   = -1329017,
    SAY_DEATH                       = -1329018,

    // Dathrohan spells
    SPELL_CRUSADERSHAMMER           = 17286,                // AOE stun
    SPELL_CRUSADERSTRIKE            = 17281,
    SPELL_HOLYSTRIKE                = 17284,                // weapon dmg +3

    // Transform
    SPELL_BALNAZZARTRANSFORM        = 17288,                // restore full HP/mana, trigger spell Balnazzar Transform Stun

    // Balnazzar spells
    SPELL_SHADOWSHOCK               = 17399,
    SPELL_MINDBLAST                 = 17287,
    SPELL_PSYCHICSCREAM             = 13704,
    SPELL_SLEEP                     = 12098,
    SPELL_MINDCONTROL               = 15690,

    NPC_DATHROHAN                   = 10812
};

struct boss_dathrohan_balnazzarAI : public ScriptedAI
{
    boss_dathrohan_balnazzarAI(Creature* pCreature) : ScriptedAI(pCreature) {Reset();}

    uint32 m_uiCrusadersHammer_Timer;
    uint32 m_uiCrusaderStrike_Timer;
    uint32 m_uiMindBlast_Timer;
    uint32 m_uiHolyStrike_Timer;
    uint32 m_uiShadowShock_Timer;
    uint32 m_uiPsychicScream_Timer;
    uint32 m_uiDeepSleep_Timer;
    uint32 m_uiMindControl_Timer;
    bool m_bTransformed;

    void Reset() override
    {
        m_uiCrusadersHammer_Timer = 8000;
        m_uiCrusaderStrike_Timer = 12000;
        m_uiMindBlast_Timer = 6000;
        m_uiHolyStrike_Timer = 18000;
        m_uiShadowShock_Timer = 4000;
        m_uiPsychicScream_Timer = 16000;
        m_uiDeepSleep_Timer = 20000;
        m_uiMindControl_Timer = 10000;
        m_bTransformed = false;

        if (m_creature->GetEntry() == NPC_BALNAZZAR)
            m_creature->UpdateEntry(NPC_DATHROHAN);
    }

    void Aggro(Unit* /*pWho*/) override
    {
        DoScriptText(SAY_AGGRO, m_creature);
    }

    void JustDied(Unit* /*Victim*/) override
    {
        DoScriptText(SAY_DEATH, m_creature);
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        // START NOT TRANSFORMED
        if (!m_bTransformed)
        {
            // MindBlast
            if (m_uiMindBlast_Timer < uiDiff)
            {
                DoCastSpellIfCan(m_creature->getVictim(), SPELL_MINDBLAST);
                m_uiMindBlast_Timer = urand(15000, 20000);
            }
            else m_uiMindBlast_Timer -= uiDiff;

            // CrusadersHammer
            if (m_uiCrusadersHammer_Timer < uiDiff)
            {
                DoCastSpellIfCan(m_creature->getVictim(), SPELL_CRUSADERSHAMMER);
                m_uiCrusadersHammer_Timer = 12000;
            }
            else m_uiCrusadersHammer_Timer -= uiDiff;

            // CrusaderStrike
            if (m_uiCrusaderStrike_Timer < uiDiff)
            {
                DoCastSpellIfCan(m_creature->getVictim(), SPELL_CRUSADERSTRIKE);
                m_uiCrusaderStrike_Timer = 15000;
            }
            else m_uiCrusaderStrike_Timer -= uiDiff;

            // HolyStrike
            if (m_uiHolyStrike_Timer < uiDiff)
            {
                DoCastSpellIfCan(m_creature->getVictim(), SPELL_HOLYSTRIKE);
                m_uiHolyStrike_Timer = 15000;
            }
            else m_uiHolyStrike_Timer -= uiDiff;

            // BalnazzarTransform
            if (m_creature->GetHealthPercent() < 40.0f)
            {
                // restore hp, mana and stun
                if (DoCastSpellIfCan(m_creature, SPELL_BALNAZZARTRANSFORM) == CAST_OK)
                {
                    m_creature->UpdateEntry(NPC_BALNAZZAR);
                    DoScriptText(SAY_TRANSFORM, m_creature);
                    // Dathrohan wield a weapon but Balnazzar does not need one
                    SetEquipmentSlots(true);
                    m_creature->SetSheath(SHEATH_STATE_UNARMED);
                    m_bTransformed = true;
                }
            }
        }
        else
        {
            // MindBlast
            if (m_uiMindBlast_Timer < uiDiff)
            {
                DoCastSpellIfCan(m_creature->getVictim(), SPELL_MINDBLAST);
                m_uiMindBlast_Timer = urand(15000, 20000);
            }
            else m_uiMindBlast_Timer -= uiDiff;

            // ShadowShock
            if (m_uiShadowShock_Timer < uiDiff)
            {
                DoCastSpellIfCan(m_creature->getVictim(), SPELL_SHADOWSHOCK);
                m_uiShadowShock_Timer = 11000;
            }
            else m_uiShadowShock_Timer -= uiDiff;

            // PsychicScream
            if (m_uiPsychicScream_Timer < uiDiff)
            {
                if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
                    DoCastSpellIfCan(pTarget, SPELL_PSYCHICSCREAM);

                m_uiPsychicScream_Timer = 20000;
            }
            else m_uiPsychicScream_Timer -= uiDiff;

            // DeepSleep
            if (m_uiDeepSleep_Timer < uiDiff)
            {
                if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
                    DoCastSpellIfCan(pTarget, SPELL_SLEEP);

                m_uiDeepSleep_Timer = 15000;
            }
            else m_uiDeepSleep_Timer -= uiDiff;

            // MindControl
            if (m_uiMindControl_Timer < uiDiff)
            {
                DoCastSpellIfCan(m_creature->getVictim(), SPELL_MINDCONTROL);
                m_uiMindControl_Timer = 15000;
            }
            else m_uiMindControl_Timer -= uiDiff;
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_dathrohan_balnazzar(Creature* pCreature)
{
    return new boss_dathrohan_balnazzarAI(pCreature);
}

void AddSC_boss_dathrohan_balnazzar()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "boss_dathrohan_balnazzar";
    pNewScript->GetAI = &GetAI_boss_dathrohan_balnazzar;
    pNewScript->RegisterSelf();
}
