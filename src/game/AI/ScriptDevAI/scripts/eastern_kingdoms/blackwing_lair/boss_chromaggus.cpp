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
SDName: Boss_Chromaggus
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
    EMOTE_GENERIC_FRENZY_KILL   = -1000001,
    EMOTE_GENERIC_FRENZY        = -1000002,
    EMOTE_SHIMMER               = -1469003,

    SPELL_BREATH_SELECTION      = 23195,
    SPELL_BROOD_AFFLICTION      = 23173, // TODO: Rework spell script

    SPELL_ELEMENTAL_SHIELD_BWL  = 22276,
    SPELL_FRENZY                = 23128,
    SPELL_ENRAGE                = 23537
};

enum ChromaggusActions
{
    CHROMAGGUS_ENRAGE,
    CHROMAGGUS_ELEMENTAL_SHIELD,
    CHROMAGGUS_BREATH_LEFT,
    CHROMAGGUS_BREATH_RIGHT,
    CHROMAGGUS_BROOD_AFFLICTION,
    CHROMAGGUS_FRENZY,
    CHROMAGGUS_ACTION_MAX,
};

struct boss_chromaggusAI : public CombatAI
{
    boss_chromaggusAI(Creature* creature) : CombatAI(creature, CHROMAGGUS_ACTION_MAX), m_instance(static_cast<ScriptedInstance*>(creature->GetInstanceData()))
    {
        // Select the 2 different breaths that we are going to use until despawned
        DoCastSpellIfCan(nullptr, SPELL_BREATH_SELECTION);
        if (m_instance)
        {
            m_breathLeftSpell = m_instance->GetData(TYPE_CHROMA_LBREATH);    // Spell for left breath, stored in instance data
            m_breathRightSpell = m_instance->GetData(TYPE_CHROMA_RBREATH);   // Spell for right breath, stored in instance data
        }
        AddTimerlessCombatAction(CHROMAGGUS_ENRAGE, true);
        AddCombatAction(CHROMAGGUS_ELEMENTAL_SHIELD, 0u);
        AddCombatAction(CHROMAGGUS_BREATH_LEFT, 30000u);
        AddCombatAction(CHROMAGGUS_BREATH_RIGHT, 60000u);
        AddCombatAction(CHROMAGGUS_BROOD_AFFLICTION, uint32(7 * IN_MILLISECONDS));
        AddCombatAction(CHROMAGGUS_FRENZY, 15000u);
    }

    ScriptedInstance* m_instance;

    uint32 m_breathLeftSpell;
    uint32 m_breathRightSpell;

    void Aggro(Unit* /*who*/) override
    {
        if (m_instance)
            m_instance->SetData(TYPE_CHROMAGGUS, IN_PROGRESS);
    }

    void JustDied(Unit* /*killer*/) override
    {
        if (m_instance)
            m_instance->SetData(TYPE_CHROMAGGUS, DONE);
    }

    void JustReachedHome() override
    {
        if (m_instance)
            m_instance->SetData(TYPE_CHROMAGGUS, FAIL);
    }

    void ExecuteAction(uint32 action) override
    {
        switch (action)
        {
            case CHROMAGGUS_ENRAGE:
            {
                if (m_creature->GetHealthPercent() < 20.0f)
                {
                    DoCastSpellIfCan(nullptr, SPELL_ENRAGE);
                    DoScriptText(EMOTE_GENERIC_FRENZY, m_creature);
                    SetActionReadyStatus(action, false);
                }
                break;
            }
            case CHROMAGGUS_ELEMENTAL_SHIELD:
            {
                if (DoCastSpellIfCan(nullptr, SPELL_ELEMENTAL_SHIELD_BWL) == CAST_OK)
                {
                    DoScriptText(EMOTE_SHIMMER, m_creature);
                    ResetCombatAction(action, 45000);
                }
                break;
            }
            case CHROMAGGUS_BREATH_LEFT:
            {
                if (DoCastSpellIfCan(nullptr, m_breathLeftSpell) == CAST_OK)
                    ResetCombatAction(action, 60000);
                break;
            }
            case CHROMAGGUS_BREATH_RIGHT:
            {
                if (DoCastSpellIfCan(nullptr, m_breathRightSpell) == CAST_OK)
                    ResetCombatAction(action, 60000);
                break;
            }
            case CHROMAGGUS_BROOD_AFFLICTION:
            {
                if (DoCastSpellIfCan(nullptr, SPELL_BROOD_AFFLICTION) == CAST_OK)
                    ResetCombatAction(action, 7 * IN_MILLISECONDS);
                break;
            }
            case CHROMAGGUS_FRENZY:
            {
                if (DoCastSpellIfCan(nullptr, SPELL_FRENZY) == CAST_OK)
                {
                    DoScriptText(EMOTE_GENERIC_FRENZY_KILL, m_creature);
                    ResetCombatAction(action, 15 * IN_MILLISECONDS);
                }
                break;
            }
        }
    }
};

void AddSC_boss_chromaggus()
{
    Script* pNewScript = new Script;
    pNewScript->Name = "boss_chromaggus";
    pNewScript->GetAI = &GetNewAIInstance<boss_chromaggusAI>;
    pNewScript->RegisterSelf();
}
