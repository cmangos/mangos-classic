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
SDName: Boss_Noxxion
SD%Complete: 100
SDComment:
SDCategory: Maraudon
EndScriptData */

#include "precompiled.h"

enum
{
    SPELL_TOXICVOLLEY           = 21687,
    SPELL_UPPERCUT              = 22916,
    SPELL_NOXXION_SPAWNS_AURA   = 21708,
    SPELL_NOXXION_SPAWNS_SUMMON = 21707,
};

struct boss_noxxionAI : public ScriptedAI
{
    boss_noxxionAI(Creature* pCreature) : ScriptedAI(pCreature) {Reset();}

    uint32 m_uiToxicVolleyTimer;
    uint32 m_uiUppercutTimer;
    uint32 m_uiSummonTimer;

    void Reset() override
    {
        m_uiToxicVolleyTimer   = 7000;
        m_uiUppercutTimer      = 16000;
        m_uiSummonTimer         = 19000;
    }

    void JustSummoned(Creature* pSummoned) override
    {
        if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
            pSummoned->AI()->AttackStart(pTarget);
    }

    void UpdateAI(const uint32 diff) override
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        if (m_uiToxicVolleyTimer < diff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_TOXICVOLLEY) == CAST_OK)
                m_uiToxicVolleyTimer = 9000;
        }
        else
            m_uiToxicVolleyTimer -= diff;

        if (m_uiUppercutTimer < diff)
        {
            if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_UPPERCUT) == CAST_OK)
                m_uiUppercutTimer = 12000;
        }
        else
            m_uiUppercutTimer -= diff;

        if (m_uiSummonTimer < diff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_NOXXION_SPAWNS_AURA) == CAST_OK)
                m_uiSummonTimer = 40000;
        }
        else
            m_uiSummonTimer -= diff;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_noxxion(Creature* pCreature)
{
    return new boss_noxxionAI(pCreature);
}

bool EffectAuraDummy_spell_aura_dummy_noxxion_spawns(const Aura* pAura, bool bApply)
{
    if (pAura->GetId() == SPELL_NOXXION_SPAWNS_AURA && pAura->GetEffIndex() == EFFECT_INDEX_0)
    {
        if (Creature* pTarget = (Creature*)pAura->GetTarget())
        {
            if (bApply)
            {
                pTarget->CastSpell(pTarget, SPELL_NOXXION_SPAWNS_SUMMON, true);
                pTarget->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            }
            else
                pTarget->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        }
    }
    return true;
}

void AddSC_boss_noxxion()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "boss_noxxion";
    pNewScript->GetAI = &GetAI_boss_noxxion;
    pNewScript->pEffectAuraDummy = &EffectAuraDummy_spell_aura_dummy_noxxion_spawns;
    pNewScript->RegisterSelf();
}
