/* This file is part of the ScriptDev2 Project. See AUTHORS file for Copyright information
 * This program is free software licensed under GPL version 2
 * Please see the included DOCS/LICENSE.TXT for more information */

/* ScriptData
SDName: FollowerAI
SD%Complete: 60
SDComment: This AI is under development
SDCategory: Npc
EndScriptData */

#include "AI/ScriptDevAI/include/precompiled.h"
#include "follower_ai.h"

const float MAX_PLAYER_DISTANCE = 100.0f;

enum
{
    POINT_COMBAT_START  = 0xFFFFFF
};

FollowerAI::FollowerAI(Creature* creature) : ScriptedAI(creature),
    m_updateFollowTimer(2500),
    m_followState(STATE_FOLLOW_NONE),
    m_questForFollow(nullptr)
{}

void FollowerAI::AttackStart(Unit* who)
{
    if (!who)
        return;

    if (m_creature->Attack(who, m_meleeEnabled))
    {
        m_creature->AddThreat(who);
        m_creature->SetInCombatWith(who);
        who->SetInCombatWith(m_creature);

        if (IsCombatMovement())
            m_creature->GetMotionMaster()->MoveChase(who);
    }
}

// This part provides assistance to a player that are attacked by pWho, even if out of normal aggro range
// It will cause m_creature to attack pWho that are attacking _any_ player (which has been confirmed may happen also on offi)
bool FollowerAI::AssistPlayerInCombat(Unit* who)
{
    if (!HasFollowState(STATE_FOLLOW_INPROGRESS))
        return false;

    if (!who->getVictim())
        return false;

    // experimental (unknown) flag not present
    if (!(m_creature->GetCreatureInfo()->CreatureTypeFlags & CREATURE_TYPEFLAGS_CAN_ASSIST))
        return false;

    // unit state prevents (similar check is done in CanInitiateAttack which also include checking unit_flags. We skip those here)
    if (m_creature->hasUnitState(UNIT_STAT_STUNNED | UNIT_STAT_FEIGN_DEATH | UNIT_STAT_CONFUSED | UNIT_STAT_FLEEING))
        return false;

    // victim of pWho is not a player
    if (!who->getVictim()->GetBeneficiaryPlayer())
        return false;

    // never attack friendly
    if (!m_creature->CanAttackNow(who))
        return false;

    // too far away and no free sight?
    if (m_creature->IsWithinDistInMap(who, MAX_PLAYER_DISTANCE) && m_creature->IsWithinLOSInMap(who))
    {
        // already fighting someone?
        if (!m_creature->getVictim())
        {
            AttackStart(who);
            return true;
        }
        who->SetInCombatWith(m_creature);
        m_creature->AddThreat(who);
        return true;
    }

    return false;
}

void FollowerAI::JustDied(Unit* /*killer*/)
{
    if (!HasFollowState(STATE_FOLLOW_INPROGRESS) || !m_leaderGuid || !m_questForFollow)
        return;

    // TODO: need a better check for quests with time limit.
    if (Player* player = GetLeaderForFollower())
    {
        if (Group* group = player->GetGroup())
        {
            for (GroupReference* ref = group->GetFirstMember(); ref != nullptr; ref = ref->next())
            {
                if (Player* member = ref->getSource())
                {
                    if (member->GetQuestStatus(m_questForFollow->GetQuestId()) == QUEST_STATUS_INCOMPLETE)
                        member->FailQuest(m_questForFollow->GetQuestId());
                }
            }
        }
        else
        {
            if (player->GetQuestStatus(m_questForFollow->GetQuestId()) == QUEST_STATUS_INCOMPLETE)
                player->FailQuest(m_questForFollow->GetQuestId());
        }
    }
}

void FollowerAI::CorpseRemoved(uint32& /*respawnDelay*/)
{
    m_followState = STATE_FOLLOW_NONE;

    if (!IsCombatMovement())
        SetCombatMovement(true);
}

void FollowerAI::EnterEvadeMode()
{
    m_creature->RemoveAllAurasOnEvade();
    m_creature->CombatStop(true);

    if (HasFollowState(STATE_FOLLOW_INPROGRESS))
    {
        debug_log("SD2: FollowerAI left combat, returning to CombatStartPosition.");

        if (m_creature->GetMotionMaster()->GetCurrentMovementGeneratorType() == CHASE_MOTION_TYPE)
        {
            float posX, posY, posZ, posO;
            m_creature->GetCombatStartPosition(posX, posY, posZ, posO);
            m_creature->GetMotionMaster()->MovePoint(POINT_COMBAT_START, posX, posY, posZ);
        }
    }
    else
    {
        if (m_creature->isAlive())
            m_creature->GetMotionMaster()->MoveTargetedHome();
    }

    m_creature->SetLootRecipient(nullptr);

    Reset();
}

void FollowerAI::UpdateAI(const uint32 diff)
{
    if (HasFollowState(STATE_FOLLOW_INPROGRESS) && !m_creature->getVictim())
    {
        if (m_updateFollowTimer < diff)
        {
            if (HasFollowState(STATE_FOLLOW_COMPLETE) && !HasFollowState(STATE_FOLLOW_POSTEVENT))
            {
                debug_log("SD2: FollowerAI is set completed, despawns.");
                m_creature->ForcedDespawn();
                return;
            }

            bool isMaxRangeExceeded = true;

            if (Player* player = GetLeaderForFollower())
            {
                if (HasFollowState(STATE_FOLLOW_RETURNING))
                {
                    debug_log("SD2: FollowerAI is returning to leader.");

                    RemoveFollowState(STATE_FOLLOW_RETURNING);
                    m_creature->GetMotionMaster()->MoveFollow(player, PET_FOLLOW_DIST, PET_FOLLOW_ANGLE);
                    return;
                }

                if (Group* pGroup = player->GetGroup())
                {
                    for (GroupReference* ref = pGroup->GetFirstMember(); ref != nullptr; ref = ref->next())
                    {
                        Player* member = ref->getSource();

                        if (member && m_creature->IsWithinDistInMap(member, MAX_PLAYER_DISTANCE))
                        {
                            isMaxRangeExceeded = false;
                            break;
                        }
                    }
                }
                else
                {
                    if (m_creature->IsWithinDistInMap(player, MAX_PLAYER_DISTANCE))
                        isMaxRangeExceeded = false;
                }
            }

            if (isMaxRangeExceeded)
            {
                debug_log("SD2: FollowerAI failed because player/group was to far away or not found");
                m_creature->ForcedDespawn();
                return;
            }

            m_updateFollowTimer = 1000;
        }
        else
            m_updateFollowTimer -= diff;
    }

    UpdateFollowerAI(diff);
}

void FollowerAI::UpdateFollowerAI(const uint32 /*diff*/)
{
    if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
        return;

    DoMeleeAttackIfReady();
}

void FollowerAI::MovementInform(uint32 motionType, uint32 pointId)
{
    if (motionType != POINT_MOTION_TYPE || !HasFollowState(STATE_FOLLOW_INPROGRESS))
        return;

    if (pointId == POINT_COMBAT_START)
    {
        if (GetLeaderForFollower())
        {
            if (!HasFollowState(STATE_FOLLOW_PAUSED))
                AddFollowState(STATE_FOLLOW_RETURNING);
        }
        else
            m_creature->ForcedDespawn();
    }
}

void FollowerAI::StartFollow(Player* leader, uint32 factionForFollower, const Quest* quest)
{
    if (m_creature->getVictim())
    {
        debug_log("SD2: FollowerAI attempt to StartFollow while in combat.");
        return;
    }

    if (HasFollowState(STATE_FOLLOW_INPROGRESS))
    {
        script_error_log("FollowerAI attempt to StartFollow while already following.");
        return;
    }

    // set variables
    m_leaderGuid = leader->GetObjectGuid();

    if (factionForFollower)
        m_creature->SetFactionTemporary(factionForFollower, TEMPFACTION_RESTORE_RESPAWN);

    m_questForFollow = quest;

    if (m_creature->GetMotionMaster()->GetCurrentMovementGeneratorType() == WAYPOINT_MOTION_TYPE)
    {
        m_creature->GetMotionMaster()->Clear();
        m_creature->GetMotionMaster()->MoveIdle();
        debug_log("SD2: FollowerAI start with WAYPOINT_MOTION_TYPE, set to MoveIdle.");
    }

    m_creature->SetUInt32Value(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_NONE);

    AddFollowState(STATE_FOLLOW_INPROGRESS);

    m_creature->GetMotionMaster()->MoveFollow(leader, PET_FOLLOW_DIST, PET_FOLLOW_ANGLE);

    debug_log("SD2: FollowerAI start follow %s (Guid %s)", leader->GetName(), m_leaderGuid.GetString().c_str());
}

Player* FollowerAI::GetLeaderForFollower()
{
    if (Player* leader = m_creature->GetMap()->GetPlayer(m_leaderGuid))
    {
        if (leader->isAlive())
            return leader;
        if (Group* group = leader->GetGroup())
        {
            for (GroupReference* ref = group->GetFirstMember(); ref != nullptr; ref = ref->next())
            {
                Player* member = ref->getSource();

                if (member && member->isAlive() && m_creature->IsWithinDistInMap(member, MAX_PLAYER_DISTANCE))
                {
                    debug_log("SD2: FollowerAI GetLeader changed and returned new leader.");
                    m_leaderGuid = member->GetObjectGuid();
                    return member;
                }
            }
        }
    }

    debug_log("SD2: FollowerAI GetLeader can not find suitable leader.");
    return nullptr;
}

void FollowerAI::SetFollowComplete(bool withEndEvent)
{
    if (m_creature->GetMotionMaster()->GetCurrentMovementGeneratorType() == FOLLOW_MOTION_TYPE)
    {
        m_creature->StopMoving();
        m_creature->GetMotionMaster()->Clear();
        m_creature->GetMotionMaster()->MoveIdle();
    }

    if (withEndEvent)
        AddFollowState(STATE_FOLLOW_POSTEVENT);
    else
    {
        if (HasFollowState(STATE_FOLLOW_POSTEVENT))
            RemoveFollowState(STATE_FOLLOW_POSTEVENT);
    }

    AddFollowState(STATE_FOLLOW_COMPLETE);
}

void FollowerAI::SetFollowPaused(bool paused)
{
    if (!HasFollowState(STATE_FOLLOW_INPROGRESS) || HasFollowState(STATE_FOLLOW_COMPLETE))
        return;

    if (paused)
    {
        AddFollowState(STATE_FOLLOW_PAUSED);

        if (m_creature->GetMotionMaster()->GetCurrentMovementGeneratorType() == FOLLOW_MOTION_TYPE)
        {
            m_creature->StopMoving();
            m_creature->GetMotionMaster()->Clear();
            m_creature->GetMotionMaster()->MoveIdle();
        }
    }
    else
    {
        RemoveFollowState(STATE_FOLLOW_PAUSED);

        if (Player* leader = GetLeaderForFollower())
            m_creature->GetMotionMaster()->MoveFollow(leader, PET_FOLLOW_DIST, PET_FOLLOW_ANGLE);
    }
}
