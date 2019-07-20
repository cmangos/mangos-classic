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
SD%Complete: 80
SDComment: Some spells need core implementation
SDCategory: Naxxramas
EndScriptData

*/

#include "AI/ScriptDevAI/include/precompiled.h"/* Additional comments:
 * Bugged spells:   28560 (needs maxTarget = 1, Summon of 16474 implementation, TODO, 30s duration)
 *                  28526 (needs ScriptEffect to cast 28522 onto random target)
 *
 * Frost-Breath ability: the dummy spell 30101 is self cast, so it won't take the needed delay of ~7s until it reaches its target
 *                       As Sapphiron is displayed visually in hight (hovering), and the spell is cast with target=self-location
 *                       which is still on the ground, the client shows a nice slow falling of the visual animation
 *                       Insisting on using the Dummy-Effect to cast the breath-dmg spells, would require, to relocate Sapphi internally,
 *                       and to hack the targeting to be "on the ground" - Hence the prefered way as it is now!
 */


#include "naxxramas.h"

enum
{
    EMOTE_BREATH                = -1533082,
    EMOTE_GENERIC_ENRAGED       = -1000003,
    EMOTE_FLY                   = -1533022,
    EMOTE_GROUND                = -1533083,

    SPELL_CLEAVE                = 19983,
    SPELL_TAIL_SWEEP            = 15847,
    //SPELL_ICEBOLT               = 28526,
    SPELL_ICEBOLT               = 28522,
    SPELL_FROST_BREATH_DUMMY    = 30101,
    SPELL_FROST_BREATH          = 28524,            // triggers 29318
    SPELL_FROST_AURA            = 28531,
    SPELL_LIFE_DRAIN            = 28542,
    SPELL_CHILL                 = 28547,
    SPELL_SUMMON_BLIZZARD       = 28560,
    SPELL_BESERK                = 26662,

    NPC_YOU_KNOW_WHO            = 16474,
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
        m_uiCleaveTimer = 5000;
        m_uiTailSweepTimer = 12000;
        m_uiFrostBreathTimer = 7000;
        m_uiLifeDrainTimer = 11000;
        m_uiBlizzardTimer = 15000;                          // "Once the encounter starts,based on your version of Naxx, this will be used x2 for normal and x6 on HC"
        m_uiFlyTimer = 46000;
        m_uiIceboltTimer = 5000;
        m_uiLandTimer = 0;
        m_uiBerserkTimer = 15 * MINUTE * IN_MILLISECONDS;
        m_Phase = PHASE_GROUND;
        m_uiIceboltCount = 0;

        SetCombatMovement(true);
        m_creature->SetLevitate(false);
    }

    void Aggro(Unit* /*pWho*/) override
    {
        DoCastSpellIfCan(m_creature, SPELL_FROST_AURA);

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
        if (pSummoned->GetEntry() == NPC_YOU_KNOW_WHO)
        {
            if (Unit* pEnemy = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
                pSummoned->AI()->AttackStart(pEnemy);
        }
    }

    void MovementInform(uint32 uiType, uint32 /*uiPointId*/) override
    {
        if (uiType == POINT_MOTION_TYPE && m_Phase == PHASE_LIFT_OFF)
        {
            DoScriptText(EMOTE_FLY, m_creature);
            m_creature->HandleEmote(EMOTE_ONESHOT_LIFTOFF);
            m_creature->SetLevitate(true);
            m_Phase = PHASE_AIR_BOLTS;

            m_uiFrostBreathTimer = 5000;
            m_uiIceboltTimer = 5000;
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
                        m_uiCleaveTimer = urand(5000, 10000);
                }
                else
                    m_uiCleaveTimer -= uiDiff;

                if (m_uiTailSweepTimer < uiDiff)
                {
                    if (DoCastSpellIfCan(m_creature, SPELL_TAIL_SWEEP) == CAST_OK)
                        m_uiTailSweepTimer = urand(7000, 10000);
                }
                else
                    m_uiTailSweepTimer -= uiDiff;

                if (m_uiLifeDrainTimer < uiDiff)
                {
                    if (DoCastSpellIfCan(m_creature, SPELL_LIFE_DRAIN) == CAST_OK)
                        m_uiLifeDrainTimer = 23000;
                }
                else
                    m_uiLifeDrainTimer -= uiDiff;

                if (m_uiBlizzardTimer < uiDiff)
                {
                    if (DoCastSpellIfCan(m_creature, SPELL_SUMMON_BLIZZARD) == CAST_OK)
                        m_uiBlizzardTimer = 20000;
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
                        m_creature->GetMotionMaster()->Clear(false);
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
                            m_uiFrostBreathTimer = 5000;
                            m_uiLandTimer = 11000;
                        }
                    }
                    else
                        m_uiFrostBreathTimer -= uiDiff;
                }
                else
                {
                    if (m_uiIceboltTimer < uiDiff)
                    {
                        if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0, uint32(0), SELECT_FLAG_PLAYER))
                        {
                            if (DoCastSpellIfCan(pTarget, SPELL_ICEBOLT) == CAST_OK)
                            {
                                ++m_uiIceboltCount;
                                m_uiIceboltTimer = 4000;
                            }
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
                        DoScriptText(EMOTE_GROUND, m_creature);
                        m_creature->HandleEmote(EMOTE_ONESHOT_LAND);
                        m_creature->SetLevitate(false);

                        m_Phase = PHASE_LANDING;
                        m_uiLandTimer = 2000;
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
                    m_creature->GetMotionMaster()->Clear(false);
                    m_creature->GetMotionMaster()->MoveChase(m_creature->getVictim());

                    m_uiFlyTimer = 67000;
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
                m_uiBerserkTimer = 300000;
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
