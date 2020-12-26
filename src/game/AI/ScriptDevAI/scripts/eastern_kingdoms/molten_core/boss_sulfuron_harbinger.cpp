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
SDName: Boss_Sulfuron_Harbringer
SD%Complete: 80
SDComment: Spells Dark strike and Flamespear need confirmation
SDCategory: Molten Core
EndScriptData

*/

#include "AI/ScriptDevAI/include/sc_common.h"
#include "molten_core.h"
#include "AI/ScriptDevAI/base/CombatAI.h"

enum
{
    SPELL_DEMORALIZING_SHOUT    = 19778,
    SPELL_INSPIRE               = 19779,
    SPELL_HAND_OF_RAGNAROS      = 19780,
    SPELL_FLAMESPEAR            = 19781,

    // Adds Spells
    SPELL_DARK_STRIKE           = 19777,
    SPELL_HEAL                  = 19775,
    SPELL_SHADOWWORD_PAIN       = 19776,
    SPELL_IMMOLATE              = 20294
};

enum SulfuronActions
{
    SULFURON_DEMORALIZING_SHOUT,
    SULFURON_INSPIRE,
    SULFURON_HAND_OF_RAGNAROS,
    SULFURON_FLAME_SPEAR,
    SULFURON_ACTION_MAX,
};

struct boss_sulfuronAI : public CombatAI
{
    boss_sulfuronAI(Creature* creature) : CombatAI(creature, SULFURON_ACTION_MAX), m_instance(static_cast<ScriptedInstance*>(creature->GetInstanceData()))
    {
        AddCombatAction(SULFURON_DEMORALIZING_SHOUT, 15000u);
        AddCombatAction(SULFURON_INSPIRE, 3000u);
        AddCombatAction(SULFURON_HAND_OF_RAGNAROS, 6000u);
        AddCombatAction(SULFURON_FLAME_SPEAR, 2000u);
        Reset();
    }

    ScriptedInstance* m_instance;

    void Aggro(Unit* /*who*/) override
    {
        if (m_instance)
            m_instance->SetData(TYPE_SULFURON, IN_PROGRESS);
    }

    void JustDied(Unit* /*killer*/) override
    {
        if (m_instance)
            m_instance->SetData(TYPE_SULFURON, DONE);
    }

    void JustReachedHome() override
    {
        if (m_instance)
            m_instance->SetData(TYPE_SULFURON, FAIL);
    }

    void ExecuteAction(uint32 action) override
    {
        switch (action)
        {
            case SULFURON_DEMORALIZING_SHOUT:
            {
                if (DoCastSpellIfCan(nullptr, SPELL_DEMORALIZING_SHOUT) == CAST_OK)
                    ResetCombatAction(action, urand(15000, 20000));
                break;
            }
            case SULFURON_INSPIRE:
            {
                Creature* target = nullptr;
                CreatureList pList = DoFindFriendlyMissingBuff(45.0f, SPELL_INSPIRE, false);
                if (!pList.empty())
                {
                    CreatureList::iterator i = pList.begin();
                    advance(i, (rand() % pList.size()));
                    target = (*i);
                }

                if (!target)
                    target = m_creature;

                if (DoCastSpellIfCan(target, SPELL_INSPIRE) == CAST_OK)
                    ResetCombatAction(action, 10000);
                break;
            }
            case SULFURON_HAND_OF_RAGNAROS:
            {
                if (DoCastSpellIfCan(nullptr, SPELL_HAND_OF_RAGNAROS) == CAST_OK)
                    ResetCombatAction(action, urand(12000, 15000));
                break;
            }
            case SULFURON_FLAME_SPEAR:
            {
                if (Unit* target = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0, SPELL_SHADOWWORD_PAIN, SELECT_FLAG_PLAYER))
                    if (DoCastSpellIfCan(target, SPELL_FLAMESPEAR) == CAST_OK)
                        ResetCombatAction(action, urand(12000, 16000));
                break;
            }
        }
    }
};

enum FlamewakerPriestActions
{
    FLAMEWAKER_PRIEST_DARK_STRIKE,
    FLAMEWAKER_PRIEST_DARK_MENDING,
    FLAMEWAKER_PRIEST_SHADOW_WORD_PAIN,
    FLAMEWAKER_PRIEST_IMMOLATE,
    FLAMEWAKER_PRIEST_ACTION_MAX,
};

struct mob_flamewaker_priestAI : public CombatAI
{
    mob_flamewaker_priestAI(Creature* creature) : CombatAI(creature, FLAMEWAKER_PRIEST_ACTION_MAX), m_instance(static_cast<ScriptedInstance*>(creature->GetInstanceData()))
    {
        AddCombatAction(FLAMEWAKER_PRIEST_DARK_STRIKE, 10000u);
        AddCombatAction(FLAMEWAKER_PRIEST_DARK_MENDING, 15000, 30000);
        AddCombatAction(FLAMEWAKER_PRIEST_SHADOW_WORD_PAIN, 2000u);
        AddCombatAction(FLAMEWAKER_PRIEST_IMMOLATE, 8000u);
    }

    ScriptedInstance* m_instance;

    void ExecuteAction(uint32 action) override
    {
        switch (action)
        {
            case FLAMEWAKER_PRIEST_DARK_STRIKE:
            {
                if (DoCastSpellIfCan(m_creature->GetVictim(), SPELL_DARK_STRIKE) == CAST_OK)
                    ResetCombatAction(action, urand(15000, 18000));
                break;
            }
            case FLAMEWAKER_PRIEST_DARK_MENDING:
            {
                if (Unit* target = DoSelectLowestHpFriendly(60.0f, 1))
                    if (DoCastSpellIfCan(target, SPELL_HEAL) == CAST_OK)
                        ResetCombatAction(action, urand(15000, 20000));
                break;
            }
            case FLAMEWAKER_PRIEST_SHADOW_WORD_PAIN:
            {
                if (Unit* target = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0, SPELL_SHADOWWORD_PAIN, SELECT_FLAG_PLAYER))
                    if (DoCastSpellIfCan(target, SPELL_SHADOWWORD_PAIN) == CAST_OK)
                        ResetCombatAction(action, urand(18000, 26000));
                break;
            }
            case FLAMEWAKER_PRIEST_IMMOLATE:
            {
                if (Unit* target = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0, SPELL_SHADOWWORD_PAIN, SELECT_FLAG_PLAYER))
                    if (DoCastSpellIfCan(target, SPELL_IMMOLATE) == CAST_OK)
                        ResetCombatAction(action, urand(15000, 25000));
                break;
            }
        }
    }
};

void AddSC_boss_sulfuron()
{
    Script* pNewScript = new Script;
    pNewScript->Name = "boss_sulfuron";
    pNewScript->GetAI = &GetNewAIInstance<boss_sulfuronAI>;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "mob_flamewaker_priest";
    pNewScript->GetAI = &GetNewAIInstance<mob_flamewaker_priestAI>;
    pNewScript->RegisterSelf();
}
