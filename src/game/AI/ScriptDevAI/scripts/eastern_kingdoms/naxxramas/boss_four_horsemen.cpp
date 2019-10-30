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

#include "AI/ScriptDevAI/include/precompiled.h"
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

struct boss_horsmenAI : public ScriptedAI
{
    boss_horsmenAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
        Reset();
    }

    ScriptedInstance* m_pInstance;

    uint32 m_uiMarkTimer;
    uint32 m_uiMarkCounter;
    uint32 m_uiHorsmenIndex;            // Used to differentiate each of the Four Horsemen in the children classes

    float m_fHealthCheck;

    void Reset() override
    {
        m_uiMarkTimer           = 20 * IN_MILLISECONDS;
        m_uiMarkCounter         = 0;
        m_uiHorsmenIndex        = INDEX_BLAUMEUX;		// Default: Lady Blaumeux (1: Highlord Morgraine, 2: Thane Korth'azz, 3: Sir Zeliek)

        m_fHealthCheck          = 50.0f;
    }

    void Aggro(Unit* /*pWho*/) override
    {
        DoScriptText(aggroSayList[m_uiHorsmenIndex], m_creature);

        if (m_pInstance)
            m_pInstance->SetData(TYPE_FOUR_HORSEMEN, IN_PROGRESS);
    }

    void KilledUnit(Unit* /*pVictim*/) override
    {
        DoScriptText(killSayList[m_uiHorsmenIndex], m_creature);
    }

    void JustDied(Unit* /*pKiller*/) override
    {
        DoScriptText(deathSayList[m_uiHorsmenIndex], m_creature);
        DoCastSpellIfCan(m_creature, spiritSpellList[m_uiHorsmenIndex], CAST_TRIGGERED);

        if (m_pInstance)
            m_pInstance->SetData(TYPE_FOUR_HORSEMEN, SPECIAL);
    }

    void JustReachedHome() override
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_FOUR_HORSEMEN, FAIL);
    }

    virtual void UpdateHorsmenAI(const uint32 /*uiDiff*/) {}

    void UpdateAI(const uint32 uiDiff) override
    {
        // Do nothing if no target
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        // Call Horsemen specific virtual function to cast special spells
        UpdateHorsmenAI(uiDiff);

        if (m_creature->GetHealthPercent() <= m_fHealthCheck)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_SHIELDWALL) == CAST_OK)
                m_fHealthCheck -= 30.0f;
        }

        if (m_uiMarkTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, markSpellList[m_uiHorsmenIndex]) == CAST_OK)
            {
                m_uiMarkCounter++;
                m_uiMarkTimer = 12 * IN_MILLISECONDS;
            }
        }
        else
            m_uiMarkTimer -= uiDiff;

        DoMeleeAttackIfReady();
    }
};

UnitAI* GetAI_boss_horsmen(Creature* pCreature)
{
    return new boss_horsmenAI(pCreature);
}

struct boss_lady_blaumeuxAI : public boss_horsmenAI
{
    boss_lady_blaumeuxAI(Creature* pCreature) : boss_horsmenAI(pCreature) { Reset(); }

    uint32 m_uiVoidZoneTimer;

    void Reset() override
    {
        boss_horsmenAI::Reset();

        m_uiVoidZoneTimer = 15000;
        // No need to define m_uiHorsmenIndex as Blaumeux is default index
    }

    void UpdateHorsmenAI(const uint32 uiDiff) override
    {
        if (m_uiVoidZoneTimer < uiDiff)
        {
            Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0, SPELL_VOID_ZONE, SELECT_FLAG_PLAYER);
            if (pTarget)
            {
                if (DoCastSpellIfCan(pTarget, SPELL_VOID_ZONE) == CAST_OK)
                {
                    m_uiVoidZoneTimer = (m_uiMarkCounter < MAX_MARK_STACKS ? 12 : 1 ) * IN_MILLISECONDS;
                    if (urand(0 ,9) < 1)
                        DoScriptText(SAY_BLAU_SPECIAL, m_creature);
                }
            }
        }
        else
            m_uiVoidZoneTimer -= uiDiff;
    }
};

UnitAI* GetAI_boss_lady_blaumeux(Creature* pCreature)
{
    return new boss_lady_blaumeuxAI(pCreature);
}

struct boss_alexandros_mograineAI : public boss_horsmenAI
{
    boss_alexandros_mograineAI(Creature* pCreature) : boss_horsmenAI(pCreature) { Reset(); }

    uint32 m_uiRighteousFireTimer;

    void Reset() override
    {
        boss_horsmenAI::Reset();

        m_uiRighteousFireTimer = 15 * IN_MILLISECONDS;
        m_uiHorsmenIndex       = INDEX_MORGRAINE;
    }

    void Aggro(Unit* /*pWho*/) override
    {
        switch (urand(0, 2))
        {
            case 0: DoScriptText(SAY_MORG_AGGRO1, m_creature); break;
            case 1: DoScriptText(SAY_MORG_AGGRO2, m_creature); break;
            case 2: DoScriptText(SAY_MORG_AGGRO3, m_creature); break;
        }

        if (m_pInstance)
            m_pInstance->SetData(TYPE_FOUR_HORSEMEN, IN_PROGRESS);
    }

    void KilledUnit(Unit* /*pVictim*/) override
    {
        DoScriptText(urand(0, 1) ? SAY_MORG_SLAY1 : SAY_MORG_SLAY2, m_creature);
    }

    void UpdateHorsmenAI(const uint32 uiDiff) override
    {
        if (m_uiRighteousFireTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_RIGHTEOUS_FIRE) == CAST_OK)
            {
                m_uiRighteousFireTimer = (m_uiMarkCounter < MAX_MARK_STACKS ? 15 : 1) * IN_MILLISECONDS;
                if (urand(0 ,9) < 1)
                    DoScriptText(SAY_MORG_SPECIAL, m_creature);
            }
        }
        else
            m_uiRighteousFireTimer -= uiDiff;
    }
};

UnitAI* GetAI_boss_alexandros_mograine(Creature* pCreature)
{
    return new boss_alexandros_mograineAI(pCreature);
}

struct boss_thane_korthazzAI : public boss_horsmenAI
{
    boss_thane_korthazzAI(Creature* pCreature) : boss_horsmenAI(pCreature) { Reset(); }

    uint32 m_uiMeteorTimer;

    void Reset() override
    {
        boss_horsmenAI::Reset();

        m_uiMeteorTimer     = 30 * IN_MILLISECONDS;
        m_uiHorsmenIndex    = INDEX_KORTHAZZ;
    }

    void UpdateHorsmenAI(const uint32 uiDiff) override
    {
        if (m_uiMeteorTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_METEOR) == CAST_OK)
            {
                m_uiMeteorTimer = (m_uiMarkCounter < MAX_MARK_STACKS ? 20 : 1) * IN_MILLISECONDS;
                if (urand(0 ,9) < 1)
                    DoScriptText(SAY_KORT_SPECIAL, m_creature);
            }
        }
        else
            m_uiMeteorTimer -= uiDiff;
    }
};

UnitAI* GetAI_boss_thane_korthazz(Creature* pCreature)
{
    return new boss_thane_korthazzAI(pCreature);
}

struct boss_sir_zeliekAI : public boss_horsmenAI
{
    boss_sir_zeliekAI(Creature* pCreature) : boss_horsmenAI(pCreature) { Reset(); }

    uint32 m_uiHolyWrathTimer;

    void Reset() override
    {
        boss_horsmenAI::Reset();

        m_uiHolyWrathTimer  = 12 * IN_MILLISECONDS;
        m_uiHorsmenIndex    = INDEX_ZELIEK;
    }

    void UpdateHorsmenAI(const uint32 uiDiff) override
    {
        if (m_uiHolyWrathTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_HOLY_WRATH) == CAST_OK)
            {
                m_uiHolyWrathTimer = (m_uiMarkCounter < MAX_MARK_STACKS ? 12 : 1) * IN_MILLISECONDS;
                if (urand(0 ,9) < 1)
                    DoScriptText(SAY_ZELI_SPECIAL, m_creature);
            }
        }
        else
            m_uiHolyWrathTimer -= uiDiff;
    }
};

UnitAI* GetAI_boss_sir_zeliek(Creature* pCreature)
{
    return new boss_sir_zeliekAI(pCreature);
}

void AddSC_boss_four_horsemen()
{
    Script* pNewScript = new Script;
    pNewScript->Name = "boss_lady_blaumeux";
    pNewScript->GetAI = &GetAI_boss_lady_blaumeux;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "boss_alexandros_mograine";
    pNewScript->GetAI = &GetAI_boss_alexandros_mograine;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "boss_thane_korthazz";
    pNewScript->GetAI = &GetAI_boss_thane_korthazz;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "boss_sir_zeliek";
    pNewScript->GetAI = &GetAI_boss_sir_zeliek;
    pNewScript->RegisterSelf();
}
