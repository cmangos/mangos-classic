#pragma once

#include "Engine.h"

namespace ai
{
    struct Reaction
    {
        bool IsValid() const { return action != nullptr; }
        bool IsActive() const { return IsValid() ? duration > 100U : false; }
        void Reset() { action = nullptr; duration = 0U; }

        bool Update(uint32 elapsed);
        
        void SetAction(Action* inAction);
        Action* GetAction() const { return action; }
        
        void SetEvent(const Event& inEvent) { event = Event(inEvent); }
        Event& GetEvent() { return event; }
        
        uint32 GetDuration() const { return duration; }
        void SetDuration(uint32 inDuration) { duration = inDuration; }
        bool ShouldInterruptCast() const { return action ? action->ShouldReactionInterruptCast() : false; }
        bool ShouldInterruptMovement() const { return action ? action->ShouldReactionInterruptMovement() : false; }

    private:
        Event event;
        Action* action = nullptr;
        uint32 duration = 0U;
    };

    class ReactionEngine : public Engine
    {
    public:
        ReactionEngine(PlayerbotAI* ai, AiObjectContext* factory, BotState engineState);

        void Reset();
        bool Update(uint32 elapsed, bool minimal, bool isStunned, bool& reactionFound);
        bool IsReacting() const { return ongoingReaction.IsValid(); }
        bool HasIncomingReaction() const { return incomingReaction.IsValid(); }
        void SetReactionDuration(const Action* action);
        const Reaction* GetReaction() const;

    private:
        bool FindReaction(bool isStunned);
        bool StartReaction();
        void StopReaction();

        bool CanUpdateAIReaction() const;

        Action* InitializeAction(ActionNode* actionNode) override;
        bool ListenAndExecute(Action* action, Event& event) override;

    protected:
        Reaction incomingReaction;
        Reaction ongoingReaction;

    private:
        uint32 aiReactionUpdateDelay;
    };
}
