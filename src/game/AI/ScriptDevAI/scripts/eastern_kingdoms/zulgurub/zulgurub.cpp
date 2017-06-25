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
EndContentData */

enum
{
    SPELL_DISPEL_MAGIC           = 17201,
    SPELL_BLOOD_FUNNEL           = 24617,
    SPELL_DRAIN_LIFE             = 24618,
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

CreatureAI* GetAI_npc_hakkari_blood_priest(Creature* pCreature)
{
    return new npc_hakkari_blood_priestAI(pCreature);
}

void AddSC_zulgurub()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "npc_hakkari_blood_priest";
    pNewScript->GetAI = &GetAI_npc_hakkari_blood_priest;
    pNewScript->RegisterSelf();
}
