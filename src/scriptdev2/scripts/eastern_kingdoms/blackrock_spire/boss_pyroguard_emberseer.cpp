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
SDName: Boss_Pyroguard_Emberseer
SD%Complete: 90
SDComment: Dummy spells used during the transformation may need further research
SDCategory: Blackrock Spire
EndScriptData */

#include "precompiled.h"
#include "blackrock_spire.h"

enum
{
    // Intro emote/say
    EMOTE_NEAR              = -1229001,
    EMOTE_FULL              = -1229002,
    SAY_FREE                = -1229003,

    MAX_GROWING_STACKS      = 20,

    // Intro spells
    SPELL_ENCAGE_EMBERSEER  = 15281,                        // cast by Blackhand Incarcerator

    SPELL_FIRE_SHIELD       = 13376,                        // not sure what's the purpose of this
    SPELL_DESPAWN_EMBERSEER = 16078,                        // not sure what's the purpose of this
    SPELL_FREEZE_ANIM       = 16245,                        // not sure what's the purpose of this
    SPELL_FULL_STRENGHT     = 16047,
    SPELL_GROWING           = 16049,                        // stacking aura
    SPELL_BONUS_DAMAGE      = 16534,                        // triggered on full grow
    SPELL_TRANSFORM         = 16052,

    // Combat spells
    SPELL_FIRENOVA          = 23462,
    SPELL_FLAMEBUFFET       = 23341,
    SPELL_PYROBLAST         = 20228                         // guesswork, but best fitting in spells-area, was 17274 (has mana cost)
};

struct boss_pyroguard_emberseerAI : public ScriptedAI
{
    boss_pyroguard_emberseerAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = (instance_blackrock_spire*) pCreature->GetInstanceData();
        Reset();
    }

    instance_blackrock_spire* m_pInstance;

    uint32 m_uiEncageTimer;
    uint32 m_uiFireNovaTimer;
    uint32 m_uiFlameBuffetTimer;
    uint32 m_uiPyroBlastTimer;
    uint8 m_uiGrowingStacks;

    void Reset() override
    {
        m_uiEncageTimer         = 10000;
        m_uiFireNovaTimer       = 6000;
        m_uiFlameBuffetTimer    = 3000;
        m_uiPyroBlastTimer      = 14000;
        m_uiGrowingStacks       = 0;

        m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
    }

    void JustDied(Unit* /*pKiller*/) override
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_EMBERSEER, DONE);
    }

    void JustReachedHome() override
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_EMBERSEER, FAIL);
    }

    // Wrapper to handle the transformation
    void DoHandleEmberseerGrowing()
    {
        ++m_uiGrowingStacks;

        if (m_uiGrowingStacks == MAX_GROWING_STACKS * 0.5f)
            DoScriptText(EMOTE_NEAR, m_creature);
        else if (m_uiGrowingStacks == MAX_GROWING_STACKS)
        {
            DoScriptText(EMOTE_FULL, m_creature);
            DoScriptText(SAY_FREE, m_creature);

            // Note: the spell order needs further research
            DoCastSpellIfCan(m_creature, SPELL_FULL_STRENGHT, CAST_TRIGGERED);
            DoCastSpellIfCan(m_creature, SPELL_BONUS_DAMAGE, CAST_TRIGGERED);
            DoCastSpellIfCan(m_creature, SPELL_TRANSFORM, CAST_TRIGGERED);

            // activate all runes
            if (m_pInstance)
            {
                m_pInstance->DoUseEmberseerRunes();
                // Redundant check: if for some reason the event isn't set in progress until this point - avoid using the altar again when the boss is fully grown
                m_pInstance->SetData(TYPE_EMBERSEER, IN_PROGRESS);
            }

            m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        }
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        // Cast Encage spell on OOC timer
        if (m_uiEncageTimer)
        {
            if (m_uiEncageTimer <= uiDiff)
            {
                if (!m_pInstance)
                {
                    script_error_log("Instance Blackrock Spire: ERROR Failed to load instance data for this instace.");
                    return;
                }

                GuidList m_lIncarceratorsGuid;
                m_pInstance->GetIncarceratorGUIDList(m_lIncarceratorsGuid);

                for (GuidList::const_iterator itr = m_lIncarceratorsGuid.begin(); itr != m_lIncarceratorsGuid.end(); ++itr)
                {
                    if (Creature* pIncarcerator = m_creature->GetMap()->GetCreature(*itr))
                        pIncarcerator->CastSpell(m_creature, SPELL_ENCAGE_EMBERSEER, false);
                }

                m_uiEncageTimer = 0;
            }
            else
                m_uiEncageTimer -= uiDiff;
        }

        // Return since we have no target
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        // FireNova Timer
        if (m_uiFireNovaTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_FIRENOVA) == CAST_OK)
                m_uiFireNovaTimer = 6000;
        }
        else
            m_uiFireNovaTimer -= uiDiff;

        // FlameBuffet Timer
        if (m_uiFlameBuffetTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_FLAMEBUFFET) == CAST_OK)
                m_uiFlameBuffetTimer = 14000;
        }
        else
            m_uiFlameBuffetTimer -= uiDiff;

        // PyroBlast Timer
        if (m_uiPyroBlastTimer < uiDiff)
        {
            if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
            {
                if (DoCastSpellIfCan(pTarget, SPELL_PYROBLAST) == CAST_OK)
                    m_uiPyroBlastTimer = 15000;
            }
        }
        else
            m_uiPyroBlastTimer -= uiDiff;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_pyroguard_emberseer(Creature* pCreature)
{
    return new boss_pyroguard_emberseerAI(pCreature);
}

bool EffectDummyCreature_pyroguard_emberseer(Unit* /*pCaster*/, uint32 uiSpellId, SpellEffectIndex uiEffIndex, Creature* pCreatureTarget, ObjectGuid /*originalCasterGuid*/)
{
    // always check spellid and effectindex
    if (uiSpellId == SPELL_GROWING && uiEffIndex == EFFECT_INDEX_0)
    {
        if (boss_pyroguard_emberseerAI* pEmberseerAI = dynamic_cast<boss_pyroguard_emberseerAI*>(pCreatureTarget->AI()))
            pEmberseerAI->DoHandleEmberseerGrowing();
    }

    return false;
}

void AddSC_boss_pyroguard_emberseer()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "boss_pyroguard_emberseer";
    pNewScript->GetAI = &GetAI_boss_pyroguard_emberseer;
    pNewScript->pEffectDummyNPC = &EffectDummyCreature_pyroguard_emberseer;
    pNewScript->RegisterSelf();
}
