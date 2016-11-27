/* This file is part of the ScriptDev2 Project. See AUTHORS file for Copyright information
 * This program is free software licensed under GPL version 2
 * Please see the included DOCS/LICENSE.TXT for more information */

#ifndef SC_ESCORTAI_H
#define SC_ESCORTAI_H

// Remove this include, when EscortAI stores uint32 quest-id instead of Quest*
#include "ObjectMgr.h"

enum EscortState
{
    STATE_ESCORT_NONE       = 0x000,                        // nothing in progress
    STATE_ESCORT_ESCORTING  = 0x001,                        // escort are in progress
    STATE_ESCORT_RETURNING  = 0x002,                        // escort is returning after being in combat
    STATE_ESCORT_PAUSED     = 0x004                         // will not proceed with waypoints before state is removed
};

struct npc_escortAI : public ScriptedAI
{
    public:
        explicit npc_escortAI(Creature* pCreature);
        ~npc_escortAI() {}

        void GetAIInformation(ChatHandler& reader) override;

        virtual void Aggro(Unit*) override;

        virtual void Reset() override = 0;

        // CreatureAI functions
        bool IsVisible(Unit*) const override;

        void MoveInLineOfSight(Unit*) override;

        void JustDied(Unit*) override;

        void JustRespawned() override;

        void UpdateAI(const uint32) override;               // the "internal" update, calls UpdateEscortAI()
        virtual void UpdateEscortAI(const uint32);          // used when it's needed to add code in update (abilities, scripted events, etc)

        void MovementInform(uint32, uint32) override;

        virtual void WaypointReached(uint32 uiPointId) = 0;
        virtual void WaypointStart(uint32 /*uiPointId*/) {}

        void Start(bool bRun = false, const Player* pPlayer = nullptr, const Quest* pQuest = nullptr, bool bInstantRespawn = false, bool bCanLoopPath = false);

        void SetRun(bool bRun = true);
        void SetEscortPaused(bool uPaused);

        bool HasEscortState(uint32 uiEscortState) const { return !!(m_uiEscortState & uiEscortState); }

        // update current point
        void SetCurrentWaypoint(uint32 uiPointId);

    protected:
        Player* GetPlayerForEscort() { return m_creature->GetMap()->GetPlayer(m_playerGuid); }
        bool IsSD2EscortMovement(uint32 uiMoveType) const;
        virtual void JustStartedEscort() {}

    private:
        bool AssistPlayerInCombat(Unit* pWho);
        bool IsPlayerOrGroupInRange();

        void AddEscortState(uint32 uiEscortState) { m_uiEscortState |= uiEscortState; }
        void RemoveEscortState(uint32 uiEscortState) { m_uiEscortState &= ~uiEscortState; }

        ObjectGuid m_playerGuid;
        uint32 m_uiPlayerCheckTimer;
        uint32 m_uiEscortState;

        const Quest* m_pQuestForEscort;                     // generally passed in Start() when regular escort script.

        bool m_bIsRunning;                                  // all creatures are walking by default (has flag SPLINEFLAG_WALKMODE)
        bool m_bCanInstantRespawn;                          // if creature should respawn instantly after escort over (if not, database respawntime are used)
        bool m_bCanReturnToStart;                           // if creature can walk same path (loop) without despawn. Not for regular escort quests.
};

#endif
