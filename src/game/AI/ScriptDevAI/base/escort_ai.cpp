/* This file is part of the ScriptDev2 Project. See AUTHORS file for Copyright information
 * This program is free software licensed under GPL version 2
 * Please see the included DOCS/LICENSE.TXT for more information */

/* ScriptData
SDName: EscortAI
SD%Complete: 100
SDComment:
SDCategory: Npc
EndScriptData */

#include "AI/ScriptDevAI/include/sc_common.h"
#include "AI/ScriptDevAI/base/escort_ai.h"
#include "AI/ScriptDevAI/system/system.h"
#include "MotionGenerators/WaypointManager.h"

const float MAX_PLAYER_DISTANCE = 66.0f;

npc_escortAI::npc_escortAI(Creature* creature) : ScriptedAI(creature),
    m_playerCheckTimer(1000),
    m_escortState(STATE_ESCORT_NONE),
    m_questForEscort(nullptr),
    m_isRunning(false),
    m_canInstantRespawn(false),
    m_canReturnToStart(false),
    m_waypointPathID(0),
    m_currentEscortWaypointPath(0)
{}

void npc_escortAI::GetAIInformation(ChatHandler& reader)
{
    std::ostringstream oss;

    oss << "EscortAI ";
    if (m_playerGuid)
        oss << "started for " << m_playerGuid.GetString() << " ";
    if (m_questForEscort)
        oss << "started with quest " << m_questForEscort->GetQuestId();

    if (HasEscortState(STATE_ESCORT_ESCORTING))
    {
        oss << "\nEscortFlags: Escorting" << (HasEscortState(STATE_ESCORT_RETURNING) ? ", Returning" : "") << (HasEscortState(STATE_ESCORT_PAUSED) ? ", Paused" : "") << "\n";
        m_creature->GetMotionMaster()->GetWaypointPathInformation(oss);
    }

    reader.PSendSysMessage("%s", oss.str().c_str());

    ScriptedAI::GetAIInformation(reader);
}

void npc_escortAI::Aggro(Unit* /*enemy*/) {}

// see followerAI
bool npc_escortAI::AssistPlayerInCombat(Unit* who)
{
    if (!HasEscortState(STATE_ESCORT_ESCORTING))
        return false;

    if (!who->GetVictim())
        return false;

    // experimental (unknown) flag not present
    if (!(m_creature->GetCreatureInfo()->CreatureTypeFlags & CREATURE_TYPEFLAGS_CAN_ASSIST))
        return false;

    // unit state prevents (similar check is done in CanInitiateAttack which also include checking unit_flags. We skip those here)
    if (m_creature->hasUnitState(UNIT_STAT_STUNNED | UNIT_STAT_FEIGN_DEATH | UNIT_STAT_CONFUSED | UNIT_STAT_FLEEING))
        return false;

    // victim of pWho is not a player
    if (!who->GetVictim()->GetBeneficiaryPlayer())
        return false;

    // never attack friendly
    if (!m_creature->CanAttackNow(who))
        return false;

    // too far away and no free sight?
    if (m_creature->IsWithinDistInMap(who, MAX_PLAYER_DISTANCE) && m_creature->IsWithinLOSInMap(who))
    {
        // already fighting someone?
        if (!m_creature->GetVictim())
        {
            AttackStart(who);
            return true;
        }
        m_creature->EngageInCombatWith(who);
        return true;
    }

    return false;
}

void npc_escortAI::JustDied(Unit* /*killer*/)
{
    if (!HasEscortState(STATE_ESCORT_ESCORTING) || !m_playerGuid || !m_questForEscort)
        return;

    FailQuestForPlayerAndGroup();
}

void npc_escortAI::FailQuestForPlayerAndGroup()
{
    if (!m_questForEscort)
        return;

    if (Player* player = GetPlayerForEscort())
        player->FailQuestForGroup(m_questForEscort->GetQuestId());
}

void npc_escortAI::CorpseRemoved(uint32& /*respawnDelay*/)
{
    m_escortState = STATE_ESCORT_NONE;

    if (!IsCombatMovement())
        SetCombatMovement(true);
}

bool npc_escortAI::IsPlayerOrGroupInRange()
{
    if (Player* player = GetPlayerForEscort())
        if (player->CheckForGroup([&](Player const* player) -> bool { return m_creature->IsWithinDistInMap(player, MAX_PLAYER_DISTANCE); }))
            return true;

    return false;
}

void npc_escortAI::UpdateAI(const uint32 diff)
{
    // Check if player or any member of his group is within range
    if (m_questForEscort && HasEscortState(STATE_ESCORT_ESCORTING) && m_playerGuid && !m_creature->GetVictim() && !HasEscortState(STATE_ESCORT_RETURNING))
    {
        if (m_playerCheckTimer < diff)
        {
            if (!HasEscortState(STATE_ESCORT_PAUSED) && !IsPlayerOrGroupInRange())
            {
                // set the quest status as failed
                FailQuestForPlayerAndGroup();

                // TODO: i am not sure this is correct, isn't that the creature should continue until it get killed?
                debug_log("SD2: EscortAI failed because player/group was to far away or not found");

                if (m_canInstantRespawn)
                {
                    m_creature->SetDeathState(JUST_DIED);
                    m_creature->Respawn();
                }
                else
                {
                    if (m_creature->IsPet())
                        static_cast<Pet*>(m_creature)->Unsummon(PET_SAVE_AS_DELETED); // we assume escort AI pet to always be non-saved
                    else
                        m_creature->ForcedDespawn();
                }

                return;
            }

            m_playerCheckTimer = 1000;
        }
        else
            m_playerCheckTimer -= diff;
    }

    UpdateEscortAI(diff);
}

void npc_escortAI::UpdateEscortAI(const uint32 diff)
{
    ScriptedAI::UpdateAI(diff);
}

/// Helper function for transition between old Escort Movment and using WaypointMMGen
bool npc_escortAI::IsSD2EscortMovement(uint32 moveType) const
{
    return moveType >= EXTERNAL_WAYPOINT_MOVE;
}

void npc_escortAI::MovementInform(uint32 moveType, uint32 pointId)
{
    if (!IsSD2EscortMovement(moveType) || !HasEscortState(STATE_ESCORT_ESCORTING))
        return;

    //uint32 pathId = uiMoveType & 0xFF;

    if (moveType < EXTERNAL_WAYPOINT_MOVE_START)
        WaypointReached(pointId);
    else if (moveType < EXTERNAL_WAYPOINT_FINISHED_LAST)
        WaypointStart(pointId);
    else                                                    // Last WP Reached
    {
        if (m_canInstantRespawn)
        {
            m_creature->SetDeathState(JUST_DIED);
            m_creature->Respawn();
        }
        else
            m_creature->ForcedDespawn();
    }
}

void npc_escortAI::SetCurrentWaypoint(uint32 pointId)
{
    if (!(HasEscortState(STATE_ESCORT_PAUSED)))             // Only when paused
    {
        script_error_log("EscortAI for %s tried to set new waypoint %u, but not paused", m_creature->GetGuidStr().c_str(), pointId);
        return;
    }

    if (!m_creature->GetMotionMaster()->SetNextWaypoint(pointId))
    {
        script_error_log("EscortAI for %s current waypoint tried to set to id %u, but doesn't exist in this path", m_creature->GetGuidStr().c_str(), pointId);
    }
}

void npc_escortAI::SetRun(bool run)
{
    if (run)
    {
        if (!m_isRunning)
            m_creature->SetWalk(false);
        else
            debug_log("SD2: EscortAI attempt to set run mode, but is already running.");
    }
    else
    {
        if (m_isRunning)
            m_creature->SetWalk(true);
        else
            debug_log("SD2: EscortAI attempt to set walk mode, but is already walking.");
    }
    m_isRunning = run;
}

// TODO: get rid of this many variables passed in function.
void npc_escortAI::Start(bool run, const Player* player, const Quest* quest, bool instantRespawn, bool canLoopPath, uint32 waypointPath)
{
    if (m_creature->GetVictim())
    {
        script_error_log("EscortAI attempt to Start while in combat for %s.", m_creature->GetScriptName().data());
        return;
    }

    if (HasEscortState(STATE_ESCORT_ESCORTING))
    {
        script_error_log("EscortAI attempt to Start while already escorting for %s.", m_creature->GetScriptName().data());
        return;
    }

    uint32 pathId = m_waypointPathID;
    WaypointPathOrigin origin = PATH_FROM_EXTERNAL;
    if (waypointPath)
    {
        pathId = waypointPath;
        origin = PATH_FROM_WAYPOINT_PATH;
    }

    if (!sWaypointMgr.GetPathFromOrigin(m_creature->GetEntry(), m_creature->GetGUIDLow(), pathId, origin))
    {
        script_error_log("EscortAI attempt to start escorting for %s, but has no waypoints loaded.", m_creature->GetScriptName().data());
        return;
    }

    if (origin == PATH_FROM_WAYPOINT_PATH)
        m_currentEscortWaypointPath = pathId;

    // set variables
    m_isRunning = run;

    m_playerGuid = player ? player->GetObjectGuid() : ObjectGuid();
    m_questForEscort = quest;

    m_canInstantRespawn = instantRespawn;
    m_canReturnToStart = canLoopPath;

    if (m_canReturnToStart && m_canInstantRespawn)
        debug_log("SD2: EscortAI is set to return home after waypoint end and instant respawn at waypoint end. Creature will never despawn.");

    // disable npcflags
    m_creature->SetUInt32Value(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_NONE);

    AddEscortState(STATE_ESCORT_ESCORTING);

    // Set initial speed
    m_creature->SetWalk(!m_isRunning);

    m_creature->StopMoving();

    // Start moving along the path with 2500ms delay
    m_creature->GetMotionMaster()->Clear(false, true);
    m_creature->GetMotionMaster()->MoveWaypoint(pathId, origin, 2500);

    JustStartedEscort();
}

void npc_escortAI::SetEscortPaused(bool paused)
{
    if (!HasEscortState(STATE_ESCORT_ESCORTING))
        return;

    if (paused)
    {
        AddEscortState(STATE_ESCORT_PAUSED);
        m_creature->GetMotionMaster()->PauseWaypoints(0);
    }
    else
    {
        RemoveEscortState(STATE_ESCORT_PAUSED);
        m_creature->GetMotionMaster()->UnpauseWaypoints();
    }
}

void npc_escortAI::SetEscortWaypoints(uint32 pathId)
{
    m_waypointPathID = pathId;
}
