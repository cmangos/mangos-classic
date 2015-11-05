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
SD%Complete: 80
SDComment: Lady Blaumeux, Thane Korthazz, Sir Zeliek, Alexandros Mograine; Berserk NYI.
SDCategory: Naxxramas
EndScriptData */

#include "precompiled.h"
#include "naxxramas.h"

enum
{
    // ***** Yells *****
    // lady blaumeux
    SAY_BLAU_AGGRO          = -1533044,
    SAY_BLAU_SPECIAL        = -1533048,
    SAY_BLAU_SLAY           = -1533049,
    SAY_BLAU_DEATH          = -1533050,
    // EMOTE_UNYIELDING_PAIN = -1533156,

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
    // EMOTE_CONDEMATION     = -1533157,

    // ***** Spells *****
    // all horsemen
    SPELL_SHIELDWALL        = 29061,
    SPELL_BESERK            = 26662,
    // Note: Berserk should be applied once 100 marks are casted.

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
};

struct boss_lady_blaumeuxAI : public ScriptedAI
{
    boss_lady_blaumeuxAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
        Reset();
    }

    ScriptedInstance* m_pInstance;

    uint32 m_uiMarkTimer;
    uint32 m_uiVoidZoneTimer;
    float m_fHealthCheck;

    void Reset() override
    {
        m_uiMarkTimer       = 20000;
        m_uiVoidZoneTimer   = 15000;
        m_fHealthCheck      = 50.0f;
    }

    void Aggro(Unit* /*pWho*/) override
    {
        DoScriptText(SAY_BLAU_AGGRO, m_creature);

        if (m_pInstance)
            m_pInstance->SetData(TYPE_FOUR_HORSEMEN, IN_PROGRESS);
    }

    void KilledUnit(Unit* /*pVictim*/) override
    {
        DoScriptText(SAY_BLAU_SLAY, m_creature);
    }

    void JustDied(Unit* /*pKiller*/) override
    {
        DoScriptText(SAY_BLAU_DEATH, m_creature);
        DoCastSpellIfCan(m_creature, SPELL_SPIRIT_BLAUMEUX, CAST_TRIGGERED);

        if (m_pInstance)
            m_pInstance->SetData(TYPE_FOUR_HORSEMEN, SPECIAL);
    }

    void JustReachedHome() override
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_FOUR_HORSEMEN, FAIL);
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        if (m_creature->GetHealthPercent() <= m_fHealthCheck)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_SHIELDWALL) == CAST_OK)
                m_fHealthCheck -= 30.0f;
        }

        if (m_uiMarkTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_MARK_OF_BLAUMEUX) == CAST_OK)
                m_uiMarkTimer = 12000;
        }
        else
            m_uiMarkTimer -= uiDiff;

        if (m_uiVoidZoneTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_VOID_ZONE) == CAST_OK)
                m_uiVoidZoneTimer = 15000;
        }
        else
            m_uiVoidZoneTimer -= uiDiff;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_lady_blaumeux(Creature* pCreature)
{
    return new boss_lady_blaumeuxAI(pCreature);
}

struct boss_alexandros_mograineAI : public ScriptedAI
{
    boss_alexandros_mograineAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
        Reset();
    }

    ScriptedInstance* m_pInstance;

    uint32 m_uiMarkTimer;
    uint32 m_uiRighteousFireTimer;
    float m_fHealthCheck;

    void Reset() override
    {
        m_uiMarkTimer          = 20000;
        m_uiRighteousFireTimer = 15000;
        m_fHealthCheck         = 50.0f;
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

    void JustDied(Unit* /*pKiller*/) override
    {
        DoScriptText(SAY_MORG_DEATH, m_creature);
        DoCastSpellIfCan(m_creature, SPELL_SPIRIT_MOGRAINE, CAST_TRIGGERED);

        if (m_pInstance)
            m_pInstance->SetData(TYPE_FOUR_HORSEMEN, SPECIAL);
    }

    void JustReachedHome() override
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_FOUR_HORSEMEN, FAIL);
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        if (m_creature->GetHealthPercent() <= m_fHealthCheck)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_SHIELDWALL) == CAST_OK)
                m_fHealthCheck -= 30.0f;
        }

        if (m_uiMarkTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_MARK_OF_MOGRAINE) == CAST_OK)
                m_uiMarkTimer = 12000;
        }
        else
            m_uiMarkTimer -= uiDiff;

        if (m_uiRighteousFireTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_RIGHTEOUS_FIRE) == CAST_OK)
                m_uiRighteousFireTimer = 15000;
        }
        else
            m_uiRighteousFireTimer -= uiDiff;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_alexandros_mograine(Creature* pCreature)
{
    return new boss_alexandros_mograineAI(pCreature);
}

struct boss_thane_korthazzAI : public ScriptedAI
{
    boss_thane_korthazzAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
        Reset();
    }

    ScriptedInstance* m_pInstance;

    uint32 m_uiMarkTimer;
    uint32 m_uiMeteorTimer;
    float m_fHealthCheck;

    void Reset() override
    {
        m_uiMarkTimer       = 20000;
        m_uiMeteorTimer     = 30000;
        m_fHealthCheck      = 50.0f;
    }

    void Aggro(Unit* /*pWho*/) override
    {
        DoScriptText(SAY_KORT_AGGRO, m_creature);

        if (m_pInstance)
            m_pInstance->SetData(TYPE_FOUR_HORSEMEN, IN_PROGRESS);
    }

    void KilledUnit(Unit* /*pVictim*/) override
    {
        DoScriptText(SAY_KORT_SLAY, m_creature);
    }

    void JustDied(Unit* /*pKiller*/) override
    {
        DoScriptText(SAY_KORT_DEATH, m_creature);
        DoCastSpellIfCan(m_creature, SPELL_SPIRIT_KORTHAZZ, CAST_TRIGGERED);

        if (m_pInstance)
            m_pInstance->SetData(TYPE_FOUR_HORSEMEN, SPECIAL);
    }

    void JustReachedHome() override
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_FOUR_HORSEMEN, FAIL);
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        if (m_creature->GetHealthPercent() <= m_fHealthCheck)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_SHIELDWALL) == CAST_OK)
                m_fHealthCheck -= 30.0f;
        }

        if (m_uiMarkTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_MARK_OF_KORTHAZZ) == CAST_OK)
                m_uiMarkTimer = 12000;
        }
        else
            m_uiMarkTimer -= uiDiff;

        if (m_uiMeteorTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_METEOR) == CAST_OK)
                m_uiMeteorTimer = 20000;
        }
        else
            m_uiMeteorTimer -= uiDiff;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_thane_korthazz(Creature* pCreature)
{
    return new boss_thane_korthazzAI(pCreature);
}

struct boss_sir_zeliekAI : public ScriptedAI
{
    boss_sir_zeliekAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
        Reset();
    }

    ScriptedInstance* m_pInstance;

    uint32 m_uiMarkTimer;
    uint32 m_uiHolyWrathTimer;
    float m_fHealthCheck;

    void Reset() override
    {
        m_uiMarkTimer       = 20000;
        m_uiHolyWrathTimer  = 12000;
        m_fHealthCheck      = 50.0f;
    }

    void Aggro(Unit* /*pWho*/) override
    {
        DoScriptText(SAY_ZELI_AGGRO, m_creature);

        if (m_pInstance)
            m_pInstance->SetData(TYPE_FOUR_HORSEMEN, IN_PROGRESS);
    }

    void KilledUnit(Unit* /*pVictim*/) override
    {
        DoScriptText(SAY_ZELI_SLAY, m_creature);
    }

    void JustDied(Unit* /*pKiller*/) override
    {
        DoScriptText(SAY_ZELI_DEATH, m_creature);
        DoCastSpellIfCan(m_creature, SPELL_SPIRIT_ZELIEK, CAST_TRIGGERED);

        if (m_pInstance)
            m_pInstance->SetData(TYPE_FOUR_HORSEMEN, SPECIAL);
    }

    void JustReachedHome() override
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_FOUR_HORSEMEN, FAIL);
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        if (m_creature->GetHealthPercent() <= m_fHealthCheck)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_SHIELDWALL) == CAST_OK)
                m_fHealthCheck -= 30.0f;
        }

        if (m_uiMarkTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_MARK_OF_ZELIEK) == CAST_OK)
                m_uiMarkTimer = 12000;
        }
        else
            m_uiMarkTimer -= uiDiff;

        if (m_uiHolyWrathTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_HOLY_WRATH) == CAST_OK)
                m_uiHolyWrathTimer = 15000;
        }
        else
            m_uiHolyWrathTimer -= uiDiff;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_sir_zeliek(Creature* pCreature)
{
    return new boss_sir_zeliekAI(pCreature);
}

void AddSC_boss_four_horsemen()
{
    Script* pNewScript;

    pNewScript = new Script;
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
