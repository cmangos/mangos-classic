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
SDName: Boss_Emperor_Dagran_Thaurissan
SD%Complete: 90
SDComment: With script for Moria
SDCategory: Blackrock Depths
EndScriptData */

#include "precompiled.h"
#include "blackrock_depths.h"

enum eEmperor
{
    FACTION_NEUTRAL             = 734,
    YELL_AGGRO_1                = -1230001,
    YELL_AGGRO_2                = -1230064,
    YELL_AGGRO_3                = -1230065,
    YELL_SLAY                   = -1230002,

    SPELL_HANDOFTHAURISSAN      = 17492,
    SPELL_AVATAROFFLAME         = 15636
};

struct boss_emperor_dagran_thaurissanAI : public ScriptedAI
{
    boss_emperor_dagran_thaurissanAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
        Reset();
    }

    ScriptedInstance* m_pInstance;

    uint32 m_uiHandOfThaurissanTimer;
    uint32 m_uiAvatarOfFlameTimer;

    void Reset() override
    {
        m_uiHandOfThaurissanTimer = 4000;
        m_uiAvatarOfFlameTimer    = 25000;
    }

    void Aggro(Unit* /*pWho*/) override
    {
        uint32 uiTextId;
        switch (urand(0, 2))
        {
            case 0: uiTextId = YELL_AGGRO_1; break;
            case 1: uiTextId = YELL_AGGRO_2; break;
            case 2: uiTextId = YELL_AGGRO_3; break;
        }
        DoScriptText(uiTextId, m_creature);
        m_creature->CallForHelp(VISIBLE_RANGE);
    }

    void JustDied(Unit* /*pVictim*/) override
    {
        if (!m_pInstance)
            return;

        if (Creature* pPrincess = m_pInstance->GetSingleCreatureFromStorage(NPC_PRINCESS))
        {
            // check if we didn't update the entry
            if (pPrincess->GetEntry() != NPC_PRINCESS)
                return;

            if (pPrincess->isAlive())
            {
                pPrincess->SetFactionTemporary(FACTION_NEUTRAL, TEMPFACTION_NONE);
                pPrincess->AI()->EnterEvadeMode();
            }
        }
    }

    void KilledUnit(Unit* /*pVictim*/) override
    {
        DoScriptText(YELL_SLAY, m_creature);
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        if (m_uiHandOfThaurissanTimer < uiDiff)
        {
            if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
            {
                if (DoCastSpellIfCan(pTarget, SPELL_HANDOFTHAURISSAN) == CAST_OK)
                    m_uiHandOfThaurissanTimer = urand(5, 10) * 1000;
            }
        }
        else
            m_uiHandOfThaurissanTimer -= uiDiff;

        // AvatarOfFlame_Timer
        if (m_uiAvatarOfFlameTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_AVATAROFFLAME) == CAST_OK)
                m_uiAvatarOfFlameTimer = 18000;
        }
        else
            m_uiAvatarOfFlameTimer -= uiDiff;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_emperor_dagran_thaurissan(Creature* pCreature)
{
    return new boss_emperor_dagran_thaurissanAI(pCreature);
}

/*######
## boss_moira_bronzebeard
######*/

enum ePrincess
{
    SPELL_HEAL                  = 15586,
    SPELL_RENEW                 = 10929,
    SPELL_SHIELD                = 10901,
    SPELL_MINDBLAST             = 15587,
    SPELL_SHADOWWORDPAIN        = 15654,
    SPELL_SMITE                 = 10934,
    SPELL_SHADOW_BOLT           = 15537,
    SPELL_OPEN_PORTAL           = 13912
};

struct boss_moira_bronzebeardAI : public ScriptedAI
{
    boss_moira_bronzebeardAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
        Reset();
    }

    ScriptedInstance* m_pInstance;

    uint32 m_uiHealTimer;
    uint32 m_uiMindBlastTimer;
    uint32 m_uiShadowWordPainTimer;
    uint32 m_uiSmiteTimer;

    void Reset() override
    {
        m_uiHealTimer           = 12000;                             // These times are probably wrong
        m_uiMindBlastTimer      = 16000;
        m_uiShadowWordPainTimer = 2000;
        m_uiSmiteTimer          = 8000;
    }

    void AttackStart(Unit* pWho) override
    {
        if (m_creature->Attack(pWho, false))
        {
            m_creature->AddThreat(pWho);
            m_creature->SetInCombatWith(pWho);
            pWho->SetInCombatWith(m_creature);

            m_creature->GetMotionMaster()->MoveChase(pWho, 25.0f);
        }
    }

    void JustReachedHome() override
    {
        if (m_pInstance)
        {
            if (Creature* pEmperor = m_pInstance->GetSingleCreatureFromStorage(NPC_EMPEROR))
            {
                // if evade, then check if he is alive. If not, start make portal
                if (!pEmperor->isAlive())
                    DoCastSpellIfCan(m_creature, SPELL_OPEN_PORTAL);
            }
        }
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        // Return since we have no target
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        // MindBlast_Timer
        if (m_uiMindBlastTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_MINDBLAST) == CAST_OK)
                m_uiMindBlastTimer = 14000;
        }
        else
            m_uiMindBlastTimer -= uiDiff;

        // ShadowWordPain_Timer
        if (m_uiShadowWordPainTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_SHADOWWORDPAIN) == CAST_OK)
                m_uiShadowWordPainTimer = 18000;
        }
        else
            m_uiShadowWordPainTimer -= uiDiff;

        // Smite_Timer
        if (m_uiSmiteTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_SMITE) == CAST_OK)
                m_uiSmiteTimer = 10000;
        }
        else
            m_uiSmiteTimer -= uiDiff;

        // Heal_Timer
        if (m_uiHealTimer < uiDiff)
        {
            if (Creature* pEmperor = m_pInstance->GetSingleCreatureFromStorage(NPC_EMPEROR))
            {
                if (pEmperor->isAlive() && pEmperor->GetHealthPercent() != 100.0f)
                {
                    if (DoCastSpellIfCan(pEmperor, SPELL_HEAL) == CAST_OK)
                        m_uiHealTimer = 10000;
                }
            }
        }
        else
            m_uiHealTimer -= uiDiff;

        // No meele?
    }
};

CreatureAI* GetAI_boss_moira_bronzebeard(Creature* pCreature)
{
    return new boss_moira_bronzebeardAI(pCreature);
}

void AddSC_boss_draganthaurissan()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "boss_emperor_dagran_thaurissan";
    pNewScript->GetAI = &GetAI_boss_emperor_dagran_thaurissan;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "boss_moira_bronzebeard";
    pNewScript->GetAI = &GetAI_boss_moira_bronzebeard;
    pNewScript->RegisterSelf();
}
