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
SDName: Boss_Kurinnaxx
SD%Complete: 90
SDComment: Summon Player ability NYI
SDCategory: Ruins of Ahn'Qiraj
EndScriptData

*/

#include "AI/ScriptDevAI/include/sc_common.h"
#include "AI/ScriptDevAI/base/CombatAI.h"

enum
{
    SAY_ENRAGE              = -1000002,

    SPELL_THRASH            = 3391,
    SPELL_WIDE_SLASH        = 25814,
    SPELL_MORTAL_WOUND      = 25646,
    SPELL_SANDTRAP          = 26524,        // summons gameobject 180647
    SPELL_ENRAGE            = 26527,
    SPELL_SUMMON_PLAYER     = 26446,

    GO_SAND_TRAP            = 180647,
};

enum KurinnaxxAction
{
    KURINNAXX_ENRAGE,
    KURINNAXX_MORTAL_WOUND,
    KURINNAXX_SAND_TRAP,
    KURINNAXX_THRASH,
    KURINNAXX_WIDE_SLASH,
    KURINNAXX_ACTION_MAX,
    KURINNAXX_TRAP_TRIGGER,
};

struct boss_kurinnaxxAI : public CombatAI
{
    boss_kurinnaxxAI(Creature* creature) : CombatAI(creature, KURINNAXX_ACTION_MAX)
    {
        AddTimerlessCombatAction(KURINNAXX_ENRAGE, true);
        AddCombatAction(KURINNAXX_MORTAL_WOUND, 8000, 10000);
        AddCombatAction(KURINNAXX_SAND_TRAP, 5000, 10000);
        AddCombatAction(KURINNAXX_THRASH, 1000, 5000);
        AddCombatAction(KURINNAXX_WIDE_SLASH, 10000, 15000);
        AddCustomAction(KURINNAXX_TRAP_TRIGGER, true, [&]() { TriggerTrap(); });
    }

    ObjectGuid m_sandtrapGuid;

    void JustSummoned(GameObject* pGo) override
    {
        if (pGo->GetEntry() == GO_SAND_TRAP)
        {
            ResetTimer(KURINNAXX_TRAP_TRIGGER, 4000);
            m_sandtrapGuid = pGo->GetObjectGuid();
        }
    }

    void TriggerTrap()
    {
        if (GameObject* trap = m_creature->GetMap()->GetGameObject(m_sandtrapGuid))
            trap->Use(m_creature);
    }

    void ExecuteAction(uint32 action) override
    {
        switch (action)
        {
            case KURINNAXX_ENRAGE:
            {
                if (m_creature->GetHealthPercent() <= 30.0f)
                {
                    if (DoCastSpellIfCan(nullptr, SPELL_ENRAGE) == CAST_OK)
                    {
                        DoScriptText(SAY_ENRAGE, m_creature);
                        SetActionReadyStatus(action, false);
                    }
                }
                break;
            }
            case KURINNAXX_MORTAL_WOUND:
            {
                if (DoCastSpellIfCan(m_creature->GetVictim(), SPELL_MORTAL_WOUND) == CAST_OK)
                    ResetCombatAction(action, urand(8000, 10000));
                break;
            }
            case KURINNAXX_SAND_TRAP:
            {
                if (DoCastSpellIfCan(m_creature->GetVictim(), SPELL_SANDTRAP) == CAST_OK)
                    ResetCombatAction(action, urand(10000, 15000));
                break;
            }
            case KURINNAXX_THRASH:
            {
                if (DoCastSpellIfCan(nullptr, SPELL_THRASH) == CAST_OK)
                    ResetCombatAction(action, urand(12000, 17000));
                break;
            }
            case KURINNAXX_WIDE_SLASH:
            {
                if (DoCastSpellIfCan(m_creature->GetVictim(), SPELL_WIDE_SLASH) == CAST_OK)
                    ResetCombatAction(action, urand(12000, 15000));
                break;
            }
        }
    }
};

void AddSC_boss_kurinnaxx()
{
    Script* pNewScript = new Script;
    pNewScript->Name = "boss_kurinnaxx";
    pNewScript->GetAI = &GetNewAIInstance<boss_kurinnaxxAI>;
    pNewScript->RegisterSelf();
}
