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
SDName: Boss_Onyxia
SD%Complete: 85
SDComment: Visual improvement needed in phase 2: flying animation while hovering. Quel'Serrar event is missing.
SDCategory: Onyxia's Lair
EndScriptData

*/

#include "AI/ScriptDevAI/include/sc_common.h"
#include "onyxias_lair.h"
#include "AI/ScriptDevAI/base/CombatAI.h"

enum
{
    SAY_AGGRO                   = -1249000,
    SAY_KILL                    = -1249001,
    SAY_PHASE_2_TRANS           = -1249002,
    SAY_PHASE_3_TRANS           = -1249003,
    EMOTE_BREATH                = -1249004,
    SAY_KITE                    = -1249005,

    SPELL_WINGBUFFET            = 18500,
    SPELL_FLAMEBREATH           = 18435,
    SPELL_CLEAVE                = 19983,
    SPELL_TAILSWEEP             = 15847,
    SPELL_KNOCK_AWAY            = 19633,
    SPELL_FIREBALL              = 18392,

    SPELL_DRAGON_HOVER          = 18430,
    SPELL_SPEED_BURST           = 18391,
    SPELL_PACIFY_SELF           = 19951,

    // Not much choise about these. We have to make own defintion on the direction/start-end point
    SPELL_BREATH_NORTH_TO_SOUTH = 17086,                    // 20x in "array"
    SPELL_BREATH_SOUTH_TO_NORTH = 18351,                    // 11x in "array"

    SPELL_BREATH_EAST_TO_WEST   = 18576,                    // 7x in "array"
    SPELL_BREATH_WEST_TO_EAST   = 18609,                    // 7x in "array"

    SPELL_BREATH_SE_TO_NW       = 18564,                    // 12x in "array"
    SPELL_BREATH_NW_TO_SE       = 18584,                    // 12x in "array"
    SPELL_BREATH_SW_TO_NE       = 18596,                    // 12x in "array"
    SPELL_BREATH_NE_TO_SW       = 18617,                    // 12x in "array"

    SPELL_VISUAL_BREATH_A       = 4880,                     // Only and all of the above Breath spells (and their triggered spells) have these visuals
    SPELL_VISUAL_BREATH_B       = 4919,

    SPELL_BREATH_ENTRANCE       = 21131,                    // 8x in "array", different initial cast than the other arrays

    SPELL_BELLOWINGROAR         = 18431,
    SPELL_HEATED_GROUND         = 22191,                    // Prevent players from hiding in the tunnels when it is time for Onyxia's breath

    // SPELL_SUMMON_ONYXIAN_WHELPS = 20171,                    // Periodic spell triggering SPELL_SUMMON_ONYXIAN_WHELP every 2 secs. Possibly used in Phase 2 for summoning whelps but data like duration or caster are unkown
    // SPELL_SUMMON_ONYXIAN_WHELP  = 20172,                    // Triggered by SPELL_SUMMON_ONYXIAN_WHELPS

    POINT_ID_NORTH              = 0,
    POINT_ID_SOUTH              = 4,
    NUM_MOVE_POINT              = 8,
    MAX_POINTS                  = 10,
    POINT_ID_LIFTOFF            = 1 + NUM_MOVE_POINT,
    POINT_ID_INIT_NORTH         = 2 + NUM_MOVE_POINT,
    POINT_ID_LAND               = 3 + NUM_MOVE_POINT,

    PHASE_START                 = 1,                        // Health above 65%, normal ground abilities
    PHASE_BREATH                = 2,                        // Breath phase (while health above 40%)
    PHASE_END                   = 3,                        // normal ground abilities + some extra abilities
    PHASE_BREATH_POST           = 4,                        // Landing and initial fearing
    PHASE_TO_LIFTOFF            = 5,                        // Movement to south-entrance of room and liftoff there
    PHASE_BREATH_PRE            = 6,                        // lifting off + initial flying to north side (summons also first pack of whelps)

    POINT_MID_AIR_LAND          = 4 + NUM_MOVE_POINT,

};

struct OnyxiaMove
{
    uint32 uiSpellId;
    float fX, fY, fZ;
};

static const OnyxiaMove aMoveData[NUM_MOVE_POINT] =
{
    {SPELL_BREATH_NORTH_TO_SOUTH,  24.16332f, -216.0808f, -58.98009f},  // north (coords verified in wotlk)
    {SPELL_BREATH_NE_TO_SW,        10.2191f,  -247.912f,  -60.896f},    // north-east
    {SPELL_BREATH_EAST_TO_WEST,   -15.00505f, -244.4841f, -60.40087f},  // east (coords verified in wotlk)
    {SPELL_BREATH_SE_TO_NW,       -63.5156f,  -240.096f,  -60.477f},    // south-east
    {SPELL_BREATH_SOUTH_TO_NORTH, -66.3589f,  -215.928f,  -64.23904f},  // south (coords verified in wotlk)
    {SPELL_BREATH_SW_TO_NE,       -58.2509f,  -189.020f,  -60.790f},    // south-west
    {SPELL_BREATH_WEST_TO_EAST,   -16.70134f, -181.4501f, -61.98513f},  // west (coords verified in wotlk)
    {SPELL_BREATH_NW_TO_SE,        12.26687f, -181.1084f, -60.23914f},  // north-west (coords verified in wotlk)
};

static const float landPoints[1][3] =
{
    {-1.060547f, -229.9293f, -86.14094f},
};

static const float afSpawnLocations[3][3] =
{
    { -30.127f, -254.463f, -89.440f},                       // whelps
    { -30.817f, -177.106f, -89.258f},                       // whelps
};

enum OnyxiaActions
{
    ONYXIA_PHASE_2_TRANSITION,
    ONYXIA_PHASE_3_TRANSITION,
    ONYXIA_CHECK_IN_LAIR,
    ONYXIA_BELLOWING_ROAR,
    ONYXIA_FLAME_BREATH,
    ONYXIA_CLEAVE,
    ONYXIA_TAIL_SWEEP,
    ONYXIA_WING_BUFFET,
    ONYXIA_KNOCK_AWAY,
    ONYXIA_MOVEMENT,
    ONYXIA_FIREBALL,
    ONYXIA_ACTION_MAX,
    ONYXIA_SUMMON_WHELPS,
    ONYXIA_PHASE_TRANSITIONS,
};

struct boss_onyxiaAI : public CombatAI
{
    boss_onyxiaAI(Creature* creature) :
        CombatAI(creature, ONYXIA_ACTION_MAX),
        m_instance(static_cast<instance_onyxias_lair*>(creature->GetInstanceData())),
        m_uiPhase(0),
        m_uiMovePoint(POINT_ID_NORTH),
        m_uiSummonCount(0),
        m_uiWhelpsPerWave(20),
        m_bIsSummoningWhelps(false),
        m_bHasYelledLured(false),
        m_HasSummonedFirstWave(false)
    {
        AddTimerlessCombatAction(ONYXIA_PHASE_2_TRANSITION, true);
        AddTimerlessCombatAction(ONYXIA_PHASE_3_TRANSITION, false);
        AddCombatAction(ONYXIA_BELLOWING_ROAR, true);
        AddCombatAction(ONYXIA_CHECK_IN_LAIR, 3000u);
        AddCombatAction(ONYXIA_FLAME_BREATH, 10000, 20000);
        AddCombatAction(ONYXIA_CLEAVE, 2000, 5000);
        AddCombatAction(ONYXIA_TAIL_SWEEP, 15000, 20000);
        AddCombatAction(ONYXIA_WING_BUFFET, 10000, 20000);
        AddCombatAction(ONYXIA_KNOCK_AWAY, 20000, 30000);
        AddCombatAction(ONYXIA_FIREBALL, true);
        AddCombatAction(ONYXIA_MOVEMENT, true);
        AddCustomAction(ONYXIA_SUMMON_WHELPS, true, [&]() { SummonWhelps(); }, TIMER_COMBAT_COMBAT);
        AddCustomAction(ONYXIA_PHASE_TRANSITIONS, true, [&]() { PhaseTransition(); }, TIMER_COMBAT_COMBAT);
        m_creature->SetWalk(false); // onyxia should run when flying
        AddOnKillText(SAY_KILL);
    }

    instance_onyxias_lair* m_instance;

    uint8 m_uiPhase;

    uint32 m_uiMovePoint;

    uint8 m_uiSummonCount;
    uint8 m_uiWhelpsPerWave;

    bool m_bIsSummoningWhelps;
    bool m_bHasYelledLured;
    bool m_HasSummonedFirstWave;

    ObjectGuid m_fireballVictim;

    void Reset() override
    {
        CombatAI::Reset();
        if (!IsCombatMovement())
            SetCombatMovement(true);

        m_uiPhase = PHASE_START;

        m_uiMovePoint = POINT_ID_NORTH;                     // First point reached by the flying Onyxia

        m_uiSummonCount = 0;
        m_uiWhelpsPerWave = 20;                               // Twenty whelps on first summon, 4 - 10 on the nexts

        m_bHasYelledLured = false;
        m_HasSummonedFirstWave = false;

        m_creature->SetStandState(UNIT_STAND_STATE_SLEEP);
        m_creature->SetSheath(SHEATH_STATE_MELEE);
        SetMeleeEnabled(true);
    }

    void Aggro(Unit* /*who*/) override
    {
        DoScriptText(SAY_AGGRO, m_creature);

        if (m_instance)
            m_instance->SetData(TYPE_ONYXIA, IN_PROGRESS);

        m_creature->SetStandState(UNIT_STAND_STATE_STAND);
    }

    void JustReachedHome() override
    {
        // in case evade in phase 2, see comments for hack where phase 2 is set

        if (m_instance)
            m_instance->SetData(TYPE_ONYXIA, FAIL);
    }

    void JustDied(Unit* /*killer*/) override
    {
        if (m_instance)
            m_instance->SetData(TYPE_ONYXIA, DONE);
    }

    void JustSummoned(Creature* summoned) override
    {
        if (!m_instance)
            return;

        if (summoned->GetEntry() == NPC_ONYXIA_WHELP)
            ++m_uiSummonCount;
    }

    void SummonedMovementInform(Creature* summoned, uint32 motionType, uint32 pointId) override
    {
        if (motionType != POINT_MOTION_TYPE || pointId != 1 || !m_creature->GetVictim())
            return;

        summoned->SetInCombatWithZone();
    }

    void SpellHit(Unit* /*caster*/, const SpellEntry* spellInfo) override
    {
        if (spellInfo->Id == SPELL_BREATH_EAST_TO_WEST ||
                spellInfo->Id == SPELL_BREATH_WEST_TO_EAST ||
                spellInfo->Id == SPELL_BREATH_SE_TO_NW ||
                spellInfo->Id == SPELL_BREATH_NW_TO_SE ||
                spellInfo->Id == SPELL_BREATH_SW_TO_NE ||
                spellInfo->Id == SPELL_BREATH_NE_TO_SW ||
                spellInfo->Id == SPELL_BREATH_SOUTH_TO_NORTH ||
                spellInfo->Id == SPELL_BREATH_NORTH_TO_SOUTH)
        {
            // This was sent with SendMonsterMove - which resulted in better speed than now
            m_creature->GetMotionMaster()->MovePoint(m_uiMovePoint, aMoveData[m_uiMovePoint].fX, aMoveData[m_uiMovePoint].fY, aMoveData[m_uiMovePoint].fZ);
            DoCastSpellIfCan(m_creature, SPELL_HEATED_GROUND, CAST_TRIGGERED);
        }
    }

    void MovementInform(uint32 motionType, uint32 pointId) override
    {
        if (motionType != POINT_MOTION_TYPE || !m_instance)
            return;

        switch (pointId)
        {
            case POINT_ID_LAND:
                // undo flying
                m_creature->HandleEmote(EMOTE_ONESHOT_LAND);
                m_creature->SetHover(false);
                m_creature->RemoveAurasDueToSpell(SPELL_PACIFY_SELF);
                ResetTimer(ONYXIA_PHASE_TRANSITIONS, 2000);                          // Start PHASE_END shortly delayed
                return;
            case POINT_ID_LIFTOFF:
                ResetTimer(ONYXIA_PHASE_TRANSITIONS, 3500);                           // Start Flying shortly delayed
                m_creature->SetHover(true);
                m_creature->HandleEmote(EMOTE_ONESHOT_LIFTOFF);
                break;
            case POINT_ID_INIT_NORTH:                           // Start PHASE_BREATH
                m_uiPhase = PHASE_BREATH;
                m_creature->CastSpell(nullptr, SPELL_DRAGON_HOVER, TRIGGERED_OLD_TRIGGERED);
                m_creature->CastSpell(nullptr, SPELL_PACIFY_SELF, TRIGGERED_OLD_TRIGGERED);
                SetCombatScriptStatus(false);
                HandlePhaseTransition();
                break;
            default:
                m_creature->CastSpell(nullptr, SPELL_DRAGON_HOVER, TRIGGERED_OLD_TRIGGERED);
                break;
        }

        if (Creature* pTrigger = m_instance->GetSingleCreatureFromStorage(NPC_ONYXIA_TRIGGER))
            m_creature->SetFacingToObject(pTrigger);
    }

    void AttackStart(Unit* who) override
    {
        if (m_uiPhase == PHASE_START || m_uiPhase == PHASE_END)
            ScriptedAI::AttackStart(who);
    }

    // Onyxian Whelps summoning during phase 2
    void SummonWhelps()
    {
        if (m_uiSummonCount >= m_uiWhelpsPerWave)
        {
            ResetTimer(ONYXIA_SUMMON_WHELPS, 90 * IN_MILLISECONDS - m_uiWhelpsPerWave * (m_HasSummonedFirstWave ? 3000 : 1750));
            m_HasSummonedFirstWave = true;
            m_uiWhelpsPerWave = urand(4, 10);
            m_uiSummonCount = 0;
            return;
        }

        for (uint8 i = 0; i < 2; i++)
        {
            // Should probably make use of SPELL_SUMMON_ONYXIAN_WHELPS instead. Correct caster and removal of the spell is unkown, so make Onyxia do the summoning
            if (Creature* pWhelp =  m_creature->SummonCreature(NPC_ONYXIA_WHELP, afSpawnLocations[i][0], afSpawnLocations[i][1], afSpawnLocations[i][2], 0.0f, TEMPSPAWN_TIMED_OOC_OR_DEAD_DESPAWN, 3 * IN_MILLISECONDS))
            {
                if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_TOPAGGRO, 0))
                    pWhelp->AI()->AttackStart(pTarget);
            }
        }
        ResetTimer(ONYXIA_SUMMON_WHELPS, m_HasSummonedFirstWave ? urand(2000, 4000) : urand(1000, 2500));
    }

    void PhaseTransition()
    {
        switch (m_uiPhase)
        {
            case PHASE_TO_LIFTOFF:
                m_uiPhase = PHASE_BREATH_PRE;
                // Initial Onyxian Whelps spawn
                ResetTimer(ONYXIA_SUMMON_WHELPS, 3000);
                if (m_instance)
                    m_instance->SetData(TYPE_ONYXIA, DATA_LIFTOFF);
                m_creature->GetMotionMaster()->MovePoint(POINT_ID_INIT_NORTH, aMoveData[POINT_ID_NORTH].fX, aMoveData[POINT_ID_NORTH].fY, aMoveData[POINT_ID_NORTH].fZ, FORCED_MOVEMENT_FLIGHT);
                break;
            case PHASE_BREATH_POST:
                m_uiPhase = PHASE_END;
                m_creature->SetTarget(m_creature->GetVictim());
                SetCombatMovement(true, true);
                SetMeleeEnabled(true);
                SetCombatScriptStatus(false);
                HandlePhaseTransition();
                break;
        }
    }

    void HandlePhaseTransition()
    {
        switch (m_uiPhase)
        {
            case PHASE_BREATH:
            {
                ResetCombatAction(ONYXIA_FIREBALL, 0);
                ResetCombatAction(ONYXIA_MOVEMENT, 25000);
                DisableCombatAction(ONYXIA_FLAME_BREATH);
                DisableCombatAction(ONYXIA_CLEAVE);
                DisableCombatAction(ONYXIA_TAIL_SWEEP);
                DisableCombatAction(ONYXIA_WING_BUFFET);
                DisableCombatAction(ONYXIA_KNOCK_AWAY);
                DisableCombatAction(ONYXIA_CHECK_IN_LAIR);
                SetActionReadyStatus(ONYXIA_PHASE_3_TRANSITION, true);
                break;
            }
            case PHASE_END:
            {
                DisableCombatAction(ONYXIA_FIREBALL);
                DisableCombatAction(ONYXIA_MOVEMENT);
                ResetCombatAction(ONYXIA_BELLOWING_ROAR, 0);
                ResetCombatAction(ONYXIA_FLAME_BREATH, urand(10000, 20000));
                ResetCombatAction(ONYXIA_CLEAVE, urand(2000, 5000));
                ResetCombatAction(ONYXIA_TAIL_SWEEP, urand(15000, 20000));
                ResetCombatAction(ONYXIA_KNOCK_AWAY, urand(10000, 20000));
                ResetCombatAction(ONYXIA_CHECK_IN_LAIR, 3000);
                break;
            }
        }
    }

    void OnSpellCooldownAdded(SpellEntry const* spellInfo) override
    {
        CombatAI::OnSpellCooldownAdded(spellInfo);
        if (spellInfo->Id == SPELL_FIREBALL)
            if (Unit* target = m_creature->GetMap()->GetUnit(m_fireballVictim))
                m_creature->getThreatManager().modifyThreatPercent(target, -100);
    }

    void ExecuteAction(uint32 action) override
    {
        switch (action)
        {
            case ONYXIA_PHASE_2_TRANSITION:
            {
                if (m_creature->GetHealthPercent() > 65.0f)
                    return;

                m_uiPhase = PHASE_TO_LIFTOFF;
                DoScriptText(SAY_PHASE_2_TRANS, m_creature);
                SetCombatMovement(false);
                SetMeleeEnabled(false);
                m_creature->SetTarget(nullptr);
                SetCombatScriptStatus(true);

                m_creature->GetMotionMaster()->MovePoint(POINT_ID_LIFTOFF, Position(aMoveData[POINT_ID_SOUTH].fX, aMoveData[POINT_ID_SOUTH].fY, -84.25523f, 6.248279f));
                SetActionReadyStatus(action, false);
                break;
            }
            case ONYXIA_PHASE_3_TRANSITION:
            {
                if (m_creature->GetHealthPercent() > 40.0f)
                    return;

                m_uiPhase = PHASE_BREATH_POST;
                DoScriptText(SAY_PHASE_3_TRANS, m_creature);
                DisableTimer(ONYXIA_SUMMON_WHELPS);

                SetCombatScriptStatus(true);
                m_creature->SetTarget(nullptr);
                m_creature->RemoveAurasDueToSpell(SPELL_DRAGON_HOVER);
                m_creature->GetMotionMaster()->MovePoint(POINT_ID_LAND, landPoints[0][0], landPoints[0][1], landPoints[0][2], FORCED_MOVEMENT_FLIGHT);
                SetActionReadyStatus(action, false);
                break;
            }
            case ONYXIA_CHECK_IN_LAIR:
            {
                if (m_creature->IsNonMeleeSpellCasted(false))
                    return;
                if (m_instance)
                {
                    Creature* onyxiaTrigger = m_instance->GetSingleCreatureFromStorage(NPC_ONYXIA_TRIGGER);
                    if (onyxiaTrigger && !m_creature->IsWithinDistInMap(onyxiaTrigger, 90.0f, false))
                    {
                        if (!m_bHasYelledLured)
                        {
                            m_bHasYelledLured = true;
                            DoScriptText(SAY_KITE, m_creature);
                        }
                        DoCastSpellIfCan(nullptr, SPELL_BREATH_ENTRANCE);
                    }
                    else
                        m_bHasYelledLured = false;
                }
                ResetCombatAction(action, 3000);
                break;
            }
            case ONYXIA_SUMMON_WHELPS:
            {
                SummonWhelps();
                break;
            }
            case ONYXIA_BELLOWING_ROAR:
            {
                if (DoCastSpellIfCan(nullptr, SPELL_BELLOWINGROAR) == CAST_OK)
                    ResetCombatAction(action, urand(15000, 45000));
                break;
            }
            case ONYXIA_FLAME_BREATH:
            {
                if (DoCastSpellIfCan(m_creature->GetVictim(), SPELL_FLAMEBREATH) == CAST_OK)
                    ResetCombatAction(action, urand(10000, 20000));
                break;
            }
            case ONYXIA_CLEAVE:
            {
                if (DoCastSpellIfCan(m_creature->GetVictim(), SPELL_CLEAVE) == CAST_OK)
                    ResetCombatAction(action, urand(5000, 10000));
                break;
            }
            case ONYXIA_TAIL_SWEEP:
            {
                if (DoCastSpellIfCan(nullptr, SPELL_TAILSWEEP) == CAST_OK)
                    ResetCombatAction(action, urand(15000, 20000));
                break;
            }
            case ONYXIA_WING_BUFFET:
            {
                if (DoCastSpellIfCan(nullptr, SPELL_WINGBUFFET) == CAST_OK)
                    ResetCombatAction(action, urand(15000, 30000));
                break;
            }
            case ONYXIA_KNOCK_AWAY:
            {
                if (DoCastSpellIfCan(m_creature->GetVictim(), SPELL_KNOCK_AWAY) == CAST_OK)
                    ResetCombatAction(action, urand(25000, 40000));
                break;
            }
            case ONYXIA_FIREBALL:
            {
                if (Unit* target = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0, SPELL_FIREBALL, SELECT_FLAG_PLAYER))
                {
                    if (DoCastSpellIfCan(target, SPELL_FIREBALL) == CAST_OK)
                    {
                        ResetCombatAction(action, urand(3000, 5000));
                        m_fireballVictim = target->GetObjectGuid();
                    }
                }
                break;
            }
            case ONYXIA_MOVEMENT:
            {
                if (m_creature->IsNonMeleeSpellCasted(false))
                    return;
                // 3 possible actions
                switch (urand(0, 2))
                {
                    case 0:                             // breath
                        DoScriptText(EMOTE_BREATH, m_creature);
                        DoCastSpellIfCan(m_creature, aMoveData[m_uiMovePoint].uiSpellId, CAST_INTERRUPT_PREVIOUS);
                        m_uiMovePoint += NUM_MOVE_POINT / 2;
                        m_uiMovePoint %= NUM_MOVE_POINT;
                        ResetCombatAction(action, 25000);
                        return;
                    case 1:                             // a point on the left side
                    {
                        // C++ is stupid, so add -1 with +7
                        m_uiMovePoint += NUM_MOVE_POINT - 1;
                        m_uiMovePoint %= NUM_MOVE_POINT;
                        break;
                    }
                    case 2:                             // a point on the right side
                        m_uiMovePoint = (m_uiMovePoint + 1) % NUM_MOVE_POINT;
                        break;
                }

                ResetCombatAction(action, urand(15000, 25000));
                m_creature->RemoveAurasDueToSpell(SPELL_DRAGON_HOVER);
                m_creature->GetMotionMaster()->MovePoint(m_uiMovePoint, aMoveData[m_uiMovePoint].fX, aMoveData[m_uiMovePoint].fY, aMoveData[m_uiMovePoint].fZ, FORCED_MOVEMENT_FLIGHT);
                break;
            }
        }
    }
};

void AddSC_boss_onyxia()
{
    Script* pNewScript = new Script;
    pNewScript->Name = "boss_onyxia";
    pNewScript->GetAI = &GetNewAIInstance<boss_onyxiaAI>;
    pNewScript->RegisterSelf();
}

/*
-- SPELL_BREATH_EAST_TO_WEST
DELETE FROM spell_target_position WHERE id IN (18576, 18578, 18579, 18580, 18581, 18582, 18583);
INSERT INTO spell_target_position VALUES (18576, 249, -37.743851, -243.667923, -88.217651, 1.416);
INSERT INTO spell_target_position VALUES (18578, 249, -35.805332, -232.028900, -87.749153, 1.416);
INSERT INTO spell_target_position VALUES (18579, 249, -34.045738, -224.714661, -85.529465, 1.416);
INSERT INTO spell_target_position VALUES (18580, 249, -32.081570, -214.916962, -88.327438, 1.416);
INSERT INTO spell_target_position VALUES (18581, 249, -36.611721, -202.684677, -85.653786, 1.416);
INSERT INTO spell_target_position VALUES (18582, 249, -37.067261, -195.758652, -87.745834, 1.416);
INSERT INTO spell_target_position VALUES (18583, 249, -37.728523, -188.616806, -88.074898, 1.416);
-- SPELL_BREATH_WEST_TO_EAST
DELETE FROM spell_target_position WHERE id IN (18609, 18611, 18612, 18613, 18614, 18615, 18616);
INSERT INTO spell_target_position VALUES (18609, 249, -37.728523, -188.616806, -88.074898, 4.526);
INSERT INTO spell_target_position VALUES (18611, 249, -37.067261, -195.758652, -87.745834, 4.526);
INSERT INTO spell_target_position VALUES (18612, 249, -36.611721, -202.684677, -85.653786, 4.526);
INSERT INTO spell_target_position VALUES (18613, 249, -32.081570, -214.916962, -88.327438, 4.526);
INSERT INTO spell_target_position VALUES (18614, 249, -34.045738, -224.714661, -85.529465, 4.526);
INSERT INTO spell_target_position VALUES (18615, 249, -35.805332, -232.028900, -87.749153, 4.526);
INSERT INTO spell_target_position VALUES (18616, 249, -37.743851, -243.667923, -88.217651, 4.526);
-- SPELL_BREATH_NW_TO_SE
DELETE FROM spell_target_position WHERE id IN (18584, 18585, 18586, 18587, 18588, 18589, 18590, 18591, 18592, 18593, 18594, 18595);
INSERT INTO spell_target_position VALUES (18584, 249, 6.016711, -181.305771, -85.654648, 3.776);
INSERT INTO spell_target_position VALUES (18585, 249, 3.860220, -183.227249, -86.375381, 3.776);
INSERT INTO spell_target_position VALUES (18586, 249, -2.529650, -188.690491, -87.172859, 3.776);
INSERT INTO spell_target_position VALUES (18587, 249, -8.449303, -193.957962, -87.564957, 3.776);
INSERT INTO spell_target_position VALUES (18588, 249, -14.321238, -199.462219, -87.922478, 3.776);
INSERT INTO spell_target_position VALUES (18589, 249, -15.602085, -216.893936, -88.403183, 3.776);
INSERT INTO spell_target_position VALUES (18590, 249, -23.650263, -221.969086, -89.172699, 3.776);
INSERT INTO spell_target_position VALUES (18591, 249, -29.495876, -213.014359, -88.910423, 3.776);
INSERT INTO spell_target_position VALUES (18592, 249, -35.439922, -217.260284, -87.336311, 3.776);
INSERT INTO spell_target_position VALUES (18593, 249, -41.762104, -221.896545, -86.114113, 3.776);
INSERT INTO spell_target_position VALUES (18594, 249, -51.067528, -228.909988, -85.765556, 3.776);
INSERT INTO spell_target_position VALUES (18595, 249, -56.559654, -241.223923, -85.423607, 3.776);
-- SPELL_BREATH_SE_TO_NW
DELETE FROM spell_target_position WHERE id IN (18564, 18565, 18566, 18567, 18568, 18569, 18570, 18571, 18572, 18573, 18574, 18575);
INSERT INTO spell_target_position VALUES (18564, 249, -56.559654, -241.223923, -85.423607, 0.666);
INSERT INTO spell_target_position VALUES (18565, 249, -51.067528, -228.909988, -85.765556, 0.666);
INSERT INTO spell_target_position VALUES (18566, 249, -41.762104, -221.896545, -86.114113, 0.666);
INSERT INTO spell_target_position VALUES (18567, 249, -35.439922, -217.260284, -87.336311, 0.666);
INSERT INTO spell_target_position VALUES (18568, 249, -29.495876, -213.014359, -88.910423, 0.666);
INSERT INTO spell_target_position VALUES (18569, 249, -23.650263, -221.969086, -89.172699, 0.666);
INSERT INTO spell_target_position VALUES (18570, 249, -15.602085, -216.893936, -88.403183, 0.666);
INSERT INTO spell_target_position VALUES (18571, 249, -14.321238, -199.462219, -87.922478, 0.666);
INSERT INTO spell_target_position VALUES (18572, 249, -8.449303, -193.957962, -87.564957, 0.666);
INSERT INTO spell_target_position VALUES (18573, 249, -2.529650, -188.690491, -87.172859, 0.666);
INSERT INTO spell_target_position VALUES (18574, 249, 3.860220, -183.227249, -86.375381, 0.666);
INSERT INTO spell_target_position VALUES (18575, 249, 6.016711, -181.305771, -85.654648, 0.666);
-- SPELL_BREATH_SW_TO_NE
DELETE FROM spell_target_position WHERE id IN (18596, 18597, 18598, 18599, 18600, 18601, 18602, 18603, 18604, 18605, 18606, 18607);
INSERT INTO spell_target_position VALUES (18596, 249, -58.250900, -189.020004, -85.292267, 5.587);
INSERT INTO spell_target_position VALUES (18597, 249, -52.006271, -193.796570, -85.808769, 5.587);
INSERT INTO spell_target_position VALUES (18598, 249, -46.135464, -198.548553, -85.901764, 5.587);
INSERT INTO spell_target_position VALUES (18599, 249, -40.500187, -203.001053, -85.555107, 5.587);
INSERT INTO spell_target_position VALUES (18600, 249, -30.907579, -211.058197, -88.592125, 5.587);
INSERT INTO spell_target_position VALUES (18601, 249, -20.098139, -218.681427, -88.937088, 5.587);
INSERT INTO spell_target_position VALUES (18602, 249, -12.223192, -224.666168, -87.856300, 5.587);
INSERT INTO spell_target_position VALUES (18603, 249, -6.475297, -229.098724, -87.076401, 5.587);
INSERT INTO spell_target_position VALUES (18604, 249, -2.010256, -232.541992, -86.995140, 5.587);
INSERT INTO spell_target_position VALUES (18605, 249, 2.736300, -236.202347, -86.790367, 5.587);
INSERT INTO spell_target_position VALUES (18606, 249, 7.197779, -239.642868, -86.307297, 5.587);
INSERT INTO spell_target_position VALUES (18607, 249, 12.120926, -243.439407, -85.874260, 5.587);
-- SPELL_BREATH_NE_TO_SW
DELETE FROM spell_target_position WHERE id IN (18617, 18619, 18620, 18621, 18622, 18623, 18624, 18625, 18626, 18627, 18628, 18618);
INSERT INTO spell_target_position VALUES (18617, 249, 12.120926, -243.439407, -85.874260, 2.428);
INSERT INTO spell_target_position VALUES (18619, 249, 7.197779, -239.642868, -86.307297, 2.428);
INSERT INTO spell_target_position VALUES (18620, 249, 2.736300, -236.202347, -86.790367, 2.428);
INSERT INTO spell_target_position VALUES (18621, 249, -2.010256, -232.541992, -86.995140, 2.428);
INSERT INTO spell_target_position VALUES (18622, 249, -6.475297, -229.098724, -87.076401, 2.428);
INSERT INTO spell_target_position VALUES (18623, 249, -12.223192, -224.666168, -87.856300, 2.428);
INSERT INTO spell_target_position VALUES (18624, 249, -20.098139, -218.681427, -88.937088, 2.428);
INSERT INTO spell_target_position VALUES (18625, 249, -30.907579, -211.058197, -88.592125, 2.428);
INSERT INTO spell_target_position VALUES (18626, 249, -40.500187, -203.001053, -85.555107, 2.428);
INSERT INTO spell_target_position VALUES (18627, 249, -46.135464, -198.548553, -85.901764, 2.428);
INSERT INTO spell_target_position VALUES (18628, 249, -52.006271, -193.796570, -85.808769, 2.428);
INSERT INTO spell_target_position VALUES (18618, 249, -58.250900, -189.020004, -85.292267, 2.428);

-- SPELL_BREATH_SOUTH_TO_NORTH
DELETE FROM spell_target_position WHERE id IN (18351, 18352, 18353, 18354, 18355, 18356, 18357, 18358, 18359, 18360, 18361);
INSERT INTO spell_target_position VALUES (18351, 249, -68.834236, -215.036163, -84.018875, 6.280);
INSERT INTO spell_target_position VALUES (18352, 249, -61.834255, -215.051910, -84.673416, 6.280);
INSERT INTO spell_target_position VALUES (18353, 249, -53.343277, -215.071014, -85.597191, 6.280);
INSERT INTO spell_target_position VALUES (18354, 249, -42.619305, -215.095139, -86.663605, 6.280);
INSERT INTO spell_target_position VALUES (18355, 249, -35.899323, -215.110245, -87.196548, 6.280);
INSERT INTO spell_target_position VALUES (18356, 249, -28.248341, -215.127457, -89.191750, 6.280);
INSERT INTO spell_target_position VALUES (18357, 249, -20.324360, -215.145279, -88.963997, 6.280);
INSERT INTO spell_target_position VALUES (18358, 249, -11.189384, -215.165833, -87.817093, 6.280);
INSERT INTO spell_target_position VALUES (18359, 249, -2.047405, -215.186386, -86.279655, 6.280);
INSERT INTO spell_target_position VALUES (18360, 249, 7.479571, -215.207809, -86.075531, 6.280);
INSERT INTO spell_target_position VALUES (18361, 249, 20.730539, -215.237610, -85.254387, 6.280);
-- SPELL_BREATH_NORTH_TO_SOUTH
DELETE FROM spell_target_position WHERE id IN (17086, 17087, 17088, 17089, 17090, 17091, 17092, 17093, 17094, 17095, 17097, 22267, 22268, 21132, 21133, 21135, 21136, 21137, 21138, 21139);
INSERT INTO spell_target_position VALUES (17086, 249, 20.730539, -215.237610, -85.254387, 3.142);
INSERT INTO spell_target_position VALUES (17087, 249, 7.479571, -215.207809, -86.075531, 3.142);
INSERT INTO spell_target_position VALUES (17088, 249, -2.047405, -215.186386, -86.279655, 3.142);
INSERT INTO spell_target_position VALUES (17089, 249, -11.189384, -215.165833, -87.817093, 3.142);
INSERT INTO spell_target_position VALUES (17090, 249, -20.324360, -215.145279, -88.963997, 3.142);
INSERT INTO spell_target_position VALUES (17091, 249, -28.248341, -215.127457, -89.191750, 3.142);
INSERT INTO spell_target_position VALUES (17092, 249, -35.899323, -215.110245, -87.196548, 3.142);
INSERT INTO spell_target_position VALUES (17093, 249, -42.619305, -215.095139, -86.663605, 3.142);
INSERT INTO spell_target_position VALUES (17094, 249, -53.343277, -215.071014, -85.597191, 3.142);
INSERT INTO spell_target_position VALUES (17095, 249, -61.834255, -215.051910, -84.673416, 3.142);
INSERT INTO spell_target_position VALUES (17097, 249, -68.834236, -215.036163, -84.018875, 3.142);
INSERT INTO spell_target_position VALUES (22267, 249, -75.736046, -214.984970, -83.394188, 3.142);
INSERT INTO spell_target_position VALUES (22268, 249, -84.087578, -214.857834, -82.640053, 3.142);
INSERT INTO spell_target_position VALUES (21132, 249, -90.424416, -214.601974, -82.482697, 3.142);
INSERT INTO spell_target_position VALUES (21133, 249, -96.572411, -214.353745, -82.239967, 3.142);
INSERT INTO spell_target_position VALUES (21135, 249, -102.069931, -214.131775, -80.571190, 3.142);
INSERT INTO spell_target_position VALUES (21136, 249, -107.385597, -213.917145, -77.447037, 3.142);
INSERT INTO spell_target_position VALUES (21137, 249, -114.281258, -213.866486, -73.851128, 3.142);
INSERT INTO spell_target_position VALUES (21138, 249, -123.328560, -213.607910, -71.559921, 3.142);
INSERT INTO spell_target_position VALUES (21139, 249, -130.788300, -213.424026, -70.751007, 3.142);
*/
