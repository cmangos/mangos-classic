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
SD%Complete: 100
SDComment:
SDCategory: Naxxramas
EndScriptData */

#include "AI/ScriptDevAI/base/CombatAI.h"
#include "AI/ScriptDevAI/include/sc_common.h"
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

    SPELL_TELEPORT                      = 29216,
    SPELL_TELEPORT_RETURN               = 29231,

    SPELL_BLINK_1                       = 29208,
    SPELL_BLINK_2                       = 29209,
    SPELL_BLINK_3                       = 29210,
    SPELL_BLINK_4                       = 29211,

    SPELL_CRIPPLE                       = 29212,
    SPELL_CURSE_PLAGUEBRINGER           = 29213,

    SPELL_IMMUNE_ALL                    = 29230,            // Cast during balcony phase to make Noth unattackable

    SPELL_SUMMON_WARRIOR_1              = 29247,            // Summon NPC 16984 (Plagued Warrior)
    SPELL_SUMMON_WARRIOR_2              = 29248,
    SPELL_SUMMON_WARRIOR_3              = 29249,

    // SW corner
    SPELL_SUMMON_CHAMP01                = 29217,            // Summon NPC 16983 (Plagued Champion)
    SPELL_SUMMON_CHAMP02                = 29227,

    // NW corner
    SPELL_SUMMON_CHAMP03                = 29224,
    SPELL_SUMMON_CHAMP04                = 29225,

    // NE corner
    SPELL_SUMMON_CHAMP05                = 29258,
    SPELL_SUMMON_CHAMP06                = 29262,

    // West
    SPELL_SUMMON_CHAMP07                = 29255,
    SPELL_SUMMON_CHAMP08                = 29257,
    SPELL_SUMMON_CHAMP09                = 29267,
    SPELL_SUMMON_CHAMP10                = 29238,

    // NE-NW corner
    SPELL_SUMMON_GUARD01                = 29226,            // Summon NPC 16981 (Plagued Guardian)
    SPELL_SUMMON_GUARD02                = 29239,

    // SW corner
    SPELL_SUMMON_GUARD03                = 29256,
    SPELL_SUMMON_GUARD04                = 29268,

    // West
    SPELL_SUMMON_CONSTR01               = 29269,            // Summon NPC 16982 (Plagued Construct)
    // NE Corner
    SPELL_SUMMON_CONSTR02               = 29240,

    SPELL_DESPAWN_SUMMONS               = 30228,            // Spell ID unsure: there are several spells doing the same thing in DBCs within Naxxramas spell IDs range
                                                            // but it is not always clear which one is for each boss so some are assigned randomly like this one
    NPC_PLAGUED_WARRIOR                 = 16984,

    PHASE_GROUND                        = 0,
    PHASE_BALCONY                       = 1,

    PHASE_SKELETON_1                    = 1,
    PHASE_SKELETON_2                    = 2,
    PHASE_SKELETON_3                    = 3
};

static uint32 const spellBlinkList[4] = { SPELL_BLINK_1, SPELL_BLINK_2, SPELL_BLINK_3, SPELL_BLINK_4 };

static uint32 const spellSummonPlaguedChampionSWList[2] = { SPELL_SUMMON_CHAMP01, SPELL_SUMMON_CHAMP02 };
static uint32 const spellSummonPlaguedChampionNWList[2] = { SPELL_SUMMON_CHAMP03, SPELL_SUMMON_CHAMP04 };
static uint32 const spellSummonPlaguedChampionNEList[2] = { SPELL_SUMMON_CHAMP05, SPELL_SUMMON_CHAMP06 };
static uint32 const spellSummonPlaguedChampionWestList[4] = { SPELL_SUMMON_CHAMP07, SPELL_SUMMON_CHAMP08, SPELL_SUMMON_CHAMP09, SPELL_SUMMON_CHAMP10 };

static uint32 const spellSummonPlaguedGuardianNENWList[2] = { SPELL_SUMMON_GUARD01, SPELL_SUMMON_GUARD02 };
static uint32 const spellSummonPlaguedGuardianSWList[2] = { SPELL_SUMMON_GUARD03, SPELL_SUMMON_GUARD04 };

enum NothActions{
    NOTH_BLINK,
    NOTH_CURSE,
    NOTH_SUMMON,
    NOTH_BALCONY_PHASE,
    NOTH_GROUND_PHASE,
    NOTH_ACTION_MAX,
};

struct boss_nothAI : public CombatAI
{
    boss_nothAI(Creature* creature) : CombatAI(creature, NOTH_ACTION_MAX), m_instance(static_cast<ScriptedInstance*>(creature->GetInstanceData()))
    {
        AddCombatAction(NOTH_BLINK, 25u * IN_MILLISECONDS);
        AddCombatAction(NOTH_CURSE, 4u * IN_MILLISECONDS);
        AddCombatAction(NOTH_SUMMON, 12u * IN_MILLISECONDS);
        AddCustomAction(NOTH_BALCONY_PHASE, true, [&]() { HandleBalconyPhase(); }, TIMER_COMBAT_COMBAT);
        AddCustomAction(NOTH_GROUND_PHASE, true, [&]() { HandleGroundPhase(); }, TIMER_COMBAT_COMBAT);
        AddOnKillText(SAY_SLAY1, SAY_SLAY1);
    }

    ScriptedInstance* m_instance;

    uint8 m_phase;
    uint8 m_phaseSub;
    uint8 m_subWavesCount;        // On balcony phase, counts if this is the first or the second wave. Used to make Noth returns early
    uint8 m_wavesNpcsCount;       // Counter for NPCs per waves during balcony phase

    void Reset() override
    {
        CombatAI::Reset();

        m_phase = PHASE_GROUND;
        m_phaseSub = PHASE_GROUND;
        m_wavesNpcsCount = 0;
        m_subWavesCount = 0;

        SetMeleeEnabled(true);
        SetCombatMovement(true);
    }

    void Aggro(Unit* /*who*/) override
    {
        switch (urand(0, 2))
        {
            case 0: DoScriptText(SAY_AGGRO1, m_creature); break;
            case 1: DoScriptText(SAY_AGGRO2, m_creature); break;
            case 2: DoScriptText(SAY_AGGRO3, m_creature); break;
        }

        if (m_instance)
            m_instance->SetData(TYPE_NOTH, IN_PROGRESS);

        ResetTimer(NOTH_BALCONY_PHASE, 90u * IN_MILLISECONDS);
    }

    void JustSummoned(Creature* summoned) override
    {
        summoned->SetInCombatWithZone();
    }

    void SummonedCreatureJustDied(Creature* summoned) override
    {
        summoned->ForcedDespawn(2000);

        // Count how many wave NPCs are killed: if all of them and this was the second wave, Noth returns early
        if (summoned->GetEntry() != NPC_PLAGUED_WARRIOR)
        {
            --m_wavesNpcsCount;
            if (m_wavesNpcsCount == 0 && m_subWavesCount == 2)
                ResetTimer(NOTH_GROUND_PHASE, 100u);
        }
    }

    void SummonedJustReachedHome(Creature* summoned) override
    {
        summoned->ForcedDespawn();
    }

    void JustDied(Unit* /*killer*/) override
    {
        DoScriptText(SAY_DEATH, m_creature);

        if (m_instance)
            m_instance->SetData(TYPE_NOTH, DONE);
    }

    void JustReachedHome() override
    {
        if (m_instance)
            m_instance->SetData(TYPE_NOTH, FAIL);

        // Clean-up stage
        DoCastSpellIfCan(m_creature, SPELL_DESPAWN_SUMMONS, CAST_TRIGGERED);
    }

    void DoSummonPlaguedChampions(bool isThirdWave)
    {
        // Four Plagued Champions are to be summoned, one in each of the three corners and the fourth randomly along the western wall
        // On the third wave, we use specific spell to also summon a Plagued Construct in West part and North-East corner
        if (DoCastSpellIfCan(m_creature, spellSummonPlaguedChampionSWList[urand(0, 1)], CAST_TRIGGERED) == CAST_OK)
            ++m_wavesNpcsCount;           // We count the addition of a new NPC on each spell cast because JustSummoned() can possibly be called more than once per summoned NPC, making the count there incorrect
        if (DoCastSpellIfCan(m_creature, spellSummonPlaguedChampionNWList[urand(0, 1)], CAST_TRIGGERED) == CAST_OK)
            ++m_wavesNpcsCount;
        if (DoCastSpellIfCan(m_creature, (isThirdWave ? SPELL_SUMMON_CONSTR02 : spellSummonPlaguedChampionNEList[urand(0, 1)]), CAST_TRIGGERED) == CAST_OK)
            (isThirdWave ? m_wavesNpcsCount += 2 : ++m_wavesNpcsCount);
        if (DoCastSpellIfCan(m_creature, (isThirdWave ? SPELL_SUMMON_CONSTR01 : spellSummonPlaguedChampionWestList[urand(0, 3)]), CAST_TRIGGERED) == CAST_OK)
            (isThirdWave ? m_wavesNpcsCount += 2 : ++m_wavesNpcsCount);
    }

    void DoSummonPlaguedGuardians()
    {
        // Two Plagued Guardians are to be summoned, one in the south-west corner and one in one of the two other corners
        if (DoCastSpellIfCan(m_creature, spellSummonPlaguedGuardianNENWList[urand(0, 1)], CAST_TRIGGERED) == CAST_OK)
            ++m_wavesNpcsCount;
        if (DoCastSpellIfCan(m_creature, spellSummonPlaguedGuardianSWList[urand(0, 1)], CAST_TRIGGERED) == CAST_OK)
            ++m_wavesNpcsCount;
    }

    void HandleBalconyPhase()
    {
        SetCombatMovement(false);
        if (DoCastSpellIfCan(m_creature, SPELL_TELEPORT) == CAST_OK)
        {
            DisableCombatAction(NOTH_CURSE);
            DisableCombatAction(NOTH_BLINK);
            DoCastSpellIfCan(m_creature, SPELL_IMMUNE_ALL, CAST_TRIGGERED); // Prevent players from damaging Noth when he is on the balcony
            SetMeleeEnabled(false);
            StopTargeting(true);
            m_creature->SetTarget(nullptr);
            ResetCombatAction(NOTH_SUMMON, 10u * IN_MILLISECONDS); // Summon first wave 10 seconds after teleport
            ++m_phaseSub;
            switch (m_phaseSub)
            {
                case PHASE_SKELETON_1: ResetTimer(NOTH_GROUND_PHASE, 70u * IN_MILLISECONDS);  break;
                case PHASE_SKELETON_2: ResetTimer(NOTH_GROUND_PHASE, 97u * IN_MILLISECONDS);  break;
                case PHASE_SKELETON_3: ResetTimer(NOTH_GROUND_PHASE, 120u * IN_MILLISECONDS); break;
            }
            m_phase = PHASE_BALCONY;
        }
        else
        {
            SetCombatMovement(true); // Restore combat movement on cast failure
            ResetTimer(NOTH_BALCONY_PHASE, 1000u);
        }
    }

    void HandleGroundPhase()
    {
        if (DoCastSpellIfCan(m_creature, SPELL_TELEPORT_RETURN, CAST_TRIGGERED) == CAST_OK)
        {
            m_creature->RemoveAurasDueToSpell(SPELL_IMMUNE_ALL);
            SetMeleeEnabled(true);
            SetCombatMovement(true);
            HandleTargetRestoration();
            ResetCombatAction(NOTH_SUMMON, 5u * IN_MILLISECONDS); // 5 seconds before summoning Plagued Warriors again
            ResetCombatAction(NOTH_CURSE, 28u * IN_MILLISECONDS);
            ResetCombatAction(NOTH_BLINK, 25u * IN_MILLISECONDS);
            m_subWavesCount = 0;
            switch (m_phaseSub)
            {
                case PHASE_SKELETON_1: ResetTimer(NOTH_BALCONY_PHASE, 110u * IN_MILLISECONDS); break;
                case PHASE_SKELETON_2: ResetTimer(NOTH_BALCONY_PHASE, 180u * IN_MILLISECONDS); break;
                case PHASE_SKELETON_3: DisableTimer(NOTH_BALCONY_PHASE); break;
            }
            m_phase = PHASE_GROUND;
        }
        else
        {
            ResetTimer(NOTH_GROUND_PHASE, 1000u);
        }
    }

    void ExecuteAction(uint32 action) override
    {
        switch (action)
        {
            case NOTH_CURSE:
            {
                DoCastSpellIfCan(m_creature, SPELL_CURSE_PLAGUEBRINGER);
                ResetCombatAction(action, 28u * IN_MILLISECONDS);
                break;
            }
            case NOTH_BLINK:
            {
                DoCastSpellIfCan(m_creature, SPELL_CRIPPLE);
                if (DoCastSpellIfCan(m_creature, spellBlinkList[urand(0, 3)]) == CAST_OK)
                {
                    DoResetThreat();
                    ResetCombatAction(action, 25u * IN_MILLISECONDS);
                }
                break;
            }
            case NOTH_SUMMON:
            {
                if (m_phase == PHASE_GROUND)
                {
                    DoScriptText(SAY_SUMMON, m_creature);
                    for (const uint32 spell : { SPELL_SUMMON_WARRIOR_1, SPELL_SUMMON_WARRIOR_2, SPELL_SUMMON_WARRIOR_3 })
                        DoCastSpellIfCan(m_creature, spell, CAST_TRIGGERED);
                    ResetCombatAction(action, 30 * IN_MILLISECONDS);
                }
                else
                {
                    switch (m_phaseSub)
                    {
                        case PHASE_SKELETON_1:
                        {
                            DoSummonPlaguedChampions(false);
                            ResetCombatAction(action, 30 * IN_MILLISECONDS);
                            break;
                        }
                        case PHASE_SKELETON_2:
                        {
                            DoSummonPlaguedChampions(false);
                            DoSummonPlaguedGuardians();
                            ResetCombatAction(action, 45 * IN_MILLISECONDS);
                            break;
                        }
                        case PHASE_SKELETON_3:
                        {
                            DoSummonPlaguedChampions(true); // Set parameter to true because we need to indicate this is the third wave and Plagued Construct need to be also summoned
                            DoSummonPlaguedGuardians();
                            ResetCombatAction(action, 60 * IN_MILLISECONDS);
                            break;
                        }
                    }
                    ++m_subWavesCount; // Keep track of which wave we currently are
                }
                break;
            }
            default:
                break;
        }
    }
};

void AddSC_boss_noth()
{
    Script* newScript = new Script;
    newScript->Name = "boss_noth";
    newScript->GetAI = &GetNewAIInstance<boss_nothAI>;
    newScript->RegisterSelf();
}