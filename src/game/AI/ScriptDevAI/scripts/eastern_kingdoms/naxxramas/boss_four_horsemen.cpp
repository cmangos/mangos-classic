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
#include "naxxramas.h"

enum
{
    // ***** Yells *****
    // lady blaumeux
    SAY_BLAU_AGGRO          = -1533044,
    SAY_BLAU_SPECIAL        = -1533048,
    SAY_BLAU_SLAY           = -1533049,
    SAY_BLAU_DEATH          = -1533050,

    // alexandros mograine
    SAY_MORG_AGGRO1         = -1533065,
    SAY_MORG_AGGRO2         = -1533066,
    SAY_MORG_AGGRO3         = -1533067,
    SAY_MORG_SLAY1          = -1533068,
    SAY_MORG_SLAY2          = -1533069,
    SAY_MORG_SPECIAL        = -1533070,
    SAY_MORG_DEATH          = -1533074,

    // thane korthazz
    SAY_KORT_AGGRO          = -1533051,
    SAY_KORT_SPECIAL        = -1533055,
    SAY_KORT_SLAY           = -1533056,
    SAY_KORT_DEATH          = -1533057,

    // sir zeliek
    SAY_ZELI_AGGRO          = -1533058,
    SAY_ZELI_SPECIAL        = -1533062,
    SAY_ZELI_SLAY           = -1533063,
    SAY_ZELI_DEATH          = -1533064,

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
static const Location2DPoint resetCoords[3] = {
        {2577.3f, -3024.0f},
        {2585.9f, -3015.1f},
        {2594.6f, -3006.7f}
};
struct boss_horsmenAI : public ScriptedAI
{
    boss_horsmenAI(Creature* creature) : ScriptedAI(creature)
    {
        m_instance = (ScriptedInstance*)creature->GetInstanceData();
        m_creature->GetCombatManager().SetLeashingCheck([&](Unit*, float x, float y, float /*z*/)
        {
            return (x > resetCoords[0].x && y < resetCoords[0].y) ||
                   (x > resetCoords[1].x && y < resetCoords[1].y) ||
                   (x > resetCoords[2].x && y < resetCoords[2].y);
        });
        Reset();
    }

    ScriptedInstance* m_instance;

    uint32 m_markTimer;
    uint32 m_markCounter;
    uint32 m_horsmenIndex;            // Used to differentiate each of the Four Horsemen in the children classes

    float m_healthPercentageCheck;

    void Reset() override
    {
        m_markTimer             = 20 * IN_MILLISECONDS;
        m_markCounter           = 0;
        m_horsmenIndex          = INDEX_BLAUMEUX;       // Default: Lady Blaumeux (1: Highlord Morgraine, 2: Thane Korth'azz, 3: Sir Zeliek)

        m_healthPercentageCheck = 50.0f;
    }

    void Aggro(Unit* /*who*/) override
    {
        DoScriptText(aggroSayList[m_horsmenIndex], m_creature);

        if (m_instance)
            m_instance->SetData(TYPE_FOUR_HORSEMEN, IN_PROGRESS);
    }

    void KilledUnit(Unit* /*victim*/) override
    {
        DoScriptText(killSayList[m_horsmenIndex], m_creature);
    }

    void JustDied(Unit* /*killer*/) override
    {
        DoScriptText(deathSayList[m_horsmenIndex], m_creature);
        DoCastSpellIfCan(m_creature, spiritSpellList[m_horsmenIndex], CAST_TRIGGERED);

        if (m_instance)
            m_instance->SetData(TYPE_FOUR_HORSEMEN, SPECIAL);
    }

    void EnterEvadeMode() override
    {
        ScriptedAI::EnterEvadeMode();

        if (m_instance)
            m_instance->SetData(TYPE_FOUR_HORSEMEN, FAIL);
    }

    virtual void UpdateHorsmenAI(const uint32 /*diff*/) {}

    void UpdateAI(const uint32 diff) override
    {
        // Do nothing if no target
        if (!m_creature->SelectHostileTarget() || !m_creature->GetVictim())
            return;

        // Call Horsemen specific virtual function to cast special spells
        UpdateHorsmenAI(diff);

        if (m_creature->GetHealthPercent() <= m_healthPercentageCheck)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_SHIELDWALL) == CAST_OK)
                m_healthPercentageCheck -= 30.0f;
        }

        if (m_markTimer < diff)
        {
            if (DoCastSpellIfCan(m_creature, markSpellList[m_horsmenIndex]) == CAST_OK)
            {
                m_markCounter++;
                m_markTimer = 12 * IN_MILLISECONDS;
            }
        }
        else
            m_markTimer -= diff;

        DoMeleeAttackIfReady();
    }
};

UnitAI* GetAI_boss_horsmen(Creature* creature)
{
    return new boss_horsmenAI(creature);
}

struct boss_lady_blaumeuxAI : public boss_horsmenAI
{
    boss_lady_blaumeuxAI(Creature* creature) : boss_horsmenAI(creature) { Reset(); }

    uint32 m_voidZoneTimer;

    void Reset() override
    {
        boss_horsmenAI::Reset();

        m_voidZoneTimer = 15000;
        // No need to define m_horsmenIndex as Blaumeux is default index
    }

    void UpdateHorsmenAI(const uint32 diff) override
    {
        if (m_voidZoneTimer < diff)
        {
            Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0, SPELL_VOID_ZONE, SELECT_FLAG_PLAYER);
            if (pTarget)
            {
                if (DoCastSpellIfCan(pTarget, SPELL_VOID_ZONE) == CAST_OK)
                {
                    m_voidZoneTimer = (m_markCounter < MAX_MARK_STACKS ? 12 : 1 ) * IN_MILLISECONDS;
                    if (urand(0 ,9) < 1)
                        DoScriptText(SAY_BLAU_SPECIAL, m_creature);
                }
            }
        }
        else
            m_voidZoneTimer -= diff;
    }
};

UnitAI* GetAI_boss_lady_blaumeux(Creature* creature)
{
    return new boss_lady_blaumeuxAI(creature);
}

struct boss_alexandros_mograineAI : public boss_horsmenAI
{
    boss_alexandros_mograineAI(Creature* creature) : boss_horsmenAI(creature) { Reset(); }

    uint32 m_righteousFireTimer;

    void Reset() override
    {
        boss_horsmenAI::Reset();

        m_righteousFireTimer = 15 * IN_MILLISECONDS;
        m_horsmenIndex       = INDEX_MORGRAINE;
    }

    void Aggro(Unit* /*pWho*/) override
    {
        switch (urand(0, 2))
        {
            case 0: DoScriptText(SAY_MORG_AGGRO1, m_creature); break;
            case 1: DoScriptText(SAY_MORG_AGGRO2, m_creature); break;
            case 2: DoScriptText(SAY_MORG_AGGRO3, m_creature); break;
        }

        if (m_instance)
            m_instance->SetData(TYPE_FOUR_HORSEMEN, IN_PROGRESS);
    }

    void KilledUnit(Unit* /*pVictim*/) override
    {
        DoScriptText(urand(0, 1) ? SAY_MORG_SLAY1 : SAY_MORG_SLAY2, m_creature);
    }

    void UpdateHorsmenAI(const uint32 diff) override
    {
        if (m_righteousFireTimer < diff)
        {
            if (DoCastSpellIfCan(m_creature->GetVictim(), SPELL_RIGHTEOUS_FIRE) == CAST_OK)
            {
                m_righteousFireTimer = (m_markCounter < MAX_MARK_STACKS ? 15 : 1) * IN_MILLISECONDS;
                if (urand(0 ,9) < 1)
                    DoScriptText(SAY_MORG_SPECIAL, m_creature);
            }
        }
        else
            m_righteousFireTimer -= diff;
    }
};

UnitAI* GetAI_boss_alexandros_mograine(Creature* creature)
{
    return new boss_alexandros_mograineAI(creature);
}

struct boss_thane_korthazzAI : public boss_horsmenAI
{
    boss_thane_korthazzAI(Creature* creature) : boss_horsmenAI(creature) { Reset(); }

    uint32 m_meteorTimer;

    void Reset() override
    {
        boss_horsmenAI::Reset();

        m_meteorTimer     = 30 * IN_MILLISECONDS;
        m_horsmenIndex    = INDEX_KORTHAZZ;
    }

    void UpdateHorsmenAI(const uint32 diff) override
    {
        if (m_meteorTimer < diff)
        {
            if (DoCastSpellIfCan(m_creature->GetVictim(), SPELL_METEOR) == CAST_OK)
            {
                m_meteorTimer = (m_markCounter < MAX_MARK_STACKS ? 20 : 1) * IN_MILLISECONDS;
                if (urand(0 ,9) < 1)
                    DoScriptText(SAY_KORT_SPECIAL, m_creature);
            }
        }
        else
            m_meteorTimer -= diff;
    }
};

UnitAI* GetAI_boss_thane_korthazz(Creature* creature)
{
    return new boss_thane_korthazzAI(creature);
}

struct boss_sir_zeliekAI : public boss_horsmenAI
{
    boss_sir_zeliekAI(Creature* creature) : boss_horsmenAI(creature) { Reset(); }

    uint32 m_holyWrathTimer;

    void Reset() override
    {
        boss_horsmenAI::Reset();

        m_holyWrathTimer  = 12 * IN_MILLISECONDS;
        m_horsmenIndex    = INDEX_ZELIEK;
    }

    void UpdateHorsmenAI(const uint32 diff) override
    {
        if (m_holyWrathTimer < diff)
        {
            if (DoCastSpellIfCan(m_creature->GetVictim(), SPELL_HOLY_WRATH) == CAST_OK)
            {
                m_holyWrathTimer = (m_markCounter < MAX_MARK_STACKS ? 12 : 1) * IN_MILLISECONDS;
                if (urand(0 ,9) < 1)
                    DoScriptText(SAY_ZELI_SPECIAL, m_creature);
            }
        }
        else
            m_holyWrathTimer -= diff;
    }
};

UnitAI* GetAI_boss_sir_zeliek(Creature* creature)
{
    return new boss_sir_zeliekAI(creature);
}

void AddSC_boss_four_horsemen()
{
    Script* newScript = new Script;
    newScript->Name = "boss_lady_blaumeux";
    newScript->GetAI = &GetAI_boss_lady_blaumeux;
    newScript->RegisterSelf();

    newScript = new Script;
    newScript->Name = "boss_alexandros_mograine";
    newScript->GetAI = &GetAI_boss_alexandros_mograine;
    newScript->RegisterSelf();

    newScript = new Script;
    newScript->Name = "boss_thane_korthazz";
    newScript->GetAI = &GetAI_boss_thane_korthazz;
    newScript->RegisterSelf();

    newScript = new Script;
    newScript->Name = "boss_sir_zeliek";
    newScript->GetAI = &GetAI_boss_sir_zeliek;
    newScript->RegisterSelf();
}
