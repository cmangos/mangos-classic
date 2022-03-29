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
SDName: Boss_Azuregos
SD%Complete: 100
SDComment:
SDCategory: Azshara
EndScriptData
*/

#include "AI/ScriptDevAI/base/CombatAI.h"
#include "AI/ScriptDevAI/include/sc_common.h"

enum
{
    SAY_TELEPORT                = -1000100,

    SPELL_ARCANE_VACUUM         = 21147,
    SPELL_MARK_OF_FROST_PLAYER  = 23182,
    SPELL_MARK_OF_FROST_AURA    = 23184,                    // Triggers 23186 on players that have 23182; unfortunatelly 23183 is missing from dbc
    SPELL_MANA_STORM            = 21097,
    SPELL_CHILL                 = 21098,
    SPELL_FROST_BREATH          = 21099,
    SPELL_REFLECT               = 22067,
    SPELL_CLEAVE                = 19983,                    // Was 8255; this one is from wowhead and seems to be the correct one
};

enum AzuregosActions
{
    AZUREGOS_MANASTORM,
    AZUREGOS_CHILL,
    AZUREGOS_FROSTBREATH,
    AZUREGOS_TELEPORT,
    AZUREGOS_REFLECTMAGIC,
    AZUREGOS_CLEAVE,
    AZUREGOS_ACTION_MAX,
};

struct boss_azuregosAI : public CombatAI
{
    boss_azuregosAI(Creature* creature) : CombatAI(creature, AZUREGOS_ACTION_MAX)
    {
        AddCombatAction(AZUREGOS_MANASTORM, 5u * IN_MILLISECONDS, 17u * IN_MILLISECONDS);
        AddCombatAction(AZUREGOS_CHILL, 10u * IN_MILLISECONDS, 30u * IN_MILLISECONDS);
        AddCombatAction(AZUREGOS_FROSTBREATH, 2u * IN_MILLISECONDS, 8u * IN_MILLISECONDS);
        AddCombatAction(AZUREGOS_TELEPORT, 30u * IN_MILLISECONDS);
        AddCombatAction(AZUREGOS_REFLECTMAGIC, 15u * IN_MILLISECONDS, 30u * IN_MILLISECONDS);
        AddCombatAction(AZUREGOS_CLEAVE, 7u * IN_MILLISECONDS);
    }

    void Reset() override
    {
        CombatAI::Reset();
    }

    void KilledUnit(Unit* victim) override
    {
        // Mark killed players with Mark of Frost
        if (victim->GetTypeId() == TYPEID_PLAYER)
            victim->CastSpell(victim, SPELL_MARK_OF_FROST_PLAYER, TRIGGERED_OLD_TRIGGERED, nullptr, nullptr, m_creature->GetObjectGuid());
    }

    void Aggro(Unit* /*who*/) override
    {
        // Boss aura which triggers the stun effect on dead players who resurrect
        DoCastSpellIfCan(m_creature, SPELL_MARK_OF_FROST_AURA);

        m_creature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
    }

    void EnterEvadeMode () override
    {
        m_creature->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);

        CombatAI::EnterEvadeMode();
    }

    void ExecuteAction(uint32 action) override
    {
        switch (action)
        {
            case AZUREGOS_MANASTORM:
            {
                if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_NEAREST_BY, 0))
                {
                    if (DoCastSpellIfCan(pTarget, SPELL_MANA_STORM) == CAST_OK)
                        ResetCombatAction(action, urand(18, 35) * IN_MILLISECONDS);
                }
                break;
            }
            case AZUREGOS_TELEPORT:
            {
                if (DoCastSpellIfCan(m_creature, SPELL_ARCANE_VACUUM) == CAST_OK)
                {
                    DoScriptText(SAY_TELEPORT, m_creature);
                    ResetCombatAction(action, urand(20, 30) * IN_MILLISECONDS);
                }
                break;
            }
            case AZUREGOS_CHILL:
            {
                if (DoCastSpellIfCan(m_creature, SPELL_CHILL) == CAST_OK)
                    ResetCombatAction(action, urand(13, 25) * IN_MILLISECONDS);
                break;
            }
            case AZUREGOS_FROSTBREATH:
            {
                if (DoCastSpellIfCan(m_creature, SPELL_FROST_BREATH) == CAST_OK)
                    ResetCombatAction(action, urand(10, 25) * IN_MILLISECONDS);
                break;
            }
            case AZUREGOS_REFLECTMAGIC:
            {
                if (DoCastSpellIfCan(m_creature, SPELL_REFLECT) == CAST_OK)
                    ResetCombatAction(action, urand(20, 35) * IN_MILLISECONDS);
                break;
            }
            case AZUREGOS_CLEAVE:
            {
                if (DoCastSpellIfCan(m_creature->GetVictim(), SPELL_CLEAVE) == CAST_OK)
                    ResetCombatAction(action, 7 * IN_MILLISECONDS);
                break;
            }
            default:
                break;
        }
    }
};

void AddSC_boss_azuregos()
{
    Script* pNewScript = new Script;
    pNewScript->Name = "boss_azuregos";
    pNewScript->GetAI = &GetNewAIInstance<boss_azuregosAI>;
    pNewScript->RegisterSelf();
}
