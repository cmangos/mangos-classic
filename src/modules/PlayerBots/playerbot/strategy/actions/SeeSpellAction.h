#pragma once

#include "playerbot/strategy/Action.h"
#include "MovementActions.h"
#include "playerbot/strategy/values/LastMovementValue.h"
#include "playerbot/strategy/values/RTSCValues.h"

namespace ai
{
    class SeeSpellAction : public MovementAction
    {
    public:
        SeeSpellAction(PlayerbotAI* ai, std::string name = "see spell") : MovementAction(ai, name) {}
        virtual bool Execute(Event& event);

        virtual bool isPossible() override { return true; }
        virtual bool isUseful() override;

        bool SelectSpell(Player* requester, WorldPosition& spellPosition);

        virtual bool MoveToSpell(Player* requester, WorldPosition& spellPosition, bool inFormation = true);
        void SetFormationOffset(Player* requester, WorldPosition& spellPosition);
    private:
        Creature* CreateWps(Player* wpOwner, float x, float y, float z, float o, uint32 entry, Creature* lastWp, bool important = false);
    };
}