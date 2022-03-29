/* This file is part of the ScriptDev2 Project. See AUTHORS file for Copyright information
 * This program is free software licensed under GPL version 2
 * Please see the included DOCS/LICENSE.TXT for more information */

#ifndef SC_FOLLOWERAI_H
#define SC_FOLLOWERAI_H

#include "AI/ScriptDevAI/include/sc_creature.h"

enum FollowState
{
    STATE_FOLLOW_NONE       = 0x000,
    STATE_FOLLOW_INPROGRESS = 0x001,                        // must always have this state for any follow
    STATE_FOLLOW_RETURNING  = 0x002,                        // when returning to combat start after being in combat
    STATE_FOLLOW_PAUSED     = 0x004,                        // disables following
    STATE_FOLLOW_COMPLETE   = 0x008,                        // follow is completed and may end
    STATE_FOLLOW_PREEVENT   = 0x010,                        // not implemented (allow pre event to run, before follow is initiated)
    STATE_FOLLOW_POSTEVENT  = 0x020                         // can be set at complete and allow post event to run
};

class FollowerAI : public ScriptedAI
{
    public:
        explicit FollowerAI(Creature* creature);
        ~FollowerAI() {}

        // virtual void WaypointReached(uint32 pointId) = 0;

        void MovementInform(uint32 motionType, uint32 pointId) override;

        void EnterEvadeMode() override;

        void JustDied(Unit* killer) override;

        void CorpseRemoved(uint32& /*respawnDelay*/) override;

        void UpdateAI(const uint32 diff) override;               // the "internal" update, calls UpdateFollowerAI()
        virtual void UpdateFollowerAI(const uint32 diff);        // used when it's needed to add code in update (abilities, scripted events, etc)

        void StartFollow(Player* leader, uint32 factionForFollower = 0, const Quest* quest = nullptr);

        void SetFollowPaused(bool paused);                 // if special event require follow mode to hold/resume during the follow
        void SetFollowComplete(bool withEndEvent = false);

        bool HasFollowState(uint32 followState) const { return (m_followState & followState) != 0; }

        bool AssistPlayerInCombat(Unit* who) override;

    protected:
        Player* GetLeaderForFollower();

    private:
        void AddFollowState(uint32 uiFollowState) { m_followState |= uiFollowState; }
        void RemoveFollowState(uint32 uiFollowState) { m_followState &= ~uiFollowState; }

        ObjectGuid m_leaderGuid;
        uint32 m_updateFollowTimer;
        uint32 m_followState;

        const Quest* m_questForFollow;                     // normally we have a quest
};

#endif
