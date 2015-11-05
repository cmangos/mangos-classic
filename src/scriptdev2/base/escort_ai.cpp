/* This file is part of the ScriptDev2 Project. See AUTHORS file for Copyright information
 * This program is free software licensed under GPL version 2
 * Please see the included DOCS/LICENSE.TXT for more information */

/* ScriptData
SDName: EscortAI
SD%Complete: 100
SDComment:
SDCategory: Npc
EndScriptData */

#include "precompiled.h"
#include "escort_ai.h"
#include "../system/system.h"

const float MAX_PLAYER_DISTANCE = 66.0f;

npc_escortAI::npc_escortAI(Creature* pCreature) : ScriptedAI(pCreature),
    m_uiPlayerCheckTimer(1000),
    m_uiEscortState(STATE_ESCORT_NONE),
    m_pQuestForEscort(nullptr),
    m_bIsRunning(false),
    m_bCanInstantRespawn(false),
    m_bCanReturnToStart(false)
{}

void npc_escortAI::GetAIInformation(ChatHandler& reader)
{
    std::ostringstream oss;

    oss << "EscortAI ";
    if (m_playerGuid)
        oss << "started for " << m_playerGuid.GetString() << " ";
    if (m_pQuestForEscort)
        oss << "started with quest " << m_pQuestForEscort->GetQuestId();

    if (HasEscortState(STATE_ESCORT_ESCORTING))
    {
        oss << "\nEscortFlags: Escorting" << (HasEscortState(STATE_ESCORT_RETURNING) ? ", Returning" : "") << (HasEscortState(STATE_ESCORT_PAUSED) ? ", Paused" : "") << "\n";
        m_creature->GetMotionMaster()->GetWaypointPathInformation(oss);
    }

    reader.PSendSysMessage("%s", oss.str().c_str());
}

bool npc_escortAI::IsVisible(Unit* pWho) const
{
    if (!pWho)
        return false;

    return m_creature->IsWithinDist(pWho, VISIBLE_RANGE) && pWho->isVisibleForOrDetect(m_creature, m_creature, true);
}

void npc_escortAI::Aggro(Unit* /*pEnemy*/) {}

// see followerAI
bool npc_escortAI::AssistPlayerInCombat(Unit* pWho)
{
    if (!pWho->getVictim())
        return false;

    // experimental (unknown) flag not present
    if (!(m_creature->GetCreatureInfo()->CreatureTypeFlags & CREATURE_TYPEFLAGS_CAN_ASSIST))
        return false;

    // unit state prevents (similar check is done in CanInitiateAttack which also include checking unit_flags. We skip those here)
    if (m_creature->hasUnitState(UNIT_STAT_STUNNED | UNIT_STAT_DIED))
        return false;

    // victim of pWho is not a player
    if (!pWho->getVictim()->GetCharmerOrOwnerPlayerOrPlayerItself())
        return false;

    // never attack friendly
    if (m_creature->IsFriendlyTo(pWho))
        return false;

    // too far away and no free sight?
    if (m_creature->IsWithinDistInMap(pWho, MAX_PLAYER_DISTANCE) && m_creature->IsWithinLOSInMap(pWho))
    {
        // already fighting someone?
        if (!m_creature->getVictim())
        {
            AttackStart(pWho);
            return true;
        }
        else
        {
            pWho->SetInCombatWith(m_creature);
            m_creature->AddThreat(pWho);
            return true;
        }
    }

    return false;
}

void npc_escortAI::MoveInLineOfSight(Unit* pWho)
{
    if (pWho->isTargetableForAttack() && pWho->isInAccessablePlaceFor(m_creature))
    {
        // AssistPlayerInCombat can start attack, so return if true
        if (HasEscortState(STATE_ESCORT_ESCORTING) && AssistPlayerInCombat(pWho))
            return;

        if (!m_creature->CanInitiateAttack())
            return;

        if (!m_creature->CanFly() && m_creature->GetDistanceZ(pWho) > CREATURE_Z_ATTACK_RANGE)
            return;

        if (m_creature->IsHostileTo(pWho))
        {
            float fAttackRadius = m_creature->GetAttackDistance(pWho);
            if (m_creature->IsWithinDistInMap(pWho, fAttackRadius) && m_creature->IsWithinLOSInMap(pWho))
            {
                if (!m_creature->getVictim())
                {
                    pWho->RemoveSpellsCausingAura(SPELL_AURA_MOD_STEALTH);
                    AttackStart(pWho);
                }
                else if (m_creature->GetMap()->IsDungeon())
                {
                    pWho->SetInCombatWith(m_creature);
                    m_creature->AddThreat(pWho);
                }
            }
        }
    }
}

void npc_escortAI::JustDied(Unit* /*pKiller*/)
{
    if (!HasEscortState(STATE_ESCORT_ESCORTING) || !m_playerGuid || !m_pQuestForEscort)
        return;

    if (Player* pPlayer = GetPlayerForEscort())
    {
        if (Group* pGroup = pPlayer->GetGroup())
        {
            for (GroupReference* pRef = pGroup->GetFirstMember(); pRef != nullptr; pRef = pRef->next())
            {
                if (Player* pMember = pRef->getSource())
                {
                    if (pMember->GetQuestStatus(m_pQuestForEscort->GetQuestId()) == QUEST_STATUS_INCOMPLETE)
                        pMember->FailQuest(m_pQuestForEscort->GetQuestId());
                }
            }
        }
        else
        {
            if (pPlayer->GetQuestStatus(m_pQuestForEscort->GetQuestId()) == QUEST_STATUS_INCOMPLETE)
                pPlayer->FailQuest(m_pQuestForEscort->GetQuestId());
        }
    }
}

void npc_escortAI::JustRespawned()
{
    m_uiEscortState = STATE_ESCORT_NONE;

    if (!IsCombatMovement())
        SetCombatMovement(true);

    Reset();
}

bool npc_escortAI::IsPlayerOrGroupInRange()
{
    if (Player* pPlayer = GetPlayerForEscort())
    {
        if (Group* pGroup = pPlayer->GetGroup())
        {
            for (GroupReference* pRef = pGroup->GetFirstMember(); pRef != nullptr; pRef = pRef->next())
            {
                Player* pMember = pRef->getSource();
                if (pMember && m_creature->IsWithinDistInMap(pMember, MAX_PLAYER_DISTANCE))
                    return true;
            }
        }
        else
        {
            if (m_creature->IsWithinDistInMap(pPlayer, MAX_PLAYER_DISTANCE))
                return true;
        }
    }
    return false;
}

void npc_escortAI::UpdateAI(const uint32 uiDiff)
{
    // Check if player or any member of his group is within range
    if (HasEscortState(STATE_ESCORT_ESCORTING) && m_playerGuid && !m_creature->getVictim() && !HasEscortState(STATE_ESCORT_RETURNING))
    {
        if (m_uiPlayerCheckTimer < uiDiff)
        {
            if (!HasEscortState(STATE_ESCORT_PAUSED) && !IsPlayerOrGroupInRange())
            {
                debug_log("SD2: EscortAI failed because player/group was to far away or not found");

                if (m_bCanInstantRespawn)
                {
                    m_creature->SetDeathState(JUST_DIED);
                    m_creature->Respawn();
                }
                else
                    m_creature->ForcedDespawn();

                return;
            }

            m_uiPlayerCheckTimer = 1000;
        }
        else
            m_uiPlayerCheckTimer -= uiDiff;
    }

    UpdateEscortAI(uiDiff);
}

void npc_escortAI::UpdateEscortAI(const uint32 /*uiDiff*/)
{
    // Check if we have a current target
    if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
        return;

    DoMeleeAttackIfReady();
}

/// Helper function for transition between old Escort Movment and using WaypointMMGen
bool npc_escortAI::IsSD2EscortMovement(uint32 uiMoveType) const
{
    return uiMoveType >= EXTERNAL_WAYPOINT_MOVE;
}

void npc_escortAI::MovementInform(uint32 uiMoveType, uint32 uiPointId)
{
    if (!IsSD2EscortMovement(uiMoveType) || !HasEscortState(STATE_ESCORT_ESCORTING))
        return;

    //uint32 pathId = uiMoveType & 0xFF;

    if (uiMoveType < EXTERNAL_WAYPOINT_MOVE_START)
        WaypointReached(uiPointId);
    else if (uiMoveType < EXTERNAL_WAYPOINT_FINISHED_LAST)
        WaypointStart(uiPointId);
    else                                                    // Last WP Reached
    {
        if (m_bCanInstantRespawn)
        {
            m_creature->SetDeathState(JUST_DIED);
            m_creature->Respawn();
        }
        else
            m_creature->ForcedDespawn();
    }
}

void npc_escortAI::SetCurrentWaypoint(uint32 uiPointId)
{
    if (!(HasEscortState(STATE_ESCORT_PAUSED)))             // Only when paused
    {
        script_error_log("EscortAI for %s tried to set new waypoint %u, but not paused", m_creature->GetGuidStr().c_str(), uiPointId);
        return;
    }

    if (!m_creature->GetMotionMaster()->SetNextWaypoint(uiPointId))
    {
        script_error_log("EscortAI for %s current waypoint tried to set to id %u, but doesn't exist in this path", m_creature->GetGuidStr().c_str(), uiPointId);
        return;
    }
}

void npc_escortAI::SetRun(bool bRun)
{
    if (bRun)
    {
        if (!m_bIsRunning)
            m_creature->SetWalk(false);
        else
            debug_log("SD2: EscortAI attempt to set run mode, but is already running.");
    }
    else
    {
        if (m_bIsRunning)
            m_creature->SetWalk(true);
        else
            debug_log("SD2: EscortAI attempt to set walk mode, but is already walking.");
    }
    m_bIsRunning = bRun;
}

// TODO: get rid of this many variables passed in function.
void npc_escortAI::Start(bool bRun, const Player* pPlayer, const Quest* pQuest, bool bInstantRespawn, bool bCanLoopPath)
{
    if (m_creature->getVictim())
    {
        script_error_log("EscortAI attempt to Start while in combat.");
        return;
    }

    if (HasEscortState(STATE_ESCORT_ESCORTING))
    {
        script_error_log("EscortAI attempt to Start while already escorting.");
        return;
    }

    if (!pSystemMgr.GetPathInfo(m_creature->GetEntry(), 1))
    {
        script_error_log("EscortAI attempt to start escorting for %s, but has no waypints loaded", m_creature->GetGuidStr().c_str());
        return;
    }

    // set variables
    m_bIsRunning = bRun;

    m_playerGuid = pPlayer ? pPlayer->GetObjectGuid() : ObjectGuid();
    m_pQuestForEscort = pQuest;

    m_bCanInstantRespawn = bInstantRespawn;
    m_bCanReturnToStart = bCanLoopPath;

    if (m_bCanReturnToStart && m_bCanInstantRespawn)
        debug_log("SD2: EscortAI is set to return home after waypoint end and instant respawn at waypoint end. Creature will never despawn.");

    // disable npcflags
    m_creature->SetUInt32Value(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_NONE);

    AddEscortState(STATE_ESCORT_ESCORTING);

    // Set initial speed
    m_creature->SetWalk(!m_bIsRunning);

    m_creature->StopMoving();

    // Start moving along the path with 2500ms delay
    m_creature->GetMotionMaster()->Clear(false, true);
    m_creature->GetMotionMaster()->MoveWaypoint(1, 3, 2500);

    JustStartedEscort();
}

void npc_escortAI::SetEscortPaused(bool bPaused)
{
    if (!HasEscortState(STATE_ESCORT_ESCORTING))
        return;

    if (bPaused)
    {
        AddEscortState(STATE_ESCORT_PAUSED);
        m_creature->addUnitState(UNIT_STAT_WAYPOINT_PAUSED);
    }
    else
    {
        RemoveEscortState(STATE_ESCORT_PAUSED);
        m_creature->clearUnitState(UNIT_STAT_WAYPOINT_PAUSED);
    }
}
