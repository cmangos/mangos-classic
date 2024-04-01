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
SDName: Boss_Magmadar
SD%Complete: 99
SDComment: Lava bomb timers may need some adjustements
SDCategory: Molten Core
EndScriptData

*/

#include "AI/ScriptDevAI/include/sc_common.h"
#include "molten_core.h"
#include "AI/ScriptDevAI/base/CombatAI.h"

enum
{
    EMOTE_GENERIC_FRENZY_KILL   = -1000001,

    // SPELL_LAVA_BREATH           = 19272,                    // Triggered by SPELL_FRENZY (19451)
    SPELL_FRENZY                = 19451,
    SPELL_MAGMASPIT             = 19449,                    // This is actually a buff he gives himself
    SPELL_PANIC                 = 19408,
    SPELL_LAVABOMB              = 19411,                    // This calls a dummy server side effect that cast spell 20494 to spawn GO 177704 for 30s
    SPELL_LAVABOMB_MANA         = 20474,                    // This calls a dummy server side effect that cast spell 20495 to spawn GO 177704 for 60s
    // SPELL_CONFLAGRATION         = 19428                     // Trap spell for GO 177704
};

enum MagmadarActions
{
    MAGMADAR_FRENZY,
    MAGMADAR_PANIC,
    MAGMADAR_LAVABOMB,
    MAGMADAR_LAVABOMB_MANA,
    MAGMADAR_ACTION_MAX,
};

struct boss_magmadarAI : public CombatAI
{
    boss_magmadarAI(Creature* creature) : CombatAI(creature, MAGMADAR_ACTION_MAX), m_instance(static_cast<ScriptedInstance*>(creature->GetInstanceData()))
    {
        AddCombatAction(MAGMADAR_FRENZY, 30000u);
        AddCombatAction(MAGMADAR_PANIC, 6000, 10000);
        AddCombatAction(MAGMADAR_LAVABOMB, 12000u);
        AddCombatAction(MAGMADAR_LAVABOMB_MANA, 18000u);
        Reset();
    }

    ScriptedInstance* m_instance;

    void Reset() override
    {
        CombatAI::Reset();
        DoCastSpellIfCan(nullptr, SPELL_MAGMASPIT, CAST_TRIGGERED | CAST_AURA_NOT_PRESENT);
    }

    void Aggro(Unit* /*who*/) override
    {
        if (m_instance)
            m_instance->SetData(TYPE_MAGMADAR, IN_PROGRESS);
    }

    void JustDied(Unit* /*killer*/) override
    {
        if (m_instance)
            m_instance->SetData(TYPE_MAGMADAR, DONE);
    }

    void JustReachedHome() override
    {
        if (m_instance)
            m_instance->SetData(TYPE_MAGMADAR, NOT_STARTED);
    }

    void ExecuteAction(uint32 action) override
    {
        switch (action)
        {
            case MAGMADAR_FRENZY:
            {
                if (DoCastSpellIfCan(nullptr, SPELL_FRENZY) == CAST_OK)
                {
                    DoScriptText(EMOTE_GENERIC_FRENZY_KILL, m_creature);
                    ResetCombatAction(action, urand(15000, 20000));
                }
                break;
            }
            case MAGMADAR_PANIC:
            {
                if (DoCastSpellIfCan(nullptr, SPELL_PANIC) == CAST_OK)
                    ResetCombatAction(action, urand(30000, 35000));
                break;
            }
            case MAGMADAR_LAVABOMB:
            {
                if (Unit* target = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0, SPELL_LAVABOMB, SELECT_FLAG_POWER_NOT_MANA))
                    if (DoCastSpellIfCan(target, SPELL_LAVABOMB) == CAST_OK)
                        ResetCombatAction(action, urand(12000, 15000));
                break;
            }
            case MAGMADAR_LAVABOMB_MANA:
            {
                if (Unit* target = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0, SPELL_LAVABOMB_MANA, SELECT_FLAG_POWER_MANA))
                    if (DoCastSpellIfCan(target, SPELL_LAVABOMB_MANA) == CAST_OK)
                        ResetCombatAction(action, urand(12000, 15000));
                break;
            }
        }
    }
};

void AddSC_boss_magmadar()
{
    Script* pNewScript = new Script;
    pNewScript->Name = "boss_magmadar";
    pNewScript->GetAI = &GetNewAIInstance<boss_magmadarAI>;
    pNewScript->RegisterSelf();
}
