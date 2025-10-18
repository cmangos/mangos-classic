#pragma once
#include "MovementActions.h"
#include "GenericActions.h"

namespace ai
{
   class WaitForAttackKeepSafeDistanceAction : public MovementAction
   {
   public:
       WaitForAttackKeepSafeDistanceAction(PlayerbotAI* ai) : MovementAction(ai, "wait for attack keep safe distance") {}
       virtual bool Execute(Event& event);

   private:
       const WorldPosition GetBestPoint(Unit* target, float minDistance, float maxDistance) const;
       bool IsEnemyClose(const WorldPosition& point, const std::list<ObjectGuid>& enemies) const;
       virtual bool isUsefulWhenStunned() override { return true; }
   };
}
