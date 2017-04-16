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
SDName: Boss_Nefarian
SD%Complete: 80
SDComment: Some issues with class calls effecting more than one class
SDCategory: Blackwing Lair
EndScriptData */

#include "precompiled.h"
#include "blackwing_lair.h"
#include "TemporarySummon.h"

enum
{
    SAY_XHEALTH                 = -1469008,             // at 5% hp
    SAY_AGGRO                   = -1469009,
    SAY_RAISE_SKELETONS         = -1469010,
    SAY_SLAY                    = -1469011,
    SAY_DEATH                   = -1469012,

    SAY_MAGE                    = -1469013,
    SAY_WARRIOR                 = -1469014,
    SAY_DRUID                   = -1469015,
    SAY_PRIEST                  = -1469016,
    SAY_PALADIN                 = -1469017,
    SAY_SHAMAN                  = -1469018,
    SAY_WARLOCK                 = -1469019,
    SAY_HUNTER                  = -1469020,
    SAY_ROGUE                   = -1469021,
    SAY_DEATH_KNIGHT            = -1469031,             // spell unk for the moment

    SPELL_SHADOWFLAME_INITIAL   = 22992,                // old spell id 22972 -> wrong
    SPELL_SHADOWFLAME           = 22539,
    SPELL_BELLOWING_ROAR        = 22686,
    SPELL_VEIL_OF_SHADOW        = 22687,                // old spell id 7068 -> wrong
    SPELL_CLEAVE                = 20691,
    SPELL_TAIL_LASH             = 23364,
    // SPELL_BONE_CONTRUST       = 23363,                // 23362, 23361   Missing from DBC!

    SPELL_MAGE                  = 23410,                // wild magic
    SPELL_WARRIOR               = 23397,                // beserk
    SPELL_DRUID                 = 23398,                // cat form
    SPELL_PRIEST                = 23401,                // corrupted healing
    SPELL_PALADIN               = 23418,                // syphon blessing
    SPELL_SHAMAN                = 23425,                // totems
    SPELL_WARLOCK               = 23427,                // infernals    -> should trigger 23426
    SPELL_HUNTER                = 23436,                // bow broke
    SPELL_ROGUE                 = 23414,                // Paralise
};

struct boss_nefarianAI : public ScriptedAI
{
    boss_nefarianAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
        Reset();
    }

    ScriptedInstance* m_pInstance;

    uint32 m_uiShadowFlameTimer;
    uint32 m_uiBellowingRoarTimer;
    uint32 m_uiVeilOfShadowTimer;
    uint32 m_uiCleaveTimer;
    uint32 m_uiTailLashTimer;
    uint32 m_uiClassCallTimer;
    bool m_bPhase3;
    bool m_bHasEndYell;

    void Reset() override
    {
        m_uiShadowFlameTimer    = 12000;                    // These times are probably wrong
        m_uiBellowingRoarTimer  = 30000;
        m_uiVeilOfShadowTimer   = 15000;
        m_uiCleaveTimer         = 7000;
        m_uiTailLashTimer       = 10000;
        m_uiClassCallTimer      = 35000;                    // 35-40 seconds
        m_bPhase3               = false;
        m_bHasEndYell           = false;
    }

    void KilledUnit(Unit* pVictim) override
    {
        if (urand(0, 4))
            return;

        DoScriptText(SAY_SLAY, m_creature, pVictim);
    }

    void JustDied(Unit* /*pKiller*/) override
    {
        DoScriptText(SAY_DEATH, m_creature);

        if (m_pInstance)
            m_pInstance->SetData(TYPE_NEFARIAN, DONE);
    }

    void JustReachedHome() override
    {
        if (m_pInstance)
        {
            m_pInstance->SetData(TYPE_NEFARIAN, FAIL);

            // Cleanup encounter
            if (m_creature->IsTemporarySummon())
            {
                TemporarySummon* pTemporary = (TemporarySummon*)m_creature;

                if (Creature* pNefarius = m_creature->GetMap()->GetCreature(pTemporary->GetSummonerGuid()))
                    pNefarius->AI()->EnterEvadeMode();
            }

            m_creature->ForcedDespawn();
        }
    }

    void Aggro(Unit* /*pWho*/) override
    {
        DoScriptText(SAY_AGGRO, m_creature);

        // Remove flying in case Nefarian aggroes before his combat point was reached
        if (m_creature->IsLevitating())
        {
            m_creature->SetByteValue(UNIT_FIELD_BYTES_1, 3, 0);
            m_creature->SetLevitate(false);
        }

        DoCastSpellIfCan(m_creature, SPELL_SHADOWFLAME_INITIAL);
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        // ShadowFlame_Timer
        if (m_uiShadowFlameTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_SHADOWFLAME) == CAST_OK)
                m_uiShadowFlameTimer = 12000;
        }
        else
            m_uiShadowFlameTimer -= uiDiff;

        // BellowingRoar_Timer
        if (m_uiBellowingRoarTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_BELLOWING_ROAR) == CAST_OK)
                m_uiBellowingRoarTimer = 30000;
        }
        else
            m_uiBellowingRoarTimer -= uiDiff;

        // VeilOfShadow_Timer
        if (m_uiVeilOfShadowTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_VEIL_OF_SHADOW) == CAST_OK)
                m_uiVeilOfShadowTimer = 15000;
        }
        else
            m_uiVeilOfShadowTimer -= uiDiff;

        // Cleave_Timer
        if (m_uiCleaveTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_CLEAVE) == CAST_OK)
                m_uiCleaveTimer = 7000;
        }
        else
            m_uiCleaveTimer -= uiDiff;

        // TailLash_Timer
        if (m_uiTailLashTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_TAIL_LASH) == CAST_OK)
                m_uiTailLashTimer = 10000;
        }
        else
            m_uiTailLashTimer -= uiDiff;

        // ClassCall_Timer
        if (m_uiClassCallTimer < uiDiff)
        {
            // Cast a random class call
            // On official it is based on what classes are currently on the hostil list
            // but we can't do that yet so just randomly call one

            switch (urand(0, 8))
            {
                case 0:
                    DoScriptText(SAY_MAGE, m_creature);
                    DoCastSpellIfCan(m_creature, SPELL_MAGE);
                    break;
                case 1:
                    DoScriptText(SAY_WARRIOR, m_creature);
                    DoCastSpellIfCan(m_creature, SPELL_WARRIOR);
                    break;
                case 2:
                    DoScriptText(SAY_DRUID, m_creature);
                    DoCastSpellIfCan(m_creature, SPELL_DRUID);
                    break;
                case 3:
                    DoScriptText(SAY_PRIEST, m_creature);
                    DoCastSpellIfCan(m_creature, SPELL_PRIEST);
                    break;
                case 4:
                    DoScriptText(SAY_PALADIN, m_creature);
                    DoCastSpellIfCan(m_creature, SPELL_PALADIN);
                    break;
                case 5:
                    DoScriptText(SAY_SHAMAN, m_creature);
                    DoCastSpellIfCan(m_creature, SPELL_SHAMAN);
                    break;
                case 6:
                    DoScriptText(SAY_WARLOCK, m_creature);
                    DoCastSpellIfCan(m_creature, SPELL_WARLOCK);
                    break;
                case 7:
                    DoScriptText(SAY_HUNTER, m_creature);
                    DoCastSpellIfCan(m_creature, SPELL_HUNTER);
                    break;
                case 8:
                    DoScriptText(SAY_ROGUE, m_creature);
                    DoCastSpellIfCan(m_creature, SPELL_ROGUE);
                    break;
            }

            m_uiClassCallTimer = urand(35000, 40000);
        }
        else
            m_uiClassCallTimer -= uiDiff;

        // Phase3 begins when we are below X health
        if (!m_bPhase3 && m_creature->GetHealthPercent() < 20.0f)
        {
            if (m_pInstance)
                m_pInstance->SetData(TYPE_NEFARIAN, SPECIAL);
            m_bPhase3 = true;
            DoScriptText(SAY_RAISE_SKELETONS, m_creature);
        }

        // 5% hp yell
        if (!m_bHasEndYell && m_creature->GetHealthPercent() < 5.0f)
        {
            m_bHasEndYell = true;
            DoScriptText(SAY_XHEALTH, m_creature);
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_nefarian(Creature* pCreature)
{
    return new boss_nefarianAI(pCreature);
}

void AddSC_boss_nefarian()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "boss_nefarian";
    pNewScript->GetAI = &GetAI_boss_nefarian;
    pNewScript->RegisterSelf();
}
