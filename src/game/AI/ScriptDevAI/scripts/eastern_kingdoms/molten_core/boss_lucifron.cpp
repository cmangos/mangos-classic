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
SDName: Boss_Lucifron
SD%Complete: 100
SDComment:
SDCategory: Molten Core
EndScriptData

*/

#include "AI/ScriptDevAI/include/sc_common.h"
#include "molten_core.h"
#include "AI/ScriptDevAI/base/CombatAI.h"

enum
{
    SPELL_SHADOWSHOCK       = 19460,
    SPELL_IMPENDINGDOOM     = 19702,
    SPELL_LUCIFRONCURSE     = 19703,
};

enum LucifronActions
{
    LUCIFRON_IMPENDING_DOOM,
    LUCIFRON_LUCIFRONS_CURSE,
    LUCIFRON_SHADOWSHOCK,
    LUCIFRON_ACTION_MAX,
};

struct boss_lucifronAI : public CombatAI
{
    boss_lucifronAI(Creature* creature) : CombatAI(creature, LUCIFRON_ACTION_MAX), m_instance(static_cast<ScriptedInstance*>(creature->GetInstanceData()))
    {
        AddCombatAction(LUCIFRON_IMPENDING_DOOM, 5 * IN_MILLISECONDS, 10 * IN_MILLISECONDS);
        AddCombatAction(LUCIFRON_LUCIFRONS_CURSE, 10 * IN_MILLISECONDS, 15 * IN_MILLISECONDS);
        AddCombatAction(LUCIFRON_SHADOWSHOCK, 3 * IN_MILLISECONDS, 6 * IN_MILLISECONDS);
        Reset();
    }

    ScriptedInstance* m_instance;

    void Aggro(Unit* /*who*/) override
    {
        if (m_instance)
            m_instance->SetData(TYPE_LUCIFRON, IN_PROGRESS);
    }

    void JustDied(Unit* /*killer*/) override
    {
        if (m_instance)
            m_instance->SetData(TYPE_LUCIFRON, DONE);
    }

    void JustReachedHome() override
    {
        if (m_instance)
            m_instance->SetData(TYPE_LUCIFRON, FAIL);
    }

    void ExecuteAction(uint32 action) override
    {
        switch (action)
        {
            case LUCIFRON_IMPENDING_DOOM:
            {
                if (DoCastSpellIfCan(nullptr, SPELL_IMPENDINGDOOM) == CAST_OK)
                    ResetCombatAction(action, urand(20 * IN_MILLISECONDS, 25 * IN_MILLISECONDS));
                break;
            }
            case LUCIFRON_LUCIFRONS_CURSE:
            {
                if (DoCastSpellIfCan(nullptr, SPELL_LUCIFRONCURSE) == CAST_OK)
                    ResetCombatAction(action, urand(20 * IN_MILLISECONDS, 25 * IN_MILLISECONDS));
                break;
            }
            case LUCIFRON_SHADOWSHOCK:
            {
                if (DoCastSpellIfCan(m_creature->GetVictim(), SPELL_SHADOWSHOCK) == CAST_OK)
                    ResetCombatAction(action, urand(3 * IN_MILLISECONDS, 6 * IN_MILLISECONDS));
                break;
            }
        }
    }
};

void AddSC_boss_lucifron()
{
    Script* pNewScript = new Script;
    pNewScript->Name = "boss_lucifron";
    pNewScript->GetAI = &GetNewAIInstance<boss_lucifronAI>;
    pNewScript->RegisterSelf();
}
