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
SDName: Boss_Broodlord_Lashlayer
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
    SAY_AGGRO                   = -1469000,
    SAY_LEASH                   = -1469001,

    SPELL_CLEAVE                = 15284,
    SPELL_KNOCK_AWAY            = 18670,
    SPELL_BLAST_WAVE            = 23331,
    SPELL_MORTAL_STRIKE         = 24573,
};

enum BroodlordActions
{
    BROODLORD_CLEAVE,
    BROODLORD_KNOCK_AWAY,
    BROODLORD_BLAST_WAVE,
    BROODLORD_MORTAL_STRIKE,
    BROODLORD_ACTION_MAX,
};

struct boss_broodlordAI : public CombatAI
{
    boss_broodlordAI(Creature* creature) : CombatAI(creature, BROODLORD_ACTION_MAX), m_instance(static_cast<instance_blackwing_lair*>(creature->GetInstanceData()))
    {
        AddCombatAction(BROODLORD_CLEAVE, 8000u);
        AddCombatAction(BROODLORD_KNOCK_AWAY, 12000u);
        AddCombatAction(BROODLORD_BLAST_WAVE, 20000u);
        AddCombatAction(BROODLORD_MORTAL_STRIKE, 30000u);
        m_creature->GetCombatManager().SetLeashingCheck([](Unit*, float /*x*/, float /*y*/, float z)
        {
            return z < 448.60f;
        });        
        Reset();
    }

    ScriptedInstance* m_instance;

    void Aggro(Unit* /*who*/) override
    {
        if (m_instance)
            m_instance->SetData(TYPE_LASHLAYER, IN_PROGRESS);

        DoScriptText(SAY_AGGRO, m_creature);
    }

    void JustDied(Unit* /*killer*/) override
    {
        if (m_instance)
            m_instance->SetData(TYPE_LASHLAYER, DONE);
    }

    void JustReachedHome() override
    {
        if (m_instance)
            m_instance->SetData(TYPE_LASHLAYER, FAIL);
    }

    void OnLeash() override
    {
        DoScriptText(SAY_LEASH, m_creature);
    }

    void ExecuteAction(uint32 action) override
    {
        switch (action)
        {
            case BROODLORD_CLEAVE:
            {
                if (DoCastSpellIfCan(m_creature->GetVictim(), SPELL_CLEAVE) == CAST_OK)
                    ResetCombatAction(action, 7000);
                break;
            }
            case BROODLORD_KNOCK_AWAY:
            {
                if (DoCastSpellIfCan(m_creature->GetVictim(), SPELL_KNOCK_AWAY) == CAST_OK)
                    ResetCombatAction(action, urand(15000, 30000));
                break;
            }
            case BROODLORD_BLAST_WAVE:
            {
                if (DoCastSpellIfCan(nullptr, SPELL_BLAST_WAVE) == CAST_OK)
                    ResetCombatAction(action, urand(8000, 16000));
                break;
            }
            case BROODLORD_MORTAL_STRIKE:
            {
                if (DoCastSpellIfCan(m_creature->GetVictim(), SPELL_MORTAL_STRIKE) == CAST_OK)
                    ResetCombatAction(action, urand(25000, 35000));
                break;
            }
        }
    }
};

void AddSC_boss_broodlord()
{
    Script* pNewScript = new Script;
    pNewScript->Name = "boss_broodlord";
    pNewScript->GetAI = &GetNewAIInstance<boss_broodlordAI>;
    pNewScript->RegisterSelf();
}
