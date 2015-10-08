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
SDName: Boss_Noth
SD%Complete: 80
SDComment: Summons need verify, need better phase-switch support (unattackable?)
SDCategory: Naxxramas
EndScriptData */

#include "precompiled.h"
#include "naxxramas.h"

enum
{
    SAY_AGGRO1                          = -1533075,
    SAY_AGGRO2                          = -1533076,
    SAY_AGGRO3                          = -1533077,
    SAY_SUMMON                          = -1533078,
    SAY_SLAY1                           = -1533079,
    SAY_SLAY2                           = -1533080,
    SAY_DEATH                           = -1533081,

    EMOTE_WARRIOR                       = -1533130,
    EMOTE_SKELETON                      = -1533131,
    EMOTE_TELEPORT                      = -1533132,
    EMOTE_TELEPORT_RETURN               = -1533133,

    SPELL_TELEPORT                      = 29216,
    SPELL_TELEPORT_RETURN               = 29231,

    SPELL_BLINK_1                       = 29208,
    SPELL_BLINK_2                       = 29209,
    SPELL_BLINK_3                       = 29210,
    SPELL_BLINK_4                       = 29211,

    SPELL_CRIPPLE                       = 29212,
    SPELL_CURSE_PLAGUEBRINGER           = 29213,

    SPELL_BERSERK                       = 26662,            // guesswork, but very common berserk spell in naxx

    SPELL_SUMMON_WARRIOR_1              = 29247,
    SPELL_SUMMON_WARRIOR_2              = 29248,
    SPELL_SUMMON_WARRIOR_3              = 29249,

    SPELL_SUMMON_WARRIOR_THREE          = 29237,

    SPELL_SUMMON_CHAMP01                = 29217,
    SPELL_SUMMON_CHAMP02                = 29224,
    SPELL_SUMMON_CHAMP03                = 29225,
    SPELL_SUMMON_CHAMP04                = 29227,
    SPELL_SUMMON_CHAMP05                = 29238,
    SPELL_SUMMON_CHAMP06                = 29255,
    SPELL_SUMMON_CHAMP07                = 29257,
    SPELL_SUMMON_CHAMP08                = 29258,
    SPELL_SUMMON_CHAMP09                = 29262,
    SPELL_SUMMON_CHAMP10                = 29267,

    SPELL_SUMMON_GUARD01                = 29226,
    SPELL_SUMMON_GUARD02                = 29239,
    SPELL_SUMMON_GUARD03                = 29256,
    SPELL_SUMMON_GUARD04                = 29268,

    PHASE_GROUND                        = 0,
    PHASE_BALCONY                       = 1,

    PHASE_SKELETON_1                    = 1,
    PHASE_SKELETON_2                    = 2,
    PHASE_SKELETON_3                    = 3
};

struct boss_nothAI : public ScriptedAI
{
    boss_nothAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = (instance_naxxramas*)pCreature->GetInstanceData();
        Reset();
    }

    instance_naxxramas* m_pInstance;

    uint8 m_uiPhase;
    uint8 m_uiPhaseSub;
    uint32 m_uiPhaseTimer;

    uint32 m_uiBlinkTimer;
    uint32 m_uiCurseTimer;
    uint32 m_uiSummonTimer;

    void Reset() override
    {
        m_uiPhase = PHASE_GROUND;
        m_uiPhaseSub = PHASE_GROUND;
        m_uiPhaseTimer = 90000;

        m_uiBlinkTimer = 25000;
        m_uiCurseTimer = 4000;
        m_uiSummonTimer = 12000;
    }

    void Aggro(Unit* /*pWho*/) override
    {
        switch (urand(0, 2))
        {
            case 0: DoScriptText(SAY_AGGRO1, m_creature); break;
            case 1: DoScriptText(SAY_AGGRO2, m_creature); break;
            case 2: DoScriptText(SAY_AGGRO3, m_creature); break;
        }

        if (m_pInstance)
            m_pInstance->SetData(TYPE_NOTH, IN_PROGRESS);
    }

    void JustSummoned(Creature* pSummoned) override
    {
        pSummoned->SetInCombatWithZone();
    }

    void KilledUnit(Unit* /*pVictim*/) override
    {
        DoScriptText(urand(0, 1) ? SAY_SLAY1 : SAY_SLAY2, m_creature);
    }

    void JustDied(Unit* /*pKiller*/) override
    {
        DoScriptText(SAY_DEATH, m_creature);

        if (m_pInstance)
            m_pInstance->SetData(TYPE_NOTH, DONE);
    }

    void JustReachedHome() override
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_NOTH, FAIL);
    }

    void SpellHit(Unit* pCaster, const SpellEntry* pSpell) override
    {
        if (pCaster == m_creature && pSpell->Effect[EFFECT_INDEX_0] == SPELL_EFFECT_LEAP)
            DoCastSpellIfCan(m_creature, SPELL_CRIPPLE);
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        if (m_uiPhase == PHASE_GROUND)
        {
            if (m_uiPhaseTimer)                             // After PHASE_SKELETON_3 we won't have a balcony phase
            {
                if (m_uiPhaseTimer <= uiDiff)
                {
                    if (DoCastSpellIfCan(m_creature, SPELL_TELEPORT) == CAST_OK)
                    {
                        DoScriptText(EMOTE_TELEPORT, m_creature);
                        m_creature->GetMotionMaster()->MoveIdle();
                        m_uiPhase = PHASE_BALCONY;
                        ++m_uiPhaseSub;

                        switch (m_uiPhaseSub)               // Set Duration of Skeleton phase
                        {
                            case PHASE_SKELETON_1: m_uiPhaseTimer = 70000;  break;
                            case PHASE_SKELETON_2: m_uiPhaseTimer = 97000;  break;
                            case PHASE_SKELETON_3: m_uiPhaseTimer = 120000; break;
                        }
                        return;
                    }
                }
                else
                    m_uiPhaseTimer -= uiDiff;
            }

            if (m_uiBlinkTimer < uiDiff)
            {
                static uint32 const auiSpellBlink[4] =
                {
                    SPELL_BLINK_1, SPELL_BLINK_2, SPELL_BLINK_3, SPELL_BLINK_4
                };

                if (DoCastSpellIfCan(m_creature, auiSpellBlink[urand(0, 3)]) == CAST_OK)
                {
                    DoResetThreat();
                    m_uiBlinkTimer = 25000;
                }
            }
            else
                m_uiBlinkTimer -= uiDiff;

            if (m_uiCurseTimer < uiDiff)
            {
                DoCastSpellIfCan(m_creature, SPELL_CURSE_PLAGUEBRINGER);
                m_uiCurseTimer = 28000;
            }
            else
                m_uiCurseTimer -= uiDiff;

            if (m_uiSummonTimer < uiDiff)
            {
                DoScriptText(SAY_SUMMON, m_creature);
                DoScriptText(EMOTE_WARRIOR, m_creature);

                // It's not very clear how many warriors it should summon, so we'll just leave it as random for now
                if (urand(0, 1))
                {
                    static uint32 const auiSpellSummonPlaguedWarrior[3] =
                    {
                        SPELL_SUMMON_WARRIOR_1, SPELL_SUMMON_WARRIOR_2, SPELL_SUMMON_WARRIOR_3
                    };

                    for (uint8 i = 0; i < 2; ++i)
                        DoCastSpellIfCan(m_creature, auiSpellSummonPlaguedWarrior[urand(0, 2)], CAST_TRIGGERED);
                }
                else
                {
                    DoCastSpellIfCan(m_creature, SPELL_SUMMON_WARRIOR_THREE, CAST_TRIGGERED);
                }

                m_uiSummonTimer = 30000;
            }
            else
                m_uiSummonTimer -= uiDiff;

            DoMeleeAttackIfReady();
        }
        else                                                // PHASE_BALCONY
        {
            if (m_uiPhaseTimer < uiDiff)
            {
                if (DoCastSpellIfCan(m_creature, SPELL_TELEPORT_RETURN) == CAST_OK)
                {
                    DoScriptText(EMOTE_TELEPORT_RETURN, m_creature);
                    m_creature->GetMotionMaster()->MoveChase(m_creature->getVictim());
                    switch (m_uiPhaseSub)
                    {
                        case PHASE_SKELETON_1: m_uiPhaseTimer = 110000; break;
                        case PHASE_SKELETON_2: m_uiPhaseTimer = 180000; break;
                        case PHASE_SKELETON_3:
                            m_uiPhaseTimer = 0;
                            // Go Berserk after third Balcony Phase
                            DoCastSpellIfCan(m_creature, SPELL_BERSERK, CAST_TRIGGERED);
                            break;
                    }
                    m_uiPhase = PHASE_GROUND;

                    return;
                }
            }
            else
                m_uiPhaseTimer -= uiDiff;

            if (m_uiSummonTimer < uiDiff)
            {
                DoScriptText(EMOTE_SKELETON, m_creature);

                static uint32 const auiSpellSummonPlaguedChampion[10] =
                {
                    SPELL_SUMMON_CHAMP01, SPELL_SUMMON_CHAMP02, SPELL_SUMMON_CHAMP03, SPELL_SUMMON_CHAMP04, SPELL_SUMMON_CHAMP05, SPELL_SUMMON_CHAMP06, SPELL_SUMMON_CHAMP07, SPELL_SUMMON_CHAMP08, SPELL_SUMMON_CHAMP09, SPELL_SUMMON_CHAMP10
                };

                static uint32 const auiSpellSummonPlaguedGuardian[4] =
                {
                    SPELL_SUMMON_GUARD01, SPELL_SUMMON_GUARD02, SPELL_SUMMON_GUARD03, SPELL_SUMMON_GUARD04
                };

                // A bit unclear how many in each sub phase
                switch (m_uiPhaseSub)
                {
                    case PHASE_SKELETON_1:
                    {
                        for (uint8 i = 0; i < 4; ++i)
                            DoCastSpellIfCan(m_creature, auiSpellSummonPlaguedChampion[urand(0, 9)], CAST_TRIGGERED);

                        break;
                    }
                    case PHASE_SKELETON_2:
                    {
                        for (uint8 i = 0; i < 2; ++i)
                        {
                            DoCastSpellIfCan(m_creature, auiSpellSummonPlaguedChampion[urand(0, 9)], CAST_TRIGGERED);
                            DoCastSpellIfCan(m_creature, auiSpellSummonPlaguedGuardian[urand(0, 3)], CAST_TRIGGERED);
                        }
                        break;
                    }
                    case PHASE_SKELETON_3:
                    {
                        for (uint8 i = 0; i < 4; ++i)
                            DoCastSpellIfCan(m_creature, auiSpellSummonPlaguedGuardian[urand(0, 3)], CAST_TRIGGERED);

                        break;
                    }
                }

                m_uiSummonTimer = 30000;
            }
            else
                m_uiSummonTimer -= uiDiff;
        }
    }
};

CreatureAI* GetAI_boss_noth(Creature* pCreature)
{
    return new boss_nothAI(pCreature);
}

void AddSC_boss_noth()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "boss_noth";
    pNewScript->GetAI = &GetAI_boss_noth;
    pNewScript->RegisterSelf();
}
