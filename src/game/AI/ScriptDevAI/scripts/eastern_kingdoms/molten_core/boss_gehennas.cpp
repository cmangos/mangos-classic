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
SDName: Boss_Gehennas
SD%Complete: 100
SDComment: -
SDCategory: Molten Core
EndScriptData

*/

#include "AI/ScriptDevAI/include/sc_common.h"
#include "molten_core.h"
#include "AI/ScriptDevAI/base/CombatAI.h"

enum
{
    SPELL_GEHENNAS_CURSE        = 19716,
    SPELL_RAIN_OF_FIRE          = 19717,
    SPELL_SHADOW_BOLT_RANDOM    = 19729,
    SPELL_SHADOW_BOLT_TARGET    = 19728,
};

enum GehennasActions
{
    GEHENNAS_GEHENNAS_CURSE,
    GEHENNAS_RAIN_OF_FIRE,
    GEHENNAS_SHADOW_BOLT_RANDOM,
    GEHENNAS_SHADOW_BOLT_TARGET,
    GEHENNAS_ACTION_MAX,
};

struct boss_gehennasAI : public CombatAI
{
    boss_gehennasAI(Creature* creature) : CombatAI(creature, GEHENNAS_ACTION_MAX), m_instance(static_cast<ScriptedInstance*>(creature->GetInstanceData()))
    {
        AddCombatAction(GEHENNAS_GEHENNAS_CURSE, 5 * IN_MILLISECONDS, 10 * IN_MILLISECONDS);
        AddCombatAction(GEHENNAS_RAIN_OF_FIRE, 6 * IN_MILLISECONDS, 12 * IN_MILLISECONDS);
        AddCombatAction(GEHENNAS_SHADOW_BOLT_RANDOM, 3 * IN_MILLISECONDS, 6 * IN_MILLISECONDS);
        AddCombatAction(GEHENNAS_SHADOW_BOLT_TARGET, 3 * IN_MILLISECONDS, 6 * IN_MILLISECONDS);
        Reset();
    }

    ScriptedInstance* m_instance;

    void Aggro(Unit* /*who*/) override
    {
        if (m_instance)
            m_instance->SetData(TYPE_GEHENNAS, IN_PROGRESS);
    }

    void JustDied(Unit* /*killer*/) override
    {
        if (m_instance)
            m_instance->SetData(TYPE_GEHENNAS, DONE);
    }

    void JustReachedHome() override
    {
        if (m_instance)
            m_instance->SetData(TYPE_GEHENNAS, FAIL);
    }

    void ExecuteAction(uint32 action) override
    {
        switch (action)
        {
            case GEHENNAS_GEHENNAS_CURSE:
            {
                if (DoCastSpellIfCan(nullptr, SPELL_GEHENNAS_CURSE) == CAST_OK)
                    ResetCombatAction(action, urand(25 * IN_MILLISECONDS, 30 * IN_MILLISECONDS));
                break;
            }
            case GEHENNAS_RAIN_OF_FIRE:
            {
                if (Unit* target = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0, SPELL_RAIN_OF_FIRE, SELECT_FLAG_PLAYER))
                    if (DoCastSpellIfCan(target, SPELL_RAIN_OF_FIRE) == CAST_OK)
                        ResetCombatAction(action, urand(6 * IN_MILLISECONDS, 12 * IN_MILLISECONDS));
                break;
            }
            case GEHENNAS_SHADOW_BOLT_RANDOM:
            {
                if (Unit* target = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0, SPELL_SHADOW_BOLT_RANDOM, SELECT_FLAG_PLAYER))
                    if (DoCastSpellIfCan(target, SPELL_SHADOW_BOLT_RANDOM) == CAST_OK)
                        ResetCombatAction(action, urand(3 * IN_MILLISECONDS, 6 * IN_MILLISECONDS));
                break;
            }
            case GEHENNAS_SHADOW_BOLT_TARGET:
            {
                if (DoCastSpellIfCan(m_creature->GetVictim(), SPELL_SHADOW_BOLT_TARGET) == CAST_OK)
                    ResetCombatAction(action, urand(3 * IN_MILLISECONDS, 6 * IN_MILLISECONDS));
                break;
            }
        }
    }
};

void AddSC_boss_gehennas()
{
    Script* pNewScript = new Script;
    pNewScript->Name = "boss_gehennas";
    pNewScript->GetAI = &GetNewAIInstance<boss_gehennasAI>;
    pNewScript->RegisterSelf();
}
