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
SDName: Boss_Shazzrah
SD%Complete: 75
SDCategory: Molten Core
EndScriptData

*/

#include "AI/ScriptDevAI/include/sc_common.h"
#include "molten_core.h"
#include "AI/ScriptDevAI/base/CombatAI.h"

enum
{
    SPELL_ARCANE_EXPLOSION          = 19712,
    SPELL_SHAZZRAH_CURSE            = 19713,
    SPELL_MAGIC_GROUNDING           = 19714,
    SPELL_COUNTERSPELL              = 19715,
    SPELL_GATE_OF_SHAZZRAH          = 23138                 // effect spell: 23139
};

enum ShazzrahActions
{
    SHAZZRAH_ARCANE_EXPLOSION,
    SHAZZRAH_SHAZZRAH_CURSE,
    SHAZZRAH_COUNTERSPELL,
    SHAZZRAH_MAGIC_GROUNDING,
    SHAZZRAH_GATE_OF_SHAZZRAH,
    SHAZZRAH_ACTION_MAX,
};

struct boss_shazzrahAI : public CombatAI
{
    boss_shazzrahAI(Creature* creature) : CombatAI(creature, SHAZZRAH_ACTION_MAX), m_instance(static_cast<ScriptedInstance*>(creature->GetInstanceData()))
    {
        AddCombatAction(SHAZZRAH_ARCANE_EXPLOSION, 6000u);
        AddCombatAction(SHAZZRAH_SHAZZRAH_CURSE, 10000u);
        AddCombatAction(SHAZZRAH_COUNTERSPELL, 15000u);
        AddCombatAction(SHAZZRAH_MAGIC_GROUNDING, 24000u);
        AddCombatAction(SHAZZRAH_GATE_OF_SHAZZRAH, 30000u);
        Reset();
    }

    ScriptedInstance* m_instance;

    void Aggro(Unit* /*who*/) override
    {
        if (m_instance)
            m_instance->SetData(TYPE_SHAZZRAH, IN_PROGRESS);
    }

    void JustDied(Unit* /*killer*/) override
    {
        if (m_instance)
            m_instance->SetData(TYPE_SHAZZRAH, DONE);
    }

    void JustReachedHome() override
    {
        if (m_instance)
            m_instance->SetData(TYPE_SHAZZRAH, NOT_STARTED);
    }

    void ReceiveAIEvent(AIEventType eventType, Unit* /*sender*/, Unit* /*invoker*/, uint32 /*miscValue*/) override
    {
        if (eventType == AI_EVENT_CUSTOM_A) // succesful teleport
        {
            DoResetThreat();
            DoCastSpellIfCan(nullptr, SPELL_ARCANE_EXPLOSION);
        }
    }

    void ExecuteAction(uint32 action) override
    {
        switch (action)
        {
            case SHAZZRAH_ARCANE_EXPLOSION:
            {
                if (DoCastSpellIfCan(nullptr, SPELL_ARCANE_EXPLOSION) == CAST_OK)
                    ResetCombatAction(action, urand(5000, 9000));
                break;
            }
            case SHAZZRAH_SHAZZRAH_CURSE:
            {
                if (DoCastSpellIfCan(nullptr, SPELL_SHAZZRAH_CURSE) == CAST_OK)
                    ResetCombatAction(action, 20000);
                break;
            }
            case SHAZZRAH_COUNTERSPELL:
            {
                if (DoCastSpellIfCan(nullptr, SPELL_COUNTERSPELL) == CAST_OK)
                    ResetCombatAction(action, urand(16000, 20000));
                break;
            }
            case SHAZZRAH_MAGIC_GROUNDING:
            {
                if (DoCastSpellIfCan(nullptr, SPELL_MAGIC_GROUNDING) == CAST_OK)
                    ResetCombatAction(action, 35000);
                break;
            }
            case SHAZZRAH_GATE_OF_SHAZZRAH:
            {
                // Teleporting him to a random target and casting Arcane Explosion after that.
                if (DoCastSpellIfCan(nullptr, SPELL_GATE_OF_SHAZZRAH) == CAST_OK)
                    ResetCombatAction(action, 45000);
                break;
            }
        }
    }
};

void AddSC_boss_shazzrah()
{
    Script* pNewScript = new Script;
    pNewScript->Name = "boss_shazzrah";
    pNewScript->GetAI = &GetNewAIInstance<boss_shazzrahAI>;
    pNewScript->RegisterSelf();
}
