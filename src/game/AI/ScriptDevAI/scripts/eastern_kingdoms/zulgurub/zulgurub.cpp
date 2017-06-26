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
SDName: Zul'Gurub
SD%Complete: 90
SDComment: Blood Priest need to do quest visual (24258)
SDCategory: Zul'Gurub
EndScriptData */

#include "AI/ScriptDevAI/PreCompiledHeader.h"

 /* ContentData
npc_hakkari_blood_priest
npc_zulian_panther
EndContentData */

enum
{
    SPELL_DISPEL_MAGIC           = 17201,
    SPELL_BLOOD_FUNNEL           = 24617,
    SPELL_DRAIN_LIFE             = 24618,

    SPELL_EXPLOIT_WEAKNESS       = 8355,
    SPELL_RAKE                   = 24332,
    SPELL_RAVAGE                 = 24333
};

/*######
## npc_hakkari_blood_priest
######*/

struct npc_hakkari_blood_priestAI : public ScriptedAI
{
    npc_hakkari_blood_priestAI(Creature* pCreature) : ScriptedAI(pCreature) {Reset();}

    uint32 m_uiDrainTimer;
    uint32 m_uiDispelTimer;
    uint32 m_uiFunnelTimer;

    void Reset() override
    {
        m_uiDrainTimer  = 0;
        m_uiDispelTimer = 0;
        m_uiFunnelTimer = 3000;
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        // when we reach 40% of HP start draining life
        if (m_uiDrainTimer < uiDiff)
        {
            if (m_creature->GetHealthPercent() < 40.0f)
                if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_FARTHEST_AWAY, 0, SPELL_DRAIN_LIFE, SELECT_FLAG_NOT_IN_MELEE_RANGE))
                    if (DoCastSpellIfCan(pTarget, SPELL_DRAIN_LIFE) == CAST_OK)
                        m_uiDrainTimer = (6000, 8000);
        }
        else
            m_uiDrainTimer -= uiDiff;

        if (m_uiDispelTimer < uiDiff)
        {
            std::list<Creature*> pList = DoFindFriendlyCC(30.0f);

            Creature* dispelTarget = nullptr;

            if (!pList.empty())
                for (std::list<Creature*>::iterator itr=pList.begin(); itr != pList.end(); ++itr)
                {
                    dispelTarget = (*itr);
                    break;
                }

            if (!dispelTarget && (m_creature->isInRoots() || m_creature->GetMaxNegativeAuraModifier(SPELL_AURA_MOD_DECREASE_SPEED)))
                dispelTarget = m_creature;

            if (dispelTarget && (DoCastSpellIfCan(dispelTarget, SPELL_DISPEL_MAGIC) == CAST_OK))
                m_uiDispelTimer = urand(15000, 20000);
        }
        else
            m_uiDispelTimer -= uiDiff;

        // Blood Funnel
        if (m_uiFunnelTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_BLOOD_FUNNEL) == CAST_OK)
                m_uiFunnelTimer = urand(4000, 6000);
        }
        else
            m_uiFunnelTimer -= uiDiff;

        if (m_creature->IsNonMeleeSpellCasted(false))
            return;

        DoMeleeAttackIfReady();
    }
};

/*######
## npc_zulian_panther
######*/

struct npc_zulian_pantherAI : public ScriptedAI 
{
    npc_zulian_pantherAI(Creature* pCreature) : ScriptedAI(pCreature) { Reset(); }

    uint32 m_uiExploitWeaknessTimer;
    uint32 m_uiRakeTimer;
    uint32 m_uiRavageTimer;

    void Reset() override
    {
        m_uiExploitWeaknessTimer = 0;
        m_uiRakeTimer = 0;
        m_uiRavageTimer = 3000;
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        // Exploit Weakness when behind an enemy
        if (m_uiExploitWeaknessTimer < uiDiff)
        {
            if (m_creature->IsFacingTargetsBack(m_creature->getVictim()))
            {
                if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_EXPLOIT_WEAKNESS) == CAST_OK)
                    m_uiExploitWeaknessTimer = urand(5000, 12000);
            }
        }
        else
            m_uiExploitWeaknessTimer -= uiDiff;

        // Rake
        if (m_uiRakeTimer < uiDiff)
        {
            if (!m_creature->getVictim()->HasAura(SPELL_RAKE)) 
            {
                if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_RAKE) == CAST_OK)
                    m_uiRakeTimer = urand(1000, 3000);
            }
        }
        else
            m_uiRakeTimer -= uiDiff;

        // Ravage
        if (m_uiRavageTimer < uiDiff)
        {
            if (!m_creature->getVictim()->HasAura(SPELL_RAVAGE))
            {
                if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_RAVAGE) == CAST_OK)
                    m_uiRavageTimer = urand(15000, 20000);
            }
        }
        else
            m_uiRavageTimer -= uiDiff;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_hakkari_blood_priest(Creature* pCreature)
{
    return new npc_hakkari_blood_priestAI(pCreature);
}

CreatureAI* GetAI_npc_zulian_panther(Creature* pCreature)
{
    return new npc_zulian_pantherAI(pCreature);
}

void AddSC_zulgurub()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "npc_hakkari_blood_priest";
    pNewScript->GetAI = &GetAI_npc_hakkari_blood_priest;
    pNewScript->RegisterSelf();

    pNewScript = new Script();
    pNewScript->Name = "npc_zulian_panther";
    pNewScript->GetAI = &GetAI_npc_zulian_panther;
    pNewScript->RegisterSelf();
}
