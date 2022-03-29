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
SDName: Boss_Golemagg
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
    SPELL_MAGMA_SPLASH      = 13879,
    SPELL_PYROBLAST         = 20228,
    SPELL_EARTHQUAKE        = 19798,
    // SPELL_ENRAGE            = 19953, // unconfirmed
    SPELL_GOLEMAGG_TRUST    = 20556,
    SPELL_DOUBLE_ATTACK     = 18943,

    // Core Rager
    EMOTE_LOW_HP            = -1409002,
    SPELL_QUIET_SUICIDE     = 3617,
    SPELL_THRASH            = 12787,
    SPELL_FULL_HEAL         = 17683,
    SPELL_MANGLE            = 19820
};

enum GolemaggActions
{
    GOLEMAGG_ENRAGE,
    GOLEMAGG_PYROBLAST,
    GOLEMAGG_EARTHQUAKE,
    GOLEMAGG_ACTION_MAX,
};

struct boss_golemaggAI : public CombatAI
{
    boss_golemaggAI(Creature* creature) : CombatAI(creature, GOLEMAGG_ACTION_MAX), m_instance(static_cast<ScriptedInstance*>(creature->GetInstanceData()))
    {
        AddTimerlessCombatAction(GOLEMAGG_ENRAGE, true);
        AddCombatAction(GOLEMAGG_PYROBLAST, uint32(7 * IN_MILLISECONDS));
        AddCombatAction(GOLEMAGG_EARTHQUAKE, true);
    }

    ScriptedInstance* m_instance;

    void Reset() override
    {
        CombatAI::Reset();

        DoCastSpellIfCan(nullptr, SPELL_MAGMA_SPLASH, CAST_TRIGGERED | CAST_AURA_NOT_PRESENT);
        DoCastSpellIfCan(nullptr, SPELL_DOUBLE_ATTACK, CAST_TRIGGERED | CAST_AURA_NOT_PRESENT);
        DoCastSpellIfCan(nullptr, SPELL_GOLEMAGG_TRUST, CAST_TRIGGERED | CAST_AURA_NOT_PRESENT);
    }

    void Aggro(Unit* /*who*/) override
    {
        if (m_instance)
            m_instance->SetData(TYPE_GOLEMAGG, IN_PROGRESS);
    }

    void JustDied(Unit* /*killer*/) override
    {
        // Send event to the Core Ragers so they know that Golemagg is dead and that they must go suicide
        CreatureList lCoreRagerList;
        GetCreatureListWithEntryInGrid(lCoreRagerList, m_creature, NPC_CORE_RAGER, 100.0f);
        for (auto& itr : lCoreRagerList)
        {
            if (itr->IsAlive())
                m_creature->AI()->SendAIEvent(AI_EVENT_CUSTOM_A, m_creature, itr);
        }
        if (m_instance)
            m_instance->SetData(TYPE_GOLEMAGG, DONE);
    }

    void JustReachedHome() override
    {
        if (m_instance)
            m_instance->SetData(TYPE_GOLEMAGG, FAIL);
    }

    void ExecuteAction(uint32 action) override
    {
        switch (action)
        {
            case GOLEMAGG_ENRAGE:
            {
                if (m_creature->GetHealthPercent() > 10.0f)
                    return;

                //if (DoCastSpellIfCan(nullptr, SPELL_ENRAGE) == CAST_OK)
                //{
                SetActionReadyStatus(action, false);
                ResetCombatAction(GOLEMAGG_EARTHQUAKE, 0u);
                //}
                break;
            }
            case GOLEMAGG_PYROBLAST:
            {
                if (Unit* target = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0, SPELL_PYROBLAST, SELECT_FLAG_PLAYER))
                    if (DoCastSpellIfCan(target, SPELL_PYROBLAST) == CAST_OK)
                        ResetCombatAction(action, 7 * IN_MILLISECONDS);
                break;
            }
            case GOLEMAGG_EARTHQUAKE:
            {
                if (DoCastSpellIfCan(nullptr, SPELL_EARTHQUAKE) == CAST_OK)
                    ResetCombatAction(action, 3 * IN_MILLISECONDS);
                break;
            }
        }
    }
};

enum CoreRagerActions
{
    CORE_RAGER_HEAL,
    CORE_RAGER_MANGLE,
    CORE_RAGER_ACTION_MAX,
};

struct mob_core_ragerAI : public CombatAI
{
    mob_core_ragerAI(Creature* creature) : CombatAI(creature, CORE_RAGER_ACTION_MAX), m_instance(static_cast<ScriptedInstance*>(creature->GetInstanceData()))
    {
        AddTimerlessCombatAction(CORE_RAGER_HEAL, true);
        AddCombatAction(CORE_RAGER_MANGLE, uint32(7 * IN_MILLISECONDS)); // These times are probably wrong
        SetDeathPrevention(true);
    }

    ScriptedInstance* m_instance;

    void Reset() override
    {
        CombatAI::Reset();
        DoCastSpellIfCan(nullptr, SPELL_THRASH, CAST_TRIGGERED | CAST_AURA_NOT_PRESENT);
    }
    
    void ReceiveAIEvent(AIEventType eventType, Unit* sender, Unit* /*invoker*/, uint32 /*miscValue*/) override
    {
        // Event sent by Golemagg at the time of his death so Core Rager knows he can self-suicide
        if (sender->GetEntry() == NPC_GOLEMAGG && eventType == AI_EVENT_CUSTOM_A)
            DoCastSpellIfCan(nullptr, SPELL_QUIET_SUICIDE, CAST_TRIGGERED);
    }

    void ExecuteAction(uint32 action) override
    {
        switch (action)
        {
            case CORE_RAGER_HEAL:
            {
                if (m_creature->GetHealthPercent() > 50.0f)
                    return;

                if (DoCastSpellIfCan(nullptr, SPELL_FULL_HEAL) == CAST_OK)
                    DoScriptText(EMOTE_LOW_HP, m_creature);
                break;
            }
            case CORE_RAGER_MANGLE:
            {
                if (DoCastSpellIfCan(m_creature->GetVictim(), SPELL_MANGLE) == CAST_OK)
                    ResetCombatAction(action, 10 * IN_MILLISECONDS);
                break;
            }
        }
    }
};

void AddSC_boss_golemagg()
{
    Script* pNewScript = new Script;
    pNewScript->Name = "boss_golemagg";
    pNewScript->GetAI = &GetNewAIInstance<boss_golemaggAI>;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "mob_core_rager";
    pNewScript->GetAI = &GetNewAIInstance<mob_core_ragerAI>;
    pNewScript->RegisterSelf();
}
