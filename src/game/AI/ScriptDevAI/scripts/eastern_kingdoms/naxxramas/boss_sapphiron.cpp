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
SDName: Boss_Sapphiron
SD%Complete: 98
SDComment: Flying visual is bugged: Sapphiron uses standing animation instead of the flying/hovering one when not moving in the air
SDCategory: Naxxramas
EndScriptData

*/

#include "AI/ScriptDevAI/include/precompiled.h"
#include "naxxramas.h"

enum
{
    EMOTE_BREATH                = -1533082,
    EMOTE_GENERIC_ENRAGED       = -1000003,

    // All phases spells
    SPELL_FROST_AURA            = 28529,            // Periodically triggers 28531
    SPELL_BESERK                = 26662,

    // Ground phase spells
    SPELL_CLEAVE                = 19983,
    SPELL_TAIL_SWEEP            = 15847,
    SPELL_LIFE_DRAIN            = 28542,
    SPELL_SUMMON_BLIZZARD       = 28560,

    // Air phase spells
    SPELL_ICEBOLT               = 28526,            // Triggers spell 28522 (Icebolt)
    SPELL_FROST_BREATH_DUMMY    = 30101,
    SPELL_FROST_BREATH          = 28524,            // Triggers spells 29318 (Frost Breath) and 30132 (Despawn Ice Block)

    NPC_BLIZZARD                = 16474,
};

static const float aLiftOffPosition[3] = {3522.386f, -5236.784f, 137.709f};

enum Phases
{
    PHASE_GROUND        = 1,
    PHASE_LIFT_OFF      = 2,
    PHASE_AIR_BOLTS     = 3,
    PHASE_AIR_BREATH    = 4,
    PHASE_LANDING       = 5,
};

struct boss_sapphironAI : public ScriptedAI
{
    boss_sapphironAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = (instance_naxxramas*)pCreature->GetInstanceData();
        Reset();
    }

    instance_naxxramas* m_pInstance;

    uint32 m_uiCleaveTimer;
    uint32 m_uiTailSweepTimer;
    uint32 m_uiIceboltTimer;
    uint32 m_uiFrostBreathTimer;
    uint32 m_uiLifeDrainTimer;
    uint32 m_uiBlizzardTimer;
    uint32 m_uiFlyTimer;
    uint32 m_uiBerserkTimer;
    uint32 m_uiLandTimer;

    uint32 m_uiIceboltCount;
    Phases m_Phase;

    void Reset() override
    {
        m_uiCleaveTimer         = 5 * IN_MILLISECONDS;
        m_uiTailSweepTimer      = 12 * IN_MILLISECONDS;
        m_uiFrostBreathTimer    = 7 * IN_MILLISECONDS;
        m_uiLifeDrainTimer      = 11 * IN_MILLISECONDS;
        m_uiBlizzardTimer       = 15 * IN_MILLISECONDS;
        m_uiFlyTimer            = 46 * IN_MILLISECONDS;
        m_uiIceboltTimer        = 7 * IN_MILLISECONDS;
        m_uiLandTimer           = 0;
        m_uiBerserkTimer        = 15 * MINUTE * IN_MILLISECONDS;
        m_Phase                 = PHASE_GROUND;
        m_uiIceboltCount        = 0;

        SetCombatMovement(true);
//      m_creature->RemoveByteFlag(UNIT_FIELD_BYTES_1, 3, UNIT_BYTE1_FLAG_FLY_ANIM);
        m_creature->SetByteFlag(UNIT_FIELD_BYTES_1, 3, UNIT_BYTE1_FLAG_ALWAYS_STAND);
        m_creature->SetCanFly(false);
        m_creature->SetHover(false);
        m_creature->SetLevitate(false);
    }

    void Aggro(Unit* /*pWho*/) override
    {
        DoCastSpellIfCan(m_creature, SPELL_FROST_AURA, CAST_TRIGGERED | CAST_AURA_NOT_PRESENT);

        if (m_pInstance)
            m_pInstance->SetData(TYPE_SAPPHIRON, IN_PROGRESS);
    }

    void JustDied(Unit* /*pKiller*/) override
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_SAPPHIRON, DONE);
    }

    void JustReachedHome() override
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_SAPPHIRON, FAIL);
    }

    void JustSummoned(Creature* pSummoned) override
    {
        if (pSummoned->GetEntry() == NPC_BLIZZARD)
        {
            if (Unit* pEnemy = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
                pSummoned->AI()->AttackStart(pEnemy);
        }
    }

    void SummonedJustReachedHome(Creature* summoned) override
    {
        summoned->ForcedDespawn();
    }

    void MovementInform(uint32 uiType, uint32 /*uiPointId*/) override
    {
        if (uiType == POINT_MOTION_TYPE && m_Phase == PHASE_LIFT_OFF)
        {
            m_creature->HandleEmote(EMOTE_ONESHOT_LIFTOFF);
            m_creature->SetLevitate(true);
            m_Phase = PHASE_AIR_BOLTS;

            m_uiFrostBreathTimer = 5 * IN_MILLISECONDS;
            m_uiIceboltTimer = 5 * IN_MILLISECONDS;
            m_uiIceboltCount = 0;
        }
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        switch (m_Phase)
        {
            case PHASE_GROUND:
                if (m_uiCleaveTimer < uiDiff)
                {
                    if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_CLEAVE) == CAST_OK)
                        m_uiCleaveTimer = urand(5, 10) * IN_MILLISECONDS;
                }
                else
                    m_uiCleaveTimer -= uiDiff;

                if (m_uiTailSweepTimer < uiDiff)
                {
                    if (DoCastSpellIfCan(m_creature, SPELL_TAIL_SWEEP) == CAST_OK)
                        m_uiTailSweepTimer = urand(7, 10) * IN_MILLISECONDS;
                }
                else
                    m_uiTailSweepTimer -= uiDiff;

                if (m_uiLifeDrainTimer < uiDiff)
                {
                    if (DoCastSpellIfCan(m_creature, SPELL_LIFE_DRAIN) == CAST_OK)
                        m_uiLifeDrainTimer = 24 * IN_MILLISECONDS;
                }
                else
                    m_uiLifeDrainTimer -= uiDiff;

                if (m_uiBlizzardTimer < uiDiff)
                {
                    if (DoCastSpellIfCan(m_creature, SPELL_SUMMON_BLIZZARD) == CAST_OK)
                        m_uiBlizzardTimer = 20 * IN_MILLISECONDS;
                }
                else
                    m_uiBlizzardTimer -= uiDiff;

                if (m_creature->GetHealthPercent() > 10.0f)
                {
                    if (m_uiFlyTimer < uiDiff)
                    {
                        m_Phase = PHASE_LIFT_OFF;
                        SetDeathPrevention(true);
                        m_creature->InterruptNonMeleeSpells(false);
                        SetCombatMovement(false);
                        m_uiIceboltTimer = 7 * IN_MILLISECONDS;
                        m_creature->GetMotionMaster()->Clear(false);
                        m_creature->RemoveByteFlag(UNIT_FIELD_BYTES_1, 3, UNIT_BYTE1_FLAG_ALWAYS_STAND);
//                        m_creature->SetByteValue(UNIT_FIELD_BYTES_1, 3, UNIT_BYTE1_FLAG_FLY_ANIM);
                        m_creature->SetCanFly(true);
                        m_creature->SetHover(true);
                        m_creature->SetLevitate(true);
                        m_creature->HandleEmote(EMOTE_ONESHOT_LIFTOFF);
                        m_creature->GetMotionMaster()->MoveIdle();
                        m_creature->GetMotionMaster()->MovePoint(1, aLiftOffPosition[0], aLiftOffPosition[1], aLiftOffPosition[2]);
                        // TODO This should clear the target, too

                        return;
                    }
                    m_uiFlyTimer -= uiDiff;
                }

                // Only Phase in which we have melee attack!
                DoMeleeAttackIfReady();
                break;
            case PHASE_LIFT_OFF:
                break;
            case PHASE_AIR_BOLTS:
                if (m_uiIceboltCount == 5)
                {
                    if (m_uiFrostBreathTimer < uiDiff)
                    {
                        if (DoCastSpellIfCan(m_creature, SPELL_FROST_BREATH) == CAST_OK)
                        {
                            DoCastSpellIfCan(m_creature, SPELL_FROST_BREATH_DUMMY, CAST_TRIGGERED);
                            DoScriptText(EMOTE_BREATH, m_creature);
                            m_Phase = PHASE_AIR_BREATH;
                            m_uiFrostBreathTimer = 4 * IN_MILLISECONDS;
                            m_uiLandTimer = 11 * IN_MILLISECONDS;
                        }
                    }
                    else
                        m_uiFrostBreathTimer -= uiDiff;
                }
                else
                {
                    if (m_uiIceboltTimer < uiDiff)
                    {
                        if (DoCastSpellIfCan(m_creature, SPELL_ICEBOLT) == CAST_OK)
                        {
                            ++m_uiIceboltCount;
                            m_uiIceboltTimer = 3 * IN_MILLISECONDS;
                        }
                    }
                    else
                        m_uiIceboltTimer -= uiDiff;
                }

                break;
            case PHASE_AIR_BREATH:
                if (m_uiLandTimer)
                {
                    if (m_uiLandTimer <= uiDiff)
                    {
                        // Begin Landing
                        m_creature->HandleEmote(EMOTE_ONESHOT_LAND);
                        m_creature->SetLevitate(false);

                        m_Phase = PHASE_LANDING;
                        m_uiLandTimer = 2 * IN_MILLISECONDS;
                    }
                    else
                        m_uiLandTimer -= uiDiff;
                }

                break;
            case PHASE_LANDING:
                if (m_uiLandTimer < uiDiff)
                {
                    m_Phase = PHASE_GROUND;
                    SetDeathPrevention(false);
                    SetCombatMovement(true);
                    m_creature->SetCanFly(false);
                    m_creature->SetHover(false);
                    m_creature->SetLevitate(false);
                    m_creature->GetMotionMaster()->Clear(false);
                    m_creature->GetMotionMaster()->MoveChase(m_creature->getVictim());

                    m_uiFlyTimer = 67 * IN_MILLISECONDS;
                    m_uiLandTimer = 0;
                }
                else
                    m_uiLandTimer -= uiDiff;

                break;
        }

        // Enrage can happen in any phase
        if (m_uiBerserkTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_BESERK) == CAST_OK)
            {
                DoScriptText(EMOTE_GENERIC_ENRAGED, m_creature);
                m_uiBerserkTimer = 300 * IN_MILLISECONDS;
            }
        }
        else
            m_uiBerserkTimer -= uiDiff;
    }
};

UnitAI* GetAI_boss_sapphiron(Creature* pCreature)
{
    return new boss_sapphironAI(pCreature);
}

bool GOUse_go_sapphiron_birth(Player* /*pPlayer*/, GameObject* pGo)
{
    ScriptedInstance* pInstance = (ScriptedInstance*)pGo->GetInstanceData();

    if (!pInstance)
        return true;

    if (pInstance->GetData(TYPE_SAPPHIRON) != NOT_STARTED)
        return true;

    // If already summoned return (safety check)
    if (pInstance->GetSingleCreatureFromStorage(NPC_SAPPHIRON, true))
        return true;

    // Set data to special and allow the Go animation to proceed
    pInstance->SetData(TYPE_SAPPHIRON, SPECIAL);
    return false;
}

void AddSC_boss_sapphiron()
{
    Script* pNewScript = new Script;
    pNewScript->Name = "boss_sapphiron";
    pNewScript->GetAI = &GetAI_boss_sapphiron;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "go_sapphiron_birth";
    pNewScript->pGOUse = &GOUse_go_sapphiron_birth;
    pNewScript->RegisterSelf();
}
