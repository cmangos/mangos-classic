/* This file is part of the ScriptDev2 Project. See AUTHORS file for Copyright information
 * This program is free software licensed under GPL version 2
 * Please see the included DOCS/LICENSE.TXT for more information */

#ifndef SC_ESCORTAI_H
#define SC_ESCORTAI_H

// Remove this include, when EscortAI stores uint32 quest-id instead of Quest*
#include "Globals/ObjectMgr.h"
#include "AI/ScriptDevAI/include/sc_creature.h"

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
        explicit npc_escortAI(Creature* creature);
        ~npc_escortAI() {}

        void GetAIInformation(ChatHandler& reader) override;

        virtual void Aggro(Unit* who) override;

        virtual void Reset() override = 0;

        // UnitAI functions
        void JustDied(Unit* killer) override;

        void CorpseRemoved(uint32& /*respawnDelay*/) override;

        void UpdateAI(const uint32 diff) override;               // the "internal" update, calls UpdateEscortAI()
        virtual void UpdateEscortAI(const uint32 diff);          // used when it's needed to add code in update (abilities, scripted events, etc)

        void MovementInform(uint32 moveType, uint32 pointId) override;

        virtual void WaypointReached(uint32 pointId) = 0;
        virtual void WaypointStart(uint32 /*pointId*/) {}

        void Start(bool run = false, const Player* player = nullptr, const Quest* quest = nullptr, bool instantRespawn = false, bool canLoopPath = false);

        void SetRun(bool run = true);
        void SetEscortPaused(bool paused);

        bool HasEscortState(uint32 escortState) const { return (m_escortState & escortState) != 0; }

        // update current point
        void SetCurrentWaypoint(uint32 pointId);

        void FailQuestForPlayerAndGroup();

        bool AssistPlayerInCombat(Unit* who) override;
    protected:
        Player* GetPlayerForEscort() const { return m_creature->GetMap()->GetPlayer(m_playerGuid); }
        bool IsSD2EscortMovement(uint32 moveType) const;
        virtual void JustStartedEscort() {}

        void SetEscortWaypoints(uint32 pathId);
    private:
        bool IsPlayerOrGroupInRange();

        void AddEscortState(uint32 escortState) { m_escortState |= escortState; }
        void RemoveEscortState(uint32 escortState) { m_escortState &= ~escortState; }

        ObjectGuid m_playerGuid;
        uint32 m_playerCheckTimer;
        uint32 m_escortState;

        const Quest* m_questForEscort;                     // generally passed in Start() when regular escort script.

        bool m_isRunning;                                  // all creatures are walking by default (has flag SPLINEFLAG_WALKMODE)
        bool m_canInstantRespawn;                          // if creature should respawn instantly after escort over (if not, database respawntime are used)
        bool m_canReturnToStart;                           // if creature can walk same path (loop) without despawn. Not for regular escort quests.

        uint32 m_waypointPathID;
};

#endif
