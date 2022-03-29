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
SDName: Boss_Garr
SD%Complete: 95
SDComment: 'Summon Player' missing
SDCategory: Molten Core
EndScriptData

*/

#include "AI/ScriptDevAI/include/sc_common.h"
#include "molten_core.h"
#include "AI/ScriptDevAI/base/CombatAI.h"

enum
{
    EMOTE_MASSIVE_ERUPTION      = -1409025,

    // Garr spells
    SPELL_ANTIMAGICPULSE        = 19492,
    SPELL_MAGMASHACKLES         = 19496,
    SPELL_ENRAGE                = 19516,
    SPELL_SUMMON_PLAYER         = 20477,
    SPELL_ERUPTION_TRIGGER      = 20482,
    SPELL_SEPARATION_ANXIETY    = 23487,                    // Aura cast on himself by Garr, if adds move out of range, they will cast spell 23492 on themselves

    // Firesworn spells
    SPELL_ERUPTION              = 19497,
    SPELL_ENRAGE_TRIGGER        = 19515,
    SPELL_MASSIVE_ERUPTION      = 20483,
    SPELL_THRASH                = 8876,
    SPELL_IMMOLATE              = 15733,
};

enum GarrActions
{
    GARR_ANTI_MAGIC_PULSE,
    GARR_MAGMA_SHACKLES,
    GARR_MASSIVE_ERUPTION,
    GARR_ACTION_MAX,
};

struct boss_garrAI : public CombatAI
{
    boss_garrAI(Creature* creature) : CombatAI(creature, GARR_ACTION_MAX), m_instance(static_cast<ScriptedInstance*>(creature->GetInstanceData()))
    {
        AddCombatAction(GARR_ANTI_MAGIC_PULSE, 10 * IN_MILLISECONDS, 15 * IN_MILLISECONDS);
        AddCombatAction(GARR_MAGMA_SHACKLES, 5 * IN_MILLISECONDS, 10 * IN_MILLISECONDS);
        AddCombatAction(GARR_MASSIVE_ERUPTION, uint32(6 * MINUTE * IN_MILLISECONDS));
        Reset();
    }

    ScriptedInstance* m_instance;

    void Aggro(Unit* /*who*/) override
    {
        if (m_instance)
            m_instance->SetData(TYPE_GARR, IN_PROGRESS);

        // Garr has a 100 yard aura to keep track of the distance of each of his adds, they will enrage if moved out of it
        DoCastSpellIfCan(nullptr, SPELL_SEPARATION_ANXIETY, CAST_TRIGGERED | CAST_AURA_NOT_PRESENT);
    }

    void JustDied(Unit* /*killer*/) override
    {
        if (m_instance)
            m_instance->SetData(TYPE_GARR, DONE);
    }

    void JustReachedHome() override
    {
        if (m_instance)
            m_instance->SetData(TYPE_GARR, FAIL);
    }

    void SpellHit(Unit* /*caster*/, const SpellEntry* spellInfo) override
    {
        if (spellInfo->Id == SPELL_ENRAGE_TRIGGER)
            DoCastSpellIfCan(nullptr, SPELL_ENRAGE, CAST_TRIGGERED);
    }

    void ExecuteAction(uint32 action) override
    {
        switch (action)
        {
            case GARR_ANTI_MAGIC_PULSE:
            {
                if (DoCastSpellIfCan(nullptr, SPELL_ANTIMAGICPULSE) == CAST_OK)
                    ResetCombatAction(action, urand(15 * IN_MILLISECONDS, 20 * IN_MILLISECONDS));
                break;
            }
            case GARR_MAGMA_SHACKLES:
            {
                if (DoCastSpellIfCan(nullptr, SPELL_MAGMASHACKLES) == CAST_OK)
                    ResetCombatAction(action, urand(10 * IN_MILLISECONDS, 15 * IN_MILLISECONDS));
                break;
            }
            case GARR_MASSIVE_ERUPTION:
            {
                if (DoCastSpellIfCan(m_creature, SPELL_ERUPTION_TRIGGER) == CAST_OK)
                {
                    DoScriptText(EMOTE_MASSIVE_ERUPTION, m_creature);
                    ResetCombatAction(action, 20 * IN_MILLISECONDS);
                }
                break;
            }
        }
    }
};

struct mob_fireswornAI : public ScriptedAI
{
    mob_fireswornAI(Creature* creature) : ScriptedAI(creature) {}

    void Reset() override
    {
        DoCastSpellIfCan(nullptr, SPELL_THRASH, CAST_AURA_NOT_PRESENT | CAST_TRIGGERED);
        DoCastSpellIfCan(nullptr, SPELL_IMMOLATE, CAST_AURA_NOT_PRESENT | CAST_TRIGGERED);
    }

    void JustDied(Unit* killer) override
    {
        if (m_creature != killer)
            DoCastSpellIfCan(nullptr, SPELL_ERUPTION);

        DoCastSpellIfCan(nullptr, SPELL_ENRAGE_TRIGGER, CAST_TRIGGERED);
    }

    void SpellHit(Unit* /*caster*/, const SpellEntry* spellInfo) override
    {
        if (spellInfo->Id == SPELL_ERUPTION_TRIGGER)
            DoCastSpellIfCan(nullptr, SPELL_MASSIVE_ERUPTION);
    }
};

void AddSC_boss_garr()
{
    Script* pNewScript = new Script;
    pNewScript->Name = "boss_garr";
    pNewScript->GetAI = &GetNewAIInstance<boss_garrAI>;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "mob_firesworn";
    pNewScript->GetAI = &GetNewAIInstance<mob_fireswornAI>;
    pNewScript->RegisterSelf();
}
