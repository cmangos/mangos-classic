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
SDName: Boss_Ebonroc
SD%Complete: 100
SDComment:
SDCategory: Blackwing Lair
EndScriptData

*/

#include "AI/ScriptDevAI/include/sc_common.h"
#include "blackwing_lair.h"
#include "AI/ScriptDevAI/base/CombatAI.h"

enum
{
    SPELL_SHADOW_FLAME          = 22539,
    SPELL_WING_BUFFET           = 23339,
    SPELL_SHADOW_OF_EBONROC     = 23340,
    SPELL_THRASH                = 3391,
};

enum EbonrocActions
{
    EBONROC_SHADOW_OF_EBONROC,
    EBONROC_SHADOW_FLAME,
    EBONROC_WING_BUFFET,
    EBONROC_THRASH,
    EBONROC_ACTION_MAX,
};

struct boss_ebonrocAI : public CombatAI
{
    boss_ebonrocAI(Creature* creature) : CombatAI(creature, EBONROC_ACTION_MAX), m_instance(static_cast<ScriptedInstance*>(creature->GetInstanceData()))
    {
        AddCombatAction(EBONROC_SHADOW_OF_EBONROC, 45000u);
        AddCombatAction(EBONROC_SHADOW_FLAME, uint32(18 * IN_MILLISECONDS));
        AddCombatAction(EBONROC_WING_BUFFET, uint32(30 * IN_MILLISECONDS));
        AddCombatAction(EBONROC_THRASH, uint32(6 * IN_MILLISECONDS));
    }

    ScriptedInstance* m_instance;

    void Aggro(Unit* /*who*/) override
    {
        if (m_instance)
            m_instance->SetData(TYPE_EBONROC, IN_PROGRESS);
    }

    void JustDied(Unit* /*killer*/) override
    {
        if (m_instance)
            m_instance->SetData(TYPE_EBONROC, DONE);
    }

    void JustReachedHome() override
    {
        if (m_instance)
            m_instance->SetData(TYPE_EBONROC, FAIL);
    }

    void SpellHitTarget(Unit* target, const SpellEntry* spellInfo, SpellMissInfo /*missInfo*/) override
    {
        if (spellInfo->Id == SPELL_WING_BUFFET) // reduces threat of everyone hit
            m_creature->getThreatManager().modifyThreatPercent(target, -50);
    }

    void ExecuteAction(uint32 action) override
    {
        switch (action)
        {
            case EBONROC_SHADOW_OF_EBONROC:
            {
                if (DoCastSpellIfCan(m_creature->GetVictim(), SPELL_SHADOW_OF_EBONROC) == CAST_OK)
                    ResetCombatAction(action, urand(25 * IN_MILLISECONDS, 35 * IN_MILLISECONDS));
                break;
            }
            case EBONROC_SHADOW_FLAME:
            {
                if (DoCastSpellIfCan(nullptr, SPELL_SHADOW_FLAME) == CAST_OK)
                    ResetCombatAction(action, urand(15 * IN_MILLISECONDS, 18 * IN_MILLISECONDS));
                break;
            }
            case EBONROC_WING_BUFFET:
            {
                if (DoCastSpellIfCan(nullptr, SPELL_WING_BUFFET) == CAST_OK)
                    ResetCombatAction(action, urand(30 * IN_MILLISECONDS, 35 * IN_MILLISECONDS));
                break;
            }
            case EBONROC_THRASH:
            {
                if (DoCastSpellIfCan(nullptr, SPELL_THRASH) == CAST_OK)
                    ResetCombatAction(action, urand(2 * IN_MILLISECONDS, 6 * IN_MILLISECONDS));
                break;
            }
        }
    }
};

void AddSC_boss_ebonroc()
{
    Script* pNewScript = new Script;
    pNewScript->Name = "boss_ebonroc";
    pNewScript->GetAI = &GetNewAIInstance<boss_ebonrocAI>;
    pNewScript->RegisterSelf();
}
