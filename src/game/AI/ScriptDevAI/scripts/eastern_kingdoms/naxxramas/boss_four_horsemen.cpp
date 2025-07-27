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
SDName: Boss_Four_Horsemen
SD%Complete: 99
SDComment: Percentage of say for special text is guessed based on videos
SDCategory: Naxxramas
EndScriptData

*/

#include "AI/ScriptDevAI/include/sc_common.h"
#include "AI/ScriptDevAI/base/BossAI.h"
#include "naxxramas.h"

enum
{
    // ***** Yells *****
    // lady blaumeux
    SAY_BLAU_AGGRO          = 13010,
    SAY_BLAU_SPECIAL        = 13013,
    SAY_BLAU_SLAY           = 13012,
    SAY_BLAU_DEATH          = 13011,

    // alexandros mograine
    SAY_MORG_AGGRO1         = 13051,
    SAY_MORG_AGGRO2         = 13052,
    SAY_MORG_AGGRO3         = 13053,
    SAY_MORG_SLAY1          = 13055,
    SAY_MORG_SLAY2          = 13056,
    SAY_MORG_SPECIAL        = 13057,
    SAY_MORG_DEATH          = 13054,

    // thane korthazz
    SAY_KORT_AGGRO          = 13034,
    SAY_KORT_SPECIAL        = 13037,
    SAY_KORT_SLAY           = 13036,
    SAY_KORT_DEATH          = 13035,

    // sir zeliek
    SAY_ZELI_AGGRO          = 13097,
    SAY_ZELI_SPECIAL        = 13100,
    SAY_ZELI_SLAY           = 13099,
    SAY_ZELI_DEATH          = 13098,

    // ***** Spells *****
    // all horsemen
    SPELL_SHIELDWALL        = 29061,

    // lady blaumeux
    SPELL_MARK_OF_BLAUMEUX  = 28833,
    SPELL_VOID_ZONE         = 28863,
    SPELL_SPIRIT_BLAUMEUX   = 28931,

    // highlord mograine
    SPELL_MARK_OF_MOGRAINE  = 28834,
    SPELL_RIGHTEOUS_FIRE    = 28882,
    SPELL_SPIRIT_MOGRAINE   = 28928,

    // thane korthazz
    SPELL_MARK_OF_KORTHAZZ  = 28832,
    SPELL_METEOR            = 28884,
    SPELL_SPIRIT_KORTHAZZ   = 28932,

    // sir zeliek
    SPELL_MARK_OF_ZELIEK    = 28835,
    SPELL_HOLY_WRATH        = 28883,
    SPELL_SPIRIT_ZELIEK     = 28934,

    INDEX_BLAUMEUX          = 0,
    INDEX_MORGRAINE         = 1,
    INDEX_KORTHAZZ          = 2,
    INDEX_ZELIEK            = 3,

    MAX_MARK_STACKS         = 100,      // Horsemen enrage at this milestone
};

// Texts used during the encounter: they are ordered so we can easily use them in a parent class
static int32 const aggroSayList[4] = { SAY_BLAU_AGGRO, 0, SAY_KORT_AGGRO, SAY_ZELI_AGGRO };
static int32 const deathSayList[4] = { SAY_BLAU_DEATH, SAY_MORG_DEATH, SAY_KORT_DEATH, SAY_ZELI_DEATH };
static int32 const killSayList[4] = { SAY_BLAU_SLAY, 0, SAY_KORT_SLAY, SAY_ZELI_SLAY };

// Spells used during the encounter: they are ordered so we can easily use them in a parent class
static uint32 const spiritSpellList[4] = { SPELL_SPIRIT_BLAUMEUX, SPELL_SPIRIT_MOGRAINE, SPELL_SPIRIT_KORTHAZZ, SPELL_SPIRIT_ZELIEK };
static uint32 const markSpellList[4] = { SPELL_MARK_OF_BLAUMEUX, SPELL_MARK_OF_MOGRAINE, SPELL_MARK_OF_KORTHAZZ, SPELL_MARK_OF_ZELIEK };

struct Location2DPoint
{
    float x, y;
};

// Coordinates at the entrance of the Four Horsemen room. Beyond these, the Horsemen reset (leashing)
static const Location2DPoint resetCoords[3] =
{
        {2577.3f, -3024.0f},
        {2585.9f, -3015.1f},
        {2594.6f, -3006.7f}
};

enum BlaumeuxActions
{
    BLAUMEUX_SHIELD_WALL_50,
    BLAUMEUX_SHIELD_WALL_20,
    BLAUMEUX_ENRAGE,
    BLAUMEUX_SPECIAL,
    BLAUMEUX_ACTIONS_MAX,
};

struct boss_lady_blaumeuxAI : public BossAI
{
    boss_lady_blaumeuxAI(Creature* creature) : BossAI(creature, BLAUMEUX_ACTIONS_MAX)
    {
        SetDataType(TYPE_FOUR_HORSEMEN);
        AddOnKillText(SAY_BLAU_SLAY);
        AddOnDeathText(SAY_BLAU_DEATH);
        AddOnAggroText(SAY_BLAU_AGGRO);
        AddCombatAction(BLAUMEUX_SPECIAL, 5s, 120s);
        AddCombatAction(BLAUMEUX_ENRAGE, 20min);
        AddTimerlessCombatAction(BLAUMEUX_SHIELD_WALL_50, true);
        AddTimerlessCombatAction(BLAUMEUX_SHIELD_WALL_20, true);
        m_creature->GetCombatManager().SetLeashingCheck([&](Unit*, float x, float y, float /*z*/)
        {
            return (x > resetCoords[0].x && y < resetCoords[0].y) ||
                (x > resetCoords[1].x && y < resetCoords[1].y) ||
                (x > resetCoords[2].x && y < resetCoords[2].y);
        });
    }

    ScriptedInstance* m_instance;

    void Reset() override
    {
        BossAI::Reset();
        m_creature->SetSpellList(m_creature->GetCreatureInfo()->SpellList);
    }

    void JustDied(Unit* killer) override
    {
        BossAI::JustDied(killer);
        DoCastSpellIfCan(nullptr, SPELL_SPIRIT_BLAUMEUX, CAST_TRIGGERED);
    }

    void ExecuteAction(uint32 action) override
    {
        switch (action)
        {
            case BLAUMEUX_SHIELD_WALL_50:
                if (DoCastSpellIfCan(nullptr, SPELL_SHIELDWALL) == CAST_OK)
                    DisableCombatAction(action);
                break;
            case BLAUMEUX_SHIELD_WALL_20:
                if (DoCastSpellIfCan(nullptr, SPELL_SHIELDWALL) == CAST_OK)
                    DisableCombatAction(action);
                break;
            case BLAUMEUX_SPECIAL:
            {
                DoBroadcastText(SAY_BLAU_SPECIAL, m_creature);
                ResetCombatAction(action, RandomTimer(5s, 120s));
                break;
            }
            case BLAUMEUX_ENRAGE:
                m_creature->SetSpellList(m_creature->GetCreatureInfo()->SpellList + 1);
                DisableCombatAction(action);
                break;
        }
    }
};

enum MograineActions
{
    MOGRAINE_SHIELD_WALL_50,
    MOGRAINE_SHIELD_WALL_20,
    MOGRAINE_ENRAGE,
    MOGRAINE_SPECIAL,
    MOGRAINE_ACTIONS_MAX,
};

struct boss_alexandros_mograineAI : public BossAI
{
    boss_alexandros_mograineAI(Creature* creature) : BossAI(creature, MOGRAINE_ACTIONS_MAX)
    {
        SetDataType(TYPE_FOUR_HORSEMEN);
        AddOnAggroText(SAY_MORG_AGGRO1, SAY_MORG_AGGRO2, SAY_MORG_AGGRO3);
        AddOnKillText(SAY_MORG_SLAY1, SAY_MORG_SLAY2);
        AddOnDeathText(SAY_MORG_DEATH);
        AddCombatAction(MOGRAINE_SPECIAL, 5s, 120s);
        AddCombatAction(MOGRAINE_ENRAGE, 20min);
        AddTimerlessCombatAction(MOGRAINE_SHIELD_WALL_50, true);
        AddTimerlessCombatAction(MOGRAINE_SHIELD_WALL_20, true);
        m_creature->GetCombatManager().SetLeashingCheck([&](Unit*, float x, float y, float /*z*/)
        {
            return (x > resetCoords[0].x && y < resetCoords[0].y) ||
                (x > resetCoords[1].x && y < resetCoords[1].y) ||
                (x > resetCoords[2].x && y < resetCoords[2].y);
        });
    }

    void Reset() override
    {
        BossAI::Reset();
        m_creature->SetSpellList(m_creature->GetCreatureInfo()->SpellList);
    }

    void JustDied(Unit* killer) override
    {
        BossAI::JustDied(killer);
        DoCastSpellIfCan(nullptr, SPELL_SPIRIT_MOGRAINE, CAST_TRIGGERED);
    }

    void ExecuteAction(uint32 action) override
    {
        switch (action)
        {
            case MOGRAINE_SHIELD_WALL_50:
                if (DoCastSpellIfCan(nullptr, SPELL_SHIELDWALL) == CAST_OK)
                    DisableCombatAction(action);
                break;
            case MOGRAINE_SHIELD_WALL_20:
                if (DoCastSpellIfCan(nullptr, SPELL_SHIELDWALL) == CAST_OK)
                    DisableCombatAction(action);
                break;
            case MOGRAINE_SPECIAL:
            {
                DoBroadcastText(SAY_MORG_SPECIAL, m_creature);
                ResetCombatAction(action, RandomTimer(5s, 120s));
                break;
            }
            case MOGRAINE_ENRAGE:
                m_creature->SetSpellList(m_creature->GetCreatureInfo()->SpellList + 1);
                DisableCombatAction(action);
                break;
        }
    }
};

enum KorthazActions
{
    KORTHAZ_SHIELD_WALL_50,
    KORTHAZ_SHIELD_WALL_20,
    KORTHAZ_ENRAGE,
    KORTHAZ_SPECIAL,
    KORTHAZ_ACTIONS_MAX,
};

struct boss_thane_korthazzAI : public BossAI
{
    boss_thane_korthazzAI(Creature* creature) : BossAI(creature, KORTHAZ_ACTIONS_MAX)
    {
        SetDataType(TYPE_FOUR_HORSEMEN);
        AddOnKillText(SAY_KORT_SLAY);
        AddOnDeathText(SAY_KORT_DEATH);
        AddOnAggroText(SAY_KORT_AGGRO);
        AddCombatAction(KORTHAZ_SPECIAL, 5s, 120s);
        AddCombatAction(KORTHAZ_ENRAGE, 20min);
        AddTimerlessCombatAction(KORTHAZ_SHIELD_WALL_50, true);
        AddTimerlessCombatAction(KORTHAZ_SHIELD_WALL_20, true);
        m_creature->GetCombatManager().SetLeashingCheck([&](Unit*, float x, float y, float /*z*/)
        {
            return (x > resetCoords[0].x && y < resetCoords[0].y) ||
                (x > resetCoords[1].x && y < resetCoords[1].y) ||
                (x > resetCoords[2].x && y < resetCoords[2].y);
        });
    }

    void Reset() override
    {
        BossAI::Reset();
        m_creature->SetSpellList(m_creature->GetCreatureInfo()->SpellList);
    }

    void JustDied(Unit* killer) override
    {
        BossAI::JustDied(killer);
        DoCastSpellIfCan(nullptr, SPELL_SPIRIT_KORTHAZZ, CAST_TRIGGERED);
    }

    void ExecuteAction(uint32 action) override
    {
        switch (action)
        {
            case KORTHAZ_SHIELD_WALL_50:
                if (DoCastSpellIfCan(nullptr, SPELL_SHIELDWALL) == CAST_OK)
                    DisableCombatAction(action);
                break;
            case KORTHAZ_SHIELD_WALL_20:
                if (DoCastSpellIfCan(nullptr, SPELL_SHIELDWALL) == CAST_OK)
                    DisableCombatAction(action);
                break;
            case KORTHAZ_SPECIAL:
            {
                DoBroadcastText(SAY_KORT_SPECIAL, m_creature);
                ResetCombatAction(action, RandomTimer(5s, 120s));
                break;
            }
            case KORTHAZ_ENRAGE:
                m_creature->SetSpellList(m_creature->GetCreatureInfo()->SpellList + 1);
                DisableCombatAction(action);
                break;
        }
    }
};

enum ZeliekActions
{
    ZELIEK_SHIELD_WALL_50,
    ZELIEK_SHIELD_WALL_20,
    ZELIEK_ENRAGE,
    ZELIEK_SPECIAL,
    ZELIEK_ACTIONS_MAX,
};

struct boss_sir_zeliekAI : public BossAI
{
    boss_sir_zeliekAI(Creature* creature) : BossAI(creature, ZELIEK_ACTIONS_MAX)
    {
        SetDataType(TYPE_FOUR_HORSEMEN);
        AddOnKillText(SAY_ZELI_SLAY);
        AddOnDeathText(SAY_ZELI_DEATH);
        AddOnAggroText(SAY_ZELI_AGGRO);
        AddCombatAction(ZELIEK_SPECIAL, 5s, 120s);
        AddCombatAction(ZELIEK_ENRAGE, 20min);
        AddTimerlessCombatAction(ZELIEK_SHIELD_WALL_50, true);
        AddTimerlessCombatAction(ZELIEK_SHIELD_WALL_20, true);
        m_creature->GetCombatManager().SetLeashingCheck([&](Unit*, float x, float y, float /*z*/)
        {
            return (x > resetCoords[0].x && y < resetCoords[0].y) ||
                (x > resetCoords[1].x && y < resetCoords[1].y) ||
                (x > resetCoords[2].x && y < resetCoords[2].y);
        });
    }

    void Reset() override
    {
        BossAI::Reset();
        m_creature->SetSpellList(m_creature->GetCreatureInfo()->SpellList);
    }

    void JustDied(Unit* killer) override
    {
        BossAI::JustDied(killer);
        DoCastSpellIfCan(nullptr, SPELL_SPIRIT_ZELIEK, CAST_TRIGGERED);
    }

    void ExecuteAction(uint32 action) override
    {
        switch (action)
        {
            case ZELIEK_SHIELD_WALL_50:
                if (DoCastSpellIfCan(nullptr, SPELL_SHIELDWALL) == CAST_OK)
                    DisableCombatAction(action);
                break;
            case ZELIEK_SHIELD_WALL_20:
                if (DoCastSpellIfCan(nullptr, SPELL_SHIELDWALL) == CAST_OK)
                    DisableCombatAction(action);
                break;
            case ZELIEK_ENRAGE:
            {
                DoBroadcastText(SAY_ZELI_SPECIAL, m_creature);
                ResetCombatAction(action, RandomTimer(5s, 120s));
                break;
            }
            case ZELIEK_SPECIAL:
                m_creature->SetSpellList(m_creature->GetCreatureInfo()->SpellList + 1);
                DisableCombatAction(action);
                break;
        }
    }
};

// 28832 - Mark of Korth'azz
// 28833 - Mark of Blaumeux
// 28834 - Mark of Rivendare
// 28835 - Mark of Zeliek
struct HorsemenMark : public AuraScript
{
    void OnApply(Aura* aura, bool apply) const override
    {
        if (!apply)
            return;

        int32 damage;
        switch (aura->GetStackAmount())
        {
            case 1:
                return;
            case 2: damage = 250; break;
            case 3: damage = 1000; break;
            case 4: damage = 3000; break;
            default:
                damage = 1000 * aura->GetStackAmount();
                break;
        }

        if (Unit* caster = aura->GetCaster())
            caster->CastCustomSpell(aura->GetTarget(), 28836, &damage, nullptr, nullptr, TRIGGERED_OLD_TRIGGERED, nullptr, aura);
    }
};

void AddSC_boss_four_horsemen()
{
    Script* newScript = new Script;
    newScript->Name = "boss_lady_blaumeux";
    newScript->GetAI = &GetNewAIInstance<boss_lady_blaumeuxAI>;
    newScript->RegisterSelf();

    newScript = new Script;
    newScript->Name = "boss_alexandros_mograine";
    newScript->GetAI = &GetNewAIInstance<boss_alexandros_mograineAI>;
    newScript->RegisterSelf();

    newScript = new Script;
    newScript->Name = "boss_thane_korthazz";
    newScript->GetAI = &GetNewAIInstance<boss_thane_korthazzAI>;
    newScript->RegisterSelf();

    newScript = new Script;
    newScript->Name = "boss_sir_zeliek";
    newScript->GetAI = &GetNewAIInstance<boss_sir_zeliekAI>;
    newScript->RegisterSelf();

    RegisterSpellScript<HorsemenMark>("spell_horsemen_mark");
}
